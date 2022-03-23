#include"../CameraData.hlsli"
#include"../LightData.hlsli"


cbuffer CameraData : register(b0)
{
	CameraData cameraData;
}

cbuffer LightData : register(b1)
{
	LightData lightData;
}

//デプスバッファ
Texture2D<float> depthBuffer : register(t0);

//出力
//影響を受けるライトの添え字が格納される
RWStructuredBuffer<uint> pointLightIndexBuffer : register(u0);

SamplerState smp: register(s0);

//共有メモリ
//タイルの最小深度
groupshared uint minZ;
//タイルの最大深度
groupshared uint maxZ;
//タイルに影響するポイントライトのインデックス
groupshared uint tileLightIndex[MAX_POINT_LIGHT_NUM];
//タイルに影響するポイントライトの数
groupshared uint tileLightNum;


//タイルごとの視推台を求める
void GetTileFrustumPlane(out float4 frustumPlanes[6], uint3 groupID)
{
	//タイルの最大最小深度を浮動小数点に変換
	float minTileZ = asfloat(minZ);
	float maxTileZ = asfloat(maxZ);

	float2 tileScale = float2(cameraData.screenWidth, cameraData.screenHeight) * rcp(float2(2 * TILE_WIDTH, 2 * TILE_HEIGHT));
	float2 tileBias = tileScale - float2(groupID.xy);

	float4 c1 = float4(cameraData.proj._11 * tileScale.x, 0.f, tileBias.x, 0.f);
	float4 c2 = float4(0.f, -cameraData.proj._22 * tileScale.y, tileBias.y, 0.f);
	float4 c4 = float4(0.f, 0.f, 1.f, 0.f);

	//右の面の法線
	frustumPlanes[0] = c4 - c1;
	//左の面の法線
	frustumPlanes[1] = c4 + c1;
	//上の面の法線
	frustumPlanes[2] = c4 - c2;
	//下の面の法線
	frustumPlanes[3] = c4 + c2;
	//奥の面の法線
	frustumPlanes[4] = float4(0.f, 0.f, 1.f, -minTileZ);//この面は原点を通らないので第四成分が0ではない
	//手前の面の法線
	frustumPlanes[5] = float4(0.f, 0.f, -1.f, maxTileZ);//この面は原点を通らないので第四成分が0ではない

	//正規化する
	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
	}
}


//カメラ空間での座標を計算する
float3 ComputePositionInCamera(uint2 globalCoords)
{
	//globalCoordsから-1,1の範囲に収めた座標を導出
	float2 st = ((float2)globalCoords + 0.5) * rcp(float2(cameraData.screenWidth, cameraData.screenHeight));
	st = st * float2(2.f, -2.f) - float2(1.f, -1.f);

	float3 screenPos;
	screenPos.xy = st.xy;
	screenPos.z = depthBuffer.SampleLevel(smp, globalCoords, 0);
	float4 cameraPos = mul(cameraData.projInv, float4(screenPos, 1.f));

	return cameraPos.xyz / cameraPos.w;
}



[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID,uint3 groupThreadID : SV_GroupThreadID )
{
	//タイル内でのインデックス
	uint groupIndex = groupThreadID.y * TILE_WIDTH + groupThreadID.x;

	//共有メモリの初期化
	if (groupIndex == 0)
	{
		tileLightNum = 0;
		minZ = 0x7F7FFFFF;//floatの最大値
		maxZ = 0;
	}

	uint2 frameUV = dispatchThreadID.xy;
	uint numCellX = (cameraData.screenWidth + TILE_WIDTH - 1) / TILE_WIDTH;
	uint tileIndex = floor(frameUV.x / TILE_WIDTH) + floor(frameUV.y / TILE_HEIGHT) * numCellX;
	uint lightStart = lightData.pointLightNum * tileIndex;

	//結果を格納するバッファの初期化
	//ClearUnorderedAccessViewUintってコマンドがあるけど
	//アップロード用の別のバッファをつくらなければ使えないっぽく
	//手間かかりそうなのでここで初期化する
	for (uint lightIndex = groupIndex; lightIndex < lightData.pointLightNum; lightIndex += TILE_NUM)
	{
		pointLightIndexBuffer[lightStart + lightIndex] = 0xffffffff;
	}

	//ビュー空間での座標
	float3 posInView = ComputePositionInCamera(frameUV);

	//他のグループを待つ
	GroupMemoryBarrierWithGroupSync();


	//タイルの最大最小深度を求める
	InterlockedMin(minZ, asuint(posInView.z));
	InterlockedMax(maxZ, asuint(posInView.z));

	//他のグループを待つ
	GroupMemoryBarrierWithGroupSync();


	//タイルの推台を求める
	float4 frustumPlanes[6];
	GetTileFrustumPlane(frustumPlanes, groupID);

	//タイルとポイントライトの衝突判定
	for (uint lightIndex = groupIndex; lightIndex < lightData.pointLightNum; lightIndex += TILE_NUM)
	{
		PointLight pointLight = lightData.pointLight[lightIndex];

		//falseなら衝突していない
		bool isHit = true;
		for (uint i = 0; i < 6; i++)
		{
			float4 lp = float4(pointLight.posInView, 1.f);
			float d = dot(frustumPlanes[i], lp);
			isHit = isHit && (d >= -pointLight.range);
		}
		
		//タイルと接触している場合
		if (isHit)
		{
			uint listIndex = 0;
			//tileLightNumに1を加算
			//listIndexには加算される前の値が格納される
			InterlockedAdd(tileLightNum, 1, listIndex);
			tileLightIndex[listIndex] = lightIndex;
		}
	}

	//他のグループを待つ
	GroupMemoryBarrierWithGroupSync();


	//結果を格納する
	for (uint lightIndex = groupIndex; lightIndex < tileLightNum; lightIndex += TILE_NUM)
	{
		pointLightIndexBuffer[lightStart + lightIndex] = tileLightIndex[lightIndex];
	}

}
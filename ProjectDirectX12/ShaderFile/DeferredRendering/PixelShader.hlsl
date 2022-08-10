#include"Header.hlsli"

static const int POISSON_DISK_SAMPLE_CNT = 12;
static const float2 POISSON_DISK[12] = {
	float2(0.0191375, 0.635275),
	float2(0.396322, 0.873851),
	float2(-0.588224, 0.588251),
	float2(-0.3404, 0.0154557),
	float2(0.510869, 0.0278614),
	float2(-0.15801, -0.659996),
	float2(0.120268, -0.200636),
	float2(-0.925312, -0.0306309),
	float2(-0.561635, -0.32798),
	float2(0.424297, -0.852628),
	float2(0.923275, -0.191526),
	float2(0.703181, 0.556563)
};

uint Alignment(uint size, uint alignment) {
	return size + alignment - size % alignment;
}


float3 CalcDiffuse(float3 lightDir, float3 lightColor, float3 normal)
{
	float t = saturate(dot(normal, -lightDir));
	return lightColor * t;
}

float3 CalcSpecular(float3 lightDir, float3 lightColor, float3 normal, float3 toEye)
{
	float3 r = reflect(lightDir, normal);
	float t = pow(saturate(dot(r, toEye)), lightData.specPow);
	return lightColor * t;
}

float3 CalcDirectionLight(float3 normal, float3 toEye)
{
	float3 result = float3(0.f, 0.f, 0.f);
	result += CalcDiffuse(normalize(lightData.directionLight.dir), lightData.directionLight.color, normal);
	result += CalcSpecular(normalize(lightData.directionLight.dir), lightData.directionLight.color, normal, toEye);
	return result;
}

float3 CalcPointLight(float2 uv,float3 worldPos,float3 normal,float3 toEye)
{
	uint screenWidth = Alignment(cameraData.screenWidth, TILE_WIDTH);

	// スクリーンをタイルで分割した時のセルのX座標
	uint numCellX = (screenWidth + TILE_WIDTH - 1) / TILE_WIDTH;

	// タイルのインデックス
	uint tileIndex = floor(uv.x / TILE_WIDTH) + floor(uv.y / TILE_HEIGHT) * numCellX;

	// このピクセルが含まれるタイルのライトインデックスのリストの開始位置
	uint indexStart = tileIndex * lightData.pointLightNum;

	// このピクセルが含まれるライトインデックスのリストの終了位置
	uint indexEnd = indexStart + lightData.pointLightNum;

	float3 result = float3(0.f, 0.f, 0.f);


	for (uint i = indexStart; i < indexEnd; i++)
	{
		uint pointLightIndex = pointLightIndexBuffer[i];
		if (pointLightIndex == 0xffffffff)
		{
			break;
		}

		float3 lightDir = normalize(worldPos.xyz - lightData.pointLight[pointLightIndex].pos.xyz);
		float distance = length(worldPos.xyz - lightData.pointLight[pointLightIndex].pos.xyz);

		// 影響率
		float affect = 1.f - min(1.f, distance / lightData.pointLight[pointLightIndex].range);

		// affectの値を線形から変更する
		// そしたらスぺきゅらの計算を行う
		result += CalcDiffuse(lightDir, lightData.pointLight[pointLightIndex].color, normal) * affect;
		// result += CalcSpecular(lightDir, lightData.pointLight[pointLightIndex].color, normal, toEye) * affect;
	}
	
	return result;
}


PSOutput main(VSOutput input)
{
	PSOutput output;

	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);

	if (depthBuffer.Sample(smp, input.uv) >= 1.f)
	{
		output.color = albedoColor;
		output.highLuminance = float4(0.f, 0.f, 0.f, 0.f);
		return output;
	}

	float3 normal = normalTexture.Sample(smp, input.uv).xyz;
	// -1,1の範囲に収める
	normal = (normal * 2.f) - 1.f;
	float4 worldPosition = float4(worldPositionTexture.Sample(smp, input.uv).xyz, 1.f);
	float3 toEye = normalize(cameraData.eyePos.xyz - worldPosition.xyz);

	float3 ambient = (albedoColor * 0.6f).xyz;
	float3 directionLightColor = CalcDirectionLight(normal, toEye);
	float3 pointLightColor = CalcPointLight(input.pos.xy, worldPosition.xyz, normal, toEye);

	output.color = float4(ambient + directionLightColor + pointLightColor, 1.f);

	float4 lightPos[SHADOW_MAP_NUM];
	[unroll]
	for (int i = 0; i < SHADOW_MAP_NUM; i++)
		lightPos[i] = mul(lightData.directionLightViewProj[i], worldPosition);

	[unroll]
	for (int i = 0; i < SHADOW_MAP_NUM; i++)
	{
		float z = lightPos[i].z / lightPos[i].w;

		if (0.f <= z && z <= 1.f)
		{
			float2 shadowMapUV = lightPos[i].xy / lightPos[i].w;
			shadowMapUV = (shadowMapUV + float2(1, -1)) * float2(0.5, -0.5);

			if (0.f <= shadowMapUV.x && shadowMapUV.x <= 1.f &&
				0.f <= shadowMapUV.y && shadowMapUV.y <= 1.f)
			{
				//
				// 使用するシャドウマップが決まったらポアソンディスクサンプリングを行う
				//

				float visibility = 1.f;
				// 一番暗い値
				const float shadowRate = 0.5;
				float delta = (1.f - shadowRate) / (float)POISSON_DISK_SAMPLE_CNT;

				for (int j = 0; j < POISSON_DISK_SAMPLE_CNT; j++)
				{
					// ポアソンディスクサンプルの半径をCPU側から変更できるようにする
					float shadowMapValue = shadowMap[i].Sample(smp, shadowMapUV + POISSON_DISK[j] / 900.f);

					// バイアスCPU側から変更できるようにする
					float bias[3] = {
						0.001f,
						0.003f,
						0.05f,
					};

					if (z >= shadowMapValue + bias[i])
					{
						visibility -= delta;
					}
				}

				output.color.rgb *= visibility;

				/*
				if (i == 0)
					output.color = float4(1, 0, 0, 1);
				if (i == 1)
					output.color = float4(0, 1, 0, 1);
				if (i == 2)
					output.color = float4(0, 0, 1, 1);
					*/
					

				break;
			}
		}
	}

	float y = dot(float3(0.299f, 0.587f, 0.114f), output.color.rgb);
	output.highLuminance = y > 0.99f ? output.color : 0.0f;
	output.highLuminance.a = 1.0;

	// アンビエントオクルージョン
	output.color.rgb *= ambientOcclusionTexture.Sample(smp, input.uv);

	return output;
}
#include"../CameraData.hlsli"

//タイルの大きさ
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

//法線のテクスチャ
Texture2D<float4> normalTexture : register(t0);

//深度のテクスチャ
Texture2D<float> depthTexture : register(t1);

SamplerState smp : register(s0);

//C++側から呼び出したDispatchのxとyの値の逆数（zは常に1）
//それぞれのスレッドで使用するUV座標を求めるのに使用
cbuffer DispatchData : register(b0)
{
	float dispatchX;
	float dispatchY;
}

cbuffer CameraDataConstantBuffer : register(b1)
{
	CameraData cameraData;
}

//結果を格のするテクスチャ
RWTexture2D<float> resultTexture : register(u0);


//[0,1)の乱数を返す
//あとでテクスチャを参照するように変更する
float random(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43578.5453);
}


[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 uv = float2(dispatchThreadID.xy) / (float2(TILE_WIDTH * dispatchX, TILE_HEIGHT * dispatchY));

	float depth = depthTexture.SampleLevel(smp, uv, 0.f);

	if (depth >= 1.f)
	{
		resultTexture[uv] = 1.f;
		return;
	}

	//乱数を取得するためだけ用なのであとで消す
	float depthTextureWidth, depthTextureHeight, depthTextureMiplevels;
	depthTexture.GetDimensions(0, depthTextureWidth, depthTextureHeight, depthTextureMiplevels);
	float dx = 1.f / depthTextureWidth;
	float dy = 1.f / depthTextureHeight;

	float4 posInView = mul(cameraData.projInv, float4(uv, depth, 1.f));
	posInView.xyz = posInView.xyz / posInView.w;

	float occlusion = 0.f;
	float3 norm = normalize((normalTexture.SampleLevel(smp, uv, 0.f).xyz * 2.f) - 1.f);

	const int tryCnt = 256;
	const float radius = 0.5f;
	const float bias = 0.025f;

	for (int i = 0; i < tryCnt; i++)
	{
		//後で変更する
		float rnd1 = random(float2(i * dx, i * dy)) * 2.f - 1.f;
		float rnd2 = random(float2(rnd1, i * dy)) * 2.f - 1.f;
		float rnd3 = random(float2(rnd2, rnd1)) * 2.f - 1.f;

		//中心の点からサンプルする点へのベクトル
		float3 omega = normalize(float3(rnd1, rnd2, rnd3));
		omega = normalize(omega);
		
		//サンプルする点がメッシュの内側にならないようにする
		float dt = dot(norm, omega);
		omega *= sign(dt);

		float4 samplePos = mul(cameraData.proj, float4(posInView.xyz + omega * radius, 1.f));
		samplePos.xyz /= samplePos.w;
		float2 samplePosUV = (samplePos.xy + float2(1.f, -1.f)) * float2(0.5f, -0.5f);

		float sampleDepth = depthTexture.SampleLevel(smp, samplePosUV, 0.f);

		occlusion += step(sampleDepth, samplePos.z + bias);
	}
	
	resultTexture[uv] = 1.f - (occlusion / (float)tryCnt);
}
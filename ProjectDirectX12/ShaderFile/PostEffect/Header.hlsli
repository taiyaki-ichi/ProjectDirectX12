#include"../CameraData.hlsli"
#include"../LightData.hlsli"

//Žg‚Á‚Ä‚È‚¢‚©‚à
cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

//Žg‚Á‚Ä‚È‚¢‚©‚à
cbuffer LightDataConstantBuffer : register(b1)
{
	LightData lightData;
}

cbuffer PostEffectData : register(b2)
{
	float2 depthDiffCenter;
	float depthDiffPower;
	float depthDiffLower;

	float luminanceDegree;
};

Texture2D<float4> mainColorTexture: register(t0);
Texture2D<float4> shrinkedHighLuminanceTexture[8]: register(t1);
Texture2D<float4> shrinkedMainColorTexture[8]: register(t9);
Texture2D<float4> depthBuffer: register(t17);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

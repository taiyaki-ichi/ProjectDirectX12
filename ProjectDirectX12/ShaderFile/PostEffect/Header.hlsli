
cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
	float4 lightColor;
	float4 lightDir;
	float3 eye;
};

cbuffer PostEffectData
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


cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
	float4 lightColor;
	float4 lightDir;
	float3 eye;
};

Texture2D<float4> albedoColorTexture: register(t0);
Texture2D<float4> normalTexture: register(t1);
Texture2D<float4> worldPositionTexture: register(t2);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
};
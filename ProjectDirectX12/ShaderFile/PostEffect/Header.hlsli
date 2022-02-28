
cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
	float4 lightColor;
	float4 lightDir;
	float3 eye;
	float luminanceDegree;
};


Texture2D<float4> mainColorTexture: register(t0);
Texture2D<float4> shrinkHighLuminanceTexture[6]: register(t1);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

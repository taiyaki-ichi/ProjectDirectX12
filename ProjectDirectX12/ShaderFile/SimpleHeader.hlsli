cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
	float3 lightDir;
	float hoge;
	float3 eye;
};

matrix world : register(b1);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float3 ray : RAY;
};
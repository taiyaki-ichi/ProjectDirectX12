cbuffer SceneData : register(b0)
{
	matrix view;
	matrix proj;
	float4 lightColor;
	float4 lightDir;
	float3 eye;
	float luminanceDegree;
};

cbuffer ModelData : register(b1)
{
	matrix world[8];
};


struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
};

struct PSOutput
{
	float4 albedoColor : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 worldPosition : SV_TARGET2;
};
#include"../CameraData.hlsli"
#include"../LightData.hlsli"

cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

//‚¢‚ç‚È‚¢Š›
cbuffer LightDataConstantBuffer : register(b1)
{
	LightData lightData;
}

cbuffer ModelData : register(b2)
{
	matrix world[8];
};

cbuffer LightViewProj : register(b3)
{
	matrix lightViewProj;
}

struct VSOutput
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL;
	float4 worldPosition : POSITION;
};

struct PSOutput
{
	float4 albedoColor : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 worldPosition : SV_TARGET2;
};
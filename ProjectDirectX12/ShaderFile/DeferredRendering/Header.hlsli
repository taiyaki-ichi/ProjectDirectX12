#include"../CameraData.hlsli"
#include"../LightData.hlsli"

cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

cbuffer LightDataConstantBuffer : register(b1)
{
	LightData lightData;
}

Texture2D<float4> albedoColorTexture: register(t0);
Texture2D<float4> normalTexture: register(t1);
Texture2D<float4> worldPositionTexture: register(t2);
Texture2D<float> depthBuffer: register(t3);

Texture2D<float> shadowMap[SHADOW_MAP_NUM] : register(t4);

StructuredBuffer<uint> pointLightIndexBuffer : register(t7);

Texture2D<float> ambientOcclusionTexture: register(t8);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOOD;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
	float4 highLuminance : SV_TARGET1;
};
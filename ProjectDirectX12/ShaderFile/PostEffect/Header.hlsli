#include"../CameraData.hlsli"
#include"../LightData.hlsli"

//縮小された高輝度のテクスチャの数
#define SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM 4

//縮小されたメインカラーのテクスチャの数
#define SHRINK_MAIN_COLOR_TEXTURE_NUM 4


//使ってないかも
cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

//使ってないかも
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
Texture2D<float4> shrinkedHighLuminanceTexture[SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM]: register(t1);
Texture2D<float4> shrinkedMainColorTexture[SHRINK_MAIN_COLOR_TEXTURE_NUM]: register(t5);
Texture2D<float> depthBuffer: register(t9);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

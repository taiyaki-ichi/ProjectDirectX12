#include"../CameraData.hlsli"
#include"../LightData.hlsli"

//�k�����ꂽ���P�x�̃e�N�X�`���̐�
#define SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM 8

//�k�����ꂽ���C���J���[�̃e�N�X�`���̐�
#define SHRINK_MAIN_COLOR_TEXTURE_NUM 8


//�g���ĂȂ�����
cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

//�g���ĂȂ�����
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
Texture2D<float4> shrinkedMainColorTexture[SHRINK_MAIN_COLOR_TEXTURE_NUM]: register(t9);
Texture2D<float4> depthBuffer: register(t17);

SamplerState smp: register(s0);

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

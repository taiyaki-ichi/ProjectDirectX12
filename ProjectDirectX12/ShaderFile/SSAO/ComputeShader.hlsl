#include"../CameraData.hlsli"

//�^�C���̑傫��
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

cbuffer CameraDataConstantBuffer : register(b0)
{
	CameraData cameraData;
}

//C++������Ăяo����Dispatch��x��y�̒l�̋t���iz�͏��1�j
//���ꂼ��̃X���b�h�Ŏg�p����UV���W�����߂�̂Ɏg�p
cbuffer DispatchData : register(b1)
{
	uint dispatchX;
	uint dispatchY;
}

//�[�x�̃e�N�X�`��
Texture2D<float> depthTexture : register(t0);

//�@���̃e�N�X�`��
Texture2D<float4> normalTexture : register(t1);

//���ʂ��i�̂���e�N�X�`��
RWTexture2D<float> resultTexture : register(u0);


SamplerState smp : register(s0);



//[0,1)�̗�����Ԃ�
//���ƂŃe�N�X�`�����Q�Ƃ���悤�ɕύX����
float random(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43578.5453);
}


[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 uv = float2(dispatchThreadID.xy) / (float2(TILE_WIDTH * dispatchX, TILE_HEIGHT * dispatchY));

	float depth = depthTexture.SampleLevel(smp, uv, 0.f);


	// uv�̈ʒu�ɉ����`�ʂ���Ă��Ȃ���
	if (depth >= 1.f)
	{
		resultTexture[dispatchThreadID.xy] = 1.f;
		return;
	}

	//�������擾���邽�߂����p�Ȃ̂ł��Ƃŏ���
	float depthTextureWidth, depthTextureHeight, depthTextureMiplevels;
	depthTexture.GetDimensions(0, depthTextureWidth, depthTextureHeight, depthTextureMiplevels);
	float dx = 1.f / depthTextureWidth;
	float dy = 1.f / depthTextureHeight;

	float4 posInView = mul(cameraData.projInv, float4(uv * float2(2.f, -2.f) + float2(-1.f, 1.f), depth, 1.f));
	posInView.xyz = posInView.xyz / posInView.w;

	float3 norm = normalize((normalTexture.SampleLevel(smp, uv, 0.f).xyz * 2.f) - 1.f);
	// ���s�ړ��͂����Ȃ�
	norm = mul(cameraData.view, float4(norm, 0.f)).xyz;


	const int tryCnt = 256;
	const float radius = 0.5f;
	// �Â��Ȃ肷���Ȃ��悤�Ƀo�C�A�X��������
	const float bias = 0.0025f;

	float occlusion = 0.f;

	for (int i = 0; i < tryCnt; i++)
	{
		//��ŕύX����
		float rnd1 = random(float2(i * dx, i * dy)) * 2.f - 1.f;
		float rnd2 = random(float2(rnd1, i * dy)) * 2.f - 1.f;
		float rnd3 = random(float2(rnd2, rnd1)) * 2.f - 1.f;

		//���S�̓_����T���v������_�ւ̃x�N�g��
		float3 omega = normalize(float3(rnd1, rnd2, rnd3));
		omega = normalize(omega);
		
		//�T���v������_�����b�V���̓����ɂȂ�Ȃ��悤�ɂ���
		float dt = dot(norm, omega);
		omega *= sign(dt);

		float4 samplePos = mul(cameraData.proj, float4(posInView.xyz + omega * radius, 1.f));
		samplePos.xyz /= samplePos.w;
		float2 samplePosUV = (samplePos.xy + float2(1.f, -1.f)) * float2(0.5f, -0.5f);

		float sampleDepth = depthTexture.SampleLevel(smp, samplePosUV, 0.f);

		// ��̓_�ƃT���v������_�̋��������������ꍇ�͎Ղ��Ă���Ɣ��肳���Ȃ��悤�ɂ���p
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(posInView.z - sampleDepth));

		// sampleDepth�̂ق���samplePos.z+bias�ȏ�̎�
		// �܂�, �ʂ̃|���S���ɎՂ��Ă���Ƃ�1�����Z
		occlusion += step(sampleDepth+bias, samplePos.z) * rangeCheck;
	}
	
	resultTexture[dispatchThreadID.xy] = 1.f - (occlusion / (float)tryCnt);
}
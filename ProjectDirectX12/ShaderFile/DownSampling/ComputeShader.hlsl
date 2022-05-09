
//�k������e�N�X�`���̐�
#define RESULT_TEXTURE_NUM 4

//�^�C���̑傫��
#define TILE_WIDTH (1<<(RESULT_TEXTURE_NUM-1))
#define TILE_HEIGHT (1<<(RESULT_TEXTURE_NUM-1))

//�_�E���T���v�����O����錳�̃e�N�X�`��
Texture2D<float4> srcTexture : register(t0);

//���ʂ��i�̂���e�N�X�`��
RWTexture2D<float4> resultTexture[RESULT_TEXTURE_NUM] : register(u0);

SamplerState smp : register(s0);

//C++������Ăяo����Dispatch��x��y�̒l�̋t���iz�͏��1�j
//���ꂼ��̃X���b�h�Ŏg�p����UV���W�����߂�̂Ɏg�p
cbuffer DispatchData : register(b0)
{
	float dispatchX;
	float dispatchY;
}

//���L������
//�^�C���̐F���i�[���郁����
groupshared float4 pixelColor[TILE_WIDTH * TILE_HEIGHT];


void calcPixelColor(uint index, uint parity, uint groupIndex, uint3 dispatchThreadID)
{
	//2^(index+1) x 2^(index+1) �̕��ς��v�Z����
	if ((parity & ((1 << (index + 1)) - 1)) == 0)
	{
		pixelColor[groupIndex] = 0.25f * (pixelColor[groupIndex] + pixelColor[groupIndex + pow(2, index)]
			+ pixelColor[groupIndex + TILE_WIDTH * (index + 1)] + pixelColor[groupIndex + TILE_WIDTH * (index + 1) + pow(2, index)]);

		resultTexture[index][dispatchThreadID.xy >> (index + 1)] = pixelColor[groupIndex];
	}
}

[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{

	uint parity = dispatchThreadID.x | dispatchThreadID.y;

	float2 centerUV = float2(dispatchThreadID.xy) / (float2(TILE_WIDTH * dispatchX, TILE_HEIGHT * dispatchY));
	float4 avgPixel = srcTexture.SampleLevel(smp, centerUV, 0.f);
	pixelColor[groupIndex] = avgPixel;

	//�҂�
	GroupMemoryBarrierWithGroupSync();

	for (uint i = 0; i < RESULT_TEXTURE_NUM; i++)
	{
		calcPixelColor(i, parity, groupIndex, dispatchThreadID);

		//�҂�
		GroupMemoryBarrierWithGroupSync();
	}
}

//�k������e�N�X�`���̐�
#define RESULT_TEXTURE_NUM 4

//�^�C���̑傫��
#define TILE_WIDTH (1<<(RESULT_TEXTURE_NUM-1))
#define TILE_HEIGHT (1<<(RESULT_TEXTURE_NUM-1))

//�_�E���T���v�����O����錳�̃e�N�X�`��
Texture2D<float4> srcTexture : register(t0);

//���ʂ��i�̂���e�N�X�`��
//�Y��������傫���Ȃ邽�ё傫���������ɂȂ�
RWTexture2D<float4> resultTexture[RESULT_TEXTURE_NUM] : register(u0);

SamplerState smp : register(s0);

//C++������Ăяo����Dispatch��x��y�̒l�iz�͏��1�j
//���ꂼ��̃X���b�h�Ŏg�p����UV���W�����߂�̂Ɏg�p
cbuffer DispatchData : register(b0)
{
	float dispatchX;
	float dispatchY;
}

//���L������
//�^�C���̐F���i�[���郁����
groupshared float4 colorStorage[TILE_WIDTH * TILE_HEIGHT];

//colorStorage����1/2�ɏk�����ꂽ�e�N�X�`���̐F���v�Z��resultTexture�Ɋi�[����
void calcShrinkedColor(uint index, uint parity, uint groupIndex, uint3 dispatchThreadID)
{
	if ((parity & ((1 << (index + 1)) - 1)) == 0)
	{
		resultTexture[index][dispatchThreadID.xy >> (index + 1)] = 0.25f * (colorStorage[groupIndex] + colorStorage[groupIndex + pow(2, index)]
			+ colorStorage[groupIndex + TILE_WIDTH * (index + 1)] + colorStorage[groupIndex + TILE_WIDTH * (index + 1) + pow(2, index)]);
	}
}

//�K�E�V�A���u���[��������֐�
//rect�̓e�N�X�`���̑傫��
float4 get5x5GaussianBlur(RWTexture2D<float4> tex, uint2 uv, uint4 rect) {
	float4 ret = tex[uv];

	int dx = 1;
	int dy = 1;

	int l1 = -dx, l2 = -2 * dx;
	int r1 = dx, r2 = 2 * dx;
	int u1 = -dy, u2 = -2 * dy;
	int d1 = dy, d2 = 2 * dy;

	l1 = max(uv.x + l1, rect.x) - uv.x;
	l2 = max(uv.x + l2, rect.x) - uv.x;
	r1 = min(uv.x + r1, rect.z - dx) - uv.x;
	r2 = min(uv.x + r2, rect.z - dx) - uv.x;

	u1 = max(uv.y + u1, rect.y) - uv.y;
	u2 = max(uv.y + u2, rect.y) - uv.y;
	d1 = min(uv.y + d1, rect.w - dy) - uv.y;
	d2 = min(uv.y + d2, rect.w - dy) - uv.y;
	
	
	return float4((
		tex[uv + uint2(l2, u2)].rgb
		+ tex[uv + uint2(l1, u2)].rgb * 4
		+ tex[uv + uint2(0, u2)].rgb * 6
		+ tex[uv + uint2(r1, u2)].rgb * 4
		+ tex[uv + uint2(r2, u2)].rgb

		+ tex[uv + uint2(l2, u1)].rgb * 4
		+ tex[uv + uint2(l1, u1)].rgb * 16
		+ tex[uv + uint2(0, u1)].rgb * 24
		+ tex[uv + uint2(r1, u1)].rgb * 16
		+ tex[uv + uint2(r2, u1)].rgb * 4

		+ tex[uv + uint2(l2, 0)].rgb * 6
		+ tex[uv + uint2(l1, 0)].rgb * 24
		+ ret.rgb * 36
		+ tex[uv + uint2(r1, 0)].rgb * 24
		+ tex[uv + uint2(r2, 0)].rgb * 6

		+ tex[uv + uint2(l2, d1)].rgb * 4
		+ tex[uv + uint2(l1, d1)].rgb * 16
		+ tex[uv + uint2(0, d1)].rgb * 24
		+ tex[uv + uint2(r1, d1)].rgb * 16
		+ tex[uv + uint2(r2, d1)].rgb * 4

		+ tex[uv + uint2(l2, d2)].rgb
		+ tex[uv + uint2(l1, d2)].rgb * 4
		+ tex[uv + uint2(0, d2)].rgb * 6
		+ tex[uv + uint2(r1, d2)].rgb * 4
		+ tex[uv + uint2(r2, d2)].rgb
		) / 256.0f, ret.a);
	
	//3x3ver
	/*
	return float4((
		+ tex[uv + uint2(l1, u1)].rgb
		+ tex[uv + uint2(0, u1)].rgb *2.f
		+ tex[uv + uint2(r1, u1)].rgb 

		+ tex[uv + uint2(l1, 0)].rgb *2.f
		+ ret.rgb*4.f
		+ tex[uv + uint2(r1, 0)].rgb*2.f

		+ tex[uv + uint2(l1, d1)].rgb
		+ tex[uv + uint2(0, d1)].rgb*2.f
		+ tex[uv + uint2(r1, d1)].rgb
		) / 16.f, ret.a);
		*/

}

float4 getTexture5x5GaussianBlur(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect) {
	float4 ret = tex.SampleLevel(smp, uv, 0.f);

	float l1 = -dx, l2 = -2 * dx;
	float r1 = dx, r2 = 2 * dx;
	float u1 = -dy, u2 = -2 * dy;
	float d1 = dy, d2 = 2 * dy;

	l1 = max(uv.x + l1, rect.x) - uv.x;
	l2 = max(uv.x + l2, rect.x) - uv.x;
	r1 = min(uv.x + r1, rect.z - dx) - uv.x;
	r2 = min(uv.x + r2, rect.z - dx) - uv.x;

	u1 = max(uv.y + u1, rect.y) - uv.y;
	u2 = max(uv.y + u2, rect.y) - uv.y;
	d1 = min(uv.y + d1, rect.w - dy) - uv.y;
	d2 = min(uv.y + d2, rect.w - dy) - uv.y;

	return float4((
		tex.SampleLevel(smp, uv + float2(l2, u2), 0.f).rgb
		+ tex.SampleLevel(smp, uv + float2(l1, u2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(0, u2), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(r1, u2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(r2, u2), 0.f).rgb

		+ tex.SampleLevel(smp, uv + float2(l2, u1), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(l1, u1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(0, u1), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r1, u1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(r2, u1), 0.f).rgb * 4

		+ tex.SampleLevel(smp, uv + float2(l2, 0), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(l1, 0), 0.f).rgb * 24
		+ ret.rgb * 36
		+ tex.SampleLevel(smp, uv + float2(r1, 0), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r2, 0), 0.f).rgb * 6

		+ tex.SampleLevel(smp, uv + float2(l2, d1), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(l1, d1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(0, d1), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r1, d1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(r2, d1), 0.f).rgb * 4

		+ tex.SampleLevel(smp, uv + float2(l2, d2), 0.f).rgb
		+ tex.SampleLevel(smp, uv + float2(l1, d2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(0, d2), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(r1, d2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(r2, d2), 0.f).rgb
		) / 256.0f, ret.a);
}

//resultTexture�ɃK�E�V�A���u���[�������Č��ʂ�colorStorage�Ɋi�[����
void calcGaussianBlur(uint index, uint parity, uint groupIndex, uint3 dispatchThreadID,uint srcTextureWidth,uint srcTextureHeight)
{
	if ((parity & ((1 << (index + 1)) - 1)) == 0)
	{
		colorStorage[groupIndex] = get5x5GaussianBlur(resultTexture[index], dispatchThreadID.xy >> (index + 1), uint4(0, 0, srcTextureWidth >> (index + 1), srcTextureHeight >> (index + 1)));

		//���̉����Q�Ƃ�4�Ƃт̍��W�ł�������
		//RWTexture2D�̓������Ƃ�Ă��Ȃ����ۂ�
		//colorStorage[groupIndex] = resultTexture[index][(dispatchThreadID.xy >> (index + 1)) + uint2(1, 0)];
	}
}

//colorStorage�̐F��resultTexture�Ɋi�[����
void storeColor(uint index, uint parity, uint groupIndex, uint3 dispatchThreadID)
{
	if ((parity & ((1 << (index + 1)) - 1)) == 0)
	{
		resultTexture[index][dispatchThreadID.xy >> (index + 1)] = colorStorage[groupIndex];
	}
}


[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint parity = dispatchThreadID.x | dispatchThreadID.y;
	
	uint srcTextureWidth, srcTextureHeight, srcTextureMiplevels;
	srcTexture.GetDimensions(0, srcTextureWidth, srcTextureHeight, srcTextureMiplevels);


	//float2 centerUV = float2(dispatchThreadID.xy) / (float2(TILE_WIDTH * dispatchX, TILE_HEIGHT * dispatchY));
	float2 centerUV = float2(dispatchThreadID.xy) / (float2(srcTextureWidth, srcTextureHeight));

	colorStorage[groupIndex] = getTexture5x5GaussianBlur(srcTexture, smp, centerUV,
		1.f / srcTextureWidth, 1.f / srcTextureHeight, float4(0, 0, 1, 1));

	//colorStorage[groupIndex] = srcTexture.SampleLevel(smp, centerUV, 0.f);

	//�҂�
	GroupMemoryBarrierWithGroupSync();

	[unroll]
	for (uint i = 0; i < RESULT_TEXTURE_NUM;i++)
	{
		//�k�����ꂽ�e�N�X�`�����v�Z
		calcShrinkedColor(i, parity, groupIndex, dispatchThreadID);

		//�҂�
		GroupMemoryBarrierWithGroupSync();

		//�k�����ꂽ�e�N�X�`�����ڂ���
		calcGaussianBlur(i, parity, groupIndex, dispatchThreadID, srcTextureWidth, srcTextureHeight);
		
		//�҂�
		GroupMemoryBarrierWithGroupSync();

		//�k������ڂ����ꂽ�e�N�X�`����resultTexture�Ɋi�[����
		storeColor(i, parity, groupIndex, dispatchThreadID);

		//�҂�
		GroupMemoryBarrierWithGroupSync();
	}
}
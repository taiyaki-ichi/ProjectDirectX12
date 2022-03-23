#include"../CameraData.hlsli"
#include"../LightData.hlsli"


cbuffer CameraData : register(b0)
{
	CameraData cameraData;
}

cbuffer LightData : register(b1)
{
	LightData lightData;
}

//�f�v�X�o�b�t�@
Texture2D<float> depthBuffer : register(t0);

//�o��
//�e�����󂯂郉�C�g�̓Y�������i�[�����
RWStructuredBuffer<uint> pointLightIndexBuffer : register(u0);

SamplerState smp: register(s0);

//���L������
//�^�C���̍ŏ��[�x
groupshared uint minZ;
//�^�C���̍ő�[�x
groupshared uint maxZ;
//�^�C���ɉe������|�C���g���C�g�̃C���f�b�N�X
groupshared uint tileLightIndex[MAX_POINT_LIGHT_NUM];
//�^�C���ɉe������|�C���g���C�g�̐�
groupshared uint tileLightNum;


//�^�C�����Ƃ̎���������߂�
void GetTileFrustumPlane(out float4 frustumPlanes[6], uint3 groupID)
{
	//�^�C���̍ő�ŏ��[�x�𕂓������_�ɕϊ�
	float minTileZ = asfloat(minZ);
	float maxTileZ = asfloat(maxZ);

	float2 tileScale = float2(cameraData.screenWidth, cameraData.screenHeight) * rcp(float2(2 * TILE_WIDTH, 2 * TILE_HEIGHT));
	float2 tileBias = tileScale - float2(groupID.xy);

	float4 c1 = float4(cameraData.proj._11 * tileScale.x, 0.f, tileBias.x, 0.f);
	float4 c2 = float4(0.f, -cameraData.proj._22 * tileScale.y, tileBias.y, 0.f);
	float4 c4 = float4(0.f, 0.f, 1.f, 0.f);

	//�E�̖ʂ̖@��
	frustumPlanes[0] = c4 - c1;
	//���̖ʂ̖@��
	frustumPlanes[1] = c4 + c1;
	//��̖ʂ̖@��
	frustumPlanes[2] = c4 - c2;
	//���̖ʂ̖@��
	frustumPlanes[3] = c4 + c2;
	//���̖ʂ̖@��
	frustumPlanes[4] = float4(0.f, 0.f, 1.f, -minTileZ);//���̖ʂ͌��_��ʂ�Ȃ��̂ő�l������0�ł͂Ȃ�
	//��O�̖ʂ̖@��
	frustumPlanes[5] = float4(0.f, 0.f, -1.f, maxTileZ);//���̖ʂ͌��_��ʂ�Ȃ��̂ő�l������0�ł͂Ȃ�

	//���K������
	[unroll]
	for (uint i = 0; i < 4; i++)
	{
		frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
	}
}


//�J������Ԃł̍��W���v�Z����
float3 ComputePositionInCamera(uint2 globalCoords)
{
	//globalCoords����-1,1�͈̔͂Ɏ��߂����W�𓱏o
	float2 st = ((float2)globalCoords + 0.5) * rcp(float2(cameraData.screenWidth, cameraData.screenHeight));
	st = st * float2(2.f, -2.f) - float2(1.f, -1.f);

	float3 screenPos;
	screenPos.xy = st.xy;
	screenPos.z = depthBuffer.SampleLevel(smp, globalCoords, 0);
	float4 cameraPos = mul(cameraData.projInv, float4(screenPos, 1.f));

	return cameraPos.xyz / cameraPos.w;
}



[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID,uint3 groupThreadID : SV_GroupThreadID )
{
	//�^�C�����ł̃C���f�b�N�X
	uint groupIndex = groupThreadID.y * TILE_WIDTH + groupThreadID.x;

	//���L�������̏�����
	if (groupIndex == 0)
	{
		tileLightNum = 0;
		minZ = 0x7F7FFFFF;//float�̍ő�l
		maxZ = 0;
	}

	uint2 frameUV = dispatchThreadID.xy;
	uint numCellX = (cameraData.screenWidth + TILE_WIDTH - 1) / TILE_WIDTH;
	uint tileIndex = floor(frameUV.x / TILE_WIDTH) + floor(frameUV.y / TILE_HEIGHT) * numCellX;
	uint lightStart = lightData.pointLightNum * tileIndex;

	//���ʂ��i�[����o�b�t�@�̏�����
	//ClearUnorderedAccessViewUint���ăR�}���h�����邯��
	//�A�b�v���[�h�p�̕ʂ̃o�b�t�@������Ȃ���Ύg���Ȃ����ۂ�
	//��Ԃ����肻���Ȃ̂ł����ŏ���������
	for (uint lightIndex = groupIndex; lightIndex < lightData.pointLightNum; lightIndex += TILE_NUM)
	{
		pointLightIndexBuffer[lightStart + lightIndex] = 0xffffffff;
	}

	//�r���[��Ԃł̍��W
	float3 posInView = ComputePositionInCamera(frameUV);

	//���̃O���[�v��҂�
	GroupMemoryBarrierWithGroupSync();


	//�^�C���̍ő�ŏ��[�x�����߂�
	InterlockedMin(minZ, asuint(posInView.z));
	InterlockedMax(maxZ, asuint(posInView.z));

	//���̃O���[�v��҂�
	GroupMemoryBarrierWithGroupSync();


	//�^�C���̐�������߂�
	float4 frustumPlanes[6];
	GetTileFrustumPlane(frustumPlanes, groupID);

	//�^�C���ƃ|�C���g���C�g�̏Փ˔���
	for (uint lightIndex = groupIndex; lightIndex < lightData.pointLightNum; lightIndex += TILE_NUM)
	{
		PointLight pointLight = lightData.pointLight[lightIndex];

		//false�Ȃ�Փ˂��Ă��Ȃ�
		bool isHit = true;
		for (uint i = 0; i < 6; i++)
		{
			float4 lp = float4(pointLight.posInView, 1.f);
			float d = dot(frustumPlanes[i], lp);
			isHit = isHit && (d >= -pointLight.range);
		}
		
		//�^�C���ƐڐG���Ă���ꍇ
		if (isHit)
		{
			uint listIndex = 0;
			//tileLightNum��1�����Z
			//listIndex�ɂ͉��Z�����O�̒l���i�[�����
			InterlockedAdd(tileLightNum, 1, listIndex);
			tileLightIndex[listIndex] = lightIndex;
		}
	}

	//���̃O���[�v��҂�
	GroupMemoryBarrierWithGroupSync();


	//���ʂ��i�[����
	for (uint lightIndex = groupIndex; lightIndex < tileLightNum; lightIndex += TILE_NUM)
	{
		pointLightIndexBuffer[lightStart + lightIndex] = tileLightIndex[lightIndex];
	}

}
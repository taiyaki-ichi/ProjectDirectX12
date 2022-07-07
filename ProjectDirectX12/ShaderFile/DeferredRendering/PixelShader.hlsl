#include"Header.hlsli"

int Alignment(int size, int alignment) {
	return size + alignment - size % alignment;
}


float3 CalcDiffuse(float3 lightDir, float3 lightColor, float3 normal)
{
	float t = saturate(dot(normal, -lightDir));
	return lightColor * t;
}

float3 CalcSpecular(float3 lightDir, float3 lightColor, float3 normal, float3 toEye)
{
	float3 r = reflect(lightDir, normal);
	float t = pow(saturate(dot(r, toEye)), lightData.specPow);
	return lightColor * t;
}

float3 CalcDirectionLight(float3 normal, float3 toEye)
{
	float3 result = float3(0.f, 0.f, 0.f);
	result += CalcDiffuse(normalize(lightData.directionLight.dir), lightData.directionLight.color, normal);
	result += CalcSpecular(normalize(lightData.directionLight.dir), lightData.directionLight.color, normal, toEye);
	return result;
}

float3 CalcPointLight(float2 uv,float3 worldPos,float3 normal,float3 toEye)
{
	int screenWidth = Alignment(cameraData.screenWidth, TILE_WIDTH);

	//�X�N���[�����^�C���ŕ����������̃Z����X���W
	uint numCellX = (screenWidth + TILE_WIDTH - 1) / TILE_WIDTH;

	//�^�C���̃C���f�b�N�X
	uint tileIndex = floor(uv.x / TILE_WIDTH) + floor(uv.y / TILE_HEIGHT) * numCellX;

	//���̃s�N�Z�����܂܂��^�C���̃��C�g�C���f�b�N�X�̃��X�g�̊J�n�ʒu
	uint indexStart = tileIndex * lightData.pointLightNum;

	//���̃s�N�Z�����܂܂�郉�C�g�C���f�b�N�X�̃��X�g�̏I���ʒu
	uint indexEnd = indexStart + lightData.pointLightNum;

	float3 result = float3(0.f, 0.f, 0.f);


	for (uint i = indexStart; i < indexEnd; i++)
	{
		uint pointLightIndex = pointLightIndexBuffer[i];
		if (pointLightIndex == 0xffffffff)
		{
			break;
		}

		float3 lightDir = normalize(worldPos.xyz - lightData.pointLight[pointLightIndex].pos.xyz);
		float distance = length(worldPos.xyz - lightData.pointLight[pointLightIndex].pos.xyz);

		//�e����
		float affect = 1.f - min(1.f, distance / lightData.pointLight[pointLightIndex].range);

		//affect�̒l����`����ύX����
		//��������X�؂����̌v�Z���s��
		result += CalcDiffuse(lightDir, lightData.pointLight[pointLightIndex].color, normal) * affect;
		//result += CalcSpecular(lightDir, lightData.pointLight[pointLightIndex].color, normal, toEye) * affect;
	}
	
	return result;
}


PSOutput main(VSOutput input)
{
	PSOutput output;

	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);

	if (depthBuffer.Sample(smp, input.uv) >= 1.f)
	{
		output.color = albedoColor;
		output.highLuminance = float4(0.f, 0.f, 0.f, 0.f);
		return output;
	}

	float3 normal = normalTexture.Sample(smp, input.uv).xyz;
	//-1,1�͈̔͂Ɏ��߂�
	normal = (normal * 2.f) - 1.f;
	float4 worldPosition = float4(worldPositionTexture.Sample(smp, input.uv).xyz, 1.f);
	float3 toEye = normalize(cameraData.eyePos.xyz - worldPosition.xyz);

	float3 ambient = (albedoColor * 0.6f).xyz;
	float3 directionLightColor = CalcDirectionLight(normal, toEye);
	float3 pointLightColor = CalcPointLight(input.pos.xy, worldPosition.xyz, normal, toEye);

	output.color = float4(ambient + directionLightColor + pointLightColor, 1.f);

	float4 lightPos[SHADOW_MAP_NUM];
	for (int i = 0; i < SHADOW_MAP_NUM; i++)
		lightPos[i] = mul(lightData.directionLightViewProj[i], worldPosition);

	for (int i = 0; i < SHADOW_MAP_NUM; i++)
	{
		float z = lightPos[i].z / lightPos[i].w;

		if (0.f <= z && z <= 1.f)
		{
			float2 shadowMapUV = lightPos[i].xy / lightPos[i].w;
			shadowMapUV = (shadowMapUV + float2(1, -1)) * float2(0.5, -0.5);

			if (0.f <= shadowMapUV.x && shadowMapUV.x <= 1.f &&
				0.f <= shadowMapUV.y && shadowMapUV.y <= 1.f)
			{
				float shadowMapValue = shadowMap[i].Sample(smp, shadowMapUV);

				if (z >= shadowMapValue + 0.001f)
				{
					output.color *= 0.8f;	
				}

				/*
				if (i == 0)
					output.color = float4(1, 0, 0, 1);
				if (i == 1)
					output.color = float4(0, 1, 0, 1);
				if (i == 2)
					output.color = float4(0, 0, 1, 1);
					*/
					

				break;
			}
		}
	}

	float y = dot(float3(0.299f, 0.587f, 0.114f), output.color);
	output.highLuminance = y > 0.99f ? output.color : 0.0f;
	output.highLuminance.a = 1.0;

	return output;
}
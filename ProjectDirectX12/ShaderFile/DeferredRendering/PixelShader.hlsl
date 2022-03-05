#include"Header.hlsli"

PSOutput main(VSOutput input)
{
	PSOutput output;

	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);
	float3 normal = normalTexture.Sample(smp, input.uv);
	float4 worldPosition = float4(worldPositionTexture.Sample(smp, input.uv).xyz, 1.f);


	//–@ü‚ğ0,1‚Ì”ÍˆÍ‚©‚ç-1,1‚Ì”ÍˆÍ‚Éû‚ß‚é
	normal = mul(view,(normal * 2.f) - 1.f);

	float3 diffuse = lightColor * 0.45 * saturate(dot(normal.xyz, normalize(mul(view,lightDir))));
	float3 ambient = albedoColor * 0.5f;

	//Œõ‚ª”½Ë‚·‚é•ûŒü
	float3 refLight = normalize(reflect(lightDir, mul(view, normal.xyz)));
	//–Úü‚Ì•ûŒü
	float3 ray = normalize(eye - worldPosition);
	float3 specular = lightColor * 0.4f * pow(saturate(dot(refLight, ray)), 10.f);

	output.color = float4(ambient + diffuse + specular, 1);


	float4 lightPos[3];
	for (int i = 0; i < 3; i++)
		lightPos[i] = mul(lightViewProj[i], worldPosition);

	for (int i = 0; i < 3; i++)
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

				if (z >= shadowMapValue + 0.0001f)
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
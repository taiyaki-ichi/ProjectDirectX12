#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);
	float3 normal = normalTexture.Sample(smp, input.uv);
	float3 worldPosition = worldPositionTexture.Sample(smp, input.uv);

	//–@ü‚ğ0,1‚Ì”ÍˆÍ‚©‚ç-1,1‚Ì”ÍˆÍ‚Éû‚ß‚é
	normal = (normal * 2.f) - 1.f;

	float3 diffuse = lightColor * 0.4f * saturate(dot(normal.xyz, normalize(lightDir)));
	float3 ambient = albedoColor * 0.4f;

	//Œõ‚ª”½Ë‚·‚é•ûŒü
	float3 refLight = normalize(reflect(lightDir, normal.xyz));
	//–Úü‚Ì•ûŒü
	float3 ray = normalize(eye - worldPosition);
	float3 specular = lightColor * 0.4f * pow(saturate(dot(refLight, ray)), 10.f);

	return float4(ambient + diffuse + specular, 1);
}
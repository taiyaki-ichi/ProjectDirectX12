#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);
	float3 normal = normalTexture.Sample(smp, input.uv);
	float3 worldPosition = worldPositionTexture.Sample(smp, input.uv);

	//法線を0,1の範囲から-1,1の範囲に収める
	normal = (normal * 2.f) - 1.f;

	float3 diffuse = lightColor * 0.4f * saturate(dot(normal.xyz, normalize(lightDir)));
	float3 ambient = albedoColor * 0.4f;

	//光が反射する方向
	float3 refLight = normalize(reflect(lightDir, normal.xyz));
	//目線の方向
	float3 ray = normalize(eye - worldPosition);
	float3 specular = lightColor * 0.4f * pow(saturate(dot(refLight, ray)), 10.f);

	return float4(ambient + diffuse + specular, 1);
}
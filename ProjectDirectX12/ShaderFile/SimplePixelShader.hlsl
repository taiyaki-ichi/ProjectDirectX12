#include"SimpleHeader.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float3 diffuse = float3(0.4f, 0.4f, 0.4f) * saturate(dot(input.normal.xyz, normalize(lightDir)));
	float3 ambient = float3(0.4f, 0.4f, 0.4f);
	float3 refLight = normalize(reflect(lightDir, input.normal.xyz));
	float3 specular = float3(0.2f, 0.2f, 0.2f) * pow(saturate(dot(refLight, input.ray)), 50.f);

	return float4(ambient + diffuse + specular, 1);
}
#include"SimpleHeader.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float3 diffuse = float3(0.3f, 0.3f, 0.3f) * saturate(dot(input.normal.xyz, normalize(lightDir)));
	float3 ambient = float3(0.6f, 0.6f, 0.6f);

	return float4(ambient + diffuse, 1);
}
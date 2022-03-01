#include"Header.hlsli"

PSOutput main(VSOutput input)
{
	PSOutput output;

	float4 albedoColor = albedoColorTexture.Sample(smp,input.uv);
	float3 normal = normalTexture.Sample(smp, input.uv);
	float3 worldPosition = worldPositionTexture.Sample(smp, input.uv);

	//–@ü‚ð0,1‚Ì”ÍˆÍ‚©‚ç-1,1‚Ì”ÍˆÍ‚ÉŽû‚ß‚é
	normal = (normal * 2.f) - 1.f;

	float3 diffuse = lightColor * 0.4f * saturate(dot(normal.xyz, normalize(lightDir)));
	float3 ambient = albedoColor * 0.4f;

	//Œõ‚ª”½ŽË‚·‚é•ûŒü
	float3 refLight = normalize(reflect(lightDir, normal.xyz));
	//–Úü‚Ì•ûŒü
	float3 ray = normalize(eye - worldPosition);
	float3 specular = lightColor * 0.4f * pow(saturate(dot(refLight, ray)), 10.f);

	output.color = float4(ambient + diffuse + specular, 1);

	float y = dot(float3(0.299f, 0.587f, 0.114f), output.color);
	output.highLuminance = y > 0.99f ? output.color : 0.0f;
	output.highLuminance.a = 1.0;

	return output;
}
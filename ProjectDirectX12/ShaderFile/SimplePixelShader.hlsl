#include"SimpleHeader.hlsli"

PSOutput main(VSOutput input)
{
	PSOutput output;
	output.albedoColor = float4(1.f, 1.f, 1.f, 1.f);
	output.normal = input.normal / 2.f + 0.5f;
	output.worldPosition = input.pos;

	return output;
}
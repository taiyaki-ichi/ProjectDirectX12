#include"Header.hlsli"

VSOutput main(float4 pos : POSITION, float4 uv : TEXCOOD)
{
	VSOutput output;
	output.pos = mul(cameraData.viewProj, mul(world, pos));
	output.normal = normalize(mul(world, float4(0.f, 0.f, -1.f, 0.f)));
	output.worldPosition = mul(world, pos);
	return output;
}
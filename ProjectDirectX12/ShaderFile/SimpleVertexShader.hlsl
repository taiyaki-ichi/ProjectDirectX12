#include"SimpleHeader.hlsli"

VSOutput main(float4 pos : POSITION, float4 normal : NORMAL) 
{
	VSOutput output;
	output.pos = mul(mul(proj, view), mul(world, pos));
	normal.w = 0.f;
	output.normal = mul(world, normal);
	return output;
}
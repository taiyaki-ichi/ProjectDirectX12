#include"Header.hlsli"

VSOutput main(float4 pos : POSITION, float4 normal : NORMAL, uint instanceID : SV_InstanceId)
{
	VSOutput output;
	output.pos = mul(mul(proj, view), mul(world[instanceID], pos));
	normal.w = 0.f;
	output.normal = normalize(mul(world[instanceID], normal));
	return output;
}
#include"Header.hlsli"

float4 main(float4 pos : POSITION, float4 normal : NORMAL, uint instanceID : SV_InstanceId) : SV_POSITION
{
	return mul(lightViewProj, mul(world[instanceID], pos));
}
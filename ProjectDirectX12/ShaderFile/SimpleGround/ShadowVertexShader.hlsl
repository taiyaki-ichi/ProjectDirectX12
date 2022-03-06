#include"Header.hlsli"

float4 main(float4 pos : POSITION, float4 uv : TEXCOOD) : SV_POSITION
{
	return mul(lightViewProj, mul(world, pos));
}
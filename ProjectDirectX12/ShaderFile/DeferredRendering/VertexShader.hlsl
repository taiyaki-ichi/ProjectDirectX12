#include"Header.hlsli"

VSOutput main(float4 pos : POSITION ,float2 uv : TEXCOOD)
{
	VSOutput output;
	output.pos = pos;
	output.uv = uv;
	return output;
}
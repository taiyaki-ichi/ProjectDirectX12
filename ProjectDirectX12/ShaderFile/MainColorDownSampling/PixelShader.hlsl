#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	return mainColorTexture.Sample(smp,input.uv);
}
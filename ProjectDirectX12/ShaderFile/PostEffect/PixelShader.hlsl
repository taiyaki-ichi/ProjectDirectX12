#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 highLuminanceSum = float4(0.f, 0.f, 0.f, 0.f);
	for (int i = 0; i < 6; i++)
	{
		highLuminanceSum += shrinkHighLuminanceTexture[i].Sample(smp, input.uv);
	}
	float4 luminance = highLuminanceSum / 6.f;
	luminance *= luminanceDegree;

	return mainColorTexture.Sample(smp, input.uv) + saturate(luminance);
}
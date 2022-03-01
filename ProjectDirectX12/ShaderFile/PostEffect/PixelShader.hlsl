#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 highLuminanceSum = float4(0.f, 0.f, 0.f, 0.f);
	for (int i = 0; i < 8; i++)
	{
		highLuminanceSum += shrinkedHighLuminanceTexture[i].Sample(smp, input.uv);
	}
	float4 luminance = highLuminanceSum / 8.f;
	luminance *= luminanceDegree;


	//深度の差
	float depthDiff = abs(depthBuffer.Sample(smp, depthDiffCenter) - depthBuffer.Sample(smp, input.uv));
	depthDiff = pow(depthDiff, depthDiffPower);

	//深度の差が0.25以下の場合
	//通常の画像でサンプリング
	float4 mainColor = float4(0.f, 0.f, 0.f, 0.f);
	if (depthDiff < depthDiffLower)
	{
		mainColor = mainColorTexture.Sample(smp, input.uv);
	}
	else
	{
		//0.25-1.f間を8等分
		float delta = (1.f - depthDiffLower) / 8.f;

		float degree = (depthDiff - depthDiffLower) / delta;
		int n = trunc(degree);
		float rate = frac(degree);

		mainColor = lerp(mainColorTexture.Sample(smp, input.uv), shrinkedMainColorTexture[0].Sample(smp, input.uv), rate);
		for (int i = 0; i < 8; i++)
		{
			if (i == n)
			{
				mainColor = lerp(shrinkedMainColorTexture[i - 1].Sample(smp, input.uv), shrinkedMainColorTexture[i].Sample(smp, input.uv), rate);
				break;
			}
		}
	}

	return mainColor + saturate(luminance);
}
#include"Header.hlsli"

float4 main(VSOutput input) : SV_TARGET
{
	float4 highLuminanceSum = float4(0.f, 0.f, 0.f, 0.f);
	for (int i = 0; i < SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM; i++)
	{
		highLuminanceSum += shrinkedHighLuminanceTexture[i].Sample(smp, input.uv);
	}
	float4 luminance = highLuminanceSum / (float)SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM;
	luminance *= luminanceDegree;


	//�[�x�̍�
	float depthDiff = abs(depthBuffer.Sample(smp, depthDiffCenter) - depthBuffer.Sample(smp, input.uv));
	depthDiff = pow(depthDiff, depthDiffPower);

	//�[�x�̍���0.25�ȉ��̏ꍇ
	//�ʏ�̉摜�ŃT���v�����O
	float4 mainColor = float4(0.f, 0.f, 0.f, 0.f);
	if (depthDiff < depthDiffLower)
	{
		mainColor = mainColorTexture.Sample(smp, input.uv);
	}
	else
	{
		//0.25-1.f�Ԃ�8����
		float delta = (1.f - depthDiffLower) / (float)SHRINK_MAIN_COLOR_TEXTURE_NUM;

		float degree = (depthDiff - depthDiffLower) / delta;
		int n = trunc(degree);
		float rate = frac(degree);

		mainColor = lerp(mainColorTexture.Sample(smp, input.uv), shrinkedMainColorTexture[0].Sample(smp, input.uv), rate);
		for (int i = 0; i < SHRINK_MAIN_COLOR_TEXTURE_NUM; i++)
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
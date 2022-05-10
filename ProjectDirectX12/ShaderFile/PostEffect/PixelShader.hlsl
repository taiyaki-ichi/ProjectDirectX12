#include"Header.hlsli"

float4 getTexture5x5GaussianBlur(Texture2D<float4> tex, SamplerState smp, float2 uv, float dx, float dy, float4 rect) {
	float4 ret = tex.SampleLevel(smp, uv, 0.f);

	float l1 = -dx, l2 = -2 * dx;
	float r1 = dx, r2 = 2 * dx;
	float u1 = -dy, u2 = -2 * dy;
	float d1 = dy, d2 = 2 * dy;

	l1 = max(uv.x + l1, rect.x) - uv.x;
	l2 = max(uv.x + l2, rect.x) - uv.x;
	r1 = min(uv.x + r1, rect.z - dx) - uv.x;
	r2 = min(uv.x + r2, rect.z - dx) - uv.x;

	u1 = max(uv.y + u1, rect.y) - uv.y;
	u2 = max(uv.y + u2, rect.y) - uv.y;
	d1 = min(uv.y + d1, rect.w - dy) - uv.y;
	d2 = min(uv.y + d2, rect.w - dy) - uv.y;

	return float4((
		tex.SampleLevel(smp, uv + float2(l2, u2), 0.f).rgb
		+ tex.SampleLevel(smp, uv + float2(l1, u2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(0, u2), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(r1, u2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(r2, u2), 0.f).rgb

		+ tex.SampleLevel(smp, uv + float2(l2, u1), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(l1, u1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(0, u1), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r1, u1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(r2, u1), 0.f).rgb * 4

		+ tex.SampleLevel(smp, uv + float2(l2, 0), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(l1, 0), 0.f).rgb * 24
		+ ret.rgb * 36
		+ tex.SampleLevel(smp, uv + float2(r1, 0), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r2, 0), 0.f).rgb * 6

		+ tex.SampleLevel(smp, uv + float2(l2, d1), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(l1, d1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(0, d1), 0.f).rgb * 24
		+ tex.SampleLevel(smp, uv + float2(r1, d1), 0.f).rgb * 16
		+ tex.SampleLevel(smp, uv + float2(r2, d1), 0.f).rgb * 4

		+ tex.SampleLevel(smp, uv + float2(l2, d2), 0.f).rgb
		+ tex.SampleLevel(smp, uv + float2(l1, d2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(0, d2), 0.f).rgb * 6
		+ tex.SampleLevel(smp, uv + float2(r1, d2), 0.f).rgb * 4
		+ tex.SampleLevel(smp, uv + float2(r2, d2), 0.f).rgb
		) / 256.0f, ret.a);
}

float4 main(VSOutput input) : SV_TARGET
{
	float4 highLuminanceSum = float4(0.f, 0.f, 0.f, 0.f);
	for (int i = 0; i < SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM; i++)
	{
		float width, height, miplevels;
		shrinkedHighLuminanceTexture[i].GetDimensions(0, width, height, miplevels);
		highLuminanceSum += getTexture5x5GaussianBlur(shrinkedHighLuminanceTexture[i], smp, input.uv, 1.f / width, 1.f / height, uint4(0, 0, 1, 1));
	}
	float4 luminance = highLuminanceSum / (float)SHRINK_HIGHT_LUMINANCE_TEXTURE_NUM;
	luminance *= luminanceDegree;


	//深度の差
	float depthDiff = abs(depthBuffer.Sample(smp, depthDiffCenter) - depthBuffer.Sample(smp, input.uv));
	//差が開きやすいようにバイアスをかける
	depthDiff = pow(depthDiff, depthDiffPower);

	//範囲を等分
	float delta = (1.f - depthDiffLower) / ((float)SHRINK_MAIN_COLOR_TEXTURE_NUM + 1.f);

	//何番目の区間か導出
	float degree = (depthDiff - depthDiffLower) / delta;

	//使用するテクスチャの番号を決める際に使用する
	int n = trunc(degree);
	//線形補完する際に使用する
	float rate = frac(degree);
	
	float4 mainColor = float4(0.f, 0.f, 0.f, 0.f);
	//基準値以下の場合通常のメイカラーを使用する
	//バイアスかける前の値でひょかした方がイイかも
	if (depthDiff < depthDiffLower)
	{
		mainColor = mainColorTexture.Sample(smp, input.uv);
	}
	else if (n == 0)
	{
		//通常のメインカラーと0番目の縮小されたメインカラーを線形補完する

		float4 color1 = mainColorTexture.Sample(smp, input.uv);

		float width2, height2, miplevels2;
		shrinkedMainColorTexture[0].GetDimensions(0, width2, height2, miplevels2);
		float4 color2 = getTexture5x5GaussianBlur(shrinkedMainColorTexture[0], smp, input.uv, 1.f / width2, 1.f / height2, uint4(0, 0, 1, 1));

		mainColor = lerp(color1, color2, rate);
	}
	else
	{
		//n番目の縮小されたメインカラーとn+1番目に縮小されたメインカラーを線形補完する

		[unroll]
		//for(;i < SHRINK_MAIN_COLOR_TEXTURE_NUM; i++)とは記述できない
		for (int i = 0; i < SHRINK_MAIN_COLOR_TEXTURE_NUM; i++)
		{
			if (i == n - 1)
			{
				float width1, height1, miplevels1;
				shrinkedMainColorTexture[i].GetDimensions(0, width1, height1, miplevels1);
				float4 color1 = getTexture5x5GaussianBlur(shrinkedMainColorTexture[i], smp, input.uv, 1.f / width1, 1.f / height1, uint4(0, 0, 1, 1));

				float width2, height2, miplevels2;
				shrinkedMainColorTexture[i + 1].GetDimensions(0, width2, height2, miplevels2);
				float4 color2 = getTexture5x5GaussianBlur(shrinkedMainColorTexture[i + 1], smp, input.uv, 1.f / width2, 1.f / height2, uint4(0, 0, 1, 1));

				mainColor = lerp(color1, color2, rate);

				break;
			}
		}
	}

	//被写界深度で使用する深度の差の分布を確認する用
	/*
	if (n == 0)
	{
		mainColor = float4(1, 0, 0, 1);
	}
	else if (n == 1)
	{
		mainColor = float4(0, 1, 0, 1);
	}
	else if (n == 2)
	{
		mainColor = float4(0, 0, 1, 1);
	}
	else if (n == 3)
	{
		mainColor = float4(1, 1, 0, 1);
	}
	*/

	return mainColor + saturate(luminance);
}
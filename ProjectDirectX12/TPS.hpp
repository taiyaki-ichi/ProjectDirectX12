#pragma once
#include<DirectXMath.h>
#include<utility>
#include<cmath>

namespace pdx12
{
	//
	// 三人称視点を扱うためのクラスと関数
	// とりあえず作成しただけなので視点移動についてイイ感じに変更したい
	//

	struct TPS
	{
		DirectX::XMFLOAT3 target{};
		float eyeRadius{};
		float eyeHeight{};
		float eyeRotation{};
	};

	// 前のフレームからの変化量をもとに三人称視点を更新する
	// x, y, rotはそれぞれx座標, y座標, 回転の変化量
	inline void UpdateTPS(TPS& tps, float x, float y, float rot)
	{
		float moveX = x * std::cos(tps.eyeRotation) - y * std::sin(tps.eyeRotation);
		float moveY = x * std::sin(tps.eyeRotation) + y * std::cos(tps.eyeRotation);
		tps.target.x += moveX;
		tps.target.z += moveY;
		tps.eyeRotation += rot;
	}

	// カメラがある位置を取得する
	inline DirectX::XMFLOAT3 GetEyePosition(TPS const& tps)
	{
		float x = tps.eyeRadius * std::cos(tps.eyeRotation) + tps.target.x;
		float z = tps.eyeRadius * std::sin(tps.eyeRotation) + tps.target.z;
		float y = tps.eyeHeight + tps.target.y;
		return { x,y,z };
	}
}
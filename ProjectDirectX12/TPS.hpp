#pragma once
#include<DirectXMath.h>
#include<utility>
#include<cmath>

namespace pdx12
{
	struct TPS
	{
		DirectX::XMFLOAT3 target{};
		float eyeRadius{};
		float eyeHeight{};
		float eyeRotation{};
	};

	inline void UpdateTPS(TPS& tps, float x, float y, float rot)
	{
		float moveX = x * std::cos(tps.eyeRotation) - y * std::sin(tps.eyeRotation);
		float moveY = x * std::sin(tps.eyeRotation) + y * std::cos(tps.eyeRotation);
		tps.target.x += moveX;
		tps.target.z += moveY;
		tps.eyeRotation += rot;
	}

	inline DirectX::XMFLOAT3 GetEye(TPS const& tps)
	{
		float x = tps.eyeRadius * std::cos(tps.eyeRotation) + tps.target.x;
		float z = tps.eyeRadius * std::sin(tps.eyeRotation) + tps.target.z;
		float y = tps.eyeHeight + tps.target.y;
		return { x,y,z };
	}
}
#pragma once
#include<stdexcept>
#include<sstream>
#include<memory>
#include<utility>
#include<DirectXMath.h>
#include<algorithm>
#include<array>
#include<d3d12.h>
#include<dxgi1_4.h>

#include<iostream>

namespace pdx12
{
	using namespace DirectX;

	// 視錐台の8頂点の導出
	void get_frustum_vertex(XMFLOAT3 const& eye, float asspect, float nearZ, float farZ, float viewAngle, XMFLOAT3 const& cameraForward, XMFLOAT3 const& cameraRight, std::array<XMFLOAT3, 8>& result);

	// 行列をかけるだけでも数行のコードになってしまうため関数を作成した
	// たくさんの行列をかける時には最適化できなそうなのであんまりよくないかも
	void apply(XMFLOAT3& float3, XMMATRIX const& matrix);

	// 8頂点を[-1,1]x[-1,1]の範囲に収めるようなクロップ行列を求める
	void get_clop_matrix(std::array<XMFLOAT3, 8> const& float3, XMMATRIX& matrix);

}
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

	void throw_exception(char const* fileName, int line, char const* func, char const* str);

#define THROW_PDX12_EXCEPTION(s)	throw_exception(__FILE__,__LINE__,__func__,s);



	template<typename T>
	struct release_deleter {
		void operator()(T* ptr) {
			ptr->Release();
		}
	};

	template<typename T>
	using release_unique_ptr = std::unique_ptr<T, release_deleter<T>>;


	//戻り値はsrc、dstの順
	std::pair<D3D12_TEXTURE_COPY_LOCATION, D3D12_TEXTURE_COPY_LOCATION> get_texture_copy_location(ID3D12Device* device, ID3D12Resource* srcResource, ID3D12Resource* dstResource);

	template<typename T>
	inline constexpr T alignment(T size, T alignment) {
		if (size % alignment == 0)
			return size;
		else
			return size + alignment - size % alignment;
	}


	//視錐台の8頂点の導出
	void get_frustum_vertex(XMFLOAT3 const& eye, float asspect, float nearZ, float farZ, float viewAngle, XMFLOAT3 const& cameraForward, XMFLOAT3 const& cameraRight, std::array<XMFLOAT3, 8>& result);

	//行列をかける
	void apply(XMFLOAT3& float3, XMMATRIX const& matrix);

	//クロップ行列を求める
	void get_clop_matrix(std::array<XMFLOAT3, 8> const& float3, XMMATRIX& matrix);

}
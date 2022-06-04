#pragma once
#include<d3d12.h>
#include"utility.hpp"
#include<vector>
#include<array>


namespace pdx12
{
	struct static_sampler
	{
		D3D12_FILTER filter;
		D3D12_TEXTURE_ADDRESS_MODE address_u;
		D3D12_TEXTURE_ADDRESS_MODE address_v;
		D3D12_TEXTURE_ADDRESS_MODE address_w;
		D3D12_COMPARISON_FUNC comparison_func;


		UINT max_anisotropy = 16;
		float min_LOD = 0.f;
		float max_LOD = D3D12_FLOAT32_MAX;
		float mip_LOD_bias = 0;
	};

	struct descriptor_range_type
	{
		D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType;
		UINT num = 1;
	};
	
	release_unique_ptr<ID3D12RootSignature> create_root_signature(ID3D12Device* device,
		std::vector<std::vector<descriptor_range_type>> const& descriptorRangeTypes, const std::vector<static_sampler>& staticSamplers);
}
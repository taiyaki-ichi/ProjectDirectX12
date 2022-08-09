#pragma once
#include<d3d12.h>
#include"utility.hpp"
#include<vector>
#include<array>


namespace pdx12
{
	// ルートシグネチャの初期化用ぐらいしか使わない
	// D3D12_STATIC_SAMPLER_DESCとあんま変らないからなくしてもイイかも
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

	// 連続した同じタイプのディスクリプタレンジを表現する用
	struct descriptor_range_type
	{
		D3D12_DESCRIPTOR_RANGE_TYPE descriptorRangeType;
		UINT num = 1;
	};
	
	// ディスクリプタレンジとスタティックサンプラってコンパイル時にたいてい決まってそう
	// vector使うのはどうなんだろ
	// ディスクリプタレンジのレジスタの番号は順に自動で割当てられる
	release_unique_ptr<ID3D12RootSignature> create_root_signature(ID3D12Device* device,
		std::vector<std::vector<descriptor_range_type>> const& descriptorRangeTypes, std::vector<static_sampler> const& staticSamplers);
}
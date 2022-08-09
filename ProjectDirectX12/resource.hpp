#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"

namespace pdx12
{
	// 新しい型を定義するほどでもないと思ったのでエイリアスですます
	// 少しみにくいかも
	using resource_and_state = std::pair<release_unique_ptr<ID3D12Resource>, D3D12_RESOURCE_STATES>;

	// 
	// create_commited_関数ではメモリ確保とリソースの作成を行っているので遅い
	// CreateReservedResource, CreatePlacedResourceとかC++でいうところのリプレイスメントnewみたいなことができるらしい
	//

	// CPUからの書き込みが可能なバッファ
	// 頂点バッファや定数バッファを作成する際に使用
	resource_and_state create_commited_upload_buffer_resource(ID3D12Device* device, UINT64 size);

	// GPUからのみ読み書きが可能
	// テクスチャとして扱うことで最適化をはかれる, みたいな感じだと思う
	resource_and_state create_commited_texture_resource(ID3D12Device* device,
		DXGI_FORMAT format, UINT64 width, UINT height, std::size_t dimension, UINT16 depthOrArraySize, UINT16 mipLevels, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE const* clearValue = nullptr);

	// この関数使いどころないかも
	// create_commited_texture_resourceで作成したリソースの方がアクセス速いと思う, たぶん
	resource_and_state create_commited_buffer_resource(ID3D12Device* device, UINT64 size, D3D12_RESOURCE_FLAGS flags);


	// リソースバリアを作成しresource.secondを更新
	void resource_barrior(ID3D12GraphicsCommandList* list, resource_and_state& resource, D3D12_RESOURCE_STATES afterState);
}
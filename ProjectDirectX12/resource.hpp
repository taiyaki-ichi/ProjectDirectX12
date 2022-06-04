#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"

namespace pdx12
{
	using resource_and_state = std::pair<release_unique_ptr<ID3D12Resource>, D3D12_RESOURCE_STATES>;

	resource_and_state create_commited_upload_buffer_resource(ID3D12Device* device, UINT64 size);

	resource_and_state create_commited_texture_resource(ID3D12Device* device,
		DXGI_FORMAT format, UINT64 width, UINT64 height, std::size_t dimension, UINT16 depthOrArraySize, UINT16 mipLevels, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE const* clearValue = nullptr);

	resource_and_state create_commited_buffer_resource(ID3D12Device* device, UINT64 size, D3D12_RESOURCE_FLAGS flags);

	//リソースバリアを作成しresource.secondを更新
	void resource_barrior(ID3D12GraphicsCommandList* list, resource_and_state& resource, D3D12_RESOURCE_STATES afterState);
}
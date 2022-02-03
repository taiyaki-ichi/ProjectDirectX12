#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{

	inline release_unique_ptr<ID3D12Resource> create_commited_upload_buffer_resource(ID3D12Device* device, UINT64 size)
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = size;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	
		ID3D12Resource* tmp = nullptr;
		if (FAILED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,//D3D12_HEAP_TYPE_UPLOADÇÃèÍçáÇÕD3D12_RESOURCE_STATE_GENERIC_READ
			nullptr,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_PDX12_EXCEPTION("failed CreateCommittedResource");
		}

		return release_unique_ptr<ID3D12Resource>{tmp};
	}


	inline release_unique_ptr<ID3D12Resource> create_commited_texture_resource(ID3D12Device* device,
		DXGI_FORMAT format, UINT64 width, UINT64 height, std::size_t dimension, UINT16 depthOrArraySize, UINT16 mipLevels,D3D12_RESOURCE_FLAGS flags)
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = depthOrArraySize;
		resourceDesc.MipLevels = mipLevels;
		resourceDesc.Format = format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = flags;

		switch (dimension)
		{
		case 1:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			break;
		case 2:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			break;
		case 3:
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			break;
		default:
			break;
		}

		ID3D12Resource* tmp = nullptr;
		if (FAILED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&tmp))))
		{
			THROW_PDX12_EXCEPTION("failed CreateCommittedResource");
		}

		return release_unique_ptr<ID3D12Resource>{tmp};
	}

}
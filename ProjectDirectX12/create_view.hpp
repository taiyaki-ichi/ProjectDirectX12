#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>

#pragma comment(lib,"d3d12.lib")

namespace pdx12
{

	inline void create_CBV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, UINT sizeInBytes)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.BufferLocation = resource->GetGPUVirtualAddress();
		desc.SizeInBytes = sizeInBytes;

		device->CreateConstantBufferView(&desc, cpuHandle);
	}


	inline void create_texture2D_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT mipLevels, UINT mostDetailedMip, UINT planeSline, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = mipLevels;
		desc.Texture2D.MostDetailedMip = mostDetailedMip;
		desc.Texture2D.PlaneSlice = planeSline;
		desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;

		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}

	inline void create_texture2D_array_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT arraySize, UINT firstArraySlice, UINT mipLevels, UINT mostDetailedMip, UINT planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipLevels = mipLevels;
		desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;

		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}


	inline void create_texture_cube_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT arraySize, UINT firstArraySlice, UINT mipLevels, UINT mostDetailedMip, UINT planeSlice, float resourceMinLODClamp)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = format;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipLevels = mipLevels;
		desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
		desc.Texture2DArray.PlaneSlice = planeSlice;
		desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;

		device->CreateShaderResourceView(resource, &desc, cpuHandle);
	}


	inline void create_texture2D_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		ID3D12Resource* counterResource, UINT mipSlice, UINT planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;

		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}

	inline void create_texture1D_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		ID3D12Resource* counterResource, UINT mipSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = mipSlice;

		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}

	inline void create_buffer_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, 
		ID3D12Resource* counterResource,UINT numElements,UINT structureByteStride,UINT firstElement,UINT counterOffsetInBytes,D3D12_BUFFER_UAV_FLAGS flags)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		//unknownŒÅ’è‚Á‚Û‚¢
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = numElements;
		desc.Buffer.StructureByteStride = structureByteStride;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.CounterOffsetInBytes = counterOffsetInBytes;
		desc.Buffer.Flags = flags;

		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}


	inline void create_texture2D_array_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		ID3D12Resource* counterResource, UINT arraySize, UINT firstArraySlice, UINT mipSlice, UINT planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		desc.Texture2DArray.PlaneSlice = planeSlice;

		device->CreateUnorderedAccessView(resource, counterResource, &desc, cpuHandle);
	}


	inline void create_texture2D_DSV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;

		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}


	inline void create_texture2D_array_DSV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT arraySize, UINT firstArraySlice, UINT mipSlice)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;

		device->CreateDepthStencilView(resource, &desc, cpuHandle);
	}


	inline void create_texture2D_RTV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT mipSlice, UINT planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mipSlice;
		desc.Texture2D.PlaneSlice = planeSlice;

		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}

	inline void create_texture2D_array_RTV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,DXGI_FORMAT format, 
		UINT arraySize, UINT firstArraySlice, UINT mipSlice, UINT planeSlice)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.ArraySize = arraySize;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice;
		desc.Texture2DArray.MipSlice = mipSlice;
		desc.Texture2DArray.PlaneSlice = planeSlice;

		device->CreateRenderTargetView(resource, &desc, cpuHandle);
	}
}
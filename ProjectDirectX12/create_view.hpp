#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>

namespace pdx12
{
	//
	// d3d12.hのビューを作成する関数では_VIEW_DESCの構造体を設定しなければいけない
	// いちいちめんどくさいのでそれぞれに対応した関数を定義した
	// たぶん全部網羅できていない
	//


	// sizeInBytesは256にアライメントしてないとエラー出るので注意
	void create_CBV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, UINT sizeInBytes);


	void create_texture2D_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT mipLevels, UINT mostDetailedMip, UINT planeSline, float resourceMinLODClamp);

	void create_texture2D_array_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT arraySize, UINT firstArraySlice, UINT mipLevels, UINT mostDetailedMip, UINT planeSlice, float resourceMinLODClamp);


	void create_texture_cube_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT arraySize, UINT firstArraySlice, UINT mipLevels, UINT mostDetailedMip, UINT planeSlice, float resourceMinLODClamp);

	void create_buffer_SRV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,
		UINT numElements, UINT structureByteStride, UINT firstElement, D3D12_BUFFER_SRV_FLAGS flags);

	void create_texture2D_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		ID3D12Resource* counterResource, UINT mipSlice, UINT planeSlice);

	void create_texture1D_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		ID3D12Resource* counterResource, UINT mipSlice);

	void create_buffer_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource,
		ID3D12Resource* counterResource, UINT numElements, UINT structureByteStride, UINT firstElement, UINT counterOffsetInBytes, D3D12_BUFFER_UAV_FLAGS flags);

	void create_texture2D_array_UAV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		ID3D12Resource* counterResource, UINT arraySize, UINT firstArraySlice, UINT mipSlice, UINT planeSlice);

	void create_texture2D_DSV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT mipSlice);


	void create_texture2D_array_DSV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT arraySize, UINT firstArraySlice, UINT mipSlice);

	void create_texture2D_RTV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT mipSlice, UINT planeSlice);

	void create_texture2D_array_RTV(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, ID3D12Resource* resource, DXGI_FORMAT format,
		UINT arraySize, UINT firstArraySlice, UINT mipSlice, UINT planeSlice);
}
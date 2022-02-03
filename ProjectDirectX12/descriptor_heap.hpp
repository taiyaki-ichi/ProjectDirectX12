#pragma once
#include<d3d12.h>
#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")

namespace pdx12
{

	class descriptor_heap
	{
		release_unique_ptr<ID3D12DescriptorHeap> ptr{};

		std::uint32_t size = 0;
		std::uint32_t increment_size = 0;

	public:
		void initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size);

		D3D12_GPU_DESCRIPTOR_HANDLE get_GPU_handle(std::size_t index = 0);
		D3D12_CPU_DESCRIPTOR_HANDLE get_CPU_handle(std::size_t index = 0);

		ID3D12DescriptorHeap* get();
	};

	//
	//
	//

	inline void pdx12::descriptor_heap::initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = size;
		desc.Type = type;

		increment_size = device->GetDescriptorHandleIncrementSize(type);

		ID3D12DescriptorHeap* tmp = nullptr;
		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&tmp))))
			THROW_PDX12_EXCEPTION("");
		ptr.reset(tmp);

		this->size = size;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE pdx12::descriptor_heap::get_GPU_handle(std::size_t index)
	{
		auto gpuHandle = ptr->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += static_cast<UINT64>(increment_size) * static_cast<UINT64>(index);
		return gpuHandle;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE pdx12::descriptor_heap::get_CPU_handle(std::size_t index)
	{
		auto cpuHandle = ptr->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += static_cast<SIZE_T>(increment_size) * static_cast<SIZE_T>(index);
		return cpuHandle;
	}

	inline ID3D12DescriptorHeap* pdx12::descriptor_heap::get()
	{
		return ptr.get();
	}


}
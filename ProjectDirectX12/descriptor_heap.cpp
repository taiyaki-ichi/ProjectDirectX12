#include"descriptor_heap.hpp"

#pragma comment(lib,"d3d12.lib")

namespace pdx12
{
	void pdx12::descriptor_heap::initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size)
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

	D3D12_GPU_DESCRIPTOR_HANDLE pdx12::descriptor_heap::get_GPU_handle(std::size_t index)
	{
		auto gpuHandle = ptr->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += static_cast<UINT64>(increment_size) * static_cast<UINT64>(index);
		return gpuHandle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE pdx12::descriptor_heap::get_CPU_handle(std::size_t index)
	{
		auto cpuHandle = ptr->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += static_cast<SIZE_T>(increment_size) * static_cast<SIZE_T>(index);
		return cpuHandle;
	}

	ID3D12DescriptorHeap* pdx12::descriptor_heap::get()
	{
		return ptr.get();
	}
}
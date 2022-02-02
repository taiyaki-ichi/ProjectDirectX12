#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include"utility.hpp"
#include<array>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{

	template<std::size_t AllocatorNum>
	class command_manager
	{
		release_unique_ptr<ID3D12CommandQueue> queue{};

		release_unique_ptr<ID3D12GraphicsCommandList> list{};

		std::array<release_unique_ptr<ID3D12CommandAllocator>, AllocatorNum> allocators{};

		std::array<release_unique_ptr<ID3D12Fence>, AllocatorNum> fences{};

		std::array<std::uint64_t, AllocatorNum> fence_values{};

		//WaitForSingleObject���Ăяo�����߂ɕK�v
		//TODO: �����o�ŕێ�����K�v������̂����ׂ�
		HANDLE fence_event_handle = nullptr;

		//���݂�list���g�p���Ă���allocaotr�̃C���f�b�N�X
		std::size_t current_allocaotr_index = 0;

	public:
		//������
		void initialize(ID3D12Device* device);

		//�t�F���X�𗧂Ă�
		void signal();

		//���Ă��t�F���X��҂�
		void wait(std::size_t index);

		//index�Ԗڂ�Allocator�Ń��X�g�����Z�b�g
		void reset_list(std::size_t index);


		void excute();

		//excute�����邩��dispatch�������ɍ���Ă���
		void dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ);


		ID3D12GraphicsCommandList* get_list();

		ID3D12CommandQueue* get_queue();
	};


	//
	//
	//

	template<std::size_t AllocatorNum>
	inline void command_manager<AllocatorNum>::initialize(ID3D12Device* device)
	{
		//allocator�̍쐬
		for (std::size_t i = 0; i < AllocatorNum; i++)
		{
			ID3D12CommandAllocator* tmp = nullptr;
			if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			allocators[i].reset(tmp);
		}

		//������Ԃł�0�Ԗڂ��g�p
		current_allocaotr_index = 0;

		//list�̍쐬
		{
			ID3D12GraphicsCommandList* tmp = nullptr;
			if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators[current_allocaotr_index].get(), nullptr, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			list.reset(tmp);
		}

		//queue�̍쐬
		{
			ID3D12CommandQueue* tmp = nullptr;
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
			cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//�^�C���A�E�g�i�V
			cmdQueueDesc.NodeMask = 0;
			cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//�v���C�I���e�B���Ɏw��Ȃ�
			cmdQueueDesc.Type = list->GetType();			//�����̓R�}���h���X�g�ƍ��킹��
			if (FAILED(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			queue.reset(tmp);
		}

		//0�ŏ�����
		std::fill(fence_values.begin(), fence_values.end(), 0);

		//fence�쐬
		for (std::size_t i = 0; i < AllocatorNum; i++)
		{
			ID3D12Fence* tmp = nullptr;
			if (FAILED(device->CreateFence(fence_values[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			fences[i].reset(tmp);
		}

		//TODO: �K�v�Ȃ�ł����H
		fence_event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

		//�쐬����list��close����Ă��Ȃ��̂ŌĂяo���K�v������
		list->Close();
	}


	template<std::size_t AllocatorNum>
	inline void pdx12::command_manager<AllocatorNum>::signal()
	{
		fence_values[current_allocaotr_index]++;
		queue->Signal(fences[current_allocaotr_index].get(), fence_values[current_allocaotr_index]);
	}

	template<std::size_t AllocatorNum>
	inline void pdx12::command_manager<AllocatorNum>::wait(std::size_t index)
	{
		if (fences[index]->GetCompletedValue() < fence_values[index])
		{
			fences[index]->SetEventOnCompletion(fence_values[index], fence_event_handle);
			WaitForSingleObject(fence_event_handle, INFINITE);
		}
	}

	template<std::size_t AllocatorNum>
	inline void pdx12::command_manager<AllocatorNum>::reset_list(std::size_t index)
	{
		allocators[index]->Reset();
		list->Reset(allocators[index].get(), nullptr);
	}

	template<std::size_t AllocatorNum>
	inline void pdx12::command_manager<AllocatorNum>::excute()
	{
		queue->ExecuteCommandLists(1, (ID3D12CommandList**)(&list));
	}

	template<std::size_t AllocatorNum>
	inline void pdx12::command_manager<AllocatorNum>::dispatch(std::uint32_t threadGroupCountX, std::uint32_t threadGroupCountY, std::uint32_t threadGroupCountZ)
	{
		list->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	template<std::size_t AllocatorNum>
	inline ID3D12GraphicsCommandList* pdx12::command_manager<AllocatorNum>::get_list()
	{
		return list.get();
	}

	template<std::size_t AllocatorNum>
	inline ID3D12CommandQueue* pdx12::command_manager<AllocatorNum>::get_queue()
	{
		return queue.get();
	}

}
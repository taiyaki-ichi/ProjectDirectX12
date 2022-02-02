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

		//WaitForSingleObjectを呼び出すために必要
		//TODO: メンバで保持する必要があるのか調べる
		HANDLE fence_event_handle = nullptr;

		//現在のlistが使用しているallocaotrのインデックス
		std::size_t current_allocaotr_index = 0;

	public:
		//初期化
		void initialize(ID3D12Device* device);

		//フェンスを立てる
		void signal();

		//立てたフェンスを待つ
		void wait(std::size_t index);

		//index番目のAllocatorでリストをリセット
		void reset_list(std::size_t index);


		void excute();

		//excuteがあるからdispatchもここに作っておく
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
		//allocatorの作成
		for (std::size_t i = 0; i < AllocatorNum; i++)
		{
			ID3D12CommandAllocator* tmp = nullptr;
			if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			allocators[i].reset(tmp);
		}

		//初期状態では0番目を使用
		current_allocaotr_index = 0;

		//listの作成
		{
			ID3D12GraphicsCommandList* tmp = nullptr;
			if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocators[current_allocaotr_index].get(), nullptr, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			list.reset(tmp);
		}

		//queueの作成
		{
			ID3D12CommandQueue* tmp = nullptr;
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
			cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//タイムアウトナシ
			cmdQueueDesc.NodeMask = 0;
			cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//プライオリティ特に指定なし
			cmdQueueDesc.Type = list->GetType();			//ここはコマンドリストと合わせる
			if (FAILED(device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			queue.reset(tmp);
		}

		//0で初期化
		std::fill(fence_values.begin(), fence_values.end(), 0);

		//fence作成
		for (std::size_t i = 0; i < AllocatorNum; i++)
		{
			ID3D12Fence* tmp = nullptr;
			if (FAILED(device->CreateFence(fence_values[i], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&tmp))))
				THROW_PDX12_EXCEPTION("");
			fences[i].reset(tmp);
		}

		//TODO: 必要なんですか？
		fence_event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

		//作成時のlistはcloseされていないので呼び出す必要がある
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
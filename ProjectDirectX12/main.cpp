#include"window.hpp"
#include"device.hpp"
#include"command_manager.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"create_view.hpp"
#include"root_signature.hpp"
#include"shader.hpp"
#include"resource.hpp"
#include"pipeline_state.hpp"
#include<iostream>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	constexpr std::size_t WINDOW_WIDTH = 500;
	constexpr std::size_t WINDOW_HEIGHT = 500;

	constexpr std::size_t FRAME_BUFFER_NUM = 2;
	constexpr DXGI_FORMAT FRAME_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	auto device = pdx12::create_device();

	pdx12::command_manager<1> commandManager{};
	commandManager.initialize(device.get());

	auto swapChain = pdx12::create_swap_chain(commandManager.get_queue(), hwnd, FRAME_BUFFER_FORMAT, FRAME_BUFFER_NUM);

	std::array<ID3D12Resource*, FRAME_BUFFER_NUM> frameBufferResources{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
		swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&frameBufferResources[i]));

	pdx12::descriptor_heap descriptorHeapRTV{};
	descriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_BUFFER_NUM);

	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
		pdx12::create_texture2D_RTV(device.get(), descriptorHeapRTV.get_CPU_handle(i), frameBufferResources[i], FRAME_BUFFER_FORMAT, 1, 1);

	auto rootSignature = pdx12::create_root_signature(device.get(), {}, {});

	auto vertexShader = pdx12::create_shader(L"ShaderFile/SimpleVertexShader.hlsl", "main", "vs_5_0");
	auto pixelShader = pdx12::create_shader(L"ShaderFile/SimplePixelShader.hlsl", "main", "ps_5_0");

	auto vertexBuffer = pdx12::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 3 * 3);
	{
		float* vertexBufferPtr = nullptr;
		vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferPtr));

		//�O�p�`
		vertexBufferPtr[0] = -0.8f;
		vertexBufferPtr[1] = -0.8f;
		vertexBufferPtr[2] = 0.f;
		vertexBufferPtr[3] = -0.8f;
		vertexBufferPtr[4] = 0.8f;
		vertexBufferPtr[5] = 0.f;
		vertexBufferPtr[6] = 0.8f;
		vertexBufferPtr[7] = -0.8f;
		vertexBufferPtr[8] = 0.f;
		vertexBufferPtr[9] = 0.f;

		vertexBuffer->Unmap(0, nullptr);
	}


	auto graphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), rootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32A32_FLOAT } }, { DXGI_FORMAT_R8G8B8A8_UNORM }, { vertexShader.get() }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
	D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };


	while (pdx12::update_window())
	{
		auto backBufferIndex = swapChain->GetCurrentBackBufferIndex();

		commandManager.reset_list(0);

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		{
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = frameBufferResources[backBufferIndex];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandManager.get_list()->ResourceBarrier(1, &barrier);
		}

		auto backBufferCPUHandle = descriptorHeapRTV.get_CPU_handle(backBufferIndex);
		commandManager.get_list()->OMSetRenderTargets(1, &backBufferCPUHandle, 0, nullptr);

		commandManager.get_list()->SetPipelineState(graphicsPipelineState.get());
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandManager.get_list()->SetGraphicsRootSignature(rootSignature.get());

		{
			D3D12_VERTEX_BUFFER_VIEW vbv{};
			vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = sizeof(float) * 3 * 3;
			vbv.StrideInBytes = sizeof(float) * 3;

			commandManager.get_list()->IASetVertexBuffers(0, 1, &vbv);
		}

		commandManager.get_list()->DrawInstanced(3, 1, 0, 0);

		{
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = frameBufferResources[backBufferIndex];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandManager.get_list()->ResourceBarrier(1, &barrier);
		}

		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();
		commandManager.wait(0);

		swapChain->Present(1, 0);
	}

	return 0;
}
#include"window.hpp"
#include"device.hpp"
#include"command_manager.hpp"
#include"swap_chain.hpp"
#include"descriptor_heap.hpp"
#include"create_view.hpp"
#include"root_signature.hpp"
#include"shader.hpp"
#include"resource.hpp"
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

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	auto device = pdx12::create_device();

	pdx12::command_manager<1> commandManager{};
	commandManager.initialize(device.get());

	auto swapChain = pdx12::create_swap_chain(commandManager.get_queue(), hwnd, DXGI_FORMAT_R8G8B8A8_UNORM, 2);

	pdx12::descriptor_heap descriptorHeapCBVSRVUAV{};
	descriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

	auto rootSignature = pdx12::create_root_signature(device.get(), {}, {});

	auto vertexShader = pdx12::create_shader(L"ShaderFile/SimpleVertexShader.hlsl", "main", "vs_5_0");
	auto pixelShader = pdx12::create_shader(L"ShaderFile/SimplePixelShader.hlsl", "main", "ps_5_0");


	auto vertexBuffer = pdx12::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 3 * 3);
	{
		float* vertexBufferPtr = nullptr;
		vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferPtr));

		//ŽOŠpŒ`
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


	while (pdx12::update_window())
	{
		commandManager.reset_list(0);






		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();

		commandManager.wait(0);
	}

	return 0;
}
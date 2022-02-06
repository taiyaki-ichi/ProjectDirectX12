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
#include"OBJ-Loader/Source/OBJ_Loader.h"
#include<iostream>
#include<DirectXMath.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace DirectX;

struct SceneData
{
	XMMATRIX view;
	XMMATRIX proj;
	XMFLOAT3 lightDir;
	float hoge;
	XMFLOAT3 eye;
};

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
	
	std::array<std::pair<pdx12::release_unique_ptr<ID3D12Resource>,D3D12_RESOURCE_STATES>, FRAME_BUFFER_NUM> frameBufferResources{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		ID3D12Resource* tmp = nullptr;
		swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&tmp));
		frameBufferResources[i] = std::make_pair(pdx12::release_unique_ptr<ID3D12Resource>{tmp}, D3D12_RESOURCE_STATE_COMMON);
	}
	
	pdx12::descriptor_heap descriptorHeapRTV{};
	descriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_BUFFER_NUM);

	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
		pdx12::create_texture2D_RTV(device.get(), descriptorHeapRTV.get_CPU_handle(i), frameBufferResources[i].first.get(), FRAME_BUFFER_FORMAT, 0, 0);

	pdx12::descriptor_heap descriptorHeapDSV{};
	descriptorHeapDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, FRAME_BUFFER_NUM);

	D3D12_CLEAR_VALUE depthStencilClearValue{};
	depthStencilClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilClearValue.DepthStencil.Depth = 1.f;

	std::array<std::pair<pdx12::release_unique_ptr<ID3D12Resource>, D3D12_RESOURCE_STATES>, FRAME_BUFFER_NUM> depthBuffers{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
		depthBuffers[i] = pdx12::create_commited_texture_resource(device.get(), DXGI_FORMAT_D32_FLOAT, WINDOW_WIDTH, WINDOW_HEIGHT, 
			2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,&depthStencilClearValue);

	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
		pdx12::create_texture2D_DSV(device.get(), descriptorHeapDSV.get_CPU_handle(i), depthBuffers[i].first.get(), DXGI_FORMAT_D32_FLOAT, 0);

	auto vertexShader = pdx12::create_shader(L"ShaderFile/SimpleVertexShader.hlsl", "main", "vs_5_0");
	auto pixelShader = pdx12::create_shader(L"ShaderFile/SimplePixelShader.hlsl", "main", "ps_5_0");

	objl::Loader Loader;

	bool loadout = Loader.LoadFile("3DModel/bun_zipper.obj");
	objl::Mesh mesh = Loader.LoadedMeshes[0];

	auto vertexBuffer = pdx12::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 6 * mesh.Vertices.size());
	{
		float* vertexBufferPtr = nullptr;
		vertexBuffer.first->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferPtr));

		for (std::size_t i = 0; i < mesh.Vertices.size(); i++)
		{
			vertexBufferPtr[i * 6] = mesh.Vertices[i].Position.X;
			vertexBufferPtr[i * 6 + 1] = mesh.Vertices[i].Position.Y;
			vertexBufferPtr[i * 6 + 2] = mesh.Vertices[i].Position.Z;
			vertexBufferPtr[i * 6 + 3] = mesh.Vertices[i].Normal.X;
			vertexBufferPtr[i * 6 + 4] = mesh.Vertices[i].Normal.Y;
			vertexBufferPtr[i * 6 + 5] = mesh.Vertices[i].Normal.Z;
		}

		vertexBuffer.first->Unmap(0, nullptr);
	}

	D3D12_VERTEX_BUFFER_VIEW triangleVertexBufferView{};
	triangleVertexBufferView.BufferLocation = vertexBuffer.first->GetGPUVirtualAddress();
	triangleVertexBufferView.SizeInBytes = sizeof(float) * 6 * mesh.Vertices.size();
	triangleVertexBufferView.StrideInBytes = sizeof(float) * 6;

	auto rootSignature = pdx12::create_root_signature(device.get(), { {D3D12_DESCRIPTOR_RANGE_TYPE_CBV,D3D12_DESCRIPTOR_RANGE_TYPE_CBV} }, {});

	auto graphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), rootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } }, { FRAME_BUFFER_FORMAT }, { vertexShader.get(),pixelShader.get() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	
	D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
	D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

	XMFLOAT3 eye{ 0,2.f,2.f };
	XMFLOAT3 target{ 0,0.1,0 };
	XMFLOAT3 up{ 0,1,0 };
	auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV2,
		static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
		0.01f,
		100.f
	);
	XMFLOAT3 lightDir{ 1.f,1.f,1.f };

	SceneData sceneData{
		view,
		proj,
		lightDir,
		0.f,
		eye
	};

	XMMATRIX world = XMMatrixScaling(10.f, 10.f, 10.f);

	auto sceneDataBufferResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(SceneData), 256));
	SceneData* mappedSceneDataPtr = nullptr;
	sceneDataBufferResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedSceneDataPtr));
	*mappedSceneDataPtr = sceneData;

	auto worldBufferResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(XMMATRIX), 256));
	XMMATRIX* mappedWorldPtr = nullptr;
	worldBufferResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedWorldPtr));

	pdx12::descriptor_heap descriptorHeapCBVSRVUAV{};
	descriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);
	pdx12::create_CBV(device.get(), descriptorHeapCBVSRVUAV.get_CPU_handle(0), sceneDataBufferResource.first.get(), pdx12::alignment<UINT64>(sizeof(SceneData), 256));
	pdx12::create_CBV(device.get(), descriptorHeapCBVSRVUAV.get_CPU_handle(1), worldBufferResource.first.get(), pdx12::alignment<UINT64>(sizeof(XMMATRIX), 256));
	
	while (pdx12::update_window())
	{
		world *= XMMatrixRotationY(0.01f);
		*mappedWorldPtr = world;

		auto backBufferIndex = swapChain->GetCurrentBackBufferIndex();

		commandManager.reset_list(0);

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		pdx12::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);

		{
			std::array<float, 4> clearColor{ 0.5f,0.5f,0.5f,1.f };
			commandManager.get_list()->ClearRenderTargetView(descriptorHeapRTV.get_CPU_handle(backBufferIndex), clearColor.data(), 0, nullptr);
		}

		pdx12::resource_barrior(commandManager.get_list(), depthBuffers[backBufferIndex], D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandManager.get_list()->ClearDepthStencilView(descriptorHeapDSV.get_CPU_handle(backBufferIndex), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0.f, 0, nullptr);

		auto backBufferCPUHandle = descriptorHeapRTV.get_CPU_handle(backBufferIndex);
		auto depthBufferCPUHandle = descriptorHeapDSV.get_CPU_handle(backBufferIndex);
		commandManager.get_list()->OMSetRenderTargets(1, &backBufferCPUHandle, false, &depthBufferCPUHandle);

		commandManager.get_list()->SetGraphicsRootSignature(rootSignature.get());
		{
			auto ptr = descriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, descriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(graphicsPipelineState.get());
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &triangleVertexBufferView);

		commandManager.get_list()->DrawInstanced(mesh.Vertices.size(), 1, 0, 0);

		pdx12::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_COMMON);
		pdx12::resource_barrior(commandManager.get_list(), depthBuffers[backBufferIndex], D3D12_RESOURCE_STATE_COMMON);

		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();
		commandManager.wait(0);

		swapChain->Present(1, 0);
	}
	
	return 0;
}
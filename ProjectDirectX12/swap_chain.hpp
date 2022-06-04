#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"

namespace pdx12
{
	release_unique_ptr<IDXGISwapChain3> create_swap_chain(ID3D12CommandQueue* commandQueue, HWND hwnd, DXGI_FORMAT frameBufferFormat, UINT frameBufferNum);
}
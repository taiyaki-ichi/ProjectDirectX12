#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<stdexcept>
#include<iterator>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{
	
	inline ID3D12Device* create_device()
	{

		//デバッグモードでコンパイルされた場合
#ifdef _DEBUG
		ID3D12Debug* debugLayer = nullptr;
		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) 
		{
			throw std::runtime_error{ "failed D3D12GetDebugInterface" };
		}
		else 
		{
			debugLayer->EnableDebugLayer();
			debugLayer->Release();
		}
#endif


		IDXGIFactory1* factory = nullptr;
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
		{
			throw std::runtime_error{ "failed CreateDXGIFactory1" };
		}


		IDXGIAdapter1* adapter = nullptr;

		//アダプターの作成
		{
			UINT adapterIndex = 0;
			DXGI_ADAPTER_DESC1 desc{};

			//adapterIndexで走査し利用できるアダプターを作成する
			while (true) 
			{
				factory->EnumAdapters1(adapterIndex, &adapter);
				adapter->GetDesc1(&desc);

				//適切なアダプタが見つかった場合
				if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
					break;

				adapterIndex++;
				//見つからなかった場合
				if (adapterIndex == DXGI_ERROR_NOT_FOUND)
				{
					throw std::runtime_error{ "not found adapter" };
				}
			}
		}


		//Direct3Dデバイスが対象する機能を識別する定数をいくつか列挙しておく
		D3D_FEATURE_LEVEL levels[]={
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};


		ID3D12Device* device = nullptr;

		{
			std::size_t i = 0;
			for (; i < std::size(levels); i++)
			{
				if (SUCCEEDED(D3D12CreateDevice(adapter, levels[i], IID_PPV_ARGS(&device))))
				{
					//作成できたらループを抜ける
					break;
				}
			}

			//生成できなかった場合は例外を投げる
			if (i == std::size(levels))
			{
				throw std::runtime_error{ "failed D3D12CreateDevice" };
			}
		}


		//もう不要なので解放する
		//TODO: 途中で例外が投げられた際に適切に解放させないのでスマートポインタにする必要あり
		factory->Release();
		adapter->Release();

		return device;
	}

}
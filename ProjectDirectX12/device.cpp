#pragma once
#include"device.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{

	pdx12::release_unique_ptr<ID3D12Device> create_device()
	{

#ifdef _DEBUG
		ID3D12Debug* debugLayer = nullptr;
		if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
		{
			THROW_PDX12_EXCEPTION("failed D3D12GetDebugInterface");
		}
		else
		{
			debugLayer->EnableDebugLayer();
			debugLayer->Release();
		}
#endif

		pdx12::release_unique_ptr<IDXGIFactory4> factory{};

		// ファクトリーの作成
		{
			IDXGIFactory4* tmp = nullptr;
			if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&tmp))))
			{
				THROW_PDX12_EXCEPTION("failed CreateDXGIFactory1");
			}
			factory.reset(tmp);
		}


		pdx12::release_unique_ptr<IDXGIAdapter3> adaptor{};

		// アダプターの作成
		{
			IDXGIAdapter3* tmp = nullptr;
			UINT adapterIndex = 0;
			DXGI_ADAPTER_DESC1 desc{};

			// adapterIndexで走査し利用できるアダプターを作成する
			while (true)
			{
				factory->EnumAdapters1(adapterIndex, (IDXGIAdapter1**)&tmp);
				tmp->GetDesc1(&desc);

				// 適切なアダプタが見つかった場合
				if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
					break;

				adapterIndex++;
				// 見つからなかった場合
				if (adapterIndex == DXGI_ERROR_NOT_FOUND)
				{
					THROW_PDX12_EXCEPTION("not found adapter");
				}
			}

			adaptor.reset(tmp);
		}


		pdx12::release_unique_ptr<ID3D12Device> device{};

		// デバイスの作成
		{
			ID3D12Device* tmp = nullptr;

			// Direct3Dデバイスが対象する機能を識別する定数をいくつか列挙しておく
			D3D_FEATURE_LEVEL levels[] = {
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};

			std::size_t i = 0;
			for (; i < std::size(levels); i++)
			{
				if (SUCCEEDED(D3D12CreateDevice(adaptor.get(), levels[i], IID_PPV_ARGS(&tmp))))
				{
					// 作成できたらループを抜ける
					break;
				}
			}

			// 生成できなかった場合は例外を投げる
			if (i == std::size(levels))
			{
				THROW_PDX12_EXCEPTION("failed D3D12CreateDevice");
			}

			device.reset(tmp);
		}

		return device;
	}

}
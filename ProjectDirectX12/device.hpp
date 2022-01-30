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

		//�f�o�b�O���[�h�ŃR���p�C�����ꂽ�ꍇ
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

		//�A�_�v�^�[�̍쐬
		{
			UINT adapterIndex = 0;
			DXGI_ADAPTER_DESC1 desc{};

			//adapterIndex�ő��������p�ł���A�_�v�^�[���쐬����
			while (true) 
			{
				factory->EnumAdapters1(adapterIndex, &adapter);
				adapter->GetDesc1(&desc);

				//�K�؂ȃA�_�v�^�����������ꍇ
				if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
					break;

				adapterIndex++;
				//������Ȃ������ꍇ
				if (adapterIndex == DXGI_ERROR_NOT_FOUND)
				{
					throw std::runtime_error{ "not found adapter" };
				}
			}
		}


		//Direct3D�f�o�C�X���Ώۂ���@�\�����ʂ���萔���������񋓂��Ă���
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
					//�쐬�ł����烋�[�v�𔲂���
					break;
				}
			}

			//�����ł��Ȃ������ꍇ�͗�O�𓊂���
			if (i == std::size(levels))
			{
				throw std::runtime_error{ "failed D3D12CreateDevice" };
			}
		}


		//�����s�v�Ȃ̂ŉ������
		//TODO: �r���ŗ�O��������ꂽ�ۂɓK�؂ɉ�������Ȃ��̂ŃX�}�[�g�|�C���^�ɂ���K�v����
		factory->Release();
		adapter->Release();

		return device;
	}

}
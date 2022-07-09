#include"root_signature.hpp"

#pragma comment(lib,"d3d12.lib")

namespace pdx12
{
	release_unique_ptr<ID3D12RootSignature> create_root_signature(ID3D12Device* device,
		std::vector<std::vector<descriptor_range_type>> const& descriptorRangeTypes, const std::vector<static_sampler>& staticSamplers)
	{
		std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRanges{};

		//配列の構造をもとにレジスタの番号を振っりD3D12_DESCRIPTOR_RANGEを作っていく
		{
			descriptorRanges.reserve(descriptorRangeTypes.size());

			std::array<std::uint32_t, 4> registerNums{};

			for (auto& rangeTypes : descriptorRangeTypes) {
				std::vector<D3D12_DESCRIPTOR_RANGE> ranges{};
				ranges.reserve(rangeTypes.size());

				for (auto& t : rangeTypes) {
					D3D12_DESCRIPTOR_RANGE descriptorRange{};
					descriptorRange.NumDescriptors = t.num;
					descriptorRange.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t.descriptorRangeType);
					descriptorRange.BaseShaderRegister = registerNums[static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t.descriptorRangeType)];
					descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

					ranges.push_back(std::move(descriptorRange));
					registerNums[static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(t.descriptorRangeType)] += t.num;
				}

				descriptorRanges.push_back(std::move(ranges));
			}
		}


		std::vector<D3D12_ROOT_PARAMETER> descriptorTables{};

		//descriptorRangesをもとにディスクリプタテーブルを作成
		{
			descriptorTables.reserve(descriptorRanges.size());

			for (auto& r : descriptorRanges) {
				D3D12_ROOT_PARAMETER tmp{};
				tmp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				tmp.DescriptorTable.pDescriptorRanges = r.data();
				tmp.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(r.size());
				tmp.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

				descriptorTables.push_back(std::move(tmp));
			}
		}


		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplerDescs(staticSamplers.size());

		{
			for (std::uint32_t i = 0; i < staticSamplers.size(); i++)
			{
				staticSamplerDescs[i].Filter = staticSamplers[i].filter;
				staticSamplerDescs[i].AddressU = staticSamplers[i].address_u;
				staticSamplerDescs[i].AddressV = staticSamplers[i].address_v;
				staticSamplerDescs[i].AddressW = staticSamplers[i].address_w;
				staticSamplerDescs[i].ComparisonFunc = staticSamplers[i].comparison_func;

				staticSamplerDescs[i].MaxAnisotropy = staticSamplers[i].max_anisotropy;

				staticSamplerDescs[i].MinLOD = staticSamplers[i].min_LOD;
				staticSamplerDescs[i].MaxLOD = staticSamplers[i].max_LOD;

				staticSamplerDescs[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				staticSamplerDescs[i].ShaderRegister = i;
			}
		}



		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSignatureDesc.NumParameters = static_cast<UINT>(descriptorTables.size());
		rootSignatureDesc.pParameters = descriptorTables.size() > 0 ? descriptorTables.data() : nullptr;
		rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplerDescs.size());
		rootSignatureDesc.pStaticSamplers = staticSamplerDescs.size() > 0 ? staticSamplerDescs.data() : nullptr;


		ID3DBlob* rootSigBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		release_unique_ptr<ID3D12RootSignature> result{};

		if (D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob))
		{
			std::string errstr{};
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errorBlob->Release();

			THROW_PDX12_EXCEPTION(errstr.data());
		}

		ID3D12RootSignature* tmp = nullptr;
		if (FAILED(device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&tmp))))
		{
			THROW_PDX12_EXCEPTION("failed CreateRootSignature");
		}

		result.reset(tmp);
		rootSigBlob->Release();

		return result;
	}

}
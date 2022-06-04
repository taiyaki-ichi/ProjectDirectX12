#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"
#include<vector>


namespace pdx12
{
	struct input_element
	{
		char const* name;
		DXGI_FORMAT format;
	};

	struct shader_desc 
	{
		ID3DBlob* vertex_shader = nullptr;
		ID3DBlob* pixcel_shader = nullptr;
		ID3DBlob* geometry_shader = nullptr;
		ID3DBlob* hull_shader = nullptr;
		ID3DBlob* domain_shader = nullptr;
	};

	release_unique_ptr<ID3D12PipelineState> create_graphics_pipeline(ID3D12Device* device,
		ID3D12RootSignature* rootSignature, std::vector<input_element> const& inputElements, std::vector<DXGI_FORMAT> const& renderTargetFormats,
		shader_desc const& shaderDescs, bool depthEnable, bool alphaBlend, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType);

	release_unique_ptr<ID3D12PipelineState> create_compute_pipeline(ID3D12Device* device,
		ID3D12RootSignature* rootSignature, ID3DBlob* computeShader);
}
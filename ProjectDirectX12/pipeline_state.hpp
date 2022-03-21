#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"
#include<vector>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


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

	inline release_unique_ptr<ID3D12PipelineState> create_graphics_pipeline(ID3D12Device* device, 
		ID3D12RootSignature* rootSignature, std::vector<input_element> const& inputElements, std::vector<DXGI_FORMAT> const& renderTargetFormats,
		shader_desc const& shaderDescs,bool depthEnable, bool alphaBlend, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineDesc{};

		//シェーダの設定
		if (shaderDescs.vertex_shader) {
			graphicsPipelineDesc.VS.pShaderBytecode = shaderDescs.vertex_shader->GetBufferPointer();
			graphicsPipelineDesc.VS.BytecodeLength = shaderDescs.vertex_shader->GetBufferSize();
		}
		if (shaderDescs.pixcel_shader) {
			graphicsPipelineDesc.PS.pShaderBytecode = shaderDescs.pixcel_shader->GetBufferPointer();
			graphicsPipelineDesc.PS.BytecodeLength = shaderDescs.pixcel_shader->GetBufferSize();
		}
		if (shaderDescs.geometry_shader) {
			graphicsPipelineDesc.GS.pShaderBytecode = shaderDescs.geometry_shader->GetBufferPointer();
			graphicsPipelineDesc.GS.BytecodeLength = shaderDescs.geometry_shader->GetBufferSize();
		}
		if (shaderDescs.hull_shader) {
			graphicsPipelineDesc.HS.pShaderBytecode = shaderDescs.hull_shader->GetBufferPointer();
			graphicsPipelineDesc.HS.BytecodeLength = shaderDescs.hull_shader->GetBufferSize();
		}
		if (shaderDescs.domain_shader) {
			graphicsPipelineDesc.DS.pShaderBytecode = shaderDescs.domain_shader->GetBufferPointer();
			graphicsPipelineDesc.DS.BytecodeLength = shaderDescs.domain_shader->GetBufferSize();
		}



		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs(inputElements.size());
		for (std::size_t i = 0; i < inputElements.size(); i++)
		{
			inputElementDescs[i].SemanticName = inputElements[i].name;
			inputElementDescs[i].Format = inputElements[i].format;
			inputElementDescs[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		}

		graphicsPipelineDesc.InputLayout.pInputElementDescs = inputElementDescs.data();
		graphicsPipelineDesc.InputLayout.NumElements = inputElementDescs.size();
		graphicsPipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//カット値はナシ
		graphicsPipelineDesc.PrimitiveTopologyType = primitiveTopologyType;



		//ラスタライザの設定
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		graphicsPipelineDesc.RasterizerState = rasterizerDesc;


		//ブレンドステート
		D3D12_RENDER_TARGET_BLEND_DESC renderTagetBlendDesc{};
		if (alphaBlend)
		{
			renderTagetBlendDesc.BlendEnable = TRUE;
			renderTagetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			renderTagetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;;
			renderTagetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			renderTagetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			renderTagetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			renderTagetBlendDesc.LogicOpEnable = FALSE;
		}
		else {
			renderTagetBlendDesc.BlendEnable = FALSE;
			renderTagetBlendDesc.LogicOpEnable = FALSE;
			renderTagetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = TRUE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0] = renderTagetBlendDesc;

		graphicsPipelineDesc.BlendState = blendDesc;


		graphicsPipelineDesc.NumRenderTargets = renderTargetFormats.size();
		std::copy(renderTargetFormats.begin(), renderTargetFormats.end(), std::begin(graphicsPipelineDesc.RTVFormats));


		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = depthEnable;
		if (depthEnable) {
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		}
		depthStencilDesc.StencilEnable = FALSE;

		graphicsPipelineDesc.DepthStencilState = depthStencilDesc;


		//サンプルディスク
		graphicsPipelineDesc.SampleDesc.Count = 1;
		graphicsPipelineDesc.SampleDesc.Quality = 0;


		//これないとまずい
		graphicsPipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;


		graphicsPipelineDesc.pRootSignature = rootSignature;


		ID3D12PipelineState* tmp = nullptr;
		if (FAILED(device->CreateGraphicsPipelineState(&graphicsPipelineDesc, IID_PPV_ARGS(&tmp))))
		{
			THROW_PDX12_EXCEPTION("failed CreateGraphicsPipelineState");
		}

		return release_unique_ptr<ID3D12PipelineState>{tmp};
	}

	inline release_unique_ptr<ID3D12PipelineState> create_compute_pipeline(ID3D12Device* device,
		ID3D12RootSignature* rootSignature, ID3DBlob* computeShader)
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc{};
		computePipelineDesc.CS.pShaderBytecode = computeShader->GetBufferPointer();
		computePipelineDesc.CS.BytecodeLength = computeShader->GetBufferSize();
		computePipelineDesc.pRootSignature = rootSignature;

		ID3D12PipelineState* tmp = nullptr;
		if (FAILED(device->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&tmp))))
		{
			THROW_PDX12_EXCEPTION("failed CreateComputePipelineState");
		}

		return release_unique_ptr<ID3D12PipelineState>{tmp};
	}
}
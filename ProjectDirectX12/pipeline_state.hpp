#pragma once
#include<d3d12.h>
#include<dxgi1_4.h>
#include"utility.hpp"
#include<vector>


namespace pdx12
{
	// グラフィックパイプラインの初期化用
	struct input_element
	{
		char const* name;
		DXGI_FORMAT format;
	};

	// グラフィックパイプラインの初期化用
	// 深度をとるだけとかのrenderTargerFormatsが空の場合はshader_desc.pixel_shaderはnullpterで良かった気がする
	// テッセレーションしたい場合shader_desc.hull_shaderとshader_desc.domain_shaderのどちらも設定する必要あり
	// 基本はshader_desc.vertex_shaderとshader_desc.pixel_shaderに値が設定されてればおけ
	struct shader_desc 
	{
		ID3DBlob* vertex_shader = nullptr;
		ID3DBlob* pixcel_shader = nullptr;
		ID3DBlob* geometry_shader = nullptr;
		ID3DBlob* hull_shader = nullptr;
		ID3DBlob* domain_shader = nullptr;
	};

	// もうちょいいろいろ設定できるように変更したい
	// ワイヤーフレームで描写できたり, ステンシルテストできたりとか
	release_unique_ptr<ID3D12PipelineState> create_graphics_pipeline(ID3D12Device* device,
		ID3D12RootSignature* rootSignature, std::vector<input_element> const& inputElements, std::vector<DXGI_FORMAT> const& renderTargetFormats,
		shader_desc const& shaderDescs, bool depthEnable, bool alphaBlend, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType);

	release_unique_ptr<ID3D12PipelineState> create_compute_pipeline(ID3D12Device* device,
		ID3D12RootSignature* rootSignature, ID3DBlob* computeShader);
}
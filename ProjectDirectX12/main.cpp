#include"../external/directx12-wrapper/dx12w/dx12w.hpp"
#include"../external/OBJ-Loader/Source/OBJ_Loader.h"
#include"Input/create_direct_input.hpp"
#include"Input/keyboard_device.hpp"
#include"Input/mouse_device.hpp"
#include"Input/gamepad_device.hpp"
#include"TPS.hpp"
#include"utility.hpp"
#include<iostream>
#include<random>
#include<chrono>
#include<DirectXMath.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

using namespace DirectX;

// 
// 定数
// 

constexpr std::size_t WINDOW_WIDTH = 512;
constexpr std::size_t WINDOW_HEIGHT = 512;

constexpr float CAMERA_NEAR_Z = 0.01f;
constexpr float CAMERA_FAR_Z = 1000.f;

constexpr float VIEW_ANGLE = DirectX::XM_PIDIV2;

constexpr std::size_t FRAME_BUFFER_NUM = 2;
constexpr DXGI_FORMAT FRAME_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

constexpr std::size_t SHADOW_MAP_NUM = 3;

constexpr std::size_t COMMAND_ALLOCATORE_NUM = 1 + SHADOW_MAP_NUM;

constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;
constexpr DXGI_FORMAT DEPTH_BUFFER_SRV_FORMAT = DXGI_FORMAT_R32_FLOAT;

// GBufferのアルベドカラーのフォーマット
// フレームバッファと同じフォーマットにしておく
constexpr DXGI_FORMAT G_BUFFER_ALBEDO_COLOR_FORMAT = FRAME_BUFFER_FORMAT;

// GBufferの法線のフォーマット
// 正規化された8ビットの浮動小数4つで表現
constexpr DXGI_FORMAT G_BUFFER_NORMAL_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

// GBufferの位置のフォーマット
// それぞれ32ビットにしたが助長かも
constexpr DXGI_FORMAT G_BUFFER_WORLD_POSITION_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;

// 高輝度のファーマっと
// フレームバッファと同じでおけ
constexpr DXGI_FORMAT HIGH_LUMINANCE_FORMAT = FRAME_BUFFER_FORMAT;

// 高輝度のリソースのサイズ
// ウィンドウと同じサイズでおｋ
constexpr std::size_t HIGH_LUMINANCE_WIDTH = WINDOW_WIDTH;
constexpr std::size_t HIGH_LUMINANCE_HEIGHT = WINDOW_HEIGHT;

// 縮小された高輝度のフォーマット
// 高輝度と同じフォーマットでおけ
constexpr DXGI_FORMAT SHRINKED_HIGH_LUMINANCE_FORMAT = HIGH_LUMINANCE_FORMAT;

// 縮小された高輝度のリソースの数
constexpr std::size_t SHRINKED_HIGH_LUMINANCE_NUM = 4;

// ポストエフェクトをかける前のリソースのフォーマット
// フレームバッファのフォーマットと同じで
constexpr DXGI_FORMAT MAIN_COLOR_RESOURCE_FORMAT = FRAME_BUFFER_FORMAT;

constexpr std::size_t MAIN_COLOR_RESOURCE_WIDTH = WINDOW_WIDTH;
constexpr std::size_t MAIN_COLOR_RESOURCE_HEIGHT = WINDOW_HEIGHT;

// 縮小されダウンサンプリングされたリソースのフォーマット
// 被写界深度のポストエフェクトをかける際に使用する
constexpr DXGI_FORMAT SHRINKED_MAIN_COLOR_RESOURCE_FORMAT = MAIN_COLOR_RESOURCE_FORMAT;

// 縮小されダウンサンプリングされたリソースのフォーマットの数
constexpr std::size_t SHRINKED_MAIN_COLOR_RESOURCE_NUM = 4;

// シャドウマップのフォーマット
constexpr DXGI_FORMAT SHADOW_MAP_FORMAT = DXGI_FORMAT_D32_FLOAT;
constexpr DXGI_FORMAT SHADOW_MAP_SRV_FORMAT = DXGI_FORMAT_R32_FLOAT;

// カスケードシャドウマップのサイズ
// 近い順
constexpr std::array<std::size_t, SHADOW_MAP_NUM> SHADOW_MAP_SIZE = {
	2048,
	2048,
	2048,
};


// これ, SHADOW_MAP_NUMの値が変るとパッキング崩れそう
template<std::size_t SHADOW_MAP_NUM>
struct ShadowMapData
{
	// シャドウマップの距離テーブル
	std::array<float, SHADOW_MAP_NUM> areaTable{};
	float poissonDiskSampleRadius;
	// シェーダで影になっているかを判定する際に使用するバイアス
	// 一応CPU側で設定できるようにしておく
	std::array<float, SHADOW_MAP_NUM> biasTanle{};
};

// ライトのビュープロジェクション行列の数
// シャドウマップと同じ数
constexpr std::size_t LIGHT_VIEW_PROJ_MATRIX_NUM = SHADOW_MAP_NUM;

constexpr std::size_t LIGHT_CULLING_TILE_WIDTH = 16;
constexpr std::size_t LIGHT_CULLING_TILE_HEIGHT = 16;
constexpr std::size_t LIGHT_CULLING_TILE_NUM = (WINDOW_WIDTH / LIGHT_CULLING_TILE_WIDTH) * (WINDOW_HEIGHT / LIGHT_CULLING_TILE_HEIGHT);


constexpr DXGI_FORMAT AMBIENT_OCCLUSION_RESOURCE_FORMAT = DXGI_FORMAT_R32_FLOAT;

constexpr std::size_t AMBIENT_OCCLUSION_RESOURCE_WIDTH = WINDOW_WIDTH;
constexpr std::size_t AMBIENT_OCCLUSION_RESOURCE_HEIGHT = WINDOW_HEIGHT;


constexpr std::size_t MAX_POINT_LIGHT_NUM = 1000;

struct PointLight
{
	XMFLOAT3 pos;
	float _pad0;
	XMFLOAT3 posInView;
	float _pad1;
	XMFLOAT3 color;
	float range;
};

struct DirectionLight
{
	XMFLOAT3 dir;
	float _pad0;
	XMFLOAT3 color;
	float _pad1;
};


struct CameraData
{
	XMMATRIX view;
	XMMATRIX viewInv;
	XMMATRIX proj;
	XMMATRIX projInv;
	XMMATRIX viewProj;
	XMMATRIX viewProjInv;
	float cameraNear;
	float cameraFar;
	float screenWidth;
	float screenHeight;
	XMFLOAT3 eyePos;
	float _pad0;
};

struct LightData
{
	DirectionLight directionLight;
	std::array<XMMATRIX, SHADOW_MAP_NUM> directionLightViewProj;
	std::array<PointLight, MAX_POINT_LIGHT_NUM> pointLight;
	std::uint32_t pointLightNum;
	float specPow;
	float _pad0;
	float _pad1;
};

struct PostEffectData
{
	// 被写界深度を計算する際の基準となるデプスのUV座標
	XMFLOAT2 depthDiffCenter;
	// デプスの差にかかる補正の定数
	float depthDiffPower;
	// デプスの差がdepthDiffLower以下の場合ぼかさない
	float depthDiffLower;

	// 高輝度をメインの色に加算するまえにかける補正値
	float luminanceDegree;

	float _pad0;
	float _pad1;
	float _pad2;
};

constexpr std::size_t MODEL_WIDTH_NUM = 10;
constexpr std::size_t MODEL_HEIGHT_NUM = 10;
constexpr std::size_t MODEL_NUM = MODEL_WIDTH_NUM * MODEL_HEIGHT_NUM;

struct ModelData
{
	XMMATRIX world[MODEL_NUM];
};

struct GroundData
{
	XMMATRIX world;
};

struct DispatchData
{
	std::uint32_t dispatchX;
	std::uint32_t dispatchY;
};

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	auto hwnd = dx12w::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	auto device = dx12w::create_device();

	dx12w::command_manager<COMMAND_ALLOCATORE_NUM> commandManager{};
	commandManager.initialize(device.get());

	auto swapChain = dx12w::create_swap_chain(commandManager.get_queue(), hwnd, FRAME_BUFFER_FORMAT, FRAME_BUFFER_NUM);

	// 
	// クリアの値などの定義
	// 

	// GBufferに描写する際に使用するデプスバッファをクリアする際に使用
	D3D12_CLEAR_VALUE depthBufferClearValue{};
	depthBufferClearValue.Format = DEPTH_BUFFER_FORMAT;
	depthBufferClearValue.DepthStencil.Depth = 1.f;

	// 灰色
	constexpr std::array<float, 4> grayColor{ 0.5f,0.5f,0.5f,1.f };
	D3D12_CLEAR_VALUE grayClearValue{};
	grayClearValue.Format = FRAME_BUFFER_FORMAT;
	std::copy(grayColor.begin(), grayColor.end(), std::begin(grayClearValue.Color));

	// 全部ゼロ
	constexpr std::array<float, 4> zeroFloat4{ 0.f,0.f,0.f,0.f };
	// ゼロでクリアするようなClearValueを生成するラムダ式
	auto getZeroFloat4CearValue = [&zeroFloat4](DXGI_FORMAT format) {
		D3D12_CLEAR_VALUE result{};
		result.Format = format;
		std::copy(zeroFloat4.begin(), zeroFloat4.end(), std::begin(result.Color));
		return result;
	};


	// 
	// リソースの作成
	// 

	// フレームバッファのリソース
	std::array<dx12w::resource_and_state, FRAME_BUFFER_NUM> frameBufferResources{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		ID3D12Resource* tmp = nullptr;
		swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&tmp));
		frameBufferResources[i] = std::make_pair(dx12w::release_unique_ptr<ID3D12Resource>{tmp}, D3D12_RESOURCE_STATE_COMMON);
	}

	// デプスバッファのリソース
	auto depthBuffer = dx12w::create_commited_texture_resource(device.get(), DEPTH_BUFFER_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT,
		2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);

	// GBUfferのアルベドカラーのリソース
	// ClearValueははフレームバッファと同じ色
	auto gBufferAlbedoColorResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	// GBufferの法線のリソース
	auto normalZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_NORMAL_FORMAT);
	auto gBufferNormalResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_NORMAL_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &normalZeroFloat4ClearValue);

	// GBufferのワールド座標のリソース
	auto worldPositionZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_WORLD_POSITION_FORMAT);
	auto gBufferWorldPositionResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_WORLD_POSITION_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &worldPositionZeroFloat4ClearValue);


	// モデルの頂点情報のリソースとそのビュー
	dx12w::resource_and_state modelVertexBuffer{};
	D3D12_VERTEX_BUFFER_VIEW modelVertexBufferView{};
	std::size_t modelVertexNum = 0;
	{
		objl::Loader Loader;

		bool loadout = Loader.LoadFile("3DModel/bun_zipper_res2.obj");
		objl::Mesh mesh = Loader.LoadedMeshes[0];

		modelVertexBuffer = dx12w::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 6 * mesh.Vertices.size());
		{
			float* vertexBufferPtr = nullptr;
			modelVertexBuffer.first->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferPtr));

			for (std::size_t i = 0; i < mesh.Vertices.size(); i++)
			{
				vertexBufferPtr[i * 6] = mesh.Vertices[i].Position.X;
				vertexBufferPtr[i * 6 + 1] = mesh.Vertices[i].Position.Y;
				vertexBufferPtr[i * 6 + 2] = mesh.Vertices[i].Position.Z;
				vertexBufferPtr[i * 6 + 3] = mesh.Vertices[i].Normal.X;
				vertexBufferPtr[i * 6 + 4] = mesh.Vertices[i].Normal.Y;
				vertexBufferPtr[i * 6 + 5] = mesh.Vertices[i].Normal.Z;
			}

			modelVertexBuffer.first->Unmap(0, nullptr);
		}

		modelVertexBufferView.BufferLocation = modelVertexBuffer.first->GetGPUVirtualAddress();
		modelVertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(float) * 6 * mesh.Vertices.size());
		modelVertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(float) * 6);

		modelVertexNum = mesh.Vertices.size();
	}

	// フレームバッファに描画する用のぺらポリゴンの頂点データのリソース
	// サイズについて座標でfloat三要素分、uvで二要素分でsize(float)*5、ペラポリゴンは四角形なのでx4
	auto peraPolygonVertexBufferResource = dx12w::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 5 * 4);
	std::size_t peraPolygonVertexNum = 4;
	{
		// floatのうちわけ-> float3: pos , float2 uv
		std::array<std::array<float, 5>, 4>* peraPolygonVertexBufferPtr = nullptr;
		peraPolygonVertexBufferResource.first->Map(0, nullptr, reinterpret_cast<void**>(&peraPolygonVertexBufferPtr));

		*peraPolygonVertexBufferPtr = { {
			{-1.f,-1.f,0.1f,0,1.f},
			{-1.f,1.f,0.1f,0,0},
			{1.f,-1.f,0.1f,1.f,1.f},
			{1.f,1.f,0.1f,1.f,0}
		} };

		peraPolygonVertexBufferResource.first->Unmap(0, nullptr);
	}
	D3D12_VERTEX_BUFFER_VIEW peraPolygonVertexBufferView{};
	peraPolygonVertexBufferView.BufferLocation = peraPolygonVertexBufferResource.first->GetGPUVirtualAddress();
	peraPolygonVertexBufferView.SizeInBytes = sizeof(float) * 5 * 4;
	peraPolygonVertexBufferView.StrideInBytes = sizeof(float) * 5;

	// モデルをGBufferに書き込む際のシーンのデータを扱うリソース
	// 定数バッファのため256アライメントする必要がある
	auto cameraDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(CameraData), 256));

	// ライトの情報を扱うリソース
	auto lightDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(LightData), 256));

	// モデルをGBufferに書き込む際のモデルのデータを扱うリソース
	// 定数バッファのため256アライメントする必要がある
	auto modelDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(ModelData), 256));

	// 地面をGbufferに書き込む際のグランドのデータを扱うリソース
	auto groundDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(GroundData), 256));

	// 高輝度のリソース
	auto highLuminanceClearValue = getZeroFloat4CearValue(HIGH_LUMINANCE_FORMAT);
	auto highLuminanceResource = dx12w::create_commited_texture_resource(device.get(), HIGH_LUMINANCE_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &highLuminanceClearValue);

	// 縮小された高輝度のリソース
	std::array<dx12w::resource_and_state, SHRINKED_HIGH_LUMINANCE_NUM> shrinkedHighLuminanceResource{};
	{
		std::size_t w = WINDOW_WIDTH;
		std::size_t h = WINDOW_HEIGHT;
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
		{
			w /= 2;
			h /= 2;
			shrinkedHighLuminanceResource[i] = dx12w::create_commited_texture_resource(device.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, w, static_cast<UINT>(h), 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, nullptr);
		}
	}

	// 高輝度をダウンサンプリングする際の定数バッファのリソース
	auto highLuminanceDownSamplingDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	// ポストエフェクトをかける前のリソース
	auto mainColorResource = dx12w::create_commited_texture_resource(device.get(), MAIN_COLOR_RESOURCE_FORMAT, MAIN_COLOR_RESOURCE_WIDTH, MAIN_COLOR_RESOURCE_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	// メインカラーをダウンサンプリングを行う際の定数バッファのリソース
	auto mainColorDownSamplingDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	// 縮小されたポストエフェクトをかける前のリソース
	// 被写界深度を考慮する際に使用する
	std::array<dx12w::resource_and_state, SHRINKED_MAIN_COLOR_RESOURCE_NUM> shrinkedMainColorResource{};
	{
		auto clearValue = getZeroFloat4CearValue(SHRINKED_MAIN_COLOR_RESOURCE_FORMAT);
		std::size_t w = WINDOW_WIDTH;
		std::size_t h = WINDOW_HEIGHT;
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
		{
			w /= 2;
			h /= 2;

			shrinkedMainColorResource[i] = dx12w::create_commited_texture_resource(device.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, w, static_cast<UINT>(h), 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, nullptr);
		}
	}

	// ポストエフェクトの情報の定数バッファ
	auto postEffectDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(PostEffectData), 256));

	// シャドウマップのリソース
	std::array<dx12w::resource_and_state, SHADOW_MAP_NUM> shadowMapResource{};
	{
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			shadowMapResource[i] = dx12w::create_commited_texture_resource(device.get(), SHADOW_MAP_FORMAT, SHADOW_MAP_SIZE[i], static_cast<UINT>(SHADOW_MAP_SIZE[i]), 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);
	}

	// ライトのビュープロジェクション行列のリソース
	std::array<dx12w::resource_and_state, LIGHT_VIEW_PROJ_MATRIX_NUM> lightViewProjMatrixResource{};
	{
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			lightViewProjMatrixResource[i] = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(XMMATRIX), 256));
	}

	// ポイントライトのインデックスを格納するリソース
	// TODO: create_commited_texture_resourceを使った方が良さげ
	auto pointLightIndexResource = dx12w::create_commited_buffer_resource(device.get(), sizeof(int) * MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// アンビエントオクルージョン用のリソース
	auto ambientOcclusionResource = dx12w::create_commited_texture_resource(device.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, AMBIENT_OCCLUSION_RESOURCE_WIDTH, AMBIENT_OCCLUSION_RESOURCE_HEIGHT,
		2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// ssaoするときのディスパッチの情報用のリソース
	auto ssaoDispatchDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	auto shadowMapDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(ShadowMapData<SHADOW_MAP_NUM>), 256));

	// 
	// デスクリプタヒープの作成
	// 


	// Gバッファのレンダーターゲット用のデスクリプタヒープ
	dx12w::descriptor_heap gBufferDescriptorHeapRTV{};
	{
		// アルベドカラー、法線、ワールド座標の3つ
		gBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3);

		// アルベドカラーのレンダーターゲットビュー
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(0), gBufferAlbedoColorResource.first.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, 0, 0);

		// 法線のレンダーターゲットビュー
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(1), gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 0, 0);

		// ワールド座標のレンダーターゲットビュー
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(2), gBufferWorldPositionResource.first.get(), G_BUFFER_WORLD_POSITION_FORMAT, 0, 0);
	}

	// モデルを書き込む時用のデスクリプタヒープ
	// Gbufferに書き込む時とカメラからのデプスを取得する時に使用
	dx12w::descriptor_heap modelDescriptorHeapCBVSRVUAV{};
	{
		modelDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		// 1つ目はCcameraData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));
		// 2つ目はLightData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));
		// 3つ目はModelData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(2), modelDataResource.first.get(), dx12w::alignment<UINT>(sizeof(ModelData), 256));

		// 4つ目以降はライトプロジェクション行列
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), dx12w::alignment<UINT>(sizeof(XMMATRIX), 256));
	}

	// 地面のモデルを書き込む用のデスクリプタヒープ
	// Gbufferに書き込む時とカメラからのデプスっを取得する時に使用
	dx12w::descriptor_heap groundDescriptorHeapCBVSRVUAV{};
	{
		groundDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		// 1つ目はCameraData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));
		// 2つめはLightData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));
		// 3つ目はGroundData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(2), groundDataResource.first.get(), dx12w::alignment<UINT>(sizeof(GroundData), 256));

		// 4つ目以降はライトプロジェクション行列
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), dx12w::alignment<UINT>(sizeof(XMMATRIX), 256));
	}

	// GBufferに書き込む際に使用するデプスバッファのビューを作る
	dx12w::descriptor_heap gBufferDescriptorHeapDSV{};
	{
		gBufferDescriptorHeapDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

		dx12w::create_texture2D_DSV(device.get(), gBufferDescriptorHeapDSV.get_CPU_handle(), depthBuffer.first.get(), DEPTH_BUFFER_FORMAT, 0);
	}

	// シャドウマップを作成する際に使用するデプスバッファのビューを作るよう
	dx12w::descriptor_heap descriptorHeapShadowDSV{};
	{
		descriptorHeapShadowDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SHADOW_MAP_NUM);

		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			dx12w::create_texture2D_DSV(device.get(), descriptorHeapShadowDSV.get_CPU_handle(i), shadowMapResource[i].first.get(), SHADOW_MAP_FORMAT, 0);
	}


	// ディファードレンダリングを行う際に利用するSRVなどを作成する用のデスクリプタヒープ
	dx12w::descriptor_heap defferredRenderingDescriptorHeapCBVSRVUAV{};
	{
		// カメラのデータ、ライトのデータ、アルベドカラー、法線、ワールド座標、デプス、シャドウマップ、ポイントライトインデックス, アンビエントオクルージョン, シャドウマップのデータ
		defferredRenderingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 6 + SHADOW_MAP_NUM + 3);

		// CameraData
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// LightData
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));

		// アルベドカラー
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			gBufferAlbedoColorResource.first.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, 1, 0, 0, 0.f);

		// 法線
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 1, 0, 0, 0.f);

		// ワールド座標
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(4),
			gBufferWorldPositionResource.first.get(), G_BUFFER_WORLD_POSITION_FORMAT, 1, 0, 0, 0.f);

		// デプス
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(5),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);

		// シャドウマップ
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + i),
				shadowMapResource[i].first.get(), SHADOW_MAP_SRV_FORMAT, 1, 0, 0, 0.f);

		// ポイントライトインデックス
		// FormatはUnknownじゃあないとダメだって
		dx12w::create_buffer_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM),
			pointLightIndexResource.first.get(), MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, sizeof(int), 0, D3D12_BUFFER_SRV_FLAG_NONE);

		// アンビエントオクルージョン
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM + 1),
			ambientOcclusionResource.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// シャドウマップのデータ
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM + 2),
			shadowMapDataResource.first.get(), dx12w::alignment<UINT>(sizeof(ShadowMapData<SHADOW_MAP_NUM>), 256));
	}

	// ディファードレンダリングでのライティングのレンダーターゲットのビューのデスクリプタヒープ
	dx12w::descriptor_heap defferredRenderingDescriptorHeapRTV{};
	{
		// 1つめが通常のカラー、2つ目が高輝度
		defferredRenderingDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);

		// 通常のカラー
		dx12w::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 0, 0);

		// 高輝度
		dx12w::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 0, 0);
	}


	// 高輝度をダウンサンプリングする際に利用するSRVなどを作成する用のデスクリプタヒープ
	dx12w::descriptor_heap highLuminanceDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		// 高輝度のリソースと縮小された高輝度のリソースのビューを作成する
		highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 1 + SHRINKED_HIGH_LUMINANCE_NUM);

		// 定数バッファのビュー
		dx12w::create_CBV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			highLuminanceDownSamplingDataResource.first.get(), dx12w::alignment<UINT>(sizeof(DispatchData), 256));

		// 通常の高輝度の高輝度のビュー
		dx12w::create_texture2D_SRV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		// 残りは縮小された高輝度のリソースのビューを順に作成していく
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
		{
			dx12w::create_texture2D_UAV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, nullptr, 0, 0);
		}
	}

	// メインカラーをダウンサンプリングする際に使用するシェーダリソース用のデスクリプタヒープ
	dx12w::descriptor_heap mainColorDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		mainColorDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2 + SHRINKED_MAIN_COLOR_RESOURCE_NUM);

		dx12w::create_CBV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			mainColorDownSamplingDataResource.first.get(), dx12w::alignment<UINT>(sizeof(DispatchData), 256));

		// 通常のサイズのメインカラーのシェーダリソースビュー
		dx12w::create_texture2D_SRV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// 以降は縮小されたメインカラーのシェーダリソースビュー
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
		{
			dx12w::create_texture2D_UAV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, nullptr, 0, 0);
		}
	}

	// フレームバッファのビューを作成する用のデスクリプタヒープ
	dx12w::descriptor_heap frameBufferDescriptorHeapRTV{};
	{
		frameBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_BUFFER_NUM);

		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			dx12w::create_texture2D_RTV(device.get(), frameBufferDescriptorHeapRTV.get_CPU_handle(i), frameBufferResources[i].first.get(), FRAME_BUFFER_FORMAT, 0, 0);
	}

	// ポストエフェクトに利用するSRVなどを作成する用のデスクリプタヒープ
	dx12w::descriptor_heap postEffectDescriptorHeapCBVSRVUAV{};
	{
		// 1つ目はCameraData、2つ目はLightData、3つ目はポストエフェクトのデータ、4つ目は通常のカラー、5つ目から縮小された高輝度のリソース
		// 12つ目からは縮小されたメインカラーのリソース、20つ目はデプスバッファ
		postEffectDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM + 1);

		// CameraData
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// LightData
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));

		// ポストエフェクトのデータ
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			postEffectDataResource.first.get(), dx12w::alignment<UINT>(sizeof(PostEffectData), 256));

		// メインの色
		dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// 縮小された高輝度
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		// 縮小されたメインカラー
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// デプスバッファ
		dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);
	}

	// ライトカリングに使用する定数バッファのビューなどを作成する用のディスクリプタヒープ
	dx12w::descriptor_heap lightCullingDescriptorHeapCBVSRVUAV{};
	{
		// CameraData、LightData、DepthBuffer、PointLightIndex
		lightCullingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4);

		// CameraData
		dx12w::create_CBV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// LightData
		dx12w::create_CBV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));

		// DepthBuffer
		dx12w::create_texture2D_SRV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);

		// PointLightIndex
		dx12w::create_buffer_UAV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			pointLightIndexResource.first.get(), nullptr,
			MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, sizeof(int), 0, 0, D3D12_BUFFER_UAV_FLAG_NONE);
	}

	// ssaoするときに使用するデスクリプタヒープ
	dx12w::descriptor_heap ssaoDescriptorHeapCBVSRVUAV{};
	{
		// CameraData, DispatchData, Depth, Normal, AmbientOcclusion
		ssaoDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5);

		// CameraData
		dx12w::create_CBV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// DispatchData
		dx12w::create_CBV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			ssaoDispatchDataResource.first.get(), dx12w::alignment<UINT>(sizeof(DispatchData), 256));

		// 深度バッファ
		dx12w::create_texture2D_SRV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			depthBuffer.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// 法線データ
		dx12w::create_texture2D_SRV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 1, 0, 0, 0.f);

		// AmbientOcclusion
		dx12w::create_texture2D_UAV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(4),
			ambientOcclusionResource.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, nullptr, 0, 0);
	}


	// 
	// シェーダの作成
	// 

	// モデルをGBufferに書き込むシェーダ
	// auto modelGBufferVertexShader = dx12w::compile_shader(L"ShaderFile/SimpleModel/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream modelGBufferVertexShaderCSO{ L"ShaderFile/SimpleModel/VertexShader.cso",std::ios::binary | std::ifstream::ate };
	auto modelGBufferVertexShader = dx12w::load_blob(modelGBufferVertexShaderCSO);
	modelGBufferVertexShaderCSO.close();
	// auto modelGBufferPixelShader = dx12w::compile_shader(L"ShaderFile/SimpleModel/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream modelGBufferPixelShaderCSO{ L"ShaderFile/SimpleModel/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto modelGBufferPixelShader = dx12w::load_blob(modelGBufferPixelShaderCSO);
	modelGBufferPixelShaderCSO.close();
	// auto modelShadowVertexShader = dx12w::compile_shader(L"ShaderFile/SimpleModel/ShadowVertexShader.hlsl", "main", "vs_5_0");
	std::ifstream modelShadowVertexShaderCSO{ L"ShaderFile/SimpleModel/ShadowVertexShader.cso",std::ios::binary | std::ifstream::ate };
	auto modelShadowVertexShader = dx12w::load_blob(modelShadowVertexShaderCSO);
	modelShadowVertexShaderCSO.close();

	// 地面のモデルをGbufferに書き込むシェーダ
	// auto groundGBufferVertexShader = dx12w::compile_shader(L"ShaderFile/SimpleGround/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream groundGBufferVertexShaderCSO{ L"ShaderFile/SimpleGround/VertexShader.cso",std::ios::binary | std::ifstream::ate };
	auto groundGBufferVertexShader = dx12w::load_blob(groundGBufferVertexShaderCSO);
	groundGBufferVertexShaderCSO.close();
	// auto groundGBufferPixelShader = dx12w::compile_shader(L"ShaderFile/SimpleGround/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream groundGBufferPixelShaderCSO{ L"ShaderFile/SimpleGround/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto groundGBufferPixelShader = dx12w::load_blob(groundGBufferPixelShaderCSO);
	groundGBufferPixelShaderCSO.close();
	// auto groudnShadowVertexShader = dx12w::compile_shader(L"ShaderFile/SimpleGround/ShadowVertexShader.hlsl", "main", "vs_5_0");
	std::ifstream groudnShadowVertexShaderCSO{ L"ShaderFile/SimpleGround/ShadowVertexShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto groudnShadowVertexShader = dx12w::load_blob(groudnShadowVertexShaderCSO);
	groudnShadowVertexShaderCSO.close();

	// GBUufferを利用したディファードレンダリングでのライティング用のシェーダ
	// auto deferredRenderingVertexShader = dx12w::compile_shader(L"ShaderFile/DeferredRendering/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream deferredRenderingVertexShaderCSO{ L"ShaderFile/DeferredRendering/VertexShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto deferredRenderingVertexShader = dx12w::load_blob(deferredRenderingVertexShaderCSO);
	deferredRenderingVertexShaderCSO.close();
	// auto deferredRenderingPixelShader = dx12w::compile_shader(L"ShaderFile/DeferredRendering/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream deferredRenderingPixelShaderCSO{ L"ShaderFile/DeferredRendering/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto deferredRenderingPixelShader = dx12w::load_blob(deferredRenderingPixelShaderCSO);
	deferredRenderingPixelShaderCSO.close();

	// ダウンサンプリングを行うコンピュートシェーダ
	// auto downSamplingComputeShader = dx12w::compile_shader(L"ShaderFile/DownSampling/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream downSamplingComputeShaderCSO{ L"ShaderFile/DownSampling/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto downSamplingComputeShader = dx12w::load_blob(downSamplingComputeShaderCSO);
	downSamplingComputeShaderCSO.close();

	// ポストエフェクトをかけるシェーダ
	// auto postEffectVertexShader = dx12w::compile_shader(L"ShaderFile/PostEffect/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream postEffectVertexShaderCSO{ L"ShaderFile/PostEffect/VertexShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto postEffectVertexShader = dx12w::load_blob(postEffectVertexShaderCSO);
	postEffectVertexShaderCSO.close();
	// auto postEffectPixelShader = dx12w::compile_shader(L"ShaderFile/PostEffect/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream postEffectPixelShaderCSO{ L"ShaderFile/PostEffect/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto postEffectPixelShader = dx12w::load_blob(postEffectPixelShaderCSO);
	postEffectPixelShaderCSO.close();

	// ライトカリング用のシェーダ
	// auto lightCullingComputeShader = dx12w::compile_shader(L"ShaderFile/LightCulling/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream lightCullingComputeShaderCSO{ L"ShaderFile/LightCulling/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto lightCullingComputeShader = dx12w::load_blob(lightCullingComputeShaderCSO);
	lightCullingComputeShaderCSO.close();

	// ssao用のシェーダ
	// auto ssaoComputeShader = dx12w::compile_shader(L"ShaderFile/SSAO/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream ssaoComputeShaderCSO{ L"ShaderFile/SSAO/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto ssaoComputeShader = dx12w::load_blob(ssaoComputeShaderCSO);
	ssaoComputeShaderCSO.close();


	// 
	// ルートシグネチャとパイプラインの作成
	// 

	// モデルを描写する用のルートシグネチャ
	auto modelRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*モデルデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} ,{{/*ライトのビュープロジェクション行列*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	// モデルを描写する用のグラフィックパイプライン
	auto modelGBufferGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT },
		{ {modelGBufferVertexShader.data(),modelGBufferVertexShader.size()} ,{modelGBufferPixelShader.data(),modelGBufferPixelShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// モデルの影を描写するグラフィックスパイプライン
	auto modelShdowGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } }, {}, { modelShadowVertexShader.data(),modelShadowVertexShader.size() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	// 地面を描写する用のルートシグネチャ
	auto groundRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*グランドのデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}},{{/*ライトのビュープロジェクション行列*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	// 地面を描写する用のグラフィックパイプライン
	auto groundGBufferGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT },
		{ {groundGBufferVertexShader.data(),groundGBufferVertexShader.size()},{groundGBufferPixelShader.data(),groundGBufferPixelShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// 地面の影を描画する用のグラフィクスパイプライン
	auto groundShadowGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } }, {}, { {groudnShadowVertexShader.data() ,groudnShadowVertexShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	auto deferredRenderingRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*アルベドカラー、法線、ワールド座標、デプスバッファの順*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,4},
		{/*シャドウマップ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHADOW_MAP_NUM},{/*ポイントライトのインデックスのリスト*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
		{/*アンビエントオクルージョン*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*ShadowMapData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} },
		{ { D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto deferredRenderringGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), deferredRenderingRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ MAIN_COLOR_RESOURCE_FORMAT,HIGH_LUMINANCE_FORMAT },
		{ {deferredRenderingVertexShader.data(),deferredRenderingVertexShader.size()},{deferredRenderingPixelShader.data(),deferredRenderingPixelShader.size()} }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	// 高輝度とメインカラーをダウンサンプリングする際に使用するルートシグネチャ
	auto downSamplingRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*ディスパッチの情報*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*ダウンサンプリングされる元のテクスチャ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*ダウンサンプリング*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV,SHRINKED_MAIN_COLOR_RESOURCE_NUM}} },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	// 高輝度とメインカラーをダウンサンプリングする際に使用するパイプライン
	auto downSamplingPipelineState = dx12w::create_compute_pipeline(device.get(), downSamplingRootSignature.get(), { downSamplingComputeShader.data(),downSamplingComputeShader.size() });


	auto postEffectRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*ポストエフェクトのデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*メインのカラーのテクスチャ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
		{/*縮小された高輝度のリソース*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_HIGH_LUMINANCE_NUM},{/*縮小されたメインカラーののリソース*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_MAIN_COLOR_RESOURCE_NUM},
		{/*デプスバッファ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV}} },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto postEffectGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), postEffectRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ FRAME_BUFFER_FORMAT }, { {postEffectVertexShader.data(),postEffectVertexShader.size()},{postEffectPixelShader.data(),postEffectPixelShader.size()} }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	auto lightCullingRootSignater = dx12w::create_root_signature(device.get(),
		{ { {/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*DepthBuffer*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*PointLightIndex*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV} } },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto lightCullingComputePipelineState = dx12w::create_compute_pipeline(device.get(), lightCullingRootSignater.get(), { lightCullingComputeShader.data(),lightCullingComputeShader.size() });


	auto ssaoRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData, DispatchData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV,2},{/*DepthBaffer, NormalBaffer*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,2},{/*AmbientOcclusion*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV}} },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto ssaoComputePipelineState = dx12w::create_compute_pipeline(device.get(), ssaoRootSignature.get(), { ssaoComputeShader.data(),ssaoComputeShader.size() });


	// 
	// その他定数
	// 

	D3D12_VIEWPORT viewport{ 0.f,0.f, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
	D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

	XMFLOAT3 eye{ 0.f,5.f,5.f };
	XMFLOAT3 target{ 0,0,0 };
	XMFLOAT3 up{ 0,1,0 };
	float asspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	XMFLOAT3 lightColor{ 0.4f,0.4f,0.4f };
	XMFLOAT3 lightDir{ 0.f,-0.5f,-0.5f };

	pdx12::TPS tps{};
	tps.eyeRadius = 5.f;
	tps.eyeHeight = 5.f;

	LightData lightData{};
	lightData.directionLight.dir = lightDir;
	lightData.directionLight.color = lightColor;

	lightData.pointLightNum = 800;

	for (std::size_t i = 0; i < lightData.pointLightNum; i++)
	{
		std::random_device rnd;
		std::mt19937 mt(rnd());
		std::uniform_real_distribution<float> r(0.f, 1.f);

		lightData.pointLight[i].color = {
			r(mt),
			r(mt),
			r(mt),
		};

		lightData.pointLight[i].pos = {
			40.f - (r(mt) * 2.f - 1.f) * 80.f,
			1.f,
			40.f - (r(mt) * 2.f - 1.f) * 80.f
		};

		lightData.pointLight[i].range = 2.f;
	}

	lightData.specPow = 100.f;

	PostEffectData posEffectData{
		{0.5f,0.5f},
		0.08f,
		0.6f,
		1.f,
	};

	ModelData modelData{};
	for (std::size_t i = 0; i < MODEL_HEIGHT_NUM; i++)
		for (std::size_t j = 0; j < MODEL_WIDTH_NUM; j++)
		{
			float rate = i % 2 == 0 && j % 2 == 0 ? 40.f : 20.f;
			modelData.world[i * MODEL_HEIGHT_NUM + j] = XMMatrixScaling(rate, rate, rate);
			modelData.world[i * MODEL_HEIGHT_NUM + j] *= XMMatrixTranslation(8.f * j - 4.f, 0.f, 8.f * i - 4.f);
		}


	GroundData groundData{};
	groundData.world = XMMatrixScaling(500.f, 500.f, 500.f) * XMMatrixRotationX(XM_PIDIV2) * XMMatrixTranslation(0.f, 50.f, 0.f);

	CameraData* mappedCameraDataPtr = nullptr;
	cameraDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedCameraDataPtr));

	LightData* mappedLightDataPtr = nullptr;
	lightDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedLightDataPtr));

	ModelData* mappedModelDataPtr = nullptr;
	modelDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedModelDataPtr));
	*mappedModelDataPtr = modelData;

	GroundData* mappedGroundDataPtr = nullptr;
	groundDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedGroundDataPtr));
	*mappedGroundDataPtr = groundData;

	PostEffectData* mappedPostEffectData = nullptr;
	postEffectDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedPostEffectData));
	*mappedPostEffectData = posEffectData;

	// 高輝度をダウンサンプリングする際に使用する定数バッファに渡すデータ
	DispatchData highLuminanceDownSamplingDispatchData{};
	{
		auto tileWidth = 1 << (SHRINKED_HIGH_LUMINANCE_NUM - 1);
		auto tiileHeight = 1 << (SHRINKED_HIGH_LUMINANCE_NUM - 1);
		auto dispatchX = dx12w::alignment<UINT>(HIGH_LUMINANCE_WIDTH, tileWidth) / tileWidth;
		auto dispatchY = dx12w::alignment<UINT>(HIGH_LUMINANCE_HEIGHT, tiileHeight) / tiileHeight;

		highLuminanceDownSamplingDispatchData.dispatchX = dispatchX;
		highLuminanceDownSamplingDispatchData.dispatchY = dispatchY;

		DispatchData* mappedDownSamplingData = nullptr;
		highLuminanceDownSamplingDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedDownSamplingData));
		*mappedDownSamplingData = highLuminanceDownSamplingDispatchData;
	}

	// メインカラーをダウンサンプリングする際に使用する定数バッファに渡すデータ
	DispatchData mainColorDownSamplingDispatchData{};
	{
		auto tileWidth = 1 << (SHRINKED_MAIN_COLOR_RESOURCE_NUM - 1);
		auto tiileHeight = 1 << (SHRINKED_MAIN_COLOR_RESOURCE_NUM - 1);
		auto dispatchX = dx12w::alignment<UINT>(MAIN_COLOR_RESOURCE_WIDTH, tileWidth) / tileWidth;
		auto dispatchY = dx12w::alignment<UINT>(MAIN_COLOR_RESOURCE_HEIGHT, tiileHeight) / tiileHeight;

		mainColorDownSamplingDispatchData.dispatchX = dispatchX;
		mainColorDownSamplingDispatchData.dispatchY = dispatchY;

		DispatchData* mappedDownSamplingData = nullptr;
		mainColorDownSamplingDataResource.first.get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedDownSamplingData));
		*mappedDownSamplingData = mainColorDownSamplingDispatchData;
	}

	// ssao用のディスパッチのデータ
	DispatchData ssaoDispatchData{};
	{
		// 定数としてちゃんと定義した方がイイかも
		ssaoDispatchData.dispatchX = 64;
		ssaoDispatchData.dispatchY = 64;

		// この書き方, 冗長な希ガス
		DispatchData* mappedDispatchData = nullptr;
		ssaoDispatchDataResource.first.get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedDispatchData));
		*mappedDispatchData = ssaoDispatchData;
	}

	ShadowMapData<SHADOW_MAP_NUM> shadowMapData{
		{10,50,static_cast<std::size_t>(CAMERA_FAR_Z)},
		0.001f,
		{0.001f,0.003f,0.05f},
	};

	{
		ShadowMapData<SHADOW_MAP_NUM>* mappedShadowMapData = nullptr;
		shadowMapDataResource.first.get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedShadowMapData));
		*mappedShadowMapData = shadowMapData;
	}


	// 
	// 入力関係の初期化
	// 

	auto directInput = pdx12::create_direct_input();

	/*
	pdx12::gamepad_device gamepad{};
	{
		// ゲームパッドの接続を待つ
		bool success = false;
		while (!success)
		{
			success = true;
			try {
				gamepad.initialize(directInput.get(), hwnd);
			}
			catch (std::runtime_error&)
			{
				success = false;
			}
		}
	}
	*/


	// 
	// メインループ
	// 

	auto start = std::chrono::system_clock::now();
	std::size_t frameCnt = 0;
	while (dx12w::update_window())
	{
		// 
		// 更新処理
		// 

		frameCnt++;

		// fpsの表示
		if (frameCnt % 100 == 0)
		{
			auto end = std::chrono::system_clock::now();
			auto time = end - start;
			std::cout << "fps: " << 100.f / (static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count()) / 1000.f) << std::endl;
			start = std::chrono::system_clock::now();
		}

		// それぞれのモデルの回転
		for (std::size_t i = 0; i < MODEL_HEIGHT_NUM; i++)
			for (std::size_t j = 0; j < MODEL_WIDTH_NUM; j++)
				modelData.world[i * MODEL_HEIGHT_NUM + j] *= XMMatrixTranslation(-(8.f * j - 4.f), 0.f, -(8.f * i - 4.f)) * XMMatrixRotationY(0.01f) * XMMatrixTranslation(8.f * j - 4.f, 0.f, 8.f * i - 4.f);
		*mappedModelDataPtr = modelData;

		// ゲームパッドの情報から視点を移動させる処理
		/*
		{
			auto padData = gamepad.get_state();
			pdx12::UpdateTPS(tps, padData.lY / 1000.f, padData.lX / 1000.f, -padData.lZ / 1000.f);

			tps.target.y += padData.rgbButtons[7] == 0x80 ? 1.f : 0.f;
			tps.target.y -= padData.rgbButtons[6] == 0x80 ? 1.f : 0.f;

			eye = pdx12::GetEyePosition(tps);
			target = tps.target;
		}
		*/

		// 以下, 更新された視点の情報からシェーダで使用する情報を更新する

		auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

		auto proj = DirectX::XMMatrixPerspectiveFovLH(
			VIEW_ANGLE,
			asspect,
			CAMERA_NEAR_Z,
			CAMERA_FAR_Z
		);
		auto inv = XMMatrixInverse(nullptr, view * proj);
		XMFLOAT3 right{ inv.r[0].m128_f32[0],inv.r[0].m128_f32[1] ,inv.r[0].m128_f32[2] };

		auto lightPosVector = XMLoadFloat3(&target) - XMVector3Normalize(XMLoadFloat3(&lightDir))
			* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];
		XMFLOAT3 lightPos{};
		XMStoreFloat3(&lightPos, lightPosVector);
		auto lightViewProj = XMMatrixLookAtLH(lightPosVector, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(100, 100, -100.f, 200.f);

		CameraData cameraData{};
		cameraData.view = view;
		cameraData.viewInv = XMMatrixInverse(nullptr, cameraData.view);
		cameraData.proj = proj;
		cameraData.projInv = XMMatrixInverse(nullptr, cameraData.proj);
		cameraData.viewProj = view * proj;
		cameraData.viewProjInv = XMMatrixInverse(nullptr, cameraData.viewProj);
		cameraData.cameraNear = CAMERA_NEAR_Z;
		cameraData.cameraFar = CAMERA_FAR_Z;
		cameraData.screenWidth = WINDOW_WIDTH;
		cameraData.screenHeight = WINDOW_HEIGHT;
		cameraData.eyePos = eye;

		inv = XMMatrixInverse(nullptr, view * proj);
		right = { inv.r[0].m128_f32[0],inv.r[0].m128_f32[1] ,inv.r[0].m128_f32[2] };

		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
		{
			XMMATRIX* mappedLightViewProj = nullptr;
			lightViewProjMatrixResource[i].first->Map(0, nullptr, reinterpret_cast<void**>(&mappedLightViewProj));

			std::array<XMFLOAT3, 8> vertex{};
			XMFLOAT3 cameraForward = { target.x - eye.x,target.y - eye.y, target.z - eye.z };
			pdx12::get_frustum_vertex(eye, asspect, CAMERA_NEAR_Z, static_cast<float>(shadowMapData.areaTable[i]), VIEW_ANGLE, cameraForward, right, vertex);
			for (std::size_t j = 0; j < vertex.size(); j++)
				pdx12::apply(vertex[j], lightViewProj);
			XMMATRIX clop{};
			pdx12::get_clop_matrix(vertex, clop);

			*mappedLightViewProj = lightViewProj * clop;
			lightData.directionLightViewProj[i] = lightViewProj * clop;
		}


		for (std::size_t i = 0; i < lightData.pointLightNum; i++)
		{
			lightData.pointLight[i].posInView = lightData.pointLight[i].pos;
			pdx12::apply(lightData.pointLight[i].posInView, view);
		}

		*mappedCameraDataPtr = cameraData;
		*mappedLightDataPtr = lightData;


		// 
		// 描画開始の準備
		// 

		auto backBufferIndex = swapChain->GetCurrentBackBufferIndex();


		// 
		// GBufferにモデルの描写
		// 0番目のアロケータを使用
		// 

		commandManager.reset_list(0);

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		// 全てのGBufferにバリアをかける
		dx12w::resource_barrior(commandManager.get_list(), gBufferAlbedoColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), gBufferNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), gBufferWorldPositionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// 全てのGbufferをクリアする
		// アルベドカラー
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		// 法線
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);
		// ワールド座標
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(2), zeroFloat4.data(), 0, nullptr);

		dx12w::resource_barrior(commandManager.get_list(), depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandManager.get_list()->ClearDepthStencilView(gBufferDescriptorHeapDSV.get_CPU_handle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE gBufferRenderTargetCPUHandle[] = {
			gBufferDescriptorHeapRTV.get_CPU_handle(0),// アルベドカラー
			gBufferDescriptorHeapRTV.get_CPU_handle(1),// 法線
			gBufferDescriptorHeapRTV.get_CPU_handle(2),// ワールド座標
		};
		auto depthBufferCPUHandle = gBufferDescriptorHeapDSV.get_CPU_handle(0);
		commandManager.get_list()->OMSetRenderTargets(static_cast<UINT>(std::size(gBufferRenderTargetCPUHandle)), gBufferRenderTargetCPUHandle, false, &depthBufferCPUHandle);

		// 
		// モデルの描画
		// 

		commandManager.get_list()->SetGraphicsRootSignature(modelRootSignature.get());
		{
			auto ptr = modelDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, modelDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(modelGBufferGraphicsPipelineState.get());
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &modelVertexBufferView);

		commandManager.get_list()->DrawInstanced(static_cast<UINT>(modelVertexNum), static_cast<UINT>(MODEL_NUM), 0, 0);

		// 
		// 地面の描写
		// 

		commandManager.get_list()->SetGraphicsRootSignature(groundRootSignature.get());
		{
			auto ptr = groundDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, groundDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(groundGBufferGraphicsPipelineState.get());
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(static_cast<UINT>(peraPolygonVertexNum), 1, 0, 0);

		dx12w::resource_barrior(commandManager.get_list(), gBufferAlbedoColorResource, D3D12_RESOURCE_STATE_COMMON);
		dx12w::resource_barrior(commandManager.get_list(), gBufferNormalResource, D3D12_RESOURCE_STATE_COMMON);
		dx12w::resource_barrior(commandManager.get_list(), gBufferWorldPositionResource, D3D12_RESOURCE_STATE_COMMON);
		dx12w::resource_barrior(commandManager.get_list(), depthBuffer, D3D12_RESOURCE_STATE_COMMON);

		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();


		// 
		// シャドウマップの描写
		// 1番目から1+SHDOW_MAP_NUM番目のアロケータを使用
		// 

		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
		{
			commandManager.reset_list(1 + i);

			D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(SHADOW_MAP_SIZE[i]),static_cast<float>(SHADOW_MAP_SIZE[i]),0.f,1.f };
			D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(SHADOW_MAP_SIZE[i]),static_cast<LONG>(SHADOW_MAP_SIZE[i]) };

			commandManager.get_list()->RSSetViewports(1, &viewport);
			commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

			dx12w::resource_barrior(commandManager.get_list(), shadowMapResource[i], D3D12_RESOURCE_STATE_DEPTH_WRITE);
			commandManager.get_list()->ClearDepthStencilView(descriptorHeapShadowDSV.get_CPU_handle(i), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

			auto depthBufferCPUHandle = descriptorHeapShadowDSV.get_CPU_handle(i);

			commandManager.get_list()->OMSetRenderTargets(0, nullptr, false, &depthBufferCPUHandle);

			// モデルの影
			commandManager.get_list()->SetGraphicsRootSignature(modelRootSignature.get());
			{
				auto ptr = modelDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetGraphicsRootDescriptorTable(0, modelDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetGraphicsRootDescriptorTable(1, modelDescriptorHeapCBVSRVUAV.get_GPU_handle(3 + i));

			commandManager.get_list()->SetPipelineState(modelShdowGraphicsPipelineState.get());
			commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandManager.get_list()->IASetVertexBuffers(0, 1, &modelVertexBufferView);

			commandManager.get_list()->DrawInstanced(static_cast<UINT>(modelVertexNum), static_cast<UINT>(MODEL_NUM), 0, 0);


			// 地面の影
			commandManager.get_list()->SetGraphicsRootSignature(groundRootSignature.get());
			{
				auto ptr = groundDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetGraphicsRootDescriptorTable(0, groundDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetGraphicsRootDescriptorTable(1, groundDescriptorHeapCBVSRVUAV.get_GPU_handle(3 + i));

			commandManager.get_list()->SetPipelineState(groundShadowGraphicsPipelineState.get());
			commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

			commandManager.get_list()->DrawInstanced(static_cast<UINT>(peraPolygonVertexNum), 1, 0, 0);

			dx12w::resource_barrior(commandManager.get_list(), shadowMapResource[i], D3D12_RESOURCE_STATE_COMMON);

			commandManager.get_list()->Close();
			commandManager.excute();
			commandManager.signal();
		}


		// 
		// GBufferの描画処理とシャドウマップの描画処理が終わるのをそれぞれ待つ
		// TODO: これGPUの性能的に意味なかった
		// 

		commandManager.wait(0);
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			commandManager.wait(1 + i);


		commandManager.reset_list(0);

		// 
		// ライトカリング
		// 


		dx12w::resource_barrior(commandManager.get_list(), pointLightIndexResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandManager.get_list()->SetComputeRootSignature(lightCullingRootSignater.get());
		{
			auto ptr = lightCullingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetComputeRootDescriptorTable(0, lightCullingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(lightCullingComputePipelineState.get());
		// DispatchDataの値を使用してないんか?
		commandManager.get_list()->Dispatch(dx12w::alignment<UINT>(WINDOW_WIDTH, LIGHT_CULLING_TILE_WIDTH) / LIGHT_CULLING_TILE_WIDTH,
			dx12w::alignment<UINT>(WINDOW_HEIGHT, LIGHT_CULLING_TILE_HEIGHT) / LIGHT_CULLING_TILE_HEIGHT, 1);
		dx12w::resource_barrior(commandManager.get_list(), pointLightIndexResource, D3D12_RESOURCE_STATE_COMMON);


		//
		// SSAO
		//

		dx12w::resource_barrior(commandManager.get_list(), ambientOcclusionResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandManager.get_list()->SetComputeRootSignature(ssaoRootSignature.get());
		{
			auto ptr = ssaoDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetComputeRootDescriptorTable(0, ssaoDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(ssaoComputePipelineState.get());
		commandManager.get_list()->Dispatch(ssaoDispatchData.dispatchX, ssaoDispatchData.dispatchY, 1);
		dx12w::resource_barrior(commandManager.get_list(), ambientOcclusionResource, D3D12_RESOURCE_STATE_COMMON);








		// 
		// ディファードレンダリング
		// 

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		dx12w::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// メインカラーのリソースのクリア
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		// 高輝度のリソースのクリア
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);

		// auto backBufferCPUHandle = frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE deferredRenderingRTVCPUHandle[] = {
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(0),// メインの色
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(1),// 高輝度
		};
		commandManager.get_list()->OMSetRenderTargets(static_cast<UINT>(std::size(deferredRenderingRTVCPUHandle)), deferredRenderingRTVCPUHandle, false, nullptr);

		commandManager.get_list()->SetGraphicsRootSignature(deferredRenderingRootSignature.get());
		{
			auto ptr = defferredRenderingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, defferredRenderingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(deferredRenderringGraphicsPipelineState.get());
		// LISTではない
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(static_cast<UINT>(peraPolygonVertexNum), 1, 0, 0);

		dx12w::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_COMMON);
		dx12w::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_COMMON);


		// 
		// 高輝度のダウンサンプリング
		// 

		{
			for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
				dx12w::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			commandManager.get_list()->SetComputeRootSignature(downSamplingRootSignature.get());
			{
				auto ptr = highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetComputeRootDescriptorTable(0, highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetPipelineState(downSamplingPipelineState.get());

			commandManager.get_list()->Dispatch(highLuminanceDownSamplingDispatchData.dispatchX, highLuminanceDownSamplingDispatchData.dispatchY, 1);

			for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
				dx12w::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_COMMON);
		}

		// 
		// メインカラーのダウンサンプリング
		// 

		{
			for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
				dx12w::resource_barrior(commandManager.get_list(), shrinkedMainColorResource[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			commandManager.get_list()->SetComputeRootSignature(downSamplingRootSignature.get());
			{
				auto ptr = mainColorDownSamplingDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetComputeRootDescriptorTable(0, mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetPipelineState(downSamplingPipelineState.get());

			commandManager.get_list()->Dispatch(mainColorDownSamplingDispatchData.dispatchX, mainColorDownSamplingDispatchData.dispatchY, 1);

			for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
				dx12w::resource_barrior(commandManager.get_list(), shrinkedMainColorResource[i], D3D12_RESOURCE_STATE_COMMON);
		}


		// 
		// ポストエフェクトをかけフレームバッファに描画
		// 

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		dx12w::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandManager.get_list()->ClearRenderTargetView(frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex), grayColor.data(), 0, nullptr);

		auto frameBufferCPUHandle = frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex);
		commandManager.get_list()->OMSetRenderTargets(1, &frameBufferCPUHandle, false, &depthBufferCPUHandle);

		commandManager.get_list()->SetGraphicsRootSignature(postEffectRootSignature.get());
		{
			auto ptr = postEffectDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		// ルートのハンドルはループごとにずらしサイズが１つ大きいテクスチャを参照できるようにする
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, postEffectDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(postEffectGraphicsPipelineState.get());
		// LISTではない
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(static_cast<UINT>(peraPolygonVertexNum), 1, 0, 0);

		dx12w::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_COMMON);

		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();

		commandManager.wait(0);

		swapChain->Present(1, 0);
	}

	return 0;
}
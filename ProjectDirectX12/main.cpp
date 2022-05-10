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


//カスケードシャドウマップの数
//TODO: 置く場所
constexpr std::size_t SHADOW_MAP_NUM = 3;

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


//
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
	//被写界深度を計算する際の基準となるデプスのUV座標
	XMFLOAT2 depthDiffCenter;
	//デプスの差にかかる補正の定数
	float depthDiffPower;
	//デプスの差がdepthDiffLower以下の場合ぼかさない
	float depthDiffLower;

	//高輝度をメインの色に加算するまえにかける補正値
	float luminanceDegree;

	float _pad0;
	float _pad1;
	float _pad2;
};

constexpr std::size_t MODEL_NUM = 8;
struct ModelData
{
	XMMATRIX world[MODEL_NUM];
};

struct GroundData
{
	XMMATRIX world;
};

struct DownSamplingData
{
	float dispatchX;
	float dispatchY;
};

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	constexpr std::size_t WINDOW_WIDTH = 800;
	constexpr std::size_t WINDOW_HEIGHT = 700;

	constexpr float CAMERA_NEAR_Z = 0.01f;
	constexpr float CAMERA_FAR_Z = 100.f;

	constexpr float VIEW_ANGLE = DirectX::XM_PIDIV2;

	constexpr std::size_t FRAME_BUFFER_NUM = 2;
	constexpr DXGI_FORMAT FRAME_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

	constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;
	constexpr DXGI_FORMAT DEPTH_BUFFER_SRV_FORMAT = DXGI_FORMAT_R32_FLOAT;

	//GBufferのアルベドカラーのフォーマット
	//フレームバッファと同じフォーマットにしておく
	constexpr DXGI_FORMAT G_BUFFER_ALBEDO_COLOR_FORMAT = FRAME_BUFFER_FORMAT;

	//GBufferの法線のフォーマット
	//正規化された8ビットの浮動小数4つで表現
	constexpr DXGI_FORMAT G_BUFFER_NORMAL_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

	//GBufferの位置のフォーマット
	//それぞれ32ビットにしたが助長かも
	constexpr DXGI_FORMAT G_BUFFER_WORLD_POSITION_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;

	//高輝度のファーマっと
	//フレームバッファと同じでおけ
	constexpr DXGI_FORMAT HIGH_LUMINANCE_FORMAT = FRAME_BUFFER_FORMAT;

	//高輝度のリソースのサイズ
	//ウィンドウと同じサイズでおｋ
	constexpr std::size_t HIGH_LUMINANCE_WIDTH = WINDOW_WIDTH;
	constexpr std::size_t HIGH_LUMINANCE_HEIGHT = WINDOW_HEIGHT;

	//縮小された高輝度のフォーマット
	//高輝度と同じフォーマットでおけ
	constexpr DXGI_FORMAT SHRINKED_HIGH_LUMINANCE_FORMAT = HIGH_LUMINANCE_FORMAT;

	//縮小された高輝度のリソースの数
	constexpr std::size_t SHRINKED_HIGH_LUMINANCE_NUM = 4;

	//ポストエフェクトをかける前のリソースのフォーマット
	//フレームバッファのフォーマットと同じで
	constexpr DXGI_FORMAT MAIN_COLOR_RESOURCE_FORMAT = FRAME_BUFFER_FORMAT;

	constexpr std::size_t MAIN_COLOR_RESOURCE_WIDTH = WINDOW_WIDTH;
	constexpr std::size_t MAIN_COLOR_RESOURCE_HEIGHT = WINDOW_HEIGHT;

	//縮小されダウンサンプリングされたリソースのフォーマット
	//被写界深度のポストエフェクトをかける際に使用する
	constexpr DXGI_FORMAT SHRINKED_MAIN_COLOR_RESOURCE_FORMAT = MAIN_COLOR_RESOURCE_FORMAT;

	//縮小されダウンサンプリングされたリソースのフォーマットの数
	constexpr std::size_t SHRINKED_MAIN_COLOR_RESOURCE_NUM = 4;

	//シャドウマップのフォーマット
	constexpr DXGI_FORMAT SHADOW_MAP_FORMAT = DXGI_FORMAT_D32_FLOAT;
	constexpr DXGI_FORMAT SHADOW_MAP_SRV_FORMAT = DXGI_FORMAT_R32_FLOAT;

	//カスケードシャドウマップのサイズ
	//近い順
	constexpr std::array<std::size_t, SHADOW_MAP_NUM> SHADOW_MAP_SIZE = {
		2048,
		2048,
		2048,
	};

	//シャドウマップの距離テーブル
	constexpr std::array<std::size_t, SHADOW_MAP_NUM> SHADOW_MAP_AREA_TABLE = {
		1,
		5,
		CAMERA_FAR_Z
	};

	//ライトのビュープロジェクション行列の数
	//シャドウマップと同じ数
	constexpr std::size_t LIGHT_VIEW_PROJ_MATRIX_NUM = SHADOW_MAP_NUM;

	constexpr std::size_t LIGHT_CULLING_TILE_WIDTH = 16;
	constexpr std::size_t LIGHT_CULLING_TILE_HEIGHT = 16;
	constexpr std::size_t LIGHT_CULLING_TILE_NUM = (WINDOW_WIDTH / LIGHT_CULLING_TILE_WIDTH) * (WINDOW_HEIGHT / LIGHT_CULLING_TILE_HEIGHT);

	//
	//基本的な部分の作成
	//

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	auto device = pdx12::create_device();

	pdx12::command_manager<1> commandManager{};
	commandManager.initialize(device.get());
	
	auto swapChain = pdx12::create_swap_chain(commandManager.get_queue(), hwnd, FRAME_BUFFER_FORMAT, FRAME_BUFFER_NUM);


	//
	//クリアの値などの定義
	//

	//GBufferに描写する際に使用するデプスバッファをクリアする際に使用
	D3D12_CLEAR_VALUE depthBufferClearValue{};
	depthBufferClearValue.Format = DEPTH_BUFFER_FORMAT;
	depthBufferClearValue.DepthStencil.Depth = 1.f;

	//灰色
	constexpr std::array<float, 4> grayColor{ 0.5f,0.5f,0.5f,1.f };

	//全部ゼロ
	constexpr std::array<float, 4> zeroFloat4{ 0.f,0.f,0.f,0.f };
	//全部ゼロUintver
	constexpr std::array<std::uint32_t, 4> zeroUint4{ 0,0,0,0 };


	D3D12_CLEAR_VALUE grayClearValue{};
	grayClearValue.Format = FRAME_BUFFER_FORMAT;
	std::copy(grayColor.begin(), grayColor.end(), std::begin(grayClearValue.Color));

	//ゼロでクリアするようなClearValueの取得
	auto getZeroFloat4CearValue = [&zeroFloat4](DXGI_FORMAT format) {
		D3D12_CLEAR_VALUE result{};
		result.Format = format;
		std::copy(zeroFloat4.begin(), zeroFloat4.end(), std::begin(result.Color));
		return result;
	};

	//
	//リソースの作成
	//


	//フレームバッファのリソース
	std::array<pdx12::resource_and_state, FRAME_BUFFER_NUM> frameBufferResources{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		ID3D12Resource* tmp = nullptr;
		swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&tmp));
		frameBufferResources[i] = std::make_pair(pdx12::release_unique_ptr<ID3D12Resource>{tmp}, D3D12_RESOURCE_STATE_COMMON);
	}

	//デプスバッファのリソース
	auto depthBuffer= pdx12::create_commited_texture_resource(device.get(), DEPTH_BUFFER_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT,
		2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);

	//GBUfferのアルベドカラーのリソース
	//ClearValueははフレームバッファと同じ色
	auto gBufferAlbedoColorResource = pdx12::create_commited_texture_resource(device.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	//GBufferの法線のリソース
	auto normalZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_NORMAL_FORMAT);
	auto gBufferNormalResource = pdx12::create_commited_texture_resource(device.get(), G_BUFFER_NORMAL_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &normalZeroFloat4ClearValue);

	//GBufferのワールド座標のリソース
	auto worldPositionZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_WORLD_POSITION_FORMAT);
	auto gBufferWorldPositionResource= pdx12::create_commited_texture_resource(device.get(), G_BUFFER_WORLD_POSITION_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &worldPositionZeroFloat4ClearValue);


	//モデルの頂点情報のリソースとそのビュー
	pdx12::resource_and_state modelVertexBuffer{};
	D3D12_VERTEX_BUFFER_VIEW modelVertexBufferView{};
	std::size_t modelVertexNum = 0;
	{
		objl::Loader Loader;

		bool loadout = Loader.LoadFile("3DModel/bun_zipper.obj");
		objl::Mesh mesh = Loader.LoadedMeshes[0];

		modelVertexBuffer = pdx12::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 6 * mesh.Vertices.size());
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
		modelVertexBufferView.SizeInBytes = sizeof(float) * 6 * mesh.Vertices.size();
		modelVertexBufferView.StrideInBytes = sizeof(float) * 6;

		modelVertexNum = mesh.Vertices.size();
	}

	//フレームバッファに描画する用のぺらポリゴンの頂点データのリソース
	//サイズについて座標でfloat三要素分、uvで二要素分でsize(float)*5、ペラポリゴンは四角形なのでx4
	auto peraPolygonVertexBufferResource = pdx12::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 5 * 4);
	std::size_t peraPolygonVertexNum = 4;
	{
		//floatのうちわけ-> float3: pos , float2 uv
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

	//モデルをGBufferに書き込む際のシーンのデータを扱うリソース
	//定数バッファのため256アライメントする必要がある
	auto cameraDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));

	//ライトの情報を扱うリソース
	auto lightDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));

	//モデルをGBufferに書き込む際のモデルのデータを扱うリソース
	//定数バッファのため256アライメントする必要がある
	auto modelDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(ModelData), 256));

	//地面をGbufferに書き込む際のグランドのデータを扱うリソース
	auto groundDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(GroundData), 256));

	//高輝度のリソース
	auto highLuminanceClearValue = getZeroFloat4CearValue(HIGH_LUMINANCE_FORMAT);
	auto highLuminanceResource = pdx12::create_commited_texture_resource(device.get(), HIGH_LUMINANCE_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &highLuminanceClearValue);

	//縮小された高輝度のリソース
	std::array<pdx12::resource_and_state, SHRINKED_HIGH_LUMINANCE_NUM> shrinkedHighLuminanceResource{};
	{
		std::size_t w = WINDOW_WIDTH;
		std::size_t h = WINDOW_HEIGHT;
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
		{
			w /= 2.f;
			h /= 2.f;
			shrinkedHighLuminanceResource[i] = pdx12::create_commited_texture_resource(device.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, w, h, 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, nullptr);
		}
	}

	//高輝度をダウンサンプリングする際の定数バッファのリソース
	auto highLuminanceDownSamplingDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(DownSamplingData), 256));

	//ポストエフェクトをかける前のリソース
	auto mainColorResource = pdx12::create_commited_texture_resource(device.get(), MAIN_COLOR_RESOURCE_FORMAT, MAIN_COLOR_RESOURCE_WIDTH, MAIN_COLOR_RESOURCE_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	//メインカラーをダウンサンプリングを行う際の定数バッファのリソース
	auto mainColorDownSamplingDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(DownSamplingData), 256));

	//縮小されたポストエフェクトをかける前のリソース
	//被写界深度を考慮する際に使用する
	std::array<pdx12::resource_and_state,SHRINKED_MAIN_COLOR_RESOURCE_NUM> shrinkedMainColorResource{};
	{
		auto clearValue = getZeroFloat4CearValue(SHRINKED_MAIN_COLOR_RESOURCE_FORMAT);
		std::size_t w = WINDOW_WIDTH;
		std::size_t h = WINDOW_HEIGHT;
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
		{
			w /= 2;
			h /= 2;

			shrinkedMainColorResource[i] = pdx12::create_commited_texture_resource(device.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, w, h, 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, nullptr);
		}
	}

	//ポストエフェクトの情報の定数バッファ
	auto postEffectDataResource = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(PostEffectData), 256));

	//シャドウマップのリソース
	std::array<pdx12::resource_and_state, SHADOW_MAP_NUM> shadowMapResource{};
	{
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			shadowMapResource[i] = pdx12::create_commited_texture_resource(device.get(), SHADOW_MAP_FORMAT, SHADOW_MAP_SIZE[i], SHADOW_MAP_SIZE[i], 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);
	}

	//ライトのビュープロジェクション行列のリソース
	std::array<pdx12::resource_and_state, LIGHT_VIEW_PROJ_MATRIX_NUM> lightViewProjMatrixResource{};
	{
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			lightViewProjMatrixResource[i] = pdx12::create_commited_upload_buffer_resource(device.get(), pdx12::alignment<UINT64>(sizeof(XMMATRIX), 256));
	}

	//ポイントライトのインデックスを格納するリソース
	auto pointLightIndexResource = pdx12::create_commited_buffer_resource(device.get(), sizeof(int) * MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	//
	//デスクリプタヒープの作成
	//


	//Gバッファのレンダーターゲット用のデスクリプタヒープ
	pdx12::descriptor_heap gBufferDescriptorHeapRTV{};
	{
		//アルベドカラー、法線、ワールド座標の3つ
		gBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3);

		//アルベドカラーのレンダーターゲットビュー
		pdx12::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(0), gBufferAlbedoColorResource.first.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, 0, 0);

		//法線のレンダーターゲットビュー
		pdx12::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(1), gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 0, 0);

		//ワールド座標のレンダーターゲットビュー
		pdx12::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(2), gBufferWorldPositionResource.first.get(), G_BUFFER_WORLD_POSITION_FORMAT, 0, 0);
	}

	//モデルを書き込む時用のデスクリプタヒープ
	//Gbufferに書き込む時とカメラからのデプスを取得する時に使用
	pdx12::descriptor_heap modelDescriptorHeapCBVSRVUAV{};
	{
		modelDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		//1つ目はCcameraData
		pdx12::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));
		//2つ目はLightData
		pdx12::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));
		//3つ目はModelData
		pdx12::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(2), modelDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(ModelData), 256));

		//4つ目以降はライトプロジェクション行列
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			pdx12::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), pdx12::alignment<UINT64>(sizeof(XMMATRIX), 256));
	}

	//地面のモデルを書き込む用のデスクリプタヒープ
	//Gbufferに書き込む時とカメラからのデプスっを取得する時に使用
	pdx12::descriptor_heap groundDescriptorHeapCBVSRVUAV{};
	{
		groundDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		//1つ目はCameraData
		pdx12::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));
		//2つめはLightData
		pdx12::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));
		//3つ目はGroundData
		pdx12::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(2), groundDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(GroundData), 256));

		//4つ目以降はライトプロジェクション行列
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			pdx12::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), pdx12::alignment<UINT64>(sizeof(XMMATRIX), 256));
	}

	//GBufferに書き込む際に使用するデプスバッファのビューを作る
	pdx12::descriptor_heap gBufferDescriptorHeapDSV{};
	{
		gBufferDescriptorHeapDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

		pdx12::create_texture2D_DSV(device.get(), gBufferDescriptorHeapDSV.get_CPU_handle(), depthBuffer.first.get(), DEPTH_BUFFER_FORMAT, 0);
	}

	//シャドウマップを作成する際に使用するデプスバッファのビューを作るよう
	pdx12::descriptor_heap descriptorHeapShadowDSV{};
	{
		descriptorHeapShadowDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SHADOW_MAP_NUM);

		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			pdx12::create_texture2D_DSV(device.get(), descriptorHeapShadowDSV.get_CPU_handle(i), shadowMapResource[i].first.get(), SHADOW_MAP_FORMAT, 0);
	}


	//ディファードレンダリングを行う際に利用するSRVなどを作成する用のデスクリプタヒープ
	pdx12::descriptor_heap defferredRenderingDescriptorHeapCBVSRVUAV{};
	{
		//カメラのデータ、ライトのデータ、アルベドカラー、法線、ワールド座標、シャドウマップ、ポイントライトインデックス
		defferredRenderingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5 + SHADOW_MAP_NUM + 1);

		//CameraData
		pdx12::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(0), 
			cameraDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));

		//LightData
		pdx12::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));

		//アルベドカラー
		pdx12::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(2), 
			gBufferAlbedoColorResource.first.get(),G_BUFFER_ALBEDO_COLOR_FORMAT, 1, 0, 0, 0.f);

		//法線
		pdx12::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(3), 
			gBufferNormalResource.first.get(),G_BUFFER_NORMAL_FORMAT, 1, 0, 0, 0.f);

		//ワールド座標
		pdx12::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(4), 
			gBufferWorldPositionResource.first.get(),G_BUFFER_WORLD_POSITION_FORMAT, 1, 0, 0, 0.f);

		//シャドウマップ
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			pdx12::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(5 + i),
				shadowMapResource[i].first.get(), SHADOW_MAP_SRV_FORMAT, 1, 0, 0, 0.f);

		//ポイントライトインデックス
		//FormatはUnknownじゃあないとダメだって
		pdx12::create_buffer_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(5 + SHADOW_MAP_NUM),
			pointLightIndexResource.first.get(), MAX_POINT_LIGHT_NUM* LIGHT_CULLING_TILE_NUM, sizeof(int), 0, D3D12_BUFFER_SRV_FLAG_NONE);

	}

	//ディファードレンダリングでのライティングのレンダーターゲットのビューのデスクリプタヒープ
	pdx12::descriptor_heap defferredRenderingDescriptorHeapRTV{};
	{
		//1つめが通常のカラー、2つ目が高輝度
		defferredRenderingDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);

		//通常のカラー
		pdx12::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 0, 0);
		
		//高輝度
		pdx12::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 0, 0);
	}


	//高輝度をダウンサンプリングする際に利用するSRVなどを作成する用のデスクリプタヒープ
	pdx12::descriptor_heap highLuminanceDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		//高輝度のリソースと縮小された高輝度のリソースのビューを作成する
		highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 1 + SHRINKED_HIGH_LUMINANCE_NUM);

		//定数バッファのビュー
		pdx12::create_CBV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			highLuminanceDownSamplingDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(DownSamplingData), 256));

		//通常の高輝度の高輝度のビュー
		pdx12::create_texture2D_SRV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		//残りは縮小された高輝度のリソースのビューを順に作成していく
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
		{
			pdx12::create_texture2D_UAV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, nullptr, 0, 0);
		}
	}

	//メインカラーをダウンサンプリングする際に使用するシェーダリソース用のデスクリプタヒープ
	pdx12::descriptor_heap mainColorDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		mainColorDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2 + SHRINKED_MAIN_COLOR_RESOURCE_NUM);

		pdx12::create_CBV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			mainColorDownSamplingDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(DownSamplingData), 256));

		//通常のサイズのメインカラーのシェーダリソースビュー
		pdx12::create_texture2D_SRV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		//以降は縮小されたメインカラーのシェーダリソースビュー
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
		{
			pdx12::create_texture2D_UAV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, nullptr, 0, 0);
		}
	}

	//フレームバッファのビューを作成する用のデスクリプタヒープ
	pdx12::descriptor_heap frameBufferDescriptorHeapRTV{};
	{
		frameBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_BUFFER_NUM);

		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			pdx12::create_texture2D_RTV(device.get(), frameBufferDescriptorHeapRTV.get_CPU_handle(i), frameBufferResources[i].first.get(), FRAME_BUFFER_FORMAT, 0, 0);
	}

	//ポストえっふぇくとにに利用するSRVなどを作成する用のデスクリプタヒープ
	pdx12::descriptor_heap postEffectDescriptorHeapCBVSRVUAV{};
	{
		//1つ目はCameraData、2つ目はLightData、3つ目はポストエフェクトのデータ、4つ目は通常のカラー、5つ目から縮小された高輝度のリソース
		//12つ目からは縮小されたメインカラーのリソース、20つ目はデプスバッファ
		postEffectDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM + 1);

		//CameraData
		pdx12::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));

		//LightData
		pdx12::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));

		//ポストエフェクトのデータ
		pdx12::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			postEffectDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(PostEffectData), 256));

		//メインの色
		pdx12::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		//縮小された高輝度
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
			pdx12::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		//縮小されたメインカラー
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
			pdx12::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		//デプスバッファ
		pdx12::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);
	}

	//ライトカリングに使用する定数バッファのビューなどを作成する用のディスクリプタヒープ
	pdx12::descriptor_heap lightCullingDescriptorHeapCBVSRVUAV{};
	{
		//CameraData、LightData、DepthBuffer、PointLightIndex
		lightCullingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4);

		//CameraData
		pdx12::create_CBV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(CameraData), 256));

		//LightData
		pdx12::create_CBV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), pdx12::alignment<UINT64>(sizeof(LightData), 256));

		//DepthBuffer
		pdx12::create_texture2D_SRV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);

		//PointLightIndex
		pdx12::create_buffer_UAV(device.get(), lightCullingDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			pointLightIndexResource.first.get(), nullptr,
			MAX_POINT_LIGHT_NUM* LIGHT_CULLING_TILE_NUM, sizeof(int), 0, 0, D3D12_BUFFER_UAV_FLAG_NONE);
	}


	//
	//シェーダの作成
	//

	//モデルをGBufferに書き込むシェーダ
	auto modelGBufferVertexShader = pdx12::create_shader(L"ShaderFile/SimpleModel/VertexShader.hlsl", "main", "vs_5_0");
	auto modelGBufferPixelShader = pdx12::create_shader(L"ShaderFile/SimpleModel/PixelShader.hlsl", "main", "ps_5_0");
	auto modelShadowVertexShader = pdx12::create_shader(L"ShaderFile/SimpleModel/ShadowVertexShader.hlsl", "main", "vs_5_0");

	//地面のモデルをGbufferに書き込むシェーダ
	auto groundGBufferVertexShader = pdx12::create_shader(L"ShaderFile/SimpleGround/VertexShader.hlsl", "main", "vs_5_0");
	auto groundGBufferPixelShader = pdx12::create_shader(L"ShaderFile/SimpleGround/PixelShader.hlsl", "main", "ps_5_0");
	auto groudnShadowVertexShader = pdx12::create_shader(L"ShaderFile/SimpleGround/ShadowVertexShader.hlsl", "main", "vs_5_0");

	//GBUufferを利用したディファードレンダリングでのライティング用のシェーダ
	auto deferredRenderingVertexShader = pdx12::create_shader(L"ShaderFile/DeferredRendering/VertexShader.hlsl", "main", "vs_5_0");
	auto deferredRenderingPixelShader = pdx12::create_shader(L"ShaderFile/DeferredRendering/PixelShader.hlsl", "main", "ps_5_0");

	//ダウンサンプリング用のシェーダ
	auto mainColorDownSamplingVertexShader = pdx12::create_shader(L"ShaderFile/MainColorDownSampling/VertexShader.hlsl", "main", "vs_5_0");
	auto mainColorDownSamplingPixelShader = pdx12::create_shader(L"ShaderFile/MainColorDownSampling/PixelShader.hlsl", "main", "ps_5_0");

	//ダウンサンプリングを行うコンピュートシェーダ
	auto downSamplingComputeShader = pdx12::create_shader(L"ShaderFile/DownSampling/ComputeShader.hlsl", "main", "cs_5_0");

	//ポストエフェクトをかけるシェーダ
	auto postEffectVertexShader = pdx12::create_shader(L"ShaderFile/PostEffect/VertexShader.hlsl", "main", "vs_5_0");
	auto postEffectPixelShader = pdx12::create_shader(L"ShaderFile/PostEffect/PixelShader.hlsl", "main", "ps_5_0");

	//ライトカリング用のシェーダ
	auto lightCullingComputeShader = pdx12::create_shader(L"ShaderFile/LightCulling/ComputeShader.hlsl", "main", "cs_5_0");

	//
	//ルートシグネチャとパイプラインの作成
	//

	//モデルを描写する用のルートシグネチャ
	auto modelRootSignature = pdx12::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*モデルデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} ,{{/*ライトのビュープロジェクション行列*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	//モデルを描写する用のグラフィックパイプライン
	auto modelGBufferGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT }, { modelGBufferVertexShader.get(),modelGBufferPixelShader.get() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	//モデルの影を描写するグラフィックスパイプライン
	auto modelShdowGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } }, {}, { modelShadowVertexShader.get() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	//地面を描写する用のルートシグネチャ
	auto groundRootSignature = pdx12::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*グランドのデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}},{{/*ライトのビュープロジェクション行列*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	//地面を描写する用のグラフィックパイプライン
	auto groundGBufferGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT }, { groundGBufferVertexShader.get(),groundGBufferPixelShader.get() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	//地面の影を描画する用のグラフィクスパイプライン
	auto groundShadowGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } }, {}, { groudnShadowVertexShader.get() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	auto deferredRenderingRootSignature = pdx12::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*GBuffer アルベドカラー、法線、ワールド座標の順*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,3},
		{/*シャドウマップ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHADOW_MAP_NUM},{/*ポイントライトのインデックスのリスト*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV}}},
		{ { D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto deferredRenderringGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), deferredRenderingRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ MAIN_COLOR_RESOURCE_FORMAT,HIGH_LUMINANCE_FORMAT }, { deferredRenderingVertexShader.get(),deferredRenderingPixelShader.get() }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	//高輝度とメインカラーをダウンサンプリングする際に使用するルートシグネチャ
	auto downSamplingRootSignature = pdx12::create_root_signature(device.get(),
		{ {{/*ディスパッチの情報*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*ダウンサンプリングされる元のテクスチャ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*ダウンサンプリング*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV,SHRINKED_MAIN_COLOR_RESOURCE_NUM}} },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	//高輝度とメインカラーをダウンサンプリングする際に使用するパイプライン
	auto downSamplingPipelineState = pdx12::create_compute_pipeline(device.get(), downSamplingRootSignature.get(), downSamplingComputeShader.get());


	auto postEffectRootSignature = pdx12::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*ポストエフェクトのデータ*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*メインのカラーのテクスチャ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
		{/*縮小された高輝度のリソース*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_HIGH_LUMINANCE_NUM},{/*縮小されたメインカラーののリソース*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_MAIN_COLOR_RESOURCE_NUM},
		{/*デプスバッファ*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV}}},
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto postEffectGraphicsPipelineState = pdx12::create_graphics_pipeline(device.get(), postEffectRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ FRAME_BUFFER_FORMAT }, { postEffectVertexShader.get(),postEffectPixelShader.get() }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	auto lightCullingRootSignater = pdx12::create_root_signature(device.get(),
		{ { {/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*DepthBuffer*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*PointLightIndex*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV} } },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto lightCullingComputePipelineState = pdx12::create_compute_pipeline(device.get(), lightCullingRootSignater.get(), lightCullingComputeShader.get());

	//
	//その他定数
	//

	
	D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(WINDOW_WIDTH),static_cast<float>(WINDOW_HEIGHT),0.f,1.f };
	D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(WINDOW_WIDTH),static_cast<LONG>(WINDOW_HEIGHT) };

	XMFLOAT3 eye{ 0.f,5.f,5.f };
	XMFLOAT3 target{ 0,0,0 };
	XMFLOAT3 up{ 0,1,0 };
	float asspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
	auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		VIEW_ANGLE,
		asspect,
		CAMERA_NEAR_Z,
		CAMERA_FAR_Z
	);
	XMFLOAT3 lightColor{ 0.4f,0.4f,0.4f };
	XMFLOAT3 lightDir{ 0.f,-0.5f,-0.5f };

	auto inv = XMMatrixInverse(nullptr, view * proj);
	XMFLOAT3 right{ inv.r[0].m128_f32[0],inv.r[0].m128_f32[1] ,inv.r[0].m128_f32[2] };

	XMFLOAT3 cameraForward = { target.x - eye.x,target.y - eye.y, target.z - eye.z };

	//posは原点でいいっぽい
	auto lightPosVector = XMLoadFloat3(&target) - XMVector3Normalize(XMLoadFloat3(&lightDir))
		* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];
	XMFLOAT3 lightPos{ 0,0,0 };
	XMStoreFloat3(&lightPos, lightPosVector);

	XMMATRIX lightViewProj = XMMatrixLookAtLH(lightPosVector, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(1024, 1024, -100.f, 200.f);

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

	LightData lightData{};
	lightData.directionLight.dir = lightDir;
	lightData.directionLight.color = lightColor;

	lightData.pointLight[0].color = { 0.5f,0.f,0.f };
	lightData.pointLight[0].pos = { 10.f,1.f,0.f };
	lightData.pointLight[0].posInView = lightData.pointLight[0].pos;
	pdx12::apply(lightData.pointLight[0].posInView, view);
	lightData.pointLight[0].range = 10.f;

	lightData.pointLight[1].color = { 0.f,0.5f,0.f };
	lightData.pointLight[1].pos = { 10.f,1.f,10.f };
	lightData.pointLight[1].posInView = lightData.pointLight[1].pos;
	pdx12::apply(lightData.pointLight[1].posInView, view);
	lightData.pointLight[1].range = 10.f;

	lightData.pointLight[2].color = { 0.f,0.f,0.5f };
	lightData.pointLight[2].pos = { 0.f,1.f,10.f };
	lightData.pointLight[2].posInView = lightData.pointLight[2].pos;
	pdx12::apply(lightData.pointLight[2].posInView, view);
	lightData.pointLight[2].range = 10.f;

	lightData.pointLightNum = 3;

	lightData.specPow = 100.f;


	PostEffectData posEffectData{
		{0.5f,0.5f},
		0.08f,
		0.6f,
		1.f,
	};

	ModelData modelData{};
	std::fill(std::begin(modelData.world), std::end(modelData.world), XMMatrixScaling(10.f, 10.f, 10.f));
	for (std::size_t i = 0; i < MODEL_NUM; i++)
		modelData.world[i] *= XMMatrixTranslation(0.f, 0.f, 4.f * i - 2.f);

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


	for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
	{
		XMMATRIX* mappedLightViewProj = nullptr;
		lightViewProjMatrixResource[i].first->Map(0, nullptr, reinterpret_cast<void**>(&mappedLightViewProj));

		std::array<XMFLOAT3, 8> vertex{};
		pdx12::get_frustum_vertex(eye, asspect, CAMERA_NEAR_Z, SHADOW_MAP_AREA_TABLE[i], VIEW_ANGLE, cameraForward, right, vertex);

		for (std::size_t j = 0; j < vertex.size(); j++)
			pdx12::apply(vertex[j], lightViewProj);

		XMMATRIX clop{};
		pdx12::get_clop_matrix(vertex, clop);
		*mappedLightViewProj = lightViewProj * clop;

		lightData.directionLightViewProj[i] = lightViewProj * clop;

		for (std::size_t j = 0; j < vertex.size(); j++)
			pdx12::apply(vertex[j], clop);
	}

	//高輝度をダウンサンプリングする際に使用する定数バッファに渡すデータ
	DownSamplingData highLuminanceDownSamplingData{};
	{
		auto tileWidth = 1 << (SHRINKED_HIGH_LUMINANCE_NUM - 1);
		auto tiileHeight = 1 << (SHRINKED_HIGH_LUMINANCE_NUM - 1);
		auto dispatchX = pdx12::alignment<UINT>(HIGH_LUMINANCE_WIDTH, tileWidth) / tileWidth;
		auto dispatchY = pdx12::alignment<UINT>(HIGH_LUMINANCE_HEIGHT, tiileHeight) / tiileHeight;

		highLuminanceDownSamplingData.dispatchX = dispatchX;
		highLuminanceDownSamplingData.dispatchY = dispatchY;

		DownSamplingData* mappedDownSamplingData = nullptr;
		highLuminanceDownSamplingDataResource.first->Map(0, nullptr, reinterpret_cast<void**>(&mappedDownSamplingData));
		*mappedDownSamplingData = highLuminanceDownSamplingData;
	}

	//メインカラーをダウンサンプリングする際に使用する定数バッファに渡すデータ
	DownSamplingData mainColorDownSamplingData{};
	{
		auto tileWidth = 1 << (SHRINKED_MAIN_COLOR_RESOURCE_NUM - 1);
		auto tiileHeight = 1 << (SHRINKED_MAIN_COLOR_RESOURCE_NUM - 1);
		auto dispatchX = pdx12::alignment<UINT>(MAIN_COLOR_RESOURCE_WIDTH, tileWidth) / tileWidth;
		auto dispatchY = pdx12::alignment<UINT>(MAIN_COLOR_RESOURCE_HEIGHT, tiileHeight) / tiileHeight;

		mainColorDownSamplingData.dispatchX = dispatchX;
		mainColorDownSamplingData.dispatchY = dispatchY;

		DownSamplingData* mappedDownSamplingData = nullptr;
		mainColorDownSamplingDataResource.first.get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedDownSamplingData));
		*mappedDownSamplingData = mainColorDownSamplingData;
	}


	//
	//メインループ
	//

	std::size_t cnt = 0;
	while (pdx12::update_window())
	{
		//
		//更新
		//

		for (std::size_t i = 0; i < MODEL_NUM; i++)
			modelData.world[i] *= XMMatrixTranslation(0.f, 0.f, -4.f * i + 2.f) * XMMatrixRotationY(0.01f) * XMMatrixTranslation(0.f, 0.f, 4.f * i - 2.f);
		*mappedModelDataPtr = modelData;

		
		cnt++;
		eye = { 5.f * std::sin(cnt * 0.01f),2.f,3.f * std::cos(cnt * 0.01f) };
		view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		proj = DirectX::XMMatrixPerspectiveFovLH(
			VIEW_ANGLE,
			asspect,
			CAMERA_NEAR_Z,
			CAMERA_FAR_Z
		);
		cameraForward = { target.x - eye.x,target.y - eye.y, target.z - eye.z };
		lightPosVector = XMLoadFloat3(&target) - XMVector3Normalize(XMLoadFloat3(&lightDir))
			* XMVector3Length(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye))).m128_f32[0];
		XMFLOAT3 lightPos{};
		XMStoreFloat3(&lightPos, lightPosVector);
		lightViewProj = XMMatrixLookAtLH(lightPosVector, XMLoadFloat3(&target), XMLoadFloat3(&up)) * XMMatrixOrthographicLH(100, 100, -100.f, 200.f);
		
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

		auto inv = XMMatrixInverse(nullptr, view * proj);
		XMFLOAT3 right{ inv.r[0].m128_f32[0],inv.r[0].m128_f32[1] ,inv.r[0].m128_f32[2] };

		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
		{
			XMMATRIX* mappedLightViewProj = nullptr;
			lightViewProjMatrixResource[i].first->Map(0, nullptr, reinterpret_cast<void**>(&mappedLightViewProj));

			std::array<XMFLOAT3, 8> vertex{};
			pdx12::get_frustum_vertex(eye, asspect, CAMERA_NEAR_Z, SHADOW_MAP_AREA_TABLE[i], VIEW_ANGLE, cameraForward, right, vertex);
			for (std::size_t j = 0; j < vertex.size(); j++)
				pdx12::apply(vertex[j], lightViewProj);
			XMMATRIX clop{};
			pdx12::get_clop_matrix(vertex, clop);

			*mappedLightViewProj = lightViewProj * clop;
			lightData.directionLightViewProj[i] = lightViewProj * clop;
		}
		
		lightData.pointLight[0].posInView = lightData.pointLight[0].pos;
		pdx12::apply(lightData.pointLight[0].posInView, view);
		lightData.pointLight[1].posInView = lightData.pointLight[1].pos;
		pdx12::apply(lightData.pointLight[1].posInView, view);
		lightData.pointLight[2].posInView = lightData.pointLight[2].pos;
		pdx12::apply(lightData.pointLight[2].posInView, view);
		
		*mappedCameraDataPtr = cameraData;
		*mappedLightDataPtr = lightData;

		//
		//コマンドリストの初期化など
		//

		auto backBufferIndex = swapChain->GetCurrentBackBufferIndex();

		commandManager.reset_list(0);


		//
		//GBufferにモデルの描写
		//

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);


		//全てのGBufferにバリアをかける
		pdx12::resource_barrior(commandManager.get_list(), gBufferAlbedoColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pdx12::resource_barrior(commandManager.get_list(), gBufferNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pdx12::resource_barrior(commandManager.get_list(), gBufferWorldPositionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//全てのGbufferをクリアする
		//アルベドカラー
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		//法線
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);
		//ワールド座標
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(2), zeroFloat4.data(), 0, nullptr);

		pdx12::resource_barrior(commandManager.get_list(), depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandManager.get_list()->ClearDepthStencilView(gBufferDescriptorHeapDSV.get_CPU_handle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0.f, 0, nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE gBufferRenderTargetCPUHandle[] = {
			gBufferDescriptorHeapRTV.get_CPU_handle(0),//アルベドカラー
			gBufferDescriptorHeapRTV.get_CPU_handle(1),//法線
			gBufferDescriptorHeapRTV.get_CPU_handle(2),//ワールド座標
		};
		auto depthBufferCPUHandle = gBufferDescriptorHeapDSV.get_CPU_handle(0);
		commandManager.get_list()->OMSetRenderTargets(std::size(gBufferRenderTargetCPUHandle), gBufferRenderTargetCPUHandle, false, &depthBufferCPUHandle);

		//
		//モデルの描画
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

		commandManager.get_list()->DrawInstanced(modelVertexNum, MODEL_NUM, 0, 0);

		//
		//地面の描写
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

		commandManager.get_list()->DrawInstanced(peraPolygonVertexNum, 1, 0, 0);

		//
		//
		//

		pdx12::resource_barrior(commandManager.get_list(), gBufferAlbedoColorResource, D3D12_RESOURCE_STATE_COMMON);
		pdx12::resource_barrior(commandManager.get_list(), gBufferNormalResource, D3D12_RESOURCE_STATE_COMMON);
		pdx12::resource_barrior(commandManager.get_list(), gBufferWorldPositionResource, D3D12_RESOURCE_STATE_COMMON);
		pdx12::resource_barrior(commandManager.get_list(), depthBuffer, D3D12_RESOURCE_STATE_COMMON);



		//
		//シャドウマップの描写
		//

		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
		{
			D3D12_VIEWPORT viewport{ 0,0, static_cast<float>(SHADOW_MAP_SIZE[i]),static_cast<float>(SHADOW_MAP_SIZE[i]),0.f,1.f };
			D3D12_RECT scissorRect{ 0,0,static_cast<LONG>(SHADOW_MAP_SIZE[i]),static_cast<LONG>(SHADOW_MAP_SIZE[i]) };

			commandManager.get_list()->RSSetViewports(1, &viewport);
			commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

			pdx12::resource_barrior(commandManager.get_list(), shadowMapResource[i], D3D12_RESOURCE_STATE_DEPTH_WRITE);
			commandManager.get_list()->ClearDepthStencilView(descriptorHeapShadowDSV.get_CPU_handle(i), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0.f, 0, nullptr);

			auto depthBufferCPUHandle = descriptorHeapShadowDSV.get_CPU_handle(i);

			commandManager.get_list()->OMSetRenderTargets(0, nullptr, false, &depthBufferCPUHandle);

			//モデルの影
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

			commandManager.get_list()->DrawInstanced(modelVertexNum, MODEL_NUM, 0, 0);

			
			//地面の影
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

			commandManager.get_list()->DrawInstanced(peraPolygonVertexNum, 1, 0, 0);
			
			pdx12::resource_barrior(commandManager.get_list(), shadowMapResource[i], D3D12_RESOURCE_STATE_COMMON);
		}

		//
		//ライトカリング
		//

		pdx12::resource_barrior(commandManager.get_list(), pointLightIndexResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandManager.get_list()->SetComputeRootSignature(lightCullingRootSignater.get());
		{
			auto ptr = lightCullingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetComputeRootDescriptorTable(0, lightCullingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(lightCullingComputePipelineState.get());
		commandManager.get_list()->Dispatch(pdx12::alignment(WINDOW_WIDTH, LIGHT_CULLING_TILE_WIDTH) / LIGHT_CULLING_TILE_WIDTH, pdx12::alignment(WINDOW_HEIGHT, LIGHT_CULLING_TILE_HEIGHT) / LIGHT_CULLING_TILE_HEIGHT, 1);
		pdx12::resource_barrior(commandManager.get_list(), pointLightIndexResource, D3D12_RESOURCE_STATE_COMMON);


		//
		//ディファードレンダリング
		//

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		pdx12::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pdx12::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//メインカラーのリソースのクリア
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		//高輝度のリソースのクリア
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);

		//auto backBufferCPUHandle = frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE deferredRenderingRTVCPUHandle[] = {
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(0),//メインの色
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(1),//高輝度
		};
		commandManager.get_list()->OMSetRenderTargets(std::size(deferredRenderingRTVCPUHandle), deferredRenderingRTVCPUHandle, false, nullptr);

		commandManager.get_list()->SetGraphicsRootSignature(deferredRenderingRootSignature.get());
		{
			auto ptr = defferredRenderingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, defferredRenderingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(deferredRenderringGraphicsPipelineState.get());
		//LISTではない
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(peraPolygonVertexNum, 1, 0, 0);

		pdx12::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_COMMON);
		pdx12::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_COMMON);


		//
		//高輝度のダウンサンプリング
		//

		/*
		{
			std::size_t w = WINDOW_WIDTH;
			std::size_t h = WINDOW_HEIGHT;
			for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
			{
				w /= 2.f;
				h /= 2.f;

				D3D12_VIEWPORT viewport{ 0,0, w,h,0.f,1.f };
				D3D12_RECT scissorRect{ 0,0,w,h };

				commandManager.get_list()->RSSetViewports(1, &viewport);
				commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

				pdx12::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_RENDER_TARGET);

				commandManager.get_list()->ClearRenderTargetView(highLuminanceDescriptorHeapRTV.get_CPU_handle(i), zeroFloat4.data(), 0, nullptr);

				auto renderTargetCPUHandle = highLuminanceDescriptorHeapRTV.get_CPU_handle(i);
				commandManager.get_list()->OMSetRenderTargets(1, &renderTargetCPUHandle, false, nullptr);

				commandManager.get_list()->SetGraphicsRootSignature(highLuminanceRootSignature.get());
				{
					auto ptr = highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get();
					commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
				}
				//ルートのハンドルはループごとにずらしサイズが１つ大きいテクスチャを参照できるようにする
				commandManager.get_list()->SetGraphicsRootDescriptorTable(0, highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_GPU_handle(i));
				commandManager.get_list()->SetPipelineState(highLuminanceGraphicsPipelineState.get());
				//LISTではない
				commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

				commandManager.get_list()->DrawInstanced(peraPolygonVertexNum, 1, 0, 0);

				pdx12::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_COMMON);
			}
		}
		*/

		{
			for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
				pdx12::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			commandManager.get_list()->SetComputeRootSignature(downSamplingRootSignature.get());
			{
				auto ptr = highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetComputeRootDescriptorTable(0, highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetPipelineState(downSamplingPipelineState.get());

			commandManager.get_list()->Dispatch(highLuminanceDownSamplingData.dispatchX, highLuminanceDownSamplingData.dispatchY, 1);

			for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
				pdx12::resource_barrior(commandManager.get_list(), shrinkedHighLuminanceResource[i], D3D12_RESOURCE_STATE_COMMON);
		}

		//
		//メインカラーのダウンサンプリング
		//

		{
			for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
				pdx12::resource_barrior(commandManager.get_list(), shrinkedMainColorResource[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			commandManager.get_list()->SetComputeRootSignature(downSamplingRootSignature.get());
			{
				auto ptr = mainColorDownSamplingDescriptorHeapCBVSRVUAV.get();
				commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
			}
			commandManager.get_list()->SetComputeRootDescriptorTable(0, mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
			commandManager.get_list()->SetPipelineState(downSamplingPipelineState.get());

			commandManager.get_list()->Dispatch(mainColorDownSamplingData.dispatchX, mainColorDownSamplingData.dispatchY, 1);

			for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
				pdx12::resource_barrior(commandManager.get_list(), shrinkedMainColorResource[i], D3D12_RESOURCE_STATE_COMMON);
		}

		//
		//ポストエフェクトをかけフレームバッファに描画
		//

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		pdx12::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandManager.get_list()->ClearRenderTargetView(frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex), grayColor.data(), 0, nullptr);

		auto frameBufferCPUHandle = frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex);
		commandManager.get_list()->OMSetRenderTargets(1, &frameBufferCPUHandle, false, &depthBufferCPUHandle);

		commandManager.get_list()->SetGraphicsRootSignature(postEffectRootSignature.get());
		{
			auto ptr = postEffectDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		//ルートのハンドルはループごとにずらしサイズが１つ大きいテクスチャを参照できるようにする
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, postEffectDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(postEffectGraphicsPipelineState.get());
		//LISTではない
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(peraPolygonVertexNum, 1, 0, 0);

		pdx12::resource_barrior(commandManager.get_list(), frameBufferResources[backBufferIndex], D3D12_RESOURCE_STATE_COMMON);

		//
		//コマンドの発行など
		//

		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();
		commandManager.wait(0);

		swapChain->Present(1, 0);
	}
	
	return 0;
}
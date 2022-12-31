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
// �萔
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

// GBuffer�̃A���x�h�J���[�̃t�H�[�}�b�g
// �t���[���o�b�t�@�Ɠ����t�H�[�}�b�g�ɂ��Ă���
constexpr DXGI_FORMAT G_BUFFER_ALBEDO_COLOR_FORMAT = FRAME_BUFFER_FORMAT;

// GBuffer�̖@���̃t�H�[�}�b�g
// ���K�����ꂽ8�r�b�g�̕�������4�ŕ\��
constexpr DXGI_FORMAT G_BUFFER_NORMAL_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

// GBuffer�̈ʒu�̃t�H�[�}�b�g
// ���ꂼ��32�r�b�g�ɂ�������������
constexpr DXGI_FORMAT G_BUFFER_WORLD_POSITION_FORMAT = DXGI_FORMAT_R32G32B32A32_FLOAT;

// ���P�x�̃t�@�[�}����
// �t���[���o�b�t�@�Ɠ����ł���
constexpr DXGI_FORMAT HIGH_LUMINANCE_FORMAT = FRAME_BUFFER_FORMAT;

// ���P�x�̃��\�[�X�̃T�C�Y
// �E�B���h�E�Ɠ����T�C�Y�ł���
constexpr std::size_t HIGH_LUMINANCE_WIDTH = WINDOW_WIDTH;
constexpr std::size_t HIGH_LUMINANCE_HEIGHT = WINDOW_HEIGHT;

// �k�����ꂽ���P�x�̃t�H�[�}�b�g
// ���P�x�Ɠ����t�H�[�}�b�g�ł���
constexpr DXGI_FORMAT SHRINKED_HIGH_LUMINANCE_FORMAT = HIGH_LUMINANCE_FORMAT;

// �k�����ꂽ���P�x�̃��\�[�X�̐�
constexpr std::size_t SHRINKED_HIGH_LUMINANCE_NUM = 4;

// �|�X�g�G�t�F�N�g��������O�̃��\�[�X�̃t�H�[�}�b�g
// �t���[���o�b�t�@�̃t�H�[�}�b�g�Ɠ�����
constexpr DXGI_FORMAT MAIN_COLOR_RESOURCE_FORMAT = FRAME_BUFFER_FORMAT;

constexpr std::size_t MAIN_COLOR_RESOURCE_WIDTH = WINDOW_WIDTH;
constexpr std::size_t MAIN_COLOR_RESOURCE_HEIGHT = WINDOW_HEIGHT;

// �k������_�E���T���v�����O���ꂽ���\�[�X�̃t�H�[�}�b�g
// ��ʊE�[�x�̃|�X�g�G�t�F�N�g��������ۂɎg�p����
constexpr DXGI_FORMAT SHRINKED_MAIN_COLOR_RESOURCE_FORMAT = MAIN_COLOR_RESOURCE_FORMAT;

// �k������_�E���T���v�����O���ꂽ���\�[�X�̃t�H�[�}�b�g�̐�
constexpr std::size_t SHRINKED_MAIN_COLOR_RESOURCE_NUM = 4;

// �V���h�E�}�b�v�̃t�H�[�}�b�g
constexpr DXGI_FORMAT SHADOW_MAP_FORMAT = DXGI_FORMAT_D32_FLOAT;
constexpr DXGI_FORMAT SHADOW_MAP_SRV_FORMAT = DXGI_FORMAT_R32_FLOAT;

// �J�X�P�[�h�V���h�E�}�b�v�̃T�C�Y
// �߂���
constexpr std::array<std::size_t, SHADOW_MAP_NUM> SHADOW_MAP_SIZE = {
	2048,
	2048,
	2048,
};


// ����, SHADOW_MAP_NUM�̒l���ς�ƃp�b�L���O���ꂻ��
template<std::size_t SHADOW_MAP_NUM>
struct ShadowMapData
{
	// �V���h�E�}�b�v�̋����e�[�u��
	std::array<float, SHADOW_MAP_NUM> areaTable{};
	float poissonDiskSampleRadius;
	// �V�F�[�_�ŉe�ɂȂ��Ă��邩�𔻒肷��ۂɎg�p����o�C�A�X
	// �ꉞCPU���Őݒ�ł���悤�ɂ��Ă���
	std::array<float, SHADOW_MAP_NUM> biasTanle{};
};

// ���C�g�̃r���[�v���W�F�N�V�����s��̐�
// �V���h�E�}�b�v�Ɠ�����
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
	// ��ʊE�[�x���v�Z����ۂ̊�ƂȂ�f�v�X��UV���W
	XMFLOAT2 depthDiffCenter;
	// �f�v�X�̍��ɂ�����␳�̒萔
	float depthDiffPower;
	// �f�v�X�̍���depthDiffLower�ȉ��̏ꍇ�ڂ����Ȃ�
	float depthDiffLower;

	// ���P�x�����C���̐F�ɉ��Z����܂��ɂ�����␳�l
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
	// �N���A�̒l�Ȃǂ̒�`
	// 

	// GBuffer�ɕ`�ʂ���ۂɎg�p����f�v�X�o�b�t�@���N���A����ۂɎg�p
	D3D12_CLEAR_VALUE depthBufferClearValue{};
	depthBufferClearValue.Format = DEPTH_BUFFER_FORMAT;
	depthBufferClearValue.DepthStencil.Depth = 1.f;

	// �D�F
	constexpr std::array<float, 4> grayColor{ 0.5f,0.5f,0.5f,1.f };
	D3D12_CLEAR_VALUE grayClearValue{};
	grayClearValue.Format = FRAME_BUFFER_FORMAT;
	std::copy(grayColor.begin(), grayColor.end(), std::begin(grayClearValue.Color));

	// �S���[��
	constexpr std::array<float, 4> zeroFloat4{ 0.f,0.f,0.f,0.f };
	// �[���ŃN���A����悤��ClearValue�𐶐����郉���_��
	auto getZeroFloat4CearValue = [&zeroFloat4](DXGI_FORMAT format) {
		D3D12_CLEAR_VALUE result{};
		result.Format = format;
		std::copy(zeroFloat4.begin(), zeroFloat4.end(), std::begin(result.Color));
		return result;
	};


	// 
	// ���\�[�X�̍쐬
	// 

	// �t���[���o�b�t�@�̃��\�[�X
	std::array<dx12w::resource_and_state, FRAME_BUFFER_NUM> frameBufferResources{};
	for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		ID3D12Resource* tmp = nullptr;
		swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&tmp));
		frameBufferResources[i] = std::make_pair(dx12w::release_unique_ptr<ID3D12Resource>{tmp}, D3D12_RESOURCE_STATE_COMMON);
	}

	// �f�v�X�o�b�t�@�̃��\�[�X
	auto depthBuffer = dx12w::create_commited_texture_resource(device.get(), DEPTH_BUFFER_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT,
		2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);

	// GBUffer�̃A���x�h�J���[�̃��\�[�X
	// ClearValue�͂̓t���[���o�b�t�@�Ɠ����F
	auto gBufferAlbedoColorResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	// GBuffer�̖@���̃��\�[�X
	auto normalZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_NORMAL_FORMAT);
	auto gBufferNormalResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_NORMAL_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &normalZeroFloat4ClearValue);

	// GBuffer�̃��[���h���W�̃��\�[�X
	auto worldPositionZeroFloat4ClearValue = getZeroFloat4CearValue(G_BUFFER_WORLD_POSITION_FORMAT);
	auto gBufferWorldPositionResource = dx12w::create_commited_texture_resource(device.get(), G_BUFFER_WORLD_POSITION_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &worldPositionZeroFloat4ClearValue);


	// ���f���̒��_���̃��\�[�X�Ƃ��̃r���[
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

	// �t���[���o�b�t�@�ɕ`�悷��p�̂؂�|���S���̒��_�f�[�^�̃��\�[�X
	// �T�C�Y�ɂ��č��W��float�O�v�f���Auv�œ�v�f����size(float)*5�A�y���|���S���͎l�p�`�Ȃ̂�x4
	auto peraPolygonVertexBufferResource = dx12w::create_commited_upload_buffer_resource(device.get(), sizeof(float) * 5 * 4);
	std::size_t peraPolygonVertexNum = 4;
	{
		// float�̂����킯-> float3: pos , float2 uv
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

	// ���f����GBuffer�ɏ������ލۂ̃V�[���̃f�[�^���������\�[�X
	// �萔�o�b�t�@�̂���256�A���C�����g����K�v������
	auto cameraDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(CameraData), 256));

	// ���C�g�̏����������\�[�X
	auto lightDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(LightData), 256));

	// ���f����GBuffer�ɏ������ލۂ̃��f���̃f�[�^���������\�[�X
	// �萔�o�b�t�@�̂���256�A���C�����g����K�v������
	auto modelDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(ModelData), 256));

	// �n�ʂ�Gbuffer�ɏ������ލۂ̃O�����h�̃f�[�^���������\�[�X
	auto groundDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(GroundData), 256));

	// ���P�x�̃��\�[�X
	auto highLuminanceClearValue = getZeroFloat4CearValue(HIGH_LUMINANCE_FORMAT);
	auto highLuminanceResource = dx12w::create_commited_texture_resource(device.get(), HIGH_LUMINANCE_FORMAT, WINDOW_WIDTH, WINDOW_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &highLuminanceClearValue);

	// �k�����ꂽ���P�x�̃��\�[�X
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

	// ���P�x���_�E���T���v�����O����ۂ̒萔�o�b�t�@�̃��\�[�X
	auto highLuminanceDownSamplingDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	// �|�X�g�G�t�F�N�g��������O�̃��\�[�X
	auto mainColorResource = dx12w::create_commited_texture_resource(device.get(), MAIN_COLOR_RESOURCE_FORMAT, MAIN_COLOR_RESOURCE_WIDTH, MAIN_COLOR_RESOURCE_HEIGHT, 2,
		1, 1, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &grayClearValue);

	// ���C���J���[���_�E���T���v�����O���s���ۂ̒萔�o�b�t�@�̃��\�[�X
	auto mainColorDownSamplingDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	// �k�����ꂽ�|�X�g�G�t�F�N�g��������O�̃��\�[�X
	// ��ʊE�[�x���l������ۂɎg�p����
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

	// �|�X�g�G�t�F�N�g�̏��̒萔�o�b�t�@
	auto postEffectDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(PostEffectData), 256));

	// �V���h�E�}�b�v�̃��\�[�X
	std::array<dx12w::resource_and_state, SHADOW_MAP_NUM> shadowMapResource{};
	{
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			shadowMapResource[i] = dx12w::create_commited_texture_resource(device.get(), SHADOW_MAP_FORMAT, SHADOW_MAP_SIZE[i], static_cast<UINT>(SHADOW_MAP_SIZE[i]), 2,
				1, 1, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &depthBufferClearValue);
	}

	// ���C�g�̃r���[�v���W�F�N�V�����s��̃��\�[�X
	std::array<dx12w::resource_and_state, LIGHT_VIEW_PROJ_MATRIX_NUM> lightViewProjMatrixResource{};
	{
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			lightViewProjMatrixResource[i] = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(XMMATRIX), 256));
	}

	// �|�C���g���C�g�̃C���f�b�N�X���i�[���郊�\�[�X
	// TODO: create_commited_texture_resource���g���������ǂ���
	auto pointLightIndexResource = dx12w::create_commited_buffer_resource(device.get(), sizeof(int) * MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// �A���r�G���g�I�N���[�W�����p�̃��\�[�X
	auto ambientOcclusionResource = dx12w::create_commited_texture_resource(device.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, AMBIENT_OCCLUSION_RESOURCE_WIDTH, AMBIENT_OCCLUSION_RESOURCE_HEIGHT,
		2, 1, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// ssao����Ƃ��̃f�B�X�p�b�`�̏��p�̃��\�[�X
	auto ssaoDispatchDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(DispatchData), 256));

	auto shadowMapDataResource = dx12w::create_commited_upload_buffer_resource(device.get(), dx12w::alignment<UINT64>(sizeof(ShadowMapData<SHADOW_MAP_NUM>), 256));

	// 
	// �f�X�N���v�^�q�[�v�̍쐬
	// 


	// G�o�b�t�@�̃����_�[�^�[�Q�b�g�p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap gBufferDescriptorHeapRTV{};
	{
		// �A���x�h�J���[�A�@���A���[���h���W��3��
		gBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3);

		// �A���x�h�J���[�̃����_�[�^�[�Q�b�g�r���[
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(0), gBufferAlbedoColorResource.first.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, 0, 0);

		// �@���̃����_�[�^�[�Q�b�g�r���[
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(1), gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 0, 0);

		// ���[���h���W�̃����_�[�^�[�Q�b�g�r���[
		dx12w::create_texture2D_RTV(device.get(), gBufferDescriptorHeapRTV.get_CPU_handle(2), gBufferWorldPositionResource.first.get(), G_BUFFER_WORLD_POSITION_FORMAT, 0, 0);
	}

	// ���f�����������ގ��p�̃f�X�N���v�^�q�[�v
	// Gbuffer�ɏ������ގ��ƃJ��������̃f�v�X���擾���鎞�Ɏg�p
	dx12w::descriptor_heap modelDescriptorHeapCBVSRVUAV{};
	{
		modelDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		// 1�ڂ�CcameraData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));
		// 2�ڂ�LightData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));
		// 3�ڂ�ModelData
		dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(2), modelDataResource.first.get(), dx12w::alignment<UINT>(sizeof(ModelData), 256));

		// 4�ڈȍ~�̓��C�g�v���W�F�N�V�����s��
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			dx12w::create_CBV(device.get(), modelDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), dx12w::alignment<UINT>(sizeof(XMMATRIX), 256));
	}

	// �n�ʂ̃��f�����������ޗp�̃f�X�N���v�^�q�[�v
	// Gbuffer�ɏ������ގ��ƃJ��������̃f�v�X�����擾���鎞�Ɏg�p
	dx12w::descriptor_heap groundDescriptorHeapCBVSRVUAV{};
	{
		groundDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3 + LIGHT_VIEW_PROJ_MATRIX_NUM);

		// 1�ڂ�CameraData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(0), cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));
		// 2�߂�LightData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(1), lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));
		// 3�ڂ�GroundData
		dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(2), groundDataResource.first.get(), dx12w::alignment<UINT>(sizeof(GroundData), 256));

		// 4�ڈȍ~�̓��C�g�v���W�F�N�V�����s��
		for (std::size_t i = 0; i < LIGHT_VIEW_PROJ_MATRIX_NUM; i++)
			dx12w::create_CBV(device.get(), groundDescriptorHeapCBVSRVUAV.get_CPU_handle(3 + i), lightViewProjMatrixResource[i].first.get(), dx12w::alignment<UINT>(sizeof(XMMATRIX), 256));
	}

	// GBuffer�ɏ������ލۂɎg�p����f�v�X�o�b�t�@�̃r���[�����
	dx12w::descriptor_heap gBufferDescriptorHeapDSV{};
	{
		gBufferDescriptorHeapDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

		dx12w::create_texture2D_DSV(device.get(), gBufferDescriptorHeapDSV.get_CPU_handle(), depthBuffer.first.get(), DEPTH_BUFFER_FORMAT, 0);
	}

	// �V���h�E�}�b�v���쐬����ۂɎg�p����f�v�X�o�b�t�@�̃r���[�����悤
	dx12w::descriptor_heap descriptorHeapShadowDSV{};
	{
		descriptorHeapShadowDSV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, SHADOW_MAP_NUM);

		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			dx12w::create_texture2D_DSV(device.get(), descriptorHeapShadowDSV.get_CPU_handle(i), shadowMapResource[i].first.get(), SHADOW_MAP_FORMAT, 0);
	}


	// �f�B�t�@�[�h�����_�����O���s���ۂɗ��p����SRV�Ȃǂ��쐬����p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap defferredRenderingDescriptorHeapCBVSRVUAV{};
	{
		// �J�����̃f�[�^�A���C�g�̃f�[�^�A�A���x�h�J���[�A�@���A���[���h���W�A�f�v�X�A�V���h�E�}�b�v�A�|�C���g���C�g�C���f�b�N�X, �A���r�G���g�I�N���[�W����, �V���h�E�}�b�v�̃f�[�^
		defferredRenderingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 6 + SHADOW_MAP_NUM + 3);

		// CameraData
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// LightData
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));

		// �A���x�h�J���[
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			gBufferAlbedoColorResource.first.get(), G_BUFFER_ALBEDO_COLOR_FORMAT, 1, 0, 0, 0.f);

		// �@��
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 1, 0, 0, 0.f);

		// ���[���h���W
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(4),
			gBufferWorldPositionResource.first.get(), G_BUFFER_WORLD_POSITION_FORMAT, 1, 0, 0, 0.f);

		// �f�v�X
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(5),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);

		// �V���h�E�}�b�v
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + i),
				shadowMapResource[i].first.get(), SHADOW_MAP_SRV_FORMAT, 1, 0, 0, 0.f);

		// �|�C���g���C�g�C���f�b�N�X
		// Format��Unknown���Ⴀ�Ȃ��ƃ_��������
		dx12w::create_buffer_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM),
			pointLightIndexResource.first.get(), MAX_POINT_LIGHT_NUM * LIGHT_CULLING_TILE_NUM, sizeof(int), 0, D3D12_BUFFER_SRV_FLAG_NONE);

		// �A���r�G���g�I�N���[�W����
		dx12w::create_texture2D_SRV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM + 1),
			ambientOcclusionResource.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// �V���h�E�}�b�v�̃f�[�^
		dx12w::create_CBV(device.get(), defferredRenderingDescriptorHeapCBVSRVUAV.get_CPU_handle(6 + SHADOW_MAP_NUM + 2),
			shadowMapDataResource.first.get(), dx12w::alignment<UINT>(sizeof(ShadowMapData<SHADOW_MAP_NUM>), 256));
	}

	// �f�B�t�@�[�h�����_�����O�ł̃��C�e�B���O�̃����_�[�^�[�Q�b�g�̃r���[�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap defferredRenderingDescriptorHeapRTV{};
	{
		// 1�߂��ʏ�̃J���[�A2�ڂ����P�x
		defferredRenderingDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);

		// �ʏ�̃J���[
		dx12w::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 0, 0);

		// ���P�x
		dx12w::create_texture2D_RTV(device.get(), defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 0, 0);
	}


	// ���P�x���_�E���T���v�����O����ۂɗ��p����SRV�Ȃǂ��쐬����p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap highLuminanceDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		// ���P�x�̃��\�[�X�Ək�����ꂽ���P�x�̃��\�[�X�̃r���[���쐬����
		highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 1 + SHRINKED_HIGH_LUMINANCE_NUM);

		// �萔�o�b�t�@�̃r���[
		dx12w::create_CBV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			highLuminanceDownSamplingDataResource.first.get(), dx12w::alignment<UINT>(sizeof(DispatchData), 256));

		// �ʏ�̍��P�x�̍��P�x�̃r���[
		dx12w::create_texture2D_SRV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			highLuminanceResource.first.get(), HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		// �c��͏k�����ꂽ���P�x�̃��\�[�X�̃r���[�����ɍ쐬���Ă���
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
		{
			dx12w::create_texture2D_UAV(device.get(), highLuminanceDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, nullptr, 0, 0);
		}
	}

	// ���C���J���[���_�E���T���v�����O����ۂɎg�p����V�F�[�_���\�[�X�p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap mainColorDownSamplingDescriptorHeapCBVSRVUAV{};
	{
		mainColorDownSamplingDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2 + SHRINKED_MAIN_COLOR_RESOURCE_NUM);

		dx12w::create_CBV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			mainColorDownSamplingDataResource.first.get(), dx12w::alignment<UINT>(sizeof(DispatchData), 256));

		// �ʏ�̃T�C�Y�̃��C���J���[�̃V�F�[�_���\�[�X�r���[
		dx12w::create_texture2D_SRV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// �ȍ~�͏k�����ꂽ���C���J���[�̃V�F�[�_���\�[�X�r���[
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
		{
			dx12w::create_texture2D_UAV(device.get(), mainColorDownSamplingDescriptorHeapCBVSRVUAV.get_CPU_handle(2 + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, nullptr, 0, 0);
		}
	}

	// �t���[���o�b�t�@�̃r���[���쐬����p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap frameBufferDescriptorHeapRTV{};
	{
		frameBufferDescriptorHeapRTV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_BUFFER_NUM);

		for (std::size_t i = 0; i < FRAME_BUFFER_NUM; i++)
			dx12w::create_texture2D_RTV(device.get(), frameBufferDescriptorHeapRTV.get_CPU_handle(i), frameBufferResources[i].first.get(), FRAME_BUFFER_FORMAT, 0, 0);
	}

	// �|�X�g�G�t�F�N�g�ɗ��p����SRV�Ȃǂ��쐬����p�̃f�X�N���v�^�q�[�v
	dx12w::descriptor_heap postEffectDescriptorHeapCBVSRVUAV{};
	{
		// 1�ڂ�CameraData�A2�ڂ�LightData�A3�ڂ̓|�X�g�G�t�F�N�g�̃f�[�^�A4�ڂ͒ʏ�̃J���[�A5�ڂ���k�����ꂽ���P�x�̃��\�[�X
		// 12�ڂ���͏k�����ꂽ���C���J���[�̃��\�[�X�A20�ڂ̓f�v�X�o�b�t�@
		postEffectDescriptorHeapCBVSRVUAV.initialize(device.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM + 1);

		// CameraData
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(0),
			cameraDataResource.first.get(), dx12w::alignment<UINT>(sizeof(CameraData), 256));

		// LightData
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(1),
			lightDataResource.first.get(), dx12w::alignment<UINT>(sizeof(LightData), 256));

		// �|�X�g�G�t�F�N�g�̃f�[�^
		dx12w::create_CBV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			postEffectDataResource.first.get(), dx12w::alignment<UINT>(sizeof(PostEffectData), 256));

		// ���C���̐F
		dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			mainColorResource.first.get(), MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// �k�����ꂽ���P�x
		for (std::size_t i = 0; i < SHRINKED_HIGH_LUMINANCE_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + i),
				shrinkedHighLuminanceResource[i].first.get(), SHRINKED_HIGH_LUMINANCE_FORMAT, 1, 0, 0, 0.f);

		// �k�����ꂽ���C���J���[
		for (std::size_t i = 0; i < SHRINKED_MAIN_COLOR_RESOURCE_NUM; i++)
			dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + i),
				shrinkedMainColorResource[i].first.get(), SHRINKED_MAIN_COLOR_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// �f�v�X�o�b�t�@
		dx12w::create_texture2D_SRV(device.get(), postEffectDescriptorHeapCBVSRVUAV.get_CPU_handle(4 + SHRINKED_HIGH_LUMINANCE_NUM + SHRINKED_MAIN_COLOR_RESOURCE_NUM),
			depthBuffer.first.get(), DEPTH_BUFFER_SRV_FORMAT, 1, 0, 0, 0.f);
	}

	// ���C�g�J�����O�Ɏg�p����萔�o�b�t�@�̃r���[�Ȃǂ��쐬����p�̃f�B�X�N���v�^�q�[�v
	dx12w::descriptor_heap lightCullingDescriptorHeapCBVSRVUAV{};
	{
		// CameraData�ALightData�ADepthBuffer�APointLightIndex
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

	// ssao����Ƃ��Ɏg�p����f�X�N���v�^�q�[�v
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

		// �[�x�o�b�t�@
		dx12w::create_texture2D_SRV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(2),
			depthBuffer.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, 1, 0, 0, 0.f);

		// �@���f�[�^
		dx12w::create_texture2D_SRV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(3),
			gBufferNormalResource.first.get(), G_BUFFER_NORMAL_FORMAT, 1, 0, 0, 0.f);

		// AmbientOcclusion
		dx12w::create_texture2D_UAV(device.get(), ssaoDescriptorHeapCBVSRVUAV.get_CPU_handle(4),
			ambientOcclusionResource.first.get(), AMBIENT_OCCLUSION_RESOURCE_FORMAT, nullptr, 0, 0);
	}


	// 
	// �V�F�[�_�̍쐬
	// 

	// ���f����GBuffer�ɏ������ރV�F�[�_
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

	// �n�ʂ̃��f����Gbuffer�ɏ������ރV�F�[�_
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

	// GBUuffer�𗘗p�����f�B�t�@�[�h�����_�����O�ł̃��C�e�B���O�p�̃V�F�[�_
	// auto deferredRenderingVertexShader = dx12w::compile_shader(L"ShaderFile/DeferredRendering/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream deferredRenderingVertexShaderCSO{ L"ShaderFile/DeferredRendering/VertexShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto deferredRenderingVertexShader = dx12w::load_blob(deferredRenderingVertexShaderCSO);
	deferredRenderingVertexShaderCSO.close();
	// auto deferredRenderingPixelShader = dx12w::compile_shader(L"ShaderFile/DeferredRendering/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream deferredRenderingPixelShaderCSO{ L"ShaderFile/DeferredRendering/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto deferredRenderingPixelShader = dx12w::load_blob(deferredRenderingPixelShaderCSO);
	deferredRenderingPixelShaderCSO.close();

	// �_�E���T���v�����O���s���R���s���[�g�V�F�[�_
	// auto downSamplingComputeShader = dx12w::compile_shader(L"ShaderFile/DownSampling/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream downSamplingComputeShaderCSO{ L"ShaderFile/DownSampling/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto downSamplingComputeShader = dx12w::load_blob(downSamplingComputeShaderCSO);
	downSamplingComputeShaderCSO.close();

	// �|�X�g�G�t�F�N�g��������V�F�[�_
	// auto postEffectVertexShader = dx12w::compile_shader(L"ShaderFile/PostEffect/VertexShader.hlsl", "main", "vs_5_0");
	std::ifstream postEffectVertexShaderCSO{ L"ShaderFile/PostEffect/VertexShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto postEffectVertexShader = dx12w::load_blob(postEffectVertexShaderCSO);
	postEffectVertexShaderCSO.close();
	// auto postEffectPixelShader = dx12w::compile_shader(L"ShaderFile/PostEffect/PixelShader.hlsl", "main", "ps_5_0");
	std::ifstream postEffectPixelShaderCSO{ L"ShaderFile/PostEffect/PixelShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto postEffectPixelShader = dx12w::load_blob(postEffectPixelShaderCSO);
	postEffectPixelShaderCSO.close();

	// ���C�g�J�����O�p�̃V�F�[�_
	// auto lightCullingComputeShader = dx12w::compile_shader(L"ShaderFile/LightCulling/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream lightCullingComputeShaderCSO{ L"ShaderFile/LightCulling/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto lightCullingComputeShader = dx12w::load_blob(lightCullingComputeShaderCSO);
	lightCullingComputeShaderCSO.close();

	// ssao�p�̃V�F�[�_
	// auto ssaoComputeShader = dx12w::compile_shader(L"ShaderFile/SSAO/ComputeShader.hlsl", "main", "cs_5_0");
	std::ifstream ssaoComputeShaderCSO{ L"ShaderFile/SSAO/ComputeShader.cso" ,std::ios::binary | std::ifstream::ate };
	auto ssaoComputeShader = dx12w::load_blob(ssaoComputeShaderCSO);
	ssaoComputeShaderCSO.close();


	// 
	// ���[�g�V�O�l�`���ƃp�C�v���C���̍쐬
	// 

	// ���f����`�ʂ���p�̃��[�g�V�O�l�`��
	auto modelRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*���f���f�[�^*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} ,{{/*���C�g�̃r���[�v���W�F�N�V�����s��*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	// ���f����`�ʂ���p�̃O���t�B�b�N�p�C�v���C��
	auto modelGBufferGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT },
		{ {modelGBufferVertexShader.data(),modelGBufferVertexShader.size()} ,{modelGBufferPixelShader.data(),modelGBufferPixelShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// ���f���̉e��`�ʂ���O���t�B�b�N�X�p�C�v���C��
	auto modelShdowGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), modelRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "NORMAL",DXGI_FORMAT_R32G32B32_FLOAT } }, {}, { modelShadowVertexShader.data(),modelShadowVertexShader.size() }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	// �n�ʂ�`�ʂ���p�̃��[�g�V�O�l�`��
	auto groundRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*�O�����h�̃f�[�^*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}},{{/*���C�g�̃r���[�v���W�F�N�V�����s��*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} }, {});

	// �n�ʂ�`�ʂ���p�̃O���t�B�b�N�p�C�v���C��
	auto groundGBufferGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ G_BUFFER_ALBEDO_COLOR_FORMAT,G_BUFFER_NORMAL_FORMAT,G_BUFFER_WORLD_POSITION_FORMAT },
		{ {groundGBufferVertexShader.data(),groundGBufferVertexShader.size()},{groundGBufferPixelShader.data(),groundGBufferPixelShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// �n�ʂ̉e��`�悷��p�̃O���t�B�N�X�p�C�v���C��
	auto groundShadowGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), groundRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } }, {}, { {groudnShadowVertexShader.data() ,groudnShadowVertexShader.size()} }
	, true, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	auto deferredRenderingRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*�A���x�h�J���[�A�@���A���[���h���W�A�f�v�X�o�b�t�@�̏�*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,4},
		{/*�V���h�E�}�b�v*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHADOW_MAP_NUM},{/*�|�C���g���C�g�̃C���f�b�N�X�̃��X�g*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
		{/*�A���r�G���g�I�N���[�W����*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*ShadowMapData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}} },
		{ { D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	auto deferredRenderringGraphicsPipelineState = dx12w::create_graphics_pipeline(device.get(), deferredRenderingRootSignature.get(),
		{ { "POSITION",DXGI_FORMAT_R32G32B32_FLOAT },{ "TEXCOOD",DXGI_FORMAT_R32G32_FLOAT } },
		{ MAIN_COLOR_RESOURCE_FORMAT,HIGH_LUMINANCE_FORMAT },
		{ {deferredRenderingVertexShader.data(),deferredRenderingVertexShader.size()},{deferredRenderingPixelShader.data(),deferredRenderingPixelShader.size()} }
	, false, false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);


	// ���P�x�ƃ��C���J���[���_�E���T���v�����O����ۂɎg�p���郋�[�g�V�O�l�`��
	auto downSamplingRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*�f�B�X�p�b�`�̏��*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*�_�E���T���v�����O����錳�̃e�N�X�`��*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},{/*�_�E���T���v�����O*/D3D12_DESCRIPTOR_RANGE_TYPE_UAV,SHRINKED_MAIN_COLOR_RESOURCE_NUM}} },
		{ {D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_COMPARISON_FUNC_NEVER} });

	// ���P�x�ƃ��C���J���[���_�E���T���v�����O����ۂɎg�p����p�C�v���C��
	auto downSamplingPipelineState = dx12w::create_compute_pipeline(device.get(), downSamplingRootSignature.get(), { downSamplingComputeShader.data(),downSamplingComputeShader.size() });


	auto postEffectRootSignature = dx12w::create_root_signature(device.get(),
		{ {{/*CameraData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*LightData*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV},{/*�|�X�g�G�t�F�N�g�̃f�[�^*/D3D12_DESCRIPTOR_RANGE_TYPE_CBV}, {/*���C���̃J���[�̃e�N�X�`��*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV},
		{/*�k�����ꂽ���P�x�̃��\�[�X*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_HIGH_LUMINANCE_NUM},{/*�k�����ꂽ���C���J���[�̂̃��\�[�X*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV,SHRINKED_MAIN_COLOR_RESOURCE_NUM},
		{/*�f�v�X�o�b�t�@*/D3D12_DESCRIPTOR_RANGE_TYPE_SRV}} },
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
	// ���̑��萔
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

	// ���P�x���_�E���T���v�����O����ۂɎg�p����萔�o�b�t�@�ɓn���f�[�^
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

	// ���C���J���[���_�E���T���v�����O����ۂɎg�p����萔�o�b�t�@�ɓn���f�[�^
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

	// ssao�p�̃f�B�X�p�b�`�̃f�[�^
	DispatchData ssaoDispatchData{};
	{
		// �萔�Ƃ��Ă����ƒ�`���������C�C����
		ssaoDispatchData.dispatchX = 64;
		ssaoDispatchData.dispatchY = 64;

		// ���̏�����, �璷�Ȋ�K�X
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
	// ���͊֌W�̏�����
	// 

	auto directInput = pdx12::create_direct_input();

	/*
	pdx12::gamepad_device gamepad{};
	{
		// �Q�[���p�b�h�̐ڑ���҂�
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
	// ���C�����[�v
	// 

	auto start = std::chrono::system_clock::now();
	std::size_t frameCnt = 0;
	while (dx12w::update_window())
	{
		// 
		// �X�V����
		// 

		frameCnt++;

		// fps�̕\��
		if (frameCnt % 100 == 0)
		{
			auto end = std::chrono::system_clock::now();
			auto time = end - start;
			std::cout << "fps: " << 100.f / (static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count()) / 1000.f) << std::endl;
			start = std::chrono::system_clock::now();
		}

		// ���ꂼ��̃��f���̉�]
		for (std::size_t i = 0; i < MODEL_HEIGHT_NUM; i++)
			for (std::size_t j = 0; j < MODEL_WIDTH_NUM; j++)
				modelData.world[i * MODEL_HEIGHT_NUM + j] *= XMMatrixTranslation(-(8.f * j - 4.f), 0.f, -(8.f * i - 4.f)) * XMMatrixRotationY(0.01f) * XMMatrixTranslation(8.f * j - 4.f, 0.f, 8.f * i - 4.f);
		*mappedModelDataPtr = modelData;

		// �Q�[���p�b�h�̏�񂩂王�_���ړ������鏈��
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

		// �ȉ�, �X�V���ꂽ���_�̏�񂩂�V�F�[�_�Ŏg�p��������X�V����

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
		// �`��J�n�̏���
		// 

		auto backBufferIndex = swapChain->GetCurrentBackBufferIndex();


		// 
		// GBuffer�Ƀ��f���̕`��
		// 0�Ԗڂ̃A���P�[�^���g�p
		// 

		commandManager.reset_list(0);

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		// �S�Ă�GBuffer�Ƀo���A��������
		dx12w::resource_barrior(commandManager.get_list(), gBufferAlbedoColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), gBufferNormalResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), gBufferWorldPositionResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// �S�Ă�Gbuffer���N���A����
		// �A���x�h�J���[
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		// �@��
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);
		// ���[���h���W
		commandManager.get_list()->ClearRenderTargetView(gBufferDescriptorHeapRTV.get_CPU_handle(2), zeroFloat4.data(), 0, nullptr);

		dx12w::resource_barrior(commandManager.get_list(), depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandManager.get_list()->ClearDepthStencilView(gBufferDescriptorHeapDSV.get_CPU_handle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE gBufferRenderTargetCPUHandle[] = {
			gBufferDescriptorHeapRTV.get_CPU_handle(0),// �A���x�h�J���[
			gBufferDescriptorHeapRTV.get_CPU_handle(1),// �@��
			gBufferDescriptorHeapRTV.get_CPU_handle(2),// ���[���h���W
		};
		auto depthBufferCPUHandle = gBufferDescriptorHeapDSV.get_CPU_handle(0);
		commandManager.get_list()->OMSetRenderTargets(static_cast<UINT>(std::size(gBufferRenderTargetCPUHandle)), gBufferRenderTargetCPUHandle, false, &depthBufferCPUHandle);

		// 
		// ���f���̕`��
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
		// �n�ʂ̕`��
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
		// �V���h�E�}�b�v�̕`��
		// 1�Ԗڂ���1+SHDOW_MAP_NUM�Ԗڂ̃A���P�[�^���g�p
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

			// ���f���̉e
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


			// �n�ʂ̉e
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
		// GBuffer�̕`�揈���ƃV���h�E�}�b�v�̕`�揈�����I���̂����ꂼ��҂�
		// TODO: ����GPU�̐��\�I�ɈӖ��Ȃ�����
		// 

		commandManager.wait(0);
		for (std::size_t i = 0; i < SHADOW_MAP_NUM; i++)
			commandManager.wait(1 + i);


		commandManager.reset_list(0);

		// 
		// ���C�g�J�����O
		// 


		dx12w::resource_barrior(commandManager.get_list(), pointLightIndexResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandManager.get_list()->SetComputeRootSignature(lightCullingRootSignater.get());
		{
			auto ptr = lightCullingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetComputeRootDescriptorTable(0, lightCullingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(lightCullingComputePipelineState.get());
		// DispatchData�̒l���g�p���ĂȂ���?
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
		// �f�B�t�@�[�h�����_�����O
		// 

		commandManager.get_list()->RSSetViewports(1, &viewport);
		commandManager.get_list()->RSSetScissorRects(1, &scissorRect);

		dx12w::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_RENDER_TARGET);
		dx12w::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// ���C���J���[�̃��\�[�X�̃N���A
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(0), grayColor.data(), 0, nullptr);
		// ���P�x�̃��\�[�X�̃N���A
		commandManager.get_list()->ClearRenderTargetView(defferredRenderingDescriptorHeapRTV.get_CPU_handle(1), zeroFloat4.data(), 0, nullptr);

		// auto backBufferCPUHandle = frameBufferDescriptorHeapRTV.get_CPU_handle(backBufferIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE deferredRenderingRTVCPUHandle[] = {
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(0),// ���C���̐F
			defferredRenderingDescriptorHeapRTV.get_CPU_handle(1),// ���P�x
		};
		commandManager.get_list()->OMSetRenderTargets(static_cast<UINT>(std::size(deferredRenderingRTVCPUHandle)), deferredRenderingRTVCPUHandle, false, nullptr);

		commandManager.get_list()->SetGraphicsRootSignature(deferredRenderingRootSignature.get());
		{
			auto ptr = defferredRenderingDescriptorHeapCBVSRVUAV.get();
			commandManager.get_list()->SetDescriptorHeaps(1, &ptr);
		}
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, defferredRenderingDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(deferredRenderringGraphicsPipelineState.get());
		// LIST�ł͂Ȃ�
		commandManager.get_list()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		commandManager.get_list()->IASetVertexBuffers(0, 1, &peraPolygonVertexBufferView);

		commandManager.get_list()->DrawInstanced(static_cast<UINT>(peraPolygonVertexNum), 1, 0, 0);

		dx12w::resource_barrior(commandManager.get_list(), mainColorResource, D3D12_RESOURCE_STATE_COMMON);
		dx12w::resource_barrior(commandManager.get_list(), highLuminanceResource, D3D12_RESOURCE_STATE_COMMON);


		// 
		// ���P�x�̃_�E���T���v�����O
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
		// ���C���J���[�̃_�E���T���v�����O
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
		// �|�X�g�G�t�F�N�g�������t���[���o�b�t�@�ɕ`��
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
		// ���[�g�̃n���h���̓��[�v���Ƃɂ��炵�T�C�Y���P�傫���e�N�X�`�����Q�Ƃł���悤�ɂ���
		commandManager.get_list()->SetGraphicsRootDescriptorTable(0, postEffectDescriptorHeapCBVSRVUAV.get_GPU_handle(0));
		commandManager.get_list()->SetPipelineState(postEffectGraphicsPipelineState.get());
		// LIST�ł͂Ȃ�
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
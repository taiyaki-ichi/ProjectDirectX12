#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{
	using namespace DirectX;

	std::pair<D3D12_TEXTURE_COPY_LOCATION, D3D12_TEXTURE_COPY_LOCATION> get_texture_copy_location(ID3D12Device* device, ID3D12Resource* srcResource, ID3D12Resource* dstResource)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		D3D12_TEXTURE_COPY_LOCATION dstLocation{};

		auto const dstDesc = dstResource->GetDesc();
		auto const width = dstDesc.Width;
		auto const height = dstDesc.Height;
		auto const uploadResourceRowPitch = srcResource->GetDesc().Width / height;

		srcLocation.pResource = srcResource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
			UINT nrow;
			UINT64 rowsize, size;
			device->GetCopyableFootprints(&dstDesc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
			srcLocation.PlacedFootprint = footprint;
		}
		srcLocation.PlacedFootprint.Offset = 0;
		srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(width);
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(uploadResourceRowPitch);
		srcLocation.PlacedFootprint.Footprint.Format = dstDesc.Format;

		dstLocation.pResource = dstResource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		return { std::move(srcLocation),std::move(dstLocation) };
	}


	void get_frustum_vertex(XMFLOAT3 const& eye, float asspect, float nearZ, float farZ, float viewAngle, XMFLOAT3 const& cameraForward, XMFLOAT3 const& cameraRight, std::array<XMFLOAT3, 8>& result)
	{

		auto cameraRightVector = XMVector3Normalize(XMLoadFloat3(&cameraRight));
		auto cameraForwrdVector = XMVector3Normalize(XMLoadFloat3(&cameraForward));
		auto cameraUpVector = -XMVector3Normalize(XMVector3Cross(cameraRightVector, cameraForwrdVector));

		// 視推台の手前の面の中心の点
		XMFLOAT3 nearClipCenterPos = { eye.x + cameraForward.x * nearZ,eye.y + cameraForward.y * nearZ,eye.z + cameraForward.z * nearZ };
		auto nearClipCenterPosVector = XMLoadFloat3(&nearClipCenterPos);

		// 視推台の奥の面の中心の点
		XMFLOAT3 farClipCenterPos = { eye.x + cameraForward.x * farZ,eye.y + cameraForward.y * farZ,eye.z + cameraForward.z * farZ };
		auto farClipCenterPosVector = XMLoadFloat3(&farClipCenterPos);

		// 視推台の手前の面の縦横の辺の半分の大きさ
		float nearY = std::tanf(viewAngle * 0.5f) * nearZ;
		float nearX = nearY * asspect;

		// 視推台の奥の面の縦横の辺の半分の大きさ
		float farY = std::tanf(viewAngle * 0.5f) * farZ;
		float farX = farY * asspect;


		// 計算結果のキャッシュ用
		XMVECTOR tmpResult{};

		// 面を構成する点の方向を示す
		constexpr std::array<float, 4> pointDirX{ 1.0,-1.0,-1.0,1.0 };
		constexpr std::array<float, 4> pointDirY{ 1.0,1.0,-1.0,-1.0 };

		// 視推台の手前の面を構成する4点
		for (std::size_t i = 0; i < 4; i++)
		{
			tmpResult = nearClipCenterPosVector + cameraRightVector * nearX * pointDirX[i] + cameraUpVector * nearY * pointDirY[i];
			XMStoreFloat3(&result[i], tmpResult);
		}

		// 視推台の奥の面を構成する4点
		for (std::size_t i = 0; i < 4; i++)
		{
			tmpResult = farClipCenterPosVector + cameraRightVector * farX * pointDirX[i] + cameraUpVector * farY * pointDirY[i];
			XMStoreFloat3(&result[i + 4], tmpResult);
		}
	}

	void apply(XMFLOAT3& float3, XMMATRIX const& matrix)
	{
		auto float3Vector = XMLoadFloat3(&float3);
		auto result = XMVector3Transform(float3Vector, matrix);
		XMStoreFloat3(&float3, result);
	}

	void get_clop_matrix(std::array<XMFLOAT3, 8> const& float3, XMMATRIX& matrix)
	{
		auto [minXIter, maxXIter] = std::minmax_element(float3.begin(), float3.end(),
			[](auto const& a, auto const& b) {return a.x < b.x; });

		auto [minYIter, maxYIter] = std::minmax_element(float3.begin(), float3.end(),
			[](auto const& a, auto const& b) {return a.y < b.y; });

		// 大きさを[-1,1]x[-1,1]に合わせる時に使用
		auto scaleX = 1.f / (maxXIter->x - minXIter->x);
		auto scaleY = 1.f / (maxYIter->y - minYIter->y);

		// 位置を原点に移動させる時に使用
		auto offsetX = (maxXIter->x + minXIter->x) * -0.25f * scaleX;
		auto offsetY = (maxYIter->y + minYIter->y) * -0.25f * scaleY;

		matrix = XMMatrixScaling(scaleX, scaleY, 1.f) * XMMatrixTranslation(offsetX, offsetY, 0.f);
	}
}
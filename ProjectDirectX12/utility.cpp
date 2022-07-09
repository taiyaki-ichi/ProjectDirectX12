#include"utility.hpp"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

namespace pdx12
{
	using namespace DirectX;

	void throw_exception(char const* fileName, int line, char const* func, char const* str)
	{
		std::stringstream ss{};
		ss << fileName << " , " << line << " , " << func << " : " << str << "\n";
		throw std::runtime_error{ ss.str() };
	}

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


	//視錐台の8頂点の導出
	void get_frustum_vertex(XMFLOAT3 const& eye, float asspect, float nearZ, float farZ, float viewAngle, XMFLOAT3 const& cameraForward, XMFLOAT3 const& cameraRight, std::array<XMFLOAT3, 8>& result)
	{
		XMVECTOR tmpResult{};

		auto cameraRightVector = XMVector3Normalize(XMLoadFloat3(&cameraRight));
		auto cameraForwrdVector = XMVector3Normalize(XMLoadFloat3(&cameraForward));
		auto cameraUpVector = -XMVector3Normalize(XMVector3Cross(cameraRightVector, cameraForwrdVector));

		XMFLOAT3 nearClipCenterPos = { eye.x + cameraForward.x * nearZ,eye.y + cameraForward.y * nearZ,eye.z + cameraForward.z * nearZ };
		auto nearClipCenterPosVector = XMLoadFloat3(&nearClipCenterPos);

		XMFLOAT3 farClipCenterPos = { eye.x + cameraForward.x * farZ,eye.y + cameraForward.y * farZ,eye.z + cameraForward.z * farZ };
		auto farClipCenterPosVector = XMLoadFloat3(&farClipCenterPos);

		float nearY = std::tanf(viewAngle * 0.5f) * nearZ;
		float nearX = nearY * asspect;

		float farY = std::tanf(viewAngle * 0.5f) * farZ;
		float farX = farY * asspect;


		tmpResult = nearClipCenterPosVector + cameraUpVector * nearY + cameraRightVector * nearX;
		XMStoreFloat3(&result[0], tmpResult);

		tmpResult = nearClipCenterPosVector + cameraUpVector * nearY + cameraRightVector * -nearX;
		XMStoreFloat3(&result[1], tmpResult);

		tmpResult = nearClipCenterPosVector + cameraUpVector * -nearY + cameraRightVector * -nearX;
		XMStoreFloat3(&result[2], tmpResult);

		tmpResult = nearClipCenterPosVector + cameraUpVector * -nearY + cameraRightVector * nearX;
		XMStoreFloat3(&result[3], tmpResult);


		tmpResult = farClipCenterPosVector + cameraUpVector * farY + cameraRightVector * farX;
		XMStoreFloat3(&result[4], tmpResult);

		tmpResult = farClipCenterPosVector + cameraUpVector * farY + cameraRightVector * -farX;
		XMStoreFloat3(&result[5], tmpResult);

		tmpResult = farClipCenterPosVector + cameraUpVector * -farY + cameraRightVector * -farX;
		XMStoreFloat3(&result[6], tmpResult);

		tmpResult = farClipCenterPosVector + cameraUpVector * -farY + cameraRightVector * farX;
		XMStoreFloat3(&result[7], tmpResult);
	}

	//行列をかける
	void apply(XMFLOAT3& float3, XMMATRIX const& matrix)
	{
		auto float3Vector = XMLoadFloat3(&float3);
		auto result = XMVector3Transform(float3Vector, matrix);
		XMStoreFloat3(&float3, result);
	}

	//クロップ行列を求める
	void get_clop_matrix(std::array<XMFLOAT3, 8> const& float3, XMMATRIX& matrix)
	{
		auto [minXIter, maxXIter] = std::minmax_element(float3.begin(), float3.end(),
			[](auto const& a, auto const& b) {return a.x < b.x; });

		auto [minYIter, maxYIter] = std::minmax_element(float3.begin(), float3.end(),
			[](auto const& a, auto const& b) {return a.y < b.y; });

		auto scaleX = 1.f / (maxXIter->x - minXIter->x);
		auto scaleY = 1.f / (maxYIter->y - minYIter->y);

		auto offsetX = (maxXIter->x + minXIter->x) * -0.25f * scaleX;
		auto offsetY = (maxYIter->y + minYIter->y) * -0.25f * scaleY;

		matrix = XMMatrixScaling(scaleX, scaleY, 1.f) * XMMatrixTranslation(offsetX, offsetY, 0.f);
	}
}
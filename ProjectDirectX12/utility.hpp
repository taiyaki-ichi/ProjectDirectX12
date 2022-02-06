#pragma once
#include<stdexcept>
#include<sstream>
#include<memory>
#include<utility>

namespace pdx12
{
	inline void throw_exception(char const* fileName, int line, char const* func, char const* str)
	{
		std::stringstream ss{};
		ss << fileName << " , " << line << " , " << func << " : " << str << "\n";
		throw std::runtime_error{ ss.str() };
	}

#define THROW_PDX12_EXCEPTION(s)	throw_exception(__FILE__,__LINE__,__func__,s);



	template<typename T>
	struct release_deleter {
		void operator()(T* ptr) {
			ptr->Release();
		}
	};

	template<typename T>
	using release_unique_ptr = std::unique_ptr<T, release_deleter<T>>;


	//ñﬂÇËílÇÕsrcÅAdstÇÃèá
	inline std::pair<D3D12_TEXTURE_COPY_LOCATION, D3D12_TEXTURE_COPY_LOCATION> get_texture_copy_location(ID3D12Device* device, ID3D12Resource* srcResource, ID3D12Resource* dstResource)
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
		srcLocation.PlacedFootprint.Footprint.Width = width;
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadResourceRowPitch;
		srcLocation.PlacedFootprint.Footprint.Format = dstDesc.Format;

		dstLocation.pResource = dstResource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;
	}

	template<typename T>
	inline constexpr T alignment(T size, T alignment) {
		return size + alignment - size % alignment;
	}

}
#include"create_direct_input.hpp"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace pdx12
{
	dx12w::release_unique_ptr<IDirectInput8W> create_direct_input()
	{
		IDirectInput8W* tmp = nullptr;

		HRESULT hr = DirectInput8Create(
			GetModuleHandle(nullptr),
			DIRECTINPUT_VERSION,
			IID_IDirectInput8,
			(void**)&tmp,
			NULL);
		if (FAILED(hr))
		{
			dx12w::THROW_PDX12_EXCEPTION("failed create_direct_input");
		}

		return dx12w::release_unique_ptr<IDirectInput8W>{ tmp };
	}
}
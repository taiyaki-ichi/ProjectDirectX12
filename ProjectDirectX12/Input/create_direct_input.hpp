#pragma once
#include<dinput.h>
#include"../../external/directx12-wrapper/dx12w/dx12w_utility.hpp"

namespace pdx12
{
	dx12w::release_unique_ptr<IDirectInput8W> create_direct_input();
}
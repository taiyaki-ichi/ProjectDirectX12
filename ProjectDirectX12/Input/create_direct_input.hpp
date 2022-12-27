#pragma once
#include<dinput.h>
#include"utility.hpp"

namespace pdx12
{
	dx12w::release_unique_ptr<IDirectInput8W> create_direct_input();
}
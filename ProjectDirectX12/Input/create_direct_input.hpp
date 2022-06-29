#pragma once
#include<dinput.h>
#include"../utility.hpp"

namespace pdx12
{
	release_unique_ptr<IDirectInput8W> create_direct_input();
}
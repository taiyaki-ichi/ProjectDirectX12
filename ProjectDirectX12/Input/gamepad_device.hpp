#pragma once
#include<dinput.h>
namespace pdx12
{
	class gamepad_device
	{
		LPDIRECTINPUTDEVICE8 m_device = nullptr;

	public:
		void initialize(IDirectInput8W*, HWND);

		//��
		DIJOYSTATE get_state();
	};
}
#pragma once
#include<dinput.h>
#include<array>
#include"button_state.hpp"

namespace pdx12
{

	enum class key_type
	{
		Q = 0x10,
		W = 0x11,
		E = 0x12,
		R = 0x13,
		T = 0x14,
		Y = 0x15,
		U = 0x16,
		I = 0x17,
		O = 0x18,
		P = 0x19,
		A = 0x1E,
		S = 0x1F,
		D = 0x20,
		F = 0x21,
		G = 0x22,
		H = 0x23,
		J = 0x24,
		K = 0x25,
		L = 0x26,
		Z = 0x2C,
		X = 0x2D,
		C = 0x2E,
		V = 0x2F,
		B = 0x30,
		N = 0x31,
		M = 0x32,

		no1 = 0x02,
		no2 = 0x03,
		no3 = 0x04,
		no4 = 0x05,
		no5 = 0x06,
		no6 = 0x07,
		no7 = 0x08,
		no8 = 0x09,
		no9 = 0x0A,
		no0 = 0x0B,

		Esc = 0x01,
		BackSpace = 0x0E,
		Tab = 0x0F,
		Enter = 0x1C,
		LeftCtrl = 0x1D,
		LeftShift = 0x2A,
		RightShift = 0x36,
		RightCtrl = 0x9D,
		Space = 0x39,

		Up = 0xC8,
		Left = 0xCB,
		Right = 0xCD,
		Down = 0xD0,

		Comma = 0x33,
		Period = 0x34,

		Slash = 0x35,

	};

	class keyboard_device
	{
		LPDIRECTINPUTDEVICE8 m_keyboard_device = nullptr;

		std::array<unsigned char, 256> m_curr_state{};
		std::array<unsigned char, 256> m_prev_state{};

	public:
		void initialize(IDirectInput8W*, HWND);

		button_state get_key_state(key_type);

		void update();
	};

}
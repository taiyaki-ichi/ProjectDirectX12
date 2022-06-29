#include"keyboard_device.hpp"
#include"../utility.hpp"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace pdx12
{
	void keyboard_device::initialize(IDirectInput8W* directInput, HWND hwnd)
	{
		if (FAILED(directInput->CreateDevice(GUID_SysKeyboard, &m_device, NULL)))
		{
			THROW_PDX12_EXCEPTION("failed keyboard_device::initialize CreateDevice");
		}

		// デバイスのフォーマットの設定
		if (FAILED(m_device->SetDataFormat(&c_dfDIKeyboard)))
		{
			THROW_PDX12_EXCEPTION("failed keyboard_device::initialize SetDataFormat");
		}

		// 協調モードの設定
		if (FAILED(m_device->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		{
			THROW_PDX12_EXCEPTION("failed keyboard_device::initialize SetCooperativeLevel");
		}

		// デバイスの取得開始
		if (FAILED(m_device->Acquire()))
		{
			THROW_PDX12_EXCEPTION("failed keyboard_device::initialize Acquire");
		}

		//初期化
		m_device->GetDeviceState(256, m_curr_state.data());
		std::copy(m_curr_state.begin(), m_curr_state.end(), m_prev_state.begin());
	}

	button_state keyboard_device::get_key_state(key_type keyType)
	{
		if (m_prev_state[static_cast<int>(keyType)] & 0x80)
		{
			if (m_curr_state[static_cast<int>(keyType)] & 0x80)
				return button_state::Held;
			else
				return button_state::Released;
		}
		else
		{
			if (m_curr_state[static_cast<int>(keyType)] & 0x80)
				return button_state::Pressed;
			else
				return button_state::None;
		}
	}

	void keyboard_device::update()
	{
		std::copy(m_curr_state.begin(), m_curr_state.end(), m_prev_state.begin());
		m_device->GetDeviceState(256, m_curr_state.data());
	}
}
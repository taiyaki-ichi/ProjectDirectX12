#include"mouse_device.hpp"
#include"../utility.hpp"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace pdx12
{
	bool start_mouse_control(LPDIRECTINPUTDEVICE8 device);


	void mouse_device::initialize(IDirectInput8W* directInput, HWND hwnd)
	{
		if (FAILED(directInput->CreateDevice(GUID_SysMouse, &m_device, NULL)))
		{
			THROW_PDX12_EXCEPTION("failed mouse_device::initialize CreateDevice");
		}

		//  入力フォーマットの指定
		if (FAILED(m_device->SetDataFormat(&c_dfDIMouse)))
		{
			THROW_PDX12_EXCEPTION("failed mouse_device::initialize SetDataFormat");
		}

		//  協調モードの設定
		if (FAILED(m_device->SetCooperativeLevel(hwnd,DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		{
			THROW_PDX12_EXCEPTION("failed mouse_device::initialize SetCooperativeLevel");
		}

		int count = 0;
		// MAX_CNT回まで挑戦する
		constexpr int MAX_CNT = 5;
		//  制御開始
		while (start_mouse_control(m_device) == false)
		{
			Sleep(100);
			count++;

			if (count >= MAX_CNT)
			{
				// THROW_PDX12_EXCEPTION("failed mouse_device::initialize start_mouse_control");
				break;
			}
		}

		m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_curr_state);
		m_prev_state = m_curr_state;
	}


	void mouse_device::update()
	{
		//  更新前に最新マウス情報を保存する
		m_prev_state = m_curr_state;

		//  マウスの状態を取得します
		HRESULT	hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_curr_state);
		if (FAILED(hr))
		{
			m_device->Acquire();
			hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_curr_state);
		}
		POINT p;
		//  マウス座標(スクリーン座標)を取得する
		GetCursorPos(&p);
		//  スクリーン座標にクライアント座標に変換する
		ScreenToClient(hwnd, &p);

		m_relative_pos.first = p.x - m_pos.first;
		m_relative_pos.second = p.x - m_pos.second;

		m_pos.first = static_cast<float>(p.x);
		m_pos.second = static_cast<float>(p.y);
	}

	std::pair<float, float> const& mouse_device::get_pos()
	{
		return m_pos;
	}

	std::pair<float, float> const& mouse_device::get_relative_pos()
	{
		return m_relative_pos;
	}

	button_state mouse_device::get_button_state(mouse_button_type mouseButtonType)
	{
		if (m_prev_state.rgbButtons[static_cast<int>(mouseButtonType)] & 0x80)
		{
			if (m_curr_state.rgbButtons[static_cast<int>(mouseButtonType)] & 0x80)
				return button_state::Held;
			else
				return button_state::Released;
		}
		else
		{
			if (m_curr_state.rgbButtons[static_cast<int>(mouseButtonType)] & 0x80)
				return button_state::Pressed;
			else
				return button_state::None;
		}
	}


	bool start_mouse_control(LPDIRECTINPUTDEVICE8 device)
	{
		//  制御開始
		if (FAILED(device->Acquire()))
		{
			return false;
		}

		DIDEVCAPS cap;
		device->GetCapabilities(&cap);
		//  ポーリング判定
		if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
		{
			DWORD error = GetLastError();
			//  ポーリング開始
			if (FAILED(device->Poll()))
			{
				return false;
			}
		}

		return true;
	}
}
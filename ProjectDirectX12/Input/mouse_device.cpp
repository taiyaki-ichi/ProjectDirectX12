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

		//  ���̓t�H�[�}�b�g�̎w��
		if (FAILED(m_device->SetDataFormat(&c_dfDIMouse)))
		{
			THROW_PDX12_EXCEPTION("failed mouse_device::initialize SetDataFormat");
		}

		//  �������[�h�̐ݒ�
		if (FAILED(m_device->SetCooperativeLevel(hwnd,DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		{
			THROW_PDX12_EXCEPTION("failed mouse_device::initialize SetCooperativeLevel");
		}

		int count = 0;
		// MAX_CNT��܂Œ��킷��
		constexpr int MAX_CNT = 5;
		//  ����J�n
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
		//  �X�V�O�ɍŐV�}�E�X����ۑ�����
		m_prev_state = m_curr_state;

		//  �}�E�X�̏�Ԃ��擾���܂�
		HRESULT	hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_curr_state);
		if (FAILED(hr))
		{
			m_device->Acquire();
			hr = m_device->GetDeviceState(sizeof(DIMOUSESTATE), &m_curr_state);
		}
		POINT p;
		//  �}�E�X���W(�X�N���[�����W)���擾����
		GetCursorPos(&p);
		//  �X�N���[�����W�ɃN���C�A���g���W�ɕϊ�����
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
		//  ����J�n
		if (FAILED(device->Acquire()))
		{
			return false;
		}

		DIDEVCAPS cap;
		device->GetCapabilities(&cap);
		//  �|�[�����O����
		if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
		{
			DWORD error = GetLastError();
			//  �|�[�����O�J�n
			if (FAILED(device->Poll()))
			{
				return false;
			}
		}

		return true;
	}
}
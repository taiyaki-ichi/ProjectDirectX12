#include"gamepad_device.hpp"
#include"../utility.hpp"

#pragma comment(lib,"xinput.lib ")

namespace pdx12
{
	BOOL CALLBACK device_find_call_back(LPCDIDEVICEINSTANCE ipddi, LPVOID pvRef)
	{
		auto ptr = reinterpret_cast<LPCDIDEVICEINSTANCE*>(pvRef);
		*ptr = ipddi;

		//���������I��������
		return DIENUM_CONTINUE;
	}

	void gamepad_device::initialize(IDirectInput8W* directInput, HWND hwnd)
	{
		LPCDIDEVICEINSTANCE deviceInsrance = nullptr;


		// �g�p�\�ȃf�o�C�X�̗�
		if (FAILED(directInput->EnumDevices(DI8DEVTYPE_GAMEPAD, device_find_call_back, (LPVOID*)(&deviceInsrance), DIEDFL_ATTACHEDONLY)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize EnumDevices");
		}

		if (deviceInsrance == nullptr)
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize no available gamepad");
		}

		//�f�o�C�X�̍쐬
		if (FAILED(directInput->CreateDevice(deviceInsrance->guidInstance, &m_device, NULL)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize CreateDevice");
		}

		// ���̓t�H�[�}�b�g�̎w��
		if (FAILED(m_device->SetDataFormat(&c_dfDIJoystick)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize SetDataFormat");
		}

		// �������[�h�̐ݒ�
		if (FAILED(m_device->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize SetCooperativeLevel");
		}

		// �����[�h���Βl���[�h�Ƃ��Đݒ�
		DIPROPDWORD diprop{};
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.diph.dwObj = 0;
		diprop.dwData = DIPROPAXISMODE_ABS;

		// �����[�h��ύX
		if (FAILED(m_device->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize SetProperty");
		}

		// X���̒l�͈̔͐ݒ�
		DIPROPRANGE diprg{};
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.diph.dwObj = DIJOFS_X;
		diprg.lMin = -1000;
		diprg.lMax = 1000;
		if (FAILED(m_device->SetProperty(DIPROP_RANGE, &diprg.diph)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize SetProperty");
		}

		// Y���̒l�͈̔͐ݒ�
		diprg.diph.dwObj = DIJOFS_Y;
		if (FAILED(m_device->SetProperty(DIPROP_RANGE, &diprg.diph)))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize SetProperty");
		}

		//����J�n
		if (FAILED(m_device->Acquire()))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize Acquire");
		}

		//�|�[�����O
		if (FAILED(m_device->Poll()))
		{
			THROW_PDX12_EXCEPTION("failed gamepad_device::initialize Poll");
		}

	}

	DIJOYSTATE gamepad_device::get_state()
	{
		DIJOYSTATE pad_data;
		m_device->GetDeviceState(sizeof(DIJOYSTATE), &pad_data);
		return pad_data;
	}




}
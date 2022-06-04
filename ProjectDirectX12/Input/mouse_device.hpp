#pragma once
#include<dinput.h>
#include<utility>
#include"button_state.hpp"

namespace pdx12
{
	enum class mouse_button_type
	{
		Left = 2,
		Middle = 0,
		Right = 1,
	};


	class mouse_device
	{
		//x,y�̏�
		std::pair<float, float> m_pos{};

		//x,y�̏�
		std::pair<float, float> m_relative_pos{};

		DIMOUSESTATE m_curr_state{};
		DIMOUSESTATE m_prev_state{};

		LPDIRECTINPUTDEVICE8 m_device = nullptr;

		HWND hwnd{};

	public:
		void initialize(IDirectInput8W*, HWND);

		void update();

		std::pair<float, float> const& get_pos();

		//�O��update���Ăяo���Ă���̑z��I�Ȉʒu
		std::pair<float, float> const& get_relative_pos();

		button_state get_button_state(mouse_button_type);
	};


}
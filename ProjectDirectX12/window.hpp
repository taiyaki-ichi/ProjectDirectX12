#pragma once
#include<Windows.h>

namespace pdx12
{
	//�E�B���h�E�̍쐻
	HWND create_window(wchar_t const* window_name, LONG width, LONG height);

	//���b�Z�[�W�̏���
	bool update_window();
}
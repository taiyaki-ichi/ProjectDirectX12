#include"window.hpp"

namespace pdx12
{
	//�R�[���o�b�N�p
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_DESTROY) {
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	HWND create_window(wchar_t const* window_name, LONG width, LONG height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		//�E�B���h�E�N���X�ݒ�
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = WndProc;
		wcex.lpszClassName = window_name;
		wcex.hInstance = hInstance;

		//�E�B���h�E�N���X�̓o�^
		RegisterClassEx(&wcex);

		//�E�B���h�E�T�C�Y�̐ݒ�
		RECT rect{ 0,0,width,height };

		HWND hwnd = CreateWindow(
			wcex.lpszClassName,			//�N���X��
			window_name,				//�^�C�g���o�[
			WS_OVERLAPPEDWINDOW,		//�^�C�g���o�[�Ƌ��E��������E�B���h�E
			CW_USEDEFAULT,				//x���W
			CW_USEDEFAULT,				//y���W
			rect.right - rect.left,		//���R
			rect.bottom - rect.top,		//�^�e
			nullptr,					//�e�E�B���h�E
			nullptr,					//���j���[�n���h��
			wcex.hInstance,				//�Ăяo���A�v���P�[�V�����n���h��
			nullptr						//�ǉ��p�����[�^
		);

		//�E�B���h�E�̕\��
		ShowWindow(hwnd, SW_SHOW);

		return hwnd;
	}

	//���b�Z�[�W�̏���
	bool update_window()
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);

		//�I���̃��b�Z�[�W�Ȃ��false
		if (msg.message == WM_QUIT)
			return false;
		else
			return true;
	}
}
#include"window.hpp"

namespace pdx12
{
	//コールバック用
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

		//ウィンドウクラス設定
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = WndProc;
		wcex.lpszClassName = window_name;
		wcex.hInstance = hInstance;

		//ウィンドウクラスの登録
		RegisterClassEx(&wcex);

		//ウィンドウサイズの設定
		RECT rect{ 0,0,width,height };

		HWND hwnd = CreateWindow(
			wcex.lpszClassName,			//クラス名
			window_name,				//タイトルバー
			WS_OVERLAPPEDWINDOW,		//タイトルバーと境界線があるウィンドウ
			CW_USEDEFAULT,				//x座標
			CW_USEDEFAULT,				//y座標
			rect.right - rect.left,		//ヨコ
			rect.bottom - rect.top,		//タテ
			nullptr,					//親ウィンドウ
			nullptr,					//メニューハンドル
			wcex.hInstance,				//呼び出しアプリケーションハンドル
			nullptr						//追加パラメータ
		);

		//ウィンドウの表示
		ShowWindow(hwnd, SW_SHOW);

		return hwnd;
	}

	//メッセージの処理
	bool update_window()
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);

		//終了のメッセージならばfalse
		if (msg.message == WM_QUIT)
			return false;
		else
			return true;
	}
}
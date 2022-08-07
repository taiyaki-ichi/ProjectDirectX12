#include"window.hpp"

namespace pdx12
{
	// メッセージを処理するためのコールバック関数を設定するために
	// その関数のポインタが必要になるので定義している
	// この関数は外部から参照される必要がないので無名名前空間でくくった方がいいかも?
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//　ウィンドウが破壊された時
		if (msg == WM_DESTROY) {
			// この関数がメッセージキューにWM_QUITを投げる
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	HWND create_window(wchar_t const* window_name, LONG width, LONG height)
	{
		HINSTANCE hInstance = GetModuleHandle(nullptr);

		// ウィンドウクラス設定
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = WndProc;
		wcex.lpszClassName = window_name;
		wcex.hInstance = hInstance;

		// ウィンドウクラスの登録
		RegisterClassEx(&wcex);

		// ウィンドウサイズの設定
		RECT rect{ 0,0,width,height };

		// TODO: CreateWindowが64bit環境ではCreateWindowW, 32bit環境ではCreateWindowAに展開されるので
		// 文字のサイズについて対応が必要
		HWND hwnd = CreateWindow(
			wcex.lpszClassName,			// クラス名
			window_name,				// タイトルバー
			WS_OVERLAPPEDWINDOW,		// タイトルバーと境界線があるウィンドウ
			CW_USEDEFAULT,				// x座標
			CW_USEDEFAULT,				// y座標
			rect.right - rect.left,		// ヨコ
			rect.bottom - rect.top,		// タテ
			nullptr,					// 親ウィンドウ
			nullptr,					// メニューハンドル
			wcex.hInstance,				// 呼び出しアプリケーションハンドル
			nullptr						// 追加パラメータ
		);

		// ウィンドウの表示
		ShowWindow(hwnd, SW_SHOW);

		return hwnd;
	}


	bool update_window()
	{
		MSG msg{};
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
		else
			return true;
	}
}
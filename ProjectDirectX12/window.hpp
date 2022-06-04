#pragma once
#include<Windows.h>

namespace pdx12
{
	//ウィンドウの作製
	HWND create_window(wchar_t const* window_name, LONG width, LONG height);

	//メッセージの処理
	bool update_window();
}
#pragma once
#include<Windows.h>

namespace pdx12
{
	// TODO: 32ビットの環境だとwindow_nameの型がcharでなければ行けないので対応する必要あり
	HWND create_window(wchar_t const* window_name, LONG width, LONG height);

	// ウィンドウについてのメッセージを処理し更新する
	// ウィンドウが閉じられた場合はfalseを返す. 通常はtrueを返す
	bool update_window();
}
#pragma once
#include"utility.hpp"
#include<d3d12.h>
#include<dxgi1_4.h>
#include<d3dcompiler.h>
#include<string>
#include<Windows.h>


namespace pdx12
{
	// ファイル名、関数名、インクルードオプション
	release_unique_ptr<ID3DBlob> create_shader(const wchar_t* fileName, const char* funcName, const char* includeOption);
}
#pragma once
#include<stdexcept>
#include<sstream>
#include<memory>
#include<utility>
#include<DirectXMath.h>
#include<algorithm>
#include<array>
#include<d3d12.h>
#include<dxgi1_4.h>

#include<iostream>

namespace pdx12
{
	using namespace DirectX;

	// �������8���_�̓��o
	void get_frustum_vertex(XMFLOAT3 const& eye, float asspect, float nearZ, float farZ, float viewAngle, XMFLOAT3 const& cameraForward, XMFLOAT3 const& cameraRight, std::array<XMFLOAT3, 8>& result);

	// �s��������邾���ł����s�̃R�[�h�ɂȂ��Ă��܂����ߊ֐����쐬����
	// ��������̍s��������鎞�ɂ͍œK���ł��Ȃ����Ȃ̂ł���܂�悭�Ȃ�����
	void apply(XMFLOAT3& float3, XMMATRIX const& matrix);

	// 8���_��[-1,1]x[-1,1]�͈̔͂Ɏ��߂�悤�ȃN���b�v�s������߂�
	void get_clop_matrix(std::array<XMFLOAT3, 8> const& float3, XMMATRIX& matrix);

}
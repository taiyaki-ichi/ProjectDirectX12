#pragma once
#include<DirectXMath.h>
#include<utility>
#include<cmath>

namespace pdx12
{
	//
	// �O�l�̎��_���������߂̃N���X�Ɗ֐�
	// �Ƃ肠�����쐬���������Ȃ̂Ŏ��_�ړ��ɂ��ăC�C�����ɕύX������
	//

	struct TPS
	{
		DirectX::XMFLOAT3 target{};
		float eyeRadius{};
		float eyeHeight{};
		float eyeRotation{};
	};

	// �O�̃t���[������̕ω��ʂ����ƂɎO�l�̎��_���X�V����
	// x, y, rot�͂��ꂼ��x���W, y���W, ��]�̕ω���
	inline void UpdateTPS(TPS& tps, float x, float y, float rot)
	{
		float moveX = x * std::cos(tps.eyeRotation) - y * std::sin(tps.eyeRotation);
		float moveY = x * std::sin(tps.eyeRotation) + y * std::cos(tps.eyeRotation);
		tps.target.x += moveX;
		tps.target.z += moveY;
		tps.eyeRotation += rot;
	}

	// �J����������ʒu���擾����
	inline DirectX::XMFLOAT3 GetEyePosition(TPS const& tps)
	{
		float x = tps.eyeRadius * std::cos(tps.eyeRotation) + tps.target.x;
		float z = tps.eyeRadius * std::sin(tps.eyeRotation) + tps.target.z;
		float y = tps.eyeHeight + tps.target.y;
		return { x,y,z };
	}
}
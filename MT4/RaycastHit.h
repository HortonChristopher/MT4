#pragma once

#include "BaseCollider.h"
#include <DirectXMath.h>

class Object3d;

/// <summary>
/// ���C�L���X�g�ɂ����𓾂�ׂ̍\���� Structure for obtaining information by raycast
/// </summary>
struct RaycastHit
{
	// �Փˑ���̃I�u�W�F�N�g Collision partner object
	Object3d* object = nullptr;
	// �Փˑ���̃R���C�_�[ Collider
	BaseCollider* collider = nullptr;
	// �Փ˓_ Collision point
	DirectX::XMVECTOR inter;
	// �Փ˓_�܂ł̋��� Distance to the collision point
	float distance = 0.0f;
};
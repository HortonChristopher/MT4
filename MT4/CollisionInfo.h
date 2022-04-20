#pragma once

#include <DirectXMath.h>

class Object3d;
class BaseCollider;

/// <summary>
/// �Փˏ�� Collision information
/// </summary>
struct CollisionInfo
{
public:
	CollisionInfo(Object3d* object, BaseCollider* collider, const DirectX::XMVECTOR& inter) {
		this->object = object;
		this->collider = collider;
		this->inter = inter;
	}

	// �Փˑ���̃I�u�W�F�N�g Collision partner object
	Object3d* object = nullptr;
	// �Փˑ���̃R���C�_�[ Collider
	BaseCollider* collider = nullptr;
	// �Փ˓_ Collision point
	DirectX::XMVECTOR inter;
};
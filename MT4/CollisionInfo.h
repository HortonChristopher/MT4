#pragma once

#include <DirectXMath.h>

class Object3d;
class BaseCollider;

/// <summary>
/// 衝突情報 Collision information
/// </summary>
struct CollisionInfo
{
public:
	CollisionInfo(Object3d* object, BaseCollider* collider, const DirectX::XMVECTOR& inter) {
		this->object = object;
		this->collider = collider;
		this->inter = inter;
	}

	// 衝突相手のオブジェクト Collision partner object
	Object3d* object = nullptr;
	// 衝突相手のコライダー Collider
	BaseCollider* collider = nullptr;
	// 衝突点 Collision point
	DirectX::XMVECTOR inter;
};
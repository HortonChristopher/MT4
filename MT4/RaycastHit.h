#pragma once

#include "BaseCollider.h"
#include <DirectXMath.h>

class Object3d;

/// <summary>
/// レイキャストによる情報を得る為の構造体 Structure for obtaining information by raycast
/// </summary>
struct RaycastHit
{
	// 衝突相手のオブジェクト Collision partner object
	Object3d* object = nullptr;
	// 衝突相手のコライダー Collider
	BaseCollider* collider = nullptr;
	// 衝突点 Collision point
	DirectX::XMVECTOR inter;
	// 衝突点までの距離 Distance to the collision point
	float distance = 0.0f;
};
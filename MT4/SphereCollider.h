#pragma once

#include "BaseCollider.h"
#include "CollisionPrimitive.h"

#include <DirectXMath.h>

/// <summary>
/// 球衝突判定オブジェクト Ball collision detection object
/// </summary>
class SphereCollider : public BaseCollider, public Sphere
{
private: //エイリアス Alias
	//DirectX::を省略 Omit DirectX
	using XMVECTOR = DirectX::XMVECTOR;
public:
	SphereCollider(XMVECTOR offset = { 0,0,0,0 }, float radius = 1.0f) :
		offset(offset),
		radius(radius)
	{
		// 球形状をセット Set the spherical shape
		shapeType = COLLISIONSHAPE_SPHERE;
	}

	/// <summary>
	/// 更新 Update
	/// </summary>
	void Update() override;

	inline void SetRadius(float radius) { this->radius = radius; }

	inline const XMVECTOR& GetOffset() { return offset; }

	inline void SetOffset(const XMVECTOR& offset) { this->offset = offset; }

	inline float GetRadius() { return radius; }

private:
	//オブジェクト中心からのオフセット Offset from object center
	XMVECTOR offset;
	// 半径 Radius
	float radius;
};
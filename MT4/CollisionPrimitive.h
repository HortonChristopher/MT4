#pragma once
/// <summary>
/// 当たり判定プリミティブ Collision Detection Primitive
/// </summary>

#include <DirectXMath.h>

/// <summary>
/// 球　Ball/Sphere
/// </summary>
struct Sphere
{
	// 中心座標 Center Coordinates
	DirectX::XMVECTOR center = { 0,0,0,1 };
	// 半径 radius
	float radius = 1.0f;
};

/// <summary>
/// 平面 Plane
/// </summary>
struct Plane
{
	// 法線ベクトル　Normal Vector
	DirectX::XMVECTOR normal = { 0, 1, 0, 0 };
	// 原点(0,0,0)からの距離 Distance from the origin
	float distance = 0.0f;
};

/// <summary>
/// 法線付き三角形（時計回りが表面）Triangle with traced line (clockwise is the surface)
/// </summary>
class Triangle
{
public:
	// 頂点座標3つ 3 vertex coordinates
	DirectX::XMVECTOR p0;
	DirectX::XMVECTOR p1;
	DirectX::XMVECTOR p2;
	// 法線ベクトル Normal Vector
	DirectX::XMVECTOR normal;

	/// <summary>
	/// 法線の計算 Normal calculation
	/// </summary>
	void ComputeNormal();
};

/// <summary>
/// レイ（半直線）
/// </summary>
struct Ray
{
	// 始点座標 Start point coordinates
	DirectX::XMVECTOR start = { 0,0,0,1 };
	// 方向 direction
	DirectX::XMVECTOR dir = { 1,0,0,0 };
};
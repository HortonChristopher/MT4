#pragma once

#include "CollisionPrimitive.h"

/// <summary>
/// 当たり判定ヘルパークラス Collision Detection helper class
/// </summary>
class Collision
{
public:
	/// <summary>
	/// 球と球の当たり判定 Collision Detection between sphere and sphere
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="inter">交点（平面上の最近接点）</param>
	/// <returns>交差しているか否か</returns>
	//static bool CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter);
	static bool CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter = nullptr, DirectX::XMVECTOR* reject = nullptr);

	/// <summary>
	/// 球と平面の当たり判定 Collision Detection between sphere and plane
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="plane">平面</param>
	/// <param name="inter">交点（平面上の最近接点）</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckSphere2Plane(const Sphere& sphere, const Plane& plane, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// 点と三角形の最近接点を求める Find the most recent contact between a point and a triangle
	/// </summary>
	/// <param name="point">点</param>
	/// <param name="triangle">三角形</param>
	/// <param name="closest">最近接点（出力用）</param>
	static void ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point, const Triangle& triangle, DirectX::XMVECTOR* closest);

	/// <summary>
	/// 球と法線付き三角形の当たりチェック Check for hits between spheres and triangles with normals
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="triangle">三角形</param>
	/// <param name="inter">交点（三角形上の最近接点）</param>
	/// <returns>交差しているか否か</returns>
	//static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter = nullptr);
	static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter = nullptr, DirectX::XMVECTOR* reject = nullptr);

	/// <summary>
	/// レイと平面の当たり判定 Collision detection between ray and plane
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="plane">平面</param>
	/// <param name="inter">距離（出力用）</param>
	/// <param name="inter">交点（出力用）</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckRay2Plane(const Ray& ray, const Plane& plane, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// レイと法線付き三角形の当たり判定 Collision detection of rays and triangles with normals
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="triangle">三角形</param>
	/// <param name="inter">距離（出力用）</param>
	/// <param name="inter">交点（出力用）</param>
	/// /// <returns>交差しているか否か</returns>
	static bool CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// レイと球の当たり判定 Collision detection of ray and sphere
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="sphere">球</param>
	/// <param name="inter">距離（出力用）</param>
	/// <param name="inter">交点（出力用）</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckRay2Sphere(const Ray& ray, const Sphere& sphere, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);
};
#pragma once

/// <summary>
/// 衝突判定用関連の型定義 Collision detection related type definition
/// </summary>

enum CollisionShapeType
{
	SHAPE_UNKNOWN = -1, // 未設定 Not set

	COLLISIONSHAPE_SPHERE, // 球 Sphere
	COLLISIONSHAPE_MESH, // メッシュ Mesh
};
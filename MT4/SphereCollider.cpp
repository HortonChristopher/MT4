#include "SphereCollider.h"

using namespace DirectX;

void SphereCollider::Update()
{
	// ワールド行列から座標を抽出 Extract coordinates from world matrix
	const XMMATRIX& matWorld = object3d->GetMatWorld();

	// 球のメンバ変数を更新 Update sphere member variables
	Sphere::center = matWorld.r[3] + offset;
	Sphere::radius = radius;
}
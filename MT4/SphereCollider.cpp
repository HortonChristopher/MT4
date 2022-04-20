#include "SphereCollider.h"

using namespace DirectX;

void SphereCollider::Update()
{
	// ���[���h�s�񂩂���W�𒊏o Extract coordinates from world matrix
	const XMMATRIX& matWorld = object3d->GetMatWorld();

	// ���̃����o�ϐ����X�V Update sphere member variables
	Sphere::center = matWorld.r[3] + offset;
	Sphere::radius = radius;
}
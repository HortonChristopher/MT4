#pragma once
/// <summary>
/// �����蔻��v���~�e�B�u Collision Detection Primitive
/// </summary>

#include <DirectXMath.h>

/// <summary>
/// ���@Ball/Sphere
/// </summary>
struct Sphere
{
	// ���S���W Center Coordinates
	DirectX::XMVECTOR center = { 0,0,0,1 };
	// ���a radius
	float radius = 1.0f;
};

/// <summary>
/// ���� Plane
/// </summary>
struct Plane
{
	// �@���x�N�g���@Normal Vector
	DirectX::XMVECTOR normal = { 0, 1, 0, 0 };
	// ���_(0,0,0)����̋��� Distance from the origin
	float distance = 0.0f;
};

/// <summary>
/// �@���t���O�p�`�i���v��肪�\�ʁjTriangle with traced line (clockwise is the surface)
/// </summary>
class Triangle
{
public:
	// ���_���W3�� 3 vertex coordinates
	DirectX::XMVECTOR p0;
	DirectX::XMVECTOR p1;
	DirectX::XMVECTOR p2;
	// �@���x�N�g�� Normal Vector
	DirectX::XMVECTOR normal;

	/// <summary>
	/// �@���̌v�Z Normal calculation
	/// </summary>
	void ComputeNormal();
};

/// <summary>
/// ���C�i�������j
/// </summary>
struct Ray
{
	// �n�_���W Start point coordinates
	DirectX::XMVECTOR start = { 0,0,0,1 };
	// ���� direction
	DirectX::XMVECTOR dir = { 1,0,0,0 };
};
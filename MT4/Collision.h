#pragma once

#include "CollisionPrimitive.h"

/// <summary>
/// �����蔻��w���p�[�N���X Collision Detection helper class
/// </summary>
class Collision
{
public:
	/// <summary>
	/// ���Ƌ��̓����蔻�� Collision Detection between sphere and sphere
	/// </summary>
	/// <param name="sphere">��</param>
	/// <param name="inter">��_�i���ʏ�̍ŋߐړ_�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	//static bool CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter);
	static bool CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter = nullptr, DirectX::XMVECTOR* reject = nullptr);

	/// <summary>
	/// ���ƕ��ʂ̓����蔻�� Collision Detection between sphere and plane
	/// </summary>
	/// <param name="sphere">��</param>
	/// <param name="plane">����</param>
	/// <param name="inter">��_�i���ʏ�̍ŋߐړ_�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	static bool CheckSphere2Plane(const Sphere& sphere, const Plane& plane, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// �_�ƎO�p�`�̍ŋߐړ_�����߂� Find the most recent contact between a point and a triangle
	/// </summary>
	/// <param name="point">�_</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="closest">�ŋߐړ_�i�o�͗p�j</param>
	static void ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point, const Triangle& triangle, DirectX::XMVECTOR* closest);

	/// <summary>
	/// ���Ɩ@���t���O�p�`�̓�����`�F�b�N Check for hits between spheres and triangles with normals
	/// </summary>
	/// <param name="sphere">��</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="inter">��_�i�O�p�`��̍ŋߐړ_�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	//static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter = nullptr);
	static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter = nullptr, DirectX::XMVECTOR* reject = nullptr);

	/// <summary>
	/// ���C�ƕ��ʂ̓����蔻�� Collision detection between ray and plane
	/// </summary>
	/// <param name="ray">���C</param>
	/// <param name="plane">����</param>
	/// <param name="inter">�����i�o�͗p�j</param>
	/// <param name="inter">��_�i�o�͗p�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	static bool CheckRay2Plane(const Ray& ray, const Plane& plane, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// ���C�Ɩ@���t���O�p�`�̓����蔻�� Collision detection of rays and triangles with normals
	/// </summary>
	/// <param name="ray">���C</param>
	/// <param name="triangle">�O�p�`</param>
	/// <param name="inter">�����i�o�͗p�j</param>
	/// <param name="inter">��_�i�o�͗p�j</param>
	/// /// <returns>�������Ă��邩�ۂ�</returns>
	static bool CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	/// <summary>
	/// ���C�Ƌ��̓����蔻�� Collision detection of ray and sphere
	/// </summary>
	/// <param name="ray">���C</param>
	/// <param name="sphere">��</param>
	/// <param name="inter">�����i�o�͗p�j</param>
	/// <param name="inter">��_�i�o�͗p�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	static bool CheckRay2Sphere(const Ray& ray, const Sphere& sphere, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);
};
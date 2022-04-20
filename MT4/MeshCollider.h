#pragma once

#include "BaseCollider.h"
#include "CollisionPrimitive.h"

#include <DirectXMath.h>

/// <summary>
/// ���b�V���Փ˔���I�u�W�F�N�g Mesh collision detection object
/// </summary>
class MeshCollider :
	public BaseCollider
{
public:
	MeshCollider()
	{
		// ���b�V���`����Z�b�g Set mesh shape
		shapeType = COLLISIONSHAPE_MESH;
	}

	/// <summary>
	/// �O�p�`�̔z����\�z���� Build an array of triangles
	/// </summary>
	void ConstructTriangles(Model* model);

	/// <summary>
	/// �X�V Update
	/// </summary>
	void Update() override;

	/// <summary>
	/// ���Ƃ̓����蔻�� Collision detection with a sphere
	/// </summary>
	/// <param name="sphere">��</param>
	/// <param name="inter">��_�i�o�͗p�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	//bool CheckCollisionSphere(const Sphere& sphere, DirectX::XMVECTOR* inter = nullptr);
	bool CheckCollisionSphere(const Sphere& sphere, DirectX::XMVECTOR* inter = nullptr, DirectX::XMVECTOR* reject = nullptr);


	/// <summary>
	/// ���C�Ƃ̓����蔻�� Collision detection with Ray
	/// </summary>
	/// <param name="sphere">���C</param>
	/// <param name="distance">�����i�o�͗p�j</param>
	/// <param name="inter">��_�i�o�͗p�j</param>
	/// <returns>�������Ă��邩�ۂ�</returns>
	bool CheckCollisionRay(const Ray& ray, float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

private:
	std::vector<Triangle> triangles;
	// ���[���h�s��̋t�s�� Inverse matrix of world matrix
	DirectX::XMMATRIX invMatWorld;
};
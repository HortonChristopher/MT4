#pragma once

#include "Object3d.h"

class Player :
	public Object3d
{
private:
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMMATRIX = DirectX::XMMATRIX;

public:
	/// <summary>
	/// 3Dオブジェクト生成 3D object generationd
	/// </summary>
	/// <returns>インスタンス</returns>
	static Player* Create(Model* model = nullptr);

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns>成否</returns>
	bool Initialize() override;

	/// <summary>
	/// 毎フレーム処理 Every frame processing
	/// </summary>
	void Update() override;

	/// <summary>
	/// 衝突時コールバック関数 Collision callback function
	/// </summary>
	/// <param name="info">衝突情報</param>
	void OnCollision(const CollisionInfo& info) override;

private:

	bool onGround = true;

	DirectX::XMVECTOR fallV;
};
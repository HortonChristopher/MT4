#pragma once

#include <DirectXMath.h>

// カメラ camera
class Camera
{
protected: // エイリアス alias
// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

public: // メンバ関数 Member function
	
	// コンストラクタ constructor
	Camera( int window_width, int window_height );

	// デストラクタ Destructor
	~Camera() = default;

	// 毎フレーム更新 Updated every frame
	void Update();

	// ビュー行列を更新 Update view matrix
	void UpdateViewMatrix();

	// 射影行列を更新 Update projection matrix
	void UpdateProjectionMatrix();

	// ビュー行列の取得 Get view matrix
	const XMMATRIX &GetViewMatrix() {
		return matView;
	}

	// 射影行列の取得 Get the projection matrix
	const XMMATRIX &GetProjectionMatrix() {
		return matProjection;
	}
	
	// ビュー射影行列の取得 Get the view projection matrix
	const XMMATRIX &GetViewProjectionMatrix() {
		return matViewProjection;
	}

	// ビルボード行列の取得 Get the billboard matrix
	const XMMATRIX &GetBillboardMatrix() {
		return matBillboard;
	}

	
	// 視点座標の取得Acquisition of viewpoint coordinates
	const XMFLOAT3 &GetEye() {
		return eye;
	}

	
	// 視点座標の設定 Setting viewpoint coordinates
	void SetEye( XMFLOAT3 eye ) {
		this->eye = eye; viewDirty = true;
	}

	// 注視点座標の取得 Acquisition of gazing point coordinates
	const XMFLOAT3 &GetTarget() {
		return target;
	}

	// 注視点座標の設定 Setting the gaze coordinate
	void SetTarget( XMFLOAT3 target ) {
		this->target = target; viewDirty = true;
	}
	
	// 上方向ベクトルの取得 Get the upward vector
	const XMFLOAT3 &GetUp() {
		return up;
	}

	// 上方向ベクトルの設定 Upward vector setting
	void SetUp( XMFLOAT3 up ) {
		this->up = up; viewDirty = true;
	}

	// ベクトルによる視点移動 Viewpoint movement by vector
	void MoveEyeVector( const XMFLOAT3 & move );
	
	// ベクトルによる移動 Vector movement
	void MoveVector( const XMFLOAT3 & move );

protected: // メンバ変数 Member variables
	// ビュー行列 View matrix
	XMMATRIX matView = DirectX::XMMatrixIdentity();
	// ビルボード行列 Billboard procession
	XMMATRIX matBillboard = DirectX::XMMatrixIdentity();
	// Y軸回りビルボード行列 Billboard matrix around Y axis
	XMMATRIX matBillboardY = DirectX::XMMatrixIdentity();
	// 射影行列 Projection matrix
	XMMATRIX matProjection = DirectX::XMMatrixIdentity();
	// ビュー射影行列 View projection matrix
	XMMATRIX matViewProjection = DirectX::XMMatrixIdentity();
	// ビュー行列ダーティフラグ View matrix dirty flag
	bool viewDirty = false;
	// 射影行列ダーティフラグ Projection matrix dirty flag
	bool projectionDirty = false;
	// 視点座標 Viewpoint coordinates
	XMFLOAT3 eye = { 0, 15, -30 };
	// 注視点座標 Gaze point coordinates
	XMFLOAT3 target = { 0, 0, 0 };
	// 上方向ベクトル Upward vector
	XMFLOAT3 up = { 0, 1, 0 };
	// アスペクト比 aspect ratio
	float aspectRatio = 1.0f;
};


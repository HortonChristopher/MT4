#include "Camera.h"

using namespace DirectX;

Camera::Camera( int window_width, int window_height )
{
	aspectRatio = (float)window_width / window_height;

	//ビュー行列の計算 View Matrix Calculation
	UpdateViewMatrix();

	// 射影行列の計算 Projection matrix calculation
	UpdateProjectionMatrix();

	// ビュープロジェクションの合成 View projection composition
	matViewProjection = matView * matProjection;
}

void Camera::Update()
{
	if ( viewDirty || projectionDirty ) {
		// 再計算必要なら If recalculation is necessary
		if ( viewDirty ) {
			// ビュー行列更新 View matrix update
			UpdateViewMatrix();
			viewDirty = false;
		}

		// 再計算必要なら If recalculation is necessary
		if ( projectionDirty ) {
			// ビュー行列更新 View matrix update
			UpdateProjectionMatrix();
			projectionDirty = false;
		}
		// ビュープロジェクションの合成 View projection composition
		matViewProjection = matView * matProjection;
	}
}

void Camera::UpdateViewMatrix()
{
	// ビュー行列の更新 View matrix update
	//matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	// 視点座標 Viewpoint coordinates
	XMVECTOR eyePosition = XMLoadFloat3( &eye );
	// 注視点座標 Gazepoint coordinates
	XMVECTOR targetPosition = XMLoadFloat3( &target );
	// （仮の）上方向 (Tenative) upward
	XMVECTOR upVector = XMLoadFloat3( &up );

	// カメラZ軸（視線方向）Camera Z axis (line of sight)
	XMVECTOR cameraAxisZ = XMVectorSubtract( targetPosition, eyePosition );
	// 0ベクトルだと向きが定まらないので除外 Exclude because the direction is not fixed if it is a 0 vector
	assert( !XMVector3Equal( cameraAxisZ, XMVectorZero() ) );
	assert( !XMVector3IsInfinite( cameraAxisZ ) );
	assert( !XMVector3Equal( upVector, XMVectorZero() ) );
	assert( !XMVector3IsInfinite( upVector ) );
	// ベクトルを正規化 Normalize vector
	cameraAxisZ = XMVector3Normalize( cameraAxisZ );

	// カメラのX軸（右方向）Camera X-axis (to the right)
	XMVECTOR cameraAxisX;
	// X軸は上方向→Z軸の外積で求まる The X-axis can be obtained by the cross product of hte upward direction -> Z axis
	cameraAxisX = XMVector3Cross( upVector, cameraAxisZ );
	// ベクトルを正規化 Normalize Vector
	cameraAxisX = XMVector3Normalize( cameraAxisX );

	// カメラのY軸（上方向）Camera Y-axis (upward)
	XMVECTOR cameraAxisY;
	// Y軸はZ軸→X軸の外積で求まる Y-axis can be obtained by the outer product of Z-axis -> X-axis
	cameraAxisY = XMVector3Cross( cameraAxisZ, cameraAxisX );

	// ここまでで直交した3方向のベクトルが揃う Vectors in 3 orthogonal directions are aligned up to this point
	//（ワールド座標系でのカメラの右方向、上方向、前方向）(Camera right, up, forward in world coordinate system)

	// カメラ回転行列 Camera Rotation Matrix
	XMMATRIX matCameraRot;
	// カメラ座標系→ワールド座標系の変換行列 Transformation matrix from camera coordinate system to world coordinate system
	matCameraRot.r[0] = cameraAxisX;
	matCameraRot.r[1] = cameraAxisY;
	matCameraRot.r[2] = cameraAxisZ;
	matCameraRot.r[3] = XMVectorSet( 0, 0, 0, 1 );
	// 転置により逆行列（逆回転）を計算 Calculate the inverse matrix (reverse rotation) by transpose
	matView = XMMatrixTranspose( matCameraRot );

	// 視点座標に-1を掛けた座標 Viewpoint coordinates multiplied by -1
	XMVECTOR reverseEyePosition = XMVectorNegate( eyePosition );
	// カメラの位置からワールド原点へのベクトル（カメラ座標系）Vector from camera position to world origin (camera coordinate system)
	XMVECTOR tX = XMVector3Dot( cameraAxisX, reverseEyePosition );	// X成分
	XMVECTOR tY = XMVector3Dot( cameraAxisY, reverseEyePosition );	// Y成分
	XMVECTOR tZ = XMVector3Dot( cameraAxisZ, reverseEyePosition );	// Z成分
	// 一つのベクトルにまとめる Combine into one vector
	XMVECTOR translation = XMVectorSet( tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f );
	// ビュー行列に平行移動成分を設定 Set translation component for view matrix
	matView.r[3] = translation;

#pragma region 全方向ビルボード行列の計算
	// ビルボード行列 Billboard procession
	matBillboard.r[0] = cameraAxisX;
	matBillboard.r[1] = cameraAxisY;
	matBillboard.r[2] = cameraAxisZ;
	matBillboard.r[3] = XMVectorSet( 0, 0, 0, 1 );
#pragma region

#pragma region Y軸回りビルボード行列の計算
	// カメラX軸、Y軸、Z軸 Camera X-axis, Y-axis, Z-axis
	XMVECTOR ybillCameraAxisX, ybillCameraAxisY, ybillCameraAxisZ;

	// X軸は共通 X-axis is common
	ybillCameraAxisX = cameraAxisX;
	// Y軸はワールド座標系のY軸 Y-axis is the Y-axis of the world coordinate system
	ybillCameraAxisY = XMVector3Normalize( upVector );
	// Z軸はX軸→Y軸の外積で求まる The Z axis can be obtained by the outer product of the X axis → Y axis.
	ybillCameraAxisZ = XMVector3Cross( ybillCameraAxisX, ybillCameraAxisY );

	// Y軸回りビルボード行列 Billboard matrix around Y axis
	matBillboardY.r[0] = ybillCameraAxisX;
	matBillboardY.r[1] = ybillCameraAxisY;
	matBillboardY.r[2] = ybillCameraAxisZ;
	matBillboardY.r[3] = XMVectorSet( 0, 0, 0, 1 );
#pragma endregion
}

void Camera::UpdateProjectionMatrix()
{
	// 平行投影による射影行列の生成 Generation of projection matrix by parallel projection
	//constMap->mat = XMMatrixOrthographicOffCenterLH(
	//	0, window_width,
	//	window_height, 0,
	//	0, 1);
	// 透視投影による射影行列の生成 Generation of projection matrix by central projection
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians( 60.0f ),
		aspectRatio,
		0.1f, 1000.0f
	);
}

void Camera::MoveEyeVector( const XMFLOAT3 &move )
{
	XMFLOAT3 eye_moved = GetEye();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	SetEye( eye_moved );
}

void Camera::MoveVector( const XMFLOAT3 &move )
{
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_moved = GetTarget();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	target_moved.x += move.x;
	target_moved.y += move.y;
	target_moved.z += move.z;

	SetEye( eye_moved );
	SetTarget( target_moved );
}


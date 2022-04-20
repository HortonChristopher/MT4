#include "Camera.h"

using namespace DirectX;

Camera::Camera( int window_width, int window_height )
{
	aspectRatio = (float)window_width / window_height;

	//�r���[�s��̌v�Z View Matrix Calculation
	UpdateViewMatrix();

	// �ˉe�s��̌v�Z Projection matrix calculation
	UpdateProjectionMatrix();

	// �r���[�v���W�F�N�V�����̍��� View projection composition
	matViewProjection = matView * matProjection;
}

void Camera::Update()
{
	if ( viewDirty || projectionDirty ) {
		// �Čv�Z�K�v�Ȃ� If recalculation is necessary
		if ( viewDirty ) {
			// �r���[�s��X�V View matrix update
			UpdateViewMatrix();
			viewDirty = false;
		}

		// �Čv�Z�K�v�Ȃ� If recalculation is necessary
		if ( projectionDirty ) {
			// �r���[�s��X�V View matrix update
			UpdateProjectionMatrix();
			projectionDirty = false;
		}
		// �r���[�v���W�F�N�V�����̍��� View projection composition
		matViewProjection = matView * matProjection;
	}
}

void Camera::UpdateViewMatrix()
{
	// �r���[�s��̍X�V View matrix update
	//matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	// ���_���W Viewpoint coordinates
	XMVECTOR eyePosition = XMLoadFloat3( &eye );
	// �����_���W Gazepoint coordinates
	XMVECTOR targetPosition = XMLoadFloat3( &target );
	// �i���́j����� (Tenative) upward
	XMVECTOR upVector = XMLoadFloat3( &up );

	// �J����Z���i���������jCamera Z axis (line of sight)
	XMVECTOR cameraAxisZ = XMVectorSubtract( targetPosition, eyePosition );
	// 0�x�N�g�����ƌ�������܂�Ȃ��̂ŏ��O Exclude because the direction is not fixed if it is a 0 vector
	assert( !XMVector3Equal( cameraAxisZ, XMVectorZero() ) );
	assert( !XMVector3IsInfinite( cameraAxisZ ) );
	assert( !XMVector3Equal( upVector, XMVectorZero() ) );
	assert( !XMVector3IsInfinite( upVector ) );
	// �x�N�g���𐳋K�� Normalize vector
	cameraAxisZ = XMVector3Normalize( cameraAxisZ );

	// �J������X���i�E�����jCamera X-axis (to the right)
	XMVECTOR cameraAxisX;
	// X���͏������Z���̊O�ςŋ��܂� The X-axis can be obtained by the cross product of hte upward direction -> Z axis
	cameraAxisX = XMVector3Cross( upVector, cameraAxisZ );
	// �x�N�g���𐳋K�� Normalize Vector
	cameraAxisX = XMVector3Normalize( cameraAxisX );

	// �J������Y���i������jCamera Y-axis (upward)
	XMVECTOR cameraAxisY;
	// Y����Z����X���̊O�ςŋ��܂� Y-axis can be obtained by the outer product of Z-axis -> X-axis
	cameraAxisY = XMVector3Cross( cameraAxisZ, cameraAxisX );

	// �����܂łŒ�������3�����̃x�N�g�������� Vectors in 3 orthogonal directions are aligned up to this point
	//�i���[���h���W�n�ł̃J�����̉E�����A������A�O�����j(Camera right, up, forward in world coordinate system)

	// �J������]�s�� Camera Rotation Matrix
	XMMATRIX matCameraRot;
	// �J�������W�n�����[���h���W�n�̕ϊ��s�� Transformation matrix from camera coordinate system to world coordinate system
	matCameraRot.r[0] = cameraAxisX;
	matCameraRot.r[1] = cameraAxisY;
	matCameraRot.r[2] = cameraAxisZ;
	matCameraRot.r[3] = XMVectorSet( 0, 0, 0, 1 );
	// �]�u�ɂ��t�s��i�t��]�j���v�Z Calculate the inverse matrix (reverse rotation) by transpose
	matView = XMMatrixTranspose( matCameraRot );

	// ���_���W��-1���|�������W Viewpoint coordinates multiplied by -1
	XMVECTOR reverseEyePosition = XMVectorNegate( eyePosition );
	// �J�����̈ʒu���烏�[���h���_�ւ̃x�N�g���i�J�������W�n�jVector from camera position to world origin (camera coordinate system)
	XMVECTOR tX = XMVector3Dot( cameraAxisX, reverseEyePosition );	// X����
	XMVECTOR tY = XMVector3Dot( cameraAxisY, reverseEyePosition );	// Y����
	XMVECTOR tZ = XMVector3Dot( cameraAxisZ, reverseEyePosition );	// Z����
	// ��̃x�N�g���ɂ܂Ƃ߂� Combine into one vector
	XMVECTOR translation = XMVectorSet( tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f );
	// �r���[�s��ɕ��s�ړ�������ݒ� Set translation component for view matrix
	matView.r[3] = translation;

#pragma region �S�����r���{�[�h�s��̌v�Z
	// �r���{�[�h�s�� Billboard procession
	matBillboard.r[0] = cameraAxisX;
	matBillboard.r[1] = cameraAxisY;
	matBillboard.r[2] = cameraAxisZ;
	matBillboard.r[3] = XMVectorSet( 0, 0, 0, 1 );
#pragma region

#pragma region Y�����r���{�[�h�s��̌v�Z
	// �J����X���AY���AZ�� Camera X-axis, Y-axis, Z-axis
	XMVECTOR ybillCameraAxisX, ybillCameraAxisY, ybillCameraAxisZ;

	// X���͋��� X-axis is common
	ybillCameraAxisX = cameraAxisX;
	// Y���̓��[���h���W�n��Y�� Y-axis is the Y-axis of the world coordinate system
	ybillCameraAxisY = XMVector3Normalize( upVector );
	// Z����X����Y���̊O�ςŋ��܂� The Z axis can be obtained by the outer product of the X axis �� Y axis.
	ybillCameraAxisZ = XMVector3Cross( ybillCameraAxisX, ybillCameraAxisY );

	// Y�����r���{�[�h�s�� Billboard matrix around Y axis
	matBillboardY.r[0] = ybillCameraAxisX;
	matBillboardY.r[1] = ybillCameraAxisY;
	matBillboardY.r[2] = ybillCameraAxisZ;
	matBillboardY.r[3] = XMVectorSet( 0, 0, 0, 1 );
#pragma endregion
}

void Camera::UpdateProjectionMatrix()
{
	// ���s���e�ɂ��ˉe�s��̐��� Generation of projection matrix by parallel projection
	//constMap->mat = XMMatrixOrthographicOffCenterLH(
	//	0, window_width,
	//	window_height, 0,
	//	0, 1);
	// �������e�ɂ��ˉe�s��̐��� Generation of projection matrix by central projection
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


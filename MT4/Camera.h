#pragma once

#include <DirectXMath.h>

// �J���� camera
class Camera
{
protected: // �G�C���A�X alias
// DirectX::���ȗ�
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

public: // �����o�֐� Member function
	
	// �R���X�g���N�^ constructor
	Camera( int window_width, int window_height );

	// �f�X�g���N�^ Destructor
	~Camera() = default;

	// ���t���[���X�V Updated every frame
	void Update();

	// �r���[�s����X�V Update view matrix
	void UpdateViewMatrix();

	// �ˉe�s����X�V Update projection matrix
	void UpdateProjectionMatrix();

	// �r���[�s��̎擾 Get view matrix
	const XMMATRIX &GetViewMatrix() {
		return matView;
	}

	// �ˉe�s��̎擾 Get the projection matrix
	const XMMATRIX &GetProjectionMatrix() {
		return matProjection;
	}
	
	// �r���[�ˉe�s��̎擾 Get the view projection matrix
	const XMMATRIX &GetViewProjectionMatrix() {
		return matViewProjection;
	}

	// �r���{�[�h�s��̎擾 Get the billboard matrix
	const XMMATRIX &GetBillboardMatrix() {
		return matBillboard;
	}

	
	// ���_���W�̎擾Acquisition of viewpoint coordinates
	const XMFLOAT3 &GetEye() {
		return eye;
	}

	
	// ���_���W�̐ݒ� Setting viewpoint coordinates
	void SetEye( XMFLOAT3 eye ) {
		this->eye = eye; viewDirty = true;
	}

	// �����_���W�̎擾 Acquisition of gazing point coordinates
	const XMFLOAT3 &GetTarget() {
		return target;
	}

	// �����_���W�̐ݒ� Setting the gaze coordinate
	void SetTarget( XMFLOAT3 target ) {
		this->target = target; viewDirty = true;
	}
	
	// ������x�N�g���̎擾 Get the upward vector
	const XMFLOAT3 &GetUp() {
		return up;
	}

	// ������x�N�g���̐ݒ� Upward vector setting
	void SetUp( XMFLOAT3 up ) {
		this->up = up; viewDirty = true;
	}

	// �x�N�g���ɂ�鎋�_�ړ� Viewpoint movement by vector
	void MoveEyeVector( const XMFLOAT3 & move );
	
	// �x�N�g���ɂ��ړ� Vector movement
	void MoveVector( const XMFLOAT3 & move );

protected: // �����o�ϐ� Member variables
	// �r���[�s�� View matrix
	XMMATRIX matView = DirectX::XMMatrixIdentity();
	// �r���{�[�h�s�� Billboard procession
	XMMATRIX matBillboard = DirectX::XMMatrixIdentity();
	// Y�����r���{�[�h�s�� Billboard matrix around Y axis
	XMMATRIX matBillboardY = DirectX::XMMatrixIdentity();
	// �ˉe�s�� Projection matrix
	XMMATRIX matProjection = DirectX::XMMatrixIdentity();
	// �r���[�ˉe�s�� View projection matrix
	XMMATRIX matViewProjection = DirectX::XMMatrixIdentity();
	// �r���[�s��_�[�e�B�t���O View matrix dirty flag
	bool viewDirty = false;
	// �ˉe�s��_�[�e�B�t���O Projection matrix dirty flag
	bool projectionDirty = false;
	// ���_���W Viewpoint coordinates
	XMFLOAT3 eye = { 0, 15, -30 };
	// �����_���W Gaze point coordinates
	XMFLOAT3 target = { 0, 0, 0 };
	// ������x�N�g�� Upward vector
	XMFLOAT3 up = { 0, 1, 0 };
	// �A�X�y�N�g�� aspect ratio
	float aspectRatio = 1.0f;
};


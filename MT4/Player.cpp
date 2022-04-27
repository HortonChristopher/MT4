#include "Player.h"
#include "Input.h"
#include "DebugText.h"
#include "SphereCollider.h"
#include "ParticleManager.h"
#include "CollisionManager.h"
#include "CollisionAttribute.h"

using namespace DirectX;

extern int cameraMove;
extern int prevCameraMove;

Player* Player::Create(Model* model)
{
	// 3D�I�u�W�F�N�g�̃C���X�^���X�𐶐� Instantiate a 3D object
	Player* instance = new Player();
	if (instance == nullptr) {
		return nullptr;
	}

	// ������ Initialize
	if (!instance->Initialize()) {
		delete instance;
		assert(0);
	}

	// ���f���̃Z�b�g Set model
	if (model) {
		instance->SetModel(model);
	}

	return instance;
}

bool Player::Initialize()
{
	if (!Object3d::Initialize())
	{
		return false;
	}

	// �R���C�_�[�̒ǉ� Add Collider
	float radius = 0.6f;
	// ���a�������������畂�������W�����̒��S�ɂ��� Make the coordinates floating from your feet the center of the sphere by the radius
	SetCollider(new SphereCollider(XMVECTOR({ 0, radius, 0, 0 }), radius));

	collider->SetAttribute(COLLISION_ATTR_ALLIES);

	return true;
}

void Player::Update()
{
	Input* input = Input::GetInstance();

	if (input->TriggerKey(DIK_W))
	{
		moving = true;
		rotation.y = 0.0f;
	}
	else
	{
		moving = false;
	}

	if (moving)
	{
		move += 1.0f;
		if (move >= 1.0f)
		{
			move = 1.0f;
		}
	}

	if (!moving)
	{
		move -= 0.05f;
		if (move <= 0.0f)
		{
			move = 0.0f;
		}
	}

	position.z += move;

	// �ړ��x�N�g����Y�����̊p�x�ŉ�] Rotate the movement vector at an angle around the Y axis
	XMVECTOR move = { 0, 0, 0.0f, 0 };
	XMMATRIX matRot = XMMatrixRotationY(XMConvertToRadians(rotation.y));
	move = XMVector3TransformNormal(move, matRot);

	// �����Ă�������Ɉړ� Move in the direction you are facing
	/*if (input->PushKey(DIK_W)) {
		moving = true;
	}
	else
	{
		moving = false;
	}

	if (moving)
	{
		const float speedUp = 0.05f;
		move.m128_f32[2] += speedUp;
		if (move.m128_f32[2] >= 1.0f)
		{
			move.m128_f32[2] = 1.0f;
		}
	}

	if (!moving)
	{
		const float slowDown = 0.05f;
		move.m128_f32[2] -= slowDown;
		if (move.m128_f32[2] <= 0.0f)
		{
			move.m128_f32[2] = 0.0f;
		}
	}

	position.x += move.m128_f32[0];
	position.y += move.m128_f32[1];
	position.z += move.m128_f32[2];*/

	// ���[���h�s��X�V
	UpdateWorldMatrix();

	if (!onGround) {

		const float fallAcc = -0.01f;
		const float fallVYMin = -0.5f;

		fallV.m128_f32[1] = max(fallV.m128_f32[1] + fallAcc, fallVYMin);

		position.x += fallV.m128_f32[0];
		position.y += fallV.m128_f32[1];
		position.z += fallV.m128_f32[2];
	}
	// �W�����v���� Jump operation
	/*else if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		onGround = false;
		const float jumpVYFist = 0.15f;
		fallV = { 0, jumpVYFist, 0, 0 };
	}*/
	
	// �R���C�_�[�X�V Collider update
	UpdateWorldMatrix();
	collider->Update();

	SphereCollider* sphereCollider = dynamic_cast<SphereCollider*>(collider);
	assert(sphereCollider);

	// �N�G���[�R�[���o�b�N�N���X
	class PlayerQueryCallback : public QueryCallback
	{
	public:
		PlayerQueryCallback(Sphere* sphere) : sphere(sphere) {};

		// �Փˎ��R�[���o�b�N�֐�
		bool OnQueryHit(const QueryHit& info) {

			const XMVECTOR up = { 0,1,0,0 };

			XMVECTOR rejectDir = XMVector3Normalize(info.reject);
			float cos = XMVector3Dot(rejectDir, up).m128_f32[0];

			// �n�ʔ��肵�����l
			const float threshold = cosf(XMConvertToRadians(30.0f));

			if (-threshold < cos && cos < threshold) {
				sphere->center += info.reject;
				move += info.reject;
			}

			return true;
		}

		Sphere* sphere = nullptr;
		DirectX::XMVECTOR move = {};
	};

	PlayerQueryCallback callback(sphereCollider);

	// ���ƒn�`�̌�����S����
	CollisionManager::GetInstance()->QuerySphere(*sphereCollider, &callback, COLLISION_ATTR_LANDSHAPE);
	// �����ɂ��r�˕�������
	position.x += callback.move.m128_f32[0];
	position.y += callback.move.m128_f32[1];
	position.z += callback.move.m128_f32[2];
	// ���[���h�s��X�V
	UpdateWorldMatrix();
	collider->Update();

	Ray ray;
	ray.start = sphereCollider->center;
	ray.start.m128_f32[1] += sphereCollider->GetRadius();
	ray.dir = { 0,-1,0,0 };
	RaycastHit raycastHit;

	if (onGround) {
		
		const float adsDistance = 0.2f;

		if (CollisionManager::GetInstance()->Raycast(ray, COLLISION_ATTR_LANDSHAPE, &raycastHit, sphereCollider->GetRadius() * 2.0f + adsDistance)) {
			onGround = true;
			position.y -= (raycastHit.distance - sphereCollider->GetRadius() * 2.0f);

			Object3d::Update();
		}
		//
		else {
			onGround = false;
			fallV = {};
		}
	}
	//
	else if (fallV.m128_f32[1] <= 0.0f) {
		if (CollisionManager::GetInstance()->Raycast(ray, COLLISION_ATTR_LANDSHAPE, &raycastHit, sphereCollider->GetRadius() * 2.0f)) {

			onGround = true;
			position.y -= (raycastHit.distance - sphereCollider->GetRadius() * 2.0f);

			Object3d::Update();
		}
	}

	//����������v���C���[�̈ʒu�������l�ɖ߂�
	if (position.y <= -20)
	{
		position.x = -12.0f;
		position.y = 0.0f;
		position.z = -12.0f;

	}

	// �s��̍X�V�Ȃ� Matrix update, etc
	Object3d::Update();
}

void Player::OnCollision(const CollisionInfo& info)
{
	// �f�o�b�O�e�L�X�g�\�� Debug text display
	//DebugText::GetInstance()->Print("Collision detected.", 0.0f, 0.0f, 12.0f);

	// �Փ˓X�Ƀp�[�e�B�N���𔭐������� Generate particles at the collision point
	/*for (int i = 0; i < 1; ++i)
	{
		const float rnd_vel = 0.1f;
		XMFLOAT3 vel{};
		vel.x = (float)rand() / RAND_MAX * rnd_vel - rnd_vel / 2.0f;
		vel.y = (float)rand() / RAND_MAX * rnd_vel - rnd_vel / 2.0f;
		vel.z = (float)rand() / RAND_MAX * rnd_vel - rnd_vel / 2.0f;

		ParticleManager::GetInstance()->Add(10, XMFLOAT3(info.inter.m128_f32), vel, XMFLOAT3(), 0.0f, 1.0f);
	}*/
}
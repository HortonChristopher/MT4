#include "GameScene.h"
#include "Model.h"
#include <cassert>
#include "Collision.h"
#include "SphereCollider.h"
#include "CollisionManager.h"
#include "Player.h"
#include <sstream>
#include <iomanip>
#include "Camera.h"
#include "TouchableObject.h"

using namespace DirectX;

//あんじょうテスト
//よっしーテスト♡

GameScene::GameScene()
{
}

GameScene::~GameScene()
{
	for (auto object : objects) {
		safe_delete(object);
	}

	safe_delete(spriteBG);

	safe_delete(objFighter);

	safe_delete(modelFighter);
	safe_delete(modelPlane);
}

void GameScene::Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio)
{
	//// nullptrチェック nullptr check
	assert(dxCommon);
	assert(input);
	assert(audio);

	this->dxCommon = dxCommon;
	this->input = input;
	this->audio = audio;

	//// カメラ生成 Camera generation
	camera = new Camera(WinApp::window_width, WinApp::window_height);

	collisionManager = CollisionManager::GetInstance();

	// カメラセット Camera set
	Object3d::SetCamera(camera);

	// デバッグテキスト用テクスチャ読み込み Import texture for debug text
	if (!Sprite::LoadTexture(debugTextTexNumber, L"Resources/debugfont.png")) {
		assert(0);
		return;
	}
	// デバッグテキスト初期化 Debug text initialization
	debugText.Initialize(debugTextTexNumber);

	// テクスチャ読み込み Texture loading
	if (!Sprite::LoadTexture(1, L"Resources/background.jpg")) {
		assert(0);
		return;
	}
	// 背景スプライト生成 Background sprite generation
	spriteBG = Sprite::Create(1, { 0.0f,0.0f });

	// パーティクルマネージャー
	particleMan = ParticleManager::Create(dxCommon->GetDevice(), camera);

	objFighter = Object3d::Create();
	objFighter2 = Object3d::Create();

	modelSphere1 = Model::CreateFromOBJ("sphere1");
	modelSphere2 = Model::CreateFromOBJ("sphere2");
	modelPlane = Model::CreateFromOBJ("yuka");

	objFighter->SetModel(modelSphere1);
	objFighter2->SetModel(modelSphere2);

	// モデルテーブル Model table
	Model* modeltable[2] = {
		modelPlane,
	};

	const int DIV_NUM = 10;
	const float LAND_SCALE = 3.0f;

	//ステージ1用外壁マップチップ
	const int WALL_NUM = 23;

	//自分側のマップチップ生成(Map chip generation)
	for (int i = 0; i < 5; i++) { // y coordinate - Bottom to Top
		for (int j = 0; j < 50; j++) { // x coordinate - Left to Right
			int modelIndex = 0;

			TouchableObject* object = TouchableObject::Create(modeltable[modelIndex]);
			object->SetScale({ LAND_SCALE, LAND_SCALE, LAND_SCALE });
			object->SetPosition({ (j - DIV_NUM / 2) * LAND_SCALE - LAND_SCALE * 1, 0, (i - DIV_NUM / 2) * LAND_SCALE });
			objects.push_back(object);
		}
	}

	objFighter->SetScale({ 1,1,1 });
	objFighter2->SetScale({ 1,1,1 });

	objFighter->SetRotation({ 0,90,0 });
	objFighter2->SetRotation({ 0,90,0 });

	objFighter->SetPosition({ 0,1,-12 });
	objFighter2->SetPosition({ 75,1,-12 });

	camera->SetTarget({ 50, 1, -20 });
	camera->MoveVector({ -12, 0, 0 });
	camera->MoveEyeVector({ 50, 0,-20 });
}

void GameScene::Update()
{
	playerPosition = objFighter->GetPosition();
	playerRotation = objFighter->GetRotation();
	playerPosition2 = objFighter2->GetPosition();
	playerRotation2 = objFighter2->GetRotation();

	if (input->TriggerKey(DIK_SPACE))
	{
		start = true;
	}

	if (!collision)
	{
		if (intersect(playerPosition, playerPosition2, 1.0f, 1.0f, 1.0f))
		{
			velocity = 0.2448f;
			velocity2 = 1.3248f;

			collision = true;
		}
	}
	//13890 - 6666
	//7224 = 1000vf1 + 600vf2
	//1000vf1 + 600 * (7.5 + vf1) = 7224
	//1000vf1 + 4500 + 600vf1 = 7224
	//1600vf1 = 2720
	//vf1 = 1.7m/s
	//7.5 + 1.7 = 9.2
	//1.7 = 6.12km/h * 0.04 = 0.2448
	//9.2 = 33.12km/h * 0.04 = 1.3248
	if (start)
	{
		if (!collision)
		{
			playerPosition.x += velocity;
			playerPosition2.x -= velocity2;
		}
		else
		{
			playerPosition.x += velocity;
			playerPosition2.x += velocity2;
		}
	}

	objFighter->SetPosition(playerPosition);
	objFighter2->SetPosition(playerPosition2);

	camera->Update();

	for (auto object : objects) {
		object->Update();
	}

	objFighter->Update();
	objFighter2->Update();

	collisionManager->CheckAllCollisions();

	//Debug Start
	//char msgbuf[256];
	//sprintf_s(msgbuf, 256, "Eye X: %f\n", eye.x);
	//OutputDebugStringA(msgbuf);
	//Debug End
}

void GameScene::Draw()
{
	// コマンドリストの取得 Get command list
	ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();

#pragma region 背景スプライト描画 Background sprite drawing
	// 背景スプライト描画前処理 Background sprite drawing pre-processing
	Sprite::PreDraw(cmdList);
	// 背景スプライト描画 Background sprite drawing
	spriteBG->Draw();

	// ここに背景スプライトの描画処理を追加できる You can add background sprite drawing processing here

	// スプライト描画後処理 Post-processing of sprite drawing
	Sprite::PostDraw();
	// 深度バッファクリア Depth buffer clear
	dxCommon->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画 3D object drawing
	// 3Dオブジェクト描画前処理 3D object drawing pre-processing
	Object3d::PreDraw(cmdList);

	for (auto object : objects) {
		object->Draw();
	}

	objFighter->Draw();
	objFighter2->Draw();

	// ここに3Dオブジェクトの描画処理を追加できる You can add 3D object drawing process here

	// 3Dオブジェクト描画後処理 Post-processing of 3D object drawing
	Object3d::PostDraw();
#pragma endregion

	#pragma region 前景スプライト描画 Foreground sprite drawing
	// 前景スプライト描画前処理 Foreground sprite drawing pre-processing
	Sprite::PreDraw( cmdList );
	
	// ここに前景スプライトの描画処理を追加できる You can add foreground sprite drawing processing here
	
	// 描画 drawing

	// デバッグテキストの描画 Debug text drawing
	// debugText.DrawAll(cmdList);
	
	// スプライト描画後処理 Post-processing of sprite drawing
	Sprite::PostDraw();
	#pragma endregion
}

int GameScene::intersect(XMFLOAT3 player, XMFLOAT3 wall, float circleR, float rectW, float rectH)
{
	XMFLOAT2 circleDistance;

	circleDistance.x = abs(player.x - wall.x);
	circleDistance.y = abs(player.z - wall.z);

	if (circleDistance.x > (rectW / 2.0f + circleR)) { return false; }
	if (circleDistance.y > (rectH / 2.0f + circleR)) { return false; }

	if (circleDistance.x <= (rectW / 2.0f)) { return true; }
	if (circleDistance.y <= (rectH / 2.0f)) { return true; }

	float cornerDistance_sq = ((circleDistance.x - rectW / 2.0f) * (circleDistance.x - rectW / 2.0f)) + ((circleDistance.y - rectH / 2.0f) * (circleDistance.y - rectH / 2.0f));

	return (cornerDistance_sq <= (circleR * circleR));
}
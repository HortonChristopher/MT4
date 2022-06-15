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
	if (!Sprite::LoadTexture(2, L"Resources/WhiteLine.png")) {
		assert(0);
		return;
	}
	if (!Sprite::LoadTexture(3, L"Resources/WhiteCircle.png")) {
		assert(0);
		return;
	}
	if (!Sprite::LoadTexture(4, L"Resources/RedCircle.png")) {
		assert(0);
		return;
	}
	// 背景スプライト生成 Background sprite generation
	spriteBG = Sprite::Create(1, { 0.0f,0.0f });
	whiteLine = Sprite::Create(2, { 300.0f, 0.0f });
	whiteCircle = Sprite::Create(3, { 360.0f, 150.0f });
	redCircle = Sprite::Create(4, { 360.0f, 150.0f });

	// パーティクルマネージャー
	particleMan = ParticleManager::Create(dxCommon->GetDevice(), camera);

	// モデルテーブル Model table
	Model* modeltable[2] = {
		modelPlane,
	};

	const int DIV_NUM = 10;
	const float LAND_SCALE = 3.0f;

	//ステージ1用外壁マップチップ
	const int WALL_NUM = 23;

	//自分側のマップチップ生成(Map chip generation)
	//for (int i = 0; i < 20; i++) { // y coordinate - Bottom to Top
	//	for (int j = 0; j < 50; j++) { // x coordinate - Left to Right
	//		int modelIndex = 0;

	//		TouchableObject* object = TouchableObject::Create(modeltable[modelIndex]);
	//		object->SetScale({ LAND_SCALE, LAND_SCALE, LAND_SCALE });
	//		object->SetPosition({ (j - DIV_NUM / 2) * LAND_SCALE - LAND_SCALE * 1, 0, (i - DIV_NUM / 2) * LAND_SCALE });
	//		objects.push_back(object);
	//	}
	//}

	circlePosition = { 360.0f, 150.0f };
	whiteCircle->SetSize({ 100.0f, 100.0f });
	redCircle->SetSize({ 100.0f, 100.0f });

	camera->SetTarget({ 0, 1, 0 });
	camera->MoveVector({ 00, 0, 0 });
	camera->MoveEyeVector({ 0, 0, 0 });
}

void GameScene::Update()
{
	if (input->PushKey(DIK_W))
	{
		linePosition.y -= 1.0f;
	}
	else if (input->PushKey(DIK_S))
	{
		linePosition.y += 1.0f;
	}

	if (input->PushKey(DIK_D))
	{
		linePosition.x += 1.0f;
	}
	else if (input->PushKey(DIK_A))
	{
		linePosition.x -= 1.0f;
	}

	lineTruePosition.x = linePosition.x + 2.5f;
	lineTruePosition.y = linePosition.y + 120.0f;
	circleTruePosition.x = circlePosition.x + 50.0f;
	circleTruePosition.y = circlePosition.y + 50.0f;

	if (intersect(lineTruePosition, circleTruePosition, 50.0f, 3.0f, 240.0f))
	{
		collision = true;
	}
	else
	{
		collision = false;
	}

	whiteLine->SetPosition(linePosition);

	camera->Update();

	//Debug Start
	char msgbuf[256];
	char msgbuf2[256];
	sprintf_s(msgbuf, 256, "Eye X: %f\n", linePosition.x);
	sprintf_s(msgbuf2, 256, "Eye X: %f\n", whiteCircle->GetPosition().x);
	OutputDebugStringA(msgbuf);
	OutputDebugStringA(msgbuf2);
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

	// ここに3Dオブジェクトの描画処理を追加できる You can add 3D object drawing process here

	// 3Dオブジェクト描画後処理 Post-processing of 3D object drawing
	Object3d::PostDraw();
#pragma endregion

	#pragma region 前景スプライト描画 Foreground sprite drawing
	// 前景スプライト描画前処理 Foreground sprite drawing pre-processing
	Sprite::PreDraw( cmdList );
	
	// ここに前景スプライトの描画処理を追加できる You can add foreground sprite drawing processing here
	
	// 描画 drawing
	whiteLine->Draw();
	if (collision)
	{
		redCircle->Draw();
	}
	else
	{
		whiteCircle->Draw();
	}

	// デバッグテキストの描画 Debug text drawing
	// debugText.DrawAll(cmdList);
	
	// スプライト描画後処理 Post-processing of sprite drawing
	Sprite::PostDraw();
	#pragma endregion
}

int GameScene::intersect(XMFLOAT2 player, XMFLOAT2 wall, float circleR, float rectW, float rectH)
{
	XMFLOAT2 circleDistance;

	circleDistance.x = abs(player.x - wall.x);
	circleDistance.y = abs(player.y - wall.y);

	if (circleDistance.x > (rectW / 2.0f + circleR)) { return false; }
	if (circleDistance.y > (rectH / 2.0f + circleR)) { return false; }

	if (circleDistance.x <= (rectW / 2.0f)) { return true; }
	if (circleDistance.y <= (rectH / 2.0f)) { return true; }

	float cornerDistance_sq = ((circleDistance.x - rectW / 2.0f) * (circleDistance.x - rectW / 2.0f)) + ((circleDistance.y - rectH / 2.0f) * (circleDistance.y - rectH / 2.0f));

	return (cornerDistance_sq <= (circleR * circleR));
}
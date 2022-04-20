#pragma once

#include "SafeDelete.h"
#include "DirectXCommon.h"
#include <DirectXMath.h>
#include "CollisionPrimitive.h"
#include "Input.h"
#include "Sprite.h"
#include "Object3d.h"
#include "DebugText.h"
#include "Audio.h"
#include "ParticleManager.h"

#include <vector>

class CollisionManager;
class Player;
class TouchableObject;

// ゲームシーン Game scene
class GameScene
{
private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

private: // 静的メンバ変数 Static member variables
	static const int debugTextTexNumber = 0;

public:

public: // メンバ関数 Member function

	// コンストクラタ Constructor
	GameScene();

	// デストラクタ Destructor
	~GameScene();

	// 初期化 Initialization
	void Initialize(DirectXCommon* dxCommon, Input* input, Audio* audio);

	// 毎フレーム処理 Every frame processing
	void Update();

	// 描画 drawing
	void Draw();

private: // メンバ変数 Member variables
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;
	Audio* audio = nullptr;
	DebugText debugText;

	// ゲームシーン用 For game scenes
	Camera* camera = nullptr;

	Sprite* spriteBG = nullptr;

	ParticleManager* particleMan = nullptr;

	Model* modelFighter = nullptr;
	Model* modelPlane = nullptr;

	TouchableObject* objGround = nullptr;

	Player* objFighter = nullptr;

	CollisionManager* collisionManager;

	std::vector<Object3d*> objects;

	//オブジェクトのポジション、回転角度
	XMFLOAT3 playerPosition;
	XMFLOAT3 playerRotation;
};
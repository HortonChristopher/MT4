#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <string>

#include "Model.h"
#include "Camera.h"
#include "CollisionInfo.h"

class BaseCollider;

// 3Dオブジェクト 3D object
class Object3d
{
private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: // サブクラス	 Subclass

	// パイプラインセット Pipeline set
	struct PipelineSet
	{
		// ルートシグネチャ Route signature
		ComPtr<ID3D12RootSignature> rootsignature;
		// パイプラインステートオブジェクト Pipeline state object
		ComPtr<ID3D12PipelineState> pipelinestate;
	};

	// 定数バッファ用データ構造体B0 Data structure for constant buffer B0
	struct ConstBufferDataB0
	{
		XMMATRIX mat;	// ３Ｄ変換行列 3D transformation matrix
	};
	
private: // 定数 constant


public: // 静的メンバ関数 Static member function
	// 静的初期化 Static initialization
	static void StaticInitialize( ID3D12Device *device, Camera *camera = nullptr);

	// グラフィックパイプラインの生成 Generate graphic pipeline
	static void CreateGraphicsPipeline();
	
	// カメラのセット Camera set
	static void SetCamera( Camera *camera ) {
		Object3d::camera = camera;
	}
	
	// 描画前処理 Pre-drawing processing
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);
	
	// 描画後処理 Post-drawing processing
	static void PostDraw();

	// 3Dオブジェクト生成 3D object generation
	static Object3d* Create( Model *model = nullptr );


private: // 静的メンバ変数 Static member variables
	// デバイス device
	static ID3D12Device* device;
	// コマンドリスト Command list
	static ID3D12GraphicsCommandList* cmdList;
	// テクスチャあり用パイプライン Pipeline for texture
	static PipelineSet pipelineSet;
	// カメラ camera
	static Camera *camera;

private:// 静的メンバ関数 Static member function
	
public: // メンバ関数 Member function
	/// <summary>
	/// コンストラクタ Constructor
	/// </summary>
	Object3d() = default;

	/// <summary>
	/// デストラクタ Destructor
	/// </summary>
	virtual ~Object3d();

	/// <summary>
	/// 初期化 Initialize
	/// </summary>
	/// <returns></returns>
	//bool Initialize();
	virtual bool Initialize();

	// 毎フレーム処理 Every frame processing
	//void Update();
	virtual void Update();

	// 描画 drawing
	//void Draw();
	virtual void Draw();

	/// <summary>
	/// ワールド行列の取得 Get the world matrix
	/// </summary>
	/// <returns>ワールド行列</returns>
	const XMMATRIX& GetMatWorld() { return matWorld; }

	/// <summary>
	/// コライダーのセット Collider Set
	/// </summary>
	/// <param name="collider">コライダー</param>
	void SetCollider(BaseCollider* collider);

	/// <summary>
	/// 衝突時コールバック関数 Collision callback function
	/// </summary>
	/// <param name="info">衝突情報</param>
	virtual void OnCollision(const CollisionInfo& info) {}

	// 座標の取得 Get coordinates
	const XMFLOAT3& GetPosition() { return position; }

	const XMFLOAT3& GetRotation() { return rotation; }

	// 座標の設定 Coordinate setting
	void SetPosition(XMFLOAT3 position) { this->position = position; }

	// 角度の設定 Angle setting
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }

	// スケールの設定 Scale setting
	void SetScale(XMFLOAT3 scale) { this->scale = scale; }

	// モデルのセット Set of models
	void SetModel(Model* model) { this->model = model; }

	void SetBillboard(bool isBillboard) { this->isBillboard = isBillboard; }

	/// <summary>
	/// モデルを取得 Get the model
	/// </summary>
	/// <returns>モデル</returns>
	inline Model* GetModel() { return model; }

	void UpdateWorldMatrix();

//private: // メンバ変数 Member variables
protected: // メンバ変数 Member variables

	// クラス名（デバッグ用 Class name (for debugging)
	const char* name = nullptr;

	// コライダー Collider
	BaseCollider* collider = nullptr;

	ComPtr<ID3D12Resource> constBuffB0; // 定数バッファ Constant buffer
	// 色 colour
	XMFLOAT4 color = { 1,1,1,1 };
	// ローカルスケール Local scale
	XMFLOAT3 scale = { 1,1,1 };
	XMFLOAT3 scale2 = { 1,1,1 };
	// X,Y,Z軸回りのローカル回転角 Local rotation angle around the X, Y, Z axes
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 rotationE = { 0,0,0 };
	// ローカル座標 Local coordinates
	XMFLOAT3 position = { -5,0,0 };
	XMFLOAT3 positionE = { 0,0,0 };
	// ローカルワールド変換行列 Local world transformation matrix
	XMMATRIX matWorld;
	XMMATRIX matWorldE;
	// 親オブジェクト Parent object
	Object3d* parent = nullptr;
	// モデル model
	Model* model = nullptr;
	// ビルボード Billboard
	bool isBillboard = false;
};


#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <forward_list>

#include "Camera.h"

// パーティクルマネージャ Particle manager
class ParticleManager
{
public: // 静的メンバ関数 Static member function
	/*static ParticleManager* GetInstance();*/

private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public: // サブクラス Subclass
	// 頂点データ構造体 Vertex data structure
	struct VertexPos
	{
		XMFLOAT3 pos; // xyz座標 xyz coordinates
		float scale; // スケール scale
	};

	// 定数バッファ用データ構造体 Data structure for constant buffer
	struct ConstBufferData
	{
		XMMATRIX mat;	// ビュープロジェクション行列 View projection matrix
		XMMATRIX matBillboard;	// ビルボード行列 Billboard procession
	};

	// パーティクル1粒 1 particle
	class Particle
	{
		// Microsoft::WRL::を省略
		template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
		// DirectX::を省略
		using XMFLOAT2 = DirectX::XMFLOAT2;
		using XMFLOAT3 = DirectX::XMFLOAT3;
		using XMFLOAT4 = DirectX::XMFLOAT4;
		using XMMATRIX = DirectX::XMMATRIX;
	public:
		// 座標 Coordinate
		XMFLOAT3 position = {};
		// 速度 speed
		XMFLOAT3 velocity = {};
		// 加速度 acceleration
		XMFLOAT3 accel = {};
		// 色 colour
		XMFLOAT3 color = {};
		// スケール scale
		float scale = 1.0f;
		// 回転 rotate
		float rotation = 0.0f;
		// 初期値 initial value
		XMFLOAT3 s_color = {};
		float s_scale = 1.0f;
		float s_rotation = 0.0f;
		// 最終値 Final value
		XMFLOAT3 e_color = {};
		float e_scale = 0.0f;
		float e_rotation = 0.0f;
		// 現在フレーム Current frame
		int frame = 0;
		// 終了フレーム End frame
		int num_frame = 0;
	};

private: // 定数 constant
	static const int vertexCount = 65536;		// 頂点数 Number of vertices

public: // 静的メンバ関数 Static member function
	// インスタンス生成 Instance generation
	static ParticleManager* Create(ID3D12Device* device, Camera* camera);

public: // メンバ関数	 Member function
	// 初期化 Initialization
	void Initialize();
	
	// 毎フレーム処理 Every frame processing
	void Update();

	// 描画 drawing
	void Draw(ID3D12GraphicsCommandList * cmdList);

	// パーティクルの追加 Add particles
	void Add(int life, XMFLOAT3 position, XMFLOAT3 velocity, XMFLOAT3 accel, float start_scale, float end_scale );

	// デスクリプタヒープの初期化 Descriptor heap initialization
	void InitializeDescriptorHeap();

	// グラフィックパイプライン生成 Graphic pipeline generation
	void InitializeGraphicsPipeline();

	// テクスチャ読み込み Texture loading
	void LoadTexture();

	// モデル作成 Modeling
	void CreateModel();

private: // メンバ変数 Member variables
	// デバイス device
	ID3D12Device* device = nullptr;
	// デスクリプタサイズ Descriptor size
	UINT descriptorHandleIncrementSize = 0u;
	// ルートシグネチャ Route signature
	ComPtr<ID3D12RootSignature> rootsignature;
	// パイプラインステートオブジェクト Pipeline state object
	ComPtr<ID3D12PipelineState> pipelinestate;
	// デスクリプタヒープ Descriptor heap
	ComPtr<ID3D12DescriptorHeap> descHeap;
	// 頂点バッファ Vertex buffer
	ComPtr<ID3D12Resource> vertBuff;
	// テクスチャバッファ Texture buffer
	ComPtr<ID3D12Resource> texbuff;
	// シェーダリソースビューのハンドル(CPU) Shader resource view handle (CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU) Shader resource view handle (CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	// 頂点バッファビュー Vertex buffer view
	D3D12_VERTEX_BUFFER_VIEW vbView;
	// 定数バッファ Constant buffer
	ComPtr<ID3D12Resource> constBuff;
	// パーティクル配列 Particle array
	std::forward_list<Particle> particles;
	// カメラ camera
	Camera* camera = nullptr;
private:
	// コンストラクタ constructor
	ParticleManager(ID3D12Device* device, Camera* camera);
};


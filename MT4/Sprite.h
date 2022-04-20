#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>

// スプライト Sprite
class Sprite
{

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
	struct VertexPosUv
	{
		XMFLOAT3 pos; // xyz座標 xyz coordinates
		XMFLOAT2 uv;  // uv座標 uv coordinates
	};

	// 定数バッファ用データ構造体 Data structure for constant buffer
	struct ConstBufferData
	{
		XMFLOAT4 color;	// 色 (RGBA) Color (RGBA)
		XMMATRIX mat;	// ３Ｄ変換行列 3D transformation matrix
	};

public: // 静的メンバ関数 Static member function

	// 静的初期化 Static initialization
	static bool StaticInitialize(ID3D12Device* device, int window_width, int window_height);

	// テクスチャ読み込み Texture loading
	static bool LoadTexture(UINT texnumber, const wchar_t*filename);

	// 描画前処理 Pre-drawing processing
	static void PreDraw(ID3D12GraphicsCommandList* cmdList);

	// 描画後処理 Post-drawing processing
	static void PostDraw();

	// スプライト生成 Sprite generation
	static Sprite* Create(UINT texNumber, XMFLOAT2 position, XMFLOAT4 color = { 1, 1, 1, 1 }, XMFLOAT2 anchorpoint = { 0.0f, 0.0f }, bool isFlipX = false, bool isFlipY = false);

private: // 静的メンバ変数 Static member variables
	// テクスチャの最大枚数 Maximum number of textures
	static const int srvCount = 512;
	// 頂点数 Number of vertices
	static const int vertNum = 4;
	// デバイス device
	static ID3D12Device* device;
	// デスクリプタサイズ Descriptor size
	static UINT descriptorHandleIncrementSize;
	// コマンドリスト Command list
	static ID3D12GraphicsCommandList* cmdList;
	// ルートシグネチャ Route signature
	static ComPtr<ID3D12RootSignature> rootSignature;
	// パイプラインステートオブジェクト Pipeline state object
	static ComPtr<ID3D12PipelineState> pipelineState;
	// 射影行列 Projection matrix
	static XMMATRIX matProjection;
	// デスクリプタヒープ Descriptor heap
	static ComPtr<ID3D12DescriptorHeap> descHeap;
	// テクスチャバッファ Texture buffer
	static ComPtr<ID3D12Resource> texBuff[srvCount];

public: // メンバ関数 Member function
	// コンストラクタ constructor
	Sprite(UINT texNumber, XMFLOAT2 position, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY);
	
	// 初期化 Initialization
	bool Initialize();

	// 角度の設定 Angle setting
	void SetRotation(float rotation);
	
	// 座標の取得 Get coordinates
	const XMFLOAT2& GetPosition() { return position; }

	// 座標の設定 Coordinate setting
	void SetPosition(XMFLOAT2 position);
	
	// サイズの設定 Size setting
	void SetSize(XMFLOAT2 size);

	// アンカーポイントの設定 Anchor point setting
	void SetAnchorPoint(XMFLOAT2 anchorpoint);

	// 左右反転の設定 Left / right inversion setting
	void SetIsFlipX(bool isFlipX);
	
	// 上下反転の設定 Upside down setting
	void SetIsFlipY(bool isFlipY);

	// テクスチャ範囲設定 Texture range setting
	void SetTextureRect(XMFLOAT2 texBase, XMFLOAT2 texSize);

	// 描画 drawing
	void Draw();

private: // メンバ変数 Member variables
	// 頂点バッファ Vertex buffer
	ComPtr<ID3D12Resource> vertBuff;
	// 定数バッファ Constant buffer
	ComPtr<ID3D12Resource> constBuff;
	// 頂点バッファビュー Vertex buffer view
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	// テクスチャ番号 Texture number
	UINT texNumber = 0; 
	// Z軸回りの回転角 Angle of rotation around the Z axis
	float rotation = 0.0f;
	// 座標 Coordinate
	XMFLOAT2 position{};
	// スプライト幅、高さ Sprite width, height
	XMFLOAT2 size = { 100.0f, 100.0f };
	// アンカーポイント Anchor point
	XMFLOAT2 anchorpoint = { 0, 0 };
	// ワールド行列 World matrix
	XMMATRIX matWorld{};		
	// 色 colour
	XMFLOAT4 color = { 1, 1, 1, 1 }; 
	// 左右反転 Flip horizontal
	bool isFlipX = false;
	// 上下反転 flip upside down
	bool isFlipY = false;
	// テクスチャ始点 Texture start point
	XMFLOAT2 texBase = { 0, 0 };
	// テクスチャ幅、高さ Texture width, height
	XMFLOAT2 texSize = { 100.0f, 100.0f };

private: // メンバ関数 Member function
	// 頂点データ転送 Vertex data transfer
	void TransferVertices();
};


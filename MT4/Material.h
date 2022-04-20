#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>

/// <summary>
/// マテリアル material
/// </summary>
class Material
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
	// 定数バッファ用データ構造体B1 Data structure for constant buffer B1
	struct ConstBufferDataB1
	{
		XMFLOAT3 ambient; // アンビエント係数 Ambient coefficient
		float pad1; // パディング Padding
		XMFLOAT3 diffuse; // ディフューズ係数 Diffuse coefficient
		float pad2; // パディング Padding
		XMFLOAT3 specular; // スペキュラー係数 Specular coefficient
		float alpha;	// アルファ alpha
	};

public: // 静的メンバ関数 Static member function

	// 静的初期化 Static initialization
	static void StaticInitialize(ID3D12Device* device);

	// マテリアル生成 Material generation
	static Material* Create();

private: // 静的メンバ変数 Static member variables
	// デバイス device
	static ID3D12Device* device;

public:
	std::string name;	// マテリアル名 Material name
	XMFLOAT3 ambient;	// アンビエント影響度 Ambient influence
	XMFLOAT3 diffuse;	// ディフューズ影響度 Diffuse influence
	XMFLOAT3 specular;	// スペキュラー影響度 Specular influence
	float alpha;		// アルファ alpha
	std::string textureFilename;	// テクスチャファイル名 Texture file name

public:
	
	// 定数バッファの取得 Get a constant buffer
	ID3D12Resource* GetConstantBuffer() { return constBuff.Get(); }

	// テクスチャ読み込み Texture loading
	void LoadTexture(const std::string& directoryPath, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle);
	
	// 更新 update
	void Update();

	const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() { return cpuDescHandleSRV; }
	const CD3DX12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() { return gpuDescHandleSRV; }

private:
	// テクスチャバッファ Texture buffer
	ComPtr<ID3D12Resource> texbuff;
	// 定数バッファ Constant buffer
	ComPtr<ID3D12Resource> constBuff;
	// シェーダリソースビューのハンドル(CPU) Shader resource view handle (CPU)
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
	// シェーダリソースビューのハンドル(CPU) Shader resource view handle (CPU)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;

private:
	// コンストラクタ constructor
	Material() {
		ambient = { 0.3f, 0.3f, 0.3f };
		diffuse = { 0.0f, 0.0f, 0.0f };
		specular = { 0.0f, 0.0f, 0.0f };
		alpha = 1.0f;
	}
	
	// 初期化 Initialization
	void Initialize();
	
	// 定数バッファの生成 Generate constant buffer
	void CreateConstantBuffer();
};


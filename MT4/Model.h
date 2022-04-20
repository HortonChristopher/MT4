#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Mesh.h"


// モデルデータ Model data
class Model
{
private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	static const std::string baseDirectory;

private: // 静的メンバ変数 Static member variables
	// デバイス device
	static ID3D12Device* device;
	// デスクリプタサイズ Descriptor size
	static UINT descriptorHandleIncrementSize;

public: // 静的メンバ関数 Static member function

	
	// 静的初期化 Static initialization
	static void StaticInitialize(ID3D12Device* device);

	// OBJファイルからメッシュ生成 Mesh generation from OBJ file
	static Model* CreateFromOBJ(const std::string& modelname);

public: // メンバ関数 Member function
	
	// デストラクタ Destructor
	~Model();
	
	// 初期化 Initialization
	void Initialize(const std::string& modelname);
	
	// 描画 drawing
	void Draw(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// メッシュコンテナを取得 Get mesh container
	/// </summary>
	/// <returns>メッシュコンテナ</returns>
	inline const std::vector<Mesh*>& GetMeshes() { return meshes; }

private: // メンバ変数 Member variables
	// 名前 name
	std::string name;
	// メッシュコンテナ Mesh container
	std::vector<Mesh*> meshes;
	// マテリアルコンテナ Material container
	std::unordered_map<std::string, Material*> materials;
	// デフォルトマテリアル Default material
	Material* defaultMaterial = nullptr;
	// デスクリプタヒープ Descriptor heap
	ComPtr<ID3D12DescriptorHeap> descHeap;

private: // メンバ関数 Member function
	
	// マテリアル読み込み Material loading
	void LoadMaterial(const std::string& directoryPath, const std::string & filename);

	// マテリアル登録 Material registration
	void AddMaterial(Material* material);

	// デスクリプタヒープの生成 Descriptor heap generation
	void CreateDescriptorHeap();
	
	// テクスチャ読み込み Texture loading
	void LoadTextures();

};


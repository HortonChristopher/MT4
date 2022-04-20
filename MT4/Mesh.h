#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include "Material.h"
#include <vector>

// 形状データ Shape data
class Mesh
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
	// 頂点データ構造体（テクスチャあり） Vertex data structure (with texture)
	struct VertexPosNormalUv
	{
		XMFLOAT3 pos; // xyz座標 xyz coordinates
		XMFLOAT3 normal; // 法線ベクトル Normal vector
		XMFLOAT2 uv;  // uv座標 uv coordinates
	};

public: // 静的メンバ関数 Static member function
	// 静的初期化 Static initialization
	static void StaticInitialize(ID3D12Device* device);	

private: // 静的メンバ変数 Static member variables
	// デバイス device
	static ID3D12Device* device;	

public: // メンバ関数 Member function
	// 名前を取得 Get the name
	const std::string& GetName() { return name; }
	
	// 名前をセット Set the name
	void SetName(const std::string& name);
	
	// 頂点データの追加 Add vertex data
	void AddVertex(const VertexPosNormalUv& vertex);
	
	// 頂点インデックスの追加 Add vertex index
	void AddIndex(unsigned short index);

	// 頂点データの数を取得 Get the number of vertex data
	inline size_t GetVertexCount() { return vertices.size(); }
	
	// マテリアルの取得 Get material
	Material* GetMaterial() { return material; }

	// マテリアルの割り当て Material assignment
	void SetMaterial(Material* material);

	// バッファの生成 Buffer generation
	void CreateBuffers();
	
	// 頂点バッファ取得 Get vertex buffer
	const D3D12_VERTEX_BUFFER_VIEW& GetVBView() { return vbView; }

	// インデックスバッファ取得 Get index buffer
	const D3D12_INDEX_BUFFER_VIEW& GetIBView() { return ibView; }
	
	// 描画 drawing
	void Draw(ID3D12GraphicsCommandList* cmdList);

	/// <summary>
	/// 頂点配列を取得 Get an array of vertices
	/// </summary>
	/// <returns>頂点配列</returns>
	inline const std::vector<VertexPosNormalUv>& GetVertices() { return vertices; }

	/// <summary>
	/// インデックス配列を取得 Get index array
	/// </summary>
	/// <returns>インデックス配列</returns>
	inline const std::vector<unsigned short>& GetIndices() { return indices; }

private: // メンバ変数 Member variables
	// 名前 name
	std::string name;
	// 頂点バッファ Vertex buffer
	ComPtr<ID3D12Resource> vertBuff;
	// インデックスバッファ Index buffer
	ComPtr<ID3D12Resource> indexBuff;
	// 頂点バッファビュー Vertex buffer view
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	// インデックスバッファビュー Index buffer view
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	// 頂点データ配列 Vertex data array
	std::vector<VertexPosNormalUv> vertices;
	// 頂点インデックス配列 Vertex index array
	std::vector<unsigned short> indices;
	// マテリアル material
	Material* material = nullptr;
};


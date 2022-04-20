#include "Material.h"
#include <DirectXTex.h>
#include <cassert>

using namespace DirectX;
using namespace std;

// 静的メンバ変数の実体 The entity of a static member variable
ID3D12Device* Material::device = nullptr;

void Material::StaticInitialize(ID3D12Device * device)
{
	Material::device = device;
}

Material * Material::Create()
{
	Material* instance = new Material;

	instance->Initialize();

	return instance;
}

void Material::Initialize()
{
	// 定数バッファの生成 Generate constant buffer
	CreateConstantBuffer();
}

void Material::CreateConstantBuffer()
{
	HRESULT result;
	// 定数バッファの生成 Generate constant buffer
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能 Uploadable
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(result)) {
		assert(0);
	}	
}

void Material::LoadTexture(const std::string& directoryPath, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
	// テクスチャなし No texture
	if (textureFilename.size() == 0) {
		textureFilename = "white1x1.png";
	}

	cpuDescHandleSRV = cpuHandle;
	gpuDescHandleSRV = gpuHandle;

	HRESULT result = S_FALSE;

	// WICテクスチャのロード Loading WIC textures
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	// ファイルパスを結合 Combine file paths
	string filepath = directoryPath + textureFilename;
	wchar_t wfilepath[128];

	// ユニコード文字列に変換 Convert to unicode string
	MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));

	result = LoadFromWICFile(
		wfilepath, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		assert(0);
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // 生データ抽出 Raw data extraction

	// リソース設定 Resource settings
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// テクスチャ用バッファの生成 Generate buffer for texture
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定 Specification for texture
		nullptr,
		IID_PPV_ARGS(&texbuff));
	if (FAILED(result)) {
		assert(0);
	}

	// テクスチャバッファにデータ転送 Data transfer to texture buffer
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // 全領域へコピー Copy to all areas
		img->pixels,   // 元データアドレス Original data address
		(UINT)img->rowPitch,  // 1ラインサイズ 1 line size
		(UINT)img->slicePitch // 1枚サイズ 1 sheet size
	);
	if (FAILED(result)) {
		assert(0);
	}

	// シェーダリソースビュー作成 Shader resource view creation
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体 Setting structure
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ 2D texture
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ Buffer associated with view
		&srvDesc, //テクスチャ設定情報 Texture setting information
		cpuDescHandleSRV
	);
}

void Material::Update()
{
	HRESULT result;
	// 定数バッファへデータ転送 Data transfer to a constant buffer
	ConstBufferDataB1* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->ambient = ambient;
		constMap->diffuse = diffuse;
		constMap->specular = specular;
		constMap->alpha = alpha;
		constBuff->Unmap(0, nullptr);
	}
}

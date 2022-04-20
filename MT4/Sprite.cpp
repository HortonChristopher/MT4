#include "Sprite.h"
#include <cassert>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <DirectXTex.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

// 静的メンバ変数の実体 The entity of a static member variable
ID3D12Device* Sprite::device = nullptr;
UINT Sprite::descriptorHandleIncrementSize;
ID3D12GraphicsCommandList* Sprite::cmdList = nullptr;
ComPtr<ID3D12RootSignature> Sprite::rootSignature;
ComPtr<ID3D12PipelineState> Sprite::pipelineState;
XMMATRIX Sprite::matProjection;
ComPtr<ID3D12DescriptorHeap> Sprite::descHeap;
ComPtr<ID3D12Resource> Sprite::texBuff[srvCount];

bool Sprite::StaticInitialize(ID3D12Device* device, int window_width, int window_height)
{
	// nullptrチェック nullptr check
	assert(device);

	Sprite::device = device;

	// デスクリプタサイズを取得 Get descriptor size
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト Vertex shader object
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト Pixel shader object
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト Error object

	// 頂点シェーダの読み込みとコンパイル Loading and compiling vertex shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/SpriteVertexShader.hlsl",	// シェーダファイル名 Shader file name
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする Enable to include
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定 Entry point name, shader model specification
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定 Debug settings
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー Copy the error content from errorBlob to string type
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示 Display error details in output window
		OutputDebugStringA(errstr.c_str());

		return false;
	}

	// ピクセルシェーダの読み込みとコンパイル Loading and compiling pixel shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/SpritePixelShader.hlsl",	// シェーダファイル名 Shader file name
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする Enable to include
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定 Entry point name, shader model specification
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定 Debug settings
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー Copy the error content from errorBlob to string type
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示 Display error details in output window
		OutputDebugStringA(errstr.c_str());
		
		return false;
	}

	// 頂点レイアウト Vertex layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい) xy coordinates (it is easier to see if written in one line)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv座標(1行で書いたほうが見やすい) uv coordinates (it is easier to see if written in one line)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプラインの流れを設定 Set the flow of the graphics pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク Sample mask
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定 Standard setting
	// ラスタライザステート Rasterizer state
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// デプスステンシルステート Depth stencil state
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; // 常に上書きルール Always overwrite rule

	// レンダーターゲットのブレンド設定 Render target blend settings
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画 RBGA Draw all channels
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	
	// ブレンドステートの設定 Blend state setting
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	// 深度バッファのフォーマット Depth buffer format
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定 Vertex layout settings
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	 
	// 図形の形状設定（三角形） Shape shape setting (triangle)
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// 描画対象は1つ One drawing target
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA RGBA specified from 0 to 255
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング Sampling once per pixel

	// デスクリプタレンジ Descriptor range
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ t0 register

	// ルートパラメータ Route parameters
	CD3DX12_ROOT_PARAMETER rootparams[2];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー Static sampler
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT); // s0 レジスタ s0 register

	// ルートシグネチャの設定 Route signature settings
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ Serialization of automatic version judgment
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (FAILED(result)) {
		assert(0);
		return false;
	}
	// ルートシグネチャの生成 Route signature generation
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	gpipeline.pRootSignature = rootSignature.Get();

	// グラフィックスパイプラインの生成 Graphics pipeline generation
	result = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineState));

	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// 射影行列計算 Projection matrix calculation
	matProjection = XMMatrixOrthographicOffCenterLH(
		0.0f, (float)window_width,
		(float)window_height, 0.0f,
		0.0f, 1.0f);

	// デスクリプタヒープを生成	Generate descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //シェーダから見えるように As visible from the shader
	descHeapDesc.NumDescriptors = srvCount;
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)); //生成 generation
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	return true;
}

bool Sprite::LoadTexture(UINT texnumber, const wchar_t * filename)
{
	// nullptrチェック nullptr check
	assert(device);

	HRESULT result;
	// WICテクスチャのロード Loading WIC textures
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(
		filename, WIC_FLAGS_NONE,
		&metadata, scratchImg);
	if (FAILED(result)) {
		assert(0);
		return false;
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
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定Specification for texture
		nullptr,
		IID_PPV_ARGS(&texBuff[texnumber]));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// テクスチャバッファにデータ転送 Data transfer to texture buffer
	result = texBuff[texnumber]->WriteToSubresource(
		0,
		nullptr, // 全領域へコピー Copy to all areas
		img->pixels,    // 元データアドレス Original data address
		(UINT)img->rowPitch,  // 1ラインサイズ 1 line size
		(UINT)img->slicePitch // 1枚サイズ 1 sheet size
	);
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// シェーダリソースビュー作成 Shader resource view creation
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体 Setting structure
	D3D12_RESOURCE_DESC resDesc = texBuff[texnumber]->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ 2D texture
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texBuff[texnumber].Get(), //ビューと関連付けるバッファ Buffer associated with view
		&srvDesc, //テクスチャ設定情報 Texture setting information
		CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), texnumber, descriptorHandleIncrementSize)
	);

	return true;
}

void Sprite::PreDraw(ID3D12GraphicsCommandList * cmdList)
{
	// PreDrawとPostDrawがペアで呼ばれていなければエラー Error if PreDraw and PostDraw are not called in pairs
	assert(Sprite::cmdList == nullptr);

	// コマンドリストをセット Set command list
	Sprite::cmdList = cmdList;

	// パイプラインステートの設定 Setting the pipeline state
	cmdList->SetPipelineState(pipelineState.Get());
	// ルートシグネチャの設定 Route signature settings
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	// プリミティブ形状を設定 Set primitive shape
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Sprite::PostDraw()
{
	// コマンドリストを解除 Cancel command list
	Sprite::cmdList = nullptr;
}

Sprite * Sprite::Create(UINT texNumber, XMFLOAT2 position, XMFLOAT4 color, XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY)
{
	// 仮サイズ Temporary size
	XMFLOAT2 size = { 100.0f, 100.0f };

	if (texBuff[texNumber])
	{
		// テクスチャ情報取得 Get texture information
		D3D12_RESOURCE_DESC resDesc = texBuff[texNumber]->GetDesc();
		// スプライトのサイズをテクスチャのサイズに設定 Set sprite size to texture size
		size = { (float)resDesc.Width, (float)resDesc.Height };
	}

	// Spriteのインスタンスを生成Create an instance of Sprite
	Sprite* sprite = new Sprite(texNumber, position, size, color, anchorpoint, isFlipX, isFlipY);
	if (sprite == nullptr) {
		return nullptr;
	}	

	// 初期化 Initialization
	if (!sprite->Initialize()) {
		delete sprite;
		assert(0);
		return nullptr;
	}

	return sprite;
}

Sprite::Sprite(UINT texNumber, XMFLOAT2 position, XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 anchorpoint, bool isFlipX, bool isFlipY)
{
	this->position = position;
	this->size = size;
	this->anchorpoint = anchorpoint;
	this->matWorld = XMMatrixIdentity();
	this->color = color;
	this->texNumber = texNumber;
	this->isFlipX = isFlipX;
	this->isFlipY = isFlipY;
	this->texSize = size;
}

bool Sprite::Initialize()
{
	// nullptrチェック nullptr check
	assert(device);

	HRESULT result = S_FALSE;

	// 頂点バッファ生成 Vertex buffer generation
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * vertNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();

	// 頂点バッファビューの作成 Creating a vertex buffer view
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexPosUv) * 4;
	vbView.StrideInBytes = sizeof(VertexPosUv);

	// 定数バッファの生成 Generate constant buffer
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能 Uploadable
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// 定数バッファにデータ転送 Data transfer to a constant buffer
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->color = color;
		constMap->mat = matProjection;
		constBuff->Unmap(0, nullptr);
	}

	return true;
}

void Sprite::SetRotation(float rotation)
{
	this->rotation = rotation;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetPosition(XMFLOAT2 position)
{
	this->position = position;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetSize(XMFLOAT2 size)
{
	this->size = size;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetAnchorPoint(XMFLOAT2 anchorpoint)
{
	this->anchorpoint = anchorpoint;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetIsFlipX(bool isFlipX)
{
	this->isFlipX = isFlipX;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetIsFlipY(bool isFlipY)
{
	this->isFlipY = isFlipY;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::SetTextureRect(XMFLOAT2 texBase, XMFLOAT2 texSize)
{
	this->texBase = texBase;
	this->texSize = texSize;

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	TransferVertices();
}

void Sprite::Draw()
{
	// ワールド行列の更新 World matrix update
	this->matWorld = XMMatrixIdentity();
	this->matWorld *= XMMatrixRotationZ(XMConvertToRadians(rotation));
	this->matWorld *= XMMatrixTranslation(position.x, position.y, 0.0f);

	// 定数バッファにデータ転送 Data transfer to a constant buffer
	ConstBufferData* constMap = nullptr;
	HRESULT result = this->constBuff->Map(0, nullptr, (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->color = this->color;
		constMap->mat = this->matWorld * matProjection;	// 行列の合成	 Matrix composition
		this->constBuff->Unmap(0, nullptr);
	}

	// 頂点バッファの設定 Vertex buffer settings
	cmdList->IASetVertexBuffers(0, 1, &this->vbView);

	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	// デスクリプタヒープをセット Set descriptor heap
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	// 定数バッファビューをセット Set constant buffer view
	cmdList->SetGraphicsRootConstantBufferView(0, this->constBuff->GetGPUVirtualAddress());
	// シェーダリソースビューをセット Set shader resource view
	cmdList->SetGraphicsRootDescriptorTable(1, CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), this->texNumber, descriptorHandleIncrementSize));
	// 描画コマンド Drawing command
	cmdList->DrawInstanced(4, 1, 0, 0);
}

void Sprite::TransferVertices()
{
	HRESULT result = S_FALSE;
	
	// 左下、左上、右下、右上 Bottom left, top left, bottom right, top right
	enum { LB, LT, RB, RT };

	float left = (0.0f - anchorpoint.x) * size.x;
	float right = (1.0f - anchorpoint.x) * size.x;
	float top = (0.0f - anchorpoint.x) * size.y;
	float bottom = (1.0f - anchorpoint.x) * size.y;
	if (isFlipX)
	{// 左右入れ替え Swap left and right
		left = -left;
		right = -right;
	}

	if (isFlipY)
	{// 上下入れ替え Swap up and down
		top = -top;
		bottom = -bottom;
	}

	// 頂点データ Vertex data
	VertexPosUv vertices[vertNum];

	vertices[LB].pos = { left,	bottom,	0.0f }; // 左下
	vertices[LT].pos = { left,	top,	0.0f }; // 左上
	vertices[RB].pos = { right,	bottom,	0.0f }; // 右下
	vertices[RT].pos = { right,	top,	0.0f }; // 右上

	// テクスチャ情報取得 Get texture information
	if (texBuff[texNumber])
	{
		D3D12_RESOURCE_DESC resDesc = texBuff[texNumber]->GetDesc();

		float tex_left = texBase.x / resDesc.Width;
		float tex_right = (texBase.x + texSize.x) / resDesc.Width;
		float tex_top = texBase.y / resDesc.Height;
		float tex_bottom = (texBase.y + texSize.y) / resDesc.Height;

		vertices[LB].uv = { tex_left,	tex_bottom }; // 左下 lower left
		vertices[LT].uv = { tex_left,	tex_top }; // 左上 upper left
		vertices[RB].uv = { tex_right,	tex_bottom }; // 右下 Bottom right
		vertices[RT].uv = { tex_right,	tex_top }; // 右上 Upper right
	}

	// 頂点バッファへのデータ転送 Data transfer to vertex buffer
	VertexPosUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		memcpy(vertMap, vertices, sizeof(vertices));
		vertBuff->Unmap(0, nullptr);
	}
}

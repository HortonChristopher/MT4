#include "ParticleManager.h"
#include <d3dcompiler.h>
#include <DirectXTex.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

//ParticleManager* ParticleManager::GetInstance()
//{
//	static ParticleManager instance;
//	return &instance;
//}

static const DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
	XMFLOAT3 result;
	result.x = lhs.x + rhs.x;
	result.y = lhs.y + rhs.y;
	result.z = lhs.z + rhs.z;
	return result;
}

static const DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs)
{
	XMFLOAT3 result;
	result.x = lhs.x - rhs.x;
	result.y = lhs.y - rhs.y;
	result.z = lhs.z - rhs.z;
	return result;
}

const DirectX::XMFLOAT3 operator/(const DirectX::XMFLOAT3& lhs, const float rhs)
{
	XMFLOAT3 result;
	result.x = lhs.x / rhs;
	result.y = lhs.y / rhs;
	result.z = lhs.z / rhs;
	return result;
}

ParticleManager * ParticleManager::Create(ID3D12Device* device, Camera* camera)
{
	// 3Dオブジェクトのインスタンスを生成 Instantiate a 3D object
	ParticleManager* partMan = new ParticleManager(device, camera);
	if (partMan == nullptr) {
		return nullptr;
	}

	// 初期化 Initialization
	partMan->Initialize();

	return partMan;
}

void ParticleManager::Initialize()
{
	// nullptrチェック nullptr check
	assert(device);

	HRESULT result;

	// デスクリプタヒープの初期化 Descriptor heap initialization
	InitializeDescriptorHeap();

	// パイプライン初期化 Pipeline initialization
	InitializeGraphicsPipeline();

	// テクスチャ読み込み Texture loading
	//LoadTexture();

	// モデル生成 Model generation
	CreateModel();

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
	}
}

void ParticleManager::Update()
{
	HRESULT result;

	// 寿命が尽きたパーティクルを全削除 Delete all particles that have reached the end of their life
	particles.remove_if([](Particle& x) { return x.frame >= x.num_frame; });

	// 全パーティクル更新 Update all particles
	for (std::forward_list<Particle>::iterator it = particles.begin();
		it != particles.end();
		it++) {

		// 経過フレーム数をカウント Count the number of elapsed frames
		it->frame++;
		// 進行度を0～1の範囲に換算 Convert progress to range 0 to 1
		float f = (float)it->num_frame / it->frame;

		// 速度に加速度を加算 Add acceleration to velocity
		it->velocity = it->velocity + it->accel;

		// 速度による移動 Movement by speed
		it->position = it->position + it->velocity;

		// カラーの線形補間 Linear interpolation of colors
		it->color = it->s_color + (it->e_color - it->s_color) / f;

		// スケールの線形補間 Linear interpolation of scale
		it->scale = it->s_scale + (it->e_scale - it->s_scale) / f;

		// スケールの線形補間 Linear interpolation of scale
		it->rotation = it->s_rotation + (it->e_rotation - it->s_rotation) / f;
	}	

	// 頂点バッファへデータ転送 Data transfer to vertex buffer
	int vertCount = 0;
	VertexPos* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		// パーティクルの情報を1つずつ反映 Reflect particle information one by one
		for (std::forward_list<Particle>::iterator it = particles.begin();
			it != particles.end();
			it++) {
			// 座標 Coordinate
			vertMap->pos = it->position;
			// スケール scale
			vertMap->scale = it->scale;
			// 次の頂点へ To the next vertex
			vertMap++;
			if (++vertCount >= vertexCount) {
				break;
			}
		}
		vertBuff->Unmap(0, nullptr);
	}

	// 定数バッファへデータ転送 Data transfer to a constant buffer
	ConstBufferData* constMap = nullptr;
	result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = camera->GetViewProjectionMatrix();
	constMap->matBillboard = camera->GetBillboardMatrix();
	constBuff->Unmap(0, nullptr);
}

void ParticleManager::Draw(ID3D12GraphicsCommandList * cmdList)
{
	UINT drawNum = (UINT)std::distance(particles.begin(), particles.end());
	if (drawNum > vertexCount) {
		drawNum = vertexCount;
	}

	// パーティクルが1つもない場合 If there are no particles
	if (drawNum == 0) {
		return;
	}

	// nullptrチェック nullptr check
	assert(cmdList);

	// パイプラインステートの設定 Setting the pipeline state
	cmdList->SetPipelineState(pipelinestate.Get());
	// ルートシグネチャの設定 Route signature settings
	cmdList->SetGraphicsRootSignature(rootsignature.Get());
	// プリミティブ形状を設定 Set primitive shape
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		
	// 頂点バッファの設定 Vertex buffer settings
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	// デスクリプタヒープの配列 Array of descriptor heaps
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット Set constant buffer view
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());
	// シェーダリソースビューをセット Set shader resource view
	cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
	// 描画コマンド Drawing command
	cmdList->DrawInstanced(drawNum, 1, 0, 0);
}

void ParticleManager::Add(int life, XMFLOAT3 position, XMFLOAT3 velocity, XMFLOAT3 accel, float start_scale, float end_scale)
{
	// リストに要素を追加 Add element to list
	particles.emplace_front();
	// 追加した要素の参照 Reference of added element
	Particle& p = particles.front();
	p.position = position;
	p.velocity = velocity;
	p.accel = accel;
	p.s_scale = start_scale;
	p.e_scale = end_scale;
	p.num_frame = life;
}

void ParticleManager::InitializeDescriptorHeap()
{
	HRESULT result = S_FALSE;

	// デスクリプタヒープを生成	 Generate descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //シェーダから見えるように As visible from the shader
	descHeapDesc.NumDescriptors = 1; // シェーダーリソースビュー1つ 1 shader resource view
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap)); //生成 generation
	if (FAILED(result)) {
		assert(0);
	}

	// デスクリプタサイズを取得 Get descriptor size
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ParticleManager::InitializeGraphicsPipeline()
{
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト Vertex shader object
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト Pixel shader object
	ComPtr<ID3DBlob> gsBlob;	// ジオメトリシェーダオブジェクト Geometry shader object
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト Error object

	// 頂点シェーダの読み込みとコンパイルLoading and compiling vertex shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/ParticleVS.hlsl",	// シェーダファイル名 Shader file name
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
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル Loading and compiling pixel shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/ParticlePS.hlsl",	// シェーダファイル名 Shader file name
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
		exit(1);
	}

	// ジオメトリシェーダの読み込みとコンパイル Loading and compiling geometry shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/ParticleGS.hlsl",	// シェーダファイル名 Shader file name
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする Enable to include
		"main", "gs_5_0",	// エントリーポイント名、シェーダーモデル指定 Entry point name, shader model specification
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定 Debug settings
		0,
		&gsBlob, &errorBlob);
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
		exit(1);
	}

	// 頂点レイアウト Vertex layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい) xy coordinates (it is easier to see if written in one line)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // スケール scale
			"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプラインの流れを設定 Set the flow of the graphics pipeline
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	gpipeline.GS = CD3DX12_SHADER_BYTECODE(gsBlob.Get());

	// サンプルマスク Sample mask
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定 Standard setting
	// ラスタライザステート Rasterizer state
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート Depth stencil state
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// デプスの書き込みを禁止 Prohibit writing depth
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// レンダーターゲットのブレンド設定 Render target blend settings
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画 RBGA Draw all channels
	blenddesc.BlendEnable = true;
	// 加算ブレンディング Additive blending
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_ONE;
	blenddesc.DestBlend = D3D12_BLEND_ONE;
	//// 減算ブレンディング Subtractive blending
	//blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;
	//blenddesc.DestBlend = D3D12_BLEND_ONE;

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
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

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
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの設定 Route signature settings
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ Serialization of automatic version judgment
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成 Route signature generation
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	if (FAILED(result)) {
		assert(0);
	}

	gpipeline.pRootSignature = rootsignature.Get();

	// グラフィックスパイプラインの生成 Graphics pipeline generation
	result = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));

	if (FAILED(result)) {
		assert(0);
	}
}

void ParticleManager::LoadTexture()
{
	HRESULT result = S_FALSE;

	// WICテクスチャのロード Loading WIC textures
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	result = LoadFromWICFile(
		L"Resources/effect1.png", WIC_FLAGS_NONE,
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
		img->pixels,    // 元データアドレス Original data address
		(UINT)img->rowPitch,  // 1ラインサイズ 1 line size
		(UINT)img->slicePitch // 1枚サイズ 1 sheet size
	);
	if (FAILED(result)) {
		assert(0);
	}

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

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

void ParticleManager::CreateModel()
{
	HRESULT result = S_FALSE;

	// 頂点バッファ生成 Vertex buffer generation
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPos)*vertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	// 頂点バッファビューの作成 Creating a vertex buffer view
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexPos)*vertexCount;
	vbView.StrideInBytes = sizeof(VertexPos);
}

ParticleManager::ParticleManager(ID3D12Device * device, Camera * camera)
{
	this->device = device;
	this->camera = camera;
}
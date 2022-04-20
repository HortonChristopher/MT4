#include "Object3d.h"
#include "BaseCollider.h"
#include "CollisionManager.h"
#include <d3dcompiler.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

// 静的メンバ変数の実体 The entity of a static member variable
ID3D12Device *Object3d::device = nullptr;
ID3D12GraphicsCommandList *Object3d::cmdList = nullptr;
Object3d::PipelineSet Object3d::pipelineSet;
Camera *Object3d::camera = nullptr;

Object3d::~Object3d()
{
	if (collider) {
		// コリジョンマネージャから登録を解除する Unregister from Collision Manager
		CollisionManager::GetInstance()->RemoveCollider(collider);

		delete collider;
	}
}

void Object3d::StaticInitialize( ID3D12Device *device, Camera *camera )
{

	// nullptrチェック nullptr check
	assert( device );

	Object3d::device = device;
	Object3d::camera = camera;

	// グラフィックパイプラインの生成 Generate graphic pipeline
	CreateGraphicsPipeline();

	// モデルの静的初期化 Static initialization of the model
	Model::StaticInitialize( device );

}

void Object3d::CreateGraphicsPipeline()
{
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト Vertex shader object
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト Pixel shader object
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト Error object

	// 頂点シェーダの読み込みとコンパイル Loading and compiling vertex shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/OBJVertexShader.hlsl",	// シェーダファイル名 Shader file name
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする Enable to include
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定 Entry point name, shader model specification
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob );
	if ( FAILED( result ) ) {
		// errorBlobからエラー内容をstring型にコピー Copy the error content from errorBlob to string type
		std::string errstr;
		errstr.resize( errorBlob->GetBufferSize() );

		std::copy_n( (char *)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin() );
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示 Display error details in output window 
		OutputDebugStringA( errstr.c_str() );
		exit( 1 );
	}

	// ピクセルシェーダの読み込みとコンパイル Loading and compiling pixel shaders
	result = D3DCompileFromFile(
		L"Resources/shaders/OBJPixelShader.hlsl",	// シェーダファイル名 Shader file name
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする Enable to include
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定 Entry point name, shader model specification
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定 Debug settings
		0,
		&psBlob, &errorBlob );
	if ( FAILED( result ) ) {
		// errorBlobからエラー内容をstring型にコピー Copy the error content from errorBlob to string type
		std::string errstr;
		errstr.resize( errorBlob->GetBufferSize() );

		std::copy_n( (char *)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin() );
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示 Display error details in output window
		OutputDebugStringA( errstr.c_str() );
		exit( 1 );
	}

	// 頂点レイアウト Vertex layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい) xy coordinates (it is easier to see if written in one line)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // 法線ベクトル(1行で書いたほうが見やすい) Normal vector (easier to see if written in one line)
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
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
	gpipeline.VS = CD3DX12_SHADER_BYTECODE( vsBlob.Get() );
	gpipeline.PS = CD3DX12_SHADER_BYTECODE( psBlob.Get() );

	// サンプルマスク Sample mask
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート Rasterizer state
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート Depth stencil state
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( D3D12_DEFAULT );

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
	gpipeline.InputLayout.NumElements = _countof( inputLayout );

	// 図形の形状設定（三角形） Shape shape setting (triangle)
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// 描画対象は1つ One drawing target
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA RGBA specified from 0 to 255
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング Sampling once per pixel

	// デスクリプタレンジ Descriptor range
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 ); // t0 レジスタ t0 register

	// ルートパラメータ Route parameters
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView( 0, 0, D3D12_SHADER_VISIBILITY_ALL );
	rootparams[1].InitAsConstantBufferView( 1, 0, D3D12_SHADER_VISIBILITY_ALL );
	rootparams[2].InitAsDescriptorTable( 1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL );

	// スタティックサンプラー Static sampler
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC( 0 );

	// ルートシグネチャの設定 Route signature settings
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0( _countof( rootparams ), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ Serialization of automatic version judgment
	result = D3DX12SerializeVersionedRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob );
	// ルートシグネチャの生成 Route signature generation
	result = device->CreateRootSignature( 0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS( &pipelineSet.rootsignature ) );
	if ( FAILED( result ) ) {
		assert( 0 );
	}

	gpipeline.pRootSignature = pipelineSet.rootsignature.Get();

	// グラフィックスパイプラインの生成 Graphics pipeline generation
	result = device->CreateGraphicsPipelineState( &gpipeline, IID_PPV_ARGS( &pipelineSet.pipelinestate ) );

	if ( FAILED( result ) ) {
		assert( 0 );
	}
}

void Object3d::PreDraw( ID3D12GraphicsCommandList *cmdList )
{
	// PreDrawとPostDrawがペアで呼ばれていなければエラー Error if PreDraw and PostDraw are not called in pairs
	assert( Object3d::cmdList == nullptr );

	// コマンドリストをセット Set command list
	Object3d::cmdList = cmdList;

	// プリミティブ形状を設定 Set primitive shape
	cmdList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

void Object3d::PostDraw()
{
	// コマンドリストを解除 Cancel command list
	Object3d::cmdList = nullptr;
}

Object3d *Object3d::Create( Model *model )
{
	// 3Dオブジェクトのインスタンスを生成 Instantiate a 3D object
	Object3d *object3d = new Object3d();
	if ( object3d == nullptr ) {
		return nullptr;
	}

	// 初期化Initialization
	if ( !object3d->Initialize() ) {
		delete object3d;
		assert( 0 );
	}

	if ( model ) {
		object3d->SetModel( model );

	}

	//float scale_val = 20;
	//object3d->scale = { scale_val,scale_val,scale_val };

	return object3d;
}



bool Object3d::Initialize()
{
	// nullptrチェック nullptr check
	assert( device );

	HRESULT result;
	// 定数バッファの生成 Generate constant buffer
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), 	// アップロード可能 Uploadable
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer( (sizeof( ConstBufferDataB0 ) + 0xff) & ~0xff ),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS( &constBuffB0 ) );

	// クラス名の文字例を取得 Get character examples of class name
	name = typeid(*this).name();

	return true;
}

void Object3d::Update()
{
	// 当たり判定更新 Collision detection update
	if (collider) {
		collider->Update();
	}

	assert( camera );

	HRESULT result;

	UpdateWorldMatrix();

	const XMMATRIX &matViewProjection = camera->GetViewProjectionMatrix();

	// 定数バッファへデータ転送 Data transfer to a constant buffer
	ConstBufferDataB0 *constMap = nullptr;
	result = constBuffB0->Map( 0, nullptr, (void **)&constMap );
	constMap->mat = matWorld * matViewProjection;	// 行列の合成 Matrix composition
	constBuffB0->Unmap( 0, nullptr );

	// 当たり判定更新 Collision detection update
	if (collider) {
		collider->Update();
	}
}

void Object3d::Draw()
{
	// nullptrチェック nullptr check
	assert( device );
	assert( Object3d::cmdList );

	// モデルの割り当てがなければ描画しない Do not draw without model assignment
	if ( model == nullptr ) {
		return;
	}

	// パイプラインステートの設定 Setting the pipeline state
	cmdList->SetPipelineState( pipelineSet.pipelinestate.Get() );
	// ルートシグネチャの設定 Route signature settings
	cmdList->SetGraphicsRootSignature( pipelineSet.rootsignature.Get() );
	// 定数バッファビューをセット Set constant buffer view
	cmdList->SetGraphicsRootConstantBufferView( 0, constBuffB0->GetGPUVirtualAddress() );

	// モデル描画 Model drawing
	model->Draw( cmdList );
}

void Object3d::SetCollider(BaseCollider* collider)
{
	collider->SetObject(this);
	this->collider = collider;

	// コリジョンマネージャ登録 Collision manager registration
	CollisionManager::GetInstance()->AddCollider(collider);

	UpdateWorldMatrix();
	// コライダーを更新しておく Keep the collider updated
	collider->Update();
}

void Object3d::UpdateWorldMatrix()
{
	assert(camera);

	XMMATRIX matScale, matRot, matTrans, matScale2, matRot2, matTrans2;

	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	matScale2 = XMMatrixScaling(scale2.x, scale2.y, scale2.z);
	matRot2 = XMMatrixIdentity();
	matRot2 *= XMMatrixRotationZ(XMConvertToRadians(rotationE.z));
	matRot2 *= XMMatrixRotationX(XMConvertToRadians(rotationE.x));
	matRot2 *= XMMatrixRotationY(XMConvertToRadians(rotationE.y));
	matTrans2 = XMMatrixTranslation(positionE.x, positionE.y, positionE.z);

	// ワールド行列の合成 World matrix composition
	if (isBillboard && camera) {
		const XMMATRIX& matBillboard = camera->GetBillboardMatrix();

		//matWorld = XMMatrixIdentity();
		//matWorld *= matScale; // ワールド行列にスケーリングを反映
		//matWorld *= matRot; // ワールド行列に回転を反映
		//matWorld *= matBillboard;
		//matWorld *= matTrans; // ワールド行列に平行移動を反映

		//matWorldE = XMMatrixIdentity();
		//matWorldE *= matScale2;
		//matWorldE *= matRot2;
		//matWorldE *= matBillboard;
		//matWorldE *= matTrans2;

		//matWorld *= matWorldE;

		matWorld = XMMatrixIdentity();
		matWorld *= (matScale * matScale2);
		//matWorld *= matScale;
		matWorld *= (matRot * matRot2);
		//matWorld *= matRot;
		matWorld *= matBillboard;
		matWorld *= (matTrans * matTrans2);
		//matWorld *= matTrans;
	} else {
		//matWorld = XMMatrixIdentity(); // 変形をリセット
		//matWorld *= matScale; // ワールド行列にスケーリングを反映
		//matWorld *= matRot; // ワールド行列に回転を反映
		//matWorld *= matTrans; // ワールド行列に平行移動を反映

		//matWorldE = XMMatrixIdentity();
		//matWorldE *= matScale2;
		//matWorldE *= matRot2;
		//matWorldE *= matTrans2;

		//matWorld *= matWorldE;

		matWorld = XMMatrixIdentity();
		matWorld *= (matScale * matScale2);
		//matWorld *= matScale;
		matWorld *= (matRot * matRot2);
		//matWorld *= matRot;
		matWorld *= (matTrans * matTrans2);
		//matWorld *= matTrans;
	}

	// 親オブジェクトがあれば If there is a parent object
	if (parent != nullptr) {
		// 親オブジェクトのワールド行列を掛ける
		matWorld *= parent->matWorld;
	}
}
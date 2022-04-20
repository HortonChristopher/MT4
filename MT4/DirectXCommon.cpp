#include "DirectXCommon.h"
#include <vector>
#include <cassert>
#include "SafeDelete.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;

long t1 = timeGetTime();
int fpsskipcnt = 0;

void DirectXCommon::Initialize(WinApp* winApp)
{
	// nullptrチェック nullptr check
	assert(winApp);

	this->winApp = winApp;

	// DXGIデバイス初期化 DXGI device initialization
	if (!InitializeDXGIDevice()) {
		assert(0);
	}

	// コマンド関連初期化 Command related initialization
	if (!InitializeCommand()) {
		assert(0);
	}

	// スワップチェーンの生成 Swap chain generation
	if (!CreateSwapChain()) {
		assert(0);
	}

	// レンダーターゲット生成 Render target generation
	if (!CreateFinalRenderTargets()) {
		assert(0);
	}

	// 深度バッファ生成 Depth buffer generation
	if (!CreateDepthBuffer()) {
		assert(0);
	}

	// フェンス生成 Fence generation
	if (!CreateFence()) {
		assert(0);
	}
}

void DirectXCommon::PreDraw()
{
	// バックバッファの番号を取得（2つなので0番か1番） Get back buffer number (0 or 1 because there are 2)
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();

	// リソースバリアを変更（表示状態→描画対象） Change resource barrier (display state → drawing target)
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得 Get the handle of the descriptor heap for the render target view
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得 Get handle of descriptor heap for depth stencil view
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// レンダーターゲットをセット Set render target
	commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	// 全画面クリア Full screen clear
	ClearRenderTarget();
	// 深度バッファクリア Full screen clear
	ClearDepthBuffer();

	// ビューポートの設定 Viewport settings
	commandList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, WinApp::window_width, WinApp::window_height));
	// シザリング矩形の設定 Scissoring rectangle settings
	commandList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, WinApp::window_width, WinApp::window_height));
}

void DirectXCommon::PostDraw()
{
	// リソースバリアを変更（描画対象→表示状態）Change resource barrier (drawing target → display state)
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 命令のクローズ Instruction close
	commandList->Close();

	// コマンドリストの実行 Command list execution
	ID3D12CommandList* cmdLists[] = { commandList.Get() }; // コマンドリストの配列
	commandQueue->ExecuteCommandLists(1, cmdLists);

	// コマンドリストの実行完了を待つ Wait for command list execution to complete
	commandQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	commandAllocator->Reset(); // キューをクリア Clear the queue
	commandList->Reset(commandAllocator.Get(), nullptr);	// 再びコマンドリストを貯める準備 Preparing to save the command list again

	if (timeGetTime() - t1 < 1000 / 60)
	{
		fpsskipcnt = 0;
	}

	else
	{
		fpsskipcnt = 1;
	}

	while (timeGetTime() - t1 < 1000 / 60)
	{
		Sleep(1);
	}

	t1 = timeGetTime();

	// バッファをフリップ Flip the buffer
	swapchain->Present(1, 0);
}

void DirectXCommon::ClearRenderTarget()
{
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();

	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得 Get the handle of the descriptor heap for the render target view
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	// 全画面クリア        Red   Green Blue  Alpha  全画面クリア
	float clearColor[] = { 0.1f,0.25f, 0.5f,0.0f }; // 青っぽい色 Bluish color
	commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
}

void DirectXCommon::ClearDepthBuffer()
{
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得 Get handle of descriptor heap for depth stencil view
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// 深度バッファのクリア Clear depth buffer
	commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

bool DirectXCommon::InitializeDXGIDevice()
{
	HRESULT result = S_FALSE;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	//デバッグレイヤーをオンに	 Turn on debug layer
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
#endif

	// 対応レベルの配列 Corresponding level array
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	// DXGIファクトリーの生成 DXGI factory generation
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// アダプターの列挙用 For enumerating adapters
	std::vector<ComPtr<IDXGIAdapter1>> adapters;
	// ここに特定の名前を持つアダプターオブジェクトが入る Here is an adapter object with a specific name
	ComPtr<IDXGIAdapter1> tmpAdapter;
	for (int i = 0;
		dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++)
	{
		adapters.push_back(tmpAdapter);	// 動的配列に追加する Add to dynamic array
	}

	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc);	// アダプターの情報を取得 Get adapter information

		// ソフトウェアデバイスを回避 Avoid software devices
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}

		std::wstring strDesc = adesc.Description;	// アダプター名 Adapter name

		// Intel UHD Graphics（オンボードグラフィック）を回避 Avoid Intel UHD Graphics
		if (strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i];	// 採用 Recruitment
			break;
		}
	}

	D3D_FEATURE_LEVEL featureLevel;

	result = S_FALSE;
	for (int i = 0; i < _countof(levels); i++)
	{
		// デバイスを生成 Generate device
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(result))
		{
			// デバイスを生成できた時点でループを抜ける Exit the loop when the device can be created
			featureLevel = levels[i];
			break;
		}
	}
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	return true;
}

bool DirectXCommon::CreateSwapChain()
{
	HRESULT result = S_FALSE;
	
	// 各種設定をしてスワップチェーンを生成 Generate a swap chain with various settings
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = WinApp::window_width;
	swapchainDesc.Height = WinApp::window_height;
	
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 色情報の書式を一般的なものに Make the format of color information general
	swapchainDesc.SampleDesc.Count = 1; // マルチサンプルしない Do not multisample
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;	// バックバッファとして使えるように To be used as a back buffer
	swapchainDesc.BufferCount = 2;	// バッファ数を２つに設定 Set the number of buffers to 2
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後は速やかに破棄 Discard immediately after flipping
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // フルスクリーン切り替えを許可	 Allow full screen switching
	ComPtr<IDXGISwapChain1> swapchain1;
	HWND hwnd = winApp->GetHwnd();
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		winApp->GetHwnd(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapchain1);
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	swapchain1.As(&swapchain);

	return true;
}

bool DirectXCommon::InitializeCommand()
{
	HRESULT result = S_FALSE;

	// コマンドアロケータを生成 Generate command allocator
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(result)) {
		assert(0);
		return result;
	}
	
	// コマンドリストを生成 Generate command list
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 標準設定でコマンドキューを生成 Generate command queue by default
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	result = device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	return true;
}

bool DirectXCommon::CreateFinalRenderTargets()
{
	HRESULT result = S_FALSE;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};

	//リフレッシュレートの最大有理数
	swcDesc.BufferDesc.RefreshRate.Numerator = 60;
	//リフレッシュレートの最小有理数
	swcDesc.BufferDesc.RefreshRate.Denominator = 1;

	result = swapchain->GetDesc(&swcDesc);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 各種設定をしてディスクリプタヒープを生成 Generate descriptor heap with various settings
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;	// レンダーターゲットビュー Render target view
	heapDesc.NumDescriptors = swcDesc.BufferCount;
	result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 裏表の２つ分について About two front and back
	backBuffers.resize(swcDesc.BufferCount);
	for (int i = 0; i < backBuffers.size(); i++)
	{
		// スワップチェーンからバッファを取得 Get buffer from swap chain
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		if (FAILED(result)) {
			assert(0);
			return result;
		}

		// ディスクリプタヒープのハンドルを取得 Get the handle of the descriptor heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), i, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		// レンダーターゲットビューの生成 Generate render target view
		device->CreateRenderTargetView(
			backBuffers[i].Get(),
			nullptr,
			handle);
	}

	return true;
}

bool DirectXCommon::CreateDepthBuffer()
{
	HRESULT result = S_FALSE;

	// リソース設定 Resource settings
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		WinApp::window_width,
		WinApp::window_height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	// リソースの生成 Resource generation
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値書き込みに使用 Used for writing depth values
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuffer));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 深度ビュー用デスクリプタヒープ作成 Create descriptor heap for depth view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1; // 深度ビューは1つ 1 depth view
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // デプスステンシルビュー Depth stencil view
	result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 深度ビュー作成 Create depth view
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // 深度値フォーマット Depth value format
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}

bool DirectXCommon::CreateFence()
{
	HRESULT result = S_FALSE;

	// フェンスの生成 Fence generation
	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	return true;
}
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Audio.h"
#include "GameScene.h"

// Windowsアプリでのエントリーポイント(main関数) Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	// 汎用機能 General-purpose function
	WinApp *win = nullptr;
	DirectXCommon *dxCommon = nullptr;
	Input *input = nullptr;
	Audio *audio = nullptr;
	GameScene *gameScene = nullptr;

	// ゲームウィンドウの作成 Creating a game window
	win = new WinApp();
	win->CreateGameWindow();

	//DirectX初期化処理 DirectX initialization process
	dxCommon = new DirectXCommon();
	dxCommon->Initialize( win );

#pragma region 汎用機能初期化 General-purpose function initialization
	// 入力の初期化 Input initialization
	input = Input::GetInstance();
	if ( !input->Initialize( win->GetInstance(), win->GetHwnd() ) ) {
		assert( 0 );
		return 1;
	}
	// オーディオの初期化 Audio initialization
	audio = new Audio();
	if ( !audio->Initialize() ) {
		assert( 0 );
		return 1;
	}
	// スプライト静的初期化 Sprite static initialization
	if ( !Sprite::StaticInitialize( dxCommon->GetDevice(), WinApp::window_width, WinApp::window_height ) ) {
		assert( 0 );
		return 1;
	}

	// 3Dオブジェクト静的初期化 3D object static initialization

	Object3d::StaticInitialize( dxCommon->GetDevice() );
#pragma endregion

	// ゲームシーンの初期化 Initialize the game scene
	gameScene = new GameScene();
	gameScene->Initialize( dxCommon, input, audio );

	// メインループ Main loop
	while ( true )
	{
		
		// メッセージ処理 Message processing
		if ( win->ProcessMessage() ) { break; }


		// 入力関連の毎フレーム処理 Input-related frame processing
		input->Update();
		// ゲームシーンの毎フレーム処理 Every frame processing of the game scene
		gameScene->Update();
		// 描画開始 Start drawing
		dxCommon->PreDraw();
		// ゲームシーンの描画 Drawing the game scene
		gameScene->Draw();
		// 描画終了 End of drawing
		dxCommon->PostDraw();
	}
	// 各種解放 Various releases
	safe_delete( gameScene );
	safe_delete( audio );
	//safe_delete( input );
	safe_delete( dxCommon );

	// ゲームウィンドウの破棄 Discard the game window
	win->TerminateGameWindow();
	safe_delete( win );

	return 0;
}
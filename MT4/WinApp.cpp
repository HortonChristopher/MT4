#include "WinApp.h"

const wchar_t WinApp::windowClassName[] = L"DirectXGame";

// ウィンドウプロシージャ Window procedure
LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// メッセージで分岐 Branch by message
	switch (msg)
	{
	case WM_DESTROY: // ウィンドウが破棄された The window was destroyed
		PostQuitMessage(0); // OSに対して、アプリの終了を伝える Tell the OS that the app is closed
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // 標準の処理を行う Do standard processing
}

void WinApp::CreateGameWindow()
{
	// ウィンドウクラスの設定 Window class settings
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = (WNDPROC)WindowProc; // ウィンドウプロシージャ Window procedure
	wndClass.lpszClassName = windowClassName; // ウィンドウクラス名 Window class name
	wndClass.hInstance = GetModuleHandle(nullptr); // ウィンドウハンドル Window handle
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); // カーソル指定 Cursor specification

	RegisterClassEx(&wndClass); // ウィンドウクラスをOSに登録 Register the window class in the OS

	// ウィンドウサイズ{ X座標 Y座標 横幅 縦幅 } Window size {X coordinate Y coordinate width Vertical width}
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false); // 自動でサイズ補正Automatic size correction

	// ウィンドウオブジェクトの生成 Creating a window object
	hwnd = CreateWindow(wndClass.lpszClassName, // クラス名 name of the class
		windowClassName,			// タイトルバーの文字 Title bar text
		WS_OVERLAPPEDWINDOW,	// タイトルバーと境界線があるウィンドウ Window with title bar and border
		CW_USEDEFAULT,			// 表示X座標（OSに任せる） Display X coordinates (leave it to the OS)
		CW_USEDEFAULT,			// 表示Y座標（OSに任せる） Display Y coordinates (leave it to the OS)
		wrc.right - wrc.left,	// ウィンドウ横幅 Window width
		wrc.bottom - wrc.top,	// ウィンドウ縦幅 Window height
		nullptr,				// 親ウィンドウハンドル Parent window handle
		nullptr,				// メニューハンドル Menu handle
		wndClass.hInstance,		// 呼び出しアプリケーションハンドル Call application handle
		nullptr);				// オプション option

	// ウィンドウ表示 Window display
	ShowWindow(hwnd, SW_SHOW);
}

void WinApp::TerminateGameWindow()
{
	// ウィンドウクラスを登録解除 Unregister window class
	UnregisterClass(wndClass.lpszClassName, wndClass.hInstance);
}

bool WinApp::ProcessMessage()
{
	MSG msg{};	// メッセージ message

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // メッセージがある？ Do you have a message?
	{ 
		TranslateMessage(&msg); // キー入力メッセージの処理 Processing keystroke messages
		DispatchMessage(&msg); // ウィンドウプロシージャにメッセージを送る Send a message to a window procedure
	}

	if (msg.message == WM_QUIT) // 終了メッセージが来たらループを抜ける Exit the loop when the end message arrives
	{
		return true;
	}

	return false;
}

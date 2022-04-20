#pragma once
#include <Windows.h>
#include <time.h>
#include <stdio.h>

#define WINDOW_H_
#define WINDOW_CLASS_NAME "DirectXGame"	//!< ウィンドウクラス名 Window class name

// ウィンドウズアプリケーション Windows application
class WinApp
{
public: // 静的メンバ変数 Static member variables
	// ウィンドウサイズ Window size
	static const int window_width = 1280;	// 横幅 Width
	static const int window_height = 720;	// 縦幅 Vertical width
	static const wchar_t windowClassName[];

public: // 静的メンバ関数 Static member function
	// ウィンドウプロシージャ Window procedure
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public: // メンバ関数 Member function
	// ゲームウィンドウの作成 Creating a game window
	void CreateGameWindow();

	// ゲームウィンドウの破棄 Discard the game window
	void TerminateGameWindow();

	// メッセージの処理 Message processing
	bool ProcessMessage();

	// ウィンドウハンドルの取得 Get window handle
	HWND GetHwnd() { return hwnd; }

	//更新
	bool Update();

	//描画
	void Draw();

	//待機
	void Wait();

	HINSTANCE GetInstance() { return wndClass.hInstance; }

private: // メンバ変数 Member variables
	// Window関連 Window related
	HWND hwnd = nullptr;	// ウィンドウハンドル Window handle
	WNDCLASSEX wndClass{};			// ウィンドウクラス Window class

	int StartTime = 0;
	int Count = 0;
	float Fps = 0;
	static const int N = 60;
	static const int FPS = 60;

	char msgbuf[256];
	

};


#pragma once

#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION     0x0800          // DirectInputのバージョン指定 DirectInput version specification
#include <dinput.h>

// 入力 input
class Input
{
public: // 静的メンバ関数 Static member function
	static Input* GetInstance();

private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public: // メンバ関数 Member function
	// 初期化 Initialization
	bool Initialize(HINSTANCE hInstance, HWND hwnd);

	// 毎フレーム処理 Every frame processing
	void Update();

	// キーの押下をチェック Check key press
	bool PushKey(BYTE keyNumber);

	// キーのトリガーをチェック Check key trigger
	bool TriggerKey(BYTE keyNumber);

private: // メンバ変数 Member variables
	ComPtr<IDirectInput8> dinput;
	ComPtr<IDirectInputDevice8> devkeyboard;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};

protected:
	Input() = default;

	~Input() = default;

public:
	Input(Input& other) = delete;

	void operator=(const Input&) = delete;

};


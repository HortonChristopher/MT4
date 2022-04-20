#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")

Input* Input::GetInstance()
{
	static Input instance;
	return &instance;
}

bool Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT result = S_FALSE;

	// DirectInputオブジェクトの生成	 Creating a DirectInput object
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// キーボードデバイスの生成	Generate keyboard device
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 入力データ形式のセット Set of input data formats
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	// 排他制御レベルのセット Set of exclusive control levels
	result = devkeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(result)) {
		assert(0);
		return result;
	}

	return true;
}

void Input::Update()
{
	HRESULT result;

	result = devkeyboard->Acquire();	// キーボード動作開始 Keyboard operation starts

	// 前回のキー入力を保存 Save last keystroke
	memcpy(keyPre, key, sizeof(key));

	// キーの入力 Key input
	result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
	// 異常な引数を検出Detects anomalous arguments
	assert(0 <= keyNumber && keyNumber <= 256);

	// 0でなければ押している If it is not 0, it is pressed
	if (key[keyNumber]) {
		return true;
	}

	// 押していない Not pressed
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	// 異常な引数を検出 Detects anomalous arguments
	assert(0 <= keyNumber && keyNumber <= 256);

	// 前回が0で、今回が0でなければトリガー Trigger if last time was 0 and this time is not 0
	if (!keyPre[keyNumber] && key[keyNumber]) {
		return true;
	}

	// トリガーでない Not a trigger
	return false;
}

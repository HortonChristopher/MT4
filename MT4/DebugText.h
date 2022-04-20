#pragma once

#include "Sprite.h"
#include <Windows.h>
#include <string>

// デバッグ用文字表示 Character display for debugging
class DebugText
{
public: // 静的メンバ関数 Static member function
	static DebugText* GetInstance();

public:	
	// デバッグテキスト用のテクスチャ番号を指定 Specify texture number for debug text
	static const int maxCharCount = 256;	// 最大文字数 Maximum number of characters
	static const int fontWidth = 9;			// フォント画像内1文字分の横幅 Width of one character in the font image
	static const int fontHeight = 18;		// フォント画像内1文字分の縦幅 Vertical width of one character in the font image
	static const int fontLineCount = 14;	// フォント画像内1行分の文字数 Number of characters for one line in the font image

	DebugText();
	~DebugText();

	void Initialize(UINT texnumber);

	void Print(const std::string & text, float x, float y, float size);

	void DrawAll(ID3D12GraphicsCommandList * cmdList);

private:
	// スプライトデータの配列 Array of sprite data
	Sprite* spriteDatas[maxCharCount] = {};
	// スプライトデータ配列の添え字番号 Subscript number of the sprite data array
	int spriteIndex = 0;
};

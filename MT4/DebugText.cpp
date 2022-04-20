#include "DebugText.h"

DebugText::DebugText()
{
}

DebugText::~DebugText()
{
	for (int i = 0; i < _countof(spriteDatas); i++) {
		delete spriteDatas[i];
	}
}

DebugText* DebugText::GetInstance()
{
	static DebugText instance;
	return &instance;
}

void DebugText::Initialize(UINT texnumber)
{
	// 全てのスプライトデータについて About all sprite data
	for (int i = 0; i < _countof(spriteDatas); i++)
	{
		// スプライトを生成する Generate sprites
		spriteDatas[i] = Sprite::Create(texnumber, { 0,0 });
	}
}

// 1文字列追加 Add 1 string
void DebugText::Print(const std::string & text, float x, float y, float scale = 1.0f)
{
	// 全ての文字について About all characters
	for (int i = 0; i < text.size(); i++)
	{
		// 最大文字数超過 Exceeding the maximum number of characters
		if (spriteIndex >= maxCharCount) {
			break;
		}

		// 1文字取り出す(※ASCIIコードでしか成り立たない) Extract one character (* Only valid with ASCII code)
		const unsigned char& character = text[i];

		int fontIndex = character - 32;
		if (character >= 0x7f) {
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		// 座標計算 Coordinate calculation
		spriteDatas[spriteIndex]->SetPosition({ x + fontWidth * scale * i, y });
		spriteDatas[spriteIndex]->SetTextureRect({ (float)fontIndexX * fontWidth, (float)fontIndexY * fontHeight }, { (float)fontWidth, (float)fontHeight });
		spriteDatas[spriteIndex]->SetSize({ fontWidth * scale, fontHeight * scale });

		// 文字を１つ進める Advance one letter
		spriteIndex++;
	}
}

// まとめて描画 Draw together
void DebugText::DrawAll(ID3D12GraphicsCommandList* cmdList)
{
	// 全ての文字のスプライトについて About all character sprites
	for (int i = 0; i < spriteIndex; i++)
	{
		// スプライト描画 Sprite drawing
		spriteDatas[i]->Draw();
	}

	spriteIndex = 0;
}
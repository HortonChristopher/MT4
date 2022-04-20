#pragma once

#include <Windows.h>
#include <xaudio2.h>
#include <wrl.h>


// オーディオコールバック Audio callback
class XAudio2VoiceCallback : public IXAudio2VoiceCallback
{
public:
	// ボイス処理パスの開始時 At the start of the voice processing path
	//STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) {};
	void OnVoiceProcessingPassStart(UINT32 BytesRequired) {};
	// ボイス処理パスの終了時 At the end of the voice processing path
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {};
	// バッファストリームの再生が終了した時 When the playback of the buffer stream is finished
	STDMETHOD_(void, OnStreamEnd) (THIS) {};
	// バッファの使用開始時 When starting to use the buffer
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) {};
	// バッファの末尾に達した時 When the end of the buffer is reached
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) {
		// バッファを解放する Free the buffer
		delete[] pBufferContext;
	};
	// 再生がループ位置に達した時 When playback reaches the loop position
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) {};
	// ボイスの実行エラー時 At the time of voice execution error
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) {};
};


// オーディオ audio
class Audio
{
private: // エイリアス alias
	// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public: // サブクラス Subclass
	// チャンクヘッダ Chunk header
	struct Chunk
	{
		char	id[4]; // チャンク毎のID ID for each chunk
		int		size;  // チャンクサイズ Chunk size
	};

	// RIFFヘッダチャンク RIFF header chunk
	struct RiffHeader
	{
		Chunk	chunk;   // "RIFF"
		char	type[4]; // "WAVE"
	};

	// FMTチャンク FMT chunk
	struct FormatChunk
	{
		Chunk		chunk; // "fmt "
		WAVEFORMAT	fmt;   // 波形フォーマット Waveform format
	};

public: // メンバ関数 Member function

	
	// 初期化 Initialization
	bool Initialize();

	// サウンドファイルの読み込みと再生 Loading and playing sound files
	void PlayWave(const char* filename);

private: // メンバ変数 Member variables
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	XAudio2VoiceCallback voiceCallback;
};


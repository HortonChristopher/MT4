#include "Audio.h"
#include <fstream>
#include <cassert>

#pragma comment(lib,"xaudio2.lib")

bool Audio::Initialize()
{
	HRESULT result;

	// XAudioエンジンのインスタンスを生成 Instantiate the XAudio engine
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if FAILED(result) {
		assert(0);
		return false;
	}

	// マスターボイスを生成 Generate Master Voice
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	if FAILED(result) {
		assert(0);
		return false;
	}

	return true;
}

void Audio::PlayWave(const char * filename)
{
	HRESULT result;
	// ファイルストリーム Filestream
	std::ifstream file;
	// Waveファイルを開く Open Wave File
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗をチェック Check file open failure
	if (file.fail()) {
		assert(0);
	}

	// RIFFヘッダーの読み込み Read RIFF header
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック Check if file is RIFF
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み Read Format Chunk
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	// Dataチャンクの読み込み Loading Data Chunk
	Chunk data;
	file.read((char*)&data, sizeof(data));

	// Dataチャンクのデータ部（波形データ）の読み込み Reading data part (waveform data) of the Data chunk
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる Close Wave File
	file.close();

	WAVEFORMATEX wfex{};
	// 波形フォーマットの設定 Waveform Format Settings
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;

	// 波形フォーマットを元にSourceVoiceの生成 Generate SourceVoice based on waveform format
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f, &voiceCallback);
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}

	// 再生する波形データの設定 Setting of waveform data to be played
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)pBuffer;
	buf.pContext = pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;

	// 波形データの再生 Playback of waveform data
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}

	result = pSourceVoice->Start();
	if FAILED(result) {
		delete[] pBuffer;
		assert(0);
		return;
	}
}

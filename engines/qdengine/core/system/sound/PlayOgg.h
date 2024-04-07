#ifndef _PLAYMPP_H_
#define _PLAYMPP_H_
#pragma once
//Balmer

#include <dsound.h>

#define MPP_STAT
#ifdef MPP_STAT
double MpegCPUUsing();//Возвращает используемое на проигрывание Mpeg время (1 - всё время загрузки)
#endif MPP_STAT

//
bool MpegInitLibrary(void* LPDIRECTSOUND_pDS);
void MpegDeinitLibrary();

enum MpegState
{
	MPEG_STOP=0,
	MPEG_PLAY=1,
	MPEG_PAUSE=2,
};

//Возвращает длинну в секундах (но не очень точно, 
//может ошибаться на 26 мс)
//Хоть эта фонкция и существует, пользоваться ей не 
//рекомендуется. В правильно написанном коде игры 
//такая функция не нужна
double MpegGetLen(const char* fname);

typedef double MpegPos;

class MpegSound
{
	class MppLoad* pMppLoad;
	MpegSound *prev,*next;

	DWORD sizeDSBuffer;
	bool b_cycled;
	long volume;

	WAVEFORMATEX  wave_format;
	LPDIRECTSOUNDBUFFER pDSBuffer;
	DWORD dwWriteOffset;
	DWORD Wraps;//Сколько раз был записан звуковой буффер
	DWORD BeginBufferOffset;
	DWORD OldWraps,OldBeginBufferOffset;
	DWORD OffsetBeginPlayFile;//С какого места начал играться файл

	bool bOldFrame;//Установлен, если играются старые данные, а пишутся новые

	char fname[260];

	bool clear_end_buffer;
	MpegState mpeg_state;

	DWORD deferred_prev_sample;
	DWORD SeekSkipByte;//Сколько байт необходимо пропустить с начала фрэйма
	MpegPos deferred_sample;
	char deferred_fname[260];

	enum {block_size=36*32};//block_size==BLK_SIZE
	short last_signal[block_size*2];
	bool last_signal_is_full;
	int  last_signal_offset;

	void HammingCorrect(short* pData,int len);

	bool enable_fade;
	double fade_begin_time,fade_time;
	long fade_begin_volume,fade_end_volume;
public:
	MpegSound();
	~MpegSound();

	bool OpenToPlay(const char* fname,bool cycled=true);
	void Stop();
	void Pause();
	void Resume();

	const char* GetFileName();
	//Не играть музыку, если громкость музыки равна 0
	//(по умолчанию музыка играется)
	void SetPauseIfNullVolume(bool set=true);

	MpegState IsPlay();

	bool DebugRealPlay();//Действительно ли проигрывается музыка

	void SetVolume(int volume);//0..255
	int GetVolume();

	inline int GetSamplePerSecond(){return 44100;}
	//Время считается в секундах

	//Возвращает величину буфера в сэмплах
	inline int GetBufferLen(){return sizeDSBuffer/4;}

	//Постепенно изменить громкость с текущей до new_volume за время time
	//очищается при смене файла или вызове SetVolume
	//Криво работает с DeferredSeek
	bool FadeVolume(float time,int new_volume=0);

	float GetLen();//в секундах
	float GetCurPos();//в секундах (неточно)
protected:
	void InternalMpegSetVolume(int _volume);
	bool InitSoundBuffer();
	void InternalMpegStop();
	bool InternalMpegOpenToPlay(const char* _fname,bool cycled);

	void TimeCallbackTrue();

	//В какое место буфера надо будет писать, 
	//если в него записали offset байт
	void AddWriteOffset(DWORD offset);

	friend DWORD WINAPI MpegThreadProc(LPVOID lpParameter);
	friend void MpegDeinitLibrary();

	bool DefferredSeek(DWORD cur_pos,DWORD cur_write_byte);

	void ClearFade();
	void FadeQuant();
};

#endif // _PLAYMPP_H_

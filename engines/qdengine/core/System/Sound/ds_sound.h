#ifndef __DS_SOUND_H__
#define __DS_SOUND_H__

#include <dsound.h>

#include "snd_sound.h"

class wavSound;

//! DirectSound звук.
class dsSound : public sndSound
{
public:
	dsSound(const sndSound& snd,LPDIRECTSOUND sound_device);
	dsSound(const dsSound& snd);
	~dsSound();

	//! Запускает проигрывание звука.
	bool play();
	//! Останавливает проигрывание звука.
	bool stop();
	//! Ставит звук на паузу.
	void pause();
	//! Возобновляет проигрывание.
	void resume();
	//! Возвращает true, если звук на паузе.
	bool is_paused() const { if(flags_ & SOUND_FLAG_PAUSED) return true; else return false; }

	//! Возвращает true, если звук не проигрывается.
	bool is_stopped() const;
	//! Возвращает состояние звука.
	sndSound::status_t status() const;

	//! Устанавливает громкость звука, параметр - в децибелах.
	/**
	Диапазон значений громкости - [-10000, 0]

	-10000 - звук совсем не слышен,
	0 - громкость самого звука по умолчанию.
	*/
	bool set_volume(int vol);

	bool change_frequency(float coeff = 1.0f);

	//! Возвращает текущую позицию звука, диапазон возвращаемых значений - [0.0, 1.0].
	float position() const;
	//! Устанавливает текущую позицию звука, диапазон значений параметра - [0.0, 1.0].
	bool set_position(float pos);

	//! Создает DirectSoundBuffer.
	bool create_sound_buffer();
	//! Удаляет DirectSoundBuffer.
	bool release_sound_buffer();

	//! Включает/выключает зацикливание звука.
	void toggle_looping(){ flags_ ^= SOUND_FLAG_LOOPING; }

private:
	//! Указатель на объект DirectSound.
	const LPDIRECTSOUND sound_device_;
	//! Указатель на DirectSoundBuffer.
	LPDIRECTSOUNDBUFFER  sound_buffer_;

	//! флаги
	enum {
		SOUND_FLAG_LOOPING	= 0x01,
		SOUND_FLAG_PAUSED	= 0x02
	};

	//! флаги
	int flags_;
};

#endif /* __DS_SOUND_H__ */


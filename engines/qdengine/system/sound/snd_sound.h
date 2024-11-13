/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef QDENGINE_SYSTEM_SOUND_SND_SOUND_H
#define QDENGINE_SYSTEM_SOUND_SND_SOUND_H

#include "audio/mixer.h"

namespace QDEngine {

class wavSound;
class qdNamedObject;

//! Класс для управления звуками.
class sndHandle {
public:
	sndHandle() { };
	virtual ~sndHandle() { };
};

//! Базовый класс для звуков.
class sndSound {
public:
	explicit sndSound(const wavSound *snd, const sndHandle *h = NULL) : _sound(snd), _handle(h), _flags(0) {}
	~sndSound();

	//! Состояние звука.
	enum status_t {
		//! звук не проигрывается
		SOUND_STOPPED,
		//! звук приостановлен
		SOUND_PAUSED,
		//! звук пригрывается
		SOUND_PLAYING
	};

	//! Возвращает состояние звука.
	status_t status() const;

	//! Возвращает указатель на данные звука.
	const wavSound *sound() const {
		return _sound;
	}
	//! Возвращает указатель на хэндл звука.
	const sndHandle *handle() const {
		return _handle;
	}

	//! Запускает проигрывание звука.
	bool play();
	//! Останавливает проигрывание звука.
	bool stop(bool rewind = true);
	//! Ставит звук на паузу.
	void pause();
	//! Возобновляет проигрывание.
	void resume();
	//! Возвращает true, если звук на паузе.
	bool is_paused() const {
		if (_flags & SOUND_FLAG_PAUSED) return true;
		else return false;
	}

	//! Возвращает true, если звук не проигрывается.
	bool is_stopped() const;

	//! Устанавливает громкость звука, параметр - в децибелах.
	/**
	Диапазон значений громкости - [-10000, 0]

	-10000 - звук совсем не слышен,
	0 - громкость самого звука по умолчанию.
	*/
	bool set_volume(int vol);

	bool change_frequency(float coeff = 1.0f);

	//! Создает DirectSoundBuffer.
	bool create_sound_buffer();
	//! Удаляет DirectSoundBuffer.
	bool release_sound_buffer();

	//! Включает/выключает зацикливание звука.
	void toggle_looping() {
		_flags ^= SOUND_FLAG_LOOPING;
	}

private:
	//! Указатель на данные.
	const wavSound *_sound;
	//! Указатель на хэндл звука.
	const sndHandle *_handle;

	//! Указатель на DirectSoundBuffer.
	//! флаги
	enum {
		SOUND_FLAG_LOOPING  = 0x01,
		SOUND_FLAG_PAUSED   = 0x02
	};

	//! флаги
	int _flags;

	Audio::SoundHandle _audHandle;

	bool _isStopped = false;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_SND_SOUND_H

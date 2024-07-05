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

#ifndef QDENGINE_SYSTEM_SOUND_DS_SOUND_H
#define QDENGINE_SYSTEM_SOUND_DS_SOUND_H

#include "qdengine/system/sound/snd_sound.h"
#include "qdengine/qd_precomp.h"


namespace QDEngine {

class wavSound;

//! DirectSound звук.
class dsSound : public sndSound {
public:
	dsSound(const sndSound &snd, LPDIRECTSOUND sound_device);
	dsSound(const dsSound &snd);
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
	bool is_paused() const {
		if (flags_ & SOUND_FLAG_PAUSED) return true;
		else return false;
	}

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
	void toggle_looping() {
		flags_ ^= SOUND_FLAG_LOOPING;
	}

private:
	//! Указатель на объект DirectSound.
	const LPDIRECTSOUND sound_device_;
	//! Указатель на DirectSoundBuffer.
	LPDIRECTSOUNDBUFFER  sound_buffer_;

	//! флаги
	enum {
		SOUND_FLAG_LOOPING  = 0x01,
		SOUND_FLAG_PAUSED   = 0x02
	};

	//! флаги
	int flags_;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_DS_SOUND_H

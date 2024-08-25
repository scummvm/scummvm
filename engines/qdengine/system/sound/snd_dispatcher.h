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

#ifndef QDENGINE_SYSTEM_SOUND_SND_DISPATCHER_H
#define QDENGINE_SYSTEM_SOUND_SND_DISPATCHER_H

#include "qdengine/system/sound/snd_sound.h"

namespace QDEngine {

//! Диспетчер звуков на DirectSound.
class sndDispatcher {
public:
	sndDispatcher();
	~sndDispatcher();

	//! Логический квант.
	void quant();
	//! Запускает проигрывание звука.
	bool play_sound(const sndSound *snd, bool loop, int vol = 255);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndSound *snd);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndHandle *handle);
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndHandle *handle) const;
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndSound *snd) const;
	//! Изменение частоты звука.
	bool set_sound_frequency(const sndHandle *snd, float coeff);

	//! Изменение громкости, диапазон значений - [0, 255].
	void set_volume(uint32 vol);

	uint32 volume() const {
		return _volume;
	}

	//! Возвращает установленную громкость в децибелах.
	int volume_dB() const {
		return _volume_dB;
	}

	void set_frequency_coeff(float coeff) {
		_frequency_coeff = coeff;
		update_frequency();
	}
	float frequency_coeff() const {
		return _frequency_coeff;
	}

	//! Пересчет громкости в децибелы.
	static int convert_volume_to_dB(int vol);

	//! Останавливает все звуки.
	void stop_sounds();
	//! Ставит все играющие в данный момент звуки на паузу.
	void pause_sounds();
	//! Возобновляет проигрывание всех звуков, которые были поставлены на паузу.
	void resume_sounds();

	//! Ставит все звуки на паузу до вызова resume().
	void pause() {
		_is_paused = true;
		pause_sounds();
	}
	//! Возобновляет проигрывание всех звуков.
	void resume() {
		_is_paused = false;
		resume_sounds();
	}
	//! Возвращает true, если звуки поставлены на паузу.
	bool is_paused() const {
		return _is_paused;
	}

	//! Возвращает true, если звук выключен.
	bool is_enabled() const {
		return _is_enabled;
	}
	//! Включает звук.
	void enable() {
		_is_enabled = true;
	}
	//! Выключает звук.
	void disable() {
		_is_enabled = false;
		stop_sounds();
	}

	void syncSoundSettings();

	//! Возвращает указатель на текущий диспетчер.
	static inline sndDispatcher *get_dispatcher() {
		return _dispatcher_ptr;
	}
	//! Устанавливает указатель на текущий диспетчер.
	static inline sndDispatcher *set_dispatcher(sndDispatcher *p) {
		sndDispatcher *old_p = _dispatcher_ptr;
		_dispatcher_ptr = p;
		return old_p;
	}

protected:

	//! Обновление установки громкости.
	bool update_volume();

	bool update_frequency();

private:

	//! Звук выключен, если false.
	bool _is_enabled;

	//! Громкость, диапазон значений - [0, 255].
	/**
	0 - звук полностью давится
	255 - звук играется в полную громкость
	*/
	uint32 _volume;

	//! Громкость в децибелах, диапазон значений - [-10000, 0].
	/**
	-10000 - звук полностью давится
	0 - звук играется в полную громкость
	*/
	int _volume_dB;

	float _frequency_coeff;

	//! Пауза.
	bool _is_paused;

	typedef Std::list<sndSound> sound_list_t;
	//! Список активных звуков.
	sound_list_t _sounds;

	//! Текущий диспетчер.
	static sndDispatcher *_dispatcher_ptr;

//	Audio::SeekableAudioStream *_audioStream;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_SND_DISPATCHER_H

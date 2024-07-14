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

class wavSound;
//! Диспетчер звука.
class sndDispatcher {
public:
	sndDispatcher();
	virtual ~sndDispatcher();

	//! Логический квант.
	virtual void quant() = 0;
	//! Запускает проигрывание звука.
	virtual bool play_sound(const sndSound *snd, bool loop, float start_position = 0.0f, int vol = 255) = 0;
	//! Останавливает проигрывание звука.
	virtual bool stop_sound(const sndSound *snd) = 0;
	//! Останавливает проигрывание звука.
	virtual bool stop_sound(const sndHandle *handle) = 0;
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	virtual sndSound::status_t sound_status(const sndHandle *handle) const = 0;
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	virtual sndSound::status_t sound_status(const sndSound *snd) const = 0;
	//! Возвращает текущую позицию звука, от 0.0 (начало) до 1.0 (конец).
	virtual float sound_position(const sndHandle *snd) const = 0;
	//! Изменение частоты звука.
	virtual bool set_sound_frequency(const sndHandle *snd, float coeff) = 0;

	//! Изменение громкости, диапазон значений - [0, 255].
	void set_volume(unsigned int vol);
	//! Возвращает установленную громкость, диапазон значений - [0, 255].
	unsigned int volume() const {
		return volume_;
	}

	//! Возвращает установленную громкость в децибелах.
	int volume_dB() const {
		return volume_dB_;
	}

	void set_frequency_coeff(float coeff) {
		frequency_coeff_ = coeff;
		update_frequency();
	}
	float frequency_coeff() const {
		return frequency_coeff_;
	}

	//! Пересчет громкости в децибелы.
	static int convert_volume_to_dB(int vol);

	//! Останавливает все звуки.
	virtual void stop_sounds() = 0;
	//! Ставит все играющие в данный момент звуки на паузу.
	virtual void pause_sounds() = 0;
	//! Возобновляет проигрывание всех звуков, которые были поставлены на паузу.
	virtual void resume_sounds() = 0;

	//! Ставит все звуки на паузу до вызова resume().
	void pause() {
		is_paused_ = true;
		pause_sounds();
	}
	//! Возобновляет проигрывание всех звуков.
	void resume() {
		is_paused_ = false;
		resume_sounds();
	}
	//! Возвращает true, если звуки поставлены на паузу.
	bool is_paused() const {
		return is_paused_;
	}

	//! Возвращает true, если звук выключен.
	bool is_enabled() const {
		return is_enabled_;
	}
	//! Включает звук.
	void enable() {
		is_enabled_ = true;
	}
	//! Выключает звук.
	void disable() {
		is_enabled_ = false;
		stop_sounds();
	}

	//! Возвращает указатель на текущий диспетчер.
	static inline sndDispatcher *get_dispatcher() {
		return dispatcher_ptr_;
	}
	//! Устанавливает указатель на текущий диспетчер.
	static inline sndDispatcher *set_dispatcher(sndDispatcher *p) {
		sndDispatcher *old_p = dispatcher_ptr_;
		dispatcher_ptr_ = p;
		return old_p;
	}

protected:

	//! Обновление установки громкости.
	virtual bool update_volume() = 0;
	virtual bool update_frequency() = 0;

private:

	//! Звук выключен, если false.
	bool is_enabled_;

	//! Громкость, диапазон значений - [0, 255].
	/**
	0 - звук полностью давится
	255 - звук играется в полную громкость
	*/
	unsigned int volume_;

	//! Громкость в децибелах, диапазон значений - [-10000, 0].
	/**
	-10000 - звук полностью давится
	0 - звук играется в полную громкость
	*/
	int volume_dB_;

	float frequency_coeff_;

	//! Пауза.
	bool is_paused_;

	//! Текущий диспетчер.
	static sndDispatcher *dispatcher_ptr_;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_SND_DISPATCHER_H

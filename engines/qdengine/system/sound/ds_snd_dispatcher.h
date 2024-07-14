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

#ifndef QDENGINE_SYSTEM_SOUND_DS_SND_DISPATCHER_H
#define QDENGINE_SYSTEM_SOUND_DS_SND_DISPATCHER_H

#include "common/textconsole.h"
#include "qdengine/system/sound/snd_dispatcher.h"
#include "qdengine/system/sound/ds_sound.h"

namespace QDEngine {

//! Диспетчер звуков на DirectSound.
class ds_sndDispatcher : public sndDispatcher {
public:
	ds_sndDispatcher();
	~ds_sndDispatcher();

	//! Логический квант.
	void quant();
	//! Запускает проигрывание звука.
	bool play_sound(const sndSound *snd, bool loop, float start_position = 0.0f, int vol = 255);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndSound *snd);
	//! Останавливает проигрывание звука.
	bool stop_sound(const sndHandle *handle);
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndHandle *handle) const;
	//! Возвращает состояние звука (играется/остановлен и т.д.).
	sndSound::status_t sound_status(const sndSound *snd) const;
	//! Возвращает текущую позицию звука, от 0.0 (начало) до 1.0 (конец).
	float sound_position(const sndHandle *snd) const;
	//! Изменение частоты звука.
	bool set_sound_frequency(const sndHandle *snd, float coeff);

	//! Останавливает все звуки.
	void stop_sounds();
	//! Ставит все играющие в данный момент звуки на паузу.
	void pause_sounds();
	//! Возобновляет проигрывание всех звуков, которые были поставлены на паузу.
	void resume_sounds();

protected:

	//! Обновление установки громкости.
	bool update_volume();

	bool update_frequency();

private:

	typedef std::list<dsSound> sound_list_t;
	//! Список активных звуков.
	sound_list_t _sounds;

	//! Указатель на DirectSound интерфейс.
	LPDIRECTSOUND _sound_device;
//	Audio::SeekableAudioStream *_audioStream;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_DS_SND_DISPATCHER_H

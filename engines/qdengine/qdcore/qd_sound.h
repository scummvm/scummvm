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

#ifndef QDENGINE_QDCORE_QD_SOUND_H
#define QDENGINE_QDCORE_QD_SOUND_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_resource.h"
#include "qdengine/qdcore/qd_sound_handle.h"

#include "qdengine/system/sound/wav_sound.h"

namespace QDEngine {

//! Звук.
class qdSound : public qdNamedObject, public qdResource {
public:
	qdSound();
	~qdSound();

	int named_object_type() const {
		return QD_NAMED_OBJECT_SOUND;
	}

	//! Загрузка данных из скрипта.
	void load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	bool load_resource();
	bool free_resource();
	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const Common::Path file_name) {
		set_file_name(file_name);
	}
	//! Возвращает имя файла, в котором хранится анимация.
	const Common::Path resource_file() const {
		return file_name();
	}
#ifdef __QD_DEBUG_ENABLE__
	uint32 resource_data_size() const {
		return _sound.data_length();
	}
#endif

	//! Возвращает имя файла, в котором хранится звук.
	const Common::Path file_name() const {
		return _file_name;
	}
	//! Устанавливает имя файла, в котором хранится звук.
	void set_file_name(const Common::Path fname) {
		_file_name = fname;
	}

	//! Запускает проигрывание звука.
	/**
	handle нужен для дальнейшего управления звуком и отслеживания его состояния,
	start_position - начальная позиция при запуске звука, значения - [0.0, 1.0]
	*/
	bool play(const qdSoundHandle *handle = NULL, bool loop = false) const;
	//! Останавливает проигрывание звука.
	/**
	Если handle нулевой - то выключится первый попавшийся активный звук с данными
	из sound_, если ненулевой - первый соответствующий этому хэндлу.
	*/
	bool stop(const qdSoundHandle *handle = NULL) const;

	//! Изменение частоты звука в frequency_coeff число раз.
	bool set_frequency(const qdSoundHandle *handle, float frequency_coeff) const;

	//! Возвращает длительность звука в секундах.
	float length() const {
		return _sound.length();
	}

	//! Возвращает true, если звук не проигрывается.
	bool is_stopped(const qdSoundHandle *handle = NULL) const;

	int volume() const {
		return _volume;
	}
	void set_volume(int vol) {
		_volume = vol;
	}

private:

	Common::Path _file_name;
	//! Sound volume, [0, 255].
	int _volume;
	wavSound _sound;
};

typedef Std::list<qdSound *> qdSoundList;

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SOUND_H

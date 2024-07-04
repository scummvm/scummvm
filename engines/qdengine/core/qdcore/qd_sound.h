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

#ifndef QDENGINE_CORE_QDCORE_QD_SOUND_H
#define QDENGINE_CORE_QDCORE_QD_SOUND_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/qdcore/qd_resource.h"
#include "qdengine/core/qdcore/qd_sound_handle.h"


#ifndef __QD_SYSLIB__
#include "qdengine/core/system/sound/wav_sound.h"
#endif

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
	bool save_script(Common::SeekableWriteStream &fh, int indent = 0) const;

	bool load_resource();
	bool free_resource();
	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	void set_resource_file(const char *file_name) {
		set_file_name(file_name);
	}
	//! Возвращает имя файла, в котором хранится анимация.
	const char *resource_file() const {
		return file_name();
	}
#ifdef __QD_DEBUG_ENABLE__
	unsigned resource_data_size() const {
		return sound_.data_length();
	}
#endif

	//! Возвращает имя файла, в котором хранится звук.
	const char *file_name() const {
		if (!file_name_.empty()) return file_name_.c_str();
		return 0;
	}
	//! Устанавливает имя файла, в котором хранится звук.
	void set_file_name(const char *fname) {
		file_name_ = fname;
	}

	//! Запускает проигрывание звука.
	/**
	handle нужен для дальнейшего управления звуком и отслеживания его состояния,
	start_position - начальная позиция при запуске звука, значения - [0.0, 1.0]
	*/
	bool play(const qdSoundHandle *handle = NULL, bool loop = false, float start_position = 0.0f) const;
	//! Останавливает проигрывание звука.
	/**
	Если handle нулевой - то выключится первый попавшийся активный звук с данными
	из sound_, если ненулевой - первый соответствующий этому хэндлу.
	*/
	bool stop(const qdSoundHandle *handle = NULL) const;

	//! Возвращает текущую позицию звука, диапазон возвращаемых значений - [0.0, 1.0].
	/**
	Если звук не играется, текущая позиция считается нулевой.
	*/
	float position(const qdSoundHandle *handle = NULL) const;

	//! Изменение частоты звука в frequency_coeff число раз.
	bool set_frequency(const qdSoundHandle *handle, float frequency_coeff) const;

	//! Возвращает длительность звука в секундах.
	float length() const {
#ifndef __QD_SYSLIB__
		return sound_.length();
#endif
	}

	//! Возвращает true, если звук не проигрывается.
	bool is_stopped(const qdSoundHandle *handle = NULL) const;

	int volume() const {
		return volume_;
	}
	void set_volume(int vol) {
		volume_ = vol;
	}

private:

	//! Имя файла.
	std::string file_name_;
	//! Громкость звука, [0, 255].
	int volume_;
	//! Данные.
#ifndef __QD_SYSLIB__
	wavSound sound_;
#endif
};

typedef std::list<qdSound *> qdSoundList;

} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_SOUND_H

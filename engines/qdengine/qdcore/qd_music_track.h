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

#ifndef QDENGINE_QDCORE_QD_MUSIC_TRACK_H
#define QDENGINE_QDCORE_QD_MUSIC_TRACK_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_conditional_object.h"


namespace QDEngine {

//! Музыкальная композиция.
/**
Формат - mp+.
*/

enum qdMusicTrackFlags {
	QD_MUSIC_TRACK_CYCLED               = 1,
	QD_MUSIC_TRACK_DISABLE_RESTART      = 2,
	QD_MUSIC_TRACK_DISABLE_SWITCH_OFF   = 4
};

class qdMusicTrack : public qdConditionalObject {
public:
	qdMusicTrack();
	qdMusicTrack(const qdMusicTrack &trk);
	~qdMusicTrack();

	qdMusicTrack &operator = (const qdMusicTrack &trk);

	int named_object_type() const {
		return QD_NAMED_OBJECT_MUSIC_TRACK;
	}

	void set_file_name(const Common::Path fname) {
		_file_name = fname;
	}
	const Common::Path file_name() const {
		return _file_name;
	}
	bool has_file_name() const {
		return !_file_name.empty();
	}

	void toggle_cycle(bool v) {
		if (v) set_flag(QD_MUSIC_TRACK_CYCLED);
		else drop_flag(QD_MUSIC_TRACK_CYCLED);
	}
	bool is_cycled() const {
		return check_flag(QD_MUSIC_TRACK_CYCLED);
	}

	int volume() const {
		return _volume;
	}
	void set_volume(int vol) {
		if (vol < 0) vol = 0;
		else if (vol > 256) vol = 256;
		_volume = vol;
	}

	//! Чтение данных из скрипта.
	bool load_script(const xml::tag *p);
	//! Запись данных в скрипт.
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	qdConditionalObject::trigger_start_mode trigger_start();

	static Common::String flag2str(int fl);

private:

	//! Имя mp+ файла.
	Common::Path _file_name;

	//! Громкость, [0, 256].
	int _volume;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_MUSIC_H

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

#ifndef QDENGINE_QDCORE_QD_VIDEO_H
#define QDENGINE_QDCORE_QD_VIDEO_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_sprite.h"
#include "qdengine/qdcore/qd_conditional_object.h"
#include "qdengine/qdcore/qd_file_owner.h"


namespace QDEngine {

//! Видеовставка.
class qdVideo : public qdConditionalObject, public qdFileOwner {
public:
	enum { // flags
		//! расположить в центре экрана
		VID_CENTER_FLAG         = 0x01,
		//! растягивать на весь экран
		VID_FULLSCREEN_FLAG     = 0x02,
		//! нельзя прерывать ролик нажатием мыши/кнопки
		VID_DISABLE_INTERRUPT_FLAG  = 0x04,
		//! показывать ролик при запуске игры до главного меню
		VID_INTRO_MOVIE         = 0x08,
		//! не прерывать музыку при показе видео
		VID_ENABLE_MUSIC        = 0x10
	};

	qdVideo();
	qdVideo(const qdVideo &v);
	~qdVideo();

	qdVideo &operator = (const qdVideo &v);
	int named_object_type() const {
		return QD_NAMED_OBJECT_VIDEO;
	}

	const Common::Path file_name() const {
		return _file_name;
	}
	void set_file_name(const Common::Path fname) {
		_file_name = fname;
	}

	// Фон, на котором будет проигрываться видео
	void set_background_file_name(const Common::Path fname) {
		_background.set_file(fname);
	}
	const Common::Path background_file_name() const {
		return _background.file();
	}
	bool draw_background();

	// Экранные координаты верхнего левого угла
	const Vect2s &position() const {
		return _position;
	}
	void set_position(const Vect2s pos) {
		_position = pos;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	qdConditionalObject::trigger_start_mode trigger_start();

	bool is_intro_movie() const {
		return check_flag(VID_INTRO_MOVIE);
	}

private:

	Vect2s _position;
	Common::Path _file_name;

	qdSprite _background;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_VIDEO_H

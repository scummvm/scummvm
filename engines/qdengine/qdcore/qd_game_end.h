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

#ifndef QDENGINE_QDCORE_QD_GAME_END_H
#define QDENGINE_QDCORE_QD_GAME_END_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/qdcore/qd_conditional_object.h"

namespace QDEngine {

//! Концовка игры.
class qdGameEnd : public qdConditionalObject {
public:
	qdGameEnd();
	qdGameEnd(const qdGameEnd &end);

	qdGameEnd &operator = (const qdGameEnd &end);

	~qdGameEnd();

	const char *interface_screen() const {
		return _interface_screen.c_str();
	}
	bool has_interface_screen() const {
		return !_interface_screen.empty();
	}
	void set_interface_screen(const char *name) {
		if (name) _interface_screen = name;
		else _interface_screen.clear();
	}

	qdConditionalObject::trigger_start_mode trigger_start();

	int named_object_type() const {
		return QD_NAMED_OBJECT_GAME_END;
	};

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

private:

	//! Имя интерфейсного экрана, на который выходить из игры.
	/**
	Если строка пустая - будет выход в main menu.
	*/
	Common::String _interface_screen;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_END_H

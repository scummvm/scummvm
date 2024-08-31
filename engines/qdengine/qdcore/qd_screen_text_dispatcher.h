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

#ifndef QDENGINE_QDCORE_QD_SCREEN_TEXT_DISPATCHER
#define QDENGINE_QDCORE_QD_SCREEN_TEXT_DISPATCHER

#include "qdengine/qdcore/qd_screen_text_set.h"
#include "qdengine/system/input/mouse_input.h"


namespace QDEngine {

//! Диспетчер экранных текстов.
class qdScreenTextDispatcher {
public:
	qdScreenTextDispatcher();
	~qdScreenTextDispatcher();

	//! Очистка всех текстов.
	void clear_texts();
	//! Очистка всех текстов с владельцем owner.
	void clear_texts(qdNamedObject *owner);

	//! Добавление текста в определенный набор.
	qdScreenText *add_text(int set_ID, const qdScreenText &txt);
	//! Добавление набора текстов.
	qdScreenTextSet *add_text_set(const qdScreenTextSet &set) {
		_text_sets.push_back(set);
		return &_text_sets.back();
	}
	//! Поиск набора текстов.
	const qdScreenTextSet *get_text_set(int id) const;
	//! Поиск набора текстов.
	qdScreenTextSet *get_text_set(int id);

	//! Отрисовка текстов.
	void redraw() const;
	void pre_redraw() const;
	void post_redraw();

	bool save_script(Common::WriteStream &fh, int indent = 0) const;

private:

	typedef Std::vector<qdScreenTextSet> text_sets_container_t;
	//! Наборы текстов.
	text_sets_container_t _text_sets;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SCREEN_TEXT_DISPATCHER

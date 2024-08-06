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

#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_screen_text_dispatcher.h"


namespace QDEngine {

qdScreenTextDispatcher::qdScreenTextDispatcher() {
	_text_sets.reserve(16);
}

qdScreenTextDispatcher::~qdScreenTextDispatcher() {
}


void qdScreenTextDispatcher::redraw() const {
}

static bool operator == (const qdScreenTextSet &set, int id) {
	return set.ID() == id;
}

qdScreenText *qdScreenTextDispatcher::add_text(int set_ID, const qdScreenText &txt) {
	Std::vector<qdScreenTextSet>::iterator it = Common::find(_text_sets.begin(), _text_sets.end(), set_ID);

	if (it != _text_sets.end())
		return it->add_text(txt);

	return NULL;
}

void qdScreenTextDispatcher::clear_texts() {
	for (auto &it : _text_sets) {
		it.clear_texts();
	}
}

void qdScreenTextDispatcher::clear_texts(qdNamedObject *p) {
	for (auto &it : _text_sets) {
		it.clear_texts(p);
	}
}

const qdScreenTextSet *qdScreenTextDispatcher::get_text_set(int id) const {
	Std::vector<qdScreenTextSet>::const_iterator it = Common::find(_text_sets.begin(), _text_sets.end(), id);

	if (it != _text_sets.end())
		return &*it;

	return NULL;
}

qdScreenTextSet *qdScreenTextDispatcher::get_text_set(int id) {
	Std::vector<qdScreenTextSet>::iterator it =
	    Common::find(_text_sets.begin(), _text_sets.end(), id);

	if (it != _text_sets.end())
		return &*it;

	return NULL;
}

void qdScreenTextDispatcher::pre_redraw() const {
}

void qdScreenTextDispatcher::post_redraw() {
}

bool qdScreenTextDispatcher::save_script(Common::WriteStream &fh, int indent) const {
	for (auto &it : _text_sets) {
		it.save_script(fh, indent);
	}
	return true;
}

} // namespace QDEngine

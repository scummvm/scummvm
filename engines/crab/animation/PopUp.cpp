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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/animation/PopUp.h"

namespace Crab {

using namespace pyrodactyl::anim;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load from xml
//------------------------------------------------------------------------
void PopUp::load(rapidxml::xml_node<char> *node) {
	_duration.load(node, "duration", false);
	_delay.load(node, "delay");
	loadStr(_text, "text", node);
	loadNum(_next, "next", node);

	bool end = false;
	loadBool(end, "end", node, false);
	if (end)
		_next = -1;

	_visible.load(node);

	_effect.clear();
	for (rapidxml::xml_node<char> *n = node->first_node("effect"); n != NULL; n = n->next_sibling("effect")) {
		Effect e;
		e.load(n);
		_effect.push_back(e);
	}
}

void PopUpCollection::load(rapidxml::xml_node<char> *node) {
	loadBool(_loop, "loop", node);
	for (auto n = node->first_node("dialog"); n != NULL; n = n->next_sibling("dialog"))
		_element.push_back(n);
}

//------------------------------------------------------------------------
// Purpose: Internal events
//------------------------------------------------------------------------
bool PopUp::internalEvents(pyrodactyl::event::Info &info, const Common::String &playerId,
						   Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &endSeq) {
	if (_visible.evaluate(info) || _startedShow) {
		if (_delay.TargetReached()) {
			if (_duration.TargetReached(g_engine->_screenSettings->text_speed)) {
				_show = false;

				for (auto &i : _effect)
					i.execute(info, playerId, result, endSeq);

				return true;
			} else {
				_startedShow = true;
				_show = true;
			}
		} else
			_show = false;
	} else
		_show = false;

	return false;
}

void PopUpCollection::internalEvents(pyrodactyl::event::Info &info, const Common::String &playerId,
									 Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &endSeq) {
	if (_cur >= 0 && (unsigned int)_cur < _element.size()) {
		if (_element[_cur].internalEvents(info, playerId, result, endSeq)) {
			if (_element[_cur]._next <= 0 || (unsigned int)_element[_cur]._next >= _element.size()) {
				// This means that this popup is the "end" node, we must loop back to start or end this
				if (_loop) {
					_cur = 0;
					_element[_cur].reset();
				} else
					_cur = -1;
			} else {
				_cur = _element[_cur]._next;
				_element[_cur].reset();
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Draw functions
//------------------------------------------------------------------------
void PopUp::draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera) {
	if (_show) {
		if (x + pop.x < camera.w / 3)
			g_engine->_textManager->draw(x + pop.x, y + pop.y, _text, pop._col, pop._font, ALIGN_LEFT, pop.line.x, pop.line.y, true);
		else if (x + pop.x > (2 * camera.w) / 3)
			g_engine->_textManager->draw(x + pop.x, y + pop.y, _text, pop._col, pop._font, ALIGN_RIGHT, pop.line.x, pop.line.y, true);
		else
			g_engine->_textManager->draw(x + pop.x, y + pop.y, _text, pop._col, pop._font, ALIGN_CENTER, pop.line.x, pop.line.y, true);
	}
}

void PopUpCollection::draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera) {
	if (_cur >= 0 && (unsigned int)_cur < _element.size())
		_element[_cur].draw(x, y, pop, camera);
}

} // End of namespace Crab

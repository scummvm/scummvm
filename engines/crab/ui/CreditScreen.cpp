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

#include "crab/ScreenSettings.h"
#include "crab/XMLDoc.h"
#include "crab/ui/CreditScreen.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

void CreditScreen::reset() {
	_start.x = g_engine->_screenSettings->_cur.w / 2 - 150;
	_start.y = g_engine->_screenSettings->_cur.h + 20;
	_cur.x = _start.x;
	_speed._cur = _speed._slow;
}

void CreditScreen::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("credits");

		if (nodeValid("bg", node))
			_bg.load(node->first_node("bg"));

		if (nodeValid("h", node))
			_heading.load(node->first_node("h"));

		if (nodeValid("p", node))
			_paragraph.load(node->first_node("p"));

		if (nodeValid("logo", node))
			_logo.load(node->first_node("logo"));

		if (nodeValid("website", node))
			_website.load(node->first_node("website"), false);

		if (nodeValid("twitter", node))
			_twitter.load(node->first_node("twitter"), false);

		if (nodeValid("twitter", node))
			_back.load(node->first_node("back"));

		if (nodeValid("fast", node)) {
			rapidxml::xml_node<char> *fnode = node->first_node("fast");
			_fast.load(fnode);
			loadNum(_speed._fast, "val", fnode);
		}

		if (nodeValid("slow", node)) {
			rapidxml::xml_node<char> *snode = node->first_node("slow");
			_slow.load(snode);
			loadNum(_speed._slow, "val", snode);
		}

		if (nodeValid("reverse", node)) {
			rapidxml::xml_node<char> *rnode = node->first_node("reverse");
			_reverse.load(rnode);
			loadNum(_speed._reverse, "val", rnode);
		}

		_speed._cur = _speed._slow;

		if (nodeValid("pause", node))
			_pause.load(node->first_node("pause"));

		if (nodeValid("text", node)) {
			rapidxml::xml_node<char> *tnode = node->first_node("text");
			for (rapidxml::xml_node<char> *n = tnode->first_node(); n != nullptr; n = n->next_sibling()) {
				CreditText t;
				t._text = n->value();
				t._heading = (n->name()[0] == 'h');
				_list.push_back(t);
			}
		}
	}
}

bool CreditScreen::handleEvents(Common::Event &event) {
	if (_slow.handleEvents(event) == BUAC_LCLICK)
		_speed._cur = _speed._slow;
	else if (_fast.handleEvents(event) == BUAC_LCLICK)
		_speed._cur = _speed._fast;
	else if (_pause.handleEvents(event) == BUAC_LCLICK)
		_speed._cur = 0.0f;
	else if (_reverse.handleEvents(event) == BUAC_LCLICK)
		_speed._cur = _speed._reverse;

	return (_back.handleEvents(event) == BUAC_LCLICK);
}

#if 0
bool CreditScreen::handleEvents(SDL_Event &Event) {
	if (slow.handleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.slow;
	else if (fast.handleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.fast;
	else if (pause.handleEvents(Event) == BUAC_LCLICK)
		speed.cur = 0.0f;
	else if (reverse.handleEvents(Event) == BUAC_LCLICK)
		speed.cur = speed.reverse;

	if (website.handleEvents(Event))
		OpenURL("http://pyrodactyl.com");
	else if (twitter.handleEvents(Event))
		OpenURL("https://www.twitter.com/pyrodactylgames");

	return (back.handleEvents(Event) == BUAC_LCLICK);
}
#endif

void CreditScreen::draw() {
	_bg.draw();

	_slow.draw();
	_fast.draw();
	_pause.draw();
	_reverse.draw();

	_logo.draw();
	_twitter.draw();
	_website.draw();

	_back.draw();

	_cur.y = _start.y;

	for (const auto &i : _list) {
		_cur.y += _paragraph._inc;

		if (i._heading) {
			_cur.y += _heading._inc;
			if (_cur.y > -30 && _cur.y < g_engine->_screenSettings->_cur.h + 40) // Only draw text if it is actually visible on screen
				g_engine->_textManager->draw(_cur.x, _cur.y, i._text, _heading._color, _heading._font, _heading._align);
		} else if (_cur.y > -30 && _cur.y < g_engine->_screenSettings->_cur.h + 40)
			g_engine->_textManager->draw(_cur.x, _cur.y, i._text, _paragraph._color, _paragraph._font, _paragraph._align);

		// If our cur value has reached below the screen, simply exit the loop as we won't draw anything else
		if (_cur.y > g_engine->_screenSettings->_cur.h + 40)
			break;
	}

	_start.y -= _speed._cur;

	// Sanity check so that we don't scroll too high or low
	if (_start.y > g_engine->_screenSettings->_cur.h + 40)
		_start.y = g_engine->_screenSettings->_cur.h + 40;
	else if (_start.y < INT_MIN + 10)
		_start.y = INT_MIN + 10;
}

void CreditScreen::setUI() {
	_bg.setUI();
	_back.setUI();

	_slow.setUI();
	_fast.setUI();
	_pause.setUI();
	_reverse.setUI();

	_logo.setUI();
	_twitter.setUI();
	_website.setUI();
}

} // End of namespace Crab

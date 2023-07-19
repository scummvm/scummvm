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

//=============================================================================
// Author:   Arvind
// Purpose:  The tray where you have inventory, map and GameObjectives icons
//=============================================================================
#include "crab/ui/hud.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void HUD::load(const Common::String &filename, pyrodactyl::level::TalkNotify &tn, pyrodactyl::level::PlayerDestMarker &pdm) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("hud");
		if (nodeValid(node)) {
			if (nodeValid("bg", node))
				_bg.load(node->first_node("bg"));

			if (nodeValid("tray", node))
				_menu.load(node->first_node("tray"));

			_pause.load(node->first_node("pause"));
			_gom.load(node->first_node("game_over"));
			_back.load(node->first_node("back"));
			// health.load(node->first_node("health"));

			if (nodeValid("notify", node)) {
				rapidxml::xml_node<char> *notifynode = node->first_node("notify");

				loadImgKey(g_engine->_imageManager->_notify, "img", notifynode);
				tn.load(notifynode);
				pdm.load(notifynode);

				if (nodeValid("anim", notifynode)) {
					rapidxml::xml_node<char> *animnode = notifynode->first_node("anim");
					loadImgKey(_notifyAnim, "img", animnode);
					_clip.load(animnode);

					_timer.load(animnode, "delay");
					_timer.Start();
				}
			}
		}

		// Create a copy of all the tooltips
		for (auto i = _menu._element.begin(); i != _menu._element.end(); ++i)
			_tooltip.push_back(i->_tooltip._text);

		setTooltip();
	}
}

void HUD::draw(pyrodactyl::event::Info &info, const Common::String &id) {
	_bg.draw();
	_menu.draw();

	if (info._unread._journal) {
		g_engine->_imageManager->draw(_menu._element[HS_JOURNAL].x + _menu._element[HS_JOURNAL].w - _clip.w / 2,
									  _menu._element[HS_JOURNAL].y - _clip.h / 2, _notifyAnim, &_clip);
	}

	if (info._unread._inventory) {
		g_engine->_imageManager->draw(_menu._element[HS_INV].x + _menu._element[HS_INV].w - _clip.w / 2,
									  _menu._element[HS_INV].y - _clip.h / 2, _notifyAnim, &_clip);
	}

	if (info._unread._trait) {
		g_engine->_imageManager->draw(_menu._element[HS_CHAR].x + _menu._element[HS_CHAR].w - _clip.w / 2,
									  _menu._element[HS_CHAR].y - _clip.h / 2, _notifyAnim, &_clip);
	}

	if (info._unread._map) {
		g_engine->_imageManager->draw(_menu._element[HS_MAP].x + _menu._element[HS_MAP].w - _clip.w / 2,
									  _menu._element[HS_MAP].y - _clip.h / 2, _notifyAnim, &_clip);
	}
//#endif
}

void HUD::internalEvents(bool showMap) {
	_menu._element[HS_MAP]._visible = showMap;

	if (_timer.TargetReached()) {
		_clip.x += _clip.w;

		if (_clip.x >= g_engine->_imageManager->getTexture(_notifyAnim).w())
			_clip.x = 0;

		_timer.Start();
	}
}

HUDSignal HUD::handleEvents(pyrodactyl::event::Info &info, const Common::Event &event) {
	g_engine->_mouse->_insideHud = _bg.Contains(g_engine->_mouse->_motion.x, g_engine->_mouse->_motion.y);

	int choice = _menu.handleEvents(event);

	if (choice == HS_JOURNAL)
		info._unread._journal = false;
	else if (choice == HS_INV)
		info._unread._inventory = false;
	else if (choice == HS_CHAR)
		info._unread._trait = false;
	else if (choice == HS_MAP)
		info._unread._map = false;

	return static_cast<HUDSignal>(choice);
}

#if 0
HUDSignal HUD::handleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event) {
	g_engine->_mouse->_insideHud = bg.Contains(g_engine->_mouse->motion.x, g_engine->_mouse->motion.y);

	int choice = menu.handleEvents(Event);

	if (choice == HS_JOURNAL)
		info.unread.journal = false;
	else if (choice == HS_INV)
		info.unread.inventory = false;
	else if (choice == HS_CHAR)
		info.unread.trait = false;
	else if (choice == HS_MAP)
		info.unread.map = false;

	return static_cast<HUDSignal>(choice);
}
#endif

void HUD::State(const int &val) {
	int count = 0;
	for (auto i = _menu._element.begin(); i != _menu._element.end(); ++i, ++count)
		i->state(val == count);
}

void HUD::setTooltip() {
	unsigned int count = 0;
	for (auto i = _menu._element.begin(); i != _menu._element.end() && count < _tooltip.size(); ++i, ++count)
		i->_tooltip._text = _tooltip[count] + " (" + i->_hotkey.name() + ")";

	_menu._element[HS_PAUSE]._tooltip._text = _tooltip[HS_PAUSE] + " (" + g_engine->_inputManager->getAssociatedKey(IG_PAUSE) + ")";
}

void HUD::setUI() {
	_bg.setUI();
	_menu.setUI();

	_gom.setUI();
	_pause.setUI();
	_back.setUI();
}

} // End of namespace Crab

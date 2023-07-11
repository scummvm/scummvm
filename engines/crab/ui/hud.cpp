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
				bg.load(node->first_node("bg"));

			if (nodeValid("tray", node))
				menu.load(node->first_node("tray"));

			pause.load(node->first_node("pause"));
			gom.load(node->first_node("game_over"));
			back.load(node->first_node("back"));
			// health.load(node->first_node("health"));

			if (nodeValid("notify", node)) {
				rapidxml::xml_node<char> *notifynode = node->first_node("notify");

				loadImgKey(g_engine->_imageManager->notify, "img", notifynode);
				tn.load(notifynode);
				pdm.load(notifynode);

				if (nodeValid("anim", notifynode)) {
					rapidxml::xml_node<char> *animnode = notifynode->first_node("anim");
					loadImgKey(notify_anim, "img", animnode);
					clip.load(animnode);

					timer.load(animnode, "delay");
					timer.Start();
				}
			}
		}

		// Create a copy of all the tooltips
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i)
			tooltip.push_back(i->tooltip.text);

		SetTooltip();
	}
}

void HUD::draw(pyrodactyl::event::Info &info, const Common::String &id) {
	bg.draw();
	menu.draw();

	if (info.unread.journal) {
		g_engine->_imageManager->draw(menu.element[HS_JOURNAL].x + menu.element[HS_JOURNAL].w - clip.w / 2,
						   menu.element[HS_JOURNAL].y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.inventory) {
		g_engine->_imageManager->draw(menu.element[HS_INV].x + menu.element[HS_INV].w - clip.w / 2,
						   menu.element[HS_INV].y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.trait) {
		g_engine->_imageManager->draw(menu.element[HS_CHAR].x + menu.element[HS_CHAR].w - clip.w / 2,
						   menu.element[HS_CHAR].y - clip.h / 2, notify_anim, &clip);
	}

	if (info.unread.map) {
		g_engine->_imageManager->draw(menu.element[HS_MAP].x + menu.element[HS_MAP].w - clip.w / 2,
						   menu.element[HS_MAP].y - clip.h / 2, notify_anim, &clip);
	}
//#endif
}

void HUD::internalEvents(bool ShowMap) {
	menu.element[HS_MAP].visible = ShowMap;

	if (timer.TargetReached()) {
		clip.x += clip.w;

		if (clip.x >= g_engine->_imageManager->GetTexture(notify_anim).W())
			clip.x = 0;

		timer.Start();
	}
}

HUDSignal HUD::handleEvents(pyrodactyl::event::Info &info, const Common::Event &Event) {
	g_engine->_mouse->inside_hud = bg.Contains(g_engine->_mouse->motion.x, g_engine->_mouse->motion.y);

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

#if 0
HUDSignal HUD::handleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event) {
	g_engine->_mouse->inside_hud = bg.Contains(g_engine->_mouse->motion.x, g_engine->_mouse->motion.y);

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
	for (auto i = menu.element.begin(); i != menu.element.end(); ++i, ++count)
		i->State(val == count);
}

void HUD::SetTooltip() {
	unsigned int count = 0;
	for (auto i = menu.element.begin(); i != menu.element.end() && count < tooltip.size(); ++i, ++count)
		i->tooltip.text = tooltip[count] + " (" + i->hotkey.Name() + ")";

	menu.element[HS_PAUSE].tooltip.text = tooltip[HS_PAUSE] + " (" + g_engine->_inputManager->GetAssociatedKey(IG_PAUSE) + ")";
}

void HUD::setUI() {
	bg.setUI();
	menu.setUI();

	gom.setUI();
	pause.setUI();
	back.setUI();
}

} // End of namespace Crab

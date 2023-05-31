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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/ui/ModMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;

void ModMenu::Load(const Common::String &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("mod_menu");

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (NodeValid("preview", node)) {
			auto prnode = node->first_node("preview");
			img.pos.Load(prnode);
			LoadStr(img.no_preview_path, "path", prnode);
		}

		if (NodeValid("offset", node)) {
			rapidxml::xml_node<char> *offnode = node->first_node("offset");

			// Stuff displayed on the slot button
			td_b[DATA_SAVENAME].Load(offnode->first_node("mod_name"));
			td_b[DATA_LASTMODIFIED].Load(offnode->first_node("last_modified"));

			// Stuff displayed when you hover over a slot button
			td_h[DATA_AUTHOR].Load(offnode->first_node("author"));
			td_h[DATA_VERSION].Load(offnode->first_node("version"));
			td_h[DATA_INFO].Load(offnode->first_node("info"));
			td_h[DATA_WEBSITE].Load(offnode->first_node("website"));

			// Titles for the stuff displayed when you hover over a slot button
			hov[DATA_AUTHOR].Load(offnode->first_node("author_title"));
			hov[DATA_VERSION].Load(offnode->first_node("info_title"));
			hov[DATA_INFO].Load(offnode->first_node("version_title"));
			hov[DATA_WEBSITE].Load(offnode->first_node("website_title"));
		}

		extension = g_engine->_filePath->mod_ext.c_str();
		directory = g_engine->_filePath->mod_path.c_str();
		ScanDir();
	}
}

bool ModMenu::HandleEvents(const Common::Event &Event) {
	int choice = menu.HandleEvents(Event);
	if (choice >= 0) {
		g_engine->_filePath->mod_cur = slot_info[menu.Index() + choice].path.c_str();
		return true;
	}

	return false;
}

#if 0
bool ModMenu::HandleEvents(const SDL_Event &Event) {
	int choice = menu.HandleEvents(Event);
	if (choice >= 0) {
		g_engine->_filePath->mod_cur = slot_info[menu.Index() + choice].path;
		return true;
	}

	return false;
}
#endif

void ModMenu::Draw() {
	bg.Draw();
	menu.Draw();
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < slot_info.size(); i++, count++) {
		auto base_x = menu.BaseX(count), base_y = menu.BaseY(count);
		td_b[DATA_SAVENAME].Draw(slot_info[i].name, base_x, base_y);
		td_b[DATA_LASTMODIFIED].Draw(slot_info[i].last_modified, base_x, base_y);
	}

	if (menu.HoverIndex() >= 0) {
		int i = menu.HoverIndex();

		if (!img.loaded || prev_hover != i) {
			img.loaded = true;
			prev_hover = i;
			if (!img.preview.Load(slot_info[i].preview.c_str()))
				img.preview.Load(img.no_preview_path.c_str());
		}

		hover = true;
		img.preview.Draw(img.pos.x, img.pos.y);

		td_h[DATA_AUTHOR].Draw(slot_info[i].author);
		td_h[DATA_VERSION].Draw(slot_info[i].version);
		td_h[DATA_INFO].Draw(slot_info[i].info);
		td_h[DATA_WEBSITE].Draw(slot_info[i].website);

		for (int num = 0; num < DATA_HOVER_TOTAL; ++num)
			hov[num].Draw();
	} else if (hover)
		Reset();
}

} // End of namespace Crab

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

#include "crab/ui/ModMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;

void ModMenu::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("mod_menu");

		if (nodeValid("bg", node))
			_bg.load(node->first_node("bg"));

		if (nodeValid("menu", node))
			_menu.load(node->first_node("menu"));

		if (nodeValid("preview", node)) {
			auto prnode = node->first_node("preview");
			_img._pos.load(prnode);
			loadStr(_img._noPreviewPath, "path", prnode);
		}

		if (nodeValid("offset", node)) {
			rapidxml::xml_node<char> *offnode = node->first_node("offset");

			// Stuff displayed on the slot button
			tdB[DATA_SAVENAME].load(offnode->first_node("mod_name"));
			tdB[DATA_LASTMODIFIED].load(offnode->first_node("last_modified"));

			// Stuff displayed when you hover over a slot button
			tdH[DATA_AUTHOR].load(offnode->first_node("author"));
			tdH[DATA_VERSION].load(offnode->first_node("version"));
			tdH[DATA_INFO].load(offnode->first_node("info"));
			tdH[DATA_WEBSITE].load(offnode->first_node("website"));

			// Titles for the stuff displayed when you hover over a slot button
			hov[DATA_AUTHOR].load(offnode->first_node("author_title"));
			hov[DATA_VERSION].load(offnode->first_node("info_title"));
			hov[DATA_INFO].load(offnode->first_node("version_title"));
			hov[DATA_WEBSITE].load(offnode->first_node("website_title"));
		}

		_extension = g_engine->_filePath->mod_ext;
		_directory = g_engine->_filePath->mod_path;
		scanDir();
	}
}

bool ModMenu::handleEvents(const Common::Event &event) {
	int choice = _menu.handleEvents(event);
	if (choice >= 0) {
		g_engine->_filePath->mod_cur = _slotInfo[_menu.index() + choice]._path;
		return true;
	}

	return false;
}

#if 0
bool ModMenu::handleEvents(const SDL_Event &Event) {
	int choice = menu.handleEvents(Event);
	if (choice >= 0) {
		g_engine->_filePath->mod_cur = slot_info[menu.Index() + choice].path;
		return true;
	}

	return false;
}
#endif

void ModMenu::draw() {
	_bg.draw();
	_menu.draw();
	for (auto i = _menu.index(), count = 0u; i < _menu.indexPlusOne() && i < _slotInfo.size(); i++, count++) {
		auto base_x = _menu.baseX(count), base_y = _menu.baseY(count);
		tdB[DATA_SAVENAME].draw(_slotInfo[i]._name, base_x, base_y);
		tdB[DATA_LASTMODIFIED].draw(_slotInfo[i]._lastModified, base_x, base_y);
	}

	if (_menu.hoverIndex() >= 0) {
		int i = _menu.hoverIndex();

		if (!_img._loaded || _prevHover != i) {
			_img._loaded = true;
			_prevHover = i;
			if (!_img._preview.load(_slotInfo[i]._preview))
				_img._preview.load(_img._noPreviewPath);
		}

		_hover = true;
		_img._preview.draw(_img._pos.x, _img._pos.y);

		tdH[DATA_AUTHOR].draw(_slotInfo[i]._author);
		tdH[DATA_VERSION].draw(_slotInfo[i]._version);
		tdH[DATA_INFO].draw(_slotInfo[i]._info);
		tdH[DATA_WEBSITE].draw(_slotInfo[i]._website);

		for (int num = 0; num < DATA_HOVER_TOTAL; ++num)
			hov[num].draw();
	} else if (_hover)
		reset();
}

} // End of namespace Crab

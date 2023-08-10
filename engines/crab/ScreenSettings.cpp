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

#include "crab/crab.h"
#include "crab/GameParam.h"
#include "crab/loaders.h"
#include "crab/ScreenSettings.h"

namespace Crab {

void ScreenSettings::load(rapidxml::xml_node<char> *node) {
	loadNum(_cur.w, "w", node);
	loadNum(_cur.h, "h", node);
	loadNum(_fps, "fps", node);
	loadNum(_gamma, "gamma", node);
	loadNum(_textSpeed, "text_speed", node);

	loadBool(_vsync, "vsync", node);
	loadBool(_border, "border", node);
	loadBool(_fullscreen, "fullscreen", node);
	loadBool(_saveOnExit, "save_on_exit", node);
	loadBool(_mouseTrap, "mouse_trap", node);
	loadBool(_quality, "quality", node);
}

void ScreenSettings::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("version", g_engine->_stringPool->get(_version)));

	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "screen");
	child->append_attribute(doc.allocate_attribute("w", g_engine->_stringPool->get(_cur.w)));
	child->append_attribute(doc.allocate_attribute("h", g_engine->_stringPool->get(_cur.h)));
	child->append_attribute(doc.allocate_attribute("fps", g_engine->_stringPool->get(_fps)));

	child->append_attribute(doc.allocate_attribute("gamma", g_engine->_stringPool->fGet(_gamma)));
	child->append_attribute(doc.allocate_attribute("text_speed", g_engine->_stringPool->fGet(_textSpeed)));

	saveBool(_vsync, "vsync", doc, child);
	saveBool(_border, "border", doc, child);
	saveBool(_fullscreen, "fullscreen", doc, child);
	saveBool(_saveOnExit, "save_on_exit", doc, child);
	saveBool(_quality, "quality", doc, child);
	saveBool(_mouseTrap, "mouse_trap", doc, child);

	root->append_node(child);
}

} // End of namespace Crab

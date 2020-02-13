/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "pink/director.h"
#include "pink/pink.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/page.h"

namespace Pink {

Page::~Page() {
	clear();
}

void Page::load(Archive &archive) {
	debugC(6, kPinkDebugLoadingObjects, "Page load");
	archive.mapObject(this);
	NamedObject::deserialize(archive);
	archive.skipString(); //skip directory
	_actors.deserialize(archive);
}

Actor *Page::findActor(const Common::String &name) {
	for (uint i = 0; i < _actors.size(); ++i) {
		if (_actors[i]->getName() == name) {
			return _actors[i];
		}
	}
	return nullptr;
}

void Page::toConsole() const {
	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->toConsole();
	}
}

void Page::init() {
	initPalette();
	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->init(0);
	}
}

void Page::initPalette() {
	for (uint i = 0; i < _actors.size(); ++i) {
		if (_actors[i]->initPalette(getGame()->getDirector()))
			break;
	}
}

void Page::clear() {
	for (uint i = 0; i < _actors.size(); ++i) {
		delete _actors[i];
	}
	_actors.clear();
	_resMgr.clear();
}

void Page::pause(bool paused) {
	for (uint i = 0; i < _actors.size(); ++i) {
		_actors[i]->pause(paused);
	}
}

} // End of namespace Pink

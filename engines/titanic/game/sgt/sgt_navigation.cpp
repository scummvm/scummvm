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

#include "titanic/game/sgt/sgt_navigation.h"

namespace Titanic {

CSGTNavigationStatics *CSGTNavigation::_statics;

void CSGTNavigation::init() {
	_statics = new CSGTNavigationStatics();
}

void CSGTNavigation::deinit() {
	delete _statics;
}

void CSGTNavigation::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_statics->_changeViewFlag, indent);
	file->writeQuotedLine(_statics->_destView, indent);
	file->writeQuotedLine(_statics->_destRoom, indent);

	CGameObject::save(file, indent);
}

void CSGTNavigation::load(SimpleFile *file) {
	file->readNumber();
	_statics->_changeViewFlag = file->readNumber();
	_statics->_destView = file->readString();
	_statics->_destRoom = file->readString();

	CGameObject::load(file);
}

} // End of namespace Titanic

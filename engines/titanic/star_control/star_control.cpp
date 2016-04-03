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

#include "titanic/support/screen_manager.h"
#include "titanic/star_control/star_control.h"

namespace Titanic {

CStarControl::CStarControl() : _fieldBC(0), _field80A0(0),
		_field80A4(0), _field80A8(0), _field80AC(0), _field80B0(0) {
}

void CStarControl::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CStarControl::load(SimpleFile *file) {
	int val = file->readNumber();
	
	if (!val) {
		_sub1.load(file, 0);
		if (!_sub1.initDocument())
			error("Couldn't initialise the StarField document");

		_sub11.load(file, 0);
		CScreenManager *screenManager = CScreenManager::setCurrent();
		if (!screenManager)
			error("There's no screen  manager during loading");

		warning("TODO");
	}
	
	CGameObject::load(file);
}

} // End of namespace Titanic

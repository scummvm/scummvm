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

CStarControl::CStarControl() : _fieldBC(0), _field80B0(0),
		_starRect(20, 10, 620, 350) {
}

void CStarControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	_sub1.save(file, indent);
	_view.save(file, indent);
	CGameObject::save(file, indent);
}

void CStarControl::load(SimpleFile *file) {
	int val = file->readNumber();
	
	if (!val) {
		_sub1.load(file, 0);
		if (!_sub1.initDocument())
			error("Couldn't initialise the StarField document");

		_view.load(file, 0);
		CScreenManager *screenManager = CScreenManager::setCurrent();
		if (!screenManager)
			error("There's no screen  manager during loading");

		_view.setup(screenManager, &_sub1, this);
		_view.reset();

		_fieldBC = 1;
	}
	
	CGameObject::load(file);
}

void CStarControl::draw(CScreenManager *screenManager) {
	if (_visible)
		_view.draw(screenManager);
}

void CStarControl::fn3() {
	warning("CStarControl::fn3");
}

void CStarControl::fn1(int v) {
	warning("CStarControl::fn1");
}

void CStarControl::fn4() {
	warning("CStarControl::fn4");
}

} // End of namespace Titanic

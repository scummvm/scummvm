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

#include "titanic/support/credit_text.h"
#include "titanic/titanic.h"

namespace Titanic {

CCreditText::CCreditText() : _screenManagerP(nullptr), _field14(0),
	_ticks(0), _fontHeight(1), _objectP(nullptr), _field34(0), _field38(0),
	_field3C(0), _field40(0), _field44(0), _field48(0), _field4C(0),
	_field50(0), _field54(0), _field58(0), _field5C(0) {
}

void CCreditText::clear() {
	_list.destroyContents();
	_objectP = nullptr;
}

void CCreditText::load(CGameObject *obj, CScreenManager *screenManager,
		const Rect &rect, int v) {
	_objectP = obj;
	_screenManagerP = screenManager;
	_field14 = v;
	_ticks = g_vm->_events->getTicksCount();
	_field40 = 0;
	_field44 = 0xFF;
	_field48 = 0xFF;
	_field4C = 0xFF;
	_field50 = 0;
	_field54 = 0;
	_field58 = 0;
	_field5C = 0;
}

void CCreditText::setup() {
	// TODO
}

bool CCreditText::draw() {
	return false;
}

} // End of namespace Titanic

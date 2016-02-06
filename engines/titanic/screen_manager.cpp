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

#include "titanic/screen_manager.h"

namespace Titanic {

CScreenManagerRec::CScreenManagerRec() {
	_field0 = 0;
	_field4 = 0;
	_field8 = 0;
	_fieldC = 0;
}

/*------------------------------------------------------------------------*/

CScreenManager::CScreenManager() {
	_screenManagerPtr = nullptr;

	_field4 = 0;
	_fontRenderSurface = nullptr;
	_mouseCursor = nullptr;
	_textCursor = nullptr;
	_fontNumber = 0;
}

CScreenManager::~CScreenManager() {
	_screenManagerPtr = nullptr;
}

void CScreenManager::proc2(int v) {
	if (v)
		_field4 = v;
}

bool CScreenManager::proc3(int v) {
	if (!v || _field4)
		return false;

	_field4 = 0;
	proc27();
	return true;
}

/*------------------------------------------------------------------------*/

OSScreenManager::OSScreenManager(): CScreenManager() {
	_field48 = 0;
	_field4C = 0;
	_field50 = 0;
	_field54 = 0;
	_directDrawManager = nullptr;
}

void OSScreenManager::setMode() {}
void OSScreenManager::proc5() {}
void OSScreenManager::proc6() {}
void OSScreenManager::proc7() {}
void OSScreenManager::proc8() {}
void OSScreenManager::proc9() {}
void OSScreenManager::proc10() {}
void OSScreenManager::proc11() {}
void OSScreenManager::proc12() {}
void OSScreenManager::proc13() {}
void OSScreenManager::proc14() {}
void OSScreenManager::proc15() {}
void OSScreenManager::proc16() {}
void OSScreenManager::getFont() {}
void OSScreenManager::proc18() {}
void OSScreenManager::proc19() {}
void OSScreenManager::proc20() {}
void OSScreenManager::proc21() {}
void OSScreenManager::proc22() {}
void OSScreenManager::proc23() {}
void OSScreenManager::proc24() {}
void OSScreenManager::proc25() {}
void OSScreenManager::showCursor() {}
void OSScreenManager::proc27() {}

} // End of namespace Titanic

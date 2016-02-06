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

#ifndef TITANIC_SCREEN_MANAGER_H
#define TITANIC_SCREEN_MANAGER_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/font.h"

namespace Titanic {

class CSurface {
};

class CScreenManagerRec {
public:
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;
public:
	CScreenManagerRec();
};

class CScreenManager {
public:
	void *_screenManagerPtr;
public:
	int _field4;
	Common::Array<CSurface> _backSurfaces;
	CSurface *_fontRenderSurface;
	CScreenManagerRec _entries[2];
	void *_mouseCursor;
	void *_textCursor;
	int _fontNumber;
public:
	CScreenManager();
	virtual ~CScreenManager();

	void fn1() {}
	void fn2() {}

	virtual void proc2(int v);
	virtual bool proc3(int v);
	virtual void setMode() = 0;
	virtual void proc5() = 0;
	virtual void proc6() = 0;
	virtual void proc7() = 0;
	virtual void proc8() = 0;
	virtual void proc9() = 0;
	virtual void proc10() = 0;
	virtual void proc11() = 0;
	virtual void proc12() = 0;
	virtual void proc13() = 0;
	virtual void proc14() = 0;
	virtual void proc15() = 0;
	virtual void proc16() = 0;
	virtual void getFont() = 0;
	virtual void proc18() = 0;
	virtual void proc19() = 0;
	virtual void proc20() = 0;
	virtual void proc21() = 0;
	virtual void proc22() = 0;
	virtual void proc23() = 0;
	virtual void proc24() = 0;
	virtual void proc25() = 0;
	virtual void showCursor() = 0;
	virtual void proc27() = 0;
};

class OSScreenManager: CScreenManager {
public:
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	void *_directDrawManager;
	STFont _fonts[4];
public:
	OSScreenManager();
};

} // End of namespace Titanic

#endif /* TITANIC_SCREEN_MANAGER_H */

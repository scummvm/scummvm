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
#include "titanic/direct_draw.h"
#include "titanic/font.h"
#include "titanic/input_handler.h"
#include "titanic/video_surface.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class TitanicEngine;

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

struct MouseCursor {
	void show() {}
	void hide() {}
};

class CScreenManager {
protected:
	TitanicEngine *_vm;
public:
	static CScreenManager *_screenManagerPtr;
	static CScreenManager *_currentScreenManagerPtr;

	/**
	 * Set the current screen manager
	 */
	static CScreenManager *setCurrent();
public:
	Common::Array<CVideoSurface *> _backSurfaces;
	CVideoSurface *_frontRenderSurface;
	CScreenManagerRec _entries[2];
	MouseCursor *_mouseCursor;
	void *_textCursor;
	CInputHandler *_inputHandler;
	int _fontNumber;
public:
	CScreenManager(TitanicEngine *vm);
	virtual ~CScreenManager();

	void fn1() {}
	void fn2() {}



	virtual void setWindowHandle(int v);
	virtual bool resetWindowHandle(int v);
	virtual void setMode(int width, int height, int bpp, uint numBackSurfaces, bool flag2) = 0;
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

	/**
	 * Clear a portion of a specified surface
	 */
	virtual void clearSurface(int surfaceNum, Common::Rect *_bounds) = 0;

	/**
	 * Resize the passed surface
	 */
	virtual void resizeSurface(CVideoSurface *surface, int width, int height) = 0;

	/**
	 * Creates a surface of a given size
	 */
	virtual CVideoSurface *createSurface(int w, int h) = 0;
	
	/**
	 * Creates a surface from a specified resource
	 */
	virtual CVideoSurface *createSurface(const CResourceKey &key) = 0;
	
	virtual void proc24() = 0;
	virtual void proc25() = 0;
	virtual void showCursor() = 0;
	virtual void proc27() = 0;
};

class OSScreenManager: CScreenManager {
private:
	DirectDrawManager _directDrawManager;

	/**
	 * Frees any surface buffers
	 */
	void destroyFrontAndBackBuffers();

	/**
	 * Load game cursors
	 */
	void loadCursors();
public:
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	STFont _fonts[4];
public:
	OSScreenManager(TitanicEngine *vm);
	virtual ~OSScreenManager();

	virtual void setMode(int width, int height, int bpp, uint numBackSurfaces, bool flag2);
	virtual void proc5();
	virtual void proc6();
	virtual void proc7();
	virtual void proc8();
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void proc13();
	virtual void proc14();
	virtual void proc15();
	virtual void proc16();
	virtual void getFont();
	virtual void proc18();
	virtual void proc19();

	/**
	 * Clear a portion of the screen surface
	 */
	virtual void clearSurface(int surfaceNum, Common::Rect *bounds);

	/**
	 * Resize the passed surface
	 */
	virtual void resizeSurface(CVideoSurface *surface, int width, int height);

	/**
	 * Creates a surface of a given size
	 */
	virtual CVideoSurface *createSurface(int w, int h);
	
	/**
	 * Creates a surface from a specified resource
	 */
	virtual CVideoSurface *createSurface(const CResourceKey &key);

	virtual void proc23();
	virtual void proc24();
	virtual void proc25();
	virtual void showCursor();
	virtual void proc27();
};

} // End of namespace Titanic

#endif /* TITANIC_SCREEN_MANAGER_H */

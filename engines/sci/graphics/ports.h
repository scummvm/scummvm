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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GRAPHICS_PORTS_H
#define SCI_GRAPHICS_PORTS_H

#include "common/list.h"
#include "common/array.h"

namespace Sci {

class SciGui;
class GfxPaint16;
class GfxScreen;
class GfxText16;

class GfxPorts {
public:
	GfxPorts(SegManager *segMan, GfxScreen *screen);
	~GfxPorts();

	void init(SciGui *gui, GfxPaint16 *paint16, GfxText16 *text16, Common::String gameId);

	void kernelSetActive(uint16 portId);
	Common::Rect kernelGetPicWindow(int16 &picTop, int16 &picLeft);
	void kernelSetPicWindow(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag);
	reg_t kernelGetActive();
	reg_t kernelNewWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	void kernelDisposeWindow(uint16 windowId, bool reanimate);

	int16 isFrontWindow(Window *wnd);
	void beginUpdate(Window *wnd);
	void endUpdate(Window *wnd);
	Window *newWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw);
	void drawWindow(Window *wnd);
	void disposeWindow(Window *pWnd, bool reanimate);
	void updateWindow(Window *wnd);

	Port *getPortById(uint16 id);

	Port *setPort(Port *newPort);
	Port *getPort();
	void setOrigin(int16 left, int16 top);
	void moveTo(int16 left, int16 top);
	void move(int16 left, int16 top);
	void openPort(Port *port);
	void penColor(int16 color);
	void backColor(int16 color);
	void penMode(int16 mode);
	void textGreyedOutput(bool state);
	int16 getPointSize();

	void offsetRect(Common::Rect &r);
	void offsetLine(Common::Point &start, Common::Point &end);

	void priorityBandsInit(int16 bandCount, int16 top, int16 bottom);
	void priorityBandsInit(byte *data);
	void priorityBandsRemember(byte *data);
	void priorityBandsRecall();
	byte coordinateToPriority(int16 y);
	int16 priorityToCoordinate(byte priority);

	Port *_wmgrPort;
	Window *_picWind;

	Port *_menuPort;
	Common::Rect _menuBarRect;
	Common::Rect _menuRect;
	Common::Rect _menuLine;
	Port *_curPort;

private:
	typedef Common::List<Port *> PortList;

	SegManager *_segMan;
	SciGui *_gui;
	GfxPaint16 *_paint16;
	GfxScreen *_screen;
	GfxText16 *_text16;

	/** The list of open 'windows' (and ports), in visual order. */
	PortList _windowList;

	/** The list of all open 'windows' (and ports), ordered by their id. */
	Common::Array<Port *> _windowsById;

	Common::Rect _bounds;
	Port *_mainPort;

	// Priority Bands related variables
	int16 _priorityTop, _priorityBottom, _priorityBandCount;
	byte _priorityBands[200];

	byte priorityBandsMemory[14];
	bool priorityBandsMemoryActive;
};

} // End of namespace Sci

#endif

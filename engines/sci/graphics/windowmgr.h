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

#ifndef SCI_GRAPHICS_WINDOWMGR_H
#define SCI_GRAPHICS_WINDOWMGR_H

#include "common/list.h"
#include "common/array.h"

namespace Sci {

class WindowMgr {
public:
	WindowMgr(SciGui *gui, Screen *screen, Gfx *gfx, Text *text);
	~WindowMgr();

	void init(Common::String gameId);

	int16 isFrontWindow(Window *wnd);
	void BeginUpdate(Window *wnd);
	void EndUpdate(Window *wnd);
	Window *NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw);
	void DrawWindow(Window *wnd);
	void DisposeWindow(Window *pWnd, bool reanimate);
	void UpdateWindow(Window *wnd);

	Port *getPortById(uint16 id) const { return _windowsById[id]; }

	Port *_wmgrPort;
	Window *_picWind;

private:
	typedef Common::List<Port *> PortList;

	SciGui *_gui;
	Screen *_screen;
	Gfx *_gfx;
	Text *_text;

	/** The list of open 'windows' (and ports), in visual order. */
	PortList _windowList;

	/** The list of all open 'windows' (and ports), ordered by their id. */
	Common::Array<Port *> _windowsById;
};

} // End of namespace Sci

#endif

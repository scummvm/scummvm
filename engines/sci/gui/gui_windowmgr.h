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

#ifndef SCI_GUI_WINDOWMGR_H
#define SCI_GUI_WINDOWMGR_H

#include "common/list.h"
#include "common/array.h"

namespace Sci {

class SciGuiWindowMgr {
public:
	SciGuiWindowMgr(SciGui *gui, SciGuiScreen *screen, SciGuiGfx *gfx, SciGuiText *text);
	~SciGuiWindowMgr();

	void init(Common::String gameName);

	int16 isFrontWindow(GuiWindow *wnd);
	void BeginUpdate(GuiWindow *wnd);
	void EndUpdate(GuiWindow *wnd);
	GuiWindow *NewWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw);
	void DrawWindow(GuiWindow *wnd);
	void DisposeWindow(GuiWindow *pWnd, bool reanimate);
	void UpdateWindow(GuiWindow *wnd);

	GuiPort *getPortById(uint16 id) const { return _windowsById[id]; }

	GuiPort *_wmgrPort;
	GuiWindow *_picWind;

private:
	typedef Common::List<GuiPort *> PortList;

	SciGui *_gui;
	SciGuiScreen *_screen;
	SciGuiGfx *_gfx;
	SciGuiText *_text;

	/** The list of open 'windows' (and ports), in visual order. */
	PortList _windowList;

	/** The list of all open 'windows' (and ports), ordered by their id. */
	Common::Array<GuiPort *> _windowsById;
};

} // End of namespace Sci

#endif

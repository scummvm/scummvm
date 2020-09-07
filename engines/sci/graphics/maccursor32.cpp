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

#include "common/system.h"          // for OSystem, g_system
#include "graphics/cursorman.h"     // for CursorMan
#include "graphics/maccursor.h"     // for MacCursor
#include "sci/graphics/maccursor32.h"

namespace Sci {

void GfxMacCursor32::setMacCursorRemapList(int cursorCount, reg_t *cursors) {
	for (int i = 0; i < cursorCount; i++) {
		_macCursorRemap.push_back(cursors[i].toUint16());
	}
}

void GfxMacCursor32::setView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) {
	_cursorInfo.resourceId = viewId;
	_cursorInfo.loopNo = loopNo;
	_cursorInfo.celNo = celNo;

	GuiResourceId viewNum = viewId;

	// Remap cursor view based on what the scripts have given us.
	for (uint32 i = 0; i < _macCursorRemap.size(); i++) {
		if (viewNum == _macCursorRemap[i]) {
			viewNum = (i + 1) * 0x100 + loopNo * 0x10 + celNo;
			break;
		}
	}

	_cursorInfo.resourceId = viewNum;

	Resource *resource = g_sci->getResMan()->findResource(ResourceId(kResourceTypeCursor, viewNum), false);

	if (!resource) {
		// The cursor resources often don't exist, this is normal behavior
		debug(0, "Mac cursor %d not found", viewNum);
		return;
	}
	Common::MemoryReadStream resStream(resource->toStream());
	Graphics::MacCursor *macCursor = new Graphics::MacCursor();

	if (!macCursor->readFromStream(resStream)) {
		warning("Failed to load Mac cursor %d", viewNum);
		delete macCursor;
		return;
	}

	hide();

	CursorMan.disableCursorPalette(false);
	CursorMan.replaceCursor(macCursor);

	delete macCursor;

	unhide();
}

void GfxMacCursor32::hide() {
	if (_hideCount++) {
		return;
	}

	CursorMan.showMouse(false);
}

void GfxMacCursor32::unhide() {
	if (_hideCount == 0 || --_hideCount) {
		return;
	}

	CursorMan.showMouse(true);
}

void GfxMacCursor32::show() {
	if (_hideCount) {
		CursorMan.showMouse(true);
		_hideCount = 0;
	}
}

} // End of namespace Sci

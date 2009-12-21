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

#ifndef SCI_GUI_DUMMY_H
#define SCI_GUI_DUMMY_H

#include "sci/gui/gui.h"

namespace Sci {

class SciGuiDummy : public SciGui {
public:
	SciGuiDummy(EngineState *s, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor) : SciGui(s, screen, palette, cursor) {}
	virtual ~SciGuiDummy() {}

	virtual uint16 onControl(byte screenMask, Common::Rect rect) { return 0; }
	virtual void animate(reg_t listReference, bool cycle, int argc, reg_t *argv) {}
	virtual void addToPicList(reg_t listReference, int argc, reg_t *argv) {}
	virtual void addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {}
	virtual void setNowSeen(reg_t objectReference) {}
	virtual bool canBeHere(reg_t curObject, reg_t listReference) { return false; }
	virtual bool isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) { return false; }
	virtual void baseSetter(reg_t object) {}

	virtual int16 getCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo) { return 0; }
	virtual int16 getCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo) { return 0; }
	virtual int16 getLoopCount(GuiResourceId viewId) { return 0; }
	virtual int16 getCelCount(GuiResourceId viewId, int16 loopNo) { return 0; }
};

} // End of namespace Sci

#endif

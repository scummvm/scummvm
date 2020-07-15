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

#ifndef GRAPHICS_MACGUI_MACBUTTON_H
#define GRAPHICS_MACGUI_MACBUTTON_H

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwidget.h"

namespace Graphics {

class MacWidget;
class MacText;
struct MacPlotData;

enum MacButtonType {
	kRound,
	kCheckBox,
	kRadio
};

class MacButton : public MacText {
public:
	MacButton(MacButtonType buttonType, TextAlign textAlignment, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor);

	virtual void setActive(bool active) override;

	void invertOuter();
	void invertInner();
	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	virtual bool draw(bool forceRedraw = false) override;
	virtual bool processEvent(Common::Event &event) override;
	virtual Common::Point calculateOffset() override;

private:
	MacButtonType _buttonType;
	MacPlotData _pd;
};

} // End of namespace Graphics

#endif

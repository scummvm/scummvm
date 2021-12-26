/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

// those name comes from Apartment
enum CheckBoxType {
	kCBNormal,
	kCBInsetBlack,
	kCBFilledBlack
};

class MacButton : public MacText {
public:
	MacButton(MacButtonType buttonType, TextAlign textAlignment, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor);

	void setActive(bool active) override;

	void invertOuter();
	void invertInner();

	void setHilite(bool hilite);
	void setCheckBoxType(int type);
	void setCheckBoxAccess(int type) { _checkBoxAccess = type; }

	bool draw(ManagedSurface *g, bool forceRedraw = false) override;
	bool draw(bool forceRedraw = false) override;
	bool processEvent(Common::Event &event) override;
	Common::Point calculateOffset() override;

private:
	void init();

private:
	MacButtonType _buttonType;
	MacPlotData _pd;
	bool _invertInner;
	int _checkBoxType;
	int _checkBoxAccess;
};

} // End of namespace Graphics

#endif

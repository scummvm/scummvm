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

#ifndef PINK_ACTION_TEXT_H
#define PINK_ACTION_TEXT_H

#include "common/events.h"

#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/mactextwindow.h"

#include "pink/objects/actions/action.h"

namespace Pink {

class ActionText : public Action {
public:
	ActionText();
	~ActionText() override;
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void start() override;
	void end() override;

	void draw(Graphics::ManagedSurface *surface); // only for non-scrollable text

	Common::Rect getBound();

private:

	void findColorsInPalette();
	void loadBorder(Graphics::MacWindow *target, Common::String filename, uint32 flags);

private:
	Common::String _fileName;
	Common::U32String _text;
	Graphics::MacTextWindow *_txtWnd;
	Graphics::MacText *_macText;

	uint32 _xLeft;
	uint32 _yTop;
	uint32 _xRight;
	uint32 _yBottom;

	uint32 _centered;
	uint32 _scrollBar;
	uint32 _textRGB;
	uint32 _backgroundRGB;

	byte _textColorIndex;
	byte _backgroundColorIndex;
};

} // End of namespace Pink

#endif

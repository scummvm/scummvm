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

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/ustr.h"
#include "common/array.h"
#include "common/keyboard.h"

namespace GUI {

class EEHandler;

class AboutDialog : public Dialog {
	typedef Common::String String;
	typedef Common::U32String U32String;
	typedef Common::Array<Common::U32String> U32StringArray;
protected:
	int	       _scrollPos;
	uint32         _scrollTime;
	U32StringArray _lines;
	uint32         _lineHeight;
	bool           _willClose;

	int _xOff, _yOff;

	void addLine(const U32String &str);

	EEHandler	*_eeHandler;

public:
	AboutDialog();

	void open() override;
	void close() override;
	void drawDialog(DrawLayer layerToDraw) override;
	void handleTickle() override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleKeyDown(Common::KeyState state) override;
	void handleKeyUp(Common::KeyState state) override;

	void reflowLayout() override;
};

} // End of namespace GUI

#endif

/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "base/engine.h"
#include "base/version.h"
#include "gui/about.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kScrollStartDelay = 2500,
	kScrollMillisPerPixel = 80,
	
	kXOff = 3,
	kYOff = 2
};

// TODO: Probably should display something regarding GPL
// We could just list our full credits section here now...
//
// The following commands can be put at the start of a line (all subject to change):
//   \C, \L, \R  -- set center/left/right alignment
//   \c0 - \c4   -- set a custom color:
//                  0 normal text (green)
//                  1 highlighted text (light green)
//                  2 light border (light gray)
//                  3 dark border (dark gray)
//                  4 background (black)
static const char *credits_intro[] = {
"\\C""Copyright (C) 2002-2004 The ScummVM project",
"\\C""http://www.scummvm.org",
"\\C""",
"\\C""LucasArts SCUMM Games (C) LucasArts",
"\\C""Humongous SCUMM Games (C) Humongous",
"\\C""Simon the Sorcerer (C) Adventure Soft",
"\\C""Beneath a Steel Sky (C) Revolution",
"\\C""Broken Sword Games (C) Revolution",
"\\C""Flight of the Amazon Queen (C) John Passfield",
"\\C""and Steve Stamatiadis",
"\\C"""
};

#include "gui/credits.h"


AboutDialog::AboutDialog()
	: Dialog(10, 20, 300, 174),
	_scrollPos(0), _scrollTime(0) {
	
	int i;
	
	_lineHeight = g_gui.getFont().getFontHeight() + 3;

	for (i = 0; i < 1; i++)
		_lines.push_back("");

	Common::String version("\\C\\c0""ScummVM ");
	version += gScummVMVersion;
	_lines.push_back(version);

	Common::String date("\\C\\c2""(built on ");
	date += gScummVMBuildDate;
	date += ')';
	_lines.push_back(date);

	Common::String features("\\C\\c2""Supports: ");
	features += gScummVMFeatures;
	_lines.push_back(features);

	_lines.push_back("");
	
	for (i = 0; i < ARRAYSIZE(credits_intro); i++)
		_lines.push_back(credits_intro[i]);
	
	for (i = 0; i < ARRAYSIZE(credits); i++)
		_lines.push_back(credits[i]);
}

void AboutDialog::open() {
	_scrollTime = g_system->getMillis() + kScrollStartDelay;
	_scrollPos = 0;

	Dialog::open();
}

void AboutDialog::drawDialog() {
	// Blend over the background
	g_gui.blendRect(_x, _y, _w, _h, g_gui._bgcolor);

	// Draw text
	// TODO: Add a "fade" effect for the top/bottom text lines
	const int firstLine = _scrollPos / _lineHeight;
	const int lastLine = MIN((_scrollPos + _h) / _lineHeight + 1, (uint32)_lines.size());
	int y = _y + kYOff - (_scrollPos % _lineHeight);

	for (int line = firstLine; line < lastLine; line++) {
		const char *str = _lines[line].c_str();
		Graphics::TextAlignment align = Graphics::kTextAlignCenter;
		OverlayColor color = g_gui._textcolor;
		while (str[0] == '\\') {
			switch (str[1]) {
			case 'C':
				align = Graphics::kTextAlignCenter;
				break;
			case 'L':
				align = Graphics::kTextAlignLeft;
				break;
			case 'R':
				align = Graphics::kTextAlignRight;
				break;
			case 'c':
				switch (str[2]) {
				case '0':
					color = g_gui._textcolor;
					break;
				case '1':
					color = g_gui._textcolorhi;
					break;
				case '2':
					color = g_gui._color;
					break;
				case '3':
					color = g_gui._shadowcolor;
					break;
				case '4':
					color = g_gui._bgcolor;
					break;
				default:
					warning("Unknown color type '%c'", str[2]);
				}
				str++;
				break;
			default:
				warning("Unknown scroller opcode '%c'\n", str[1]);
				break;
			}
			str += 2;
		}
		// Trim leading whitespaces if center mode is on
		if (align == Graphics::kTextAlignCenter)
			while (*str && *str == ' ')
				str++;
	
		g_gui.drawString(str, _x + kXOff, y, _w - 2 * kXOff, color, align);
		y += _lineHeight;
	}

	// Draw a border
	g_gui.box(_x, _y, _w, _h, g_gui._color, g_gui._shadowcolor);

	// Finally blit it all to the screen
	g_gui.addDirtyRect(_x, _y, _w, _h);
}


void AboutDialog::handleTickle() {
	const uint32 t = g_system->getMillis();
	int scrollOffset = ((int)t - (int)_scrollTime) / kScrollMillisPerPixel;
	if (scrollOffset > 0) {
		_scrollPos += scrollOffset;
		_scrollTime = t;

		if (_scrollPos < 0) {
			_scrollPos = 0;
		} else if ((uint32)_scrollPos > _lines.size() * _lineHeight) {
			_scrollPos = 0;
			_scrollTime += kScrollStartDelay;
		} else {
			g_gui.addDirtyRect(_x, _y, _w, _h);
		}
		draw();	// Issue a full redraw
	}
}

void AboutDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Close upon any mouse click
	close();
}

void AboutDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
}

void AboutDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	close();
}


} // End of namespace GUI

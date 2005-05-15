/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
#include "common/system.h"
#include "common/util.h"
#include "gui/about.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kScrollStartDelay = 1500,
	kScrollMillisPerPixel = 80,
	
	kXOff = 3,
	kYOff = 2
};

// The following commands can be put at the start of a line (all subject to change):
//   \C, \L, \R  -- set center/left/right alignment
//   \c0 - \c4   -- set a custom color:
//                  0 normal text (green)
//                  1 highlighted text (light green)
//                  2 light border (light gray)
//                  3 dark border (dark gray)
//                  4 background (black)
static const char *credits_intro[] = {
"\\C""Copyright (C) 2002-2005 The ScummVM project",
"\\C""http://www.scummvm.org",
"\\C""",
"\\C""LucasArts SCUMM Games (C) LucasArts",
"\\C""Humongous SCUMM Games (C) Humongous",
"\\C""Simon the Sorcerer (C) Adventure Soft",
"\\C""Beneath a Steel Sky (C) Revolution",
"\\C""Broken Sword Games (C) Revolution",
"\\C""Flight of the Amazon Queen (C) John Passfield",
"\\C""and Steve Stamatiadis",
"\\C""",
"\\C""This program is free software; you can",
"\\C""redistribute it and/or modify it under the",
"\\C""terms of the GNU General Public License as",
"\\C""published by the Free Software Foundation;",
"\\C""either version 2 of the License, or (at your",
"\\C""option) any later version.",
"\\C""",
"\\C""This program is distributed in the hope that",
"\\C""it will be useful, but WITHOUT ANY WARRANTY;",
"\\C""without even the implied warranty of",
"\\C""MERCHANTABILITY or FITNESS FOR A PARTICULAR",
"\\C""PURPOSE.  See the GNU General Public License",
"\\C""for more details.",
"\\C""",
"\\C""You should have received a copy of the GNU",
"\\C""General Public License along with this",
"\\C""program; if not, write to the Free Software",
"\\C""Foundation, Inc., 59 Temple Place - Suite 330,",
"\\C""Boston, MA  02111-1307, USA.",
"\\C"""
};

#include "gui/credits.h"


AboutDialog::AboutDialog()
	: Dialog(10, 20, 300, 174),
	_scrollPos(0), _scrollTime(0), _modifiers(0), _willClose(false) {
	
	int i;
	
	_w = g_system->getOverlayWidth() - 2 * 10;
	_h = g_system->getOverlayHeight() - 20 - 16;
	
	if (_w < 450)
		_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	else
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	
	_lineHeight = _font->getFontHeight() + 3;

	for (i = 0; i < 1; i++)
		_lines.push_back("");

	Common::String version("\\C\\c0""ScummVM ");
	version += gScummVMVersion;
	_lines.push_back(version);

	Common::String date("\\C\\c2""(built on ");
	date += gScummVMBuildDate;
	date += ')';
	_lines.push_back(date);

	Common::String features("Supports: ");
	features += gScummVMFeatures;

	// If the features string is too wide, split it up
	const int maxWidth = _w - 2*kXOff;
	if (_font->getStringWidth(features) > maxWidth) {
		Common::StringList wrappedLines;
		_font->wordWrapText(features, maxWidth, wrappedLines);
		
		for (i = 0; i < (int)wrappedLines.size(); ++i)
			_lines.push_back("\\C\\c2" + wrappedLines[i]);
	} else
		_lines.push_back("\\C\\c2" + features);

	_lines.push_back("");
	
	for (i = 0; i < ARRAYSIZE(credits_intro); i++)
		_lines.push_back(credits_intro[i]);
	
	for (i = 0; i < ARRAYSIZE(credits); i++)
		_lines.push_back(credits[i]);
}

void AboutDialog::open() {
	_scrollTime = getMillis() + kScrollStartDelay;
	_scrollPos = 0;
	_modifiers = 0;
	_willClose = false;
	_canvas.pixels = NULL;

	Dialog::open();
}

void AboutDialog::close() {
	free(_canvas.pixels);
	Dialog::close();
}

void AboutDialog::drawDialog() {
	if (!_canvas.pixels) {
		// Blend over the background. Since we can't afford to do that
		// every time the text is updated (it's horribly CPU intensive)
		// we do it just once and then use a copy of the result as our
		// static background for the remainder of the credits.
		g_gui.blendRect(_x, _y, _w, _h, g_gui._bgcolor);
		g_gui.copyToSurface(&_canvas, _x, _y, _w, _h);
	}

	g_gui.drawSurface(_canvas, _x, _y);

	// Draw text
	// TODO: Add a "fade" effect for the top/bottom text lines
	// TODO: Maybe prerender all of the text into another surface,
	//       and then simply compose that over the screen surface
	//       in the right way. Should be even faster...
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
	
		_font->drawString(&g_gui.getScreen(), str, _x + kXOff, y, _w - 2 * kXOff, color, align, 0, false);
		y += _lineHeight;
	}

	// Draw a border
	g_gui.box(_x, _y, _w, _h, g_gui._color, g_gui._shadowcolor);

	// Finally blit it all to the screen
	g_gui.addDirtyRect(_x, _y, _w, _h);
}


void AboutDialog::handleTickle() {
	// We're in the process of doing a full redraw to re-create the
	// background image for the text. That means we need to wait for the
	// GUI itself to clear the overlay and call drawDialog() in all of the
	// dialogs, otherwise we'll only redraw this one and it'll still have
	// the remains of the old image, including the text that was on it.
	if (!_canvas.pixels)
		return;

	const uint32 t = getMillis();
	int scrollOffset = ((int)t - (int)_scrollTime) / kScrollMillisPerPixel;
	if (scrollOffset > 0) {
		// Scroll faster when shift is pressed
		if (_modifiers & OSystem::KBD_SHIFT)
			scrollOffset *= 4;
		// Reverse scrolling when alt is pressed
		if (_modifiers & OSystem::KBD_ALT)
			scrollOffset *= -1;
		_scrollPos += scrollOffset;
		_scrollTime = t;

		if (_scrollPos < 0) {
			_scrollPos = 0;
		} else if ((uint32)_scrollPos > _lines.size() * _lineHeight) {
			_scrollPos = 0;
			_scrollTime += kScrollStartDelay;
		}
		drawDialog();
	}
}

void AboutDialog::handleScreenChanged() {
	// The screen has changed. That means the overlay colors in the canvas
	// may no longer be correct. Reset it, and issue a full redraw.
	// TODO: We could check if the bit format has changed, like we do in
	// the MPEG player.
	free(_canvas.pixels);
	_canvas.pixels = NULL;
	draw();
}

void AboutDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Close upon any mouse click
	close();
}

void AboutDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	_modifiers = modifiers;
	if (ascii)
		_willClose = true;
}

void AboutDialog::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	_modifiers = modifiers;
	if (ascii && _willClose)
		close();
}


} // End of namespace GUI

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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_DIALOG_H
#define WAGE_DIALOG_H

namespace Wage {

struct DialogButton {
	Common::String text;
	Common::Rect bounds;

	DialogButton(const char *t, int x1, int y1, int w, int h) {
		text = t;
		bounds.left = x1;
		bounds.top = y1;
		bounds.right = x1 + w - 1;
		bounds.bottom = y1 + h - 1;
	}
};

typedef Common::Array<DialogButton *> DialogButtonArray;

class Dialog {
public:
	Dialog(Gui *gui, int width, const char *text, DialogButtonArray *buttons, uint defaultButton);
	~Dialog();

	int run();

private:
	Gui *_gui;
	Graphics::ManagedSurface _tempSurface;
	Common::Rect _bbox;
	Common::String _text;

	const Graphics::Font *_font;
	DialogButtonArray *_buttons;
	int _pressedButton;
	uint _defaultButton;
	bool _mouseOverPressedButton;

	bool _needsRedraw;

private:
	const Graphics::Font *getDialogFont();
	void drawOutline(Common::Rect &bounds, int *spec, int speclen);
	void paint();
	void mouseMove(int x, int y);
	void mouseClick(int x, int y);
	int mouseRaise(int x, int y);
	int matchButton(int x, int y);
};

} // End of namespace Wage

#endif

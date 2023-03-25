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

#ifndef TEXTVIEWER_DIALOG_H
#define TEXTVIEWER_DIALOG_H

#include "common/array.h"
#include "common/str.h"

#include "gui/dialog.h"

namespace Graphics {
class Font;
}

namespace GUI {

class ScrollBarWidget;

class TextViewerDialog : public Dialog {
private:
	int	_lineWidth;
	int	_linesPerPage;
	int	_currentPos;
	int	_scrollLine;

	int _scrollbarWidth;
	int _charWidth;
	int _lineHeight;
	int _padX, _padY;

	Common::StringArray _linesArray;

	ScrollBarWidget *_scrollBar;

	Common::String _fname;
	const Graphics::Font *_font = nullptr;

	bool loadFile(Common::String &fname);
	void reflowLayout();

public:
	TextViewerDialog(Common::String fname);
	~TextViewerDialog();

	void destroy();
	void open();
	void drawDialog(DrawLayer layerToDraw);

	void handleMouseWheel(int x, int y, int direction);
	void handleKeyDown(Common::KeyState state);
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

} // End of namespace GUI

#endif

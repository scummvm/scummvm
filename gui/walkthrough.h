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

#ifndef WALKTHROUGH_DIALOG_H
#define WALKTHROUGH_DIALOG_H

#include "common/array.h"
#include "common/str.h"

#include "gui/dialog.h"
#include "gui/newgui.h"

#include <stdarg.h>

namespace GUI {

class ScrollBarWidget;

class WalkthroughDialog : public Dialog {
private:
	bool _initialized;
	int	_lineWidth;
	int	_linesPerPage;
	int	_currentPos;
	int	_scrollLine;
	int	_firstLineInBuffer;

	typedef Common::String String;

	struct Entry {
		String text;
	};
	typedef Common::Array<Entry> EntryList;
	EntryList _linesArray;

	ScrollBarWidget *_scrollBar;

	float _widthPercent, _heightPercent;

	void reflowLayout();
	bool loadWalkthroughText(const char *gameName);

public:
	WalkthroughDialog(float widthPercent, float heightPercent);
	~WalkthroughDialog();

	void create(const char *gameName);
	void destroy();
	void open();
	void drawDialog();

	void handleMouseWheel(int x, int y, int direction);
	void handleKeyDown(uint16 ascii, int keycode, int modifiers);
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

} // End of namespace GUI

#endif

/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#include "browser.h"
#include "newgui.h"
#include "ListWidget.h"

#include "backends/fs/fs.h"

/* We want to use this as a general directory selector at some point... possible uses
 * - to select the data dir for a game
 * - to select the place where save games are stored
 * - others???
 */

BrowserDialog::BrowserDialog(NewGui *gui)
	: Dialog(gui, 50, 20, 320-2*50, 200-2*20)
{
	// Headline - TODO: should be customizable during creation time
	new StaticTextWidget(this, 10, 10, _w-2*10, kLineHeight,
		"Select directory with game data", kTextAlignCenter);

	// Add file list
	_fileList = new ListWidget(this, 10, 20, _w-2*10, _h-20-24-10);
	
	// Buttons
	addButton(10, _h-24, "Go up", kCloseCmd, 0);
	addButton(_w-2*(kButtonWidth+10), _h-24, "Cancel", kCloseCmd, 0);
	addButton(_w-(kButtonWidth+10), _h-24, "Choose", kCloseCmd, 0);

	// TODO - populate list item, implement buttons, etc. etc.
	// TODO - will the choose button select the directory we are currrently in?!?
	// TODO - double clicking an item should traverse into that directory
}

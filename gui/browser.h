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

#ifndef BROWSER_DIALOG_H
#define BROWSER_DIALOG_H

#include "dialog.h"
#include "common/str.h"
#include "common/list.h"

#ifdef MACOSX
#include <Carbon/Carbon.h>
#endif

class FilesystemNode;
class FSList;

namespace GUI {

class ListWidget;
class StaticTextWidget;

class BrowserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	BrowserDialog(const char *title);
	virtual ~BrowserDialog();

#ifdef MACOSX
	virtual int runModal();
#else
	virtual void open();
	virtual void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
#endif

	FilesystemNode	*getResult() { return _choice; };


protected:
#ifdef MACOSX
	CFStringRef		_titleRef;
#else
	ListWidget		*_fileList;
	StaticTextWidget*_currentPath;
	FilesystemNode	*_node;
	FSList			*_nodeContent;
#endif
	FilesystemNode	*_choice;

#ifndef MACOSX
	void updateListing();
#endif
};

} // End of namespace GUI

#endif

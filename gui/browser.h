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

#ifndef BROWSER_DIALOG_H
#define BROWSER_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "backends/fs/fs.h"

#ifdef MACOSX
#include <Carbon/Carbon.h>
#endif

namespace GUI {

class ListWidget;
class StaticTextWidget;

// TODO: Common parent class for DirBrowserDialog and FileBrowserDialog

class DirBrowserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	DirBrowserDialog(const char *title);

#ifdef MACOSX
	~DirBrowserDialog();
	virtual int runModal();
#else
	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
#endif

	const FilesystemNode	&getResult() { return _choice; }

protected:
#ifdef MACOSX
	CFStringRef		_titleRef;
#else
	ListWidget		*_fileList;
	StaticTextWidget	*_currentPath;
	FilesystemNode	_node;
	FSList			_nodeContent;
#endif
	FilesystemNode	_choice;

#ifndef MACOSX
	void updateListing();
#endif
};

// TODO: MACOSX version

class FileBrowserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	FileBrowserDialog(const char *title);

	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	const FilesystemNode &getResult() { return _choice; }

protected:
	ListWidget *_fileList;
	StaticTextWidget *_currentPath;
	FilesystemNode _node;
	FSList _nodeContent;
	FilesystemNode _choice;

	void updateListing();
};

} // End of namespace GUI

#endif

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
 * $URL$
 * $Id$
 */

#ifndef GUI_THEMEBROWSER_H
#define GUI_THEMEBROWSER_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/fs.h"
#include "common/array.h"

namespace GUI {

class ListWidget;
class StaticTextWidget;

class ThemeBrowser : public Dialog {
public:
	ThemeBrowser();

	void open();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	const Common::String &selected() const { return _select; }
private:
	struct Entry {
		Common::String name;
		Common::String file;
	};

	ListWidget *_fileList;
	Common::String _select;
	typedef Common::Array<Entry> ThList;
	ThList _themes;

	void updateListing();

	void addDir(ThList &list, const Common::FSNode &node);
	bool isTheme(const Common::FSNode &node, Entry &out);
};

} // end of namespace GUI

#endif


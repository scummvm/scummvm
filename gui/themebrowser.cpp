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

#include "gui/themebrowser.h"
#include "gui/ListWidget.h"
#include "gui/widget.h"
#include "gui/theme.h"

#ifdef MACOSX
#include "CoreFoundation/CoreFoundation.h"
#endif

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

// TODO: this is a rip off of GUI::Browser right now
// it will get some more output like theme name,
// theme style, theme preview(?) in the future
// but for now this simple browser works,
// also it will get its own theme config values
// and not use 'browser_' anymore
ThemeBrowser::ThemeBrowser() : Dialog("browser") {
	_fileList = 0;

	new StaticTextWidget(this, "browser_headline", "Select a Theme");

	// Add file list
	_fileList = new ListWidget(this, "browser_list");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_fileList->setHints(THEME_HINT_PLAIN_COLOR);

	// Buttons
	new ButtonWidget(this, "browser_cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "browser_choose", "Choose", kChooseCmd, 0);
}

void ThemeBrowser::open() {
	// Alway refresh file list
	updateListing();

	// Call super implementation
	Dialog::open();
}

void ThemeBrowser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd: {
		int selection = _fileList->getSelected();
		if (selection < 0)
			break;
		_select = _themes[selection].file;
		setResult(1);
		close();
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void ThemeBrowser::updateListing() {
	_themes.clear();

	// classic is always build in
	Entry th;
	th.name = "Classic (Builtin)";
	th.type = "Classic";
	th.file = "Classic (Builtin)";
	_themes.push_back(th);

	// we are using only the paths 'themepath', 'extrapath', DATA_PATH and '.'
	// since these are the default locations for the theme files
	// files in other places are ignored in this dialog
	// TODO: let the user browse the complete FS too/only the FS?
	if (ConfMan.hasKey("themepath"))
		addDir(_themes, Common::FilesystemNode(ConfMan.get("themepath")), 0);

#ifdef DATA_PATH
	addDir(_themes, Common::FilesystemNode(DATA_PATH));
#endif

#ifdef MACOSX
	CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (resourceUrl) {
		char buf[256];
		if (CFURLGetFileSystemRepresentation(resourceUrl, true, (UInt8 *)buf, 256)) {
			Common::FilesystemNode resourcePath(buf);
			addDir(_themes, resourcePath, 0);
		}
		CFRelease(resourceUrl);
	}
#endif

	if (ConfMan.hasKey("extrapath"))
		addDir(_themes, Common::FilesystemNode(ConfMan.get("extrapath")));

	addDir(_themes, Common::FilesystemNode("."), 0);

	// Populate the ListWidget
	Common::StringList list;

	for (ThList::const_iterator i = _themes.begin(); i != _themes.end(); ++i)
		list.push_back(i->name);

	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	draw();
}

void ThemeBrowser::addDir(ThList &list, const Common::FilesystemNode &node, int level) {
	if (level < 0)
		return;

	if (!node.exists() || !node.isReadable())
		return;

	Common::FSList fslist;
	if (!node.getChildren(fslist, Common::FilesystemNode::kListAll))
		return;

	for (Common::FSList::const_iterator i = fslist.begin(); i != fslist.end(); ++i) {
		if (i->isDirectory()) {
			addDir(list, *i, level-1);
		} else {
			Entry th;
			if (isTheme(*i, th)) {
				bool add = true;
				for (ThList::const_iterator p = list.begin(); p != list.end(); ++p) {
					if (p->name == th.name || p->file == th.file) {
						add = false;
						break;
					}
				}

				if (add)
					list.push_back(th);
			}
		}
	}
}

bool ThemeBrowser::isTheme(const Common::FilesystemNode &node, Entry &out) {
	Common::ConfigFile cfg;
	Common::String type;

	out.file = node.getName();
	// Remove the filename extension
	while (out.file.lastChar() != '.') {
		out.file.deleteLastChar();
	}
	out.file.deleteLastChar();

	if (out.file.empty())
		return false;

	if (!Theme::themeConfigUseable(out.file, "", &type, &cfg))
		return false;

	out.type = type;

	if (cfg.hasKey("name", "theme"))
		cfg.getKey("name", "theme", out.name);
	else
		out.name = out.file;

	return true;
}

} // end of namespace GUI


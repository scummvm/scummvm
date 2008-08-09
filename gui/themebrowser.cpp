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
ThemeBrowser::ThemeBrowser() : Dialog("Browser") {
	_fileList = 0;

	new StaticTextWidget(this, "Browser.Headline", "Select a Theme");

	// Add file list
	_fileList = new ListWidget(this, "Browser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_fileList->setHints(THEME_HINT_PLAIN_COLOR);

	// Buttons
	new ButtonWidget(this, "Browser.Cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "Browser.Choose", "Choose", kChooseCmd, 0);
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
	th.name = "Modern Development Theme (Builtin) - WIP";
	th.file = "builtin";
	_themes.push_back(th);

	// we are using only the paths 'themepath', 'extrapath', DATA_PATH and '.'
	// since these are the default locations for the theme files
	// files in other places are ignored in this dialog
	// TODO: let the user browse the complete FS too/only the FS?
	if (ConfMan.hasKey("themepath"))
		addDir(_themes, ConfMan.get("themepath"), 0);

#ifdef DATA_PATH
	addDir(_themes, DATA_PATH);
#endif

#ifdef MACOSX
	CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (resourceUrl) {
		char buf[256];
		if (CFURLGetFileSystemRepresentation(resourceUrl, true, (UInt8 *)buf, 256)) {
			Common::String resourcePath = buf;
			addDir(_themes, resourcePath, 0);
		}
		CFRelease(resourceUrl);
	}
#endif

	if (ConfMan.hasKey("extrapath"))
		addDir(_themes, ConfMan.get("extrapath"));

	addDir(_themes, ".", 0);

	// Populate the ListWidget
	Common::StringList list;

	for (ThList::const_iterator i = _themes.begin(); i != _themes.end(); ++i)
		list.push_back(i->name);

	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	draw();
}

void ThemeBrowser::addDir(ThList &list, const Common::String &dir, int level) {
	if (level < 0)
		return;

	FilesystemNode node(dir);

	if (!node.exists() || !node.isReadable())
		return;

	FSList fslist;
	if (!node.getChildren(fslist, FilesystemNode::kListAll))
		return;

	for (FSList::const_iterator i = fslist.begin(); i != fslist.end(); ++i) {
		if (i->isDirectory()) {
			addDir(list, i->getPath(), level-1);
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

bool ThemeBrowser::isTheme(const FilesystemNode &node, Entry &out) {
	out.file = node.getName();
	
	if (!out.file.hasSuffix(".zip") && !out.file.hasSuffix(".stx"))
		return false;
	
	for (int i = out.file.size()-1; out.file[i] != '.' && i > 0; --i) {
		out.file.deleteLastChar();
	}
	out.file.deleteLastChar();

	if (out.file.empty())
		return false;

// TODO: Check if theme is usable.
//	if (!Theme::themeConfigUseable(out.file, "", &type, &cfg))
//		return false;

//	if (cfg.hasKey("name", "theme"))
//		cfg.getKey("name", "theme", out.name);
//	else
		out.name = out.file;

	return true;
}

} // end of namespace GUI


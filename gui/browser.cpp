/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "gui/browser.h"
#include "gui/GuiManager.h"
#include "gui/ListWidget.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/algorithm.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos',
	kGoUpCmd = 'GoUp'
};

#ifdef MACOSX
/* On Mac OS X, use the native file selector dialog. We could do the same for
 * other operating systems.
 */

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog("Browser") {
	_titleRef = CFStringCreateWithCString(0, title, CFStringGetSystemEncoding());
	_isDirBrowser = dirBrowser;
}

BrowserDialog::~BrowserDialog() {
	CFRelease(_titleRef);
}

int BrowserDialog::runModal() {
	NavDialogRef dialogRef;
	WindowRef windowRef = 0;
	NavDialogCreationOptions options;
	NavUserAction result;
	NavReplyRecord reply;
	OSStatus err;
	bool choiceMade = false;

	// Temporarily show the real mouse
	CGDisplayShowCursor(kCGDirectMainDisplay);

	err = NavGetDefaultDialogCreationOptions(&options);
	assert(err == noErr);
	options.windowTitle = _titleRef;
//	options.message = CFSTR("Select your game directory");
	options.modality = kWindowModalityAppModal;

	if (_isDirBrowser)
		err = NavCreateChooseFolderDialog(&options, 0, 0, 0, &dialogRef);
	else
		err = NavCreateChooseFileDialog(&options, 0, 0, 0, 0, 0, &dialogRef);
	assert(err == noErr);

	windowRef = NavDialogGetWindow(dialogRef);

	err = NavDialogRun(dialogRef);
	assert(err == noErr);

	CGDisplayHideCursor(kCGDirectMainDisplay);

	result = NavDialogGetUserAction(dialogRef);

	if (result == kNavUserActionChoose) {
		err = NavDialogGetReply(dialogRef, &reply);
		assert(err == noErr);

		if (reply.validRecord && err == noErr) {
			SInt32 theCount;
			AECountItems(&reply.selection, &theCount);
			assert(theCount == 1);

			AEKeyword keyword;
			FSRef ref;
			char buf[4096];
			err = AEGetNthPtr(&reply.selection, 1, typeFSRef, &keyword, NULL, &ref, sizeof(ref), NULL);
			assert(err == noErr);
			err = FSRefMakePath(&ref, (UInt8*)buf, sizeof(buf)-1);
			assert(err == noErr);

			_choice = Common::FSNode(buf);
			choiceMade = true;
		}

		err = NavDisposeReply(&reply);
		assert(err == noErr);
	}

	NavDialogDispose(dialogRef);

	return choiceMade;
}

#else

/* We want to use this as a general directory selector at some point... possible uses
 * - to select the data dir for a game
 * - to select the place where save games are stored
 * - others???
 */

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog("Browser") {

	_isDirBrowser = dirBrowser;
	_fileList = NULL;
	_currentPath = NULL;

	// Headline - TODO: should be customizable during creation time
	new StaticTextWidget(this, "Browser.Headline", title);

	// Current path - TODO: handle long paths ?
	_currentPath = new StaticTextWidget(this, "Browser.Path", "DUMMY");

	// Add file list
	_fileList = new ListWidget(this, "Browser.List");
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	new ButtonWidget(this, "Browser.Up", "Go up", kGoUpCmd, 0);
	new ButtonWidget(this, "Browser.Cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "Browser.Choose", "Choose", kChooseCmd, 0);
}

void BrowserDialog::open() {
	// Call super implementation
	Dialog::open();

	if (ConfMan.hasKey("browser_lastpath"))
		_node = Common::FSNode(ConfMan.get("browser_lastpath"));
	if (!_node.isDirectory())
		_node = Common::FSNode(".");

	// Alway refresh file list
	updateListing();
}

void BrowserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
		if (_isDirBrowser) {
			// If nothing is selected in the list widget, choose the current dir.
			// Else, choose the dir that is selected.
			int selection = _fileList->getSelected();
			if (selection >= 0) {
				_choice = _nodeContent[selection];
			} else {
				_choice = _node;
			}
			setResult(1);
			close();
		} else {
			int selection = _fileList->getSelected();
			if (selection < 0)
				break;
			if (_nodeContent[selection].isDirectory()) {
				_node = _nodeContent[selection];
				updateListing();
			} else {
				_choice = _nodeContent[selection];
				setResult(1);
				close();
			}
		}
		break;
	case kGoUpCmd:
		_node = _node.getParent();
		updateListing();
		break;
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		if (_nodeContent[data].isDirectory()) {
			_node = _nodeContent[data];
			updateListing();
		} else {
			_choice = _nodeContent[data];
			setResult(1);
			close();
		}
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void BrowserDialog::updateListing() {
	// Update the path display
	_currentPath->setLabel(_node.getPath());

	// We memorize the last visited path.
	ConfMan.set("browser_lastpath", _node.getPath());

	// Read in the data from the file system
	Common::FSNode::ListMode listMode =
	         _isDirBrowser ? Common::FSNode::kListDirectoriesOnly
	                       : Common::FSNode::kListAll;
	if (!_node.getChildren(_nodeContent, listMode)) {
		_nodeContent.clear();
	} else {
		Common::sort(_nodeContent.begin(), _nodeContent.end());
	}

	// Populate the ListWidget
	Common::StringList list;
	for (Common::FSList::iterator i = _nodeContent.begin(); i != _nodeContent.end(); ++i) {
		if (!_isDirBrowser && i->isDirectory())
			list.push_back(i->getDisplayName() + "/");
		else
			list.push_back(i->getDisplayName());
	}
	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	draw();
}

#endif	// MACOSX

} // End of namespace GUI

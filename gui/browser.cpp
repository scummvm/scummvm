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

#include "stdafx.h"
#include "gui/browser.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

#include "backends/fs/fs.h"

#include "common/system.h"

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
	: Dialog(20, 10, 320 -2 * 20, 200 - 2 * 10) {
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
	
	// If in fullscreen mode, switch to windowed mode
	bool wasFullscreen = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	if (wasFullscreen)
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, false);

	// Temporarily show the real mouse
	ShowCursor();
	
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

	HideCursor();

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
			
			_choice = FilesystemNode(buf);
			choiceMade = true;
		}
 
		err = NavDisposeReply(&reply);
		assert(err == noErr);
	}

	NavDialogDispose(dialogRef);

	// If we were in fullscreen mode, switch back
	if (wasFullscreen)
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);

	return choiceMade;
}

#else

/* We want to use this as a general directory selector at some point... possible uses
 * - to select the data dir for a game
 * - to select the place where save games are stored
 * - others???
 */

BrowserDialog::BrowserDialog(const char *title, bool dirBrowser)
	: Dialog(0, 0, 320, 200) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	GUI::WidgetSize ws;
	int lineHeight;
	int buttonHeight;
	int buttonWidth;
	const Graphics::Font *font;

	_w = (screenW * 7) / 8;
	_h = (screenH * 9) / 10;
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
	} else {
		ws = GUI::kNormalWidgetSize;
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		buttonWidth = kButtonWidth;
		buttonHeight = kButtonHeight;
	}

	lineHeight = font->getFontHeight() + 2;

	_isDirBrowser = dirBrowser;
	_fileList = NULL;
	_currentPath = NULL;

	// Headline - TODO: should be customizable during creation time
	new StaticTextWidget(this, 10, lineHeight, _w - 2 * 10, lineHeight, title, kTextAlignCenter, ws);

	// Current path - TODO: handle long paths ?
	_currentPath = new StaticTextWidget(this, 10, 2 * lineHeight, _w - 2 * 10, kLineHeight, "DUMMY", kTextAlignLeft, ws);

	// Add file list
	_fileList = new ListWidget(this, 10, 3 * lineHeight, _w - 2 * 10, _h - 3 * lineHeight - buttonHeight - 14, ws);
	_fileList->setNumberingMode(kListNumberingOff);
	_fileList->setEditable(false);

	// Buttons
	addButton(10, _h - buttonHeight - 8, "Go up", kGoUpCmd, 0, ws);
	addButton(_w - 2 * (buttonWidth + 10), _h - buttonHeight - 8, "Cancel", kCloseCmd, 0, ws);
	addButton(_w - (buttonWidth + 10), _h - buttonHeight - 8, "Choose", kChooseCmd, 0, ws);
}

void BrowserDialog::open() {
	// If no node has been set, or the last used one is now invalid,
	// go back to the root/default dir.
	if (!_node.isValid()) {
		_node = FilesystemNode();
	}

	// Alway refresh file list
	updateListing();
	
	// Call super implementation
	Dialog::open();
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
	_currentPath->setLabel(_node.path());

	// Read in the data from the file system
	if (_isDirBrowser)
		_nodeContent = _node.listDir(AbstractFilesystemNode::kListDirectoriesOnly);
	else
		_nodeContent = _node.listDir(AbstractFilesystemNode::kListAll);
	_nodeContent.sort();

	// Populate the ListWidget
	Common::StringList list;
	int size = _nodeContent.size();
	for (int i = 0; i < size; i++) {
		if (!_isDirBrowser && _nodeContent[i].isDirectory())
			list.push_back(_nodeContent[i].displayName() + "/");
		else
			list.push_back(_nodeContent[i].displayName());
	}
	_fileList->setList(list);
	_fileList->scrollTo(0);

	// Finally, redraw
	draw();
}

#endif	// MACOSX

} // End of namespace GUI

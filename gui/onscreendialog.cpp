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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifdef SDL_BACKEND

#include "onscreendialog.h"
#include "gui/gui-manager.h"
#include "common/rect.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "gui/editrecorddialog.h"

namespace GUI {

bool OnScreenDialog::isVisible() const {
	return true;
}

enum {
	kStopCmd = 'STOP',
	kEditCmd = 'EDIT',
	kSwitchModeCmd = 'MODE',
	kFastModeCmd = 'FAST'
};

void OnScreenDialog::reflowLayout() {
	GuiObject::reflowLayout();
}

void OnScreenDialog::releaseFocus() {
}

OnScreenDialog::OnScreenDialog(bool isRecord) : Dialog(0, 0, 210, 40) {
	GUI::PicButtonWidget *btn;
	int textWidth = 50;
	int buttonWidth = 32;
	int buttonHeight = 32;
	int buttonX = 5;
	int buttonY = 5;
	int buttonDistance = 5;
	btn = new PicButtonWidget(this, buttonX, buttonY, buttonWidth, buttonHeight, 0, kStopCmd, 0);
	btn->useThemeTransparency(true);
	btn->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageStopbtn));
	buttonX += buttonHeight + buttonDistance;
	if (isRecord) {
		btn = new PicButtonWidget(this, buttonX, buttonY, buttonWidth, buttonHeight, 0, kEditCmd, 0);
		btn->useThemeTransparency(true);
		btn->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageEditbtn));
		buttonX += buttonHeight + buttonDistance;
	} else {
		btn = new PicButtonWidget(this, buttonX, buttonY, buttonWidth, buttonHeight, 0, kSwitchModeCmd, 0);
		btn->useThemeTransparency(true);
		btn->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSwitchModebtn));
		buttonX += buttonHeight + buttonDistance;
		btn = new PicButtonWidget(this, buttonX, buttonY, buttonWidth, buttonHeight, 0, kFastModeCmd, 0);
		btn->useThemeTransparency(true);
		btn->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageFastReplaybtn));
		buttonX += buttonHeight + buttonDistance;
	}
	text = new GUI::StaticTextWidget(this, buttonX, buttonY, textWidth, buttonHeight, "00:00:00", Graphics::kTextAlignLeft);
	_enableDrag = false;
	_mouseOver = false;
	_editDlgShown = false;
}

void OnScreenDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Common::Event eventRTL;
	switch (cmd) {
	case kStopCmd:
		eventRTL.type = Common::EVENT_RTL;
		g_system->getEventManager()->pushEvent(eventRTL);
		close();
		break;
	case kEditCmd:
		dlg = new EditRecordDialog(g_eventRec.getAuthor(), g_eventRec.getName(), g_eventRec.getNotes());
		CursorMan.lock(false);
		g_eventRec.setRedraw(false);
		g_system->showOverlay();
		_editDlgShown = true;
		dlg->runModal();
		_editDlgShown = false;
		g_system->hideOverlay();
		g_eventRec.setRedraw(true);
		CursorMan.lock(true);
		g_eventRec.setAuthor(((EditRecordDialog *)dlg)->getAuthor());
		g_eventRec.setName(((EditRecordDialog *)dlg)->getName());
		g_eventRec.setNotes(((EditRecordDialog *)dlg)->getNotes());
		delete dlg;
		break;
	case kSwitchModeCmd:
		if (g_eventRec.switchMode()) {
			close();
		}
		break;
	case kFastModeCmd:
		g_eventRec.switchFastMode();
		break;
	}
}

void OnScreenDialog::setReplayedTime(uint32 newTime) {
	if (newTime - lastTime > 1000) {
		uint32 seconds = newTime / 1000;
		text->setLabel(Common::String::format("%.2d:%.2d:%.2d", seconds / 3600 % 24, seconds / 60 % 60, seconds % 60));
		lastTime = newTime;
	}
}

OnScreenDialog::~OnScreenDialog() {
}

void OnScreenDialog::handleMouseMoved(int x, int y, int button) {
	if (_enableDrag) {
		_x = _x + x - _dragPoint.x;
		_y = _y + y - _dragPoint.y;
	}
	Dialog::handleMouseMoved(x, y, button);
	if (isMouseOver(x, y)) {
		g_system->updateScreen();
		if (_mouseOver == false) {
			g_gui.theme()->showCursor();
			CursorMan.lock(true);
		}
		_mouseOver = true;
	} else {
		if (_mouseOver == true) {
			CursorMan.lock(false);
			g_gui.theme()->hideCursor();
		}
		_mouseOver = false;
	}
}

void OnScreenDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isMouseOver(x, y)) {
		_dragPoint.x = x;
		_dragPoint.y = y;
		_enableDrag = true;
	}
	Dialog::handleMouseDown(x, y, button, clickCount);
}

void OnScreenDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isMouseOver(x, y)) {

	}
	_enableDrag = false;
	Dialog::handleMouseUp(x, y, button, clickCount);
}

bool OnScreenDialog::isMouseOver(int x, int y) {
	return (x >= 0 && x < _w && y >= 0 && y < _h);
}

bool OnScreenDialog::isMouseOver() {
	return _mouseOver;
}

void OnScreenDialog::close() {
	CursorMan.lock(false);
	Dialog::close();
}

Dialog *OnScreenDialog::getActiveDlg() {
	if (_editDlgShown) {
		return dlg;
	} else {
		return this;
	}
}

bool OnScreenDialog::isEditDlgVisible() {
	return _editDlgShown;
}

}
#endif
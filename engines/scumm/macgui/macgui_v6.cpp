/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/config-manager.h"
#include "common/macresman.h"

#include "engines/engine.h"

#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/macgui/macwindowmanager.h"

#if 0
#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"
#endif

#include "graphics/surface.h"

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/file.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_v6.h"

#if 0
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#endif

namespace Scumm {

// ===========================================================================
// The Mac SCUMM v6 (and later) GUI.
// ===========================================================================

MacV6Gui::MacV6Gui(ScummEngine *vm, const Common::Path &resourceFile) : MacGuiImpl(vm, resourceFile) {
	_backupPalette = nullptr;
	_backupSurface = nullptr;
}

MacV6Gui::~MacV6Gui() {
	if (_backupSurface) {
		_backupSurface->free();
		delete _backupSurface;
	}

	delete _backupPalette;
}

const Graphics::Font *MacV6Gui::getFontByScummId(int32 id) {
	// V5 games do not use CharsetRendererMac
	return nullptr;
}

bool MacV6Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	return false;
}

bool MacV6Gui::handleMenu(int id, Common::String &name) {
	// Don't call the original method. The menus are too different.
	// TODO: Separate the common code into its own method?

	// This menu item (e.g. a menu separator) has no action, so it's
	// handled trivially.
	if (id == 0)
		return true;

	// This is how we keep the menu bar visible.
	Graphics::MacMenu *menu = _windowManager->getMenu();

	// If the menu is opened through a shortcut key, force it to activate
	// to avoid screen corruption. In that case, we also force the menu to
	// close afterwards, or the game will stay paused. Which is
	// particularly bad during a restart.

	if (!menu->_active) {
		_windowManager->activateMenu();
		_forceMenuClosed = true;
	}

	menu->closeMenu();
	menu->setActive(true);
	menu->setVisible(true);
	updateWindowManager();

	int saveSlotToHandle = -1;
	Common::String savegameName;

	// The Dig and Full Throttle don't have a Restart menu entry
	if (_vm->_game.version > 6 && id >= 204 && id < 300)
		id++;

	switch (id) {
	case 100:	// About
		runAboutDialog();
		return true;

	case 200:	// Open
		debug("Open");
		if (runOpenDialog(saveSlotToHandle)) {
			if (saveSlotToHandle > -1) {
				_vm->loadGameState(saveSlotToHandle);
			}
		}

		return true;

	case 201:	// Save
		debug("Save");
		_vm->beginTextInput();
		if (runSaveDialog(saveSlotToHandle, savegameName)) {
			if (saveSlotToHandle > -1) {
				_vm->saveGameState(saveSlotToHandle, savegameName);
			}
		}
		_vm->endTextInput();
		return true;

	case 202:
		debug("Skip scene");
		return true;

	case 203:
		debug("Resume");
		return true;

	case 204:	// Restart
		debug("Restart");
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 205:
		debug("Preferences");
		break;

	case 206:
		debug("Quit");
		break;

	// In the original, the Edit menu is active during save dialogs, though
	// only Cut, Copy and Paste.

	case 300:	// Undo
	case 301:	// Cut
	case 302:	// Copy
	case 303:	// Paste
	case 304:	// Clear
		return true;

	case 403:
		debug("Graphics smoothing");
		break;
	}

	return false;
}

void MacV6Gui::onMenuOpen() {
	MacGuiImpl::onMenuOpen();

	Graphics::Surface *screen = _vm->_macScreen;

	_backupSurface = new Graphics::Surface();
	_backupSurface->copyFrom(*screen);

	_backupPalette = new byte[256 * 3];

	screen->fillRect(Common::Rect(screen->w, screen->h), 255);
	memcpy(_backupPalette, _vm->_currentPalette, 256 * 3);

	// HACK: Make sure we have the Mac window manager's preferred colors

	_vm->setPalColor(0, 0, 0, 0);          // Black
	_vm->setPalColor(1, 0x80, 0x80, 0x80); // Gray80
	_vm->setPalColor(2, 0x88, 0x88, 0x88); // Gray88
	_vm->setPalColor(3, 0xEE, 0xEE, 0xEE); // GrayEE
	_vm->setPalColor(4, 0xFF, 0xFF, 0xFF); // White
	_vm->setPalColor(5, 0x00, 0xFF, 0x00); // Green
	_vm->setPalColor(6, 0x00, 0xCF, 0x00); // Green2

	for (int i = 7; i < 256; i++)
		_vm->setPalColor(i, 0, 0, 0);

	_vm->updatePalette();
	_system->copyRectToScreen(screen->getBasePtr(0, 0), screen->pitch, 0, 0, screen->w, screen->h);
	_system->updateScreen();

	// HACK: Make sure the Mac window manager is operating on a blank screen
	_windowManager->disableScreenCopy();
	_windowManager->activateScreenCopy();
}

void MacV6Gui::onMenuClose() {
	MacGuiImpl::onMenuClose();
	Graphics::Surface *screen = _vm->_macScreen;

	screen->copyFrom(*_backupSurface);

	byte *p = _backupPalette;
	for (int i = 0; i < 256; i++, p += 3)
		_vm->setPalColor(i, p[0], p[1], p[2]);

	_system->copyRectToScreen(screen->getBasePtr(0, 0), screen->pitch, 0, 0, screen->w, screen->h);
	_vm->updatePalette();

	_backupSurface->free();
	delete _backupSurface;
	_backupSurface = nullptr;

	delete _backupPalette;
	_backupPalette = nullptr;
}

void MacV6Gui::runAboutDialog() {
	ScummFile aboutFile(_vm);
	if (!_vm->openFile(aboutFile, "ABOUT"))
		return;

	PauseToken token = _vm->pauseEngine();

	const int aboutW = 480;
	const int aboutH = 299;

	Graphics::Surface *screen = _vm->_macScreen;
	Common::Rect aboutArea(aboutW, aboutH);

	const int aboutX = (screen->w - aboutW) / 2;
	const int aboutY = (screen->h - aboutH) / 2;

	aboutArea.moveTo(aboutX, aboutY);

	Graphics::Surface aboutImage = screen->getSubArea(aboutArea);

	uint black = 0;

	for (uint i = 0; i < 256; i++) {
		byte r = aboutFile.readByte();
		byte g = aboutFile.readByte();
		byte b = aboutFile.readByte();
		_vm->setPalColor(i, r, g, b);

		if (r == 0 && g == 0 && b == 0)
			black = i;
	}

	// The screen is already black, but what's black in the palette may
	// have changed. Also, we want to clear the menu area.
	screen->fillRect(Common::Rect(screen->w, screen->h), black);

	for (int y = 0; y < aboutH; y++) {
		for (int x = 0; x < aboutW; x++) {
			byte *dst = (byte *)aboutImage.getBasePtr(x, y);
			*dst = aboutFile.readByte();
		}
	}

	aboutFile.close();

	_system->copyRectToScreen(screen->getBasePtr(0, 0), screen->pitch, 0, 0, screen->w, screen->h);
	_vm->updatePalette();

	bool done = false;

	while (!_vm->shouldQuit() && !done) {
		Common::Event event;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				done = true;
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					done = true;
				break;

			default:
				break;
			}
		}

		_system->delayMillis(10);
		_system->updateScreen();
	}

	token.clear();
}

bool MacV6Gui::runOptionsDialog() {
	return false;
}

void MacV6Gui::resetAfterLoad() {
	reset();
}

bool MacV6Gui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	if (_vm->_userPut <= 0)
		return false;

	return false;
}

} // End of namespace Scumm

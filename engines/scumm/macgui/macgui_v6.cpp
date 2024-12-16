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
	_backupScreen = nullptr;
	_backupPalette = nullptr;

	if (_vm->_game.id == GID_TENTACLE)
		_gameName = "Day of the Tentacle";
	else if (_vm->_game.id == GID_SAMNMAX)
		_gameName = "Sam & Max";
	else if (_vm->_game.id == GID_DIG)
		_gameName = "The Dig";
	else if (_vm->_game.id == GID_FT)
		_gameName = "Full Throttle";
	else if (_vm->_game.id == GID_MANIAC)
		_gameName = "Maniac Mansion";
	else
		_gameName = "Some Game I Do Not Know";
}

MacV6Gui::~MacV6Gui() {
	if (_backupScreen) {
		_backupScreen->free();
		delete _backupScreen;
	}

	delete _backupPalette;
}

bool MacV6Gui::readStrings() {
	_strsStrings.clear();
	_strsStrings.reserve(128);
	for (int i = 0; i < 128; i++)
		_strsStrings.emplace_back("");

	_strsStrings[kMSIAboutGameName] = "About " + _gameName + "...";
	return true;
}

const Graphics::Font *MacV6Gui::getFontByScummId(int32 id) {
	// V6 and V7 games (and Maniac Mansion) do not use CharsetRendererMac
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
		_vm->processKeyboard(Common::KEYCODE_ESCAPE);
		return true;

	case 203:
		debug("Resume");
		return true;

	case 204:	// Restart
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 205:
		debug("Preferences");
		break;

	case 206:
		if (runQuitDialog())
			_vm->quitGame();
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

uint32 MacV6Gui::getBlack() const {
	return 255;
}

uint32 MacV6Gui::getWhite() const {
	return 251;
}

void MacV6Gui::saveScreen() {
	if (_screenSaveLevel++ == 0) {
		_suspendPaletteUpdates = true;

		Graphics::Surface *screen = _vm->_macScreen;

		_backupScreen = new Graphics::Surface();
		_backupScreen->copyFrom(*screen);

		// We have to grab the actual palette, becaues the engine
		// palette may not be what's on screen, e.g. during SMUSH
		// movies.

		_backupPalette = new byte[256 * 3];

		_system->getPaletteManager()->grabPalette(_backupPalette, 0, 256);

		Graphics::Palette palette(256);

		// Colors used by the Mac Window Manager
		palette.set(255, 0x00, 0x00, 0x00); // Black
		palette.set(254, 0x80, 0x80, 0x80); // Gray80
		palette.set(253, 0x88, 0x88, 0x88); // Gray88
		palette.set(252, 0xEE, 0xEE, 0xEE); // GrayEE
		palette.set(251, 0xFF, 0xFF, 0xFF); // White
		palette.set(250, 0x00, 0xFF, 0x00); // Green
		palette.set(249, 0x00, 0xCF, 0x00); // Green2

		// Colors used by Mac dialog window borders
		palette.set(248, 0xCC, 0xCC, 0xFF);
		palette.set(247, 0xBB, 0xBB, 0xBB);
		palette.set(246, 0x66, 0x66, 0x99);

		for (int i = 0; i < 246; i++)
			palette.set(i, 0x00, 0x00, 0x00);

		_windowManager->passPalette(palette.data(), 256);

		for (int i = 0; i < 256; i++) {
			byte r, g, b;

			palette.get(i, r, g, b);
			r = _vm->_macGammaCorrectionLookUp[r];
			g = _vm->_macGammaCorrectionLookUp[g];
			b = _vm->_macGammaCorrectionLookUp[b];
			palette.set(i, r, g, b);
		}

		screen->fillRect(Common::Rect(screen->w, screen->h), getBlack());
		_system->copyRectToScreen(screen->getBasePtr(0, 0), screen->pitch, 0, 0, screen->w, screen->h);

		_system->getPaletteManager()->setPalette(palette);

		if (_windowManager->_screenCopy)
			_windowManager->_screenCopy->copyFrom(*_vm->_macScreen);

		_system->updateScreen();
	}
}

void MacV6Gui::restoreScreen() {
	if (--_screenSaveLevel == 0) {
		_suspendPaletteUpdates = false;

		Graphics::Surface *screen = _vm->_macScreen;

		screen->copyFrom(*_backupScreen);

		_system->copyRectToScreen(screen->getBasePtr(0, 0), screen->pitch, 0, 0, screen->w, screen->h);

		_system->getPaletteManager()->setPalette(_backupPalette, 0, 256);

		_backupScreen->free();
		delete _backupScreen;
		_backupScreen = nullptr;

		delete _backupPalette;
		_backupPalette = nullptr;
	}
}

void MacV6Gui::onMenuOpen() {
	MacGuiImpl::onMenuOpen();
	saveScreen();
}

void MacV6Gui::onMenuClose() {
	MacGuiImpl::onMenuClose();
	restoreScreen();
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

	Graphics::Palette palette(256);

	for (uint i = 0; i < 256; i++) {
		byte r = aboutFile.readByte();
		byte g = aboutFile.readByte();
		byte b = aboutFile.readByte();

		if (r == 0 && g == 0 && b == 0)
			black = i;

		r = _vm->_macGammaCorrectionLookUp[r];
		g = _vm->_macGammaCorrectionLookUp[g];
		b = _vm->_macGammaCorrectionLookUp[b];

		palette.set(i, r, g, b);
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
	_system->getPaletteManager()->setPalette(palette);

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

bool MacV6Gui::runQuitDialog() {
	// TODO: 192 in Maniac Mansion? The icon looks wrong in that one.
	MacDialogWindow *window = createDialog(128);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacStaticText *textWidget = (MacStaticText *)window->getWidget(kWidgetStaticText);

	textWidget->setWordWrap(true);

	window->setDefaultWidget(buttonOk);

	Common::Array<int> deferredActionsIds;

	// When quitting, the default action is to quit
	bool ret = true;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonOk->getId())
			break;

		if (clicked == buttonCancel->getId()) {
			ret = false;
			break;
		}
	}

	delete window;
	return ret;
}

bool MacV6Gui::runRestartDialog() {
	// TODO: 193 in Maniac Mansion? The icon looks wrong in that one.
	MacDialogWindow *window = createDialog(137);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacStaticText *textWidget = (MacStaticText *)window->getWidget(kWidgetStaticText);

	textWidget->setWordWrap(true);

	window->setDefaultWidget(buttonOk);

	window->addSubstitution("");
	window->addSubstitution("");
	window->addSubstitution("");
	window->addSubstitution(_gameName);

	Common::Array<int> deferredActionsIds;

	// When quitting, the default action is to quit
	bool ret = true;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == buttonOk->getId())
			break;

		if (clicked == buttonCancel->getId()) {
			ret = false;
			break;
		}
	}

	delete window;
	return ret;
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

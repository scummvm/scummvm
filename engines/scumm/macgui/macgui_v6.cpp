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

#include "audio/mixer.h"

#include "engines/engine.h"
#include "engines/metaengine.h"
#include "engines/savestate.h"

#include "graphics/color_quantizer.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_v6.h"

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
#ifdef ENABLE_SCUMM_7_8
	else if (_vm->_game.id == GID_DIG)
		_gameName = "The Dig";
	else if (_vm->_game.id == GID_FT)
		_gameName = "Full Throttle";
#endif
	else if (_vm->_game.id == GID_MANIAC)
		_gameName = "Maniac Mansion";
	else
		_gameName = "Some Game I Do Not Know";

	if (_vm->_game.features & GF_DEMO)
		_gameName += " Demo";
}

MacV6Gui::~MacV6Gui() {
	if (_backupScreen) {
		_backupScreen->free();
		delete _backupScreen;
	}

	delete[] _backupPalette;
}

bool MacV6Gui::initialize() {
	if (!MacGuiImpl::initialize())
		return false;

	// We can't use the name of the menus here, because there are
	// non-English versions. Let's hope the menu positions are always the
	// same, at least!

	Graphics::MacMenu *menu = _windowManager->getMenu();
	Graphics::MacMenuItem *videoMenu = menu->getMenuItem(3);

	menu->getSubMenuItem(videoMenu, 0)->enabled = false; // Small
	menu->getSubMenuItem(videoMenu, 1)->enabled = false; // Interlaced

	if (_vm->_game.id == GID_MANIAC) {
		Graphics::MacMenuItem *soundMenu = menu->getMenuItem(4);

		menu->getSubMenuItem(soundMenu, 0)->enabled = false; // Music
		menu->getSubMenuItem(soundMenu, 1)->enabled = false; // Effects
		menu->getSubMenuItem(soundMenu, 3)->enabled = false; // Toggle Text & Voice
		menu->getSubMenuItem(soundMenu, 5)->enabled = false; // Text Only
		menu->getSubMenuItem(soundMenu, 6)->enabled = false; // Voice Only
		menu->getSubMenuItem(soundMenu, 7)->enabled = false; // Text & Voice
	}

	return true;
}

bool MacV6Gui::readStrings() {
	_strsStrings.clear();
	_strsStrings.reserve(128);
	for (int i = 0; i < 128; i++)
		_strsStrings.emplace_back("");
	return true;
}

const Graphics::Font *MacV6Gui::getFontByScummId(int32 id) {
	// V6 and V7 games (and Maniac Mansion) do not use CharsetRendererMac
	return nullptr;
}

bool MacV6Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	return false;
}

void MacV6Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	if (_vm->_game.id != GID_MANIAC)
		return;

	byte invertedMacArrow[] = {
		0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0xFF,
		0x00, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x0F, 0x0F, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x0F, 0x00, 0xFF, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0xFF, 0xFF, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF, 0xFF,
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0x0F, 0x00, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF
	};

	memcpy(_vm->_grabbedCursor, invertedMacArrow, sizeof(invertedMacArrow));

	width = 11;
	height = 16;
	hotspotX = 1;
	hotspotY = 1;
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
	bool syncSoundSettings = false;
	Common::String savegameName;

	// The Dig and Full Throttle don't have a Restart menu entry
	if (_vm->_game.version > 6 && id >= 204 && id < 300)
		id++;

	switch (id) {
	case 100:	// About
		runAboutDialog();
		return true;

	case 200:	// Open
		if (runOpenDialog(saveSlotToHandle)) {
			if (saveSlotToHandle > -1) {
				_vm->loadGameState(saveSlotToHandle);
			}
		}

		return true;

	case 201:	// Save
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
		// Resume is supposed to simply close the Mac menu, and that
		// happens automatically when selecting any menu item.
		return true;

	case 204:	// Restart
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 205:
		runOptionsDialog();
		return true;

	case 206:
		if (runQuitDialog())
			_vm->quitGame();
		return true;

	// In the original, the Edit menu is active during save dialogs, though
	// only Cut, Copy and Paste.

	case 300:	// Undo
	case 301:	// Cut
	case 302:	// Copy
	case 303:	// Paste
	case 304:	// Clear
		return true;

	case 403:	// Graphics Smoothing
		if (_vm->_game.id != GID_MANIAC)
			_vm->mac_toggleSmoothing();
		return true;

	case 500:	// Music
		_vm->_soundEnabled = (_vm->_soundEnabled & ~8) ^ 2;
		ConfMan.setBool("music_mute", !(_vm->_soundEnabled & 2));
		ConfMan.setBool("mute", (_vm->_soundEnabled == 0 && _vm->_voiceMode == 2));
		syncSoundSettings = true;
		break;

	case 501:	// Effects
		_vm->_soundEnabled = (_vm->_soundEnabled & ~8) ^ 1;
		ConfMan.setBool("sfx_mute", !(_vm->_soundEnabled & 1));
		ConfMan.setBool("mute", (_vm->_soundEnabled == 0 && _vm->_voiceMode == 2));
		syncSoundSettings = true;
		break;

	case 502:	// Toggle Text & Voice
		switch (_vm->_voiceMode) {
		case 0:	// Voice Only -> Text & Voice
			ConfMan.setBool("subtitles", true);
			ConfMan.setBool("speech_mute", false);
			break;

		case 1:	// Text & Voice -> Text Only
			ConfMan.setBool("subtitles", true);
			ConfMan.setBool("speech_mute", true);
			break;

		case 2:	// Text Only -> Voice Only
			ConfMan.setBool("subtitles", false);
			ConfMan.setBool("speech_mute", false);
			break;

		default:
			warning("Invalid voice mode %d", _vm->_voiceMode);
			return true;
		}

		syncSoundSettings = true;
		break;

	case 503:	// Text Only
		ConfMan.setBool("subtitles", true);
		ConfMan.setBool("speech_mute", true);
		syncSoundSettings = true;
		break;

	case 504:	// Voice Only
		ConfMan.setBool("subtitles", false);
		ConfMan.setBool("speech_mute", false);
		syncSoundSettings = true;
		break;

	case 505:	// Text & Voice
		ConfMan.setBool("subtitles", true);
		ConfMan.setBool("speech_mute", false);
		syncSoundSettings = true;
		break;

	default:
		warning("Unknown menu command: %d", id);
		break;
	}

	if (syncSoundSettings) {
		ConfMan.flushToDisk();
		_vm->syncSoundSettings();
		return true;
	}

	return false;
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

		setMacGuiColors(palette);

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

		delete[] _backupPalette;
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

void MacV6Gui::drawDottedFrame(MacDialogWindow *window, Common::Rect bounds, int x1, int x2) {
	Graphics::Surface *s = window->innerSurface();
	uint32 black = getBlack();

	for (int x = bounds.left; x < bounds.right; x++) {
		if (((x + bounds.bottom - 1) & 1) == 0)
			s->setPixel(x, bounds.bottom - 1, black);

		if ((x <= x1 || x >= x2) && ((x + bounds.top) & 1) == 0)
			s->setPixel(x, bounds.top, black);
	}

	for (int y = bounds.top; y < bounds.bottom; y++) {
		if (((bounds.left + y) & 1) == 0)
			s->setPixel(bounds.left, y, black);

		if (((bounds.right - 1 + y) & 1) == 0)
			s->setPixel(bounds.right - 1, y, black);
	}
}

MacGuiImpl::MacImageSlider *MacV6Gui::addSlider(MacDialogWindow *window, int x, int y, int width, int minValue, int maxValue, int primaryMarkings) {
	int numMarkings = (maxValue - minValue) + 1;

	Graphics::Surface *s = window->innerSurface();

	uint32 gray = _windowManager->findBestColor(0xCD, 0xCD, 0xCD);
	uint32 black = getBlack();

	Common::Rect r(width, 12);
	r.moveTo(x, y);

	s->fillRect(r, gray);
	s->frameRect(r, black);

	int yt = y + 14;

	int *positions = new int[numMarkings];

	for (int i = 0; i < numMarkings; i++) {
		int ht = ((i % primaryMarkings) == 0) ? 4 : 2;
		int xt = x + (i * (width - 1)) / (numMarkings - 1);
		s->vLine(xt, yt, yt + ht, black);
		positions[i] = xt - x;
	}

	MacImage *handle = window->addIcon(x - 6, y - 4, 300, true);
	MacImageSlider *slider = window->addImageSlider(Common::Rect(x - 6, y - 4, x + width + 7, y + 16), handle, true, 0, width - 1, minValue, maxValue);

	for (int i = 0; i < numMarkings; i++)
		slider->addStop(positions[i], minValue + i);

	slider->setSnapWhileDragging(true);
	slider->setValue(minValue);

	delete[] positions;
	return slider;
}

void MacV6Gui::runAboutDialog() {
	// While there is a menu item for a Maniac Mansion credits screen, it
	// doesn't do anything.
	//
	// "I was probably exhausted from hand-placing all the letters on the
	// DOTT credits screen to make one for Maniac." -- Aaron Giles,
	// December 30, 2024.
	if (_vm->_game.id == GID_MANIAC)
		return;

	if (_vm->_game.features & GF_DEMO) {
		// HACK: Use the largest bounds as default for unknown demos
		// It would be nice if we could figure these out automatically
		Common::Rect bounds(117, 5, 523, 384);

		if (_vm->_game.id == GID_TENTACLE || _vm->_game.id == GID_SAMNMAX) {
			bounds.left = 117;
			bounds.top = 5;
			bounds.right = 523;
			bounds.bottom = 384;
#ifdef ENABLE_SCUMM_7_8
		} else if (_vm->_game.id == GID_DIG || _vm->_game.id == GID_FT) {
			bounds.left = 121;
			bounds.top = 15;
			bounds.right = 519;
			bounds.bottom = 364;
#endif
		}

		MacDialogWindow *window = createDialog(136, bounds);
		MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);

		window->setDefaultWidget(buttonOk);

		while (!_vm->shouldQuit()) {
			MacDialogEvent event;

			while (window->runDialog(event)) {
				switch (event.type) {
				case kDialogClick:
					if (event.widget == buttonOk) {
						delete window;
						return;
					}
					break;

				default:
					break;
				}
			}

			window->delayAndUpdate();
		}

		delete window;
		return;
	}

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

void MacV6Gui::updateThumbnail(MacDialogWindow *window, Common::Rect drawArea, int saveSlot) {
	if (_vm->_game.id == GID_MANIAC)
		return;

	if (saveSlot < 0)
		return;

	SaveStateDescriptor desc = _vm->getMetaEngine()->querySaveMetaInfos(_vm->_targetName.c_str(), saveSlot);

	const Graphics::Surface *thumbnail = desc.getThumbnail();

	Common::HashMap<uint32, uint16> paletteMap;

	int diff = thumbnail->h - drawArea.height();

	int yMin = diff / 2;
	int yMax = thumbnail->h - (diff / 2);

	assert(drawArea.width() == thumbnail->w);
	assert(drawArea.height() == yMax - yMin);

	Common::Rect thumbnailRect(0, yMin, drawArea.width(), yMax);

	// We don't know in advance how many colors the thumbnail is going to
	// use. Reduce the image to a smaller palette. We reserve 10 colors for
	// the Mac GUI itself.

	Graphics::ColorQuantizer quantizer(245);

	for (int y = yMin; y < yMax; y++) {
		for (int x = 0; x < thumbnail->w; x++) {
			uint32 color = thumbnail->getPixel(x, y);

			byte r, g, b;
			thumbnail->format.colorToRGB(color, r, g, b);

			quantizer.addColor(r, g, b);
		}
	}

	Graphics::Palette *palette = quantizer.getPalette();
	_system->getPaletteManager()->setPalette(*palette);
	delete palette;

	Graphics::Palette fullPalette = _system->getPaletteManager()->grabPalette(0, 256);

	Graphics::Surface *palettedThumbnail = thumbnail->convertTo(window->innerSurface()->format, nullptr, 0, fullPalette.data(), fullPalette.size());

	window->innerSurface()->copyRectToSurface(*palettedThumbnail, drawArea.left, drawArea.top, thumbnailRect);
	window->markRectAsDirty(drawArea);

	palettedThumbnail->free();
	delete palettedThumbnail;
}

bool MacV6Gui::runOpenDialog(int &saveSlotToHandle) {
	// Widgets:
	//
	// 0 - Open button
	// 1 - Cancel button
	// 2 - Unknown item type 129
	// 3 - User item
	// 4 - Eject button
	// 5 - Desktop button
	// 6 - User item
	// 7 - User item
	// 8 - Picture (thumbnail?)
	//
	// Not in Maniac Mansion:
	//
	// 9 - User item
	// 10 - "Where you were:" text

	int dialogId = (_vm->_game.id == GID_MANIAC) ? 384 : 256;
	MacDialogWindow *window = createDialog(dialogId);

	MacButton *buttonSave = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacButton *buttonEject = (MacButton *)window->getWidget(kWidgetButton, 2);
	MacButton *buttonDesktop = (MacButton *)window->getWidget(kWidgetButton, 3);

	window->setDefaultWidget(buttonSave);
	buttonEject->setEnabled(false);
	buttonDesktop->setEnabled(false);

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	Common::Rect thumbnailRect(12, 32, 172, 132);

	MacListBox *listBox;

	if (_vm->_game.id == GID_MANIAC) {
		listBox = window->addListBox(Common::Rect(10, 31, 228, 161), savegameNames, true);
		drawFakePathList(window, Common::Rect(10, 8, 228, 27), _gameName.c_str());
		drawFakeDriveLabel(window, Common::Rect(238, 10, 335, 26), "ScummVM");
	} else {
		listBox = window->addListBox(Common::Rect(184, 31, 402, 161), savegameNames, true);
		drawFakePathList(window, Common::Rect(184, 8, 402, 27), _gameName.c_str());
		drawFakeDriveLabel(window, Common::Rect(412, 10, 509, 26), "ScummVM");
		window->innerSurface()->frameRect(Common::Rect(thumbnailRect.left - 1, thumbnailRect.top - 1, thumbnailRect.right + 1, thumbnailRect.bottom + 1), getBlack());
	}

	int saveSlot = listBox->getValue() < ARRAYSIZE(slotIds) ? slotIds[listBox->getValue()] : -1;
	updateThumbnail(window, thumbnailRect, saveSlot);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonSave || event.widget == listBox) {
					saveSlotToHandle =
						listBox->getValue() < ARRAYSIZE(slotIds) ? slotIds[listBox->getValue()] : -1;
					delete window;
					return true;
				} else if (event.widget == buttonCancel) {
					delete window;
					return false;
				}

				break;

			case kDialogValueChange:
				if (event.widget == listBox) {
					saveSlot = listBox->getValue() < ARRAYSIZE(slotIds) ? slotIds[listBox->getValue()] : -1;
					updateThumbnail(window, thumbnailRect, saveSlot);
				}
				break;

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	// When quitting, do not load the saved game
	delete window;
	return false;
}

bool MacV6Gui::runSaveDialog(int &saveSlotToHandle, Common::String &saveName) {
	// Widgets:
	//
	// 0 - Save button
	// 1 - Cancel button
	// 2 - Unknown item type 129
	// 3 - User item
	// 4 - Eject button
	// 5 - Desktop button
	// 6 - User item
	// 7 - User item
	// 8 - Picture
	// 9 - "Save as:" text
	// 10 - User item

	int dialogId = (_vm->_game.id == GID_MANIAC) ? 386 : 258;

	MacDialogWindow *window = createDialog(dialogId);

	MacButton *buttonSave = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacButton *buttonEject = (MacButton *)window->getWidget(kWidgetButton, 2);
	MacButton *buttonDrive = (MacButton *)window->getWidget(kWidgetButton, 3);
	MacEditText *editText = (MacEditText *)window->getWidget(kWidgetEditText);

	window->setDefaultWidget(buttonSave);
	buttonEject->setEnabled(false);
	buttonDrive->setEnabled(false);

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	drawFakePathList(window, Common::Rect(14, 8, 232, 27), _gameName.c_str());
	drawFakeDriveLabel(window, Common::Rect(242, 10, 339, 26), "ScummVM");

	int firstAvailableSlot = -1;
	for (int i = 0; i < ARRAYSIZE(busySlots); i++) {
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	window->addListBox(Common::Rect(14, 31, 232, 129), savegameNames, true, true);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonSave) {
					saveName = editText->getText();
					saveSlotToHandle = firstAvailableSlot;
					delete window;
					return true;
				} else if (event.widget == buttonCancel) {
					delete window;
					return false;
				}

				break;

			case kDialogValueChange:
				if (event.widget == editText) {
					buttonSave->setEnabled(!editText->getText().empty());
				}
				break;

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	// When quitting, do not save the game
	delete window;
	return false;
}

void MacV6Gui::setVolume(int type, int volume) {
	const char *keys[] = {
		"music_volume",
		"sfx_volume",
		"speech_volume"
	};

	const Audio::Mixer::SoundType soundTypes[] = {
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType,
		Audio::Mixer::kSpeechSoundType
	};

	// Input volume comes from the options dialog slider. It's 0-16.
	// The regular mixer uses 0-256 while Digital iMUSE uses 0-127.

	int mixerVolume = CLIP(16 * volume, 0, 256);

	if (_vm->_game.version >= 7) {
#ifdef ENABLE_SCUMM_7_8
		int dimuseVolume = CLIP(8 * volume, 0, 127);

		switch (type) {
		case 0:
			_vm->_imuseDigital->diMUSESetMusicGroupVol(dimuseVolume);
			break;
		case 1:
			_vm->_imuseDigital->diMUSESetSFXGroupVol(dimuseVolume);
			break;
		case 2:
			_vm->_imuseDigital->diMUSESetVoiceGroupVol(dimuseVolume);
			break;
		}
#endif
	} else {
		_vm->_mixer->setVolumeForSoundType(soundTypes[type], mixerVolume);
	}

	ConfMan.setInt(keys[type], mixerVolume);
}

bool MacV6Gui::runOptionsDialog() {
	// There are too many different variations to list all widgets here.
	// The important thing that they share are that the first three buttons
	// are OK, Cancel, and Defaults, and that with the exception of Maniac
	// Mansion they expect the first text to contain the name of the game
	// as "^3".

	int dialogId = (_vm->_game.id == GID_MANIAC) ? 385 : 257;

	MacDialogWindow *window = createDialog(dialogId);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);
	MacButton *buttonDefaults = (MacButton *)window->getWidget(kWidgetButton, 2);

	MacImageSlider *sliderMusicVolume = nullptr;
	MacImageSlider *sliderEffectVolume = nullptr;
	MacImageSlider *sliderVoiceVolume = nullptr;
	MacImageSlider *sliderTextSpeed = nullptr;

#ifdef ENABLE_SCUMM_7_8
	MacCheckbox *checkboxSpoolMusic = nullptr;
#endif

	MacPopUpMenu *popUpInteraction = nullptr;
	MacPopUpMenu *popUpVideoQuality = nullptr;

	if (_vm->_game.id != GID_MANIAC) {
		window->addSubstitution("");
		window->addSubstitution("");
		window->addSubstitution("");
		window->addSubstitution(_gameName);

		popUpInteraction = (MacPopUpMenu *)window->getWidget(kWidgetPopUpMenu, 0);
		popUpVideoQuality = (MacPopUpMenu *)window->getWidget(kWidgetPopUpMenu, 1);
	} else {
		popUpVideoQuality = (MacPopUpMenu *)window->getWidget(kWidgetPopUpMenu, 0);
	}

	// Note: The video quality menu contains an additional "Graphics
	// Smoothing" entry. I don't know why it doesn't show up in the
	// original, but as long as we disabled the pop-up that's not a
	// problem. My future self can thank me later.

	popUpVideoQuality->setValue(1);
	popUpVideoQuality->setEnabled(false);

	window->setDefaultWidget(buttonOk);

	if (_vm->_game.id == GID_TENTACLE) {
		// Yes, the frames really are supposed to be slightly
		// misaligned to match the original appearance.

		drawDottedFrame(window, Common::Rect(12, 41, 337, 113), 21, 137);
		drawDottedFrame(window, Common::Rect(11, 130, 336, 203), 20, 168);

		sliderMusicVolume = addSlider(window, 152, 63, 147, 0, 16);
		sliderVoiceVolume = addSlider(window, 152, 87, 147, 0, 16);
		sliderTextSpeed = addSlider(window, 151, 177, 147, 1, 9);
	} else if (_vm->_game.id == GID_MANIAC) {
		sliderMusicVolume = addSlider(window, 152, 41, 147, 0, 16);
		sliderTextSpeed = addSlider(window, 152, 72, 147, 0, 9, 5);
	} else if (_vm->_game.id == GID_SAMNMAX || _vm->_game.id == GID_DIG) {
		drawDottedFrame(window, Common::Rect(12, 41, 337, 136), 21, 137);
		drawDottedFrame(window, Common::Rect(12, 156, 337, 229), 20, 168);

		sliderMusicVolume = addSlider(window, 152, 63, 147, 0, 16);
		sliderEffectVolume = addSlider(window, 152, 87, 147, 0, 16);
		sliderVoiceVolume = addSlider(window, 152, 111, 147, 0, 16);
		sliderTextSpeed = addSlider(window, 152, 203, 147, 1, 9);
#ifdef ENABLE_SCUMM_7_8
	} else if (_vm->_game.id == GID_FT) {
		drawDottedFrame(window, Common::Rect(12, 41, 337, 164), 21, 137);
		drawDottedFrame(window, Common::Rect(12, 184, 337, 257), 20, 168);

		sliderMusicVolume = addSlider(window, 152, 63, 147, 0, 16);
		sliderEffectVolume = addSlider(window, 152, 87, 147, 0, 16);
		sliderVoiceVolume = addSlider(window, 152, 111, 147, 0, 16);
		sliderTextSpeed = addSlider(window, 152, 231, 147, 1, 9);

		checkboxSpoolMusic = (MacCheckbox *)window->getWidget(kWidgetCheckbox, 0);
#endif
	} else
		error("MacV6Gui::runOptionsDialog: Unknown game");

	if (popUpInteraction) {
		switch (_vm->_voiceMode) {
		case 0: // Voice Only
			popUpInteraction->setValue(1);
			break;
		case 1: // Voice And Text
			popUpInteraction->setValue(2);
			break;
		case 2: // Text Only
			popUpInteraction->setValue(0);
			break;
		default:
			warning("MacGuiImpl::MacV6Gui::runOptionsDialog(): Invalid voice mode %d", _vm->_voiceMode);
			break;
		}
	}

	int musicVolume = 0;
	int effectVolume = 0;
	int voiceVolume = 0;

	// There is always a music volume slider. The rest are not guaranteed.
	// If there is a voice volume slider but no effect volume slider, the
	// voice volume slider cover both.

	musicVolume = ConfMan.getInt("music_volume") / 16;
	sliderMusicVolume->setValue(musicVolume);

	if (sliderEffectVolume) {
		effectVolume = ConfMan.getInt("sfx_volume") / 16;
		sliderEffectVolume->setValue(effectVolume);
	}

	if (sliderVoiceVolume) {
		voiceVolume = ConfMan.getInt("speech_volume") / 16;
		sliderVoiceVolume->setValue(voiceVolume);
	}

#ifdef ENABLE_SCUMM_7_8
	if (checkboxSpoolMusic)
		checkboxSpoolMusic->setValue(_vm->_spooledMusicIsToBeEnabled);
#endif

	if (sliderTextSpeed)
		sliderTextSpeed->setValue(_vm->_defaultTextSpeed);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonOk) {
					musicVolume = sliderMusicVolume->getValue();

					if (_vm->_game.id == GID_MANIAC) {
						effectVolume = musicVolume;
					} else if (_vm->_game.id == GID_TENTACLE) {
						musicVolume = sliderMusicVolume->getValue();
						voiceVolume = sliderVoiceVolume->getValue();
						effectVolume = voiceVolume;
					} else {
						musicVolume = sliderMusicVolume->getValue();
						effectVolume = sliderEffectVolume->getValue();
						voiceVolume = sliderVoiceVolume->getValue();
					}

					setVolume(0, musicVolume);
					setVolume(1, effectVolume);

					if (sliderVoiceVolume) {
						voiceVolume = sliderVoiceVolume->getValue();
						setVolume(2, voiceVolume);
					}

					// The DOS version uses 0-9. Apparently the Mac version
					// uses 1-9 instead, except for Maniac Mansion.

					_vm->_defaultTextSpeed = sliderTextSpeed->getValue();
					ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
					_vm->setTalkSpeed(_vm->_defaultTextSpeed);

#ifdef ENABLE_SCUMM_7_8
					if (checkboxSpoolMusic) {
						bool spoolMusic = (checkboxSpoolMusic->getValue() != 0);

						_vm->_spooledMusicIsToBeEnabled = spoolMusic;

						if (_vm->_imuseDigital) {
							if (spoolMusic) {
								_vm->_imuseDigital->diMUSEEnableSpooledMusic();
							} else {
								_vm->_imuseDigital->diMUSEDisableSpooledMusic();
							}
						}
					}
#endif

					if (popUpInteraction) {
						switch (popUpInteraction->getValue()) {
						case 0: // Text Only
							ConfMan.setBool("subtitles", true);
							ConfMan.setBool("speech_mute", true);
							break;
						case 1: // Voice Only
							ConfMan.setBool("subtitles", false);
							ConfMan.setBool("speech_mute", false);
							break;
						case 2: // Text & Voice
							ConfMan.setBool("subtitles", true);
							ConfMan.setBool("speech_mute", false);
							break;
						}
					}

					ConfMan.flushToDisk();
					_vm->syncSoundSettings();

					delete window;
					return true;
				} else if (event.widget == buttonCancel) {
					delete window;
					return false;
				} else if (event.widget == buttonDefaults) {
					if (sliderMusicVolume)
						sliderMusicVolume->setValue(16);
					if (sliderEffectVolume)
						sliderEffectVolume->setValue(12);
					if (sliderVoiceVolume)
						sliderVoiceVolume->setValue(16);
					if (popUpInteraction)
						popUpInteraction->setValue(1);
					if (sliderTextSpeed)
						sliderTextSpeed->setValue((_vm->_game.id == GID_MANIAC) ? 4 : 5);
#ifdef ENABLE_SCUMM_7_8
					if (checkboxSpoolMusic)
						checkboxSpoolMusic->setValue(1);
#endif
				}

				break;

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	delete window;
	return false;
}

bool MacV6Gui::runQuitDialog() {
	int dialogId = (_vm->_game.id == GID_MANIAC) ? 192 : 128;
	MacDialogWindow *window = createDialog(dialogId);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);

	window->setDefaultWidget(buttonOk);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonOk) {
					delete window;
					return true;
				} else if (event.widget == buttonCancel) {
					delete window;
					return false;
				}

				break;

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	// When quitting, quit
	delete window;
	return true;
}

bool MacV6Gui::runRestartDialog() {
	int dialogId = (_vm->_game.id == GID_MANIAC) ? 193 : 137;
	MacDialogWindow *window = createDialog(dialogId);

	// If there is no dialog, just restart without asking.
	if (!window)
		return true;

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);

	window->setDefaultWidget(buttonOk);

	window->addSubstitution("");
	window->addSubstitution("");
	window->addSubstitution("");
	window->addSubstitution(_gameName);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonOk) {
					delete window;
					return true;
				} else if (event.widget == buttonCancel) {
					delete window;
					return false;
				}

				break;

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	// When quitting, do not restart
	delete window;
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

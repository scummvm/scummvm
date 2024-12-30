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
#include "common/enc-internal.h"
#include "common/macresman.h"
#include "common/str.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macwindowmanager.h"

#include "image/cicn.h"
#include "image/pict.h"

#include "scumm/macgui/macgui_impl.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v4.h"

namespace Scumm {

static void activateMenuCallback(void *ref) {
	MacGuiImpl *gui = (MacGuiImpl *)ref;

	gui->onMenuOpen();
}

// ===========================================================================
// Base class for Macintosh game user interface. Handles menus, fonts, image
// loading, etc.
// ===========================================================================

MacGuiImpl::MacGuiImpl(ScummEngine *vm, const Common::Path &resourceFile) : _vm(vm), _system(_vm->_system), _surface(_vm->_macScreen), _resourceFile(resourceFile) {
	_fonts.clear();
	_strsStrings.clear();

	// kMacRomanConversionTable is a conversion table from Mac Roman
	// 128-255 to unicode. What we need, however, is a mapping from
	// unicode 160-255 to Mac Roman.

	for (int i = 0; i < ARRAYSIZE(_unicodeToMacRoman); i++)
		_unicodeToMacRoman[i] = 0;

	for (int i = 0; i < ARRAYSIZE(Common::kMacRomanConversionTable); i++) {
		int unicode = Common::kMacRomanConversionTable[i];

		if (unicode >= 160 && unicode <= 255)
			_unicodeToMacRoman[unicode - 160] = 128 + i;
	}
}

MacGuiImpl::~MacGuiImpl() {
	delete _bannerWindow;
	delete _windowManager;
}

uint32 MacGuiImpl::getBlack() const {
	return _windowManager->_colorBlack;
}

uint32 MacGuiImpl::getWhite() const {
	return _windowManager->_colorWhite;
}

int MacGuiImpl::toMacRoman(int unicode) const {
	if (unicode >= 32 && unicode <= 127)
		return unicode;

	if (unicode < 160 || unicode > 255)
		return 0;

	int macRoman = _unicodeToMacRoman[unicode - 160];

	// These characters are defined in Mac Roman, but apparently not
	// present in older fonts like Chicago?

	if (macRoman >= 0xD9 && macRoman != 0xF0)
		macRoman = 0;

	return macRoman;
}

void MacGuiImpl::setPaletteDirty() {
	_paletteDirty = true;
}

void MacGuiImpl::updatePalette() {
	if (_paletteDirty && !_suspendPaletteUpdates) {
		_paletteDirty = false;
		_windowManager->passPalette(_vm->_currentPalette, getNumColors());
	}
}

bool MacGuiImpl::handleEvent(Common::Event event) {
	// The situation we're trying to avoid here is the user opening e.g.
	// the save dialog using keyboard shortcuts while the game is paused.

	if (_bannerWindow || _vm->_messageBannerActive)
		return false;

	return _windowManager->processEvent(event);
}

MacGuiImpl::DelayStatus MacGuiImpl::delay(uint32 ms) {
	uint32 to;

	to = _system->getMillis() + ms;

	while (ms == 0 || _system->getMillis() < to) {
		Common::Event event;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				return kDelayAborted;

			case Common::EVENT_LBUTTONDOWN:
				return kDelayInterrupted;

			default:
				break;
			}
		}

		uint32 delta = to - _system->getMillis();

		if (delta > 0) {
			_system->delayMillis(MIN<uint32>(delta, 10));
			_system->updateScreen();
		}
	}

	return kDelayDone;
}

// --------------------------------------------------------------------------
// Menu handling
//
// In the original, the menu was activated by pressing the "Command key".
// This does not seem very friendly to touch devices, so we instead use the
// "mouse over" method in the Mac Window Manager class.
//
// Later games added clicking to open the menu, and even an option to keep the
// menu bar always visible.
//
// TODO: Ideally we should perhaps implement some of these behaviors, but I'm
//       not sure which one can sensibly coexist.
// --------------------------------------------------------------------------

void MacGuiImpl::menuCallback(int id, Common::String &name, void *data) {
	MacGuiImpl *gui = (MacGuiImpl *)data;

	gui->handleMenu(id, name);

	if (gui->_forceMenuClosed) {
		gui->_windowManager->getMenu()->closeMenu();
		gui->_forceMenuClosed = false;
	}
}

bool MacGuiImpl::initialize() {
	if (!readStrings()) {
		// FIXME: THIS IS A TEMPORARY WORKAROUND
		// The Mac GUI initialization must never fail for Loom or Last Crusade,
		// because it's used for more than just the Mac menus and dialogs. So
		// for those games we just silently disable the original GUI, and let
		// the game start anyway.
		if (_vm->_game.id == GID_LOOM || _vm->_game.id == GID_INDY3)
			_vm->_useOriginalGUI = false;
		else
			return false;
	}

	uint32 menuMode = Graphics::kWMModeNoDesktop | Graphics::kWMModeAutohideMenu |
		Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride | Graphics::kWMModeForceMacFonts;

	// Allow a more modern UX: the menu doesn't close if the mouse accidentally goes outside the menu area
	if (_vm->enhancementEnabled(kEnhUIUX))
		menuMode |= Graphics::kWMModeWin95 | Graphics::kWMModeForceMacFontsInWin95 | Graphics::kWMModeForceMacBorder;

	_windowManager = new Graphics::MacWindowManager(menuMode);
	_windowManager->setEngine(_vm);
	_windowManager->setScreen(640, _vm->_useMacScreenCorrectHeight ? 480 : 400);
	_windowManager->setEngineActivateMenuCallback(this, activateMenuCallback);

	if (_vm->isUsingOriginalGUI()) {
		_windowManager->setMenuHotzone(Common::Rect(640, 23));
		_windowManager->setMenuDelay(250);

		Common::MacResManager resource;
		Graphics::MacMenu *menu = _windowManager->addMenu();

		resource.open(_resourceFile);

		// Add the Apple menu

		const Graphics::MacMenuData menuSubItems[] = {
			{ 0, nullptr, 0, 0, false }
		};

		menu->setCommandsCallback(menuCallback, this);

		// Newer games define their menus through an MBAR resource

		Common::SeekableReadStream *mbar = resource.getResource(MKTAG('M', 'B', 'A', 'R'), 128);

		if (mbar) {
			uint16 numMenus = mbar->readUint16BE();
			for (uint i = 0; i < numMenus; i++) {
				uint16 menuId = mbar->readUint16BE();
				addMenu(menu, menuId);
			}
			delete mbar;
		} else {
			Common::String aboutMenuDef = _strsStrings[kMSIAboutGameName].c_str();
			int maxMenu = -1;
			switch (_vm->_game.id) {
			case GID_INDY3:
			case GID_LOOM:
				maxMenu = 130;
				break;
			case GID_MONKEY:
				maxMenu = 131;
				break;
			default:
				maxMenu = 132;
			}

			if (_vm->_game.id == GID_LOOM) {
				aboutMenuDef += ";";

				if (!_vm->enhancementEnabled(kEnhUIUX))
					aboutMenuDef += "(";

				aboutMenuDef += "Drafts Inventory";
			}

			menu->addStaticMenus(menuSubItems);
			menu->createSubMenuFromString(0, aboutMenuDef.c_str(), 0);

			for (int i = 129; i <= maxMenu; i++) {
				addMenu(menu, i);
			}

			resource.close();
		}

		// Assign sensible IDs to the menu items

		int numberOfMenus = menu->numberOfMenus();

		for (int i = 0; i < numberOfMenus; i++) {
			Graphics::MacMenuItem *item = menu->getMenuItem(i);
			int numberOfMenuItems = menu->numberOfMenuItems(item);
			int id = 100 * (i + 1);
			for (int j = 0; j < numberOfMenuItems; j++) {
				Graphics::MacMenuItem *subItem = menu->getSubMenuItem(item, j);
				Common::String str = menu->getName(subItem);

				if (!str.empty()) {
					menu->setAction(subItem, id++);
				}

				if (str.contains("^3")) {
					Common::replace(str, "^3", name());
					menu->setName(subItem, str);
				}
			}
		}
	}

	// Register custom fonts. The font family just happens to match the
	// printed name of the game.

	const Common::String fontFamily = name();

	const Common::Array<Graphics::MacFontFamily *> &fontFamilies = _windowManager->_fontMan->getFontFamilies();

	_windowManager->_fontMan->loadFonts(_resourceFile);

	for (uint i = 0; i < fontFamilies.size(); i++) {
		if (fontFamilies[i]->getName() == fontFamily) {
			_gameFontId = fontFamilies[i]->getFontFamilyId();
			break;
		}
	}

	return true;
}

void MacGuiImpl::addMenu(Graphics::MacMenu *menu, int menuId) {
	Common::MacResManager resource;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('M', 'E', 'N', 'U'), menuId);

	if (!res) {
		resource.close();
		return;
	}

	Common::StringArray *menuDef = Graphics::MacMenu::readMenuFromResource(res);
	Common::String name = menuDef->operator[](0);
	Common::String string = menuDef->operator[](1);
	int id = menu->addMenuItem(nullptr, name);

	// The CD version of Fate of Atlantis has a menu item for toggling graphics
	// smoothing. We retroactively add that to the remaining V5 games, but not
	// to Loom and Last Crusade.

	if (_vm->enhancementEnabled(kEnhUIUX)) {
		if ((_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) && id == 3) {
			string += ";(-;Smooth Graphics";
		}

		// Floppy version
		if (_vm->_game.id == GID_INDY4 && !string.contains("Smooth Graphics") && id == 3) {
			string += ";(-;Smooth Graphics";
		}
	}

	menu->createSubMenuFromString(id, string.c_str(), 0);

	delete menuDef;
	delete res;

	resource.close();
}

bool MacGuiImpl::handleMenu(int id, Common::String &name) {
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

	switch (id) {
	case 100:	// About
		runAboutDialog();
		return true;

	case 200:	// Open
		if (runOpenDialog(saveSlotToHandle)) {
			if (saveSlotToHandle > -1) {
				_vm->loadGameState(saveSlotToHandle);
				if (_vm->_game.id == GID_INDY3)
					((ScummEngine_v4 *)_vm)->updateIQPoints();
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

	case 202:	// Restart
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 203:	// Pause
		if (!_vm->_messageBannerActive) {
			menu->closeMenu();

			if (_vm->_game.version == 3)
				_vm->mac_showOldStyleBannerAndPause(_vm->getGUIString(gsPause), -1);
			else
				_vm->showBannerAndPause(0, -1, _vm->getGUIString(gsPause));
		}
		return true;

	// In the original, the Edit menu is active during save dialogs, though
	// only Cut, Copy and Paste.

	case 300:	// Undo
	case 301:	// Cut
	case 302:	// Copy
	case 303:	// Paste
	case 304:	// Clear
		return true;
	}

	return false;
}

void MacGuiImpl::updateWindowManager() {
	Graphics::MacMenu *menu = _windowManager->getMenu();

	if (!menu)
		return;

	// We want the arrow cursor for menus. Note that the menu triggers even
	// when the mouse is invisible, which may or may not be a bug. But the
	// original did allow you to open the menu with Alt even when the
	// cursor was visible, so for now it's a feature.

	bool saveCondition = true;
	bool loadCondition = true;

	if (_vm->_game.id == GID_INDY3) {
		// Taken from Mac disasm...
		// The VAR(94) part tells us whether the copy protection has
		// failed or not, while the VAR(58) part uses bitmasks to enable
		// or disable saving and loading during normal gameplay.
		saveCondition = (_vm->VAR(58) & 0x01) && !(_vm->VAR(94) & 0x10);
		loadCondition = (_vm->VAR(58) & 0x02) && !(_vm->VAR(94) & 0x10);
	} else if (_vm->_game.id == GID_LOOM) {
		// TODO: Complete LOOM with the rest of the proper code from disasm,
		// for now we only have the copy protection code and a best guess in place...
		//
		// Details:
		// VAR(221) & 0x4000:           Copy protection bit (the only thing I could confirm from the disasm)
		// VAR(VAR_VERB_SCRIPT) == 5:   Best guess... it prevents saving/loading from e.g. difficulty selection screen
		// _userPut > 0:                Best guess... it prevents saving/loading during cutscenes

		saveCondition = loadCondition =
			!(_vm->VAR(221) & 0x4000) &&
			(_vm->VAR(_vm->VAR_VERB_SCRIPT) == 5) &&
			(_vm->_userPut > 0);
	} else {
		saveCondition = true;
		loadCondition = true;
	}

	bool canLoad = _vm->canLoadGameStateCurrently() && loadCondition;
	bool canSave = _vm->canSaveGameStateCurrently() && saveCondition;

	Graphics::MacMenuItem *gameMenu = menu->getMenuItem(1);
	Graphics::MacMenuItem *loadMenu = menu->getSubMenuItem(gameMenu, 0);
	Graphics::MacMenuItem *saveMenu = menu->getSubMenuItem(gameMenu, 1);

	if (loadMenu)
		loadMenu->enabled = canLoad;

	if (saveMenu)
		saveMenu->enabled = canSave;

	if (!_windowManager->isMenuActive() && _menuIsActive)
		onMenuClose();

	if (_vm->_game.version > 3 && _vm->_game.version < 6) {
		Graphics::MacMenuItem *windowMenu = menu->getMenuItem("Window");
		Graphics::MacMenuItem *hideDesktopMenu = menu->getSubMenuItem(windowMenu, 0);
		Graphics::MacMenuItem *hideBarMenu = menu->getSubMenuItem(windowMenu, 1);

		hideDesktopMenu->enabled = false;
		hideBarMenu->enabled = false;

		// "Fix color map"
		menu->getSubMenuItem(gameMenu, 5)->enabled = false;

		// Window mode
		menu->getSubMenuItem(windowMenu, 3)->enabled = false;
		menu->getSubMenuItem(windowMenu, 4)->enabled = false;
		menu->getSubMenuItem(windowMenu, 5)->enabled = false;

		if (menu->numberOfMenuItems(windowMenu) >= 8)
			menu->getSubMenuItem(windowMenu, 7)->checked = _vm->_useMacGraphicsSmoothing;

		Graphics::MacMenuItem *speechMenu = menu->getMenuItem("Speech");

		if (speechMenu) {
			menu->getSubMenuItem(speechMenu, 0)->checked = false; // Voice Only
			menu->getSubMenuItem(speechMenu, 1)->checked = false; // Text Only
			menu->getSubMenuItem(speechMenu, 2)->checked = false; // Voice and Text

			switch (_vm->_voiceMode) {
			case 0: // Voice Only
				menu->getSubMenuItem(speechMenu, 0)->checked = true;
				break;
			case 1: // Voice and Text
				menu->getSubMenuItem(speechMenu, 2)->checked = true;
				break;
			case 2: // Text Only
				menu->getSubMenuItem(speechMenu, 1)->checked = true;
				break;
			default:
				warning("MacGuiImpl::updateWindowManager(): Invalid voice mode %d", _vm->_voiceMode);
				break;
			}
		}
	} else if (_vm->_game.version >= 6) {
		// We can't use the name of the menus here, because there are
		// non-English versions. Let's hope the menu positions are
		// always the same, at least!

		Graphics::MacMenuItem *videoMenu = menu->getMenuItem(3);

		menu->getSubMenuItem(videoMenu, 0)->enabled = false;
		menu->getSubMenuItem(videoMenu, 1)->enabled = false;
		menu->getSubMenuItem(videoMenu, 2)->checked = true;
		menu->getSubMenuItem(videoMenu, 3)->checked = _vm->_useMacGraphicsSmoothing;

		Graphics::MacMenuItem *soundMenu = menu->getMenuItem(4);

		menu->getSubMenuItem(soundMenu, 0)->checked = (_vm->_soundEnabled & 2); // Music
		menu->getSubMenuItem(soundMenu, 1)->checked = (_vm->_soundEnabled & 1); // Effects
		menu->getSubMenuItem(soundMenu, 5)->checked = false; // Text Only
		menu->getSubMenuItem(soundMenu, 6)->checked = false; // Voice Only
		menu->getSubMenuItem(soundMenu, 7)->checked = false; // Text & Voice

		switch (_vm->_voiceMode) {
		case 0:	// Voice Only
			menu->getSubMenuItem(soundMenu, 6)->checked = true;
			break;
		case 1: // Voice and Text
			menu->getSubMenuItem(soundMenu, 7)->checked = true;
			break;
		case 2:	// Text Only
			menu->getSubMenuItem(soundMenu, 5)->checked = true;
			break;
		default:
			warning("MacGuiImpl::updateWindowManager(): Invalid voice mode %d", _vm->_voiceMode);
			break;
		}
	}

	if (menu->isVisible())
		updatePalette();

	_windowManager->draw();
}

void MacGuiImpl::onMenuOpen() {
	if (!_menuIsActive) {
		_menuIsActive = true;
		_cursorWasVisible = CursorMan.showMouse(true);
		_windowManager->pushCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	}
}

void MacGuiImpl::onMenuClose() {
	if (_menuIsActive) {
		_menuIsActive = false;
		if (_windowManager->getCursorType() == Graphics::MacGUIConstants::kMacCursorArrow)
			_windowManager->popCursor();
		CursorMan.showMouse(_cursorWasVisible);
	}
}

// ---------------------------------------------------------------------------
// Font handling
// ---------------------------------------------------------------------------

const Graphics::Font *MacGuiImpl::getFont(FontId fontId) {
	const Graphics::Font *font = _fonts.getValOrDefault((int)fontId);

	if (font)
		return font;

	int id;
	int size;
	int slant;

	switch (fontId) {
	case kSystemFont:
		id = Graphics::kMacFontSystem;
		size = 12;
		slant = Graphics::kMacFontRegular;
		break;

	default:
		getFontParams(fontId, id, size, slant);
		break;
	}


	font = _windowManager->_fontMan->getFont(Graphics::MacFont(id, size, slant));
	_fonts[(int)fontId] = font;

	return font;
}

bool MacGuiImpl::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	switch (fontId) {
	case kAboutFontHeaderOutside:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontItalic | Graphics::kMacFontBold | Graphics::kMacFontOutline;
		return true;

	case kAboutFontHeaderInside:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontItalic | Graphics::kMacFontBold | Graphics::kMacFontExtend;
		return true;

	case kAboutFontRegular:
		id = Graphics::kMacFontApplication;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kAboutFontBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kAboutFontExtraBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontBold;
		return true;

	default:
		return false;
	}
}

Graphics::Surface *MacGuiImpl::createRemappedSurface(const Graphics::Surface *surface, const byte *palette, int colorCount) {
	Graphics::Surface *s = new Graphics::Surface();
	s->create(surface->w, surface->h, Graphics::PixelFormat::createFormatCLUT8());

	byte paletteMap[256];
	memset(paletteMap, 0, sizeof(paletteMap));

	const byte monoPalette[] = {
		0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00
	};

	if (colorCount == 0) {
		colorCount = 2;
		palette = monoPalette;
	}

	for (int i = 0; i < colorCount; i++) {
		int r = palette[3 * i];
		int g = palette[3 * i + 1];
		int b = palette[3 * i + 2];

		uint32 c;

		c = _windowManager->findBestColor(r, g, b);
		paletteMap[i] = c;
	}

	// Colors outside the palette are not remapped.

	for (int i = colorCount; i < 256; i++)
		paletteMap[i] = i;

	if (palette) {
		for (int y = 0; y < s->h; y++) {
			for (int x = 0; x < s->w; x++) {
				int color = surface->getPixel(x, y);
				if (color > colorCount)
					color = getBlack();
				else
					color = paletteMap[color];

				s->setPixel(x, y, color);
			}
		}
	} else {
		s->copyFrom(*surface);
	}

	return s;
}

// ---------------------------------------------------------------------------
// Icon loader
// ---------------------------------------------------------------------------

bool MacGuiImpl::loadIcon(int id, Graphics::Surface **icon, Graphics::Surface **mask) {
	bool result = false;
	Common::MacResManager resource;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('c', 'i', 'c', 'n'), id);

	Image::CicnDecoder iconDecoder;

	*icon = nullptr;
	*mask = nullptr;

	if (res && iconDecoder.loadStream(*res)) {
		result = true;
		const Graphics::Surface *s1 = iconDecoder.getSurface();
		const Graphics::Surface *s2 = iconDecoder.getMask();
		const byte *palette = iconDecoder.getPalette();

		*icon = createRemappedSurface(s1, palette, iconDecoder.getPaletteColorCount());
		*mask = new Graphics::Surface();
		(*mask)->copyFrom(*s2);
	}

	delete res;
	resource.close();

	return result;
}

// ---------------------------------------------------------------------------
// PICT loader
// ---------------------------------------------------------------------------

Graphics::Surface *MacGuiImpl::loadPict(int id) {
	Common::MacResManager resource;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('P', 'I', 'C', 'T'), id);

	Image::PICTDecoder pictDecoder;
	Graphics::Surface *s = nullptr;

	if (res && pictDecoder.loadStream(*res)) {
		const Graphics::Surface *surface = pictDecoder.getSurface();
		const byte *palette = pictDecoder.getPalette();

		s = createRemappedSurface(surface, palette, pictDecoder.getPaletteColorCount());
	}

	delete res;
	resource.close();

	return s;
}

// ---------------------------------------------------------------------------
// Window handling
// ---------------------------------------------------------------------------

// In the older Mac games, the GUI and game graphics are made to coexist, so
// the GUI should look good regardless of what's on screen. In the newer ones
// there is no such guarantee, so the screen is blacked out whenever the GUI
// is shown.
//
// We hard-code the upper part of the palette to have all the colors used by
// the Mac Window manager and window borders. This is so that drawing the
// window will not mess up what's already on screen. The lower part is used for
// any graphical elements (pictures and icons) in the window. Anything in
// between is set to black.
//
// The Mac window manager gets the palette before gamma correction, the backend
// gets it afterwards.

void MacGuiImpl::setMacGuiColors(Graphics::Palette &palette) {
	// Colors used by the Mac Window Manager
	palette.set(255, 0x00, 0x00, 0x00); // Black
	palette.set(254, 0xFF, 0xFF, 0xFF); // White
	palette.set(253, 0x80, 0x80, 0x80); // Gray80
	palette.set(252, 0x88, 0x88, 0x88); // Gray88
	palette.set(251, 0xEE, 0xEE, 0xEE); // GrayEE
	palette.set(250, 0x00, 0xFF, 0x00); // Green
	palette.set(249, 0x00, 0xCF, 0x00); // Green2

	// Colors used by Mac dialog window borders
	palette.set(248, 0xCC, 0xCC, 0xFF);
	palette.set(247, 0xBB, 0xBB, 0xBB);
	palette.set(246, 0x66, 0x66, 0x99);

	for (int i = 0; i < 246; i++)
		palette.set(i, 0x00, 0x00, 0x00);
}

MacGuiImpl::MacDialogWindow *MacGuiImpl::createWindow(Common::Rect bounds, MacDialogWindowStyle windowStyle, MacDialogMenuStyle menuStyle) {
	if (_vm->_game.version < 6 && _vm->_game.id != GID_MANIAC) {
		updatePalette();
		_macBlack = _windowManager->_colorBlack;
		_macWhite = _windowManager->_colorWhite;
	}

	if (bounds.left < 0 || bounds.top < 0 || bounds.right >= 640 || bounds.bottom >= 400) {
		// This happens with the Last Crusade file dialogs.
		bounds.moveTo((640 - bounds.width()) / 2, 27);
	}

	// Adjust the dialog to the actual screen size. This is slightly
	// wasteful since we've already adjusted the dialog coordinates for
	// 640x400 pixels, but that may not be a bad thing if we want to keep
	// support for that resolution later.

	bounds.translate(0, 2 * _vm->_macScreenDrawOffset);

	return new MacDialogWindow(this, _system, _surface, bounds, windowStyle, menuStyle);
}

MacGuiImpl::MacDialogWindow *MacGuiImpl::createDialog(int dialogId) {
	Common::Rect bounds;

	// Default dialog sizes for dialogs without a DITL resource.

	if (_vm->_game.version < 6) {
		bounds.top = 0;
		bounds.left = 0;
		bounds.bottom = 86;
		bounds.right = 340;

		bounds.translate(86, 88);
	} else {
		bounds.top = 0;
		bounds.left = 0;
		bounds.bottom = 113;
		bounds.right = 267;

		bounds.translate(187, 94);
	}

	return createDialog(dialogId, bounds);
}

MacGuiImpl::MacDialogWindow *MacGuiImpl::createDialog(int dialogId, Common::Rect bounds) {
	uint32 black = getBlack();

	Common::MacResManager resource;
	Common::SeekableReadStream *res;

	Common::String gameFileResStr = _strsStrings[kMSIGameFile].c_str();

	resource.open(_resourceFile);

	res = resource.getResource(MKTAG('D', 'L', 'O', 'G'), dialogId);
	if (res) {
		bounds.top = res->readUint16BE();
		bounds.left = res->readUint16BE();
		bounds.bottom = res->readUint16BE();
		bounds.right = res->readUint16BE();

		// Grow the window to include the outer bounds
		bounds.grow(8);

		// Compensate for the original not drawing the game at the very top of
		// the screen.
		bounds.translate(0, -40);
	}

	delete res;

	_macWhite = _windowManager->_colorWhite;
	_macBlack = _windowManager->_colorBlack;

	if (_vm->_game.version >= 6 || _vm->_game.id == GID_MANIAC) {
		res = resource.getResource(MKTAG('D', 'I', 'T', 'L'), dialogId);
		if (!res)
			return nullptr;

		saveScreen();

		// Collect the palettes from all the icons and pictures in the
		// dialog, and combine them into a single palette that they
		// will all be remapped to use. This is probably not what the
		// original did, as the colors become slightly different. Maybe
		// the original just picked one of the palettes, and then used
		// the closest available colors for the rest?
		//
		// That might explain why some of them have seemingly larger
		// palettes than necessary, but why not use the exact colors
		// when we can?

		Common::HashMap<uint32, byte> paletteMap;
		int numWindowColors = 0;

		// Additional colors for hard-coded elements
		paletteMap[0xCDCDCD] = numWindowColors++;

		int numItems = res->readUint16BE() + 1;

		for (int i = 0; i < numItems; i++) {
			res->skip(12);
			int type = res->readByte();
			int len = res->readByte();

			Image::PICTDecoder pictDecoder;
			Image::CicnDecoder iconDecoder;

			const byte *palette = nullptr;
			int paletteColorCount = 0;

			Common::SeekableReadStream *imageRes = nullptr;
			Image::ImageDecoder *decoder = nullptr;

			switch (type & 0x7F) {
			case 32:
				imageRes = resource.getResource(MKTAG('c', 'i', 'c', 'n'), res->readUint16BE());
				decoder = &iconDecoder;
				break;

			case 64:
				imageRes = resource.getResource(MKTAG('P', 'I', 'C', 'T'), res->readUint16BE());
				decoder = &pictDecoder;
				break;

			default:
				res->skip(len);
				break;
			}

			if (imageRes && decoder->loadStream(*imageRes)) {
				palette = decoder->getPalette();
				paletteColorCount = decoder->getPaletteColorCount();
				for (int j = 0; j < paletteColorCount; j++) {
					uint32 color = (palette[3 * j] << 16) | (palette[3 * j + 1] << 8) | palette[3 * j + 2];
					if (!paletteMap.contains(color))
						paletteMap[color] = numWindowColors++;
				}
			}

			if (len & 1)
				res->skip(1);

			delete imageRes;
		}

		delete res;

		Graphics::Palette palette(256);
		setMacGuiColors(palette);

		for (auto &k : paletteMap) {
			int r = (k._key >> 16) & 0xFF;
			int g = (k._key >> 8) & 0xFF;
			int b = k._key & 0xFF;
			palette.set(k._value, r, g, b);
		}

		_windowManager->passPalette(palette.data(), 256);

		for (int i = 0; i < 256; i++) {
			byte r, g, b;

			palette.get(i, r, g, b);
			r = _vm->_macGammaCorrectionLookUp[r];
			g = _vm->_macGammaCorrectionLookUp[g];
			b = _vm->_macGammaCorrectionLookUp[b];
			palette.set(i, r, g, b);
		}

		_system->getPaletteManager()->setPalette(palette);
	}

	res = resource.getResource(MKTAG('D', 'I', 'T', 'L'), dialogId);
	if (!res)
		return nullptr;

	MacDialogWindow *window = createWindow(bounds);

	if (res) {
		int numItems = res->readUint16BE() + 1;

		for (int i = 0; i < numItems; i++) {
			res->skip(4);	// Placeholder for handle or procedure pointer

			Common::Rect r;

			r.top = res->readUint16BE();
			r.left = res->readUint16BE();
			r.bottom = res->readUint16BE();
			r.right = res->readUint16BE();

			// Move to appropriate position on inner surface
			r.translate(2, 2);

			int type = res->readByte();
			int len = res->readByte();

			Common::String str;
			bool enabled = ((type & 0x80) == 0);

			switch (type & 0x7F) {
			case 0:
				// User item
//				window->innerSurface()->frameRect(r, black);
				res->skip(len);
				break;
			case 4:
				// Button
				res->seek(-1, SEEK_CUR);
				str = res->readPascalString();
				window->addButton(r, str, enabled);
				break;

			case 5:
				// Checkbox
				res->seek(-1, SEEK_CUR);
				str = res->readPascalString();
				window->addCheckbox(r, str, enabled);
				break;

			case 7:
				// Control
				window->addControl(r, res->readUint16BE());
				break;

			case 8:
				// Static text
				res->seek(-1, SEEK_CUR);
				str = res->readPascalString();
				window->addStaticText(r, str, enabled);
				break;

			case 16:
			{
				// Editable text

				// Adjust for pixel accuracy...
				r.left -= 1;

				MacGuiImpl::MacEditText *editText = window->addEditText(r, gameFileResStr, enabled);
				editText->selectAll();

				window->innerSurface()->frameRect(Common::Rect(r.left - 2, r.top - 3, r.right + 3, r.bottom + 3), black);
				res->skip(len);
				break;
			}
			case 32:
				// Icon
				window->addIcon(r.left, r.top, res->readUint16BE(), enabled);
				break;

			case 64:
				// Picture
				window->addPicture(r, res->readUint16BE(), enabled);
				break;

			default:
				warning("MacGuiImpl::createDialog(): Unknown item type %d", type);
				res->skip(len);
				break;
			}

			if (len & 1)
				res->skip(1);
		}
	}

	delete res;
	resource.close();

	return window;
}

// ---------------------------------------------------------------------------
// Standard dialogs
// ---------------------------------------------------------------------------

// A standard file picker dialog doesn't really make sense in ScummVM, so we
// make something that just looks similar to one.

bool MacGuiImpl::runOpenDialog(int &saveSlotToHandle) {
	Common::Rect bounds(88, 28, 448, 210);

	MacDialogWindow *window = createWindow(bounds);

	MacButton *buttonOpen = window->addButton(Common::Rect(254, 137, 334, 157), "Open", true);
	MacButton *buttonCancel = window->addButton(Common::Rect(254, 106, 334, 126), "Cancel", true);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	window->drawDottedHLine(253, 93, 334);

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	drawFakePathList(window, Common::Rect(14, 8, 232, 27), name().c_str());
	drawFakeDriveLabel(window, Common::Rect(232, 10, 344, 28), "ScummVM");

	MacListBox *listBox = window->addListBox(Common::Rect(14, 31, 232, 161), savegameNames, true);

	window->setDefaultWidget(buttonOpen);

	while (!_vm->shouldQuit()) {
		MacDialogEvent event;

		while (window->runDialog(event)) {
			switch (event.type) {
			case kDialogClick:
				if (event.widget == buttonOpen || event.widget == listBox) {
					saveSlotToHandle =
						listBox->getValue() < ARRAYSIZE(slotIds) ?
						slotIds[listBox->getValue()] : -1;
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

	// When quitting, do not load the saved game
	delete window;
	return false;
}

bool MacGuiImpl::runSaveDialog(int &saveSlotToHandle, Common::String &saveName) {
	uint32 black = getBlack();

	Common::Rect bounds(110, 27, 470, 231);

	MacDialogWindow *window = createWindow(bounds);

	MacButton *buttonSave = window->addButton(Common::Rect(254, 163, 334, 183), "Save", true);
	MacButton *buttonCancel = window->addButton(Common::Rect(254, 132, 334, 152), "Cancel", true);
	window->addButton(Common::Rect(254, 90, 334, 110), "New", false);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	Common::String saveGameFileAsResStr = _strsStrings[kMSISaveGameFileAs].c_str();
	Common::String gameFileResStr = _strsStrings[kMSIGameFile].c_str();

	int firstAvailableSlot = -1;
	for (int i = 1; i < ARRAYSIZE(busySlots); i++) { // Skip the autosave slot
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	drawFakePathList(window, Common::Rect(14, 8, 232, 27), name().c_str());
	drawFakeDriveLabel(window, Common::Rect(232, 10, 344, 28), "ScummVM");

	window->addListBox(Common::Rect(14, 31, 232, 129), savegameNames, true, true);

	MacGuiImpl::MacEditText *editText = window->addEditText(Common::Rect(16, 159, 229, 175), gameFileResStr, true);

	Graphics::Surface *s = window->innerSurface();
	const Graphics::Font *font = getFont(kSystemFont);

	s->frameRect(Common::Rect(14, 156, 232, 178), black);

	window->drawDottedHLine(253, 121, 334);

	font->drawString(s, saveGameFileAsResStr, 14, 138, 218, black, Graphics::kTextAlignLeft, 4);

	window->setDefaultWidget(buttonSave);
	editText->selectAll();

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

			case kDialogKeyDown:
				if (event.widget == editText) {
					// Disable "Save" button when text is empty
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

void MacGuiImpl::prepareSaveLoad(Common::StringArray &savegameNames, bool *availSlots, int *slotIds, int size) {
	int saveCounter = 0;

	for (int i = 0; i < size; i++) {
		slotIds[i] = -1;
	}

	Common::String name;
	_vm->listSavegames(availSlots, size);

	for (int i = 0; i < size; i++) {
		if (availSlots[i]) {
			// Save the slot ids for slots which actually contain savegames...
			slotIds[saveCounter] = i;
			saveCounter++;
			if (_vm->getSavegameName(i, name)) {
				Common::String temp = Common::U32String(name, _vm->getDialogCodePage()).encode(Common::kMacRoman);
				savegameNames.push_back(temp);
			} else {
				// The original printed "WARNING... old savegame", but we do support old savegames :-)
				savegameNames.push_back(Common::String::format("%s", "WARNING: wrong save version"));
			}
		}
	}
}

bool MacGuiImpl::runOkCancelDialog(Common::String text) {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - "^0" text

	MacDialogWindow *window = createDialog(502);

	MacButton *buttonOk = (MacButton *)window->getWidget(kWidgetButton, 0);
	MacButton *buttonCancel = (MacButton *)window->getWidget(kWidgetButton, 1);

	window->setDefaultWidget(buttonOk);
	window->addSubstitution(text);

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

			default:
				break;
			}
		}

		window->delayAndUpdate();
	}

	// Quitting is the same as clicking Ok
	delete window;
	return true;
}

void MacGuiImpl::drawFakePathList(MacDialogWindow *window, Common::Rect r, const char *text) {
	uint32 black = getBlack();
	const Graphics::Font *font = getFont(kSystemFont);

	// Draw the text...
	int x0 = r.left + 23;
	int x1 = r.right - 23;

	font->drawString(window->innerSurface(), text, x0 + 1, r.top + 2, x1 - x0, black, Graphics::kTextAlignCenter, 0, true);

	int width = font->getStringWidth(text);

	if (width < x1 - x0) {
		int middle = (x0 + x1) / 2;
		x0 = middle - width / 2;
		x1 = middle + width / 2;
	}

	Common::Rect iconRect(16, 16);

	const uint16 folderIcon[16] = {
		0x0000,	0x0000,	0x0000,	0x0000,	0x1E00,	0x21FC,	0x2002,	0xFFE2,
		0x8012,	0x4012,	0x400A,	0x200A,	0x2006,	0x1FFE,	0x0000,	0x0000
	};

	const uint16 arrowDownIcon[16] = {
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3FF8, 0x1FF0, 0x0FE0,
		0x07C0, 0x0380, 0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};

	// Draw the icon...
	iconRect.moveTo(x0 - 19, r.top + 1);
	drawBitmap(window->innerSurface(), iconRect, folderIcon, black);

	// Draw the arrow...
	iconRect.moveTo(x1 + 4, r.top + 1);
	drawBitmap(window->innerSurface(), iconRect, arrowDownIcon, black);

	// Draw the black frame...
	window->innerSurface()->frameRect(Common::Rect(x0 - 23, r.top, x1 + 23, r.bottom - 1), black);

	// Draw the shadows...
	window->innerSurface()->hLine(x0 - 20, r.bottom - 1, x1 + 23, black);
	window->innerSurface()->vLine(x1 + 23, r.top + 3, r.bottom - 1, black);
}

void MacGuiImpl::drawFakeDriveLabel(MacDialogWindow *window, Common::Rect r, const char *text) {
	uint32 black = getBlack();
	const Graphics::Font *font = getFont(kSystemFont);

	font->drawString(window->innerSurface(), text, r.left, r.top, r.width(), black, Graphics::kTextAlignCenter, 9, true);

	const uint16 hardDriveIcon[16] = {
		0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x7FFE,	0x8001,	0x8001,
		0xA001,	0x8001,	0x7FFE,	0x0000,	0x0000,	0x0000,	0x0000, 0x0000
	};

	Common::Rect iconRect(16, 16);
	int width = font->getStringWidth(text);

	if (width < r.width())
		iconRect.moveTo((r.left + r.right - width - 20) / 2, r.top);
	else
		iconRect.moveTo(r.left, r.top);

	drawBitmap(window->innerSurface(), iconRect, hardDriveIcon, black);
}

bool MacGuiImpl::runQuitDialog() {
	return runOkCancelDialog(_strsStrings[kMSIAreYouSureYouWantToQuit].c_str());
}

bool MacGuiImpl::runRestartDialog() {
	return runOkCancelDialog(_strsStrings[kMSIAreYouSureYouWantToRestart].c_str());
}

void MacGuiImpl::drawBanner(char *message) {
	if (_bannerWindow)
		undrawBanner();

	_bannerWindow = createWindow(
		Common::Rect(70, 189, 570, 211),
		kWindowStyleRounded, kMenuStyleNone);
	const Graphics::Font *font = getFont(_vm->_game.id == GID_INDY3 ? kIndy3FontMedium : kLoomFontMedium);

	Graphics::Surface *s = _bannerWindow->innerSurface();
	font->drawString(s, (char *)message, 0, 0, s->w, getBlack(), Graphics::kTextAlignCenter);

	_bannerWindow->show();
}

void MacGuiImpl::undrawBanner() {
	if (_bannerWindow) {
		delete _bannerWindow;
		_bannerWindow = nullptr;
	}
}

void MacGuiImpl::drawBitmap(Graphics::Surface *s, Common::Rect r, const uint16 *bitmap, uint32 color) const {
	assert(r.width() <= 16);

	for (int y = 0; y < r.height(); y++) {
		uint16 bit = 0x8000;
		for (int x = 0; x < r.width(); x++) {
			if (bitmap[y] & bit)
				s->setPixel(r.left + x, r.top + y, color);
			bit >>= 1;
		}
	}
}

} // End of namespace Scumm

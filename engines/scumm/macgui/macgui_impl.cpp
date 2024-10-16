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
#include "common/enc-internal.h"
#include "common/macresman.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macwindowmanager.h"

#include "image/pict.h"

#include "scumm/macgui/macgui_impl.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v4.h"

namespace Scumm {

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

Common::String MacGuiImpl::readCString(uint8 *&data) {
	Common::String result(reinterpret_cast<const char *>(data));
	data += result.size() + 1;

	while (data[0] == '\0') {
		data++;
	}

	debug(8, "MacGuiImpl::readCString(): %s", result.c_str());
	return result;
}

Common::String MacGuiImpl::readPascalString(uint8 *&data) {
	Common::String result(reinterpret_cast<const char *>(&data[1]), (uint32)data[0]);
	data += (uint32)data[0] + 1;

	while (data[0] == '\0') {
		data++;
	}

	debug(8, "MacGuiImpl::readPascalString(): %s", result.c_str());
	return result;
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

void MacGuiImpl::setPalette(const byte *palette, uint size) {
	_windowManager->passPalette(palette, size);
}

bool MacGuiImpl::handleEvent(Common::Event event) {
	// The situation we're trying to avoid here is the user opening e.g.
	// the save dialog using keyboard shortcuts while the game is paused.

	if (_bannerWindow)
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
// TODO: Ideally we should handle both, but I don't know if there's a way for
//       them to coexist.
// --------------------------------------------------------------------------

void MacGuiImpl::menuCallback(int id, Common::String &name, void *data) {
	MacGuiImpl *gui = (MacGuiImpl *)data;

	gui->handleMenu(id, name);

	if (gui->_forceMenuClosed) {
		gui->_windowManager->getMenu()->closeMenu();
		gui->_forceMenuClosed = false;
	}
}

void MacGuiImpl::initialize() {
	uint32 menuMode = Graphics::kWMModeNoDesktop | Graphics::kWMModeAutohideMenu |
		Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride | Graphics::kWMModeForceMacFonts;

	// Allow a more modern UX: the menu doesn't close if the mouse accidentally goes outside the menu area
	if (_vm->enhancementEnabled(kEnhUIUX))
		menuMode |= Graphics::kWMModeWin95 | Graphics::kWMModeForceMacFontsInWin95 | Graphics::kWMModeForceMacBorder;

	_windowManager = new Graphics::MacWindowManager(menuMode);
	_windowManager->setEngine(_vm);
	_windowManager->setScreen(640, _vm->_useMacScreenCorrectHeight ? 480 : 400);

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

		menu->setCommandsCallback(menuCallback, this);

		for (int i = 129; i <= maxMenu; i++) {
			Common::SeekableReadStream *res = resource.getResource(MKTAG('M', 'E', 'N', 'U'), i);

			if (!res)
				continue;

			Common::StringArray *menuDef = Graphics::MacMenu::readMenuFromResource(res);
			Common::String name = menuDef->operator[](0);
			Common::String string = menuDef->operator[](1);
			int id = menu->addMenuItem(nullptr, name);

			if ((_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) && id == 3) {
				string += ";(-;Smooth Graphics/G";
			}

			// Floppy version
			if (_vm->_game.id == GID_INDY4 && !string.contains("Smooth Graphics") && id == 3) {
				string += ";(-;Smooth Graphics/G";
			}

			menu->createSubMenuFromString(id, string.c_str(), 0);

			delete menuDef;
			delete res;
		}

		resource.close();

		// Assign sensible IDs to the menu items

		int numberOfMenus = menu->numberOfMenus();

		for (int i = 0; i < numberOfMenus; i++) {
			Graphics::MacMenuItem *item = menu->getMenuItem(i);
			int numberOfMenuItems = menu->numberOfMenuItems(item);
			int id = 100 * (i + 1);
			for (int j = 0; j < numberOfMenuItems; j++) {
				Graphics::MacMenuItem *subItem = menu->getSubMenuItem(item, j);
				Common::String name = menu->getName(subItem);

				if (!name.empty())
					menu->setAction(subItem, id++);
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
		if (runSaveDialog(saveSlotToHandle, savegameName)) {
			if (saveSlotToHandle > -1) {
				_vm->saveGameState(saveSlotToHandle, savegameName);
			}
		}

		return true;

	case 202:	// Restart
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 203:	// Pause
		if (!_vm->_messageBannerActive) {
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

	// Window menu
	case 402: // Tiny
	case 403: // Medium
	case 404: // Large
		return true;

	case 405: // Graphics Smoothing
		_vm->mac_toggleSmoothing();
		return true;

	case 500: // Voice Only
		_vm->_voiceMode = 0;
		_vm->_v5VoiceMode = 0;
		_vm->syncSoundSettings();
		return true;
	case 501: // Text Only
		_vm->_voiceMode = 2;
		_vm->_v5VoiceMode = 2;
		_vm->syncSoundSettings();
		return true;
	case 502: // Voice and Text
		_vm->_voiceMode = 1;
		_vm->_v5VoiceMode = 1;
		_vm->syncSoundSettings();
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

	bool isActive = _windowManager->isMenuActive();

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

	Graphics::MacMenuItem *gameMenu = menu->getMenuItem("Game");
	Graphics::MacMenuItem *loadMenu = menu->getSubMenuItem(gameMenu, 0);
	Graphics::MacMenuItem *saveMenu = menu->getSubMenuItem(gameMenu, 1);

	if (loadMenu)
		loadMenu->enabled = canLoad;

	if (saveMenu)
		saveMenu->enabled = canSave;

	if (isActive) {
		if (!_menuIsActive) {
			_cursorWasVisible = CursorMan.showMouse(true);
			_windowManager->pushCursor(Graphics::MacGUIConstants::kMacCursorArrow);
		}
	} else {
		if (_menuIsActive) {
			if (_windowManager->getCursorType() == Graphics::MacGUIConstants::kMacCursorArrow)
				_windowManager->popCursor();
			CursorMan.showMouse(_cursorWasVisible);
		}
	}

	if (_vm->_game.version > 3) {
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
				warning("MacGuiImpl::updateWindowManager(): Invalid voice mode");
			}
		}
	}

	_menuIsActive = isActive;
	_windowManager->draw();
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
		id = Graphics::kMacFontChicago;
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

	case kAboutFontBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case kAboutFontExtraBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular | Graphics::kMacFontExtend;
		return true;

	case kAboutFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	default:
		return false;
	}
}

// ---------------------------------------------------------------------------
// PICT loader
// ---------------------------------------------------------------------------

Graphics::Surface *MacGuiImpl::loadPict(int id) {
	Common::MacResManager resource;
	Graphics::Surface *s = nullptr;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('P', 'I', 'C', 'T'), id);

	// IQ logos are PICT v2
	Image::PICTDecoder pict;
	if (pict.loadStream(*res)) {
		const Graphics::Surface *s1 = pict.getSurface();
		const byte *palette = pict.getPalette();

		s = new Graphics::Surface();
		s->create(s1->w, s1->h, Graphics::PixelFormat::createFormatCLUT8());

		byte paletteMap[256];
		memset(paletteMap, 0, ARRAYSIZE(paletteMap));

		for (int i = 0; i < pict.getPaletteColorCount(); i++) {
			int r = palette[3 * i];
			int g = palette[3 * i + 1];
			int b = palette[3 * i + 2];

			uint32 c = _windowManager->findBestColor(r, g, b);
			paletteMap[i] = c;
		}

		if (!pict.getPaletteColorCount()) {
			paletteMap[15] = getBlack();
			paletteMap[0] = getWhite();
		}

		if (palette) {
			for (int y = 0; y < s->h; y++) {
				for (int x = 0; x < s->w; x++) {
					int color = s1->getPixel(x, y);
					s->setPixel(x, y, paletteMap[color]);
				}
			}
		} else
			s->copyFrom(*s1);

	}

	delete res;
	resource.close();

	return s;
}

// ---------------------------------------------------------------------------
// Window handling
// ---------------------------------------------------------------------------

MacGuiImpl::MacDialogWindow *MacGuiImpl::createWindow(Common::Rect bounds, MacDialogWindowStyle style) {
	if (bounds.left < 0 || bounds.top < 0 || bounds.right >= 640 || bounds.bottom >= 400) {
		// This happens with the Last Crusade file dialogs.
		bounds.moveTo((640 - bounds.width()) / 2, 27);
	}

	// Adjust the dialog to the actual screen size. This is slightly
	// wasteful since we've already adjusted the dialog coordinates for
	// 640x400 pixels, but that may not be a bad thing if we want to keep
	// support for that resolution later.

	bounds.translate(0, 2 * _vm->_macScreenDrawOffset);

	return new MacDialogWindow(this, _system, _surface, bounds, style);
}

Common::String MacGuiImpl::getDialogString(Common::SeekableReadStream *res, int len) {
	Common::String str;

	for (int i = 0; i < len; i++)
		str += res->readByte();

	return str;
}

MacGuiImpl::MacDialogWindow *MacGuiImpl::createDialog(int dialogId) {
	uint32 black = getBlack();

	Common::MacResManager resource;
	Common::SeekableReadStream *res;

	Common::String saveGameFileAsResStr, gameFileResStr;
	saveGameFileAsResStr = _strsStrings[kMSISaveGameFileAs].c_str();
	gameFileResStr = _strsStrings[kMSIGameFile].c_str();

	resource.open(_resourceFile);

	Common::Rect bounds;

	bool isOpenDialog = dialogId == 4000 || dialogId == 4001;
	bool isSaveDialog = dialogId == 3998 || dialogId == 3999;

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
	} else {
		bounds.top = 0;
		bounds.left = 0;
		bounds.bottom = 86;
		bounds.right = 340;

		bounds.translate(86, 88);
	}

	delete res;

	MacDialogWindow *window = createWindow(bounds);

	res = resource.getResource(MKTAG('D', 'I', 'T', 'L'), dialogId);

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
			{
				// User item

				// Skip drive label box and listbox
				bool doNotDraw = (isOpenDialog && (i == 6 || i == 7)) || ((isOpenDialog || isSaveDialog) && i == 3);
				if (!doNotDraw) {
					window->innerSurface()->frameRect(r, black);
				} else if (_vm->_game.id == GID_INDY3 && i == 3) {
					drawFakeDriveLabel(window, Common::Rect(r.left + 9, r.top, r.right, r.bottom), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);
				}

				break;
			}
			case 4:
				// Button
				str = getDialogString(res, len);
				if ((isOpenDialog || isSaveDialog) && (i == 4 || i == 5)) // "Eject" and "Drive"
					enabled = false;

				window->addButton(r, str, enabled);
				break;

			case 5:
				// Checkbox
				str = getDialogString(res, len);
				window->addCheckbox(r, str, enabled);
				break;

			case 8:
				// Static text
				str = getDialogString(res, len);
				if (isSaveDialog && i == 2)
					str = saveGameFileAsResStr;

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

	window->setDefaultWidget(0);
	window->addSubstitution(text);

	MacStaticText *widget = (MacStaticText *)window->getWidget(2);
	widget->setWordWrap(true);

	// When quitting, the default action is to quit
	bool ret = true;

	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0)
			break;

		if (clicked == 1) {
			ret = false;
			break;
		}
	}

	delete window;
	return ret;
}

void MacGuiImpl::drawFakePathList(MacDialogWindow *window, Common::Rect r, byte *icon, const char *text, Graphics::TextAlign alignment) {
	uint32 black = getBlack();

	// Draw the text...
	window->addStaticText(Common::Rect(r.left + 22, r.top + 2, r.right - 21, r.bottom - 1), text, true, alignment);

	// Draw the icon...
	Graphics::Surface *iconSurf = new Graphics::Surface();
	iconSurf->create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
	iconSurf->setPixels(icon);
	window->drawSprite(iconSurf, r.left + 4, r.top + 1);
	delete iconSurf;

	// Draw the arrow...
	Graphics::Surface *arrowSurf = new Graphics::Surface();
	arrowSurf->create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
	arrowSurf->setPixels(_arrowDownIcon);
	window->drawSprite(arrowSurf, r.right - 19, r.top + 1);
	delete arrowSurf;

	// Draw the black frame...
	window->innerSurface()->frameRect(r, black);

	// Draw the shadows...
	window->innerSurface()->hLine(r.left + 3, r.bottom, r.right, black);
	window->innerSurface()->vLine(r.right, r.top + 3, r.bottom, black);
}

void MacGuiImpl::drawFakeDriveLabel(MacDialogWindow *window, Common::Rect r, byte *icon, const char *text, Graphics::TextAlign alignment) {
	// Draw the text...
	window->addStaticText(Common::Rect(r.left + 25, r.top, r.right, r.bottom), text, true, alignment);

	// Draw the icon...
	Graphics::Surface *surf = new Graphics::Surface();
	surf->create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
	surf->setPixels(icon);
	window->drawSprite(surf, r.left + 6, r.top);
	delete surf;
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
		kStyleRounded);
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

#define SKIP_C(x) { MacGuiImpl::kMSISkip, "C",      x }
#define SKIP_P(x) { MacGuiImpl::kMSISkip, "Pascal", x }

static MacGuiImpl::MacSTRSParsingEntry strsIndy3Table[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	SKIP_P(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "C",      1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	// SKIP_C(1),
	// SKIP_P(4),
	// SKIP_C(3),
	// SKIP_P(2),
	// SKIP_C(4),
	// SKIP_P(1),
	// SKIP_C(1),
	// SKIP_P(5),
};

static MacGuiImpl::MacSTRSParsingEntry strsLoomTable[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "C",      1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	// SKIP_C(1),
	// SKIP_P(4),
	// SKIP_C(3),
	// SKIP_P(2),
	// SKIP_C(4),
	// SKIP_P(1),
	// SKIP_C(1),
	// SKIP_P(5),
	// SKIP_C(7),
};

static MacGuiImpl::MacSTRSParsingEntry strsMI1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	// SKIP_C(75)
};

static MacGuiImpl::MacSTRSParsingEntry strsMI2Variant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString39,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString40,              "Pascal", 1 },
	// SKIP_C(95)
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4CDVariant1Table[] = {
	SKIP_C(144),
	SKIP_P(1),
	SKIP_C(2),
	SKIP_P(10),
	SKIP_C(51),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant2Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

#undef SKIP_C
#undef SKIP_P

void MacGuiImpl::readStrings() {
	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);

	if (strsLen <= 0)
		return;

	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;
	MacSTRSParsingEntry *parsingTable = nullptr;
	int parsingTableSize = 0;

	if (_vm->_game.id == GID_INDY3) {
		parsingTable = strsIndy3Table;
		parsingTableSize = ARRAYSIZE(strsIndy3Table);
	} else if (_vm->_game.id == GID_LOOM) {
		parsingTable = strsLoomTable;
		parsingTableSize = ARRAYSIZE(strsLoomTable);
	} else if (_vm->_game.id == GID_MONKEY) {
		parsingTable = strsMI1Table;
		parsingTableSize = ARRAYSIZE(strsMI1Table);
	} else if (_vm->_game.id == GID_MONKEY2) {
		parsingTable = strsMI2Variant1Table;
		parsingTableSize = ARRAYSIZE(strsMI2Variant1Table);
	} else if (_vm->_game.id == GID_INDY4) {
		switch (strsLen) {
		case 6516:
			parsingTable = strsIndy4FloppyVariant1Table;
			parsingTableSize = ARRAYSIZE(strsIndy4FloppyVariant1Table);
			break;
		case 6612:
			// V1.0 10-9-92 from the LucasArts Mac CD Game Pack II
			parsingTable = strsIndy4FloppyVariant2Table;
			parsingTableSize = ARRAYSIZE(strsIndy4FloppyVariant2Table);
			break;
		case 6836: // CD
			parsingTable = strsIndy4CDVariant1Table;
			parsingTableSize = ARRAYSIZE(strsIndy4CDVariant1Table);
			break;
		default:
			warning("MacGuiImpl::readStrings(): String parsing table not defined for this variant of the game; STRS resource %d bytes", strsLen);
			break;
		}

	} else {
		error("MacGuiImpl::readStrings(): String parsing table not defined for this game");
	}

	parseSTRSBlock(strsData, parsingTable, parsingTableSize);

	free(strsBlock);
	delete strsStream;
}

void MacGuiImpl::parseSTRSBlock(uint8 *strsData, MacSTRSParsingEntry *parsingTable, int parsingTableSize) {
	_strsStrings.clear();
	_strsStrings.reserve(128);
	for (int i = 0; i < 128; i++) {
		_strsStrings.emplace_back("");
	} 

	for (int i = 0; i < parsingTableSize; i++) {
		MacSTRSParsingEntry entry = parsingTable[i];

		if (entry.strId == kMSISkip) {
			for (int j = 0; j < entry.numStrings; j++) {
				entry.parsingMethod == "C" ? readCString(strsData) : readPascalString(strsData);
			}
		} else {
			for (int j = 0; j < entry.numStrings; j++) {
				if (entry.parsingMethod == "C") {
					_strsStrings[entry.strId] = readCString(strsData);
				} else if (entry.parsingMethod == "Pascal") {
					_strsStrings[entry.strId] = readPascalString(strsData);
				} else {
					error("MacGuiImpl::parseSTRSBlock(): invalid parsing method encountered (%s)", entry.parsingMethod.c_str());
				}
			}
		}
	}
}

} // End of namespace Scumm

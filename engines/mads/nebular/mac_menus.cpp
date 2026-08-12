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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "graphics/primitives.h"
#include "gui/saveload.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/sound_manager.h"
#include "mads/mads.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

namespace {

enum {
	kDifficultyDialog = 2500,
	kDialogButton = 4,
	kDialogRadioButton = 6,
	kDialogStaticText = 8,

	kFileMenu = 1,
	kOptionsMenu = 3,
	kWindowMenu = 4,

	kFileOpen = (1001 << 16) | 2,
	kFileSave = (1001 << 16) | 4,
	kFileSaveAs = (1001 << 16) | 5,
	kFileQuit = (1001 << 16) | 9,
	kOptionNoSound = (2001 << 16),
	kOptionEasyInterface = (2001 << 16) | 1,
	kFadeSmooth = (101 << 16),
	kFadeMedium = (101 << 16) | 1,
	kFadeFast = (101 << 16) | 2,
	kStoryNaughty = (102 << 16),
	kStoryNice = (102 << 16) | 1
};

struct DialogResourceItem {
	Common::Rect bounds;
	Common::String text;
	byte type = 0;
	bool enabled = false;
};

struct DialogResource {
	Common::Rect bounds;
	uint16 itemList = 0;
	Common::Array<DialogResourceItem> items;
};

struct MenuResourceItem {
	Common::String text;
	byte key = 0;
	byte mark = 0;
	byte style = 0;
	bool enabled = false;
};

struct MenuResource {
	uint16 id = 0;
	Common::String title;
	Common::Array<MenuResourceItem> items;
};

bool readMenuResource(Common::SeekableReadStream &stream, MenuResource &resource) {
	if (stream.size() < 15)
		return false;

	resource.id = stream.readUint16BE();
	stream.skip(8); // Width, height, resource ID, and placeholder.
	const uint32 enabledItems = stream.readUint32BE();
	resource.title = stream.readPascalString();

	for (uint itemNumber = 1; stream.pos() < stream.size(); ++itemNumber) {
		MenuResourceItem item;
		item.text = stream.readPascalString();
		if (item.text.empty())
			return !stream.err();
		if (stream.pos() + 4 > stream.size())
			return false;

		/* byte icon = */ stream.readByte();
		item.key = stream.readByte();
		item.mark = stream.readByte();
		item.style = stream.readByte();
		item.enabled = itemNumber < 32 &&
			(enabledItems & (1U << itemNumber)) != 0;
		resource.items.push_back(item);
	}

	return false;
}

bool readDialogResource(Common::SeekableReadStream &stream,
		DialogResource &resource) {
	if (stream.size() < 21)
		return false;

	resource.bounds.top = stream.readUint16BE();
	resource.bounds.left = stream.readUint16BE();
	resource.bounds.bottom = stream.readUint16BE();
	resource.bounds.right = stream.readUint16BE();
	stream.skip(10); // Procedure, visibility, go-away flag, and refCon.
	resource.itemList = stream.readUint16BE();
	/* Common::String title = */ stream.readPascalString();
	return !stream.err() && resource.bounds.isValidRect();
}

bool readDialogItems(Common::SeekableReadStream &stream,
		DialogResource &resource) {
	if (stream.size() < 2)
		return false;

	const uint itemCount = stream.readUint16BE() + 1;
	for (uint itemNumber = 0; itemNumber < itemCount; ++itemNumber) {
		if (stream.pos() + 14 > stream.size())
			return false;

		DialogResourceItem item;
		stream.skip(4); // Placeholder for a handle or procedure pointer.
		item.bounds.top = stream.readUint16BE();
		item.bounds.left = stream.readUint16BE();
		item.bounds.bottom = stream.readUint16BE();
		item.bounds.right = stream.readUint16BE();
		const byte rawType = stream.readByte();
		const uint length = stream.readByte();
		if (stream.pos() + length > stream.size())
			return false;

		item.type = rawType & 0x7f;
		item.enabled = (rawType & 0x80) == 0;
		if (item.type == kDialogButton ||
				item.type == kDialogRadioButton ||
				item.type == kDialogStaticText) {
			for (uint i = 0; i < length; ++i)
				item.text += (char)stream.readByte();
			resource.items.push_back(item);
		} else {
			stream.skip(length);
		}

		if (length & 1)
			stream.skip(1);
	}

	return !stream.err();
}

Common::Rect getDialogItemBounds(const DialogResource &dialog,
		const DialogResourceItem &item) {
	Common::Rect bounds(item.bounds);
	bounds.translate(dialog.bounds.left, dialog.bounds.top);
	return bounds;
}

void drawDifficultyDialog(Graphics::ManagedSurface &screen,
		Graphics::MacWindowManager &windowManager,
		const Graphics::Font &font, const DialogResource &dialog,
		int selectedRadio, bool pressedButton) {
	Graphics::Primitives &primitives = windowManager.getDrawPrimitives();
	Graphics::MacPlotData plot(&screen, nullptr,
		&windowManager.getBuiltinPatterns(), 1, 0, 0,
		Common::Point(1, 1), windowManager._colorWhite, false);
	primitives.drawFilledRect1(dialog.bounds, windowManager._colorWhite, &plot);
	primitives.drawRect1(dialog.bounds, windowManager._colorBlack, &plot);

	int radio = 0;
	for (uint i = 0; i < dialog.items.size(); ++i) {
		const DialogResourceItem &item = dialog.items[i];
		const Common::Rect bounds = getDialogItemBounds(dialog, item);
		const int textY = bounds.top +
			(bounds.height() - font.getFontHeight()) / 2;

		switch (item.type) {
		case kDialogButton: {
			primitives.drawRect1(bounds, windowManager._colorBlack, &plot);
			if (pressedButton) {
				Common::Rect inside(bounds);
				inside.grow(-2);
				primitives.drawFilledRect1(inside,
					windowManager._colorBlack, &plot);
			}
			const uint32 color = pressedButton ? windowManager._colorWhite :
				windowManager._colorBlack;
			const int textX = bounds.left +
				(bounds.width() - font.getStringWidth(item.text)) / 2;
			font.drawString(&screen, item.text, textX, textY,
				bounds.width(), color);
			break;
		}
		case kDialogRadioButton: {
			const int circleTop = bounds.top + (bounds.height() - 12) / 2;
			primitives.drawEllipse(bounds.left, circleTop,
				bounds.left + 11, circleTop + 11,
				windowManager._colorBlack, false, &plot);
			if (radio == selectedRadio)
				primitives.drawEllipse(bounds.left + 4, circleTop + 4,
					bounds.left + 7, circleTop + 7,
					windowManager._colorBlack, true, &plot);
			font.drawString(&screen, item.text, bounds.left + 17, textY,
				bounds.width() - 17, windowManager._colorBlack);
			++radio;
			break;
		}
		case kDialogStaticText:
			font.drawString(&screen, item.text, bounds.left, textY,
				bounds.width(), windowManager._colorBlack);
			break;
		default:
			break;
		}
	}
}

int difficultyForRadio(int radio) {
	switch (radio) {
	case 0:
		return DIFFICULTY_EASY;
	case 2:
		return DIFFICULTY_HARD;
	default:
		return DIFFICULTY_MEDIUM;
	}
}

} // namespace

MacNebularMenu::MacNebularMenu(RexNebularEngine &engine,
		MacResourceProvider &resources, Graphics::ManagedSurface &screen) :
		_engine(engine), _resources(resources), _screen(screen) {
}

MacNebularMenu::~MacNebularMenu() {
	delete _windowManager;
}

bool MacNebularMenu::initializeWindowManager() {
	if (_windowManager)
		return true;

	// MacWindowManager installs its default palette during construction. Save
	// and restore the active MADS palette before it can reach the backend.
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	const uint32 mode = Graphics::kWMModeNoDesktop |
		Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride |
		Graphics::kWMModeForceMacFonts | Graphics::kWMModeNoSystemRedraw |
		Graphics::kWMNoScummVMWallpaper;
	_windowManager = new Graphics::MacWindowManager(mode);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	_windowManager->setEngine(&_engine);
	_windowManager->setScreen(&_screen);
	_windowManager->passPalette(palette, 256);
	memcpy(_palette, palette, sizeof(_palette));
	_paletteValid = true;
	return true;
}

bool MacNebularMenu::initialize() {
	if (!initializeWindowManager())
		return false;
	if (_menu)
		return true;

	_menu = _windowManager->addMenu();
	_menu->setCommandsCallback(menuCallback, this);

	if (!loadMenuResource(1000) || !loadMenuResource(1001) ||
			!loadMenuResource(1002) || !loadMenuResource(2001) ||
			!loadMenuResource(2101, _menu->getSubmenu(nullptr, 3), 2) ||
			!loadMenuResource(2102, _menu->getSubmenu(nullptr, 3), 3) ||
			!loadMenuResource(2000)) {
		warning("Could not construct the Macintosh Rex menu bar");
		return false;
	}

	_menu->calcDimensions();
	updateState();
	return true;
}

bool MacNebularMenu::loadMenuResource(uint16 resourceID,
		Graphics::MacMenuSubMenu *parent, int parentItem) {
	Common::SeekableReadStream *stream = _resources.openResource(
		MacResourceProvider::kApplicationContainer, MKTAG('M', 'E', 'N', 'U'),
		resourceID);
	if (!stream)
		return false;

	MenuResource resource;
	const bool valid = readMenuResource(*stream, resource);
	delete stream;
	if (!valid)
		return false;

	Graphics::MacMenuSubMenu *submenu = nullptr;
	if (parent) {
		if (parentItem < 0 || (uint)parentItem >= parent->items.size())
			return false;
		// In a classic MENU resource, the parent item's mark byte carries
		// the hierarchical submenu ID. Refuse to attach a mismatched resource.
		if (parent->items[parentItem]->checkSymbol != resource.id)
			return false;
		submenu = _menu->addSubMenu(parent, parentItem);
	} else {
		const int menuIndex = _menu->addMenuItem(nullptr, resource.title);
		submenu = _menu->addSubMenu(nullptr, menuIndex);
	}

	for (uint index = 0; index < resource.items.size(); ++index) {
		const MenuResourceItem &item = resource.items[index];
		const char shortcut = item.key == 0x1b ? 0 : item.key;
		const int itemIndex = _menu->addMenuItem(submenu, item.text,
			(resource.id << 16) | index, item.style, shortcut, item.enabled);
		submenu->items[itemIndex]->checkSymbol = item.mark;
	}

	return true;
}

Graphics::MacMenuItem *MacNebularMenu::getMenuItem(int menu, int item) const {
	Graphics::MacMenuItem *topLevel = _menu->getMenuItem(menu);
	if (!topLevel || !topLevel->submenu || item < 0 ||
			(uint)item >= topLevel->submenu->items.size())
		return nullptr;
	return topLevel->submenu->items[item];
}

Graphics::MacMenuItem *MacNebularMenu::getSubMenuItem(int menu,
		int parentItem, int item) const {
	Graphics::MacMenuItem *parent = getMenuItem(menu, parentItem);
	if (!parent || !parent->submenu || item < 0 ||
			(uint)item >= parent->submenu->items.size())
		return nullptr;
	return parent->submenu->items[item];
}

void MacNebularMenu::setItemState(Graphics::MacMenuItem *item,
		bool enabled, bool checked) {
	if (!item)
		return;
	if (item->enabled != enabled)
		_menu->setEnabled(item, enabled);
	if (item->checked != checked)
		_menu->setCheckMark(item, checked);
}

void MacNebularMenu::updateState() {
	// Start with every direct action disabled. This preserves separators and
	// keeps native actions without a safe ScummVM equivalent visible.
	for (int menuIndex = 0; menuIndex < _menu->numberOfMenus(); ++menuIndex) {
		Graphics::MacMenuItem *topLevel = _menu->getMenuItem(menuIndex);
		for (int itemIndex = 0;
				itemIndex < _menu->numberOfMenuItems(topLevel); ++itemIndex)
			setItemState(_menu->getSubMenuItem(topLevel, itemIndex), false, false);
	}

	setItemState(getMenuItem(kFileMenu, 2),
		_engine.canLoadGameStateCurrently(nullptr), false);
	const bool canSave = _engine.canSaveGameStateCurrently(nullptr);
	setItemState(getMenuItem(kFileMenu, 4), canSave, false);
	setItemState(getMenuItem(kFileMenu, 5), canSave, false);
	setItemState(getMenuItem(kFileMenu, 9), true, false);

	const bool noSound = !_engine._musicFlag && !_engine._soundFlag;
	setItemState(getMenuItem(kOptionsMenu, 0), true, noSound);
	setItemState(getMenuItem(kOptionsMenu, 1), true, inter_report_hotspots);
	setItemState(getMenuItem(kOptionsMenu, 2), true, false);
	setItemState(getMenuItem(kOptionsMenu, 3), true, false);

	for (int fade = SCREEN_FADE_SMOOTH; fade <= SCREEN_FADE_FAST; ++fade)
		setItemState(getSubMenuItem(kOptionsMenu, 2, fade), true,
			kernel_screen_fade == fade);

	setItemState(getSubMenuItem(kOptionsMenu, 3, 0),
		config_file.naughtiness != NAUGHTY,
		config_file.naughtiness == NAUGHTY);
	setItemState(getSubMenuItem(kOptionsMenu, 3, 1),
		config_file.naughtiness != NICE,
		config_file.naughtiness == NICE);
	setItemState(getSubMenuItem(kOptionsMenu, 3, 2), false,
		config_file.naughtiness != NAUGHTY && config_file.naughtiness != NICE);

	setItemState(getMenuItem(kWindowMenu, 0), false, false);
	setItemState(getMenuItem(kWindowMenu, 1), false, false);
	setItemState(getMenuItem(kWindowMenu, 2), true, true);

	if (_outerMenuActive) {
		// CODE 133 disables New, Resume, Open, Save, and Save As while
		// the native room-990 controller owns the application window.
		setItemState(getMenuItem(kFileMenu, 0), false, false);
		setItemState(getMenuItem(kFileMenu, 1), false, false);
		setItemState(getMenuItem(kFileMenu, 2), false, false);
		setItemState(getMenuItem(kFileMenu, 4), false, false);
		setItemState(getMenuItem(kFileMenu, 5), false, false);
	}
}

void MacNebularMenu::dispatchCommand(int commandId) {
	switch (commandId) {
	case kFileOpen:
		if (_engine.canLoadGameStateCurrently(nullptr))
			_engine.loadGameDialog();
		break;
	case kFileSave:
	case kFileSaveAs:
		if (_engine.canSaveGameStateCurrently(nullptr))
			_engine.saveGameDialog();
		break;
	case kFileQuit:
		_engine.quitGame();
		break;
	case kOptionNoSound: {
		const bool enableSound = !_engine._musicFlag && !_engine._soundFlag;
		_engine._musicFlag = enableSound;
		_engine._soundFlag = enableSound;
		config_file.music_flag = enableSound;
		config_file.sound_flag = enableSound;
		if (!enableSound && _engine._soundManager)
			_engine._soundManager->stop();
		ConfMan.setBool("music_mute", !enableSound);
		ConfMan.setBool("sfx_mute", !enableSound);
		ConfMan.flushToDisk();
		break;
	}
	case kOptionEasyInterface:
		inter_report_hotspots = !inter_report_hotspots;
		config_file.interface_hotspots = inter_report_hotspots ?
			INTERFACE_BRAINDEAD : INTERFACE_MACINTOSH;
		ConfMan.setBool("interface_hotspots", inter_report_hotspots);
		ConfMan.flushToDisk();
		break;
	case kFadeSmooth:
	case kFadeMedium:
	case kFadeFast:
		kernel_screen_fade = commandId - kFadeSmooth;
		config_file.screen_fade = kernel_screen_fade;
		ConfMan.setInt("screen_fade", kernel_screen_fade);
		ConfMan.flushToDisk();
		break;
	case kStoryNaughty:
	case kStoryNice:
		config_file.naughtiness = commandId == kStoryNaughty ? NAUGHTY : NICE;
		ConfMan.setBool("naughtiness", config_file.naughtiness == NAUGHTY);
		ConfMan.flushToDisk();
		break;
	default:
		break;
	}

	updateState();
}

void MacNebularMenu::syncPalette() {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	if (_paletteValid && memcmp(_palette, palette, sizeof(_palette)) == 0)
		return;

	memcpy(_palette, palette, sizeof(_palette));
	_paletteValid = true;
	_windowManager->passPalette(_palette, 256);
}

bool MacNebularMenu::processEvent(Common::Event &event) {
	if (!_windowManager)
		return false;

	updateState();
	const bool handled = _windowManager->processEvent(event);
	if (_pendingCommand != -1) {
		const int commandId = _pendingCommand;
		_pendingCommand = -1;
		dispatchCommand(commandId);
	}
	return handled;
}

void MacNebularMenu::draw() {
	if (!_menu)
		return;

	syncPalette();
	updateState();
	_menu->draw(&_screen, true);
}

byte MacNebularMenu::getBlackColor() {
	byte menuBlack, menuWhite;
	getMenuColors(menuBlack, menuWhite);
	return menuBlack;
}

void MacNebularMenu::getMenuColors(byte &menuBlack, byte &menuWhite) {
	menuBlack = 0;
	menuWhite = 0;
	if (!initializeWindowManager())
		return;
	syncPalette();
	menuBlack = (byte)_windowManager->_colorBlack;
	menuWhite = (byte)_windowManager->_colorWhite;
}

void MacNebularMenu::menuCallback(int commandId, Common::String &, void *data) {
	MacNebularMenu *menus = (MacNebularMenu *)data;
	menus->_pendingCommand = commandId;
	menus->_menu->closeMenu();
}

int MacNebularMenu::runDifficultyDialog() {
	if (!initializeWindowManager())
		return -1;

	Common::SeekableReadStream *dialogStream = _resources.openResource(
		MacResourceProvider::kApplicationContainer,
		MKTAG('D', 'L', 'O', 'G'), kDifficultyDialog);
	if (!dialogStream)
		return -1;

	DialogResource dialog;
	const bool validDialog = readDialogResource(*dialogStream, dialog);
	delete dialogStream;
	if (!validDialog || dialog.itemList != kDifficultyDialog)
		return -1;
	dialog.bounds.moveTo((_screen.w - dialog.bounds.width()) / 2,
		(_screen.h - dialog.bounds.height()) / 2);

	Common::SeekableReadStream *itemsStream = _resources.openResource(
		MacResourceProvider::kApplicationContainer,
		MKTAG('D', 'I', 'T', 'L'), dialog.itemList);
	if (!itemsStream)
		return -1;

	const bool validItems = readDialogItems(*itemsStream, dialog);
	delete itemsStream;
	const Graphics::Font *font = _resources.getDialogFont();
	if (!validItems || !font)
		return -1;

	Graphics::ManagedSurface saved;
	saved.create(dialog.bounds.width(), dialog.bounds.height(), _screen.format);
	saved.copyRectToSurface(_screen.getBasePtr(dialog.bounds.left,
		dialog.bounds.top), _screen.pitch, 0, 0,
		dialog.bounds.width(), dialog.bounds.height());

	int selectedRadio = 1;
	bool pressedButton = false;
	bool done = false;
	bool quit = false;
	bool redraw = true;
	const bool cursorWasVisible = CursorMan.isVisible();
	_windowManager->clearHandlingWidgets();
	_windowManager->pushCursor(Graphics::kMacCursorArrow);
	CursorMan.showMouse(true);

	while (!done) {
		if (redraw) {
			drawDifficultyDialog(_screen, *_windowManager, *font, dialog,
				selectedRadio, pressedButton);
			g_system->copyRectToScreen(_screen.getBasePtr(dialog.bounds.left,
				dialog.bounds.top), _screen.pitch, dialog.bounds.left,
				dialog.bounds.top, dialog.bounds.width(), dialog.bounds.height());
			redraw = false;
		}

		Common::Event event;
		while (!done && g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				quit = true;
				done = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					done = true;
				} else if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedRadio = MAX(0, selectedRadio - 1);
					redraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedRadio = MIN(2, selectedRadio + 1);
					redraw = true;
				}
				break;
			case Common::EVENT_LBUTTONDOWN: {
				int radio = 0;
				for (uint i = 0; i < dialog.items.size(); ++i) {
					const DialogResourceItem &item = dialog.items[i];
					const Common::Rect bounds = getDialogItemBounds(dialog, item);
					if (item.type == kDialogRadioButton) {
						if (item.enabled && bounds.contains(event.mouse.x,
								event.mouse.y)) {
							selectedRadio = radio;
							redraw = true;
						}
						++radio;
					} else if (item.type == kDialogButton && item.enabled &&
							bounds.contains(event.mouse.x, event.mouse.y)) {
						pressedButton = true;
						redraw = true;
					}
				}
				break;
			}
			case Common::EVENT_LBUTTONUP:
				if (pressedButton) {
					for (uint i = 0; i < dialog.items.size(); ++i) {
						const DialogResourceItem &item = dialog.items[i];
						if (item.type == kDialogButton && item.enabled &&
								getDialogItemBounds(dialog, item).contains(
									event.mouse.x, event.mouse.y))
							done = true;
					}
				}
				pressedButton = false;
				redraw = true;
				break;
			default:
				break;
			}
		}

		if (!done) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}

	_screen.copyRectToSurface(saved.getPixels(), saved.pitch,
		dialog.bounds.left, dialog.bounds.top,
		dialog.bounds.width(), dialog.bounds.height());
	g_system->copyRectToScreen(_screen.getBasePtr(dialog.bounds.left,
		dialog.bounds.top), _screen.pitch, dialog.bounds.left,
		dialog.bounds.top, dialog.bounds.width(), dialog.bounds.height());
	g_system->updateScreen();
	_windowManager->popCursor();
	CursorMan.showMouse(cursorWasVisible);
	if (quit)
		_engine.quitGame();

	return difficultyForRadio(selectedRadio);
}

int MacNebularMenu::selectResumeSlot() {
	GUI::SaveLoadChooser dialog(false);
	return dialog.runModalWithCurrentTarget();
}

void selectMacintoshDifficulty(MacNebularMenu *menus) {
	const int configuredDifficulty = ConfMan.getInt("difficulty");
	if (configuredDifficulty >= DIFFICULTY_HARD && configuredDifficulty <= DIFFICULTY_EASY) {
		game.difficulty = configuredDifficulty;
		return;
	}

	const int nativeDifficulty = menus ? menus->runDifficultyDialog() :
		DIFFICULTY_MEDIUM;
	if (nativeDifficulty >= DIFFICULTY_HARD &&
			nativeDifficulty <= DIFFICULTY_EASY) {
		game.difficulty = nativeDifficulty;
		return;
	}

	game.difficulty = DIFFICULTY_MEDIUM;
}

void macintoshGameMenu() {
	g_engine->flushKeys();
	const int requestedMenu = kernel.activate_menu;
	kernel.activate_menu = GAME_NO_MENU;

	if (requestedMenu == GAME_DIFFICULTY_MENU)
		((RexNebularEngine *)g_engine)->selectMacintoshDifficulty();
	else if (requestedMenu != GAME_NO_MENU)
		g_engine->openMainMenuDialog();
}

} // namespace RexNebular
} // namespace MADS

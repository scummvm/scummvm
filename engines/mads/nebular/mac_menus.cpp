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

#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "gui/message.h"
#include "gui/saveload.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/core/sound_manager.h"
#include "mads/mads.h"
#include "mads/nebular/mac_dialogs.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

namespace {

enum {
	kDifficultyDialog = 2500,
	kStartupPreferencesDialog = 2005,
	kPreferencesDialog = 2105,
	kStoryPasswordDialog = 2006,
	kSaveDialog = 3001,
	kOpenDialog = 3010,
	kCopyProtectionDialog = 4000,

	kAppleMenu = 0,
	kFileMenu = 1,
	kEditMenu = 2,
	kOptionsMenu = 3,
	kWindowMenu = 4,

	kAppleAbout = (1000 << 16),
	kFileOpen = (1001 << 16) | 2,
	kFileSave = (1001 << 16) | 4,
	kFileSaveAs = (1001 << 16) | 5,
	kFilePreferences = (1001 << 16) | 7,
	kFileQuit = (1001 << 16) | 9,
	kEditUndo = (1002 << 16),
	kEditCut = (1002 << 16) | 2,
	kEditCopy = (1002 << 16) | 3,
	kEditPaste = (1002 << 16) | 4,
	kEditClear = (1002 << 16) | 5,
	kOptionNoSound = (2001 << 16),
	kOptionEasyInterface = (2001 << 16) | 1,
	kFadeSmooth = (101 << 16),
	kFadeMedium = (101 << 16) | 1,
	kFadeFast = (101 << 16) | 2,
	kStoryNaughty = (102 << 16),
	kStoryNice = (102 << 16) | 1,
	kStoryNiceLocked = (102 << 16) | 2,
	kWindowStandard = (2000 << 16),
	kWindow150 = (2000 << 16) | 1,
	kWindow200 = (2000 << 16) | 2
};

void collectSaves(RexNebularEngine &engine, Common::StringArray &names,
		Common::Array<int> &slots) {
	const SaveStateList saves = engine.listSaves();
	for (SaveStateList::const_iterator save = saves.begin();
			save != saves.end(); ++save) {
		if (save->getSaveSlot() < 1 || save->getSaveSlot() > 99)
			continue;
		names.push_back(save->getDescription());
		slots.push_back(save->getSaveSlot());
	}
}

int findSaveSlot(const Common::Array<int> &slots, int slot) {
	for (uint index = 0; index < slots.size(); ++index) {
		if (slots[index] == slot)
			return index;
	}
	return -1;
}

int findSaveName(const Common::StringArray &names,
		const Common::String &name) {
	for (uint index = 0; index < names.size(); ++index) {
		if (names[index].equalsIgnoreCase(name))
			return index;
	}
	return -1;
}

int firstFreeSaveSlot(const Common::Array<int> &slots) {
	for (int slot = 1; slot <= 99; ++slot) {
		if (findSaveSlot(slots, slot) < 0)
			return slot;
	}
	return -1;
}

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

	_windowManagerMode = Graphics::kWMModeNoDesktop |
		Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride |
		Graphics::kWMModeForceMacFonts | Graphics::kWMModeNoSystemRedraw |
		Graphics::kWMNoScummVMWallpaper;
	if (_engine.getMacintoshHideMenuBar())
		_windowManagerMode |= Graphics::kWMModeAutohideMenu;
	_windowManager = new Graphics::MacWindowManager(_windowManagerMode);
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
	setItemState(getMenuItem(kAppleMenu, 0), true, false);
	if (_activeDialog) {
		setItemState(getMenuItem(kEditMenu, 0),
			_activeDialog->isEditCommandEnabled(kMacDialogUndo), false);
		setItemState(getMenuItem(kEditMenu, 2),
			_activeDialog->isEditCommandEnabled(kMacDialogCut), false);
		setItemState(getMenuItem(kEditMenu, 3),
			_activeDialog->isEditCommandEnabled(kMacDialogCopy), false);
		setItemState(getMenuItem(kEditMenu, 4),
			_activeDialog->isEditCommandEnabled(kMacDialogPaste), false);
		setItemState(getMenuItem(kEditMenu, 5),
			_activeDialog->isEditCommandEnabled(kMacDialogClear), false);
		return;
	}

	setItemState(getMenuItem(kFileMenu, 2),
		_engine.canLoadGameStateCurrently(nullptr), false);
	const bool canSave = _engine.canSaveGameStateCurrently(nullptr);
	setItemState(getMenuItem(kFileMenu, 4), canSave, false);
	setItemState(getMenuItem(kFileMenu, 5), canSave, false);
	setItemState(getMenuItem(kFileMenu, 7), true, false);
	setItemState(getMenuItem(kFileMenu, 9), true, false);

	const bool noSound = !_engine._musicFlag && !_engine._soundFlag;
	setItemState(getMenuItem(kOptionsMenu, 0), true, noSound);
	setItemState(getMenuItem(kOptionsMenu, 1), true, inter_report_hotspots);
	setItemState(getMenuItem(kOptionsMenu, 2), true, false);
	setItemState(getMenuItem(kOptionsMenu, 3), true, false);

	for (int fade = SCREEN_FADE_SMOOTH; fade <= SCREEN_FADE_FAST; ++fade)
		setItemState(getSubMenuItem(kOptionsMenu, 2, fade), true,
			kernel_screen_fade == fade);

	const bool storyLocked = _engine.getMacintoshStoryLocked();
	setItemState(getSubMenuItem(kOptionsMenu, 3, 0),
		storyLocked || config_file.naughtiness != NAUGHTY,
		!storyLocked && config_file.naughtiness == NAUGHTY);
	setItemState(getSubMenuItem(kOptionsMenu, 3, 1),
		storyLocked || config_file.naughtiness != NICE,
		!storyLocked && config_file.naughtiness == NICE);
	setItemState(getSubMenuItem(kOptionsMenu, 3, 2), !storyLocked,
		storyLocked);

	const int displaySize = _engine.getMacintoshDisplaySize();
	for (int size = kMacNebularDisplay100;
			size <= kMacNebularDisplay200; ++size)
		setItemState(getMenuItem(kWindowMenu, size), true,
			displaySize == size);

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
	case kAppleAbout:
		// Run the full-frame presentation after the menu event has unwound.
		// Otherwise the closing menu window can be composited over room 990.
		_aboutRequested = true;
		break;
	case kFileOpen:
		if (_engine.canLoadGameStateCurrently(nullptr))
			runOpenDialog();
		break;
	case kFileSave:
		if (_engine.canSaveGameStateCurrently(nullptr))
			runSaveDialog(false);
		break;
	case kFileSaveAs:
		if (_engine.canSaveGameStateCurrently(nullptr))
			runSaveDialog(true);
		break;
	case kFilePreferences:
		// Like About, this opens a modal presentation. Defer it until the
		// menu event and its transient window have been fully dismissed.
		_preferencesRequested = true;
		break;
	case kFileQuit:
		_engine.quitGame();
		game.going = false;
		break;
	case kEditUndo:
		if (_activeDialog)
			_activeDialog->handleEditCommand(kMacDialogUndo);
		break;
	case kEditCut:
		if (_activeDialog)
			_activeDialog->handleEditCommand(kMacDialogCut);
		break;
	case kEditCopy:
		if (_activeDialog)
			_activeDialog->handleEditCommand(kMacDialogCopy);
		break;
	case kEditPaste:
		if (_activeDialog)
			_activeDialog->handleEditCommand(kMacDialogPaste);
		break;
	case kEditClear:
		if (_activeDialog)
			_activeDialog->handleEditCommand(kMacDialogClear);
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
		if (_engine.getMacintoshStoryLocked() &&
				!runStoryPasswordDialog(true))
			break;
		_engine.setMacintoshStoryLocked(false, Common::String());
		config_file.naughtiness = commandId == kStoryNaughty ? NAUGHTY : NICE;
		ConfMan.setBool("naughtiness", config_file.naughtiness == NAUGHTY);
		ConfMan.flushToDisk();
		break;
	case kStoryNiceLocked:
		if (runStoryPasswordDialog(false)) {
			config_file.naughtiness = NICE;
			ConfMan.setBool("naughtiness", false);
			ConfMan.flushToDisk();
		}
		break;
	case kWindowStandard:
	case kWindow150:
	case kWindow200:
		_engine.setMacintoshDisplaySize(commandId - kWindowStandard, true);
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
	if (!_windowManager || !_menu)
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

bool MacNebularMenu::processDialogEvent(Common::Event &event) {
	return processEvent(event);
}

void MacNebularMenu::draw() {
	if (!_menu)
		return;

	syncPalette();
	updateState();
	_menu->draw(&_screen, true);
}

bool MacNebularMenu::takeAboutRequest() {
	const bool requested = _aboutRequested;
	_aboutRequested = false;
	return requested;
}

void MacNebularMenu::waitForAboutDismissal() {
	bool pressed = false;
	bool released = false;
	bool pressedMouse = false;
	uint32 releaseTime = 0;
	g_system->delayMillis(166);
	while (!_engine.shouldQuit() &&
			(!pressed || !released || g_system->getMillis() < releaseTime)) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				_engine.quitGame();
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (!pressed) {
					pressed = true;
					pressedMouse = true;
					releaseTime = g_system->getMillis() + 333;
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (!pressed) {
					pressed = true;
					pressedMouse = false;
					releaseTime = g_system->getMillis() + 333;
				}
				break;
			case Common::EVENT_LBUTTONUP:
				if (pressed && pressedMouse)
					released = true;
				break;
			case Common::EVENT_KEYUP:
				if (pressed && !pressedMouse)
					released = true;
				break;
			default:
				break;
			}
		}
		g_system->delayMillis(10);
	}
}

bool MacNebularMenu::takePreferencesRequest() {
	const bool requested = _preferencesRequested;
	_preferencesRequested = false;
	return requested;
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

void MacNebularMenu::setMenuBarHidden(bool hidden) {
	if (!initialize())
		return;
	if (hidden)
		_windowManagerMode |= Graphics::kWMModeAutohideMenu;
	else
		_windowManagerMode &= ~Graphics::kWMModeAutohideMenu;
	_windowManager->setMode(_windowManagerMode);
	_menu->setVisible(!hidden, true);
}

bool MacNebularMenu::runPreferencesDialog(bool startup) {
	if (!initializeWindowManager())
		return false;

	syncPalette();
	if (_windowManager->_colorBlack == _windowManager->_colorWhite)
		return false;

	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager,
		this);
	if (!dialog.load(startup ? kStartupPreferencesDialog :
			kPreferencesDialog))
		return false;
	dialog.center();
	dialog.setItemEnabled(2, !startup);
	dialog.setItemChecked(4, _engine.getMacintoshHideMenuBar());
	dialog.setItemEnabled(5, false);
	dialog.setItemEnabled(6, false);
	dialog.setItemChecked(7, true);
	dialog.setItemChecked(8,
		_engine.getMacintoshPreferencesAtStartup());
	_activeDialog = &dialog;
	const int result = dialog.runModal(1, startup ? 0 : 2);
	_activeDialog = nullptr;
	if (result == 1) {
		const bool persist = dialog.isItemChecked(7);
		_engine.setMacintoshHideMenuBar(dialog.isItemChecked(4), persist);
		_engine.setMacintoshPreferencesAtStartup(dialog.isItemChecked(8),
			persist);
	}

	return true;
}

int MacNebularMenu::runPopupEditor(const Common::Rect &bounds,
		char *target, int maxLength) {
	if (!initializeWindowManager() || !target || maxLength < 1)
		return -1;

	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager,
		this);
	dialog.configureInlineEditable(bounds, target, maxLength);
	_activeDialog = &dialog;
	const int result = dialog.runModal(1, 2);
	_activeDialog = nullptr;
	if (result != 1)
		return 1;

	Common::strcpy_s(target, maxLength + 1, dialog.getItemText(1).c_str());
	return 0;
}

int MacNebularMenu::runCopyProtectionDialog(const Common::String &title,
		const Common::String &subtitle, const Common::String &prompt,
		char *target, int maxLength) {
	if (!initializeWindowManager() || !target || maxLength < 1)
		return -1;

	syncPalette();
	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager);
	if (!dialog.load(kCopyProtectionDialog))
		return -1;
	dialog.center();
	dialog.setItemText(2, target);
	dialog.setItemMaxLength(2, maxLength);
	dialog.setItemText(3, title);
	dialog.setItemText(4, subtitle);
	dialog.setItemText(7, prompt);

	_activeDialog = &dialog;
	const int result = dialog.runModal(1, 0);
	_activeDialog = nullptr;
	if (result != 1)
		return 1;

	Common::strcpy_s(target, maxLength + 1, dialog.getItemText(2).c_str());
	return 0;
}

bool MacNebularMenu::runStoryPasswordDialog(bool leavingLocked) {
	if (!initializeWindowManager())
		return false;

	Common::String prompt = leavingLocked ?
		"Enter the password needed to unlock NICE mode." :
		"Enter your password for future unlocking of NICE mode.";
	for (;;) {
		MacNebularDialog dialog(_engine, _resources, _screen,
			*_windowManager, this);
		if (!dialog.load(kStoryPasswordDialog))
			return false;
		dialog.center();
		dialog.setItemText(3, prompt);
		dialog.setItemText(4, Common::String());

		_activeDialog = &dialog;
		const int result = dialog.runModal(1, 2);
		_activeDialog = nullptr;
		if (result != 1)
			return false;

		const Common::String password = dialog.getItemText(4);
		if (!leavingLocked) {
			_engine.setMacintoshStoryLocked(true, password);
			return true;
		}
		if (_engine.verifyMacintoshStoryPassword(password))
			return true;
		prompt = "Incorrect password. Try again.";
	}
}

void MacNebularMenu::runOpenDialog() {
	if (!initializeWindowManager())
		return;

	Common::StringArray names;
	Common::Array<int> slots;
	collectSaves(_engine, names, slots);
	if (slots.empty())
		return;

	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager,
		this);
	if (!dialog.load(kOpenDialog))
		return;
	dialog.center();
	dialog.setItemEnabled(5, false);
	dialog.setItemEnabled(6, false);
	int selection = findSaveSlot(slots, game.last_save + 1);
	if (selection < 0)
		selection = 0;
	dialog.setList(7, names, selection);

	_activeDialog = &dialog;
	const int result = dialog.runModal(1, 3);
	_activeDialog = nullptr;
	if (result != 1)
		return;

	selection = dialog.getListSelection(7);
	if (selection < 0 || (uint)selection >= slots.size())
		return;
	const int slot = slots[selection];
	if (_engine.loadGameState(slot).getCode() == Common::kNoError)
		game.last_save = slot - 1;
}

void MacNebularMenu::runSaveDialog(bool saveAs) {
	Common::StringArray names;
	Common::Array<int> slots;
	collectSaves(_engine, names, slots);
	const int currentSlot = game.last_save + 1;
	const int currentIndex = findSaveSlot(slots, currentSlot);
	if (!saveAs && currentIndex >= 0) {
		const int previousLastSave = game.last_save;
		game.last_save = currentSlot - 1;
		if (_engine.saveGameState(currentSlot,
				names[currentIndex]).getCode() != Common::kNoError)
			game.last_save = previousLastSave;
		return;
	}

	if (!initializeWindowManager())
		return;
	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager,
		this);
	if (!dialog.load(kSaveDialog))
		return;
	dialog.center();
	dialog.setItemEnabled(4, false);
	dialog.setItemEnabled(5, false);
	dialog.setList(8, names, currentIndex >= 0 ? currentIndex : 0);
	if (currentIndex >= 0)
		dialog.setItemText(7, names[currentIndex]);

	_activeDialog = &dialog;
	const int result = dialog.runModal(1, 2);
	_activeDialog = nullptr;
	if (result != 1)
		return;

	Common::String name = dialog.getItemText(7);
	if (name.empty()) {
		const int selection = dialog.getListSelection(8);
		if (selection >= 0 && (uint)selection < names.size())
			name = names[selection];
	}
	if (name.empty())
		return;

	const int matchingIndex = findSaveName(names, name);
	int slot = matchingIndex >= 0 ? slots[matchingIndex] :
		firstFreeSaveSlot(slots);
	if (slot < 0)
		return;
	if (matchingIndex >= 0) {
		GUI::MessageDialog replaceDialog(
			Common::U32String::format(_("A saved game named '%s' already "
				"exists. Replace it?"), name.c_str()),
			_("Replace"), _("Cancel"));
		if (replaceDialog.runModal() != GUI::kMessageOK)
			return;
	}

	const int previousLastSave = game.last_save;
	game.last_save = slot - 1;
	if (_engine.saveGameState(slot, name).getCode() != Common::kNoError)
		game.last_save = previousLastSave;
}

void MacNebularMenu::menuCallback(int commandId, Common::String &, void *data) {
	MacNebularMenu *menus = (MacNebularMenu *)data;
	menus->_pendingCommand = commandId;
	menus->_menu->closeMenu();
}

int MacNebularMenu::runDifficultyDialog() {
	if (!initializeWindowManager())
		return -1;

	MacNebularDialog dialog(_engine, _resources, _screen, *_windowManager,
		this);
	if (!dialog.load(kDifficultyDialog))
		return -1;
	dialog.center();
	dialog.setItemChecked(4, true);
	_activeDialog = &dialog;
	const int result = dialog.runModal(1, 0);
	_activeDialog = nullptr;
	if (result != 1)
		return -1;
	if (dialog.isItemChecked(3))
		return DIFFICULTY_EASY;
	if (dialog.isItemChecked(5))
		return DIFFICULTY_HARD;
	return DIFFICULTY_MEDIUM;
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

	if (requestedMenu == GAME_DIFFICULTY_MENU) {
		((RexNebularEngine *)g_engine)->selectMacintoshDifficulty();
	} else if (requestedMenu != GAME_NO_MENU) {
		g_engine->openMainMenuDialog();

		if (g_engine->shouldQuit())
			game.going = false;
	}
}

} // namespace RexNebular
} // namespace MADS

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
#include "common/translation.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/managed_surface.h"
#include "graphics/paletteman.h"
#include "gui/chooser.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/mads.h"
#include "mads/nebular/mac_menus.h"
#include "mads/nebular/mac_resources.h"
#include "mads/nebular/nebular.h"

namespace MADS {
namespace RexNebular {

namespace {

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

bool MacNebularMenu::initialize() {
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
	disableActions();
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
		submenu = _menu->addSubMenu(parent, parentItem);
	} else {
		const int menuIndex = _menu->addMenuItem(nullptr, resource.title);
		submenu = _menu->addSubMenu(nullptr, menuIndex);
	}

	for (uint index = 0; index < resource.items.size(); ++index) {
		const MenuResourceItem &item = resource.items[index];
		const char shortcut = item.key == 0x1b ? 0 : item.key;
		_menu->addMenuItem(submenu, item.text,
			(resource.id << 16) | index, item.style, shortcut, item.enabled);
	}

	return true;
}

void MacNebularMenu::disableActions() {
	for (int menuIndex = 0; menuIndex < _menu->numberOfMenus(); ++menuIndex) {
		Graphics::MacMenuItem *topLevel = _menu->getMenuItem(menuIndex);
		for (int itemIndex = 0;
				itemIndex < _menu->numberOfMenuItems(topLevel); ++itemIndex)
			_menu->setEnabled(_menu->getSubMenuItem(topLevel, itemIndex), false);
	}
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
	return _windowManager && _windowManager->processEvent(event);
}

void MacNebularMenu::draw() {
	if (!_menu)
		return;

	syncPalette();
	_menu->draw(&_screen, true);
}

void MacNebularMenu::menuCallback(int, Common::String &, void *data) {
	MacNebularMenu *menus = (MacNebularMenu *)data;
	menus->_menu->closeMenu();
}

void selectMacintoshDifficulty() {
	const int configuredDifficulty = ConfMan.getInt("difficulty");
	if (configuredDifficulty >= DIFFICULTY_HARD && configuredDifficulty <= DIFFICULTY_EASY) {
		game.difficulty = configuredDifficulty;
		return;
	}

	Common::U32StringArray choices;
	choices.push_back(_("Novice - Easy"));
	choices.push_back(_("Advanced - Difficult"));
	choices.push_back(_("Expert - Very Difficult"));
	// The generic browser layout is tied to the launcher's game-list widget.
	// Use its screen-based counterpart because this chooser runs in-engine.
	GUI::ChooserDialog dialog(_("Select a Difficulty Level:"), "FileBrowser");
	dialog.setList(choices);

	switch (dialog.runModal()) {
	case 0:
		game.difficulty = DIFFICULTY_EASY;
		break;
	case 1:
		game.difficulty = DIFFICULTY_MEDIUM;
		break;
	case 2:
		game.difficulty = DIFFICULTY_HARD;
		break;
	default:
		game.difficulty = DIFFICULTY_MEDIUM;
		break;
	}
}

void macintoshGameMenu() {
	g_engine->flushKeys();
	const int requestedMenu = kernel.activate_menu;
	kernel.activate_menu = GAME_NO_MENU;

	if (requestedMenu == GAME_DIFFICULTY_MENU)
		selectMacintoshDifficulty();
	else if (requestedMenu != GAME_NO_MENU)
		g_engine->openMainMenuDialog();
}

} // namespace RexNebular
} // namespace MADS

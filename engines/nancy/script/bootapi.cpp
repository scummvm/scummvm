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

#include "common/algorithm.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/ui/ndui/button.h"

namespace Nancy {
namespace Script {

// The boot scripts describe the game by handing tables to the "Init" objects:
//
//   FlagsInit:Create({ "Met_KM", "Saw_Printer", ... })
//   EnvInit:Create({ LIB = { fullName = "Library", medley = { ... } }, ... })
//
// A table with names for keys has no order of its own, so what comes out of one
// is put in the order of the names. That is the order items and characters get
// their numbers in, which end up in saved games.

static const BootTables *s_tables = nullptr;

bool runBootScript(const char *name, const BootTables &tables) {
	LuaState state;

	s_tables = &tables;
	bool result = state.runScript(name);
	s_tables = nullptr;

	return result;
}

// Every Create function takes the object itself and one table
static void checkCreateArguments(lua_State *L, const char *typeName, bool hasTable) {
	checkUserData(L, 1, typeName);
	luaL_checktype(L, 2, LUA_TTABLE);

	if (!s_tables || !hasTable) {
		luaL_error(L, "%s cannot be used by this script", typeName);
	}
}

static void requireField(lua_State *L, bool found, const char *name) {
	if (!found) {
		luaL_error(L, "Field '%s' must be specified", name);
	}
}

static Common::Rect readRectField(lua_State *L, int tableIndex, const char *name) {
	Common::Rect rect;
	if (pushField(L, tableIndex, name)) {
		rect = checkRect(L, -1).toRect();
		lua_pop(L, 1);
	}

	return rect;
}

static bool compareNames(const Common::String &a, const Common::String &b) {
	return a.compareToIgnoreCase(b) < 0;
}

// The keys of a table that are strings, in alphabetical order
static void getSortedKeys(lua_State *L, int tableIndex, Common::Array<Common::String> &keys) {
	lua_pushnil(L);
	while (lua_next(L, tableIndex)) {
		if (lua_type(L, -2) == LUA_TSTRING) {
			keys.push_back(lua_tostring(L, -2));
		}

		lua_pop(L, 1);
	}

	Common::sort(keys.begin(), keys.end(), compareNames);
}

// BootInit

static int bootCreate(lua_State *L) {
	checkCreateArguments(L, "ND.BootInit", s_tables && s_tables->boot);
	ScriptBootData &boot = *s_tables->boot;

	readField(L, 2, "windowName", boot.windowName);
	requireField(L, readField(L, 2, "playerTimeScale", boot.playerTimeScale), "playerTimeScale");
	requireField(L, readField(L, 2, "diagFontNum", boot.diagnosticFontID), "diagFontNum");

	if (g_nancy->getGameType() <= kGameTypeNancy19) {
		requireField(L, readField(L, 2, "saveFolderName", boot.saveFolderName), "saveFolderName");
		return 0;
	}

	// Nancy20 names the files of texts, and the scripts every scene starts with
	requireField(L, pushField(L, 2, "textFiles"), "textFiles");
	readStringList(L, -1, boot.textFileNames);
	lua_pop(L, 1);

	requireField(L, pushField(L, 2, "includeFiles"), "includeFiles");
	readStringList(L, -1, boot.includeFileNames);
	lua_pop(L, 1);
	return 0;
}

// PlayersInit

static int playersCreate(lua_State *L) {
	checkCreateArguments(L, "ND.PlayersInit", s_tables && s_tables->players);
	PCUI &players = *s_tables->players;

	requireField(L, readField(L, 2, "startingPlayer", players.startCharacterName), "startingPlayer");

	Common::String uiName;
	readField(L, 2, "masterUIFile", uiName);
	players.uiName = Common::Path(uiName);

	// Every character is the name of the tree their UI is in
	if (pushField(L, 2, "characters")) {
		luaL_checktype(L, -1, LUA_TTABLE);
		int characters = lua_gettop(L);

		Common::Array<Common::String> names;
		getSortedKeys(L, characters, names);

		players.characters.resize(names.size());
		for (uint i = 0; i < names.size(); ++i) {
			PCUI::Character &character = players.characters[i];
			character.name = names[i];
			character.id = (uint16)i;
			readField(L, characters, names[i].c_str(), character.defaultImageName);
			character.imageName = character.defaultImageName;
		}

		lua_pop(L, 1);
	}

	return 0;
}

// PlayerInit, which is in the boot script of a character's own tree

static int playerCreate(lua_State *L) {
	checkCreateArguments(L, "ND.PlayerInit", s_tables && s_tables->boot && s_tables->playerIndex >= 0);
	ScriptBootData &boot = *s_tables->boot;

	if ((uint)s_tables->playerIndex >= boot.players.size()) {
		boot.players.resize(s_tables->playerIndex + 1);
	}

	// A character's script runs again every time they get switched to
	ScriptBootData::Player &player = boot.players[s_tables->playerIndex];
	player = ScriptBootData::Player();

	Common::String name;
	requireField(L, readField(L, 2, "switchToScene", name), "switchToScene");
	player.becomeSceneName = Common::Path(name);

	name.clear();
	readField(L, 2, "switchAwayScene", name);
	player.leaveSceneName = Common::Path(name);

	name.clear();
	readField(L, 2, "autotextFile", name);
	player.autotextName = Common::Path(name);

	if (pushField(L, 2, "taskYesSounds")) {
		readStringList(L, -1, player.taskDoneSounds);
		lua_pop(L, 1);
	}

	if (pushField(L, 2, "taskNoSounds")) {
		readStringList(L, -1, player.taskUndoneSounds);
		lua_pop(L, 1);
	}

	return 0;
}

// EnvInit

static int envCreate(lua_State *L) {
	checkCreateArguments(L, "ND.EnvInit", s_tables && s_tables->environments);
	ENVS &envs = *s_tables->environments;

	Common::Array<Common::String> codes;
	getSortedKeys(L, 2, codes);

	for (const Common::String &code : codes) {
		lua_getfield(L, 2, code.c_str());
		luaL_checktype(L, -1, LUA_TTABLE);
		int env = lua_gettop(L);

		ENVS::Environment environment;
		environment.code = code;

		if (!readField(L, env, "fullName", environment.name)) {
			luaL_error(L, "Environments must specify the 'fullName' field");
		}

		Common::String name;
		readField(L, env, "onEnterScene", name);
		environment.ambientScriptName = Common::Path(name);

		name.clear();
		readField(L, env, "onLeaveScene", name);
		environment.leaveScriptName = Common::Path(name);

		if (pushField(L, env, "medley")) {
			readStringList(L, -1, environment.musicNames);
			lua_pop(L, 1);
		}

		envs.environments.push_back(environment);
		lua_pop(L, 1);
	}

	return 0;
}

// LoadSaveInit

static int loadSaveCreate(lua_State *L) {
	checkCreateArguments(L, "ND.LoadSaveInit", s_tables && s_tables->boot);
	ScriptBootData &boot = *s_tables->boot;

	requireField(L, readField(L, 2, "cheatSavePrefix", boot.cheatSavePrefix), "cheatSavePrefix");
	requireField(L, readField(L, 2, "saveExtension", boot.saveExtension), "saveExtension");
	requireField(L, readField(L, 2, "defaultSaveName", boot.defaultSaveName), "defaultSaveName");
	requireField(L, readField(L, 2, "savePromptFlag", boot.savePromptFlag), "savePromptFlag");
	requireField(L, readField(L, 2, "savePromptFlagState", boot.savePromptFlagState), "savePromptFlagState");
	return 0;
}

// ViewportInit

static int viewportCreate(lua_State *L) {
	checkCreateArguments(L, "ND.ViewportInit", s_tables && s_tables->viewport && s_tables->boot);
	VIEW &viewport = *s_tables->viewport;
	ScriptBootData &boot = *s_tables->boot;

	// The viewport, and the resolution the game gets drawn at
	viewport.screenPosition = readRectField(L, 2, "location");
	viewport.bounds = readRectField(L, 2, "windowRes");

	boot.leftNodeHotspot = readRectField(L, 2, "leftNodeHotspot");
	boot.rightNodeHotspot = readRectField(L, 2, "rightNodeHotspot");
	boot.uturnNodeHotspot = readRectField(L, 2, "uturnNodeHotspot");
	return 0;
}

// FlagsInit

static int flagsCreate(lua_State *L) {
	checkCreateArguments(L, "ND.FlagsInit", s_tables && s_tables->flags);
	EVNT &flags = *s_tables->flags;

	readStringList(L, 2, flags.eventFlagNames);
	flags.numRecords = flags.eventFlagNames.size();
	return 0;
}

// InventoryInit

static int inventoryCreate(lua_State *L) {
	checkCreateArguments(L, "ND.InventoryInit", s_tables && s_tables->inventory && s_tables->boot);
	INVD &inventory = *s_tables->inventory;

	Common::String imageName;
	if (!readField(L, 2, "ovl", imageName)) {
		luaL_error(L, "InventoryInit:Create fed with table that has no 'ovl' field");
	}

	s_tables->boot->inventoryImageName = Common::Path(imageName);

	Common::Array<Common::String> names;
	getSortedKeys(L, 2, names);

	for (const Common::String &name : names) {
		lua_getfield(L, 2, name.c_str());
		if (!lua_istable(L, -1)) {
			// The image
			lua_pop(L, 1);
			continue;
		}

		int itemTable = lua_gettop(L);

		INVD::Item item;
		item.id = (int16)inventory.items.size();
		item.name = name;
		item.sourceRect = readRectField(L, itemTable, "srcRect");

		bool isViewable = false;
		if (!readField(L, itemTable, "isViewable", isViewable)) {
			luaL_error(L, "Inventory item %s does not set required 'isViewable' field", name.c_str());
		}

		Common::String sceneName;
		readField(L, itemTable, "popUpScene", sceneName);
		item.sceneName = Common::Path(sceneName);

		readField(L, itemTable, "tableIndex", item.valueName);

		// What a click on the item does follows from what it has
		if (!item.valueName.empty()) {
			item.kind = UI::NDUI::InvButton::kItemOpenCountedScene;
		} else if (sceneName.empty()) {
			item.kind = UI::NDUI::InvButton::kItemPickUp;
		} else if (isViewable) {
			item.kind = UI::NDUI::InvButton::kItemView;
		} else {
			item.kind = UI::NDUI::InvButton::kItemOpenScene;
		}

		inventory.items.push_back(item);
		lua_pop(L, 1);
	}

	return 0;
}

// CursorInit

// Reads the cursors of a table into a list; the keys are names for the cursors
// of the game and items for the ones of the inventory
static void readCursor(lua_State *L, int cursorTable, ScriptBootData::Cursor &cursor) {
	bool hasAll = true;

	static const char *const kSourceNames[] = { "regSrc", "hotSrc" };
	static const char *const kHotspotNames[] = { "regActivePt", "hotActivePt" };

	for (uint i = 0; i < ScriptBootData::kNumCursorVariants; ++i) {
		if (pushField(L, cursorTable, kSourceNames[i])) {
			cursor.sources[i] = checkRect(L, -1).toRect();
			lua_pop(L, 1);
		} else {
			hasAll = false;
		}

		ScriptPoint hotspot;
		if (pushField(L, cursorTable, kHotspotNames[i])) {
			if (!readPoint(L, -1, hotspot)) {
				hasAll = false;
			}

			cursor.hotspots[i] = hotspot.toPoint();
			lua_pop(L, 1);
		} else {
			hasAll = false;
		}
	}

	if (!hasAll) {
		luaL_error(L, "Expecting fields regSrc, hotSrc, regActivePt and hotActivePt");
	}

	readField(L, cursorTable, "overrideInv", cursor.overridesItem);
}

static void requireCursorField(lua_State *L, const char *name, Common::String &value) {
	if (!readField(L, 2, name, value)) {
		luaL_error(L, "CursorInit:Create fed with table that has no '%s' field", name);
	}
}

static bool hasCursor(const ScriptBootData &boot, const Common::String &name) {
	for (const ScriptBootData::Cursor &cursor : boot.cursors) {
		if (cursor.name.equalsIgnoreCase(name)) {
			return true;
		}
	}

	return false;
}

static int cursorCreate(lua_State *L) {
	checkCreateArguments(L, "ND.CursorInit", s_tables && s_tables->boot);
	ScriptBootData &boot = *s_tables->boot;

	Common::String imageName;
	requireCursorField(L, "ovl", imageName);
	boot.cursorImageName = Common::Path(imageName);

	requireCursorField(L, "default", boot.defaultCursorName);
	requireCursorField(L, "rightNode", boot.rightNodeCursorName);
	requireCursorField(L, "leftNode", boot.leftNodeCursorName);
	requireCursorField(L, "uturnNode", boot.uturnNodeCursorName);

	Common::Array<Common::String> names;
	getSortedKeys(L, 2, names);

	for (const Common::String &name : names) {
		lua_getfield(L, 2, name.c_str());
		if (lua_istable(L, -1)) {
			ScriptBootData::Cursor cursor;
			cursor.name = name;
			readCursor(L, lua_gettop(L), cursor);
			boot.cursors.push_back(cursor);
		}

		lua_pop(L, 1);
	}

	if (!hasCursor(boot, boot.defaultCursorName)) {
		luaL_error(L, "CursorInit:Create specified default as %s but there's no cursor by that name", boot.defaultCursorName.c_str());
	}

	return 0;
}

// The cursors of the items that can be held. The keys are the items themselves.
static int cursorCreateInv(lua_State *L) {
	checkCreateArguments(L, "ND.CursorInit", s_tables && s_tables->boot && s_tables->inventory);
	ScriptBootData &boot = *s_tables->boot;
	const INVD &inventory = *s_tables->inventory;

	Common::String imageName;
	requireCursorField(L, "ovl", imageName);
	boot.itemCursorImageName = Common::Path(imageName);

	// One for every item, so they can be found by the id of the item
	boot.itemCursors.resize(inventory.items.size());

	lua_pushnil(L);
	while (lua_next(L, 2)) {
		if (lua_isuserdata(L, -2)) {
			int16 itemID = checkInventoryItem(L, -2);
			luaL_checktype(L, -1, LUA_TTABLE);

			ScriptBootData::Cursor &cursor = boot.itemCursors[itemID];
			cursor.name = inventory.items[itemID].name;
			readCursor(L, lua_gettop(L), cursor);
		}

		lua_pop(L, 1);
	}

	return 0;
}

// FontsInit and ColorsInit, whose tables go by number

static int fontsCreate(lua_State *L) {
	checkCreateArguments(L, "ND.FontsInit", s_tables && s_tables->boot);
	ScriptBootData &boot = *s_tables->boot;

	lua_pushnil(L);
	while (lua_next(L, 2)) {
		ScriptBootData::SystemFont font;
		font.id = luaL_checkint(L, -2);
		luaL_checktype(L, -1, LUA_TTABLE);
		int fontTable = lua_gettop(L);

		readField(L, fontTable, "ndName", font.name);
		readField(L, fontTable, "winName", font.faceName);
		readField(L, fontTable, "size", font.height);
		readField(L, fontTable, "scale", font.isScaled);

		Common::String weight;
		readField(L, fontTable, "weight", weight);
		if (weight == "bold") {
			font.isBold = true;
		} else if (weight != "normal") {
			luaL_error(L, "Font weight must be normal or bold");
		}

		boot.fonts.push_back(font);
		lua_pop(L, 1);
	}

	return 0;
}

static uint32 readColorField(lua_State *L, int tableIndex, const char *name) {
	uint32 color = 0;
	if (pushField(L, tableIndex, name)) {
		color = checkColor(L, -1);
		lua_pop(L, 1);
	}

	return color;
}

static int colorsCreate(lua_State *L) {
	checkCreateArguments(L, "ND.ColorsInit", s_tables && s_tables->boot);
	ScriptBootData &boot = *s_tables->boot;

	lua_pushnil(L);
	while (lua_next(L, 2)) {
		ScriptBootData::FontColor color;
		color.id = luaL_checkint(L, -2);
		if (!lua_istable(L, -1)) {
			luaL_argerror(L, 2, "Expected all fields of parameter to be tables");
		}

		int colorTable = lua_gettop(L);
		color.normal = readColorField(L, colorTable, "normal");
		color.disabled = readColorField(L, colorTable, "disabled");
		color.mouseOver = readColorField(L, colorTable, "mouseOver");
		color.pressed = readColorField(L, colorTable, "pressed");

		boot.fontColors.push_back(color);
		lua_pop(L, 1);
	}

	return 0;
}

// AutotextInit, which holds texts by their keys

static int autotextCreate(lua_State *L) {
	checkCreateArguments(L, "ND.AutotextInit", s_tables && s_tables->texts);
	CVTX &texts = *s_tables->texts;

	lua_pushnil(L);
	while (lua_next(L, 2)) {
		if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING) {
			texts.texts.setVal(lua_tostring(L, -2), lua_tostring(L, -1));
		}

		lua_pop(L, 1);
	}

	return 0;
}

struct InitObject {
	const char *globalName;
	const char *typeName;
	lua_CFunction create;
};

static const InitObject kInitObjects[] = {
	{ "BootInit", "ND.BootInit", bootCreate },
	{ "PlayersInit", "ND.PlayersInit", playersCreate },
	{ "PlayerInit", "ND.PlayerInit", playerCreate },
	{ "EnvInit", "ND.EnvInit", envCreate },
	{ "LoadSaveInit", "ND.LoadSaveInit", loadSaveCreate },
	{ "ViewportInit", "ND.ViewportInit", viewportCreate },
	{ "FlagsInit", "ND.FlagsInit", flagsCreate },
	{ "InventoryInit", "ND.InventoryInit", inventoryCreate },
	{ "FontsInit", "ND.FontsInit", fontsCreate },
	{ "ColorsInit", "ND.ColorsInit", colorsCreate },
	{ "AutotextInit", "ND.AutotextInit", autotextCreate }
};

// Nancy20 sets InventoryInit.addToFront before the items get made. It picks
// where new item buttons go in the inventory, and the only value used is the
// one the earlier games always had, so it is only taken in.
static int inventorySetField(lua_State *L) {
	checkUserData(L, 1, "ND.InventoryInit");
	const char *name = luaL_checkstring(L, 2);
	if (scumm_stricmp(name, "addToFront") != 0) {
		luaL_error(L, "InventoryInit has no field %s", name);
	}

	luaL_checktype(L, 3, LUA_TBOOLEAN);
	return 0;
}

void registerBootAPI(lua_State *L) {
	for (const InitObject &object : kInitObjects) {
		const luaL_Reg functions[] = {
			{ "__index", indexFunctions },
			{ "Create", object.create },
			{ nullptr, nullptr }
		};

		registerGlobal(L, object.globalName, object.typeName, functions);
	}

	luaL_getmetatable(L, "ND.InventoryInit");
	lua_pushcfunction(L, inventorySetField);
	lua_setfield(L, -2, "__newindex");
	lua_pop(L, 1);

	static const luaL_Reg kCursorInitFunctions[] = {
		{ "__index", indexFunctions },
		{ "Create", cursorCreate },
		{ "CreateInv", cursorCreateInv },
		{ nullptr, nullptr }
	};

	registerGlobal(L, "CursorInit", "ND.CursorInit", kCursorInitFunctions);
}

} // End of namespace Script
} // End of namespace Nancy

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
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/tokenizer.h"

#include "audio/mixer.h"

#include "engines/achievements.h"
#include "engines/metaengine.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/resource/resource.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"
#include "engines/nancy/script/v5/brain.h"

#include "engines/nancy/script/v5/gamefunctions.h"
#include "engines/nancy/script/v5/messaging.h"
#include "engines/nancy/script/v5/scriptinput.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/ndui/manager.h"

namespace Nancy {
namespace Script {

// Puts a function in front of an entry of the metatable of a type, such as its
// __index. What was there becomes the upvalue of the function, which hands the
// call on to it with callReplaced() for what it doesn't handle itself.
static void replaceFunction(lua_State *L, const char *typeName, const char *name, lua_CFunction function) {
	luaL_getmetatable(L, typeName);
	lua_getfield(L, -1, name);
	lua_pushcclosure(L, function, 1);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

static int callReplaced(lua_State *L) {
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_insert(L, 1);
	lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
	return lua_gettop(L);
}

// Adds functions to the metatable of a type
static void addFunctions(lua_State *L, const char *typeName, const luaL_Reg *functions) {
	luaL_getmetatable(L, typeName);
	luaL_register(L, nullptr, functions);
	lua_pop(L, 1);
}

// Flags and VarTable

// Flags:All() and VarTable:All(): the names of all of them, for the cheat tools
static void pushNameList(lua_State *L, const Common::Array<Common::String> &names) {
	lua_createtable(L, names.size(), 0);
	for (uint i = 0; i < names.size(); ++i) {
		lua_pushstring(L, names[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}
}

static int flagsAll(lua_State *L) {
	const EVNT *flags = GetEngineData(EVNT);
	pushNameList(L, flags ? flags->eventFlagNames : Common::Array<Common::String>());
	return 1;
}

static int varTableAll(lua_State *L) {
	Common::Array<Common::String> names;
	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	for (ScriptBootData::NameSet::const_iterator it = boot->valueNames.begin(); it != boot->valueNames.end(); ++it) {
		names.push_back(it->_key);
	}

	pushNameList(L, names);
	return 1;
}

static int flagsIndex(lua_State *L) {
	if (strcmp(luaL_checkstring(L, 2), "All") == 0) {
		lua_pushcfunction(L, flagsAll);
		return 1;
	}

	return callReplaced(L);
}

static int varTableIndex(lua_State *L) {
	if (strcmp(luaL_checkstring(L, 2), "All") == 0) {
		lua_pushcfunction(L, varTableAll);
		return 1;
	}

	return callReplaced(L);
}

// Cursor: where the mouse is on the screen, and whether it was just clicked twice
static int cursorIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "activePoint") {
		pushPoint(L, getScriptMousePoint());
		return 1;
	} else if (key == "leftDouble" || key == "rightDouble") {
		lua_pushboolean(L, isDoubleClick(key == "leftDouble" ? 0 : 1));
		return 1;
	}

	return callReplaced(L);
}

// Key

// Key:IsDown(key). Only the modifier keys can be told apart while they
// are held; the others come one press at a time.
static int keyIsDown(lua_State *L) {
	const int modifiers = g_system->getEventManager()->getModifierState();

	bool isDown = false;
	switch (checkKey(L, 2)) {
	case Common::KEYCODE_LCTRL:
		isDown = (modifiers & Common::KBD_CTRL) != 0;
		break;
	case Common::KEYCODE_LSHIFT:
		isDown = (modifiers & Common::KBD_SHIFT) != 0;
		break;
	default:
		break;
	}

	lua_pushboolean(L, isDown);
	return 1;
}

// Key.F1 to Key.F12, and Key:IsDown()
static int keyIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	if (strcmp(name, "IsDown") == 0) {
		lua_pushcfunction(L, keyIsDown);
		return 1;
	}

	for (int i = 0; i < 12; ++i) {
		if (Common::String::format("F%d", i + 1).equalsIgnoreCase(name)) {
			pushKey(L, (Common::KeyCode)(Common::KEYCODE_F1 + i));
			return 1;
		}
	}

	return callReplaced(L);
}

// Game

static ScriptGameState &getGameState() {
	ScriptGameState *state = (ScriptGameState *)const_cast<EngineData *>(g_nancy->getEngineData("ScriptGameState"));
	assert(state);
	return *state;
}

// The saved games the scripts made by themselves, the second chances, which
// the dialog for loading leaves out. ScummVM's saves have no place for that, so
// their names are kept with the settings of the game.
static const char *const kContinueSavesKey = "continue_saves";

static Common::Array<Common::String> getContinueSaves() {
	Common::Array<Common::String> names;
	if (ConfMan.hasKey(kContinueSavesKey, ConfMan.getActiveDomainName())) {
		Common::String list = ConfMan.get(kContinueSavesKey, ConfMan.getActiveDomainName());
		for (Common::String::size_type start = 0; start < list.size(); ) {
			Common::String::size_type end = list.findFirstOf('|', start);
			if (end == Common::String::npos) {
				end = list.size();
			}

			names.push_back(list.substr(start, end - start));
			start = end + 1;
		}
	}

	return names;
}

static void setContinueSave(const Common::String &name, bool isContinue) {
	Common::Array<Common::String> names = getContinueSaves();
	Common::String list;
	for (const Common::String &other : names) {
		if (!other.equalsIgnoreCase(name)) {
			list += (list.empty() ? "" : "|") + other;
		}
	}

	if (isContinue) {
		list += (list.empty() ? "" : "|") + name;
	}

	ConfMan.set(kContinueSavesKey, list, ConfMan.getActiveDomainName());
	ConfMan.flushToDisk();
}

// The name of a saved game, from a table { name, useAutotext }
static Common::String readSaveName(lua_State *L, int index) {
	Common::String name;
	if (lua_isstring(L, index)) {
		return lua_tostring(L, index);
	}

	luaL_checktype(L, index, LUA_TTABLE);
	readField(L, index, "name", name);

	bool usesAutotext = true;
	readField(L, index, "useAutotext", usesAutotext);
	if (usesAutotext) {
		Common::String text;
		if (findText(name, text)) {
			name = text;
		}
	}

	return name;
}

static UI::NDUI::Manager &getSaveManager(lua_State *L) {
	UI::NDUI::Manager *ui = NancySceneState.getNDUI();
	if (!ui) {
		luaL_error(L, "There is no game to save or load");
	}

	return *ui;
}

// Game:Save({ name, useAutotext, isContinue }); the scripts' own saves are
// second chances unless they say otherwise
static int gameSave(lua_State *L) {
	const Common::String name = readSaveName(L, 2);

	bool isContinue = true;
	if (lua_istable(L, 2)) {
		readField(L, 2, "isContinue", isContinue);
	}

	getSaveManager(L).requestSave(name);
	setContinueSave(name, isContinue);

	// The menus save from a Render function, below themselves, so the picture
	// of the saved game shows the scene
	g_nancy->_graphics->keepSavePicture();
	return 0;
}

static int gameLoad(lua_State *L) {
	getSaveManager(L).requestLoad(readSaveName(L, 2));
	return 0;
}

static int gameIsContinue(lua_State *L) {
	const Common::String name = readSaveName(L, 2);

	bool isContinue = false;
	for (const Common::String &other : getContinueSaves()) {
		isContinue |= other.equalsIgnoreCase(name);
	}

	lua_pushboolean(L, isContinue);
	return 1;
}

static int gameSaveExists(lua_State *L) {
	lua_pushboolean(L, getSaveManager(L).hasSave(readSaveName(L, 2)));
	return 1;
}

static int gameAllSaves(lua_State *L) {
	Common::Array<Common::String> names;
	getSaveManager(L).getSaveNames(names);

	lua_createtable(L, names.size(), 0);
	for (uint i = 0; i < names.size(); ++i) {
		lua_pushstring(L, names[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static int gameNew(lua_State *L) {
	++getGameState().numRestarts;
	getSaveManager(L).requestNewGame();
	return 0;
}

static int gameExit(lua_State *L) {
	getSaveManager(L).requestQuit();
	return 0;
}

static int gameSetWindowed(lua_State *L) {
	g_system->beginGFXTransaction();
	g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !lua_toboolean(L, 2));
	g_system->endGFXTransaction();
	return 0;
}

// Game:SetScriptNameFilter(text), Nancy28: the cheat menu that starts streams
// lists the scripts whose names have every word of the text in them, and
// selects the first one that comes after the whole text in alphabetical order
static int gameSetScriptNameFilter(lua_State *L) {
	ScriptGameState &state = getGameState();
	if (state.scriptNames.empty()) {
		Common::Array<Common::Path> paths;
		g_nancy->_resource->list("", paths, CifInfo::kResTypeScript);
		for (const Common::Path &path : paths) {
			Common::String name = path.toString();
			name.toUppercase();
			state.scriptNames.push_back(name);
		}

		Common::sort(state.scriptNames.begin(), state.scriptNames.end());
	}

	Common::String filter = luaL_checkstring(L, 2);
	filter.toUppercase();

	Common::Array<Common::String> words;
	Common::StringTokenizer tokenizer(filter, " ");
	while (!tokenizer.empty()) {
		const Common::String word = tokenizer.nextToken();
		if (!word.empty()) {
			words.push_back(word);
		}
	}

	state.filteredScriptNames.clear();
	state.closestFilteredMatch = 0;
	if (words.empty()) {
		return 0;
	}

	bool hasClosest = false;
	for (const Common::String &name : state.scriptNames) {
		bool hasAllWords = true;
		for (const Common::String &word : words) {
			if (!name.contains(word)) {
				hasAllWords = false;
				break;
			}
		}

		if (!hasAllWords) {
			continue;
		}

		state.filteredScriptNames.push_back(name);
		if (!hasClosest && name.compareTo(filter) >= 0) {
			hasClosest = true;
			state.closestFilteredMatch = state.filteredScriptNames.size() - 1;
		}
	}

	return 0;
}

// Game:GetScriptNameRange(first, last), Nancy28: the scripts the filter let
// through from first to last, counting from 1, as a table with those numbers
// for keys
static int gameGetScriptNameRange(lua_State *L) {
	const Common::Array<Common::String> &names = getGameState().filteredScriptNames;
	const int first = MAX(luaL_checkint(L, 2), 1);
	const int last = MIN(MAX(luaL_checkint(L, 3), first), (int)names.size());

	lua_newtable(L);
	for (int i = first; i <= last; ++i) {
		lua_pushstring(L, names[i - 1].c_str());
		lua_rawseti(L, -2, i);
	}

	return 1;
}

static const luaL_Reg kGameFunctions[] = {
	{ "SetScriptNameFilter", gameSetScriptNameFilter },
	{ "GetScriptNameRange", gameGetScriptNameRange },
	{ "Save", gameSave },
	{ "Load", gameLoad },
	{ "IsContinue", gameIsContinue },
	{ "SaveExists", gameSaveExists },
	{ "AllSaves", gameAllSaves },
	{ "New", gameNew },
	{ "Exit", gameExit },
	{ "SetWindowed", gameSetWindowed },
	{ nullptr, nullptr }
};

static bool pushGameField(lua_State *L, const Common::String &key) {
	for (const luaL_Reg *function = kGameFunctions; function->name; ++function) {
		if (key == function->name) {
			lua_pushcfunction(L, function->func);
			return true;
		}
	}

	if (key == "isWindowed") {
		lua_pushboolean(L, !g_system->getFeatureState(OSystem::kFeatureFullscreenMode));
	} else if (key == "time") {
		lua_pushnumber(L, NancySceneState._timers.lastTotalTime.getTotalMilliseconds() / 1000.0);
	} else if (key == "buildNumber") {
		const ScriptBootData *boot = GetEngineData(ScriptBootData);
		lua_pushstring(L, boot ? boot->buildNumber.c_str() : "");
	} else if (key == "fastConvo") {
		lua_pushboolean(L, getGameState().isFastConvo);
	} else if (key == "cheatMode") {
		lua_pushboolean(L, getGameState().isCheatMode);
	} else if (key == "diags") {
		lua_pushboolean(L, getGameState().showsDiagnostics);
	} else if (key == "needsSave") {
		lua_pushboolean(L, true);
	} else if (key == "restarts") {
		lua_pushinteger(L, getGameState().numRestarts);
	} else if (key == "closestFilteredMatch") {
		// Nancy28, counting from 1
		lua_pushinteger(L, getGameState().closestFilteredMatch + 1);
	} else {
		return false;
	}

	return true;
}

static int gameIndex(lua_State *L) {
	if (pushGameField(L, luaL_checkstring(L, 2))) {
		return 1;
	}

	return callReplaced(L);
}

// Only the settings of the cheat tools can be set
static int gameNewIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "cheatMode") {
		getGameState().isCheatMode = lua_toboolean(L, 3) != 0;
	} else if (key == "fastConvo") {
		getGameState().isFastConvo = lua_toboolean(L, 3) != 0;
	} else if (key == "diags") {
		getGameState().showsDiagnostics = lua_toboolean(L, 3) != 0;
	} else {
		return luaL_error(L, "Field %s is not settable on Game ", key.c_str());
	}

	return 0;
}

// Viewport: the whole screen, which the UI is placed on
static int viewportIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "uiSize" || key == "uiLocation" || key == "windowSize") {
		pushRect(L, ScriptRect(g_nancy->_graphics->getScreenBounds()));
		return 1;
	}

	return callReplaced(L);
}

// Inventory

// The inventory goes by the names of the items, and the item in the player's
// hand stays in it

static const INVD::Item *findItem(const Common::String &name) {
	const INVD *inventory = GetEngineData(INVD);
	if (inventory) {
		for (const INVD::Item &item : inventory->items) {
			if (item.name.equalsIgnoreCase(name)) {
				return &item;
			}
		}
	}

	return nullptr;
}

static const INVD::Item &checkItem(lua_State *L, int index) {
	const char *name = luaL_checkstring(L, index);
	const INVD::Item *item = findItem(name);
	if (!item) {
		luaL_error(L, "There is no inventory item %s", name);
	}

	return *item;
}

static int inventoryAdd(lua_State *L) {
	const INVD::Item &item = checkItem(L, 2);
	if (!NancySceneState.hasItem(item.id)) {
		NancySceneState.addItemToInventory(item.id);
	}

	return 0;
}

static int inventoryRemove(lua_State *L) {
	const INVD::Item &item = checkItem(L, 2);
	if (NancySceneState.getHeldItem() == item.id) {
		NancySceneState.setHeldItem(-1);
	}

	if (NancySceneState.hasItem(item.id)) {
		NancySceneState.removeItemFromInventory(item.id, false);
	}

	return 0;
}

// Inventory:Clear(), Nancy28: puts the item in hand away, and takes every item
// out of the inventory
static int inventoryClear(lua_State *L) {
	NancySceneState.setHeldItem(-1);

	const Common::Array<int16> items = NancySceneState.getInventoryOrder();
	for (int16 id : items) {
		NancySceneState.removeItemFromInventory(id, false);
	}

	return 0;
}

static int inventoryContains(lua_State *L) {
	lua_pushboolean(L, NancySceneState.hasItem(checkItem(L, 2).id) == g_nancy->_true);
	return 1;
}

static int inventoryExists(lua_State *L) {
	lua_pushboolean(L, findItem(luaL_checkstring(L, 2)) != nullptr);
	return 1;
}

static int inventoryIsViewable(lua_State *L) {
	const INVD::Item *item = findItem(luaL_checkstring(L, 2));
	lua_pushboolean(L, item && !item->sceneName.empty());
	return 1;
}

static int inventoryScene(lua_State *L) {
	const INVD::Item *item = findItem(luaL_checkstring(L, 2));
	lua_pushstring(L, item ? item->sceneName.toString().c_str() : "");
	return 1;
}

// Every item there is
static int inventoryAll(lua_State *L) {
	const INVD *inventory = GetEngineData(INVD);
	lua_newtable(L);
	for (uint i = 0; inventory && i < inventory->items.size(); ++i) {
		lua_pushstring(L, inventory->items[i].name.c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static const luaL_Reg kInventoryFunctions[] = {
	{ "Add", inventoryAdd },
	{ "Remove", inventoryRemove },
	{ "Clear", inventoryClear },
	{ "Contains", inventoryContains },
	{ "Exists", inventoryExists },
	{ "IsViewable", inventoryIsViewable },
	{ "Scene", inventoryScene },
	{ "All", inventoryAll },
	{ nullptr, nullptr }
};

static bool pushInventoryField(lua_State *L, const Common::String &key) {
	for (const luaL_Reg *function = kInventoryFunctions; function->name; ++function) {
		if (key == function->name) {
			lua_pushcfunction(L, function->func);
			return true;
		}
	}

	const INVD *inventory = GetEngineData(INVD);
	if (key == "current") {
		// The items in the order they came into the inventory
		const Common::Array<int16> &order = NancySceneState.getInventoryOrder();
		lua_createtable(L, order.size(), 0);
		for (uint i = 0; inventory && i < order.size(); ++i) {
			lua_pushstring(L, inventory->items[order[i]].name.c_str());
			lua_rawseti(L, -2, i + 1);
		}
	} else if (key == "inHand") {
		const int16 heldItem = NancySceneState.getHeldItem();
		lua_pushstring(L, inventory && heldItem >= 0 && (uint)heldItem < inventory->items.size() ? inventory->items[heldItem].name.c_str() : "");
	} else if (key == "currentChangeCount") {
		lua_pushinteger(L, NancySceneState.getInventoryChangeCount());
	} else if (key == "inHandChangeCount") {
		lua_pushinteger(L, NancySceneState.getHeldItemChangeCount());
	} else {
		return false;
	}

	return true;
}

// Inventory.inHand = name, or "" to put it away. Nancy27 also puts it away
// with nil.
static int inventoryNewIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);
	if (key != "inHand") {
		return luaL_error(L, "Inventory does not have a settable field %s", key.c_str());
	}

	const Common::String name = lua_isnil(L, 3) ? "" : luaL_checkstring(L, 3);
	const int16 heldItem = name.empty() ? -1 : checkItem(L, 3).id;
	if (heldItem != NancySceneState.getHeldItem()) {
		NancySceneState.setHeldItem(heldItem);
	}

	return 0;
}

static int inventoryIndex(lua_State *L) {
	if (pushInventoryField(L, luaL_checkstring(L, 2))) {
		return 1;
	}

	return callReplaced(L);
}

// Save, Sound, Scene and Log

// Save:IsNameAcceptable(name): whether a saved game can go by the name. The exe
// makes a file of it, and turns down what a file name can't be. ScummVM keeps
// the name only as the description of the save, so any name goes, an empty one
// too: the save screen checks the name after every key, and puts the last one
// back when it isn't acceptable.
static int saveIsNameAcceptable(lua_State *L) {
	luaL_checkstring(L, 2);
	lua_pushboolean(L, true);
	return 1;
}

// Sound:GetGroupVolume(group) and Sound:SetGroupVolume(group, volume):
// the volume of the voices, the music or the effects, from 0 to 1, which are
// ScummVM's volumes of speech, music and sound effects
static Audio::Mixer::SoundType checkSoundGroup(lua_State *L, int index) {
	const char *group = luaL_checkstring(L, index);
	if (scumm_stricmp(group, "voice") == 0) {
		return Audio::Mixer::kSpeechSoundType;
	} else if (scumm_stricmp(group, "music") == 0) {
		return Audio::Mixer::kMusicSoundType;
	} else if (scumm_stricmp(group, "effects") == 0) {
		return Audio::Mixer::kSFXSoundType;
	}

	luaL_argerror(L, index, "the group must be \"voice\", \"music\" or \"effects\"");
	return Audio::Mixer::kPlainSoundType;
}

static int soundGetGroupVolume(lua_State *L) {
	const int volume = g_system->getMixer()->getVolumeForSoundType(checkSoundGroup(L, 2));
	lua_pushnumber(L, (double)volume / Audio::Mixer::kMaxMixerVolume);
	return 1;
}

static int soundSetGroupVolume(lua_State *L) {
	const Audio::Mixer::SoundType type = checkSoundGroup(L, 2);
	const int volume = (int)(CLIP<double>(luaL_checknumber(L, 3), 0.0, 1.0) * Audio::Mixer::kMaxMixerVolume + 0.5);
	g_system->getMixer()->setVolumeForSoundType(type, volume);

	static const char *const kVolumeKeys[] = { "speech_volume", "music_volume", "sfx_volume" };
	const uint key = type == Audio::Mixer::kSpeechSoundType ? 0 : (type == Audio::Mixer::kMusicSoundType ? 1 : 2);
	ConfMan.setInt(kVolumeKeys[key], volume);
	return 0;
}

// Sound:PauseAbandoned() and Sound:UnpauseAbandoned(), Nancy27: pause and
// resume the sounds that went on playing after their records were gone, as
// the UI pops up over the game and goes away again

static void addPlayingChannels(const SceneContext *context, Common::Array<bool> &isOwned) {
	if (!context) {
		return;
	}

	for (const Record *record : context->getRecords()) {
		uint16 channel;
		if (record->getType() == kRecordSound && ((const SoundRecord *)record)->getPlayingChannel(channel) && channel < isOwned.size()) {
			isOwned[channel] = true;
		}
	}
}

static int soundPauseAbandoned(lua_State *L) {
	Common::Array<bool> isOwned;
	isOwned.resize(g_nancy->getStaticData().soundChannelInfo.numChannels, false);

	Common::Array<State::SceneStream *> streams;
	NancySceneState.getStreams().getDrawOrder(streams);
	for (const State::SceneStream *stream : streams) {
		addPlayingChannels(stream ? stream->getScriptScene() : NancySceneState.getScriptScene(), isOwned);
	}

	Common::Array<ScriptGameState::PausedChannel> &pausedChannels = getGameState().pausedAbandonedChannels;
	for (uint16 i = 0; i < isOwned.size(); ++i) {
		if (isOwned[i] || !g_nancy->_sound->isSoundPlaying(i)) {
			continue;
		}

		bool isPaused = false;
		for (const ScriptGameState::PausedChannel &paused : pausedChannels) {
			isPaused = isPaused || paused.channel == i;
		}

		if (!isPaused) {
			g_nancy->_sound->pauseSound(i, true);

			ScriptGameState::PausedChannel paused;
			paused.channel = i;
			paused.serial = g_nancy->_sound->getChannelSerial(i);
			pausedChannels.push_back(paused);
		}
	}

	return 0;
}

// A channel that got another sound in the meantime is left alone
static int soundUnpauseAbandoned(lua_State *L) {
	Common::Array<ScriptGameState::PausedChannel> &pausedChannels = getGameState().pausedAbandonedChannels;
	for (const ScriptGameState::PausedChannel &paused : pausedChannels) {
		if (g_nancy->_sound->getChannelSerial(paused.channel) == paused.serial) {
			g_nancy->_sound->pauseSound(paused.channel, false);
		}
	}

	pausedChannels.clear();
	return 0;
}

// Scene:IsStreamRunning(stream)
static int sceneIsStreamRunning(lua_State *L) {
	const char *streamName = luaL_checkstring(L, 2);
	const State::SceneStreamManager &streams = NancySceneState.getStreams();
	const bool isRunning = streams.isRunning(streamName) ||
		scumm_stricmp(streamName, State::SceneStreamManager::getMainStreamName()) == 0;

	lua_pushboolean(L, isRunning);
	return 1;
}

// Log:Warn(text) and Log:Warn(location, text)
static int logWarn(lua_State *L) {
	if (lua_gettop(L) >= 3) {
		debugC(1, kDebugScene, "Script warning: %s: %s", luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	} else {
		debugC(1, kDebugScene, "Script warning: %s", luaL_checkstring(L, 2));
	}

	return 0;
}

// Message:Send(recipient, action, ...), Nancy28: a message that comes from no
// record, which returns what the receiver returns. The Message global of the
// earlier games sends to the UI, and from Nancy26 on none of their scripts that
// still use it are run.
static int messageSend(lua_State *L) {
	const Common::String recipient = luaL_checkstring(L, 2);
	const Common::String action = luaL_checkstring(L, 3);
	return sendMessage(L, "", recipient, action, 4, MAX(0, lua_gettop(L) - 3));
}

// Achievements, Nancy28: the achievements of the Steam release, which are
// kept by ScummVM's achievements manager. Achievements.<id> is whether it is
// unlocked.

static const char *const kAchievementsType = "ND.Achievements";

static bool isAchievementKnown(const Common::String &id) {
	for (const Common::String &known : getGameState().achievementIds) {
		if (known == id) {
			return true;
		}
	}

	return false;
}

// Achievements:InitIDs({ id, ... }) gives the achievements there are, once
static int achievementsInitIDs(lua_State *L) {
	luaL_checktype(L, 2, LUA_TTABLE);
	ScriptGameState &state = getGameState();
	if (state.hasAchievementIds) {
		return luaL_error(L, "Achievements are already loaded");
	}

	state.hasAchievementIds = true;
	const int numIds = (int)lua_objlen(L, 2);
	for (int i = 1; i <= numIds; ++i) {
		lua_rawgeti(L, 2, i);
		state.achievementIds.push_back(luaL_checkstring(L, -1));
		lua_pop(L, 1);
	}

	AchMan.setActiveDomain(g_engine->getMetaEngine()->getAchievementsInfo(ConfMan.getActiveDomainName()));
	return 0;
}

static int achievementsGetIDs(lua_State *L) {
	const Common::Array<Common::String> &ids = getGameState().achievementIds;
	lua_createtable(L, ids.size(), 0);
	for (uint i = 0; i < ids.size(); ++i) {
		lua_pushstring(L, ids[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// Achievements:Unlock(id) does nothing for an achievement InitIDs didn't give
static int achievementsUnlock(lua_State *L) {
	const Common::String id = luaL_checkstring(L, 2);
	if (isAchievementKnown(id) && AchMan.isReady()) {
		AchMan.setAchievement(id);
	}

	return 0;
}

// Achievements:ClearAll(), for the cheat menu
static int achievementsClearAll(lua_State *L) {
	if (AchMan.isReady()) {
		for (const Common::String &id : getGameState().achievementIds) {
			AchMan.clearAchievement(id);
		}
	}

	return 0;
}

static int achievementsIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);
	indexFunctions(L);
	if (!lua_isnil(L, -1)) {
		return 1;
	}

	lua_pop(L, 1);
	lua_pushboolean(L, isAchievementKnown(key) && AchMan.isReady() && AchMan.isAchieved(key));
	return 1;
}

void registerNancy26GameAPI(lua_State *L) {
	replaceFunction(L, "ND.Flags", "__index", flagsIndex);
	replaceFunction(L, "ND.VarTable", "__index", varTableIndex);
	replaceFunction(L, "ND.Cursor", "__index", cursorIndex);
	replaceFunction(L, "ND.KeyHelper", "__index", keyIndex);
	replaceFunction(L, "ND.Game", "__index", gameIndex);
	replaceFunction(L, "ND.Viewport", "__index", viewportIndex);
	replaceFunction(L, "ND.Inventory", "__index", inventoryIndex);

	static const luaL_Reg kGameSetters[] = {
		{ "__newindex", gameNewIndex },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kInventorySetters[] = {
		{ "__newindex", inventoryNewIndex },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kSaveFunctions[] = {
		{ "IsNameAcceptable", saveIsNameAcceptable },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kSoundFunctions[] = {
		{ "GetGroupVolume", soundGetGroupVolume },
		{ "SetGroupVolume", soundSetGroupVolume },
		{ "PauseAbandoned", soundPauseAbandoned },
		{ "UnpauseAbandoned", soundUnpauseAbandoned },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kSceneFunctions[] = {
		{ "IsStreamRunning", sceneIsStreamRunning },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kLogFunctions[] = {
		{ "Warn", logWarn },
		{ nullptr, nullptr }
	};

	addFunctions(L, "ND.Game", kGameSetters);
	addFunctions(L, "ND.Inventory", kInventorySetters);
	addFunctions(L, "ND.Save", kSaveFunctions);
	addFunctions(L, "ND.Sound", kSoundFunctions);
	addFunctions(L, "ND.Scene", kSceneFunctions);
	addFunctions(L, "ND.Log", kLogFunctions);

	if (g_nancy->getGameType() >= kGameTypeNancy28) {
		static const luaL_Reg kMessageFunctions[] = {
			{ "Send", messageSend },
			{ nullptr, nullptr }
		};

		addFunctions(L, "ND.Message", kMessageFunctions);
	}

	static const luaL_Reg kAchievementsFunctions[] = {
		{ "__index", achievementsIndex },
		{ "InitIDs", achievementsInitIDs },
		{ "GetIDs", achievementsGetIDs },
		{ "Unlock", achievementsUnlock },
		{ "ClearAll", achievementsClearAll },
		{ nullptr, nullptr }
	};

	registerGlobal(L, "Achievements", kAchievementsType, kAchievementsFunctions);

	registerBrainAPI(L);
}

} // End of namespace Script
} // End of namespace Nancy

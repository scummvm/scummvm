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

#include "common/debug.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/records.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Script {

// Flags.<name>

// The label of a flag, or -1 when there is none of that name
static int16 findFlag(const Common::String &name) {
	const EVNT *flags = GetEngineData(EVNT);
	if (flags) {
		for (uint i = 0; i < flags->eventFlagNames.size(); ++i) {
			if (flags->eventFlagNames[i].equalsIgnoreCase(name)) {
				return (int16)(EVNT::kFirstEventFlag + i);
			}
		}
	}

	return -1;
}

bool setFlagByName(const Common::String &name, bool value) {
	const int16 label = findFlag(name);
	if (label < 0) {
		return false;
	}

	NancySceneState.setEventFlag(label, value ? g_nancy->_true : g_nancy->_false);
	return true;
}

bool getFlagByName(const Common::String &name, bool &value) {
	const int16 label = findFlag(name);
	if (label < 0) {
		return false;
	}

	value = NancySceneState.getEventFlag(label, g_nancy->_true);
	return true;
}

static int flagsIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	bool value = false;
	if (!getFlagByName(name, value)) {
		return luaL_error(L, "Flag \"%s\" referenced but does not exist.", name);
	}

	lua_pushboolean(L, value);
	return 1;
}

static int flagsNewIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	if (!setFlagByName(name, lua_toboolean(L, 3) != 0)) {
		return luaL_error(L, "Attempted to set flag \"%s\" but it does not exist.", name);
	}

	return 0;
}

static const luaL_Reg kFlagsFunctions[] = {
	{ "__index", flagsIndex },
	{ "__newindex", flagsNewIndex },
	{ nullptr, nullptr }
};

// VarTable.<name>, the whole numbers scripts keep by name

static int varTableIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	ScriptValueData *values = (ScriptValueData *)NancySceneState.getPuzzleData(ScriptValueData::getTag());
	lua_pushnumber(L, values->values.getValOrDefault(name, 0));
	return 1;
}

static int varTableNewIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);
	const int32 value = (int32)luaL_checknumber(L, 3);

	ScriptValueData *values = (ScriptValueData *)NancySceneState.getPuzzleData(ScriptValueData::getTag());
	values->values.setVal(name, value);
	return 0;
}

static const luaL_Reg kVarTableFunctions[] = {
	{ "__index", varTableIndex },
	{ "__newindex", varTableNewIndex },
	{ nullptr, nullptr }
};

// Z, which makes the numbers records are ordered by. Higher is closer to the player;
// both functions can take the number to count from in place of the middle.

static int zCloser(lua_State *L) {
	const double amount = luaL_checknumber(L, 2);
	const double from = lua_gettop(L) >= 3 ? luaL_checknumber(L, 3) : 0.0;
	lua_pushnumber(L, from + amount);
	return 1;
}

static int zFarther(lua_State *L) {
	const double amount = luaL_checknumber(L, 2);
	const double from = lua_gettop(L) >= 3 ? luaL_checknumber(L, 3) : 0.0;
	lua_pushnumber(L, from - amount);
	return 1;
}

static const luaL_Reg kZFunctions[] = {
	{ "__index", indexFunctions },
	{ "Closer", zCloser },
	{ "Farther", zFarther },
	{ nullptr, nullptr }
};

// Cursor.<name>, and the variants of a cursor: Cursor.<name>.hot

static const char *const kCursorIndexType = "ND.CursorIndex";

struct CursorIndex {
	uint type;
	bool isHot;
};

bool testCursor(lua_State *L, int index, uint &cursorType, bool &isHot) {
	const CursorIndex *cursor = (const CursorIndex *)testUserData(L, index, kCursorIndexType);
	if (!cursor) {
		return false;
	}

	cursorType = cursor->type;
	isHot = cursor->isHot;
	return true;
}

void pushCursor(lua_State *L, uint cursorType, bool isHot) {
	CursorIndex cursor;
	cursor.type = cursorType;
	cursor.isHot = isHot;
	pushValue(L, kCursorIndexType, cursor);
}

// What "Cursor.currentInv" is while no item is held
static const uint kNoCursorType = 0xFFFFFFFF;

// Cursor.<name>: one of the cursors of the game or of an item, or what the
// mouse is up to
static int cursorIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	const int type = g_nancy->_cursor->getCursorTypeByName(name);
	if (type >= 0) {
		pushCursor(L, type, false);
		return 1;
	}

	const INVD *inventory = GetEngineData(INVD);
	if (inventory) {
		for (const INVD::Item &item : inventory->items) {
			if (item.name.equalsIgnoreCase(name)) {
				pushCursor(L, g_nancy->_cursor->getItemCursorType(item.id), false);
				return 1;
			}
		}
	}

	const Common::String key = name;
	if (key == "currentInv") {
		// The cursor of the item that is being held
		const int16 heldItem = NancySceneState.getHeldItem();
		pushCursor(L, heldItem >= 0 ? g_nancy->_cursor->getItemCursorType(heldItem) : kNoCursorType, false);
		return 1;
	}

	if (key == "activePoint") {
		// Where the mouse is in the viewport, while it is in there
		ScriptPoint point;
		if (!SceneContext::getViewportMouse(g_nancy->_input->getInput(), point)) {
			point = ScriptPoint(-1, -1);
		}

		pushPoint(L, point);
		return 1;
	}

	if (key == "activeUIPoint") {
		// Where the mouse is on the screen
		pushPoint(L, ScriptPoint(g_nancy->_input->getInput().mousePos));
		return 1;
	}

	return luaL_error(L, "Tried to change the cursor to %s, which is not valid.", name);
}

static int cursorEquals(lua_State *L) {
	const CursorIndex &a = *(const CursorIndex *)checkUserData(L, 1, kCursorIndexType);
	const CursorIndex &b = *(const CursorIndex *)checkUserData(L, 2, kCursorIndexType);
	lua_pushboolean(L, a.type == b.type && a.isHot == b.isHot);
	return 1;
}

static int cursorVariantIndex(lua_State *L) {
	CursorIndex cursor = *(const CursorIndex *)checkUserData(L, 1, kCursorIndexType);
	const Common::String key = luaL_checkstring(L, 2);

	if (key.equalsIgnoreCase("Hot")) {
		cursor.isHot = true;
	} else if (key.equalsIgnoreCase("Regular")) {
		cursor.isHot = false;
	} else {
		lua_pushnil(L);
		return 1;
	}

	pushValue(L, kCursorIndexType, cursor);
	return 1;
}

static const luaL_Reg kCursorFunctions[] = {
	{ "__index", cursorIndex },
	{ nullptr, nullptr }
};

static const luaL_Reg kCursorIndexFunctions[] = {
	{ "__index", cursorVariantIndex },
	{ "__eq", cursorEquals },
	{ nullptr, nullptr }
};

// Key.<name>

static const char *const kKeyType = "ND.Key";

struct KeyName {
	const char *name;
	Common::KeyCode key;
};

static const KeyName kKeyNames[] = {
	{ "SPACE", Common::KEYCODE_SPACE },
	{ "END", Common::KEYCODE_END },
	{ "HOME", Common::KEYCODE_HOME },
	{ "LEFT", Common::KEYCODE_LEFT },
	{ "UP", Common::KEYCODE_UP },
	{ "RIGHT", Common::KEYCODE_RIGHT },
	{ "DOWN", Common::KEYCODE_DOWN },
	{ "INSERT", Common::KEYCODE_INSERT },
	{ "DELETE", Common::KEYCODE_DELETE },
	{ "ESC", Common::KEYCODE_ESCAPE },
	{ "ESCAPE", Common::KEYCODE_ESCAPE },
	{ "RETURN", Common::KEYCODE_RETURN },
	{ "TAB", Common::KEYCODE_TAB },
	{ "CONTROL", Common::KEYCODE_LCTRL },
	{ "CTRL", Common::KEYCODE_LCTRL },
	{ "SHIFT", Common::KEYCODE_LSHIFT }
};

void pushKey(lua_State *L, Common::KeyCode key) {
	pushValue(L, kKeyType, (int)key);
}

Common::KeyCode checkKey(lua_State *L, int index) {
	return (Common::KeyCode)*(const int *)checkUserData(L, index, kKeyType);
}

static int keyIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	for (const KeyName &key : kKeyNames) {
		if (scumm_stricmp(key.name, name) == 0) {
			pushKey(L, key.key);
			return 1;
		}
	}

	// A letter or a digit stands for its own key
	if (name[0] != 0 && name[1] == 0 && Common::isAlnum(name[0])) {
		pushKey(L, (Common::KeyCode)tolower(name[0]));
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static int keyEquals(lua_State *L) {
	lua_pushboolean(L, checkKey(L, 1) == checkKey(L, 2));
	return 1;
}

static const luaL_Reg kKeyHelperFunctions[] = {
	{ "__index", keyIndex },
	{ nullptr, nullptr }
};

static const luaL_Reg kKeyFunctions[] = {
	{ "__eq", keyEquals },
	{ nullptr, nullptr }
};

// Game
// TODO: Game.time

// The two levels the game can be played at
static const uint kDifficultyJunior = 0;
static const uint kDifficultySenior = 2;

static int gameIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "difficulty") {
		lua_pushstring(L, NancySceneState.getDifficulty() == kDifficultySenior ? "senior" : "junior");
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static int gameNewIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key != "difficulty") {
		return luaL_error(L, "Game does not have field %s", key.c_str());
	}

	const Common::String value = luaL_checkstring(L, 3);
	if (value == "senior") {
		NancySceneState.setDifficulty(kDifficultySenior);
	} else if (value == "junior") {
		NancySceneState.setDifficulty(kDifficultyJunior);
	} else {
		return luaL_error(L, "difficulty must be set to \"senior\" or \"junior\"");
	}

	return 0;
}

static const luaL_Reg kGameFunctions[] = {
	{ "__index", gameIndex },
	{ "__newindex", gameNewIndex },
	{ nullptr, nullptr }
};

// Draw, for the "Render" functions of the records
// TODO: Draw:Tri

// Draw:Rect(rect, color)
static int drawRect(lua_State *L) {
	const ScriptRect &rect = checkRect(L, 2);
	const uint32 color = checkColor(L, 3);

	Graphics::ManagedSurface *target = getDrawTarget();
	if (!target) {
		return luaL_error(L, "Draw can only be used while a record is rendering");
	}

	Common::Rect area = rect.toRect();
	area.clip(Common::Rect(target->w, target->h));
	if (!area.isEmpty()) {
		target->fillRect(area, target->format.ARGBToColor(color >> 24, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
	}

	return 0;
}

static const luaL_Reg kDrawFunctions[] = {
	{ "__index", indexFunctions },
	{ "Rect", drawRect },
	{ nullptr, nullptr }
};

// SoundChannel.<name> and Sound

static const char *const kSoundChannelType = "ND.SoundChannel";

enum SoundChannelID {
	kChannelFirstMusic		= 0,	// Theme
	kChannelFirstAmbient	= 6,
	kChannelFirstFX			= 14,
	kChannelFirstVoice		= 24,
	kNumSoundChannels		= 30,

	// These stand for every channel of a kind
	kChannelAll				= 0x21,
	kChannelAllMusic		= 0x22,
	kChannelAllFX			= 0x23,
	kChannelAllAmbient		= 0x24,
	kChannelAllVoice		= 0x25
};

struct SoundChannelName {
	const char *name;
	uint16 channel;
};

static const SoundChannelName kSoundChannelNames[] = {
	{ "Theme", 0 }, { "Music1", 1 }, { "Music2", 2 }, { "Music3", 3 }, { "UIMusic", 4 }, { "EngineMusic", 5 },
	{ "Ambient1", 6 }, { "Ambient2", 7 }, { "Ambient3", 8 }, { "Ambient4", 9 },
	{ "Ambient5", 10 }, { "Ambient6", 11 }, { "Ambient7", 12 }, { "Ambient8", 13 },
	{ "FX1", 14 }, { "FX2", 15 }, { "FX3", 16 }, { "FX4", 17 }, { "FX5", 18 }, { "FX6", 19 }, { "FX7", 20 }, { "FX8", 21 },
	{ "UIFX", 22 }, { "EngineFX", 23 },
	{ "NPCVoice", 24 }, { "PlayerVoice", 25 }, { "Voice1", 26 }, { "Voice2", 27 }, { "UIVoice", 28 }, { "EngineVoice", 29 },
	{ "All", kChannelAll }, { "AllMusic", kChannelAllMusic }, { "AllFX", kChannelAllFX },
	{ "AllAmbient", kChannelAllAmbient }, { "AllVoice", kChannelAllVoice }
};

void pushSoundChannel(lua_State *L, uint16 channel) {
	pushValue(L, kSoundChannelType, channel);
}

uint16 checkSoundChannel(lua_State *L, int index) {
	return *(const uint16 *)checkUserData(L, index, kSoundChannelType);
}

static int soundChannelIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	for (const SoundChannelName &channel : kSoundChannelNames) {
		if (scumm_stricmp(channel.name, name) == 0) {
			pushSoundChannel(L, channel.channel);
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

// The channels a channel object stands for
static void getChannelRange(uint16 channel, uint16 &first, uint16 &last) {
	switch (channel) {
	case kChannelAll:
		first = 0;
		last = kNumSoundChannels - 1;
		break;
	case kChannelAllMusic:
		first = kChannelFirstMusic;
		last = kChannelFirstAmbient - 1;
		break;
	case kChannelAllAmbient:
		first = kChannelFirstAmbient;
		last = kChannelFirstFX - 1;
		break;
	case kChannelAllFX:
		first = kChannelFirstFX;
		last = kChannelFirstVoice - 1;
		break;
	case kChannelAllVoice:
		first = kChannelFirstVoice;
		last = kNumSoundChannels - 1;
		break;
	default:
		first = last = channel;
		break;
	}
}

// Sound:Stop(channel)
static int soundStop(lua_State *L) {
	uint16 first, last;
	getChannelRange(checkSoundChannel(L, 2), first, last);

	for (uint16 channel = first; channel <= last; ++channel) {
		g_nancy->_sound->stopSound(channel);
	}

	return 0;
}

// Sound:SetVolume(channel, volume), with the volume going from 0 to 1
static int soundSetVolume(lua_State *L) {
	uint16 first, last;
	getChannelRange(checkSoundChannel(L, 2), first, last);
	const uint16 volume = (uint16)CLIP<int>((int)(luaL_checknumber(L, 3) * 100.0), 0, 100);

	for (uint16 channel = first; channel <= last; ++channel) {
		g_nancy->_sound->setVolume(channel, volume);
	}

	return 0;
}

static const luaL_Reg kSoundChannelHelperFunctions[] = {
	{ "__index", soundChannelIndex },
	{ nullptr, nullptr }
};

static const luaL_Reg kSoundChannelFunctions[] = {
	{ nullptr, nullptr }
};

static const luaL_Reg kSoundFunctions[] = {
	{ "__index", indexFunctions },
	{ "Stop", soundStop },
	{ "SetVolume", soundSetVolume },
	{ nullptr, nullptr }
};

// Scene, which is the scene the script belongs to

static SceneContext *checkContext(lua_State *L) {
	SceneContext *context = LuaState::getContext(L);
	if (!context) {
		luaL_error(L, "The Scene object can only be used by the script of a scene");
	}

	return context;
}

static int sceneIndex(lua_State *L) {
	SceneContext *context = checkContext(L);
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "name") {
		lua_pushstring(L, context->getSceneName().c_str());
	} else if (key == "time") {
		lua_pushnumber(L, context->getTime());
	} else if (key == "frame") {
		lua_pushinteger(L, context->getFrame());
	} else if (key == "streamName") {
		lua_pushstring(L, context->getStreamName().c_str());
	} else {
		return indexFunctions(L);
	}

	return 1;
}

// The parts of a scene a script can take over
static const char *const kSceneFunctionNames[] = {
	"RunOnce", "Run", "RenderAttachment", "ReceiveMessage", "OnSave"
};

static int sceneNewIndex(lua_State *L) {
	SceneContext *context = checkContext(L);
	const char *key = luaL_checkstring(L, 2);

	if (!lua_isfunction(L, 3)) {
		return luaL_error(L, "Can only set functions to the Scene object");
	}

	for (const char *name : kSceneFunctionNames) {
		if (strcmp(name, key) == 0) {
			lua_pushvalue(L, 3);
			context->setSceneFunction(name);
			return 0;
		}
	}

	return luaL_error(L, "Field %s is not overridable from script", key);
}

// Scene:Include(script)
static int sceneInclude(lua_State *L) {
	SceneContext *context = checkContext(L);
	const char *name = luaL_checkstring(L, 2);

	// A scene can't include itself
	if (!context->getSceneName().equalsIgnoreCase(name)) {
		context->include(name);
	}

	return 0;
}

// Scene:BeginStream(name, scene, frame, lifetime, capturesInput, runsFirst)
static int sceneBeginStream(lua_State *L) {
	checkContext(L);
	const char *name = luaL_checkstring(L, 2);
	const char *sceneName = luaL_checkstring(L, 3);
	const int frame = luaL_checkint(L, 4);
	const Common::String lifetime = luaL_checkstring(L, 5);

	State::StreamFlags flags;
	if (lifetime == "manual") {
		flags.lifetime = State::StreamFlags::kIndependent;
	} else if (lifetime == "withScene") {
		flags.lifetime = State::StreamFlags::kEndsWithParentScene;
	} else if (lifetime == "withStream") {
		flags.lifetime = State::StreamFlags::kEndsWithParent;
	} else {
		return luaL_argerror(L, 5, "parameter must be \"manual\", \"withScene\" or \"withStream\"");
	}

	luaL_checktype(L, 6, LUA_TBOOLEAN);
	luaL_checktype(L, 7, LUA_TBOOLEAN);
	flags.capturesInput = lua_toboolean(L, 6) != 0;
	flags.runsFirst = lua_toboolean(L, 7) != 0;

	State::SceneStreamManager &streams = NancySceneState.getStreams();
	const bool isNew = !streams.isRunning(name);
	streams.begin(name, sceneName, (uint16)frame, flags);

	lua_pushboolean(L, isNew);
	return 1;
}

// Scene:EndStream(name)
static int sceneEndStream(lua_State *L) {
	checkContext(L);
	const char *name = luaL_checkstring(L, 2);

	State::SceneStreamManager &streams = NancySceneState.getStreams();
	const bool wasRunning = streams.isRunning(name);
	streams.end(name);

	lua_pushboolean(L, wasRunning);
	return 1;
}

// What "for key, ar in Scene:ARs(direction)" calls for every round: gives the
// next record, and its place in the order asked for
static int sceneNextRecord(lua_State *L) {
	SceneContext *context = checkContext(L);
	const bool isForward = lua_toboolean(L, lua_upvalueindex(1)) != 0;
	const uint numDone = (uint)lua_tointeger(L, lua_upvalueindex(2));

	const Common::Array<Record *> &records = context->getRecords();
	if (numDone >= records.size()) {
		return 0;
	}

	lua_pushinteger(L, numDone + 1);
	lua_replace(L, lua_upvalueindex(2));

	lua_pushinteger(L, numDone + 1);
	records[isForward ? numDone : records.size() - 1 - numDone]->push();
	return 2;
}

// Scene:ARs(direction), which goes over the records of the scene, in the order
// they were made in ("forward") or the other way around ("backward")
static int sceneRecords(lua_State *L) {
	checkContext(L);
	const Common::String direction = luaL_checkstring(L, 2);

	if (direction != "forward" && direction != "backward") {
		return luaL_error(L, "Iteration direction (must be forward or backward) is %s", direction.c_str());
	}

	lua_pushboolean(L, direction == "forward");
	lua_pushinteger(L, 0);
	lua_pushcclosure(L, sceneNextRecord, 2);
	return 1;
}

static const luaL_Reg kSceneFunctions[] = {
	{ "__index", sceneIndex },
	{ "__newindex", sceneNewIndex },
	{ "ARs", sceneRecords },
	{ "Include", sceneInclude },
	{ "BeginStream", sceneBeginStream },
	{ "EndStream", sceneEndStream },
	{ nullptr, nullptr }
};

// Player, who is the character being played unless a function is told otherwise
// TODO: AddTask, LoadTaskFlag, AddJournalEntry and AddCellPhoneNumber

// The number of a character, or -1 when there is none of that name
static int findCharacter(const char *name) {
	const PCUI *players = GetEngineData(PCUI);
	if (players) {
		for (uint i = 0; i < players->characters.size(); ++i) {
			if (players->characters[i].name.equalsIgnoreCase(name)) {
				return i;
			}
		}
	}

	return -1;
}

// The character named by an optional argument
static uint checkCharacter(lua_State *L, int index) {
	if (lua_gettop(L) < index) {
		return g_nancy->getPlayerCharacter();
	}

	const char *name = luaL_checkstring(L, index);
	const int character = findCharacter(name);
	if (character < 0) {
		luaL_error(L, "Invalid player %s", name);
	}

	return character;
}

// Player:Switch(character), which hands the game over to another character.
// Nothing happens when they are the one being played, or when there is no such
// character.
static int playerSwitch(lua_State *L) {
	const int character = findCharacter(luaL_checkstring(L, 2));
	if (character >= 0) {
		NancySceneState.changePlayerCharacter(character);
	}

	return 0;
}

// Player:HasInventoryItem(item[, character])
static int playerHasInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	const uint character = checkCharacter(L, 3);

	lua_pushboolean(L, NancySceneState.hasCharacterItem(character, itemID) == g_nancy->_true);
	return 1;
}

// Player:AddInventoryItem(item, intoHand[, character])
static int playerAddInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	luaL_checktype(L, 3, LUA_TBOOLEAN);
	const bool intoHand = lua_toboolean(L, 3) != 0;
	const uint character = checkCharacter(L, 4);

	NancySceneState.giveItemToCharacter(character, itemID, intoHand, false);
	return 0;
}

// Player:RemoveInventoryItem(item[, character])
static int playerRemoveInventoryItem(lua_State *L) {
	const int16 itemID = checkInventoryItem(L, 2);
	const uint character = checkCharacter(L, 3);

	NancySceneState.removeItemFromCharacterInventory(character, itemID);
	return 0;
}

static const luaL_Reg kPlayerFunctions[] = {
	{ "__index", indexFunctions },
	{ "Switch", playerSwitch },
	{ "HasInventoryItem", playerHasInventoryItem },
	{ "AddInventoryItem", playerAddInventoryItem },
	{ "RemoveInventoryItem", playerRemoveInventoryItem },
	{ nullptr, nullptr }
};

// Viewport

static int viewportIndex(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "size") {
		const VIEW *viewport = GetEngineData(VIEW);
		assert(viewport);
		pushRect(L, ScriptRect(0, 0, viewport->screenPosition.width(), viewport->screenPosition.height()));
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

static const luaL_Reg kViewportFunctions[] = {
	{ "__index", viewportIndex },
	{ nullptr, nullptr }
};

// Log:Write(text)
static int logWrite(lua_State *L) {
	debugC(1, kDebugScene, "Script: %s", luaL_checkstring(L, 2));
	return 0;
}

static const luaL_Reg kLogFunctions[] = {
	{ "__index", indexFunctions },
	{ "Write", logWrite },
	{ nullptr, nullptr }
};

// TODO: The Autotext, Streams, Generics and UIMgr objects, and Log:WriteAll
void registerGameAPI(lua_State *L) {
	registerGlobal(L, "Flags", "ND.Flags", kFlagsFunctions);
	registerGlobal(L, "VarTable", "ND.VarTable", kVarTableFunctions);

	registerGlobal(L, "Z", "ND.ZHelper", kZFunctions);
	luaL_getmetatable(L, "ND.ZHelper");
	lua_pushnumber(L, 0.0);
	lua_setfield(L, -2, "middle");
	lua_pop(L, 1);

	registerType(L, kCursorIndexType, kCursorIndexFunctions);
	registerGlobal(L, "Cursor", "ND.Cursor", kCursorFunctions);

	registerType(L, kSoundChannelType, kSoundChannelFunctions);
	registerGlobal(L, "SoundChannel", "ND.SoundHelper", kSoundChannelHelperFunctions);
	registerGlobal(L, "Sound", "ND.Sound", kSoundFunctions);

	registerGlobal(L, "Scene", "ND.Scene", kSceneFunctions);
	registerGlobal(L, "Player", "ND.CurPlayer", kPlayerFunctions);
	registerGlobal(L, "Viewport", "ND.Viewport", kViewportFunctions);

	registerType(L, kKeyType, kKeyFunctions);
	registerGlobal(L, "Key", "ND.KeyHelper", kKeyHelperFunctions);

	registerGlobal(L, "Game", "ND.Game", kGameFunctions);
	registerGlobal(L, "Draw", "ND.Draw", kDrawFunctions);
	registerGlobal(L, "Log", "ND.Log", kLogFunctions);
}

} // End of namespace Script
} // End of namespace Nancy

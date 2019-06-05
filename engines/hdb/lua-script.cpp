/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"
#include "common/debug.h"

#include "hdb/lua-script.h"

namespace HDB {

LuaScript::LuaScript() {
	_state = NULL;
	_systemInit = false;
}

LuaScript::~LuaScript() {
	if (_state) {
		lua_close(_state);
	}
}


bool LuaScript::init() {
	// Load Global Lua Code
	_globalLuaStream = g_hdb->fileMan->findFirstData("GLOBAL_LUA", TYPE_BINARY);
	_globalLuaLength = g_hdb->fileMan->getLength("GLOBAL_LUA", TYPE_BINARY);
	if (_globalLuaStream == NULL || _globalLuaLength == 0) {
		error("LuaScript::initScript: 'global code' failed to load");
		return false;
	}

	return true;
}


/*
	Called from Lua, this will pop into the menu
*/

static int cineStart(lua_State *L) {
	warning("STUB: START CINE");
	return 0;
}

static int cineStop(lua_State *L) {
	warning("STUB: STOP CINE");
	return 0;
}

static int cineFadeInBlack(lua_State *L) {
	warning("STUB: CINE FADE IN BLACK");
	return 0;
}

static int cineFadeOutBlack(lua_State *L) {
	warning("STUB: CINE FADE OUT BLACK");
	return 0;
}

static int cineFadeInWhite(lua_State *L) {
	warning("STUB: CINE FADE IN WHITE");
	return 0;
}

static int cineFadeOutWhite(lua_State *L) {
	warning("STUB: CINE FADE OUT WHITE");
	return 0;
}

static int cineStartMap(lua_State *L) {
	warning("STUB: CINE START MAP");
	return 0;
}

static int cineLockPlayer(lua_State *L) {
	warning("STUB: CINE LOCK PLAYER");
	return 0;
}

static int cineUnlockPlayer(lua_State *L) {
	warning("STUB: CINE UNLOCK PLAYER");
	return 0;
}

static int cineSetCamera(lua_State *L) {
	warning("STUB: CINE SET CAMERA");
	return 0;
}

static int cineResetCamera(lua_State *L) {
	warning("STUB: CINE RESET CAMERA");
	return 0;
}

static int cineMoveCamera(lua_State *L) {
	warning("STUB: CINE MOVE CAMERA");
	return 0;
}

static int cineWait(lua_State *L) {
	warning("STUB: CINE WAIT");
	return 0;
}

static int cineWaitUntilDone(lua_State *L) {
	warning("STUB: CINE WAIT UNTIL DONE");
	return 0;
}

static int cinePlaySound(lua_State *L) {
	warning("STUB: CINE PLAY SOUND");
	return 0;
}

static int cinePlayVoice(lua_State *L) {
	warning("STUB: CINE PLAY VOICE");
	return 0;
}

static int cineUseEntity(lua_State *L) {
	warning("STUB: CINE USE ENTITY");
	return 0;
}

static int cineSetEntity(lua_State *L) {
	warning("STUB: CINE SET ENTITY");
	return 0;
}

static int cineRemoveEntity(lua_State *L) {
	warning("STUB: CINE REMOVE ENTITY");
	return 0;
}

static int cineMoveEntity(lua_State *L) {
	warning("STUB: CINE MOVE ENTITY");
	return 0;
}

static int cineAnimEntity(lua_State *L) {
	warning("STUB: CINE ANIM ENTITY");
	return 0;
}

static int cineSetAnimFrame(lua_State *L) {
	warning("STUB: CINE SET ANIM FRAME");
	return 0;
}

static int cineDialog(lua_State *L) {
	warning("STUB: CINE DIALOG");
	return 0;
}

static int cineDrawPic(lua_State *L) {
	warning("STUB: CINE DRAW PIC");
	return 0;
}

static int cineDrawMaskedPic(lua_State *L) {
	warning("STUB: CINE DRAW MASKED PIC");
	return 0;
}

static int cineMovePic(lua_State *L) {
	warning("STUB: CINE MOVE PIC");
	return 0;
}

static int cineMoveMaskedPic(lua_State *L) {
	warning("STUB: CINE MOVE MASKED PIC");
	return 0;
}

static int cineSpawnEntity(lua_State *L) {
	warning("STUB: CINE SPAWN ENTITY");
	return 0;
}

static int cineClearForeground(lua_State *L) {
	warning("STUB: CINE CLEAR FOREGROUND");
	return 0;
}

static int cineSetForeground(lua_State *L) {
	warning("STUB: CINE SET FOREGROUND");
	return 0;
}

static int cineSetBackground(lua_State *L) {
	warning("STUB: CINE SET BACKGROUND");
	return 0;
}

static int cineFunction(lua_State *L) {
	warning("STUB: CINE FUNCTION");
	return 0;
}

static int cineEntityFace(lua_State *L) {
	warning("STUB: CINE ENTITY FACE");
	return 0;
}

static int cineTextOut(lua_State *L) {
	warning("STUB: CINE TEXT OUT");
	return 0;
}

static int cineCenterTextOut(lua_State *L) {
	warning("STUB: CINE CENTER TEXT OUT");
	return 0;
}

static int newDelivery(lua_State *L) {
	warning("STUB: NEW DELIVERY");
	return 0;
}

static int completeDelivery(lua_State *L) {
	warning("STUB: COMPLETE DELIVERY");
	return 0;
}

static int deliveriesLeft(lua_State *L) {
	warning("STUB: DELIVERIES LEFT");
	return 0;
}

static int getEntityXY(lua_State *L) {
	warning("STUB: GET ENTITYXY");
	return 0;
}

static int setEntity(lua_State *L) {
	warning("STUB: SET ENTITY");
	return 0;
}

static int setEntDir(lua_State *L) {
	warning("STUB: SET ENTITY DIR");
	return 0;
}

static int removeEntity(lua_State *L) {
	warning("STUB: REMOVE ENTITY");
	return 0;
}

static int animEntity(lua_State *L) {
	warning("STUB: ANIM ENTITY");
	return 0;
}

static int setAnimFrame(lua_State *L) {
	warning("STUB: SET ANIM FRAME");
	return 0;
}

static int useEntity(lua_State *L) {
	warning("STUB: USE ENTITY");
	return 0;
}

static int entityFace(lua_State *L) {
	warning("STUB: ENTITY FACE");
	return 0;
}

static int clearForeground(lua_State *L) {
	warning("STUB: CLEAR FOREGROUND");
	return 0;
}

static int setForeground(lua_State *L) {
	warning("STUB: SET FOREGROUND");
	return 0;
}

static int setBackground(lua_State *L) {
	warning("STUB: CLEAR BACKGROUND");
	return 0;
}

static int dialog(lua_State *L) {
	warning("STUB: DIALOG");
	return 0;
}

static int dialogChoice(lua_State *L) {
	warning("STUB: DIALOG CHOICE");
	return 0;
}

static int message(lua_State *L) {
	warning("STUB: MESSAGE");
	return 0;
}

static int animation(lua_State *L) {
	warning("STUB: ANIMATION");
	return 0;
}

static int spawnEntity(lua_State *L) {
	warning("STUB: SPAWN ENTITY");
	return 0;
}

static int addInvItem(lua_State *L) {
	warning("STUB: ADD INVENTORY ITEM");
	return 0;
}

static int keepInvItem(lua_State *L) {
	warning("STUB: KEEP INVENTORY ITEM");
	return 0;
}

static int queryInv(lua_State *L) {
	warning("STUB: QUERY INVENTORY");
	return 0;
}

static int purgeInv(lua_State *L) {
	warning("STUB: PURGE INVENTORY");
	return 0;
}

static int queryInvItem(lua_State *L) {
	warning("STUB: QUERY INVENTORY ITEM");
	return 0;
}

static int removeInv(lua_State *L) {
	warning("STUB: REMOVE INVENTORY");
	return 0;
}

static int removeInvItem(lua_State *L) {
	warning("STUB: REMOVE INVENTORY ITEM");
	return 0;
}

static int killTrigger(lua_State *L) {
	warning("STUB: KILL TRIGGER");
	return 0;
}

static int startMusic(lua_State *L) {
	warning("STUB: START MUSIC");
	return 0;
}

static int fadeInMusic(lua_State *L) {
	warning("STUB: FADE IN MUSIC");
	return 0;
}

static int stopMusic(lua_State *L) {
	warning("STUB: STOP MUSIC");
	return 0;
}

static int fadeOutMusic(lua_State *L) {
	warning("STUB: FADE OUT MUSIC");
	return 0;
}

static int registerSound(lua_State *L) {
	warning("STUB: REGISTER SOUND");
	return 0;
}

static int playSound(lua_State *L) {
	warning("STUB: PLAY SOUND");
	return 0;
}

static int freeSound(lua_State *L) {
	warning("STUB: FREE SOUND");
	return 0;
}

static int startMap(lua_State *L) {
	warning("STUB: START MAP");
	return 0;
}

static int saveGlobal(lua_State *L) {
	warning("STUB: SAVE GLOBAL");
	return 0;
}

static int loadGlobal(lua_State *L) {
	warning("STUB: LOAD GLOBAL");
	return 0;
}

static int purgeGlobals(lua_State *L) {
	warning("STUB: PURGE GLOBALS");
	return 0;
}

static int textOut(lua_State *L) {
	warning("STUB: TEXT OUT");
	return 0;
}

static int centerTextOut(lua_State *L) {
	warning("STUB: CENTER TEXT OUT");
	return 0;
}

static int turnOnSnow(lua_State *L) {
	warning("STUB: TURN ON SNOW");
	return 0;
}

static int turnOffSnow(lua_State *L) {
	warning("STUB: TURN OFF SNOW");
	return 0;
}

static int gotoMenu(lua_State *L) {
	g_hdb->changeGameState();
	return 0;
}

static int setInfobarDark(lua_State *L) {
	warning("STUB: SET INFOBAR DARK");
	return 0;
}

static int setPointerState(lua_State *L) {
	warning("STUB: SET POINTER STATE");
	return 0;
}

static int playVoice(lua_State *L) {
	warning("STUB: PLAY VOICE");
	return 0;
}

/*
	Lua Initialization Code
*/

struct VarInit {
	char *realName;
	char *luaName;
} luaGlobalStrings[] = {
	{ "Map00",	"MAP00"},
	{NULL, NULL}
};

// For AI States, to be implemented
struct NumberInit {
	int value;
	char *luaName;
} luaGlobalValues[] = {
	{NULL, NULL}
};

struct FuncInit {
	char *luaName;
	int (*function) (lua_State *L);
} luaFuncs[] = {
	{  "Dialog",				dialog				},
	{  "DialogChoice",			dialogChoice		},
	{  "Message",				message				},
	{  "QueryInv",				queryInv			},
	{  "QueryInvItem",			queryInvItem		},
	{  "RemoveInv",				removeInv			},
	{  "RemoveInvItem",			removeInvItem		},
	{  "AddInvItem",			addInvItem			},
	{  "KeepInvItem",			keepInvItem			},
	{  "PurgeInv",				purgeInv			},
	{  "StartMusic",			startMusic			},
	{  "StopMusic",				stopMusic			},
	{  "FadeInMusic",			fadeInMusic			},
	{  "FadeOutMusic",			fadeOutMusic		},
	{  "RegisterSound",			registerSound		},
	{  "PlaySound",				playSound			},
	{  "FreeSound",				freeSound			},
	{  "StartMap",				startMap			},
	{  "Tile_ClearFG",			clearForeground		},
	{  "Tile_SetFG",			setForeground		},
	{  "Tile_SetBG",			setBackground		},
	{  "GetEntityXY",			getEntityXY			},
	{  "UseEntity",				useEntity			},
	{  "SetEntity",				setEntity			},
	{  "SetEntDir",				setEntDir			},
	{  "RemoveEntity",			removeEntity		},
	{  "AnimEntity",			animEntity			},
	{  "SetAnimFrame",			setAnimFrame		},
	{  "EntityFace",			entityFace			},
	{  "KillTrigger",			killTrigger			},
	{  "SpawnEntity",			spawnEntity			},
	{  "Animation",				animation			},
	{  "NewDelivery",			newDelivery			},
	{  "CompleteDelivery",		completeDelivery	},
	{  "DeliveriesLeft",		deliveriesLeft		},
	{  "SaveGlobal",			saveGlobal			},
	{  "LoadGlobal",			loadGlobal			},
	{  "PurgeGlobals",			purgeGlobals		},
	{  "TextOut",				textOut				},
	{  "CenterTextOut",			centerTextOut		},
	{  "SnowOn",				turnOnSnow			},
	{  "SnowOff",				turnOffSnow			},
	{  "GotoMenu",				gotoMenu			},
	{  "SetInfobarDark",		setInfobarDark		},
	{  "SetPointerState",		setPointerState		},
	{  "PlayVoice",				playVoice			},

	{  "Cine_StartCine",		cineStart			},
	{  "Cine_StopCine",			cineStop			},
	{  "Cine_StartMap",			cineStartMap		},
	{  "Cine_LockPlayer",		cineLockPlayer		},
	{  "Cine_UnlockPlayer",		cineUnlockPlayer	},
	{  "Cine_SetCamera",		cineSetCamera		},
	{  "Cine_ResetCamera",		cineResetCamera		},
	{  "Cine_MoveCamera",		cineMoveCamera		},
	{  "Cine_Wait",				cineWait			},
	{  "Cine_WaitUntilDone",	cineWaitUntilDone	},
	{  "Cine_UseEntity",		cineUseEntity		},
	{  "Cine_SetEntity",		cineSetEntity		},
	{  "Cine_RemoveEntity",		cineRemoveEntity	},
	{  "Cine_EntityFace",		cineEntityFace		},
	{  "Cine_MoveEntity",		cineMoveEntity		},
	{  "Cine_AnimEntity",		cineAnimEntity		},
	{  "Cine_SetAnimFrame",		cineSetAnimFrame	},
	{  "Cine_Dialog",			cineDialog			},
	{  "Cine_DrawPic",			cineDrawPic			},
	{  "Cine_DrawMaskedPic",	cineDrawMaskedPic	},
	{  "Cine_MovePic",			cineMovePic			},
	{  "Cine_MoveMaskedPic",	cineMoveMaskedPic	},
	{  "Cine_FadeOutBlack",		cineFadeOutBlack	},
	{  "Cine_FadeInBlack",		cineFadeInBlack		},
	{  "Cine_FadeOutWhite",		cineFadeOutWhite	},
	{  "Cine_FadeInWhite",		cineFadeInWhite		},
	{  "Cine_SpawnEntity",		cineSpawnEntity		},
	{  "Cine_PlaySound",		cinePlaySound		},
	{  "Cine_Tile_ClearFG",		cineClearForeground },
	{  "Cine_Tile_SetFG",		cineSetForeground	},
	{  "Cine_Tile_SetBG",		cineSetBackground	},
	{  "Cine_Function",			cineFunction		},
	{  "Cine_TextOut",			cineTextOut			},
	{  "Cine_CenterTextOut",	cineCenterTextOut	},
	{  "Cine_PlayVoice",		cinePlayVoice		},
	{ NULL, NULL }
};

bool LuaScript::initScript(Common::SeekableReadStream *stream, int32 length) {

	if (_systemInit) {
		return false;
	}

	// Initialize Lua Environment
	_state = lua_open();
	if (_state == NULL) {
		error("Couldn't initialize Lua script.");
		return false;
	}
	luaL_openlibs(_state);

	_systemInit = true;

	// Register Extensions
	for (int i = 0; luaFuncs[i].luaName; i++) {
		lua_register(_state, luaFuncs[i].luaName, luaFuncs[i].function);
	}

	// Register Lua Globals

	for (int i = 0; luaGlobalStrings[i].realName; i++) {
		lua_pushstring(_state, luaGlobalStrings[i].realName);
		lua_setglobal(_state, luaGlobalStrings[i].luaName);
	}

	for (int j = 0; luaGlobalValues[j].luaName; j++) {
		lua_pushnumber(_state, luaGlobalValues[j].value);
		lua_setglobal(_state, luaGlobalValues[j].luaName);
	}

	/*
		TODO: Set the last mapName as a global
		after implementing the map-manager.
	*/

	// Set the lowest printable line
	lua_pushnumber(_state, 480 - 14);
	lua_setglobal(_state, "BOTTOM_Y");

	/*
		TODO: Load the sound names and entity
		spawn names into Lua once they are implemented.
	*/

	// Load GLOBAL_LUA and execute it

	if (!executeMPC(_globalLuaStream, "global code", _globalLuaLength)) {
		error("LuaScript::initScript: 'global code' failed to execute");
		return false;
	}

	// Load script and execute it

	if (!executeMPC(stream, "level code", length)) {
		error("LuaScript::initScript: 'level code' failed to execute");
		return false;
	}

	lua_getglobal(_state, "level_init");
	lua_pcall(_state, 0, 0, 0);

	return true;
}

bool LuaScript::executeMPC(Common::SeekableReadStream *stream, const char *name, int32 length) {

	if (!_systemInit) {
		return false;
	}

	char *chunk = new char[length];
	stream->read((void *)chunk, length);

	/*
		Remove C-style comments from the script
		and update the upvalue syntax for Lua 5.1.3
	*/
	sanitizeScript(chunk);

	if (!executeChunk(chunk, length, name)) {
		delete[] chunk;

		return false;
	}

	delete[] chunk;

	return true;
}

#if 0
bool LuaScript::executeFile(const Common::String &filename) {

	if (!_systemInit) {
		return false;
	}

	Common::File *file = new Common::File;

	if (!file->open(filename)) {
		error("Cannot find \"%s\"", filename.c_str());
	}

	uint fileSize = file->size();
	const char *fileData = new char[fileSize];
	file->read((void *)fileData, fileSize);

	if (!executeChunk(fileData, fileSize, filename)) {
		delete[] fileData;
		delete file;

		return false;
	}

	delete[] fileData;
	delete file;

	return true;
}
#endif

namespace {
int panicCB(lua_State *L) {
        error("Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
        return 0;
}

void debugHook(lua_State *L, lua_Debug *ar) {
        if (!lua_getinfo(L, "Sn", ar))
                return;

        debug("LUA: %s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline);
}
}

bool LuaScript::executeChunk(const char *chunk, uint chunkSize, const Common::String &chunkName) const {

	if (!_systemInit) {
		return false;
	}

	lua_atpanic(_state, panicCB);

	const char errorHandlerCode[] =
		"local function ErrorHandler(message) "
		"   return message .. '\\n' .. debug.traceback('', 2) "
		"end "
		"return ErrorHandler";

	if (luaL_loadbuffer(_state, errorHandlerCode, strlen(errorHandlerCode), "PCALL ERRORHANDLER") != 0) {
                // An error occurred, so dislay the reason and exit
                error("Couldn't compile luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
                lua_pop(_state, 1);

                return false;
        }


		// Running the code, the error handler function sets the top of the stack
	        if (lua_pcall(_state, 0, 1, 0) != 0) {
	                // An error occurred, so dislay the reason and exit
	                error("Couldn't prepare luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
	                lua_pop(_state, 1);

	                return false;
	        }

	lua_sethook(_state, debugHook, LUA_MASKCALL | LUA_MASKLINE, 0);

	// Compile Chunk
	if (luaL_loadbuffer(_state, chunk, chunkSize, chunkName.c_str())) {
		error("Couldn't compile \"%s\": %s", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	// Execute Chunk
	if (lua_pcall(_state, 0, 0, 0)) {
		error("An error occured while executing \"%s\": %s.", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	return true;
}

void LuaScript::sanitizeScript(char *chunk) {
	uint32 offset = 0;

	while (chunk[offset]) {
		// Strip C-Style comments
		if (chunk[offset] == '/' && chunk[offset + 1] == '/') {
			while (chunk[offset] != 0x0d) {
				chunk[offset++] = ' ';
			}
		} else if (chunk[offset] == '%' && chunk[offset] != ' ') { // Update the Upvalue syntax
			chunk[offset] = ' ';
		}
		offset++;
	}
}
}

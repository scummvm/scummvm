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

#include "ags/globals.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/debugging/debugmanager.h"
#include "ags/shared/game/roomstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/dynobj/cc_dialog.h"
#include "ags/engine/ac/dynobj/cc_guiobject.h"
#include "ags/engine/ac/dynobj/cc_character.h"
#include "ags/engine/ac/dynobj/cc_hotspot.h"
#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/ac/dynobj/cc_inventory.h"
#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/cc_object.h"
#include "ags/engine/ac/dynobj/cc_audiochannel.h"
#include "ags/engine/ac/dynobj/cc_audioclip.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/dynobj/scripthotspot.h"
#include "ags/engine/ac/dynobj/scriptinvitem.h"
#include "ags/engine/ac/dynobj/scriptobject.h"
#include "ags/engine/ac/dynobj/scriptregion.h"
#include "ags/engine/ac/dynobj/scriptstring.h"

namespace AGS3 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;

	Common::fill(&_mousecurs[0], &_mousecurs[MAXCURSORS], nullptr);

	// debugmanager.cpp globals
	_DbgMgr = new AGS::Shared::DebugManager();

	// game.cpp globals
	_ccDynamicGUIObject = new CCGUIObject();
	_ccDynamicCharacter = new CCCharacter();
	_ccDynamicHotspot = new CCHotspot();
	_ccDynamicRegion = new CCRegion();
	_ccDynamicInv = new CCInventory();
	_ccDynamicGUI = new CCGUI();
	_ccDynamicObject = new CCObject();
	_ccDynamicDialog = new CCDialog();
	_ccDynamicAudioClip = new CCAudioClip();
	_ccDynamicAudio = new CCAudioChannel();
	_myScriptStringImpl = new ScriptString();
	_guis = new std::vector<AGS::Shared::GUIMain>();
	_play = new GameState();
	_game = new GameSetupStruct();
	_spriteset = new SpriteCache(_game->SpriteInfos);
	_thisroom = new AGS::Shared::RoomStruct();
	_troom = new RoomStatus();
	_scrObj = new ScriptObject[MAX_ROOM_OBJECTS];
	_scrHotspot = new ScriptHotspot[MAX_ROOM_HOTSPOTS];
	_scrRegion = new ScriptRegion[MAX_ROOM_REGIONS];
	_scrInv = new ScriptInvItem[MAX_INV];
	_objcache = new ObjectCache[MAX_ROOM_OBJECTS];
}

Globals::~Globals() {
	g_globals = nullptr;

	delete _DbgMgr;

	delete _ccDynamicGUIObject;
	delete _ccDynamicCharacter;
	delete _ccDynamicHotspot;
	delete _ccDynamicRegion;
	delete _ccDynamicInv;
	delete _ccDynamicGUI;
	delete _ccDynamicObject;
	delete _ccDynamicDialog;
	delete _ccDynamicAudioClip;
	delete _ccDynamicAudio;
	delete _myScriptStringImpl;
	delete _guis;
	delete _game;
	delete _play;
	delete _spriteset;
	delete _thisroom;
	delete _troom;
	delete[] _scrObj;
	delete[] _scrHotspot;
	delete[] _scrRegion;
	delete[] _scrInv;
	delete[] _objcache;
}

} // namespace AGS3

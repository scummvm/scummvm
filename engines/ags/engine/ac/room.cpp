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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string_utils.h" //strlwr()
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/charactercache.h"
#include "ags/engine/ac/characterextras.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/movelist.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/objectcache.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/region.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/roomobject.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/walkablearea.h"
#include "ags/engine/ac/walkbehind.h"
#include "ags/engine/ac/dynobj/scriptobject.h"
#include "ags/engine/ac/dynobj/scripthotspot.h"
#include "ags/shared/gui/guidefines.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/game/room_version.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/plugins/agsplugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/script/cc_error.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/engine/ac/dynobj/all_dynamicclasses.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/util/math.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/scriptstring.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

ScriptDrawingSurface *Room_GetDrawingSurfaceForBackground(int backgroundNumber) {
	if (_G(displayed_room) < 0)
		quit("!Room.GetDrawingSurfaceForBackground: no room is currently loaded");

	if (backgroundNumber == SCR_NO_VALUE) {
		backgroundNumber = _GP(play).bg_frame;
	}

	if ((backgroundNumber < 0) || ((size_t)backgroundNumber >= _GP(thisroom).BgFrameCount))
		quit("!Room.GetDrawingSurfaceForBackground: invalid background number specified");


	ScriptDrawingSurface *surface = new ScriptDrawingSurface();
	surface->roomBackgroundNumber = backgroundNumber;
	ccRegisterManagedObject(surface, surface);
	return surface;
}

ScriptDrawingSurface *Room_GetDrawingSurfaceForMask(RoomAreaMask mask) {
	if (_G(displayed_room) < 0)
		quit("!Room_GetDrawingSurfaceForMask: no room is currently loaded");
	ScriptDrawingSurface *surface = new ScriptDrawingSurface();
	surface->roomMaskType = mask;
	ccRegisterManagedObject(surface, surface);
	return surface;
}

int Room_GetObjectCount() {
	return _G(croom)->numobj;
}

int Room_GetWidth() {
	return _GP(thisroom).Width;
}

int Room_GetHeight() {
	return _GP(thisroom).Height;
}

int Room_GetColorDepth() {
	return _GP(thisroom).BgFrames[0].Graphic->GetColorDepth();
}

int Room_GetLeftEdge() {
	return _GP(thisroom).Edges.Left;
}

int Room_GetRightEdge() {
	return _GP(thisroom).Edges.Right;
}

int Room_GetTopEdge() {
	return _GP(thisroom).Edges.Top;
}

int Room_GetBottomEdge() {
	return _GP(thisroom).Edges.Bottom;
}

int Room_GetMusicOnLoad() {
	return _GP(thisroom).Options.StartupMusic;
}

int Room_GetProperty(const char *property) {
	return get_int_property(_GP(thisroom).Properties, _G(croom)->roomProps, property);
}

const char *Room_GetTextProperty(const char *property) {
	return get_text_property_dynamic_string(_GP(thisroom).Properties, _G(croom)->roomProps, property);
}

bool Room_SetProperty(const char *property, int value) {
	return set_int_property(_G(croom)->roomProps, property, value);
}

bool Room_SetTextProperty(const char *property, const char *value) {
	return set_text_property(_G(croom)->roomProps, property, value);
}

const char *Room_GetMessages(int index) {
	if ((index < 0) || ((size_t)index >= _GP(thisroom).MessageCount)) {
		return nullptr;
	}
	char buffer[STD_BUFFER_SIZE];
	buffer[0] = 0;
	replace_tokens(get_translation(_GP(thisroom).Messages[index]), buffer, STD_BUFFER_SIZE);
	return CreateNewScriptString(buffer);
}


//=============================================================================

// Makes sure that room background and walk-behind mask are matching room size
// in game resolution coordinates; in other words makes graphics appropriate
// for display in the game.
void convert_room_background_to_game_res() {
	if (!_GP(game).AllowRelativeRes() || !_GP(thisroom).IsRelativeRes())
		return;

	int bkg_width = _GP(thisroom).Width;
	int bkg_height = _GP(thisroom).Height;
	data_to_game_coords(&bkg_width, &bkg_height);

	for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i)
		_GP(thisroom).BgFrames[i].Graphic = FixBitmap(_GP(thisroom).BgFrames[i].Graphic, bkg_width, bkg_height);

	// Fix walk-behinds to match room background
	// TODO: would not we need to do similar to each mask if they were 1:1 in hires room?
	_GP(thisroom).WalkBehindMask = FixBitmap(_GP(thisroom).WalkBehindMask, bkg_width, bkg_height);
}


void save_room_data_segment() {
	_G(croom)->FreeScriptData();

	_G(croom)->tsdatasize = _G(roominst)->globaldatasize;
	if (_G(croom)->tsdatasize > 0) {
		_G(croom)->tsdata = (char *)malloc(_G(croom)->tsdatasize + 10);
		memcpy(_G(croom)->tsdata, &_G(roominst)->globaldata[0], _G(croom)->tsdatasize);
	}

}

void unload_old_room() {
	int ff;

	// if switching games on restore, don't do this
	if (_G(displayed_room) < 0)
		return;

	debug_script_log("Unloading room %d", _G(displayed_room));

	current_fade_out_effect();

	dispose_room_drawdata();

	for (ff = 0; ff < _G(croom)->numobj; ff++)
		_G(objs)[ff].moving = 0;

	if (!_GP(play).ambient_sounds_persist) {
		for (ff = 1; ff < MAX_SOUND_CHANNELS; ff++)
			StopAmbientSound(ff);
	}

	cancel_all_scripts();
	_G(numevents) = 0;  // cancel any pending room events

	if (_G(roomBackgroundBmp) != nullptr) {
		_G(gfxDriver)->DestroyDDB(_G(roomBackgroundBmp));
		_G(roomBackgroundBmp) = nullptr;
	}

	if (_G(croom) == nullptr) ;
	else if (_G(roominst) != nullptr) {
		save_room_data_segment();
		delete _G(roominstFork);
		delete _G(roominst);
		_G(roominstFork) = nullptr;
		_G(roominst) = nullptr;
	} else _G(croom)->tsdatasize = 0;
	memset(&_GP(play).walkable_areas_on[0], 1, MAX_WALK_AREAS + 1);
	_GP(play).bg_frame = 0;
	_GP(play).bg_frame_locked = 0;
	remove_screen_overlay(-1);
	delete _G(raw_saved_screen);
	_G(raw_saved_screen) = nullptr;
	for (ff = 0; ff < MAX_ROOM_BGFRAMES; ff++)
		_GP(play).raw_modified[ff] = 0;
	for (size_t i = 0; i < _GP(thisroom).LocalVariables.size() && i < MAX_GLOBAL_VARIABLES; ++i)
		_G(croom)->interactionVariableValues[i] = _GP(thisroom).LocalVariables[i].Value;

	// wipe the character cache when we change rooms
	for (ff = 0; ff < _GP(game).numcharacters; ff++) {
		if (_G(charcache)[ff].inUse) {
			delete _G(charcache)[ff].image;
			_G(charcache)[ff].image = nullptr;
			_G(charcache)[ff].inUse = 0;
		}
		// ensure that any half-moves (eg. with scaled movement) are stopped
		_G(charextra)[ff].xwas = INVALID_X;
	}

	_GP(play).swap_portrait_lastchar = -1;
	_GP(play).swap_portrait_lastlastchar = -1;

	for (ff = 0; ff < _G(croom)->numobj; ff++) {
		// un-export the object's script object
		if (_G(objectScriptObjNames)[ff].IsEmpty())
			continue;

		ccRemoveExternalSymbol(_G(objectScriptObjNames)[ff]);
	}

	for (ff = 0; ff < MAX_ROOM_HOTSPOTS; ff++) {
		if (_GP(thisroom).Hotspots[ff].ScriptName.IsEmpty())
			continue;

		ccRemoveExternalSymbol(_GP(thisroom).Hotspots[ff].ScriptName);
	}

	croom_ptr_clear();

	// clear the object cache
	for (ff = 0; ff < MAX_ROOM_OBJECTS; ff++) {
		delete _G(objcache)[ff].image;
		_G(objcache)[ff].image = nullptr;
	}
	// clear the _G(actsps) buffers to save memory, since the
	// objects/characters involved probably aren't on the
	// new screen. this also ensures all cached data is flushed
	for (ff = 0; ff < MAX_ROOM_OBJECTS + _GP(game).numcharacters; ff++) {
		delete _G(actsps)[ff];
		_G(actsps)[ff] = nullptr;

		if (_G(actspsbmp)[ff] != nullptr)
			_G(gfxDriver)->DestroyDDB(_G(actspsbmp)[ff]);
		_G(actspsbmp)[ff] = nullptr;

		delete _G(actspswb)[ff];
		_G(actspswb)[ff] = nullptr;

		if (_G(actspswbbmp)[ff] != nullptr)
			_G(gfxDriver)->DestroyDDB(_G(actspswbbmp)[ff]);
		_G(actspswbbmp)[ff] = nullptr;

		_G(actspswbcache)[ff].valid = 0;
	}

	// if Hide Player Character was ticked, restore it to visible
	if (_GP(play).temporarily_turned_off_character >= 0) {
		_GP(game).chars[_GP(play).temporarily_turned_off_character].on = 1;
		_GP(play).temporarily_turned_off_character = -1;
	}

}

// Convert all room objects to the data resolution (only if it's different from game resolution).
// TODO: merge this into UpdateRoomData? or this is required for engine only?
void convert_room_coordinates_to_data_res(RoomStruct *rstruc) {
	if (_GP(game).GetDataUpscaleMult() == 1)
		return;

	const int mul = _GP(game).GetDataUpscaleMult();
	for (size_t i = 0; i < rstruc->ObjectCount; ++i) {
		rstruc->Objects[i].X /= mul;
		rstruc->Objects[i].Y /= mul;
		if (rstruc->Objects[i].Baseline > 0) {
			rstruc->Objects[i].Baseline /= mul;
		}
	}

	for (size_t i = 0; i < rstruc->HotspotCount; ++i) {
		rstruc->Hotspots[i].WalkTo.X /= mul;
		rstruc->Hotspots[i].WalkTo.Y /= mul;
	}

	for (size_t i = 0; i < rstruc->WalkBehindCount; ++i) {
		rstruc->WalkBehinds[i].Baseline /= mul;
	}

	rstruc->Edges.Left /= mul;
	rstruc->Edges.Top /= mul;
	rstruc->Edges.Bottom /= mul;
	rstruc->Edges.Right /= mul;
	rstruc->Width /= mul;
	rstruc->Height /= mul;
}

void update_letterbox_mode() {
	const Size real_room_sz = Size(data_to_game_coord(_GP(thisroom).Width), data_to_game_coord(_GP(thisroom).Height));
	const Rect game_frame = RectWH(_GP(game).GetGameRes());
	Rect new_main_view = game_frame;
	// In the original engine the letterbox feature only allowed viewports of
	// either 200 or 240 (400 and 480) pixels, if the room height was equal or greater than 200 (400).
	// Also, the UI viewport should be matching room viewport in that case.
	// NOTE: if "OPT_LETTERBOX" is false, altsize.Height = size.Height always.
	const int viewport_height =
	    real_room_sz.Height < _GP(game).GetLetterboxSize().Height ? real_room_sz.Height :
	    (real_room_sz.Height >= _GP(game).GetLetterboxSize().Height && real_room_sz.Height < _GP(game).GetGameRes().Height) ? _GP(game).GetLetterboxSize().Height :
	    _GP(game).GetGameRes().Height;
	new_main_view.SetHeight(viewport_height);

	_GP(play).SetMainViewport(CenterInRect(game_frame, new_main_view));
	_GP(play).SetUIViewport(new_main_view);
}

// Automatically reset primary room viewport and camera to match the new room size
static void adjust_viewport_to_room() {
	const Size real_room_sz = Size(data_to_game_coord(_GP(thisroom).Width), data_to_game_coord(_GP(thisroom).Height));
	const Rect main_view = _GP(play).GetMainViewport();
	Rect new_room_view = RectWH(Size::Clamp(real_room_sz, Size(1, 1), main_view.GetSize()));

	auto view = _GP(play).GetRoomViewport(0);
	view->SetRect(new_room_view);
	auto cam = view->GetCamera();
	if (cam) {
		cam->SetSize(new_room_view.GetSize());
		cam->SetAt(0, 0);
		cam->Release();
	}
}

// Run through all viewports and cameras to make sure they can work in new room's bounds
static void update_all_viewcams_with_newroom() {
	for (int i = 0; i < _GP(play).GetRoomCameraCount(); ++i) {
		auto cam = _GP(play).GetRoomCamera(i);
		const Rect old_pos = cam->GetRect();
		cam->SetSize(old_pos.GetSize());
		cam->SetAt(old_pos.Left, old_pos.Top);
	}
}

// forchar = _G(playerchar) on NewRoom, or NULL if restore saved game
void load_new_room(int newnum, CharacterInfo *forchar) {

	debug_script_log("Loading room %d", newnum);

	String room_filename;
	int cc;
	_G(done_es_error) = 0;
	_GP(play).room_changes ++;
	// TODO: find out why do we need to temporarily lower color depth to 8-bit.
	// Or do we? There's a serious usability problem in this: if any bitmap is
	// created meanwhile it will have this color depth by default, which may
	// lead to unexpected errors.
	set_color_depth(8);
	_G(displayed_room) = newnum;

	room_filename.Format("room%d.crm", newnum);
	if (newnum == 0) {
		// support both room0.crm and intro.crm
		// 2.70: Renamed intro.crm to room0.crm, to stop it causing confusion
		if ((_G(loaded_game_file_version) < kGameVersion_270 && Shared::AssetManager::DoesAssetExist("intro.crm")) ||
		        (_G(loaded_game_file_version) >= kGameVersion_270 && !Shared::AssetManager::DoesAssetExist(room_filename))) {
			room_filename = "intro.crm";
		}
	}

	update_polled_stuff_if_runtime();

	// load the room from disk
	_G(our_eip) = 200;
	_GP(thisroom).GameID = NO_GAME_ID_IN_ROOM_FILE;
	load_room(room_filename, &_GP(thisroom), _GP(game).IsLegacyHiRes(), _GP(game).SpriteInfos);

	if ((_GP(thisroom).GameID != NO_GAME_ID_IN_ROOM_FILE) &&
	        (_GP(thisroom).GameID != _GP(game).uniqueid)) {
		quitprintf("!Unable to load '%s'. This room file is assigned to a different game.", room_filename.GetCStr());
	}

	convert_room_coordinates_to_data_res(&_GP(thisroom));

	update_polled_stuff_if_runtime();
	_G(our_eip) = 201;
	/*  // apparently, doing this stops volume spiking between tracks
	if (_GP(thisroom).Options.StartupMusic>0) {
	stopmusic();
	delay(100);
	}*/

	_GP(play).room_width = _GP(thisroom).Width;
	_GP(play).room_height = _GP(thisroom).Height;
	_GP(play).anim_background_speed = _GP(thisroom).BgAnimSpeed;
	_GP(play).bg_anim_delay = _GP(play).anim_background_speed;

	// do the palette
	for (cc = 0; cc < 256; cc++) {
		if (_GP(game).paluses[cc] == PAL_BACKGROUND)
			_G(palette)[cc] = _GP(thisroom).Palette[cc];
		else {
			// copy the gamewide colours into the room palette
			for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i)
				_GP(thisroom).BgFrames[i].Palette[cc] = _G(palette)[cc];
		}
	}

	for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i) {
		update_polled_stuff_if_runtime();
		_GP(thisroom).BgFrames[i].Graphic = PrepareSpriteForUse(_GP(thisroom).BgFrames[i].Graphic, false);
	}

	update_polled_stuff_if_runtime();

	_G(our_eip) = 202;
	// Update game viewports
	if (_GP(game).IsLegacyLetterbox())
		update_letterbox_mode();
	SetMouseBounds(0, 0, 0, 0);

	_G(our_eip) = 203;
	_G(in_new_room) = 1;

	// _G(walkable_areas_temp) is used by the pathfinder to generate a
	// copy of the walkable areas - allocate it here to save time later
	delete _G(walkable_areas_temp);
	_G(walkable_areas_temp) = BitmapHelper::CreateBitmap(_GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight(), 8);

	// Make a backup copy of the walkable areas prior to
	// any RemoveWalkableArea commands
	delete _G(walkareabackup);
	// copy the walls screen
	_G(walkareabackup) = BitmapHelper::CreateBitmapCopy(_GP(thisroom).WalkAreaMask.get());

	_G(our_eip) = 204;
	update_polled_stuff_if_runtime();
	redo_walkable_areas();
	update_polled_stuff_if_runtime();

	set_color_depth(_GP(game).GetColorDepth());
	convert_room_background_to_game_res();
	recache_walk_behinds();
	update_polled_stuff_if_runtime();

	_G(our_eip) = 205;
	// setup objects
	if (forchar != nullptr) {
		// if not restoring a game, always reset this room
		_GP(troom).beenhere = 0;
		_GP(troom).FreeScriptData();
		_GP(troom).FreeProperties();
		memset(&_GP(troom).hotspot_enabled[0], 1, MAX_ROOM_HOTSPOTS);
		memset(&_GP(troom).region_enabled[0], 1, MAX_ROOM_REGIONS);
	}
	if ((newnum >= 0) & (newnum < MAX_ROOMS))
		_G(croom) = getRoomStatus(newnum);
	else _G(croom) = &_GP(troom);

	if (_G(croom)->beenhere > 0) {
		// if we've been here before, save the Times Run information
		// since we will overwrite the actual NewInteraction structs
		// (cos they have pointers and this might have been loaded from
		// a save game)
		if (_GP(thisroom).EventHandlers == nullptr) {
			// legacy interactions
			_GP(thisroom).Interaction->CopyTimesRun(_G(croom)->intrRoom);
			for (cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
				_GP(thisroom).Hotspots[cc].Interaction->CopyTimesRun(_G(croom)->intrHotspot[cc]);
			for (cc = 0; cc < MAX_ROOM_OBJECTS; cc++)
				_GP(thisroom).Objects[cc].Interaction->CopyTimesRun(_G(croom)->intrObject[cc]);
			for (cc = 0; cc < MAX_ROOM_REGIONS; cc++)
				_GP(thisroom).Regions[cc].Interaction->CopyTimesRun(_G(croom)->intrRegion[cc]);
		}
	}
	if (_G(croom)->beenhere == 0) {
		_G(croom)->numobj = _GP(thisroom).ObjectCount;
		_G(croom)->tsdatasize = 0;
		for (cc = 0; cc < _G(croom)->numobj; cc++) {
			_G(croom)->obj[cc].x = _GP(thisroom).Objects[cc].X;
			_G(croom)->obj[cc].y = _GP(thisroom).Objects[cc].Y;
			_G(croom)->obj[cc].num = _GP(thisroom).Objects[cc].Sprite;
			_G(croom)->obj[cc].on = _GP(thisroom).Objects[cc].IsOn;
			_G(croom)->obj[cc].view = -1;
			_G(croom)->obj[cc].loop = 0;
			_G(croom)->obj[cc].frame = 0;
			_G(croom)->obj[cc].wait = 0;
			_G(croom)->obj[cc].transparent = 0;
			_G(croom)->obj[cc].moving = -1;
			_G(croom)->obj[cc].flags = _GP(thisroom).Objects[cc].Flags;
			_G(croom)->obj[cc].baseline = -1;
			_G(croom)->obj[cc].zoom = 100;
			_G(croom)->obj[cc].last_width = 0;
			_G(croom)->obj[cc].last_height = 0;
			_G(croom)->obj[cc].blocking_width = 0;
			_G(croom)->obj[cc].blocking_height = 0;
			if (_GP(thisroom).Objects[cc].Baseline >= 0)
				//        _G(croom)->obj[cc].baseoffs=_GP(thisroom).Objects.Baseline[cc]-_GP(thisroom).Objects[cc].y;
				_G(croom)->obj[cc].baseline = _GP(thisroom).Objects[cc].Baseline;
		}
		for (size_t i = 0; i < (size_t)MAX_WALK_BEHINDS; ++i)
			_G(croom)->walkbehind_base[i] = _GP(thisroom).WalkBehinds[i].Baseline;
		for (cc = 0; cc < MAX_FLAGS; cc++) _G(croom)->flagstates[cc] = 0;

		/*    // we copy these structs for the Score column to work
		_G(croom)->misccond=_GP(thisroom).misccond;
		for (cc=0;cc<MAX_ROOM_HOTSPOTS;cc++)
		_G(croom)->hscond[cc]=_GP(thisroom).hscond[cc];
		for (cc=0;cc<MAX_ROOM_OBJECTS;cc++)
		_G(croom)->objcond[cc]=_GP(thisroom).objcond[cc];*/

		for (cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++) {
			_G(croom)->hotspot_enabled[cc] = 1;
		}
		for (cc = 0; cc < MAX_ROOM_REGIONS; cc++) {
			_G(croom)->region_enabled[cc] = 1;
		}

		_G(croom)->beenhere = 1;
		_G(in_new_room) = 2;
	} else {
		// We have been here before
		for (size_t i = 0; i < _GP(thisroom).LocalVariables.size() && i < (size_t)MAX_GLOBAL_VARIABLES; ++i)
			_GP(thisroom).LocalVariables[i].Value = _G(croom)->interactionVariableValues[i];
	}

	update_polled_stuff_if_runtime();

	if (_GP(thisroom).EventHandlers == nullptr) {
		// legacy interactions
		// copy interactions from room file into our temporary struct
		_G(croom)->intrRoom = *_GP(thisroom).Interaction;
		for (cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
			_G(croom)->intrHotspot[cc] = *_GP(thisroom).Hotspots[cc].Interaction;
		for (cc = 0; cc < MAX_ROOM_OBJECTS; cc++)
			_G(croom)->intrObject[cc] = *_GP(thisroom).Objects[cc].Interaction;
		for (cc = 0; cc < MAX_ROOM_REGIONS; cc++)
			_G(croom)->intrRegion[cc] = *_GP(thisroom).Regions[cc].Interaction;
	}

	_G(objs) = &_G(croom)->obj[0];

	for (cc = 0; cc < MAX_ROOM_OBJECTS; cc++) {
		// 64 bit: Using the id instead
		// _G(scrObj)[cc].obj = &_G(croom)->obj[cc];
		_G(objectScriptObjNames)[cc].Free();
	}

	for (cc = 0; cc < _G(croom)->numobj; cc++) {
		// export the object's script object
		if (_GP(thisroom).Objects[cc].ScriptName.IsEmpty())
			continue;
		_G(objectScriptObjNames)[cc] = _GP(thisroom).Objects[cc].ScriptName;
		ccAddExternalDynamicObject(_G(objectScriptObjNames)[cc], &_G(scrObj)[cc], &_GP(ccDynamicObject));
	}

	for (cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++) {
		if (_GP(thisroom).Hotspots[cc].ScriptName.IsEmpty())
			continue;

		ccAddExternalDynamicObject(_GP(thisroom).Hotspots[cc].ScriptName, &_G(scrHotspot)[cc], &_GP(ccDynamicHotspot));
	}

	_G(our_eip) = 206;
	/*  THIS IS DONE IN THE EDITOR NOW
	_GP(thisroom).BgFrames.IsPaletteShared[0] = 1;
	for (dd = 1; dd < _GP(thisroom).BgFrameCount; dd++) {
	if (memcmp (&_GP(thisroom).BgFrames.Palette[dd][0], &palette[0], sizeof(color) * 256) == 0)
	_GP(thisroom).BgFrames.IsPaletteShared[dd] = 1;
	else
	_GP(thisroom).BgFrames.IsPaletteShared[dd] = 0;
	}
	// only make the first frame shared if the last is
	if (_GP(thisroom).BgFrames.IsPaletteShared[_GP(thisroom).BgFrameCount - 1] == 0)
	_GP(thisroom).BgFrames.IsPaletteShared[0] = 0;*/

	update_polled_stuff_if_runtime();

	_G(our_eip) = 210;
	if (IS_ANTIALIAS_SPRITES) {
		// sometimes the palette has corrupt entries, which crash
		// the create_rgb_table call
		// so, fix them
		for (int ff = 0; ff < 256; ff++) {
			if (_G(palette)[ff].r > 63)
				_G(palette)[ff].r = 63;
			if (_G(palette)[ff].g > 63)
				_G(palette)[ff].g = 63;
			if (_G(palette)[ff].b > 63)
				_G(palette)[ff].b = 63;
		}
		create_rgb_table(&_GP(rgb_table), _G(palette), nullptr);
		_G(rgb_map) = &_GP(rgb_table);
	}
	_G(our_eip) = 211;
	if (forchar != nullptr) {
		// if it's not a Restore Game

		// if a following character is still waiting to come into the
		// previous room, force it out so that the timer resets
		for (int ff = 0; ff < _GP(game).numcharacters; ff++) {
			if ((_GP(game).chars[ff].following >= 0) && (_GP(game).chars[ff].room < 0)) {
				if ((_GP(game).chars[ff].following == _GP(game).playercharacter) &&
				        (forchar->prevroom == newnum))
					// the player went back to the previous room, so make sure
					// the following character is still there
					_GP(game).chars[ff].room = newnum;
				else
					_GP(game).chars[ff].room = _GP(game).chars[_GP(game).chars[ff].following].room;
			}
		}

		forchar->prevroom = forchar->room;
		forchar->room = newnum;
		// only stop moving if it's a new room, not a restore game
		for (cc = 0; cc < _GP(game).numcharacters; cc++)
			StopMoving(cc);

	}

	update_polled_stuff_if_runtime();

	_G(roominst) = nullptr;
	if (_G(debug_flags) & DBG_NOSCRIPT) ;
	else if (_GP(thisroom).CompiledScript != nullptr) {
		compile_room_script();
		if (_G(croom)->tsdatasize > 0) {
			if (_G(croom)->tsdatasize != _G(roominst)->globaldatasize)
				quit("room script data segment size has changed");
			memcpy(&_G(roominst)->globaldata[0], _G(croom)->tsdata, _G(croom)->tsdatasize);
		}
	}
	_G(our_eip) = 207;
	_GP(play).entered_edge = -1;

	if ((_G(new_room_x) != SCR_NO_VALUE) && (forchar != nullptr)) {
		forchar->x = _G(new_room_x);
		forchar->y = _G(new_room_y);

		if (_G(new_room_loop) != SCR_NO_VALUE)
			forchar->loop = _G(new_room_loop);
	}
	_G(new_room_x) = SCR_NO_VALUE;
	_G(new_room_loop) = SCR_NO_VALUE;

	if ((_G(new_room_pos) > 0) & (forchar != nullptr)) {
		if (_G(new_room_pos) >= 4000) {
			_GP(play).entered_edge = 3;
			forchar->y = _GP(thisroom).Edges.Top + get_fixed_pixel_size(1);
			forchar->x = _G(new_room_pos) % 1000;
			if (forchar->x == 0) forchar->x = _GP(thisroom).Width / 2;
			if (forchar->x <= _GP(thisroom).Edges.Left)
				forchar->x = _GP(thisroom).Edges.Left + 3;
			if (forchar->x >= _GP(thisroom).Edges.Right)
				forchar->x = _GP(thisroom).Edges.Right - 3;
			forchar->loop = 0;
		} else if (_G(new_room_pos) >= 3000) {
			_GP(play).entered_edge = 2;
			forchar->y = _GP(thisroom).Edges.Bottom - get_fixed_pixel_size(1);
			forchar->x = _G(new_room_pos) % 1000;
			if (forchar->x == 0) forchar->x = _GP(thisroom).Width / 2;
			if (forchar->x <= _GP(thisroom).Edges.Left)
				forchar->x = _GP(thisroom).Edges.Left + 3;
			if (forchar->x >= _GP(thisroom).Edges.Right)
				forchar->x = _GP(thisroom).Edges.Right - 3;
			forchar->loop = 3;
		} else if (_G(new_room_pos) >= 2000) {
			_GP(play).entered_edge = 1;
			forchar->x = _GP(thisroom).Edges.Right - get_fixed_pixel_size(1);
			forchar->y = _G(new_room_pos) % 1000;
			if (forchar->y == 0) forchar->y = _GP(thisroom).Height / 2;
			if (forchar->y <= _GP(thisroom).Edges.Top)
				forchar->y = _GP(thisroom).Edges.Top + 3;
			if (forchar->y >= _GP(thisroom).Edges.Bottom)
				forchar->y = _GP(thisroom).Edges.Bottom - 3;
			forchar->loop = 1;
		} else if (_G(new_room_pos) >= 1000) {
			_GP(play).entered_edge = 0;
			forchar->x = _GP(thisroom).Edges.Left + get_fixed_pixel_size(1);
			forchar->y = _G(new_room_pos) % 1000;
			if (forchar->y == 0) forchar->y = _GP(thisroom).Height / 2;
			if (forchar->y <= _GP(thisroom).Edges.Top)
				forchar->y = _GP(thisroom).Edges.Top + 3;
			if (forchar->y >= _GP(thisroom).Edges.Bottom)
				forchar->y = _GP(thisroom).Edges.Bottom - 3;
			forchar->loop = 2;
		}
		// if starts on un-walkable area
		if (get_walkable_area_pixel(forchar->x, forchar->y) == 0) {
			if (_G(new_room_pos) >= 3000) { // bottom or top of screen
				int tryleft = forchar->x - 1, tryright = forchar->x + 1;
				while (1) {
					if (get_walkable_area_pixel(tryleft, forchar->y) > 0) {
						forchar->x = tryleft;
						break;
					}
					if (get_walkable_area_pixel(tryright, forchar->y) > 0) {
						forchar->x = tryright;
						break;
					}
					int nowhere = 0;
					if (tryleft > _GP(thisroom).Edges.Left) {
						tryleft--;
						nowhere++;
					}
					if (tryright < _GP(thisroom).Edges.Right) {
						tryright++;
						nowhere++;
					}
					if (nowhere == 0) break; // no place to go, so leave him
				}
			} else if (_G(new_room_pos) >= 1000) { // left or right
				int tryleft = forchar->y - 1, tryright = forchar->y + 1;
				while (1) {
					if (get_walkable_area_pixel(forchar->x, tryleft) > 0) {
						forchar->y = tryleft;
						break;
					}
					if (get_walkable_area_pixel(forchar->x, tryright) > 0) {
						forchar->y = tryright;
						break;
					}
					int nowhere = 0;
					if (tryleft > _GP(thisroom).Edges.Top) {
						tryleft--;
						nowhere++;
					}
					if (tryright < _GP(thisroom).Edges.Bottom) {
						tryright++;
						nowhere++;
					}
					if (nowhere == 0) break; // no place to go, so leave him
				}
			}
		}
		_G(new_room_pos) = 0;
	}
	if (forchar != nullptr) {
		_GP(play).entered_at_x = forchar->x;
		_GP(play).entered_at_y = forchar->y;
		if (forchar->x >= _GP(thisroom).Edges.Right)
			_GP(play).entered_edge = 1;
		else if (forchar->x <= _GP(thisroom).Edges.Left)
			_GP(play).entered_edge = 0;
		else if (forchar->y >= _GP(thisroom).Edges.Bottom)
			_GP(play).entered_edge = 2;
		else if (forchar->y <= _GP(thisroom).Edges.Top)
			_GP(play).entered_edge = 3;
	}
	if (_GP(thisroom).Options.StartupMusic > 0)
		PlayMusicResetQueue(_GP(thisroom).Options.StartupMusic);

	_G(our_eip) = 208;
	if (forchar != nullptr) {
		if (_GP(thisroom).Options.PlayerCharOff == 0) {
			forchar->on = 1;
			enable_cursor_mode(0);
		} else {
			forchar->on = 0;
			disable_cursor_mode(0);
			// remember which character we turned off, in case they
			// use SetPlyaerChracter within this room (so we re-enable
			// the correct character when leaving the room)
			_GP(play).temporarily_turned_off_character = _GP(game).playercharacter;
		}
		if (forchar->flags & CHF_FIXVIEW) ;
		else if (_GP(thisroom).Options.PlayerView == 0) forchar->view = forchar->defview;
		else forchar->view = _GP(thisroom).Options.PlayerView - 1;
		forchar->frame = 0; // make him standing
	}
	_G(color_map) = nullptr;

	_G(our_eip) = 209;
	update_polled_stuff_if_runtime();
	generate_light_table();
	update_music_volume();

	// If we are not restoring a save, update cameras to accomodate for this
	// new room; otherwise this is done later when cameras are recreated.
	if (forchar != nullptr) {
		if (_GP(play).IsAutoRoomViewport())
			adjust_viewport_to_room();
		update_all_viewcams_with_newroom();
		_GP(play).UpdateRoomCameras(); // update auto tracking
	}
	init_room_drawdata();

	_G(our_eip) = 212;
	invalidate_screen();
	for (cc = 0; cc < _G(croom)->numobj; cc++) {
		if (_G(objs)[cc].on == 2)
			MergeObject(cc);
	}
	_G(new_room_flags) = 0;
	_GP(play).gscript_timer = -1; // avoid screw-ups with changing screens
	_GP(play).player_on_region = 0;
	// trash any input which they might have done while it was loading
	ags_clear_input_buffer();
	// no fade in, so set the palette immediately in case of 256-col sprites
	if (_GP(game).color_depth > 1)
		setpal();

	_G(our_eip) = 220;
	update_polled_stuff_if_runtime();
	debug_script_log("Now in room %d", _G(displayed_room));
	_G(guis_need_update) = 1;
	pl_run_plugin_hooks(AGSE_ENTERROOM, _G(displayed_room));
	//  MoveToWalkableArea(_GP(game).playercharacter);
	//  MSS_CHECK_ALL_BLOCKS;
}

// new_room: changes the current room number, and loads the new room from disk
void new_room(int newnum, CharacterInfo *forchar) {
	EndSkippingUntilCharStops();

	debug_script_log("Room change requested to room %d", newnum);

	update_polled_stuff_if_runtime();

	// we are currently running Leaves Screen scripts
	_G(in_leaves_screen) = newnum;

	// player leaves screen event
	run_room_event(8);
	// Run the global OnRoomLeave event
	run_on_event(GE_LEAVE_ROOM, RuntimeScriptValue().SetInt32(_G(displayed_room)));

	pl_run_plugin_hooks(AGSE_LEAVEROOM, _G(displayed_room));

	// update the new room number if it has been altered by OnLeave scripts
	newnum = _G(in_leaves_screen);
	_G(in_leaves_screen) = -1;

	if ((_G(playerchar)->following >= 0) &&
	        (_GP(game).chars[_G(playerchar)->following].room != newnum)) {
		// the player character is following another character,
		// who is not in the new room. therefore, abort the follow
		_G(playerchar)->following = -1;
	}
	update_polled_stuff_if_runtime();

	// change rooms
	unload_old_room();

	if (_G(psp_clear_cache_on_room_change)) {
		// Delete all cached sprites
		_GP(spriteset).DisposeAll();

		// Delete all gui background images
		for (int i = 0; i < _GP(game).numgui; i++) {
			delete _G(guibg)[i];
			_G(guibg)[i] = nullptr;

			if (_G(guibgbmp)[i])
				_G(gfxDriver)->DestroyDDB(_G(guibgbmp)[i]);
			_G(guibgbmp)[i] = nullptr;
		}
		_G(guis_need_update) = 1;
	}

	update_polled_stuff_if_runtime();

	load_new_room(newnum, forchar);
}

int find_highest_room_entered() {
	int qq, fndas = -1;
	for (qq = 0; qq < MAX_ROOMS; qq++) {
		if (isRoomStatusValid(qq) && (getRoomStatus(qq)->beenhere != 0))
			fndas = qq;
	}
	// This is actually legal - they might start in room 400 and save
	//if (fndas<0) quit("find_highest_room: been in no rooms?");
	return fndas;
}

void first_room_initialization() {
	_G(starting_room) = _G(displayed_room);
	set_loop_counter(0);
	_G(mouse_z_was) = _G(mouse_z);
}

void check_new_room() {
	// if they're in a new room, run Player Enters Screen and on_event(ENTER_ROOM)
	if ((_G(in_new_room) > 0) & (_G(in_new_room) != 3)) {
		EventHappened evh;
		evh.type = EV_RUNEVBLOCK;
		evh.data1 = EVB_ROOM;
		evh.data2 = 0;
		evh.data3 = 5;
		evh.player = _GP(game).playercharacter;
		// make sure that any script calls don't re-call enters screen
		int newroom_was = _G(in_new_room);
		_G(in_new_room) = 0;
		_GP(play).disabled_user_interface ++;
		process_event(&evh);
		_GP(play).disabled_user_interface --;
		_G(in_new_room) = newroom_was;
		//    setevent(EV_RUNEVBLOCK,EVB_ROOM,0,5);
	}
}

void compile_room_script() {
	_G(ccError) = 0;

	_G(roominst) = ccInstance::CreateFromScript(_GP(thisroom).CompiledScript);

	if ((_G(ccError) != 0) || (_G(roominst) == nullptr)) {
		quitprintf("Unable to create local script: %s", _G(ccErrorString).GetCStr());
	}

	_G(roominstFork) = _G(roominst)->Fork();
	if (_G(roominstFork) == nullptr)
		quitprintf("Unable to create forked room instance: %s", _G(ccErrorString).GetCStr());

	_GP(repExecAlways).roomHasFunction = true;
	_GP(lateRepExecAlways).roomHasFunction = true;
	_GP(getDialogOptionsDimensionsFunc).roomHasFunction = true;
}

void on_background_frame_change() {

	invalidate_screen();
	mark_current_background_dirty();
	invalidate_cached_walkbehinds();

	// get the new frame's palette
	memcpy(_G(palette), _GP(thisroom).BgFrames[_GP(play).bg_frame].Palette, sizeof(color) * 256);

	// hi-colour, update the palette. It won't have an immediate effect
	// but will be drawn properly when the screen fades in
	if (_GP(game).color_depth > 1)
		setpal();

	if (_G(in_enters_screen))
		return;

	// Don't update the palette if it hasn't changed
	if (_GP(thisroom).BgFrames[_GP(play).bg_frame].IsPaletteShared)
		return;

	// 256-colours, tell it to update the palette (will actually be done as
	// close as possible to the screen update to prevent flicker problem)
	if (_GP(game).color_depth == 1)
		_G(bg_just_changed) = 1;
}

void croom_ptr_clear() {
	_G(croom) = nullptr;
	_G(objs) = nullptr;
}


AGS_INLINE int room_to_mask_coord(int coord) {
	return coord * _GP(game).GetDataUpscaleMult() / _GP(thisroom).MaskResolution;
}

AGS_INLINE int mask_to_room_coord(int coord) {
	return coord * _GP(thisroom).MaskResolution / _GP(game).GetDataUpscaleMult();
}

void convert_move_path_to_room_resolution(MoveList *ml) {
	if ((_GP(game).options[OPT_WALKSPEEDABSOLUTE] != 0) && _GP(game).GetDataUpscaleMult() > 1) {
		// Speeds are independent from MaskResolution
		for (int i = 0; i < ml->numstage; i++) {
			// ...so they are not multiplied by MaskResolution factor when converted to room coords
			ml->xpermove[i] = ml->xpermove[i] / _GP(game).GetDataUpscaleMult();
			ml->ypermove[i] = ml->ypermove[i] / _GP(game).GetDataUpscaleMult();
		}
	}

	if (_GP(thisroom).MaskResolution == _GP(game).GetDataUpscaleMult())
		return;

	ml->fromx = mask_to_room_coord(ml->fromx);
	ml->fromy = mask_to_room_coord(ml->fromy);
	ml->lastx = mask_to_room_coord(ml->lastx);
	ml->lasty = mask_to_room_coord(ml->lasty);

	for (int i = 0; i < ml->numstage; i++) {
		uint16_t lowPart = mask_to_room_coord(ml->pos[i] & 0x0000ffff);
		uint16_t highPart = mask_to_room_coord((ml->pos[i] >> 16) & 0x0000ffff);
		ml->pos[i] = ((int)highPart << 16) | (lowPart & 0x0000ffff);
	}

	if (_GP(game).options[OPT_WALKSPEEDABSOLUTE] == 0) {
		// Speeds are scaling with MaskResolution
		for (int i = 0; i < ml->numstage; i++) {
			ml->xpermove[i] = mask_to_room_coord(ml->xpermove[i]);
			ml->ypermove[i] = mask_to_room_coord(ml->ypermove[i]);
		}
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================



// ScriptDrawingSurface* (int backgroundNumber)
RuntimeScriptValue Sc_Room_GetDrawingSurfaceForBackground(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT(ScriptDrawingSurface, Room_GetDrawingSurfaceForBackground);
}

// int (const char *property)
RuntimeScriptValue Sc_Room_GetProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Room_GetProperty, const char);
}

// const char* (const char *property)
RuntimeScriptValue Sc_Room_GetTextProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_POBJ(const char, _GP(myScriptStringImpl), Room_GetTextProperty, const char);
}

RuntimeScriptValue Sc_Room_SetProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL_POBJ_PINT(Room_SetProperty, const char);
}

// const char* (const char *property)
RuntimeScriptValue Sc_Room_SetTextProperty(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL_POBJ2(Room_SetTextProperty, const char, const char);
}

// int ()
RuntimeScriptValue Sc_Room_GetBottomEdge(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetBottomEdge);
}

// int ()
RuntimeScriptValue Sc_Room_GetColorDepth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetColorDepth);
}

// int ()
RuntimeScriptValue Sc_Room_GetHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetHeight);
}

// int ()
RuntimeScriptValue Sc_Room_GetLeftEdge(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetLeftEdge);
}

// const char* (int index)
RuntimeScriptValue Sc_Room_GetMessages(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_PINT(const char, _GP(myScriptStringImpl), Room_GetMessages);
}

// int ()
RuntimeScriptValue Sc_Room_GetMusicOnLoad(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetMusicOnLoad);
}

// int ()
RuntimeScriptValue Sc_Room_GetObjectCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetObjectCount);
}

// int ()
RuntimeScriptValue Sc_Room_GetRightEdge(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetRightEdge);
}

// int ()
RuntimeScriptValue Sc_Room_GetTopEdge(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetTopEdge);
}

// int ()
RuntimeScriptValue Sc_Room_GetWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Room_GetWidth);
}

// void (int xx,int yy,int mood)
RuntimeScriptValue Sc_RoomProcessClick(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(RoomProcessClick);
}


void RegisterRoomAPI() {
	ccAddExternalStaticFunction("Room::GetDrawingSurfaceForBackground^1",   Sc_Room_GetDrawingSurfaceForBackground);
	ccAddExternalStaticFunction("Room::GetProperty^1",                      Sc_Room_GetProperty);
	ccAddExternalStaticFunction("Room::GetTextProperty^1",                  Sc_Room_GetTextProperty);
	ccAddExternalStaticFunction("Room::SetProperty^2",                      Sc_Room_SetProperty);
	ccAddExternalStaticFunction("Room::SetTextProperty^2",                  Sc_Room_SetTextProperty);
	ccAddExternalStaticFunction("Room::ProcessClick^3",                     Sc_RoomProcessClick);
	ccAddExternalStaticFunction("ProcessClick",                             Sc_RoomProcessClick);
	ccAddExternalStaticFunction("Room::get_BottomEdge",                     Sc_Room_GetBottomEdge);
	ccAddExternalStaticFunction("Room::get_ColorDepth",                     Sc_Room_GetColorDepth);
	ccAddExternalStaticFunction("Room::get_Height",                         Sc_Room_GetHeight);
	ccAddExternalStaticFunction("Room::get_LeftEdge",                       Sc_Room_GetLeftEdge);
	ccAddExternalStaticFunction("Room::geti_Messages",                      Sc_Room_GetMessages);
	ccAddExternalStaticFunction("Room::get_MusicOnLoad",                    Sc_Room_GetMusicOnLoad);
	ccAddExternalStaticFunction("Room::get_ObjectCount",                    Sc_Room_GetObjectCount);
	ccAddExternalStaticFunction("Room::get_RightEdge",                      Sc_Room_GetRightEdge);
	ccAddExternalStaticFunction("Room::get_TopEdge",                        Sc_Room_GetTopEdge);
	ccAddExternalStaticFunction("Room::get_Width",                          Sc_Room_GetWidth);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("Room::GetDrawingSurfaceForBackground^1", (void *)Room_GetDrawingSurfaceForBackground);
	ccAddExternalFunctionForPlugin("Room::GetProperty^1", (void *)Room_GetProperty);
	ccAddExternalFunctionForPlugin("Room::GetTextProperty^1", (void *)Room_GetTextProperty);
	ccAddExternalFunctionForPlugin("Room::get_BottomEdge", (void *)Room_GetBottomEdge);
	ccAddExternalFunctionForPlugin("Room::get_ColorDepth", (void *)Room_GetColorDepth);
	ccAddExternalFunctionForPlugin("Room::get_Height", (void *)Room_GetHeight);
	ccAddExternalFunctionForPlugin("Room::get_LeftEdge", (void *)Room_GetLeftEdge);
	ccAddExternalFunctionForPlugin("Room::geti_Messages", (void *)Room_GetMessages);
	ccAddExternalFunctionForPlugin("Room::get_MusicOnLoad", (void *)Room_GetMusicOnLoad);
	ccAddExternalFunctionForPlugin("Room::get_ObjectCount", (void *)Room_GetObjectCount);
	ccAddExternalFunctionForPlugin("Room::get_RightEdge", (void *)Room_GetRightEdge);
	ccAddExternalFunctionForPlugin("Room::get_TopEdge", (void *)Room_GetTopEdge);
	ccAddExternalFunctionForPlugin("Room::get_Width", (void *)Room_GetWidth);
}

} // namespace AGS3

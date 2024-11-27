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

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string_utils.h" //strlwr()
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/region.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_object.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/engine/ac/dynobj/script_object.h"
#include "ags/engine/ac/dynobj/script_hotspot.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/game/room_version.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin_evts.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/ac/dynobj/all_dynamic_classes.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/util/math.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/main/game_run.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
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
	replace_tokens(get_translation(_GP(thisroom).Messages[index].GetCStr()), buffer, STD_BUFFER_SIZE);
	return CreateNewScriptString(buffer);
}

bool Room_Exists(int room) {
	String room_filename;
	room_filename.Format("room%d.crm", room);
	return _GP(AssetMgr)->DoesAssetExist(room_filename);
}

ScriptDrawingSurface *GetDrawingSurfaceForWalkableArea() {
	return Room_GetDrawingSurfaceForMask(kRoomAreaWalkable);
}

ScriptDrawingSurface *GetDrawingSurfaceForWalkbehind() {
	return Room_GetDrawingSurfaceForMask(kRoomAreaWalkBehind);
}

ScriptDrawingSurface *Hotspot_GetDrawingSurface() {
	return Room_GetDrawingSurfaceForMask(kRoomAreaHotspot);
}

ScriptDrawingSurface *Region_GetDrawingSurface() {
	return Room_GetDrawingSurfaceForMask(kRoomAreaRegion);
}

//=============================================================================

// Makes sure that room background and walk-behind mask are matching room size
// in game resolution coordinates; in other words makes graphics appropriate
// for display in the game.
void convert_room_background_to_game_res() {
	if (!_GP(game).AllowRelativeRes() || !_GP(thisroom).IsRelativeRes())
		return;

	const int bkg_width = data_to_game_coord(_GP(thisroom).Width);
	const int bkg_height = data_to_game_coord(_GP(thisroom).Height);

	for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i)
		_GP(thisroom).BgFrames[i].Graphic = FixBitmap(_GP(thisroom).BgFrames[i].Graphic, bkg_width, bkg_height);

	// Fix masks to match resized room background
	// Walk-behind is always 1:1 with room background size
	_GP(thisroom).WalkBehindMask = FixBitmap(_GP(thisroom).WalkBehindMask, bkg_width, bkg_height);
	// For the rest we keep the masks at original res, but update the MaskResolution,
	// as it must correspond to the runtime data->game coordinate conversion
	_GP(thisroom).MaskResolution = data_to_game_coord(_GP(thisroom).MaskResolution);
}

void save_room_data_segment() {
	_G(croom)->FreeScriptData();

	_G(croom)->tsdatasize = _G(roominst)->globaldatasize;
	if (_G(croom)->tsdatasize > 0) {
		_G(croom)->tsdata.resize(_G(croom)->tsdatasize);
		memcpy(_G(croom)->tsdata.data(), &_G(roominst)->globaldata[0], _G(croom)->tsdatasize);
	}

}

void unload_old_room() {
	// if switching games on restore, don't do this
	if (_G(displayed_room) < 0)
		return;

	current_fade_out_effect();

	// room unloaded callback
	run_room_event(EVROM_AFTERFADEOUT);
	// global room unloaded event
	run_on_event(GE_LEAVE_ROOM_AFTERFADE, RuntimeScriptValue().SetInt32(_G(displayed_room)));

	debug_script_log("Unloading room %d", _G(displayed_room));

	dispose_room_drawdata();

	for (uint32_t ff = 0; ff < _G(croom)->numobj; ff++)
		_G(objs)[ff].moving = 0;

	if (!_GP(play).ambient_sounds_persist) {
		for (int ff = NUM_SPEECH_CHANS; ff < _GP(game).numGameChannels; ff++)
			StopAmbientSound(ff);
	}

	cancel_all_scripts();
	_GP(events).clear();  // cancel any pending room events

	if (_G(roomBackgroundBmp) != nullptr) {
		_G(gfxDriver)->DestroyDDB(_G(roomBackgroundBmp));
		_G(roomBackgroundBmp) = nullptr;
	}

	if (_G(croom) == nullptr) ;
	else if (_G(roominst) != nullptr) {
		save_room_data_segment();
		FreeRoomScriptInstance();
	} else _G(croom)->tsdatasize = 0;
	memset(&_GP(play).walkable_areas_on[0], 1, MAX_WALK_AREAS);
	_GP(play).bg_frame = 0;
	_GP(play).bg_frame_locked = 0;
	remove_all_overlays();
	_G(raw_saved_screen).reset();
	for (int ff = 0; ff < MAX_ROOM_BGFRAMES; ff++)
		_GP(play).raw_modified[ff] = 0;
	for (size_t i = 0; i < _GP(thisroom).LocalVariables.size() && i < MAX_GLOBAL_VARIABLES; ++i)
		_G(croom)->interactionVariableValues[i] = _GP(thisroom).LocalVariables[i].Value;

	// ensure that any half-moves (eg. with scaled movement) are stopped
	for (int ff = 0; ff < _GP(game).numcharacters; ff++) {
		_GP(charextra)[ff].xwas = INVALID_X;
	}

	_GP(play).swap_portrait_lastchar = -1;
	_GP(play).swap_portrait_lastlastchar = -1;

	for (uint32_t ff = 0; ff < _G(croom)->numobj; ff++) {
		// un-export the object's script object
		if (_GP(thisroom).Objects[ff].ScriptName.IsEmpty())
			continue;

		ccRemoveExternalSymbol(_GP(thisroom).Objects[ff].ScriptName);
	}

	for (int ff = 0; ff < MAX_ROOM_HOTSPOTS; ff++) {
		if (_GP(thisroom).Hotspots[ff].ScriptName.IsEmpty())
			continue;

		ccRemoveExternalSymbol(_GP(thisroom).Hotspots[ff].ScriptName);
	}

	croom_ptr_clear();

	// clear the draw caches to save memory, since many of the the involved
	// objects probably aren't on the new screen
	clear_drawobj_cache();

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
	for (auto &obj : rstruc->Objects) {
		obj.X /= mul;
		obj.Y /= mul;
		if (obj.Baseline > 0) {
			obj.Baseline /= mul;
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
	on_mainviewport_changed();
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

// Looks up for the room script available as a separate asset.
// This is optional, so no error is raised if one is not found.
// If found however, it will replace room script if one had been loaded
// from the room file itself.
HError LoadRoomScript(RoomStruct *room, int newnum) {
	String filename = String::FromFormat("room%d.o", newnum);
	std::unique_ptr<Stream> in(_GP(AssetMgr)->OpenAsset(filename));
	if (in) {
		PScript script(ccScript::CreateFromStream(in.get()));
		if (!script)
			return new Error(String::FromFormat(
				"Failed to load a script module: %s", filename.GetCStr()),
				cc_get_error().ErrorString);
		room->CompiledScript = script;
	}
	return HError::None();
}

static void reset_temp_room() {
	_GP(troom) = RoomStatus();
}

// forchar = playerchar on NewRoom, or NULL if restore saved game
void load_new_room(int newnum, CharacterInfo *forchar) {

	debug_script_log("Loading room %d", newnum);

	String room_filename;
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
		if ((_G(loaded_game_file_version) < kGameVersion_270 && _GP(AssetMgr)->DoesAssetExist("intro.crm")) ||
		        (_G(loaded_game_file_version) >= kGameVersion_270 && !_GP(AssetMgr)->DoesAssetExist(room_filename))) {
			room_filename = "intro.crm";
		}
	}

	// load the room from disk
	set_our_eip(200);
	_GP(thisroom).GameID = NO_GAME_ID_IN_ROOM_FILE;
	load_room(room_filename, &_GP(thisroom), _GP(game).IsLegacyHiRes(), _GP(game).SpriteInfos);

	if ((_GP(thisroom).GameID != NO_GAME_ID_IN_ROOM_FILE) &&
	        (_GP(thisroom).GameID != _GP(game).uniqueid)) {
		quitprintf("!Unable to load '%s'. This room file is assigned to a different game.", room_filename.GetCStr());
	}

	HError err = LoadRoomScript(&_GP(thisroom), newnum);
	if (!err)
		quitprintf("!Unable to load '%s'. Error: %s", room_filename.GetCStr(),
			err->FullMessage().GetCStr());

	convert_room_coordinates_to_data_res(&_GP(thisroom));

	set_our_eip(201);

	_GP(play).room_width = _GP(thisroom).Width;
	_GP(play).room_height = _GP(thisroom).Height;
	_GP(play).anim_background_speed = _GP(thisroom).BgAnimSpeed;
	_GP(play).bg_anim_delay = _GP(play).anim_background_speed;

	// Fixup the frame index, in case the new room does not have enough background frames
	if (_GP(play).bg_frame < 0 || static_cast<size_t>(_GP(play).bg_frame) >= _GP(thisroom).BgFrameCount)
		_GP(play).bg_frame = 0;

	// do the palette
	for (size_t cc = 0; cc < 256; cc++) {
		if (_GP(game).paluses[cc] == PAL_BACKGROUND)
			_G(palette)[cc] = _GP(thisroom).Palette[cc];
		else {
			// copy the gamewide colours into the room palette
			for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i)
				_GP(thisroom).BgFrames[i].Palette[cc] = _G(palette)[cc];
		}
	}

	for (size_t i = 0; i < _GP(thisroom).BgFrameCount; ++i) {
		_GP(thisroom).BgFrames[i].Graphic = PrepareSpriteForUse(_GP(thisroom).BgFrames[i].Graphic, false);
	}

	set_our_eip(202);
	// Update game viewports
	if (_GP(game).IsLegacyLetterbox())
		update_letterbox_mode();
	SetMouseBounds(0, 0, 0, 0);

	set_our_eip(203);
	_G(in_new_room) = 1;

	set_color_depth(_GP(game).GetColorDepth());
	// Make sure the room gfx and masks are matching game's native res
	convert_room_background_to_game_res();

	// walkable_areas_temp is used by the pathfinder to generate a
	// copy of the walkable areas - allocate it here to save time later
	delete _G(walkable_areas_temp);
	_G(walkable_areas_temp) = BitmapHelper::CreateBitmap(_GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight(), 8);

	// Make a backup copy of the walkable areas prior to
	// any RemoveWalkableArea commands
	delete _G(walkareabackup);
	// copy the walls screen
	_G(walkareabackup) = BitmapHelper::CreateBitmapCopy(_GP(thisroom).WalkAreaMask.get());

	set_our_eip(204);
	redo_walkable_areas();
	walkbehinds_recalc();

	set_our_eip(205);
	// setup objects
	if (forchar != nullptr) {
		// if not restoring a game, always reset this room
		reset_temp_room();
	}
	if ((newnum >= 0) & (newnum < MAX_ROOMS))
		_G(croom) = getRoomStatus(newnum);
	else _G(croom) = &_GP(troom);

	// Decide what to do if we have been or not in this room before
	if (_G(croom)->beenhere > 0) {
		// if we've been here before, save the Times Run information
		// since we will overwrite the actual NewInteraction structs
		// (cos they have pointers and this might have been loaded from
		// a save game)
		if (_GP(thisroom).EventHandlers == nullptr) {
			// legacy interactions
			_GP(thisroom).Interaction->CopyTimesRun(_G(croom)->intrRoom);
			for (int cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
				_GP(thisroom).Hotspots[cc].Interaction->CopyTimesRun(_G(croom)->intrHotspot[cc]);
			for (size_t cc = 0; cc < _GP(thisroom).Objects.size(); cc++)
				_GP(thisroom).Objects[cc].Interaction->CopyTimesRun(_G(croom)->intrObject[cc]);
			for (int cc = 0; cc < MAX_ROOM_REGIONS; cc++)
				_GP(thisroom).Regions[cc].Interaction->CopyTimesRun(_G(croom)->intrRegion[cc]);
		}
		for (size_t i = 0; i < _GP(thisroom).LocalVariables.size() && i < (size_t)MAX_GLOBAL_VARIABLES; ++i)
			_GP(thisroom).LocalVariables[i].Value = _G(croom)->interactionVariableValues[i];

		// Always copy object and hotspot names for < 3.6.0 games, because they were not settable
		if ((_G(loaded_game_file_version) < kGameVersion_360_16) ||	(_G(croom)->contentFormat < kRoomStatSvgVersion_36025)) {
			for (size_t cc = 0; cc < _GP(thisroom).Objects.size(); ++cc)
				_G(croom)->obj[cc].name = _GP(thisroom).Objects[cc].Name;
			for (int cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
				_G(croom)->hotspot[cc].Name = _GP(thisroom).Hotspots[cc].Name;
		}
	} else {
		// If we have not been in this room before, then copy necessary fields from _GP(thisroom)
		_G(croom)->numobj = _GP(thisroom).Objects.size();
		_G(croom)->tsdatasize = 0;
		_G(croom)->obj.resize(_G(croom)->numobj);
		_G(croom)->objProps.resize(_G(croom)->numobj);
		_G(croom)->intrObject.resize(_G(croom)->numobj);
		for (size_t cc = 0; cc < _G(croom)->numobj; cc++) {
			const auto &trobj = _GP(thisroom).Objects[cc];
			auto &crobj = _G(croom)->obj[cc];
			crobj.x = trobj.X;
			crobj.y = trobj.Y;
			crobj.num = Math::InRangeOrDef<uint16_t>(trobj.Sprite, 0);
			crobj.on = trobj.IsOn;
			crobj.view = RoomObject::NoView;
			crobj.loop = 0;
			crobj.frame = 0;
			crobj.wait = 0;
			crobj.transparent = 0;
			crobj.moving = -1;
			crobj.flags = trobj.Flags;
			crobj.baseline = -1;
			crobj.zoom = 100;
			crobj.last_width = 0;
			crobj.last_height = 0;
			crobj.blocking_width = 0;
			crobj.blocking_height = 0;
			crobj.name = trobj.Name;
			if (trobj.Baseline >= 0)
				crobj.baseline = trobj.Baseline;
			if (trobj.Sprite > UINT16_MAX)
				debug_script_warn("Warning: object's (id %d) sprite %d outside of internal range (%d), reset to 0",
				                  cc, trobj.Sprite, UINT16_MAX);
		}
		for (size_t i = 0; i < (size_t)MAX_WALK_BEHINDS; ++i)
			_G(croom)->walkbehind_base[i] = _GP(thisroom).WalkBehinds[i].Baseline;

		for (int cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++) {
			_G(croom)->hotspot[cc].Enabled = true;
			_G(croom)->hotspot[cc].Name = _GP(thisroom).Hotspots[cc].Name;
		}
		for (int cc = 0; cc < MAX_ROOM_REGIONS; cc++) {
			_G(croom)->region_enabled[cc] = 1;
		}

#if defined (OBSOLETE)
		for (uint cc = 0; cc < MAX_LEGACY_ROOM_FLAGS; cc++) _G(croom)->flagstates[cc] = 0;
		// we copy these structs for the Score column to work
		_G(croom)->misccond = _GP(thisroom).misccond;
		for (uint cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
			_G(croom)->hscond[cc] = _GP(thisroom).hscond[cc];
		for (uint cc = 0; cc < MAX_ROOM_OBJECTS; cc++)
			_G(croom)->objcond[cc] = _GP(thisroom).objcond[cc];
#endif

		_G(croom)->beenhere = 1;
		_G(in_new_room) = 2;
	}
	// Reset contentFormat hint to avoid doing fixups later
	_G(croom)->contentFormat = kRoomStatSvgVersion_Current;

	if (_GP(thisroom).EventHandlers == nullptr) {
		// legacy interactions
		// copy interactions from room file into our temporary struct
		_G(croom)->intrRoom = *_GP(thisroom).Interaction;
		for (int cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++)
			_G(croom)->intrHotspot[cc] = *_GP(thisroom).Hotspots[cc].Interaction;
		for (size_t cc = 0; cc < _GP(thisroom).Objects.size(); cc++)
			_G(croom)->intrObject[cc] = *_GP(thisroom).Objects[cc].Interaction;
		for (int cc = 0; cc < MAX_ROOM_REGIONS; cc++)
			_G(croom)->intrRegion[cc] = *_GP(thisroom).Regions[cc].Interaction;
	}

	_G(objs) = _G(croom)->obj.size() > 0 ? &_G(croom)->obj[0] : nullptr;

	for (size_t cc = 0; cc < _G(croom)->numobj; cc++) {
		// export the object's script object
		if (_GP(thisroom).Objects[cc].ScriptName.IsEmpty())
			continue;
		ccAddExternalScriptObject(_GP(thisroom).Objects[cc].ScriptName, &_G(scrObj)[cc], &_GP(ccDynamicObject));
	}

	for (int cc = 0; cc < MAX_ROOM_HOTSPOTS; cc++) {
		if (_GP(thisroom).Hotspots[cc].ScriptName.IsEmpty())
			continue;

		ccAddExternalScriptObject(_GP(thisroom).Hotspots[cc].ScriptName, &_G(scrHotspot)[cc], &_GP(ccDynamicHotspot));
	}

	set_our_eip(210);
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
	set_our_eip(211);
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
		for (int cc = 0; cc < _GP(game).numcharacters; cc++)
			StopMoving(cc);
	}

	_G(roominst).reset();
	if (_G(debug_flags) & DBG_NOSCRIPT) ;
	else if (_GP(thisroom).CompiledScript != nullptr) {
		compile_room_script();
		if (_G(croom)->tsdatasize > 0) {
			if (_G(croom)->tsdatasize != (unsigned) _G(roominst)->globaldatasize)
				quit("room script data segment size has changed");
			memcpy(&_G(roominst)->globaldata[0], _G(croom)->tsdata.data(), _G(croom)->tsdatasize);
		}
	}
	set_our_eip(207);
	_GP(play).entered_edge = -1;

	if ((_G(new_room_x) != SCR_NO_VALUE) && (forchar != nullptr)) {
		forchar->x = _G(new_room_x);
		forchar->y = _G(new_room_y);
		if (_G(new_room_placeonwalkable))
			Character_PlaceOnWalkableArea(forchar);

		if (_G(new_room_loop) != SCR_NO_VALUE)
			forchar->loop = _G(new_room_loop);
	}

	// reset new_room instructions
	_G(new_room_x) = _G(new_room_y) = SCR_NO_VALUE;
	_G(new_room_loop) = SCR_NO_VALUE;
	_G(new_room_placeonwalkable) = false;

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

	set_our_eip(208);
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

	set_our_eip(209);
	generate_light_table();
	update_music_volume();

	// If we are not restoring a save, update cameras to accommodate for this
	// new room; otherwise this is done later when cameras are recreated.
	if (forchar != nullptr) {
		if (_GP(play).IsAutoRoomViewport())
			adjust_viewport_to_room();
		update_all_viewcams_with_newroom();
		_GP(play).UpdateRoomCameras(); // update auto tracking
	}
	init_room_drawdata();

	set_our_eip(212);
	invalidate_screen();
	for (size_t cc = 0; cc < _G(croom)->numobj; cc++) {
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

	set_our_eip(220);
	update_polled_stuff();
	debug_script_log("Now in room %d", _G(displayed_room));
	GUI::MarkAllGUIForUpdate(true, true);
	pl_run_plugin_hooks(AGSE_ENTERROOM, _G(displayed_room));
}

// new_room: changes the current room number, and loads the new room from disk
void new_room(int newnum, CharacterInfo *forchar) {
	EndSkippingUntilCharStops();

	debug_script_log("Room change requested to room %d", newnum);

	// we are currently running Leaves Screen scripts
	_G(in_leaves_screen) = newnum;

	// player leaves screen event
	run_room_event(EVROM_LEAVE);
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

	// change rooms
	unload_old_room();

	if (_GP(usetup).clear_cache_on_room_change) {
		// Delete all cached sprites
		_GP(spriteset).DisposeAllCached();
	}

	load_new_room(newnum, forchar);

	// Update background frame state (it's not a part of the RoomStatus currently)
	_GP(play).bg_frame = 0;
	_GP(play).bg_frame_locked = (_GP(thisroom).Options.Flags & kRoomFlag_BkgFrameLocked) != 0;
}

void set_room_placeholder() {
	_GP(thisroom).InitDefaults();
	std::shared_ptr<Bitmap> dummy_bg(new Bitmap(1, 1, 8));
	_GP(thisroom).BgFrames[0].Graphic = dummy_bg;
	_GP(thisroom).HotspotMask = dummy_bg;
	_GP(thisroom).RegionMask = dummy_bg;
	_GP(thisroom).WalkAreaMask = dummy_bg;
	_GP(thisroom).WalkBehindMask = dummy_bg;
	reset_temp_room();
	_G(croom) = &_GP(troom);
}

int find_highest_room_entered() {
	int qq, fndas = -1;
	for (qq = 0; qq < MAX_ROOMS; qq++) {
		if (isRoomStatusValid(qq) && (getRoomStatus(qq)->beenhere != 0))
			fndas = qq;
	}
	return fndas;
}

void first_room_initialization() {
	_G(starting_room) = _G(displayed_room);
	_G(playerchar)->prevroom = -1;
	set_loop_counter(0);
	_G(mouse_z_was) = _G(sys_mouse_z);
	// Reset background frame state
	_GP(play).bg_frame = 0;
	_GP(play).bg_frame_locked = (_GP(thisroom).Options.Flags & kRoomFlag_BkgFrameLocked) != 0;
}

void check_new_room() {
	// if they're in a new room, run Player Enters Screen and on_event(ENTER_ROOM)
	if ((_G(in_new_room) > 0) & (_G(in_new_room) != 3)) {
		EventHappened evh(EV_RUNEVBLOCK, EVB_ROOM, 0, EVROM_BEFOREFADEIN, _GP(game).playercharacter);
		// make sure that any script calls don't re-call enters screen
		int newroom_was = _G(in_new_room);
		_G(in_new_room) = 0;
		_GP(play).disabled_user_interface ++;
		process_event(&evh);
		_GP(play).disabled_user_interface --;
		_G(in_new_room) = newroom_was;
	}
}

void compile_room_script() {
	cc_clear_error();

	_G(roominst) = ccInstance::CreateFromScript(_GP(thisroom).CompiledScript);
	if (cc_has_error() || (_G(roominst) == nullptr)) {
		quitprintf("Unable to create local script:\n%s", cc_get_error().ErrorString.GetCStr());
	}

	if (!_G(roominst)->ResolveScriptImports(_G(roominst)->instanceof.get()))
		quitprintf("Unable to resolve imports in room script:\n%s", cc_get_error().ErrorString.GetCStr());

	if (!_G(roominst)->ResolveImportFixups(_G(roominst)->instanceof.get()))
		quitprintf("Unable to resolve import fixups in room script:\n%s", cc_get_error().ErrorString.GetCStr());

	_G(roominstFork) = _G(roominst)->Fork();
	if (_G(roominstFork) == nullptr)
		quitprintf("Unable to create forked room instance:\n%s", cc_get_error().ErrorString.GetCStr());

	_GP(repExecAlways).roomHasFunction = true;
	_GP(lateRepExecAlways).roomHasFunction = true;
	_GP(getDialogOptionsDimensionsFunc).roomHasFunction = true;
}

void on_background_frame_change() {
	invalidate_screen();
	mark_current_background_dirty();

	// get the new frame's palette
	memcpy(_G(palette), _GP(thisroom).BgFrames[_GP(play).bg_frame].Palette, sizeof(RGB) * 256);

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

// coordinate conversion (data) ---> game ---> (room mask)
int room_to_mask_coord(int coord) {
	return coord * _GP(game).GetDataUpscaleMult() / _GP(thisroom).MaskResolution;
}

// coordinate conversion (room mask) ---> game ---> (data)
int mask_to_room_coord(int coord) {
	return coord * _GP(thisroom).MaskResolution / _GP(game).GetDataUpscaleMult();
}

void convert_move_path_to_room_resolution(MoveList *ml, int from_step, int to_step) {
	if (to_step < 0)
		to_step = ml->numstage;
	to_step = CLIP(to_step, 0, ml->numstage - 1);
	from_step = CLIP(from_step, 0, to_step);

	// If speed is independent from MaskResolution...
	if ((_GP(game).options[OPT_WALKSPEEDABSOLUTE] != 0) && _GP(game).GetDataUpscaleMult() > 1) {
		for (int i = from_step; i <= to_step; i++) { // ...we still need to convert from game to data coords
			ml->xpermove[i] = game_to_data_coord(ml->xpermove[i]);
			ml->ypermove[i] = game_to_data_coord(ml->ypermove[i]);
		}
	}

	// Skip the conversion if these are equal, as they are multiplier and divisor
	if (_GP(thisroom).MaskResolution == _GP(game).GetDataUpscaleMult())
		return;

	if (from_step == 0) {
		ml->from = {mask_to_room_coord(ml->from.X), mask_to_room_coord(ml->from.Y)};
	}

	for (int i = from_step; i <= to_step; i++) {
		ml->pos[i] = {mask_to_room_coord(ml->pos[i].X), mask_to_room_coord(ml->pos[i].Y)};
	}

	// If speed is scaling with MaskResolution...
	if (_GP(game).options[OPT_WALKSPEEDABSOLUTE] == 0) {
		for (int i = from_step; i <= to_step; i++) {
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
	API_SCALL_OBJ_POBJ(const char, _GP(myScriptStringImpl), Room_GetTextProperty, const char);
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
	API_SCALL_OBJ_PINT(const char, _GP(myScriptStringImpl), Room_GetMessages);
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

RuntimeScriptValue Sc_Room_Exists(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL_PINT(Room_Exists);
}


void RegisterRoomAPI() {
	ScFnRegister room_api[] = {
		{"Room::GetDrawingSurfaceForBackground^1", API_FN_PAIR(Room_GetDrawingSurfaceForBackground)},
		{"Room::GetProperty^1", API_FN_PAIR(Room_GetProperty)},
		{"Room::GetTextProperty^1", API_FN_PAIR(Room_GetTextProperty)},
		{"Room::SetProperty^2", API_FN_PAIR(Room_SetProperty)},
		{"Room::SetTextProperty^2", API_FN_PAIR(Room_SetTextProperty)},
		{"Room::ProcessClick^3", API_FN_PAIR(RoomProcessClick)},
		{"ProcessClick", API_FN_PAIR(RoomProcessClick)},
		{"Room::get_BottomEdge", API_FN_PAIR(Room_GetBottomEdge)},
		{"Room::get_ColorDepth", API_FN_PAIR(Room_GetColorDepth)},
		{"Room::get_Height", API_FN_PAIR(Room_GetHeight)},
		{"Room::get_LeftEdge", API_FN_PAIR(Room_GetLeftEdge)},
		{"Room::geti_Messages", API_FN_PAIR(Room_GetMessages)},
		{"Room::get_MusicOnLoad", API_FN_PAIR(Room_GetMusicOnLoad)},
		{"Room::get_ObjectCount", API_FN_PAIR(Room_GetObjectCount)},
		{"Room::get_RightEdge", API_FN_PAIR(Room_GetRightEdge)},
		{"Room::get_TopEdge", API_FN_PAIR(Room_GetTopEdge)},
		{"Room::get_Width", API_FN_PAIR(Room_GetWidth)},
		{"Room::Exists", API_FN_PAIR(Room_Exists)},
	};

	ccAddExternalFunctions361(room_api);
}

} // namespace AGS3

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

#ifndef SCOOBY_ROOM_SCENE_LOADER_H
#define SCOOBY_ROOM_SCENE_LOADER_H

#include "common/scummsys.h"

#include "room_actor_initializer.h"
#include "room_interface_transition_executor.h"
#include "room_tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/action_menu_controller.h"
#include "scooby/runtime/runtime_state.h"

// Owns active-room asset decoding, logical publication, coordinate tables, and initial placement.

namespace Scooby {

class RoomSceneLoader {
public:
	// Binds room loading to the verified cartridge, logical scene, and direct child owners.
	// rom: verified cartridge data containing room descriptors and assets.
	// scene: logical pattern and layer destination replacing VRAM publication.
	// state: shared room, actor, palette, and interaction state.
	// actionMenu: owner of inventory composition and action-prompt publication.
	// actorInitializer: owner of built-in and object-backed room actor initialization.
	// roomInterfaceTransitions: owner of lower-interface save and clear behavior.
	// tileStreamer: owner of camera-relative room-plane publication.
	RoomSceneLoader(const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
					ActionMenuController &actionMenu, RoomActorInitializer &actorInitializer,
					RoomInterfaceTransitionExecutor &roomInterfaceTransitions, RoomTileStreamer &tileStreamer)
		: _rom(rom), _scene(scene), _state(state), _actionMenu(actionMenu), _actorInitializer(actorInitializer),
		  _roomInterfaceTransitions(roomInterfaceTransitions), _tileStreamer(tileStreamer) {
	}

	// Loads every logical resource and initial state required by the selected room.
	//
	// Ghidra: loadRoomScene (0x0000758A). Room ID minus two is sign-extended for the
	// 128-byte palette stride but remains an unsigned word for the 20-byte room-record multiply. The three
	// room streams decode to independent background, foreground, and collision resources; their relative
	// prefixes locate the following stream and immutable coordinate tables. The cleared logical pattern set
	// replaces the full VRAM clear, so undefined stale decompression-workspace bytes below the original
	// 0x400-byte minimum are deliberately represented as blank patterns while retaining the allocation branch.
	void loadRoomScene();

private:
	static const int kActionPromptTilesOffset = 0x30916;
	static const int kBlankTileByteCount = 0x20;
	static const int kCursorSpriteTileByteCount = 0x80;
	static const int kCursorSpriteTilesOffset = 0x31AFA;
	static const int kDefaultSharedPaletteOffset = 0x32480;
	static const int kEpisodeAssetBaseField = 0x0C;
	static const int kEpisodeLayoutBaseField = 0x10;
	static const int kEpisodePaletteBaseField = 0x14;
	static const int kEpisodeRoomNameBaseField = 0x20;
	static const int kEpisodeRoomRecordTableField = 0x08;
	static const int kInterfaceTemplateColumnCount = 64;
	static const int kInterfaceTemplateExplicitColumnCount = 61;
	static const int kInterfaceTemplateOffset = 0x30016;
	static const int kInterfaceTemplateRowCount = 6;
	static const int kInterfaceTemplateStartRow = 21;
	static const int kMaskTileByteCount = 0xBE0;
	static const int kMinimumRoomTileByteCount = 0x400;
	static const int kPositionCoordinateTableByteCount = 0x20;
	static const uint16 kPriorityAttribute = 0x8000;
	static const int kRoomPaletteByteStride = 0x80;
	static const int kRoomPaletteOffsetWithinRecord = 0x40;
	static const int kRoomRecordSize = 0x14;
	static const int kShortInterfaceSpriteTileByteCount = 0x100;
	static const int kShortInterfaceSpriteTilesOffset = 0x31CFA;
	static const int kTallInterfaceSpriteAdvanceByteCount = 0xC0;
	static const int kTallInterfaceSpriteTileByteCount = 0x180;
	static const int kTallInterfaceSpriteTilesOffset = 0x31B7A;
	static const int kViewportHeightTiles = 19;
	static const int kViewportWidthTiles = 32;

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	ActionMenuController &_actionMenu;
	RoomActorInitializer &_actorInitializer;
	RoomInterfaceTransitionExecutor &_roomInterfaceTransitions;
	RoomTileStreamer &_tileStreamer;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCENE_LOADER_H

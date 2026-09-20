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

#ifndef SCOOBY_ROOM_SPRITE_RENDERER_H
#define SCOOBY_ROOM_SPRITE_RENDERER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/sprite_instance.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns actor tile publication and construction of the room's logical sprite table.

namespace Scooby {

class RoomSpriteRenderer {
public:
	// Binds staged actor graphics and sprite state to their logical scene destination.
	// rom: verified cartridge containing the scripted sprite's authored coordinates.
	// scene: logical tile and sprite destination replacing actor VRAM and sprite-table transfers.
	// state: six-slot actor graphics, publication flags, and animation delays.
	RoomSpriteRenderer(const ScoobyDooRom &rom, TileScene &scene,
					   RuntimeState &state)
		: _rom(rom), _scene(scene), _state(state) {
	}

	// Publishes one pending actor tile bank, rebuilds room sprites, and advances actor delays.
	//
	// Ghidra: refreshRoomSprites (0x0000AF24). At most the first active pending slot is published per
	// invocation. Native VDP-counter branches gate DMA timing only; logical scene mutation is atomic and does
	// not retain the stale D0 value that participates in that hardware-only decision.
	void refreshRoomSprites();

private:
	static const int kActorCount = 6;
	static const int kScriptedSpriteCoordinateTableOffset = 0x323BE;
	static const int kScriptedSpriteYCoordinateByteOffset = 0x48;
	static const int kVisibleCoordinateBias = 0x80;

	// Preserves the recovered per-family cumulative-coordinate, tile, and shape table geometry shared by every
	// actor sprite layout family, replacing the C# "private readonly record struct ActorSpriteLayout".
	struct ActorSpriteLayout {
		int PartCount;
		int UnflippedXOffset;
		int FlippedXOffset;
		int YOffset;
		int TileOffset;
		int ShapeOffset;

		ActorSpriteLayout(int partCount, int unflippedXOffset, int flippedXOffset, int yOffset, int tileOffset,
						  int shapeOffset)
			: PartCount(partCount), UnflippedXOffset(unflippedXOffset), FlippedXOffset(flippedXOffset),
			  YOffset(yOffset), TileOffset(tileOffset), ShapeOffset(shapeOffset) {
		}
	};

	// Executable actor-tile destinations at ROM 0x000FC860-0x000FC86B, converted from byte addresses to tiles.
	static const Common::Array<int> kActorTileDestinationIndices;

	// Ghidra g_awActorSpriteLayout70* arrays: X 0xFC86C-0xFC88B, Y 0xFC88C-0xFC89B, tiles 0xFC968-0xFC977, and
	// shapes 0xFC9C0-0xFC9CF.
	static const ActorSpriteLayout kActorSpriteLayout70;

	// Ghidra g_awActorSpriteLayout40* arrays: X 0xFC92C-0xFC93B, Y 0xFC93C-0xFC943, tiles 0xFC984-0xFC98B, and
	// shapes 0xFC9DC-0xFC9E3.
	static const ActorSpriteLayout kActorSpriteLayout40;

	// Ghidra g_awActorSpriteLayout30* arrays: X 0xFC8C0-0xFC8D7, Y 0xFC8D8-0xFC8E3, tiles 0xFC950-0xFC95B, and
	// shapes 0xFC9A8-0xFC9B3.
	static const ActorSpriteLayout kActorSpriteLayout30;

	// Shared Ghidra g_awSixPartSprite* arrays: X 0xFC89C-0xFC8B3, Y 0xFC8B4-0xFC8BF, tiles 0xFC944-0xFC94F, and
	// shapes 0xFC99C-0xFC9A7.
	static const ActorSpriteLayout kActorSpriteLayoutDefault;

	// Ghidra g_awActorSpriteLayout20x50* arrays: X 0xFC8E4-0xFC8FB, Y 0xFC8FC-0xFC907, tiles 0xFC95C-0xFC967,
	// and shapes 0xFC9B4-0xFC9BF.
	static const ActorSpriteLayout kActorSpriteLayout20x50;

	// Ghidra g_awActorSpriteLayout20* arrays: X 0xFC908-0xFC91F, Y 0xFC920-0xFC92B, tiles 0xFC978-0xFC983, and
	// shapes 0xFC9D0-0xFC9DB.
	static const ActorSpriteLayout kActorSpriteLayout20;

	// Builds and atomically publishes every enabled room sprite in original priority order.
	//
	// Ghidra: buildRoomSpriteTable (0x0000A984). Native link indices and the shared 0xFF0008 scratch table
	// become ordered logical sprites. Function-only globals g_bPreviousActorSpritePresenceFlags at 0xFF0AC5 and
	// g_bActorSpriteBuildCandidateFlags at 0xFF0ABA become invocation locals. ROM g_awScriptedSpriteCoordinates
	// at 0x000323BE-0x0003244D remains cartridge-owned as thirty-six X words followed by thirty-six Y words at
	// byte offset 0x48.
	void buildRoomSpriteTable();

	void appendCursorOrLowerInterfaceSprite(Common::Array<SpriteInstance> &sprites);
	void appendScriptedCoordinateSprite(Common::Array<SpriteInstance> &sprites);
	void appendInteractionStripSprites(Common::Array<SpriteInstance> &sprites);
	void appendDuelOverlaySprites(Common::Array<SpriteInstance> &sprites);
	static void appendFixedTerminalSprites(Common::Array<SpriteInstance> &sprites);
	static void addPackedSprite(Common::Array<SpriteInstance> &sprites, uint16 packedX,
								uint16 packedY, uint16 sizeAttributes,
								uint16 tileAttributes);

	// Appends one actor's published frame decomposition to the room sprite list.
	// sprites: ordered logical table receiving every visible frame part.
	// actor: selected six-slot actor index.
	//
	// Ghidra: appendRoomActorSprites (0x0000AD16). The 30 named layout arrays spanning ROM
	// 0x000FC86C-0x000FC9E3 provide signed cumulative coordinates, tile offsets, and packed shapes; managed
	// code reads only the complete family selected by the authored frame dimensions.
	void appendRoomActorSprites(Common::Array<SpriteInstance> &sprites, int actor);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SPRITE_RENDERER_H

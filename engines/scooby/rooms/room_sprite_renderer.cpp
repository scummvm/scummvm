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

#include "room_sprite_renderer.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"

namespace Scooby {

const Common::Array<int> RoomSpriteRenderer::kActorTileDestinationIndices =
	{0x780, 0x7BC, 0x4D8, 0x518, 0x49C, 0x460};

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayout70(
	8, 0xFC86C, 0xFC87C, 0xFC88C,
	0xFC968, 0xFC9C0);

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayout40(
	4, 0xFC92C, 0xFC934, 0xFC93C,
	0xFC984, 0xFC9DC);

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayout30(
	6, 0xFC8C0, 0xFC8CC, 0xFC8D8,
	0xFC950, 0xFC9A8);

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayoutDefault(6, 0xFC89C, 0xFC8A8,
																						  0xFC8B4, 0xFC944, 0xFC99C);

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayout20x50(
	6, 0xFC8E4, 0xFC8F0, 0xFC8FC,
	0xFC95C, 0xFC9B4);

const RoomSpriteRenderer::ActorSpriteLayout RoomSpriteRenderer::kActorSpriteLayout20(
	6, 0xFC908, 0xFC914, 0xFC920,
	0xFC978, 0xFC9D0);

void RoomSpriteRenderer::refreshRoomSprites() {
	// Ghidra 0x0000AF24-0x0000AF4D: initialize the ascending six-slot traversal and snapshot the active,
	// pending-upload, and graphics-ready masks. The unused VDP command word has no managed state.
	uint8 pendingFlags = _state.ActorTileUploadPendingFlags;
	uint8 readyFlags = _state.ActorGraphicsReadyFlags;
	for (int actor = 0; actor < kActorCount; actor++) {
		uint8 actorMask = static_cast<uint8>(1 << actor);

		// Ghidra 0x0000AF4E-0x0000AF79: inactive slots skip all work. In transition mode, the selected actor
		// and the stale-D0-zero path bypass the VDP counter; all other paths require 0xE000-0xF000. The
		// managed scene has no unsafe DMA interval, so every active path reaches the same logical check.
		if ((_state.ActiveActorFlags & actorMask) == 0) {
			continue;
		}

		// Transition-mode clear, selected-actor equality, stale-D0 zero, and both VDP range outcomes are all
		// retained in the contract above. They select only the replaced hardware timing gate; native D0 is an
		// incidental prior-call clobber and is not promoted into managed state.

		// Ghidra 0x0000AF7A-0x0000AF93: a clear pending bit continues to the next slot. Otherwise publish the
		// exact staged extent, clear both publication latches, set ready, and terminate the actor scan.
		if ((pendingFlags & actorMask) == 0) {
			continue;
		}

		std::size_t transferByteCount = static_cast<std::size_t>(
			_state.ActorTileTransferWordCounts[static_cast<std::size_t>(actor)] * sizeof(uint16));
		Common::Array<uint8> &actorTileData = _state.ActorTileData[static_cast<std::size_t>(actor)];
		_scene.loadTilesAt(MakeSpan(actorTileData).slice(0, transferByteCount),
						   kActorTileDestinationIndices[static_cast<std::size_t>(actor)]);
		pendingFlags &= static_cast<uint8>(~actorMask);
		_state.ActorFramePublicationLatchFlags &= static_cast<uint8>(~actorMask);
		readyFlags |= actorMask;
		break;
	}

	// Ghidra 0x0000AF94-0x0000AFA1: the managed loop performs the same pointer/table advance for every
	// inactive or nonpending slot and exits after slot five when no upload was selected.

	// Ghidra 0x0000AFA2-0x0000AFAD: commit the local pending and ready snapshots after either termination.
	_state.ActorTileUploadPendingFlags = pendingFlags;
	_state.ActorGraphicsReadyFlags = readyFlags;

	// Ghidra 0x0000AFAE-0x0000AFB1: rebuild the complete room sprite table after tile publication.
	buildRoomSpriteTable();

	// Ghidra 0x0000AFB2-0x0000AFC3: replace the 0xC8-word scratch-to-VDP sprite-table transfer with the logical
	// publication owned by BuildRoomSpriteTable; no shared Work RAM or VRAM table is retained here.

	// Ghidra 0x0000AFC4-0x0000AFD9: visit all six signed delays independently, decrement only positive values,
	// preserve zero and negative values, and return.
	for (int actor = 0; actor < kActorCount; actor++) {
		if (_state.ActorAnimationDelays[static_cast<std::size_t>(actor)] > 0) {
			_state.ActorAnimationDelays[static_cast<std::size_t>(actor)]--;
		}
	}
}

void RoomSpriteRenderer::buildRoomSpriteTable() {
	// Ghidra 0x0000A984-0x0000A99D: snapshot the preceding actor-presence mask, clear the current mask, start
	// link numbering at one, and replace the shared sprite scratch cursor with an ordered list.
	uint8 previousActorSpritePresenceFlags = _state.ActorSpritePresenceFlags;
	_state.ActorSpritePresenceFlags = 0;
	Common::Array<SpriteInstance> sprites;

	// Ghidra 0x0000A99E-0x0000AAB5: retain the independent hidden, direct-cursor, transition-suppressed,
	// short-interface, and tall-interface branches, including signed snapping and cursor-step updates.
	appendCursorOrLowerInterfaceSprite(sprites);

	// Ghidra 0x0000AAB6-0x0000AB3D: optionally append the coordinate-driven sprite, advance or clamp its
	// authored cursor, and service its signed frame timer and two-frame phase.
	appendScriptedCoordinateSprite(sprites);

	// Ghidra 0x0000AB3E-0x0000AB91: independently gate and append all eight interaction-strip sprites.
	appendInteractionStripSprites(sprites);

	// Ghidra 0x0000AB92-0x0000AC09: independently gate and append all four duel-overlay sprites.
	appendDuelOverlaySprites(sprites);

	// Ghidra 0x0000AC0A-0x0000AC47: preserve the four fixed linked-table records in their authored order.
	appendFixedTerminalSprites(sprites);

	// Ghidra 0x0000AC48-0x0000AC67: derive the six-slot actor candidate mask from active and blocked flags.
	uint8 actorSpriteBuildCandidateFlags =
		static_cast<uint8>(_state.ActiveActorFlags & ~_state.ActorVisibilityBlockFlags);
	while (actorSpriteBuildCandidateFlags != 0) {
		// Ghidra 0x0000AC94-0x0000ACC9: scan slots five through zero. Transition bit two selects the first
		// candidate immediately; otherwise select greatest signed integer Y and retain the higher-slot tie.
		int selectedActor = kActorCount - 1;
		int16 greatestY = static_cast<int16>(-0x7FFF);
		for (int actor = kActorCount - 1; actor >= 0; actor--) {
			uint8 actorMask = static_cast<uint8>(1 << actor);
			if ((actorSpriteBuildCandidateFlags & actorMask) == 0) {
				continue;
			}

			if ((_state.TransitionFlags & 0x04) != 0) {
				selectedActor = actor;
				break;
			}

			int16 actorY =
				static_cast<int16>(_state.ActorYFixedCoordinates[static_cast<std::size_t>(actor)] >> 16);
			if (greatestY >= actorY) {
				continue;
			}

			selectedActor = actor;
			greatestY = actorY;
		}

		// Ghidra 0x0000ACCA-0x0000AD13: consume the selected candidate, retain prior sprite state until
		// replacement graphics are ready, mark appended actors, and delegate their complete expansion.
		uint8 selectedActorMask = static_cast<uint8>(1 << selectedActor);
		actorSpriteBuildCandidateFlags &= static_cast<uint8>(~selectedActorMask);
		if ((previousActorSpritePresenceFlags & selectedActorMask) == 0) {
			if ((_state.ActorGraphicsReadyFlags & selectedActorMask) == 0) {
				continue;
			}
		}

		_state.ActorSpritePresenceFlags |= selectedActorMask;
		appendRoomActorSprites(sprites, selectedActor);
	}

	// Ghidra 0x0000AC68-0x0000AC93: every actor path returns to the candidate test. Terminate the preceding
	// logical link when records exist, or clear the complete table through the separate empty-table branch.
	// MakeSpan on an empty vector safely yields an empty span, so the empty and non-empty C# branches collapse
	// into one publication call here.
	_scene.replaceSprites(MakeSpan(sprites));
}

void RoomSpriteRenderer::appendCursorOrLowerInterfaceSprite(Common::Array<SpriteInstance> &sprites) {
	if ((_state.DisplayFlags & 0x08) == 0) {
		return;
	}

	if (_state.CursorY < 0x00A8) {
		_state.CursorHorizontalStep = 2;
		_state.CursorVerticalStep = 2;
		addPackedSprite(sprites, static_cast<uint16>(_state.CursorX + kVisibleCoordinateBias),
						static_cast<uint16>(_state.CursorY + kVisibleCoordinateBias), 0x0500,
						_state.CursorSpriteTileAttributes);
		return;
	}

	if (_state.RoomInterfaceTransitionCountdown >= 0) {
		return;
	}

	int16 packedY = 0x0128;
	if (_state.CursorY >= 0x00C0) {
		packedY = 0x0140;
	} else if ((_state.DisplayFlags & 0x10) == 0) {
		packedY = static_cast<int16>(packedY + 8);
	}

	int16 remainingX = _state.CursorX;
	int16 snappedX = 0;
	while (true) {
		remainingX = static_cast<int16>(remainingX - 0x28);
		if (remainingX < 0) {
			break;
		}

		snappedX = static_cast<int16>(snappedX + 0x28);
	}

	uint16 sizeAttributes;
	uint16 tileAttributes;
	if ((_state.DisplayFlags & 0x10) == 0) {
		snappedX = static_cast<int16>(snappedX + 8);
		if (snappedX > 0x00A8) {
			snappedX = 0x00A8;
		}

		sizeAttributes = 0x0D00;
		tileAttributes = _state.ShortInterfaceSpriteTileIndex;
	} else {
		if (snappedX > 0x00C8) {
			snappedX = 0x00C8;
		}

		if (snappedX == 0) {
			snappedX = 0x28;
		}

		sizeAttributes = 0x0E00;
		tileAttributes = _state.TallInterfaceSpriteTileIndex;
	}

	if (_state.CursorHorizontalStep != 2) {
		_state.CursorX = static_cast<int16>(snappedX + 0x14);
	}

	addPackedSprite(sprites, static_cast<uint16>(snappedX + 0x88), static_cast<uint16>(packedY),
					sizeAttributes, tileAttributes);
}

void RoomSpriteRenderer::appendScriptedCoordinateSprite(Common::Array<SpriteInstance> &sprites) {
	if ((_state.RoomBehaviorFlags & 0x10) == 0) {
		return;
	}

	int32 coordinateOffset = _state.ScriptedCoordinateSprite._coordinateOffset;
	uint16 packedX = static_cast<uint16>(
		_rom.readUInt16(kScriptedSpriteCoordinateTableOffset + coordinateOffset) + 0x90);
	uint16 packedY = static_cast<uint16>(
		_rom.readUInt16(kScriptedSpriteCoordinateTableOffset + kScriptedSpriteYCoordinateByteOffset +
						coordinateOffset) +
		0x90);
	coordinateOffset += static_cast<int32>(sizeof(uint16));
	_state.ScriptedCoordinateSprite._coordinateOffset = coordinateOffset;
	if ((_state.RoomBehaviorFlags & 0x20) != 0 && coordinateOffset == kScriptedSpriteYCoordinateByteOffset) {
		_state.ScriptedCoordinateSprite._coordinateOffset -= static_cast<int32>(sizeof(uint16));
	}

	_state.ScriptedCoordinateSprite._frameDelay =
		static_cast<int8>(_state.ScriptedCoordinateSprite._frameDelay - 1);
	if (_state.ScriptedCoordinateSprite._frameDelay < 0) {
		_state.ScriptedCoordinateSprite._frameDelay = 7;
		_state.RoomBehaviorFlags ^= 0x40;
	}

	int frameTileOffset = (_state.RoomBehaviorFlags & 0x40) == 0 ? 0 : 4;
	addPackedSprite(sprites, packedX, packedY, 0x0500,
					static_cast<uint16>(_state.ScriptedCoordinateSprite._tileAttributes +
										frameTileOffset));
}

void RoomSpriteRenderer::appendInteractionStripSprites(Common::Array<SpriteInstance> &sprites) {
	if ((_state.InteractionFlags & 0x04) == 0) {
		return;
	}

	if (_state.InteractionStripColumn < 0) {
		return;
	}

	uint16 packedX = static_cast<uint16>((_state.InteractionStripColumn << 3) + 0x80);
	uint16 packedY = static_cast<uint16>((_state.InteractionStripRow << 3) + 0x128);
	uint16 sizeAttributes = static_cast<uint16>(0x0C00 | _state.InteractionStripHeightAttributes);
	for (int sprite = 0; sprite < 8; sprite++) {
		addPackedSprite(sprites, packedX, packedY, sizeAttributes, _state.TallInterfaceSpriteTileIndex);
		packedX = static_cast<uint16>(packedX + 0x20);
	}
}

void RoomSpriteRenderer::appendDuelOverlaySprites(Common::Array<SpriteInstance> &sprites) {
	if ((_state.DuelStateFlags & 0x01) == 0) {
		return;
	}

	addPackedSprite(sprites, 0x00B0, 0x0130, 0x0C00, 0x0518);
	addPackedSprite(sprites, 0x00D0, 0x0130, 0x0C00, 0x051C);
	addPackedSprite(sprites, 0x0110, 0x0130, 0x0C00, 0x0520);
	addPackedSprite(sprites, 0x0130, 0x0130, 0x0C00, 0x0524);
}

void RoomSpriteRenderer::appendFixedTerminalSprites(Common::Array<SpriteInstance> &sprites) {
	addPackedSprite(sprites, 1, 0x0128, 0x0300, 0);
	addPackedSprite(sprites, 0, 0x0128, 0x0300, 0);
	addPackedSprite(sprites, 1, 0x0148, 0x0300, 0);
	addPackedSprite(sprites, 0, 0x0148, 0x0300, 0);
}

void RoomSpriteRenderer::addPackedSprite(Common::Array<SpriteInstance> &sprites, uint16 packedX,
										 uint16 packedY, uint16 sizeAttributes,
										 uint16 tileAttributes) {
	sprites.push_back(SpriteInstance(
		static_cast<int32>((packedX & 0x01FF) - kVisibleCoordinateBias),
		static_cast<int32>((packedY & 0x03FF) - kVisibleCoordinateBias),
		static_cast<int32>((sizeAttributes >> 10 & 3) + 1),
		static_cast<int32>((sizeAttributes >> 8 & 3) + 1),
		static_cast<int32>(tileAttributes & 0x07FF), static_cast<uint8>(tileAttributes >> 13 & 3),
		(tileAttributes & 0x0800) != 0, (tileAttributes & 0x1000) != 0, (tileAttributes & 0x8000) != 0));
}

void RoomSpriteRenderer::appendRoomActorSprites(Common::Array<SpriteInstance> &sprites, int actor) {
	uint8 actorMask = static_cast<uint8>(1 << actor);
	int baseTileIndex = kActorTileDestinationIndices[static_cast<std::size_t>(actor)];

	// Ghidra 0x0000AD16-0x0000AD53: derive the actor's base tile. Preserve all three independent pending gates
	// before latching and snapshotting pending frame identity, scale, and compact-frame anchors.
	if ((_state.ActorFramePublicationLatchFlags & actorMask) == 0) {
		if ((_state.ActorCompactConversionPendingFlags & actorMask) == 0) {
			if ((_state.ActorTileUploadPendingFlags & actorMask) == 0) {
				_state.ActorFramePublicationLatchFlags |= actorMask;
				_state.ActorCurrentFrameDataOffsets[static_cast<std::size_t>(actor)] =
					_state.ActorPendingFrameDataOffsets[static_cast<std::size_t>(actor)];
				_state.ActorPreviousHorizontalScales[static_cast<std::size_t>(actor)] =
					_state.ActorHorizontalScaleFactors[static_cast<std::size_t>(actor)];
				_state.ActorPreviousVerticalScales[static_cast<std::size_t>(actor)] =
					_state.ActorVerticalScaleFactors[static_cast<std::size_t>(actor)];
				_state.ActorPublishedFrameAnchorX[static_cast<std::size_t>(actor)] =
					_state.ActorFrameAnchorX[static_cast<std::size_t>(actor)];
				_state.ActorPublishedFrameAnchorY[static_cast<std::size_t>(actor)] =
					_state.ActorFrameAnchorY[static_cast<std::size_t>(actor)];
			}
		}
	}

	// Ghidra 0x0000AD54-0x0000AD59: a negative frame-pointer high byte terminates this actor immediately.
	if (!_state.ActorCurrentFrameDataOffsets[static_cast<std::size_t>(actor)].hasValue()) {
		return;
	}
	int32 frameDataOffset = _state.ActorCurrentFrameDataOffsets[static_cast<std::size_t>(actor)].value();

	// Ghidra 0x0000AD5A-0x0000AD8F: derive signed camera-relative coordinates and retain each of the four
	// viewport rejection branches independently.
	int16 relativeX = static_cast<int16>(
		static_cast<int16>(_state.ActorXFixedCoordinates[static_cast<std::size_t>(actor)] >> 16) -
		_state.CameraX);
	int16 relativeY = static_cast<int16>(
		static_cast<int16>(_state.ActorYFixedCoordinates[static_cast<std::size_t>(actor)] >> 16) -
		_state.CameraY);
	if (relativeX < -0x50) {
		return;
	}

	if (relativeY < -0x50) {
		return;
	}

	if (relativeX > 0x150) {
		return;
	}

	if (relativeY > 0xE8) {
		return;
	}

	// Ghidra 0x0000AD90-0x0000ADAF: apply packed coordinate biases, then select published compact anchors or
	// the direct frame record's authored X/Y anchors through separate branches.
	int16 packedX = static_cast<int16>(relativeX + 0x80);
	int16 packedY = static_cast<int16>(relativeY + 0x90);
	if ((_state.ActorCompactFrameFlags & actorMask) != 0) {
		packedX = static_cast<int16>(packedX -
									 _state.ActorPublishedFrameAnchorX[static_cast<std::size_t>(actor)]);
		packedY = static_cast<int16>(packedY -
									 _state.ActorPublishedFrameAnchorY[static_cast<std::size_t>(actor)]);
	} else {
		packedX = static_cast<int16>(packedX - _rom.readInt16(frameDataOffset));
		packedY =
			static_cast<int16>(packedY - _rom.readInt16(frameDataOffset + static_cast<int>(sizeof(int16))));
	}

	// Ghidra 0x0000ADB0-0x0000AEEB: preserve all six dimension families, including the nested 0x20 height split
	// and its non-0x50-only vertical displacement. Each family retains both horizontal-flip tables.
	uint8 frameWidth = _rom.readByte(frameDataOffset + 4);
	uint8 frameHeight = _rom.readByte(frameDataOffset + 5);
	const ActorSpriteLayout *layout;
	switch (frameWidth) {
	case 0x70:
		layout = &kActorSpriteLayout70;
		break;
	case 0x40:
		layout = &kActorSpriteLayout40;
		break;
	case 0x30:
		layout = &kActorSpriteLayout30;
		break;
	case 0x20:
		if (frameHeight == 0x50) {
			layout = &kActorSpriteLayout20x50;
		} else {
			packedY = static_cast<int16>(packedY - 0x4C);
			layout = &kActorSpriteLayout20;
		}

		break;
	default:
		layout = &kActorSpriteLayoutDefault;
		break;
	}

	uint16 frameAttributes = _rom.readUInt16(frameDataOffset + 10);
	int xOffsetTableOffset;
	if ((frameAttributes & 0x0800) == 0) {
		xOffsetTableOffset = layout->UnflippedXOffset;
	} else {
		xOffsetTableOffset = layout->FlippedXOffset;
	}

	// Ghidra 0x0000AEEC-0x0000AF21: consume every selected table entry with 16-bit cumulative arithmetic,
	// combine slot tile base, authored tile offset, frame attributes, and actor priority, and append in order.
	for (int part = 0; part < layout->PartCount; part++) {
		int tableByteOffset = part * static_cast<int>(sizeof(uint16));
		packedX = static_cast<int16>(packedX + _rom.readInt16(xOffsetTableOffset + tableByteOffset));
		packedY = static_cast<int16>(packedY + _rom.readInt16(layout->YOffset + tableByteOffset));
		uint16 tileAttributes =
			static_cast<uint16>(baseTileIndex + _rom.readUInt16(layout->TileOffset + tableByteOffset));
		tileAttributes = static_cast<uint16>(tileAttributes | frameAttributes);
		if ((_state.ActorHighPriorityFlags & actorMask) != 0) {
			tileAttributes = static_cast<uint16>(tileAttributes | 0x8000);
		}

		addPackedSprite(sprites, static_cast<uint16>(packedX), static_cast<uint16>(packedY),
						_rom.readUInt16(layout->ShapeOffset + tableByteOffset), tileAttributes);
	}

	// Ghidra 0x0000AF22-0x0000AF23: return the advanced logical list to actor arbitration.
}
} // namespace Scooby
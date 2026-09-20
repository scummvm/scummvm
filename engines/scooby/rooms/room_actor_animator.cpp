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

#include "room_actor_animator.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/compression/byte_rle_decoder.h"

namespace Scooby {
namespace {
// Executable transition-animation lookup at ROM 0x000FC760-0x000FC85F.
const Common::Array<int16> kTransitionAnimationByOffset = {
	0, 1, 3, 2, 4, 5, 7, 6, 8, 9, 11, 10, 12, 13, 15, 14, 16, 17, 19, 18, 20, 21,
	23, 22, 26, 27, 24, 25, 30, 31, 28, 29, 32, 33, 103, 104, 36, 37, 38, 39, 40, 41, 42, 43,
	44, 45, 46, 47, 48, 49, 51, 50, 52, 53, 55, 54, 58, 59, 56, 57, 62, 63, 60, 61, 0, 1,
	3, 2, 4, 5, 7, 6, 8, 9, 11, 10, 12, 13, 15, 14, 16, 17, 19, 18, 20, 21, 23, 22,
	26, 27, 24, 25, 30, 31, 28, 29, 32, 33, 103, 104, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 51, 50, 52, 53, 55, 54, 58, 59, 56, 57, 62, 63, 60, 61};
} // namespace

void RoomActorAnimator::updateActorAnimationFrames() {
	// Ghidra 0x0000B0B8-0x0000B0F7: couple actor zero's restart to transition actor slot two.
	coupleTransitionAnimation();

	// Ghidra 0x0000B0F8-0x0000B1A5: visit active, due actors in reverse slot order and reject
	// transition-ready or offscreen slots that have no explicit restart or position update.
	// Ghidra 0x0000B1A6-0x0000B2E9: restart eligible streams and interpret their four-byte commands.
	// Ghidra 0x0000B2EA-0x0000B37F: resolve selected frame records, decode direct tile streams, and stage
	// compact sources with their complete transfer extents.
	for (int actor = kActorCount - 1; actor >= 0; --actor) {
		uint8 actorMask = static_cast<uint8>(1 << actor);
		if (canAdvanceActor(actor, actorMask)) {
			advanceActor(actor, actorMask);
		}
	}

	// Ghidra 0x0000B380-0x0000B4F1: decode and dispatch every eligible compact frame while retaining a
	// transition pair until both selected frames are ready.
	convertCompactFrames();

	// Ghidra 0x0000B4F2-0x0000B5B9: synchronize a completed transition pair, derive its paired position,
	// publish both tile banks through the installed VBlank callback, and wait until both pending bits clear.
	publishTransitionPair();

	// Ghidra 0x0000B5BA-0x0000B5BE: the original restores all registers and returns without a result.
}

void RoomActorAnimator::coupleTransitionAnimation() {
	if ((_state.TransitionFlags & kTransitionMask) == 0 || (_state.ActorAnimationRestartFlags & 0x01) == 0) {
		return;
	}

	_state.ActorAnimationDelays[2] = _state.ActorAnimationDelays[0];
	_state.ActorAnimationOffsets[2] =
		kTransitionAnimationByOffset[static_cast<std::size_t>(static_cast<uint16>(_state.ActorAnimationOffsets[0]))];
	_state.ActorAnimationRestartFlags |= 0x04;
}

bool RoomActorAnimator::canAdvanceActor(int actor, uint8 actorMask) const {
	if ((_state.ActiveActorFlags & actorMask) == 0 || _state.ActorAnimationDelays[actor] > 0 ||
		((_state.RoomBehaviorFlags & kTransitionMask) != 0 &&
		 (_state.ActorTransitionFrameReadyFlags & actorMask) != 0)) {
		return false;
	}

	if ((_state.ActorAnimationRestartFlags & actorMask) != 0 ||
		(_state.ActorPositionUpdateFlags & actorMask) != 0 || actor == 1) {
		return true;
	}

	int16 screenX = static_cast<int16>(
		static_cast<int16>(_state.ActorXFixedCoordinates[actor] >> 16) - _state.CameraX);
	int16 screenY = static_cast<int16>(
		static_cast<int16>(_state.ActorYFixedCoordinates[actor] >> 16) - _state.CameraY);
	return screenX >= -80 && screenX <= 336 && screenY >= -80 && screenY <= 232;
}

void RoomActorAnimator::advanceActor(int actor, uint8 actorMask) {
	int descriptorOffset = _state.ActorAnimationDescriptorOffsets[actor];
	if ((_state.ActorAnimationRestartFlags & actorMask) != 0) {
		if ((_state.ActorTileUploadPendingFlags & actorMask) != 0 ||
			(_state.ActorCompactConversionPendingFlags & actorMask) != 0) {
			return;
		}

		_state.ActorAnimationHoldFlags &= static_cast<uint8>(~actorMask);
		_state.ActorGraphicsReadyFlags &= static_cast<uint8>(~actorMask);
		int16 animationTableOffset = static_cast<int16>(
			_rom.readUInt16(descriptorOffset + 2) +
			_state.ActorAnimationOffsets[actor] * static_cast<int>(sizeof(int16)));
		_state.ActorAnimationCommandOffsets[actor] = _rom.readInt16(descriptorOffset + animationTableOffset);
		_state.ActorAnimationDelays[actor] = -1;
		_state.ActorAnimationRestartFlags &= static_cast<uint8>(~actorMask);
	}

	if ((_state.ActorAnimationHoldFlags & actorMask) != 0) {
		if ((_state.ActorPositionUpdateFlags & actorMask) == 0) {
			return;
		}

		_state.ActorPositionUpdateFlags &= static_cast<uint8>(~actorMask);
		stageActorFrame(actor, actorMask, descriptorOffset, _state.ActorSelectedFrameOffsets[actor]);
		return;
	}

	while (true) {
		int16 commandOffset = _state.ActorAnimationCommandOffsets[actor];
		_state.ActorAnimationCommandOffsets[actor] = static_cast<int16>(commandOffset + 4);
		int16 argument = _rom.readInt16(descriptorOffset + commandOffset + 2);
		switch (_rom.readInt16(descriptorOffset + commandOffset)) {
		case 4:
			_state.ActorStagedAnimationDelays[actor] = argument;
			continue;
		case 5:
			_state.ActorAnimationDelays[actor] = argument;
			return;
		case 3:
			if ((_state.ActorAnimationLoopExitFlags & actorMask) == 0) {
				_state.ActorAnimationCommandOffsets[actor] = argument;
				continue;
			}

			_state.ActorAnimationLoopExitFlags &= static_cast<uint8>(~actorMask);
			_state.ActorGraphicsReadyFlags &= static_cast<uint8>(~actorMask);
			_state.ActorAnimationOffsets[actor] = argument;
			continue;
		case 2:
			_state.ActorSelectedFrameOffsets[actor] = argument;
			if (actor == 0 || actor == _state.TransitionActorIndex) {
				_state.ActorTransitionFrameReadyFlags |= actorMask;
			}

			stageActorFrame(actor, actorMask, descriptorOffset, argument);
			return;
		case 9:
			_state.ActorAnimationHoldFlags |= actorMask;
			return;
		case 13:
			_state.ActorGraphicsReadyFlags &= static_cast<uint8>(~actorMask);
			_state.ActorAnimationOffsets[actor] = argument;
			continue;
		case 6:
			applyCommandPositionDelta(actor, argument);
			continue;
		case 0x11:
			if (argument >= 0) {
				playAudioCommand(argument);
			} else {
				stopAudioPlayback(static_cast<uint16>(argument) & 0x7FFF);
			}

			continue;
		default:
			continue;
		}
	}
}

void RoomActorAnimator::applyCommandPositionDelta(int actor, int16 packedDelta) {
	int8 xDelta = static_cast<int8>(static_cast<uint8>(packedDelta));
	int8 yDelta = static_cast<int8>(static_cast<uint16>(packedDelta) >> 8);
	_state.ActorXFixedCoordinates[actor] = _state.ActorXFixedCoordinates[actor] + (xDelta << 16);
	_state.ActorYFixedCoordinates[actor] = _state.ActorYFixedCoordinates[actor] + (yDelta << 16);
}

void RoomActorAnimator::stageActorFrame(int actor, uint8 actorMask, int descriptorOffset,
										int16 selectedFrameOffset) {
	int frameDataOffset = descriptorOffset + selectedFrameOffset;
	int compressedSourceOffset = descriptorOffset + static_cast<int>(_rom.readUInt32(frameDataOffset + 6));
	_state.ActorTileTransferWordCounts[actor] = getTileTransferWordCount(frameDataOffset);
	if ((_state.ActorCompactFrameFlags & actorMask) != 0) {
		if ((_state.ActorTileUploadPendingFlags & actorMask) != 0) {
			return;
		}

		_state.ActorCompactFrameSourceOffsets[actor] = compressedSourceOffset;
		_state.ActorCompactConversionPendingFlags |= actorMask;
	} else {
		_state.ActorTileData[actor] = decompressByteRle(
			_rom, compressedSourceOffset);
		_state.ActorTileUploadPendingFlags |= actorMask;
	}

	_state.ActorPendingFrameDataOffsets[actor] = frameDataOffset;
	_state.ActorAnimationDelays[actor] = _state.ActorStagedAnimationDelays[actor];
}

uint16 RoomActorAnimator::getTileTransferWordCount(int frameDataOffset) const {
	uint8 layout = _rom.readByte(frameDataOffset + 4);
	if (layout == 0x70) {
		return 0x0700;
	}

	if (layout == 0x40) {
		return 0x0400;
	}

	if (layout == 0x20 && _rom.readByte(frameDataOffset + 5) == 0x50) {
		return 0x0280;
	}

	if (layout == 0x20) {
		return 0x0200;
	}

	return 0x03C0;
}

void RoomActorAnimator::convertCompactFrames() {
	for (int actor = kActorCount - 1; actor >= 0; --actor) {
		uint8 actorMask = static_cast<uint8>(1 << actor);
		if ((_state.ActorCompactConversionPendingFlags & actorMask) == 0 || !canConvertCompactFrame(actor)) {
			continue;
		}

		int descriptorOffset = _state.ActorAnimationDescriptorOffsets[actor];
		int frameDataOffset = descriptorOffset + _state.ActorSelectedFrameOffsets[actor];
		Common::Array<uint8> decodedPixels = decompressByteRle(
			_rom, _state.ActorCompactFrameSourceOffsets[actor].value());
		int sourceOffset = actor == 0 && _rom.readByte(frameDataOffset + 4) == kExtendedSpriteLayout
							   ? 0x1000
							   : 0;
		uint16 horizontalScale = _state.ActorHorizontalScaleFactors[actor];
		uint16 verticalScale = _state.ActorVerticalScaleFactors[actor];
		uint16 frameX = _rom.readUInt16(frameDataOffset);
		uint16 anchorX =
			(_rom.readByte(frameDataOffset + 10) & 0x08) != 0
				? static_cast<uint16>(
					  _rom.readByte(frameDataOffset + 4) -
					  scaleUnsignedWord(static_cast<uint16>(_rom.readByte(frameDataOffset + 4) - frameX),
										horizontalScale))
				: scaleUnsignedWord(frameX, horizontalScale);
		uint16 anchorY = scaleUnsignedWord(_rom.readUInt16(frameDataOffset + 2), verticalScale);
		_state.ActorFrameAnchorX[actor] = static_cast<int16>(anchorX);
		_state.ActorFrameAnchorY[actor] = static_cast<int16>(anchorY);

		Common::Array<uint8> tileData;
		if ((_state.TransitionFlags & kTransitionMask) != 0 && static_cast<int16>(anchorY) < 2) {
			tileData = _rom.readByte(frameDataOffset + 4) == kCompactSprite48Layout
						   ? _spriteTransformer.expandCompactSprite48UsingPrimaryLayout(
								 decodedPixels, sourceOffset)
						   : _spriteTransformer.expandCompactSprite80UsingSecondaryLayout(decodedPixels,
																						  sourceOffset);
		} else {
			tileData = _rom.readByte(frameDataOffset + 4) == kCompactSprite48Layout
						   ? _spriteTransformer.scaleCompactSprite48(
								 decodedPixels, sourceOffset, horizontalScale,
								 verticalScale)
						   : _spriteTransformer.scaleCompactSprite80(
								 decodedPixels, sourceOffset, horizontalScale,
								 verticalScale);
		}

		_state.ActorTileData[actor] = tileData;
		_state.ActorCompactFrameSourceOffsets[actor].reset();
		if ((_state.RoomBehaviorFlags & kTransitionMask) == 0 ||
			(actor != 0 && actor != _state.TransitionActorIndex)) {
			_state.ActorTileUploadPendingFlags |= actorMask;
			_state.ActorCompactConversionPendingFlags &= static_cast<uint8>(~actorMask);
		}
	}
}

bool RoomActorAnimator::canConvertCompactFrame(int actor) const {
	if ((_state.RoomBehaviorFlags & kTransitionMask) == 0 ||
		(actor != 0 && actor != _state.TransitionActorIndex)) {
		return true;
	}

	uint8 transitionActorMask = static_cast<uint8>(1 << _state.TransitionActorIndex);
	return (_state.ActorTransitionFrameReadyFlags & 0x01) != 0 &&
		   (_state.ActorTransitionFrameReadyFlags & transitionActorMask) != 0;
}

void RoomActorAnimator::publishTransitionPair() {
	uint16 transitionActor = _state.TransitionActorIndex;
	uint8 transitionActorMask = static_cast<uint8>(1 << transitionActor);
	if ((_state.RoomBehaviorFlags & kTransitionMask) == 0 ||
		(_state.ActorTransitionFrameReadyFlags & 0x01) == 0 ||
		(_state.ActorTransitionFrameReadyFlags & transitionActorMask) == 0) {
		return;
	}

	_state.ActorTransitionFrameReadyFlags = 0;
	int frameDataOffset = _state.ActorAnimationDescriptorOffsets[0] + _state.ActorSelectedFrameOffsets[0];
	int16 actorX = static_cast<int16>(_state.ActorXFixedCoordinates[0] >> 16);
	int16 actorY = static_cast<int16>(_state.ActorYFixedCoordinates[0] >> 16);
	int16 pairedX = static_cast<int16>(
		actorX - scaleTransitionOffset(_rom.readInt16(frameDataOffset + 14),
									   _state.ActorHorizontalScaleFactors[0]));
	int16 pairedY = static_cast<int16>(
		actorY - scaleTransitionOffset(_rom.readInt16(frameDataOffset + 16),
									   _state.ActorVerticalScaleFactors[0]));
	_state.ActorXFixedCoordinates[transitionActor] =
		replaceIntegerCoordinate(_state.ActorXFixedCoordinates[transitionActor], pairedX);
	_state.ActorYFixedCoordinates[transitionActor] =
		replaceIntegerCoordinate(_state.ActorYFixedCoordinates[transitionActor], pairedY);

	_state.ActorCompactConversionPendingFlags &= static_cast<uint8>(~(0x01 | transitionActorMask));
	uint8 transitionPairMask = static_cast<uint8>(0x01 | transitionActorMask);
	_state.ActorTileUploadPendingFlags |= transitionPairMask;
	do {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	} while ((_state.ActorTileUploadPendingFlags & transitionPairMask) != 0);
}

uint16 RoomActorAnimator::scaleUnsignedWord(uint16 value, uint16 scale) {
	uint32 product = static_cast<uint32>(value) * static_cast<uint16>(0x0100 - scale);
	return static_cast<uint16>(static_cast<uint16>(product) >> 8);
}

int16 RoomActorAnimator::scaleTransitionOffset(int16 value, uint16 scale) {
	return static_cast<int16>((static_cast<int16>(0x00FF - scale) * value) >> 8);
}

int RoomActorAnimator::replaceIntegerCoordinate(int coordinate, int16 integer) {
	return (coordinate & 0x0000FFFF) | (integer << 16);
}

void RoomActorAnimator::playAudioCommand(int command) {
	(void)command;
	// AUDIO FRONTIER: Preserve command ordering while the PC audio domain remains deliberately deferred.
}

void RoomActorAnimator::stopAudioPlayback(int command) {
	(void)command;
	// AUDIO FRONTIER: Preserve command ordering while the PC audio domain remains deliberately deferred.
}
} // namespace Scooby
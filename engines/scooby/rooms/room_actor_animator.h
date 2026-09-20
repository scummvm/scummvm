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

#ifndef SCOOBY_ROOM_ACTOR_ANIMATOR_H
#define SCOOBY_ROOM_ACTOR_ANIMATOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "actor_sprite_transformer.h"
#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Advances the six recovered actor animation streams and stages their authored frame graphics.
//
// Ghidra UpdateActorAnimationFrames owns exactly six homogeneous slots. Four-byte command records preserve
// delay, loop, frame-selection, hold, animation-switch, signed packed position-delta, and audio operations.
// Compact source identity, decoded tile storage, pending publication, and transition-pair state remain
// distinct typed lifecycles instead of sharing the original aliased pointer workspace. Transition-pair
// publication waits use callback-aware clocked frames so pending tiles cannot be drained at host-CPU speed.

namespace Scooby {

class RoomActorAnimator {
public:
	// Binds actor animation interpretation to the verified ROM and shared room state.
	// rom: verified cartridge address space containing actor descriptors and frame streams.
	// state: shared actor state receiving command, graphics, and publication changes.
	// waitForRoomVerticalBlank: callback-aware room frame boundary publishing transition frames.
	RoomActorAnimator(const ScoobyDooRom &rom, RuntimeState &state,
					  Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _state(state), _spriteTransformer(rom), _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Advances every eligible actor command stream and converts newly selected compact frames.
	// Ghidra: updateActorAnimationFrames (0x0000B0B8).
	void updateActorAnimationFrames();

private:
	static const int kActorCount = 6;
	static const uint8 kCompactSprite48Layout = 0x30;
	static const uint8 kExtendedSpriteLayout = 0x70;
	static const uint8 kTransitionMask = 0x08;

	void coupleTransitionAnimation();
	bool canAdvanceActor(int actor, uint8 actorMask) const;
	void advanceActor(int actor, uint8 actorMask);
	void applyCommandPositionDelta(int actor, int16 packedDelta);
	void stageActorFrame(int actor, uint8 actorMask, int descriptorOffset,
						 int16 selectedFrameOffset);
	uint16 getTileTransferWordCount(int frameDataOffset) const;
	void convertCompactFrames();
	bool canConvertCompactFrame(int actor) const;
	void publishTransitionPair();

	static uint16 scaleUnsignedWord(uint16 value, uint16 scale);
	static int16 scaleTransitionOffset(int16 value, uint16 scale);
	static int replaceIntegerCoordinate(int coordinate, int16 integer);
	static void playAudioCommand(int command);
	static void stopAudioPlayback(int command);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	ActorSpriteTransformer _spriteTransformer;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_ACTOR_ANIMATOR_H

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
 */

#include "ripper/puzzles/state.h"

#include "common/debug.h"
#include "common/serializer.h"

#include "ripper/detection.h"
#include "ripper/display.h"

namespace Ripper {

ClockPuzzleState::ClockPuzzleState() : halfDay(false) {
	memset(dialIndices, 0, sizeof(dialIndices));
	memset(firstClockDigits, 0, sizeof(firstClockDigits));
	memset(secondClockDigits, 0, sizeof(secondClockDigits));
}

BoardArrangementPuzzleState::BoardArrangementPuzzleState() :
		positions{
			Common::Point(103, 63), Common::Point(168, 63),
			Common::Point(103, 120), Common::Point(168, 120),
			Common::Point(103, 294), Common::Point(168, 176),
			Common::Point(103, 239), Common::Point(103, 176)
		} {
}

KeyGroupPuzzleState::KeyGroupPuzzleState() {
	for (uint index = 0; index < ARRAYSIZE(sourceSlots); ++index)
		sourceSlots[index] = index;
	for (uint index = 0; index < ARRAYSIZE(targetSlots); ++index)
		targetSlots[index] = -1;
}

namespace {

bool syncClock(Common::Serializer &serializer, ClockPuzzleState &clock) {
	for (uint i = 0; i < ARRAYSIZE(clock.dialIndices); ++i) {
		serializer.syncAsSint32LE(clock.dialIndices[i]);
		if (clock.dialIndices[i] < 0 || clock.dialIndices[i] >= 12)
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(clock.firstClockDigits); ++i) {
		serializer.syncAsSint32LE(clock.firstClockDigits[i]);
		serializer.syncAsSint32LE(clock.secondClockDigits[i]);
		if (clock.firstClockDigits[i] < 0 || clock.firstClockDigits[i] > 9 ||
				clock.secondClockDigits[i] < 0 || clock.secondClockDigits[i] > 9)
			return false;
	}
	byte halfDay = clock.halfDay ? 1 : 0;
	serializer.syncAsByte(halfDay);
	clock.halfDay = halfDay != 0;
	return halfDay <= 1 &&
		clock.firstClockDigits[0] * 10 + clock.firstClockDigits[1] <= 23 &&
		clock.firstClockDigits[2] * 10 + clock.firstClockDigits[3] <= 59 &&
		clock.secondClockDigits[0] * 10 + clock.secondClockDigits[1] <= 12 &&
		clock.secondClockDigits[2] * 10 + clock.secondClockDigits[3] <= 59;
}

bool syncKeySlots(Common::Serializer &serializer, int *slots, uint count,
		int pieceCount, uint32 &seen) {
	for (uint i = 0; i < count; ++i) {
		serializer.syncAsSint32LE(slots[i]);
		if (slots[i] < -1 || slots[i] >= pieceCount)
			return false;
		if (slots[i] < 0)
			continue;
		const uint32 bit = 1U << slots[i];
		if (seen & bit)
			return false;
		seen |= bit;
	}
	return true;
}

} // End of anonymous namespace

bool PuzzleState::syncGame(Common::Serializer &serializer) {
	PuzzleState state = serializer.isLoading() ? PuzzleState() : *this;
	if (!syncClock(serializer, state.clock))
		return false;
	for (uint i = 0; i < ARRAYSIZE(state.boardArrangement.positions); ++i) {
		Common::Point &point = state.boardArrangement.positions[i];
		serializer.syncAsSint16LE(point.x);
		serializer.syncAsSint16LE(point.y);
		if (point.x < 0 || point.x >= kRipperScreenWidth ||
				point.y < 0 || point.y >= kRipperScreenHeight)
			return false;
	}
	uint32 seen = 0;
	const int pieceCount = ARRAYSIZE(state.keyGroup.sourceSlots);
	if (!syncKeySlots(serializer, state.keyGroup.sourceSlots,
			pieceCount, pieceCount, seen) ||
			!syncKeySlots(serializer, state.keyGroup.targetSlots,
				ARRAYSIZE(state.keyGroup.targetSlots), pieceCount, seen))
		return false;
	if (serializer.err())
		return false;
	if (serializer.isLoading())
		*this = state;
	debugC(2, kDebugSaveLoad, "Ripper: %s retained puzzle state",
		serializer.isLoading() ? "restored" : "saved");
	return true;
}

} // End of namespace Ripper

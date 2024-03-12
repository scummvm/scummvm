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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_FIGHTMOVES_H
#define CRAB_FIGHTMOVES_H

#include "crab/timer.h"
#include "crab/animation/fightmove.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
enum FrameUpdateResult {
	FUR_FAIL,
	FUR_WAIT,
	FUR_SUCCESS
};

// This state value indicates that the move should execute regardless of actual sprite state
const uint SPRITE_STATE_OVERRIDE = std::numeric_limits<uint>::max();

class FightMoves {
	// The fighting moves of a sprite
	Common::Array<FightMove> _move;

	// The currently selected move
	int _cur;

	// For AI - the move about to be executed
	int _next;

	// The timer used for playing animations
	Timer _timer;

	// We need to instantly show the new frame for a move that has started
	bool _start;

	// The current frame and total frames
	uint _frameCur, _frameTotal;

public:
	FightMoves();

	~FightMoves() {}

	void reset() {
		_cur = -1;
	}

	void load(rapidxml::xml_node<char> *node);

	bool curMove(FightMove &fm);
	bool nextMove(FightMove &fm);

	FrameUpdateResult updateFrame(const Direction &d);
	bool curFrame(FightAnimFrame &faf, const Direction &d);

	uint findMove(const pyrodactyl::input::FightAnimationType &type, const int &state);

	void listAttackMoves(Common::Array<uint> &list);

	bool forceUpdate(const uint &index, pyrodactyl::input::FightInput &input, const Direction &d);

	bool lastFrame() {
		return _frameCur >= _frameTotal;
	}

	void frameIndex(uint val) {
		_frameCur = val;
	}

	void curCombo(pyrodactyl::input::FightInput &input) {
		input = _move[_cur]._input;
	}

	bool validMove() {
		return _cur >= 0 && (uint)_cur < _move.size();
	}

	bool empty() {
		return _move.empty();
	}

	bool flip(TextureFlipType &flip, Direction d);

	const ShadowOffset &shadow(Direction d) {
		return _move[_cur]._frames[d]._shadow;
	}

	void next(int val) {
		_next = val;
	}

	int next() {
		return _next;
	}

	void evaluate(pyrodactyl::event::Info &info);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FIGHTMOVES_H

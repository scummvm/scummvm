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

#include "crab/music/MusicManager.h"
#include "crab/common_header.h"
#include "crab/animation/fightmove.h"
#include "crab/timer.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
enum FrameUpdateResult { FUR_FAIL,
						 FUR_WAIT,
						 FUR_SUCCESS };

// This state value indicates that the move should execute regardless of actual sprite state
const unsigned int SPRITE_STATE_OVERRIDE = std::numeric_limits<unsigned int>::max();

class FightMoves {
	// The fighting moves of a sprite
	std::vector<FightMove> move;

	// The currently selected move
	int cur;

	// For AI - the move about to be executed
	int next;

	// The timer used for playing animations
	Timer timer;

	// We need to instantly show the new frame for a move that has started
	bool start;

	// The current frame and total frames
	unsigned int frame_cur, frame_total;

public:
	FightMoves();
	~FightMoves() {}
	void Reset() { cur = -1; }

	void Load(rapidxml::xml_node<char> *node);

	bool CurMove(FightMove &fm);
	bool NextMove(FightMove &fm);

	FrameUpdateResult UpdateFrame(const Direction &d);
	bool CurFrame(FightAnimFrame &faf, const Direction &d);

	unsigned int FindMove(const pyrodactyl::input::FightAnimationType &type, const int &state);

	void ListAttackMoves(std::vector<unsigned int> &list);

	bool ForceUpdate(const unsigned int &index, pyrodactyl::input::FightInput &input, const Direction &d);

	bool LastFrame() { return frame_cur >= frame_total; }
	void FrameIndex(unsigned int val) { frame_cur = val; }

	void CurCombo(pyrodactyl::input::FightInput &input) { input = move[cur].input; }

	bool ValidMove() { return cur >= 0 && (unsigned int)cur < move.size(); }
	bool Empty() { return move.empty(); }

	bool Flip(TextureFlipType &flip, Direction d);
	const ShadowOffset &Shadow(Direction d) { return move[cur].frames[d].shadow; }

	void Next(int val) { next = val; }
	int Next() { return next; }

	void Evaluate(pyrodactyl::event::Info &info);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FIGHTMOVES_H

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

#include "crab/animation/fightmoves.h"

namespace Crab {

using namespace pyrodactyl::anim;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
FightMoves::FightMoves() {
	cur = -1;
	next = -1;
	start = false;

	frame_cur = 0;
	frame_total = 0;

	move.clear();
	timer.Start();
}

//------------------------------------------------------------------------
// Purpose: Load from file
//------------------------------------------------------------------------
void FightMoves::Load(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("move"); n != NULL; n = n->next_sibling("move")) {
		FightMove fm;
		fm.load(n);
		move.push_back(fm);
	}
}

//------------------------------------------------------------------------
// Purpose: Return current or next move
//------------------------------------------------------------------------
bool FightMoves::CurMove(FightMove &fm) {
	if (cur >= 0 && (unsigned int)cur < move.size()) {
		fm = move[cur];
		return true;
	}

	return false;
}

bool FightMoves::NextMove(FightMove &fm) {
	if (next >= 0 && (unsigned int)next < move.size()) {
		fm = move[next];
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Get the current frame of the sprite
//------------------------------------------------------------------------
bool FightMoves::CurFrame(FightAnimFrame &faf, const Direction &d) {
	// Check validity of current move
	if (cur >= 0 && (unsigned int)cur < move.size()) {
		// Check validity of current frame
		if (frame_cur < frame_total && frame_cur < move[cur]._frames[d]._frame.size()) {
			faf = move[cur]._frames[d]._frame[frame_cur];
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Update frame
//------------------------------------------------------------------------
FrameUpdateResult FightMoves::UpdateFrame(const Direction &d) {
	// Check validity of current move
	if (cur >= 0 && (unsigned int)cur < move.size()) {
		// Check validity of current frame
		if (frame_cur < frame_total && frame_cur < move[cur]._frames[d]._frame.size()) {
			// Has the current frame finished playing?
			// OR Is this the first frame of the move?
			if (timer.Ticks() >= move[cur]._frames[d]._frame[frame_cur]._repeat || start) {
				frame_cur++;
				timer.Start();
				start = false;

				return FUR_SUCCESS;
			} else
				return FUR_WAIT;
		}
	}

	return FUR_FAIL;
}

//------------------------------------------------------------------------
// Purpose: Find a move corresponding to the input and sprite state
//------------------------------------------------------------------------
unsigned int FightMoves::FindMove(const pyrodactyl::input::FightAnimationType &type, const int &state) {
	unsigned int pos = 0;
	for (auto i = move.begin(); i != move.end(); ++i, ++pos)
		if (i->_input.type == type && i->_input.state == (unsigned int)state)
			return pos;

	pos = 0;
	for (auto i = move.begin(); i != move.end(); ++i, ++pos)
		if (i->_input.type == type && i->_input.state == SPRITE_STATE_OVERRIDE)
			return pos;

	return SPRITE_STATE_OVERRIDE;
}

//------------------------------------------------------------------------
// Purpose: Function for AI
//------------------------------------------------------------------------
void FightMoves::ListAttackMoves(Common::Array<unsigned int> &list) {
	list.clear();
	unsigned int pos = 0;
	for (auto i = move.begin(); i != move.end(); ++i, ++pos)
		if (i->_ai.type == MOVE_ATTACK)
			list.push_back(pos);
}

//------------------------------------------------------------------------
// Purpose: Force update to a new move
//------------------------------------------------------------------------
bool FightMoves::ForceUpdate(const unsigned int &index, pyrodactyl::input::FightInput &input, const Direction &d) {
	frame_cur = 0;
	cur = index;

	if ((unsigned int)cur < move.size()) {
		if (move[cur]._unlock.Result()) {
			frame_total = move[cur]._frames[d]._frame.size();
			if (frame_total > 0) {
				input = move[cur]._input;
				input.state = move[cur]._frames[d]._frame[0]._state;
			} else
				input.Reset();

			timer.Start();
			start = true;
			g_engine->_musicManager->PlayEffect(move[cur]._eff._activate, 0);
			return true;
		}
	}

	cur = 0;
	frame_total = 0;
	return false;
}

//------------------------------------------------------------------------
// Purpose: Set unlock status
//------------------------------------------------------------------------
void FightMoves::Evaluate(pyrodactyl::event::Info &info) {
	for (auto i = move.begin(); i != move.end(); ++i)
		i->_unlock.Evaluate(info);
}

//------------------------------------------------------------------------
// Purpose: Find which style to flip the texture in
//------------------------------------------------------------------------
bool FightMoves::Flip(TextureFlipType &flip, Direction d) {
	// Check validity of current move
	if (ValidMove()) {
		flip = move[cur]._frames[d]._flip;
		return true;
	}

	return false;
}

} // End of namespace Crab

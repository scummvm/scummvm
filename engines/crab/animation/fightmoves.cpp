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
	_cur = -1;
	_next = -1;
	_start = false;

	_frameCur = 0;
	_frameTotal = 0;

	_move.clear();
	_timer.Start();
}

//------------------------------------------------------------------------
// Purpose: Load from file
//------------------------------------------------------------------------
void FightMoves::load(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("move"); n != NULL; n = n->next_sibling("move")) {
		FightMove fm;
		fm.load(n);
		_move.push_back(fm);
	}
}

//------------------------------------------------------------------------
// Purpose: Return current or next move
//------------------------------------------------------------------------
bool FightMoves::curMove(FightMove &fm) {
	if (_cur >= 0 && (unsigned int)_cur < _move.size()) {
		fm = _move[_cur];
		return true;
	}

	return false;
}

bool FightMoves::nextMove(FightMove &fm) {
	if (_next >= 0 && (unsigned int)_next < _move.size()) {
		fm = _move[_next];
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Get the current frame of the sprite
//------------------------------------------------------------------------
bool FightMoves::curFrame(FightAnimFrame &faf, const Direction &d) {
	// Check validity of current move
	if (_cur >= 0 && (unsigned int)_cur < _move.size()) {
		// Check validity of current frame
		if (_frameCur < _frameTotal && _frameCur < _move[_cur]._frames[d]._frame.size()) {
			faf = _move[_cur]._frames[d]._frame[_frameCur];
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Update frame
//------------------------------------------------------------------------
FrameUpdateResult FightMoves::updateFrame(const Direction &d) {
	// Check validity of current move
	if (_cur >= 0 && (unsigned int)_cur < _move.size()) {
		// Check validity of current frame
		if (_frameCur < _frameTotal && _frameCur < _move[_cur]._frames[d]._frame.size()) {
			// Has the current frame finished playing?
			// OR Is this the first frame of the move?
			if (_timer.Ticks() >= _move[_cur]._frames[d]._frame[_frameCur]._repeat || _start) {
				_frameCur++;
				_timer.Start();
				_start = false;

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
unsigned int FightMoves::findMove(const pyrodactyl::input::FightAnimationType &type, const int &state) {
	unsigned int pos = 0;
	for (auto i = _move.begin(); i != _move.end(); ++i, ++pos)
		if (i->_input._type == type && i->_input._state == (unsigned int)state)
			return pos;

	pos = 0;
	for (auto i = _move.begin(); i != _move.end(); ++i, ++pos)
		if (i->_input._type == type && i->_input._state == SPRITE_STATE_OVERRIDE)
			return pos;

	return SPRITE_STATE_OVERRIDE;
}

//------------------------------------------------------------------------
// Purpose: Function for AI
//------------------------------------------------------------------------
void FightMoves::listAttackMoves(Common::Array<unsigned int> &list) {
	list.clear();
	unsigned int pos = 0;
	for (auto i = _move.begin(); i != _move.end(); ++i, ++pos)
		if (i->_ai._type == MOVE_ATTACK)
			list.push_back(pos);
}

//------------------------------------------------------------------------
// Purpose: Force update to a new move
//------------------------------------------------------------------------
bool FightMoves::forceUpdate(const unsigned int &index, pyrodactyl::input::FightInput &input, const Direction &d) {
	_frameCur = 0;
	_cur = index;

	if ((unsigned int)_cur < _move.size()) {
		if (_move[_cur]._unlock.result()) {
			_frameTotal = _move[_cur]._frames[d]._frame.size();
			if (_frameTotal > 0) {
				input = _move[_cur]._input;
				input._state = _move[_cur]._frames[d]._frame[0]._state;
			} else
				input.reset();

			_timer.Start();
			_start = true;
			g_engine->_musicManager->PlayEffect(_move[_cur]._eff._activate, 0);
			return true;
		}
	}

	_cur = 0;
	_frameTotal = 0;
	return false;
}

//------------------------------------------------------------------------
// Purpose: Set unlock status
//------------------------------------------------------------------------
void FightMoves::evaluate(pyrodactyl::event::Info &info) {
	for (auto i = _move.begin(); i != _move.end(); ++i)
		i->_unlock.evaluate(info);
}

//------------------------------------------------------------------------
// Purpose: Find which style to flip the texture in
//------------------------------------------------------------------------
bool FightMoves::flip(TextureFlipType &flip, Direction d) {
	// Check validity of current move
	if (validMove()) {
		flip = _move[_cur]._frames[d]._flip;
		return true;
	}

	return false;
}

} // End of namespace Crab

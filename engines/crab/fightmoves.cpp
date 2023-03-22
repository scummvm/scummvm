#include "fightmoves.h"
#include "stdafx.h"

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
		fm.Load(n);
		move.push_back(fm);
	}
}

//------------------------------------------------------------------------
// Purpose: Return current or next move
//------------------------------------------------------------------------
bool FightMoves::CurMove(FightMove &fm) {
	if (cur >= 0 && cur < move.size()) {
		fm = move.at(cur);
		return true;
	}

	return false;
}

bool FightMoves::NextMove(FightMove &fm) {
	if (next >= 0 && next < move.size()) {
		fm = move.at(next);
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Get the current frame of the sprite
//------------------------------------------------------------------------
bool FightMoves::CurFrame(FightAnimFrame &faf, const Direction &d) {
	// Check validity of current move
	if (cur >= 0 && cur < move.size()) {
		// Check validity of current frame
		if (frame_cur < frame_total && frame_cur < move.at(cur).frames[d].frame.size()) {
			faf = move.at(cur).frames[d].frame.at(frame_cur);
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
	if (cur >= 0 && cur < move.size()) {
		// Check validity of current frame
		if (frame_cur < frame_total && frame_cur < move.at(cur).frames[d].frame.size()) {
			// Has the current frame finished playing?
			// OR Is this the first frame of the move?
			if (timer.Ticks() >= move.at(cur).frames[d].frame.at(frame_cur).repeat || start) {
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
		if (i->input.type == type && i->input.state == state)
			return pos;

	pos = 0;
	for (auto i = move.begin(); i != move.end(); ++i, ++pos)
		if (i->input.type == type && i->input.state == SPRITE_STATE_OVERRIDE)
			return pos;

	return SPRITE_STATE_OVERRIDE;
}

//------------------------------------------------------------------------
// Purpose: Function for AI
//------------------------------------------------------------------------
void FightMoves::ListAttackMoves(std::vector<unsigned int> &list) {
	list.clear();
	unsigned int pos = 0;
	for (auto i = move.begin(); i != move.end(); ++i, ++pos)
		if (i->ai.type == MOVE_ATTACK)
			list.push_back(pos);
}

//------------------------------------------------------------------------
// Purpose: Force update to a new move
//------------------------------------------------------------------------
bool FightMoves::ForceUpdate(const unsigned int &index, pyrodactyl::input::FightInput &input, const Direction &d) {
	frame_cur = 0;
	cur = index;

	if (cur < move.size()) {
		if (move[cur].unlock.Result()) {
			frame_total = move[cur].frames[d].frame.size();
			if (frame_total > 0) {
				input = move[cur].input;
				input.state = move[cur].frames[d].frame.at(0).state;
			} else
				input.Reset();

			timer.Start();
			start = true;
			pyrodactyl::music::gMusicManager.PlayEffect(move[cur].eff.activate, 0);
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
		i->unlock.Evaluate(info);
}

//------------------------------------------------------------------------
// Purpose: Find which style to flip the texture in
//------------------------------------------------------------------------
bool FightMoves::Flip(TextureFlipType &flip, Direction d) {
	// Check validity of current move
	if (ValidMove()) {
		flip = move.at(cur).frames[d].flip;
		return true;
	}

	return false;
}
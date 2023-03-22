#include "stdafx.h"

#include "walkframes.h"

using namespace pyrodactyl::anim;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Loader Function
//------------------------------------------------------------------------
void WalkFrames::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("stand", node))
		set[WT_STAND].Load(node->first_node("stand"));

	if (NodeValid("walk", node))
		set[WT_WALK].Load(node->first_node("walk"));

	if (NodeValid("fight", node))
		set[WT_FIGHT].Load(node->first_node("fight"));

	if (NodeValid("ko", node))
		set[WT_KO].Load(node->first_node("ko"));
}

//------------------------------------------------------------------------
// Purpose: Used for walking inside levels
//------------------------------------------------------------------------
bool WalkFrames::UpdateClip(Direction d, bool reset) {
	if (timer.Ticks() > set[cur].frames[d].CurrentFrame().repeat || reset) {
		timer.Start();
		return set[cur].frames[d].UpdateClip();
	}

	return false;
}

void WalkFrames::ResetClip(Direction d) {
	set[cur].frames[d].Reset();
	timer.Start();
}

//------------------------------------------------------------------------
// Purpose: Used inside dialog box
//------------------------------------------------------------------------
void WalkFrames::UpdateClip(WalkAnimType type, Direction d) {
	if (!timer.Started())
		timer.Start();

	if (timer.Ticks() > set[type].frames[d].CurrentFrame().repeat) {
		set[type].frames[d].UpdateClip();
		timer.Start();
	}
}

Rect WalkFrames::DialogClip(const PersonState &state) {
	if (state == PST_FIGHT)
		return set[WT_FIGHT].frames[DIRECTION_DOWN].CurrentFrame().clip;
	else if (state == PST_KO)
		return set[WT_KO].frames[DIRECTION_DOWN].CurrentFrame().clip;

	return set[WT_STAND].frames[DIRECTION_DOWN].CurrentFrame().clip;
}

void WalkFrames::UpdateClip(const PersonState &state) {
	if (state == PST_FIGHT)
		UpdateClip(WT_FIGHT, DIRECTION_DOWN);
	else if (state == PST_KO)
		UpdateClip(WT_KO, DIRECTION_DOWN);
	else
		UpdateClip(WT_STAND, DIRECTION_DOWN);
}

//------------------------------------------------------------------------
// Purpose: Decide direction and type of animation, return whether it has changed
//------------------------------------------------------------------------
bool WalkFrames::Type(const Vector2f &vel, Direction &dir, const pyrodactyl::people::PersonState &pst, const bool &first_x) {
	Direction prev_dir = dir;
	WalkAnimType prev_type = cur;

	if (pst == PST_KO) {
		cur = WT_KO;
	} else if (first_x) {
		// If we prioritize the X direction, X velocity is checked first for direction and then Y velocity
		if (vel.x > 0) {
			dir = DIRECTION_RIGHT;
			cur = WT_WALK;
		} else if (vel.x < 0) {
			dir = DIRECTION_LEFT;
			cur = WT_WALK;
		} else if (vel.y > 0) {
			dir = DIRECTION_DOWN;
			cur = WT_WALK;
		} else if (vel.y < 0) {
			dir = DIRECTION_UP;
			cur = WT_WALK;
		} else {
			cur = WT_STAND;
		}
	} else {
		// If we prioritize the Y direction, Y velocity is checked first for direction and then Y velocity
		if (vel.y > 0) {
			dir = DIRECTION_DOWN;
			cur = WT_WALK;
		} else if (vel.y < 0) {
			dir = DIRECTION_UP;
			cur = WT_WALK;
		} else if (vel.x > 0) {
			dir = DIRECTION_RIGHT;
			cur = WT_WALK;
		} else if (vel.x < 0) {
			dir = DIRECTION_LEFT;
			cur = WT_WALK;
		} else {
			cur = WT_STAND;
		}
	}

	if (prev_dir != dir || prev_type != cur)
		return true;

	return false;
}
#include "fightanim.h"
#include "stdafx.h"

using namespace pyrodactyl::anim;

//------------------------------------------------------------------------
// Purpose: Load a single frame of a fighting move
//------------------------------------------------------------------------
void FightAnimFrame::Load(rapidxml::xml_node<char> *node, const Rect &VBOX, const Uint32 &rep, const int &AX, const int &AY) {
	AnimFrame::Load(node, VBOX, rep, AX, AY);

	if (NodeValid("box_d", node, false))
		box_d.Load(node->first_node("box_d"));

	if (NodeValid("shift", node, false))
		delta.Load(node->first_node("shift"));

	if (!LoadNum(state, "state", node, false))
		state = 0;

	LoadBool(branch, "branch", node, false);
}

//------------------------------------------------------------------------
// Purpose: Load a fighting move
//------------------------------------------------------------------------
void FightAnimFrames::Load(rapidxml::xml_node<char> *node) {
	LoadTextureFlipType(flip, node);

	if (!LoadNum(repeat, "repeat", node, false))
		repeat = 0;

	if (NodeValid("anchor", node, false))
		anchor.Load(node->first_node("anchor"));

	if (NodeValid("box_v", node))
		box_v.Load(node->first_node("box_v"));

	if (NodeValid("shadow", node)) {
		shadow.Load(node->first_node("shadow"));
		shadow.valid = true;
	}

	if (NodeValid("frames", node)) {
		frame.clear();
		rapidxml::xml_node<char> *framenode = node->first_node("frames");
		for (auto n = framenode->first_node("frame"); n != NULL; n = n->next_sibling("frame")) {
			FightAnimFrame faf;
			faf.Load(n, box_v, repeat, anchor.x, anchor.y);
			frame.push_back(faf);
		}
	}
}
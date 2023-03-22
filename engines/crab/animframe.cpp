#include "animframe.h"
#include "stdafx.h"

using namespace pyrodactyl::anim;

void AnimFrame::Load(rapidxml::xml_node<char> *node, const Rect &VBOX, const Uint32 &rep, const int &AX, const int &AY) {
	clip.Load(node);

	if (rep == 0)
		LoadNum(repeat, "repeat", node);
	else
		repeat = rep;

	if (AX == 0.0f && AY == 0.0f) {
		if (NodeValid("anchor", node, false))
			anchor.Load(node->first_node("anchor"));
	} else {
		anchor.x = AX;
		anchor.y = AY;
	}

	if (VBOX.w == 0 || VBOX.h == 0) {
		if (NodeValid("box_v", node))
			box_v.Load(node->first_node("box_v"));
	} else
		box_v = VBOX;
}

void AnimationFrames::Load(rapidxml::xml_node<char> *node) {
	LoadTextureFlipType(flip, node);

	if (!LoadNum(repeat, "repeat", node, false))
		repeat = 0;

	LoadBool(random, "random", node, false);

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
			AnimFrame af;
			af.Load(n, box_v, repeat, anchor.x, anchor.y);
			frame.push_back(af);
		}
	}

	if (random)
		current_clip = gRandom.Num() % frame.size();
	else
		current_clip = 0;
}

bool AnimationFrames::UpdateClip() {
	if (current_clip < frame.size()) {
		current_clip = (current_clip + 1) % frame.size();
		return true;
	} else
		current_clip = 0;

	return false;
}

const AnimFrame &AnimationFrames::CurrentFrame() {
	return frame.at(current_clip);
}
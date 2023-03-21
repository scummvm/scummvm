#include "stdafx.h"
#include "fightmove.h"

using namespace pyrodactyl::anim;

void FightMove::Load(rapidxml::xml_node<char> *node)
{
	frames[DIRECTION_DOWN].Load(node->first_node("down"));
	frames[DIRECTION_UP].Load(node->first_node("up"));
	frames[DIRECTION_LEFT].Load(node->first_node("left"));
	frames[DIRECTION_RIGHT].Load(node->first_node("right"));

	if (NodeValid("input", node))
		input.Load(node->first_node("input"));

	if (NodeValid("unlock", node, false))
		unlock.Load(node->first_node("unlock"));

	if (NodeValid("effect", node))
		eff.Load(node->first_node("effect"));

	if (NodeValid("ai", node, false))
		ai.Load(node->first_node("ai"));
}
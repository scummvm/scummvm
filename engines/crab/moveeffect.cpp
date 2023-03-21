#include "stdafx.h"
#include "moveeffect.h"

using namespace pyrodactyl::anim;

FightMoveEffect::FightMoveEffect()
{
	activate = -1;
	hit = -1;
	dmg = 0;
	stun = 0;
	hurt = -1;
	death = -1;
}

void FightMoveEffect::Load(rapidxml::xml_node<char> * node)
{
	LoadNum(stun, "stun", node);
	LoadNum(dmg, "damage", node);
	LoadNum(hurt, "hurt", node);
	LoadNum(death, "death", node);

	if (NodeValid("image", node, false))
		img.Load(node->first_node("image"));

	if (NodeValid("sound", node))
	{
		rapidxml::xml_node<char> * soundnode = node->first_node("sound");

		if (!LoadNum(activate, "activate", soundnode, false))
			activate = -1;

		if (!LoadNum(activate, "hit", soundnode, false))
			activate = -1;
	}
}
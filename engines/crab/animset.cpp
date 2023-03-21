#include "stdafx.h"
#include "animset.h"

using namespace pyrodactyl::anim;

void AnimSet::Load(const std::string &filename)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node();
		if (NodeValid(node))
		{
			fight.Load(node);
			walk.Load(node);

			if (NodeValid("bounds", node))
				bounds.Load(node->first_node("bounds"));

			if (NodeValid("shadow", node))
				shadow.Load(node->first_node("shadow"));

			if (NodeValid("focus", node))
				focus.Load(node->first_node("focus"));
		}
	}
}

TextureFlipType AnimSet::Flip(const Direction &dir)
{
	TextureFlipType ret;
	if (fight.Flip(ret, dir))
		return ret;

	return walk.Flip(dir);
}

const ShadowOffset& AnimSet::Shadow(const Direction &dir)
{
	if (fight.ValidMove())
		return fight.Shadow(dir);

	return walk.Shadow(dir);
}

const int AnimSet::AnchorX(const Direction &dir)
{
	FightAnimFrame faf;
	if (fight.CurFrame(faf, dir))
		return faf.anchor.x;

	return walk.AnchorX(dir);
}

const int AnimSet::AnchorY(const Direction &dir)
{
	FightAnimFrame faf;
	if (fight.CurFrame(faf, dir))
		return faf.anchor.y;

	return walk.AnchorY(dir);
}
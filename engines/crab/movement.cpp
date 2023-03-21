#include "stdafx.h"
#include "movement.h"

using namespace pyrodactyl::ai;

//------------------------------------------------------------------------
// Purpose: Walk in preset paths
//------------------------------------------------------------------------
void MovementSet::Load(rapidxml::xml_node<char> *node)
{
	enabled = true;
	LoadBool(repeat, "repeat", node);
	for (auto n = node->first_node("walk"); n != NULL; n = n->next_sibling("walk"))
		path.push_back(n);
}

//------------------------------------------------------------------------
// Purpose: To make the AI patrol/wait along certain points
//------------------------------------------------------------------------
bool MovementSet::InternalEvents(const Rect rect)
{
	if (enabled)
	{
		//If we are at the current waypoint, get to the next waypoint
		if (path.at(cur).target.Collide(rect))
		{
			cur = (cur + 1) % path.size();
			timer.Start();
		}

		//Wait according to the delay value in the node
		if (timer.Ticks() >= path.at(cur).delay)
			return true;
	}

	return false;
}
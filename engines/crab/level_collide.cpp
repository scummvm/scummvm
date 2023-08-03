#include "stdafx.h"
#include "level.h"

using namespace TMX;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;
using namespace pyrodactyl::input;
using namespace pyrodactyl::event;
using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: See if player clicked on a sprite they are colliding with
//------------------------------------------------------------------------
bool Level::ContainsClick(const std::string &id, const SDL_Event &Event)
{
	//If mouse is moved and is hovering on the specified sprite (id), set hover = true
	if (Event.type == SDL_MOUSEMOTION)
	{
		for (auto &i : objects)
		{
			Rect r = i.PosRect();

			if (r.Contains(gMouse.motion.x + camera.x, gMouse.motion.y + camera.y))
			{
				//This is to show the sprite's name on top of their head
				i.hover = true;

				//This is to update the mouse cursor only if we're in talking range of a sprite
				if (i.ID() == id)
					gMouse.hover = true;
			}
			else
				i.hover = false;
		}

		return false;
	}
	else if (gMouse.Pressed())
	{
		for (auto &i : objects)
		{
			if (i.ID() == id)
			{
				Rect r = i.PosRect();
				if (r.Contains(gMouse.button.x + camera.x, gMouse.button.y + camera.y))
				{
					gMouse.hover = true;
					return true;
				}
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Find if a layer is visible (used only for objects with layers associated with them)
//------------------------------------------------------------------------
bool Level::LayerVisible(Sprite *obj)
{
	if (obj->layer < 0)
		return true;

	if (obj->layer < terrain.layer.size())
		return terrain.layer.at(obj->layer).collide;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with the trigger areas
// std::string &id is set to the id of colliding object
//------------------------------------------------------------------------
void Level::CalcTrigCollide(Info &info)
{
	for (auto i = objects.begin(); i != objects.end(); ++i)
		if (info.PersonValid(i->ID()))
			terrain.CollideWithTrigger(i->BoundRect(), info.PersonGet(i->ID()).trig);
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with interactive objects
// std::string &id is set to the id of colliding object
//------------------------------------------------------------------------
bool Level::CollidingWithObject(Info &info, std::string &id)
{
	//Clip and Bounding rectangle of player
	Rect p_pos = objects[player_index].PosRect(), p_bound = objects[player_index].BoundRect();

	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index)
	{
		if (i->Visible() && player_index != index && info.State(i->ID()) == PST_NORMAL)
		{
			//Clip and bounding rectangles for the NPC sprite
			Rect i_pos = i->PosRect(), i_bound = i->BoundRect();

			if (p_pos.Collide(i_pos) || i_pos.Contains(p_pos) || p_bound.Collide(i_bound) || i_bound.Contains(p_bound))
			{
				id = i->ID();
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with non-interactive stuff in the level
// id is set to the id of colliding object
//------------------------------------------------------------------------
bool Level::CollidingWithLevel(Info &info, Sprite &s)
{
	terrain.CollideWithNoWalk(s.BoundRect(), s.collide_data);

	terrain.CollideWithStairs(s.BoundRect(), s.vel_mod);

	if (terrain.CollideWithMusic(s.BoundRect(), music))
	{
		if (music.track)
			gMusicManager.PlayMusic(music.id);
		else
			gMusicManager.PlayEffect(music.id, music.loops);
	}

	//If we are colliding with something, return true
	return !s.collide_data.empty();
}
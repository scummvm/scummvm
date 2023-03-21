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

//------------------------------------------------------------------------
// Purpose: We re-use the same level object each time
// this function cleans up everything to make it good as new (get it)
//------------------------------------------------------------------------
void Level::Reset()
{
	if (player_index > 0 && objects.size() > player_index)
		objects[player_index].pathing.shutdown();

	player_index = 0;
	terrain.Reset();

	objects.clear();
	obj_seq.clear();
	background.clear();
	fly.clear();

	Camera(0, 0, gScreenSettings.cur.w, gScreenSettings.cur.h);

	showmap.Set(true);
	inside_exit = false;
	first_hit = true;
	music.id = -1;
	preview_path.clear();
}

//------------------------------------------------------------------------
// Purpose: Get index of a sprite in the object array
//------------------------------------------------------------------------
pyrodactyl::anim::Sprite *Level::GetSprite(const std::string &id)
{
	int count = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++count)
		if (i->ID() == id)
			return &objects[count];

	return NULL;
}

//------------------------------------------------------------------------
// Purpose: Figure visibility and let the AI see what moves are allowed
//------------------------------------------------------------------------
void Level::CalcProperties(Info &info)
{
	for (auto i = objects.begin(); i != objects.end(); ++i)
		i->CalcProperties(info);
}

//------------------------------------------------------------------------
// Purpose: Handle the level movement
//------------------------------------------------------------------------
void Level::HandleEvents(Info &info, const SDL_Event &Event)
{
	if (!gMouse.inside_hud)
	{
		if (Event.type == SDL_MOUSEBUTTONDOWN)
		{
			//Only go to path if our mouse is a normal walking cursor
			if (!gMouse.hover)
			{
				//The destination coordinate is set by adding camera position to click position
				Vector2i dest = gMouse.button;
				dest.x += camera.x;
				dest.y += camera.y;

				Rect b = objects[player_index].BoundRect();
				b.w /= 2;
				b.h /= 2;
				b.x = dest.x - b.w / 2;
				b.y = dest.y - b.h / 2;

				objects[player_index].SetDestPathfinding(dest, !terrain.InsideNoWalk(dest));
			}
		}
	}

	objects[player_index].HandleEvents(info, camera, sc_default, Event);
}

//------------------------------------------------------------------------
// Purpose: Swap the player sprites
//------------------------------------------------------------------------
void Level::PlayerID(const std::string &ID, const int &X, const int &Y)
{
	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index)
	{
		if (i->ID() == ID)
		{
			player_index = index;

			if (X != -1) i->X(X);
			if (Y != -1) i->Y(Y);

			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Rearrange stuff after resolution change
//------------------------------------------------------------------------
void Level::SetUI()
{
	camera.w = gScreenSettings.cur.w;
	camera.h = gScreenSettings.cur.h;
	SetCamera();
}
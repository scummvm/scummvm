#include "stdafx.h"
#include "MapMarkerMenu.h"

using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void MapMarkerMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("player", node))
		player.Load(node->first_node("player"));

	if (NodeValid("offset", node))
	{
		rapidxml::xml_node<char> *offnode = node->first_node("offset");

		if (NodeValid("marker", offnode))
			offset.marker.Load(offnode->first_node("marker"));

		if (NodeValid("player", offnode))
			offset.player.Load(offnode->first_node("player"));
	}

	menu.UseKeyboard(true);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void MapMarkerMenu::Draw(const Element &pos, const Vector2i &player_pos, const Rect &camera)
{
	//Calculate all offsets
	Vector2i offset_p(pos.x + player_pos.x + offset.player.x - camera.x, pos.y + player_pos.y + offset.player.y - camera.y);
	Vector2i offset_m(pos.x - camera.x + offset.marker.x, pos.y - camera.y + offset.marker.y);

	//Only draw the image - captions drawn later to prevent drawing another button over caption
	player.ImageCaptionOnlyDraw(offset_p.x, offset_p.y);

	for (auto &i : menu.element)
		i.ImageCaptionOnlyDraw(offset_m.x, offset_m.y);

	//Now draw the tool-tips for everything combined
	player.HoverInfoOnlyDraw(offset_p.x, offset_p.y);

	for (auto &i : menu.element)
		i.HoverInfoOnlyDraw(offset_m.x, offset_m.y);
}

//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
void MapMarkerMenu::HandleEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, const SDL_Event &Event)
{
	if (player_pos.x >= camera.x && player_pos.y >= camera.y)
		player.HandleEvents(Event, pos.x + player_pos.x - camera.x + offset.player.x, pos.y + player_pos.y - camera.y + offset.player.y);

	int choice = menu.HandleEvents(Event, pos.x - camera.x + offset.marker.x, pos.y - camera.y + offset.marker.y);
	if (choice != -1)
	{
		int c = 0;
		for (auto &i : menu.element)
		{
			if (c == choice) //For an already selected marker, clicking it toggles the selection state
				i.State(!i.State());
			else
				i.State(false);

			++c;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void MapMarkerMenu::InternalEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, Rect bounds)
{
	//Find if the player marker is visible or not
	{
		Rect r(pos.x + player_pos.x - offset.marker.x - camera.x,
			pos.y + player_pos.y - offset.marker.y - camera.y,
			player.w + offset.marker.x,
			player.h + offset.marker.y);

		player.visible = bounds.Contains(r);
	}

	//Redefine p for marker buttons
	Vector2i p(pos.x - camera.x + offset.marker.x, pos.y - camera.y + offset.marker.y);

	//Calculate visibility for each marker
	for (auto &i : menu.element)
	{
		Rect r(i.x + p.x - offset.marker.x, i.y + p.y - offset.marker.y,
			i.w + offset.marker.x, i.h + offset.marker.y);

		i.visible = bounds.Contains(r);
	}
}

//------------------------------------------------------------------------
// Purpose: Reposition UI
//------------------------------------------------------------------------
void MapMarkerMenu::SetUI()
{
	player.SetUI();
	menu.SetUI();
}
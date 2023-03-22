#pragma once

#include "GameEventInfo.h"
#include "ImageData.h"
#include "ImageManager.h"
#include "MapData.h"
#include "MapMarkerMenu.h"
#include "StateButton.h"
#include "ToggleButton.h"
#include "common_header.h"
#include "mapbutton.h"
#include "triggerset.h"

namespace pyrodactyl {
namespace ui {
class Map {
	// We have multiple world maps, each with their own data
	std::vector<MapData> map;

	// Index of the currently visible map
	unsigned int cur;

	// The currently loaded map background image
	pyrodactyl::image::Image img_bg, img_overlay;

	// The position at which map image has to be drawn
	Element pos;

	// Foreground image of the map
	ImageData fg;

	// size = Dimensions of the map image
	// mouse = The current coordinates of the mouse
	// vel = The speed at which the map is moving
	Vector2i size, mouse, vel;

	// The reference speed of the camera movement
	int speed;

	// The pan toggle is used when the mouse is down and moving simultaneously
	// overlay = true if we are showing a more detailed world map
	bool pan, overlay;

	// The camera size and position for the map
	// Bounds is the area we draw the map elements for
	Rect camera, bounds;

	// The button to toggle between showing the overlay or not
	ToggleButton bu_overlay;

	// All data for drawing map markers
	MapMarkerMenu marker;

	// The map name is drawn here
	HoverInfo title;

	// The buttons for scrolling the map (only visible if there is area to scroll)
	ButtonMenu scroll;

	// The menu for fast travel locations
	MapButtonMenu travel;

	void CalcBounds() {
		bounds.x = pos.x;
		bounds.y = pos.y;
		bounds.w = camera.w;
		bounds.h = camera.h;
	}

public:
	// The currently selected location
	std::string cur_loc;

	// The coordinates of the player's current location
	Vector2i player_pos;

	Map() {
		speed = 1;
		pan = false;
		cur = 0;
		overlay = true;
	}
	~Map() {}

	void Load(const std::string &filename, pyrodactyl::event::Info &info);

	void Draw(pyrodactyl::event::Info &info);
	bool HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
	void InternalEvents(pyrodactyl::event::Info &info);

	void Center(const Vector2i &pos);
	void Move(const SDL_Event &Event);
	void Validate();

	void RevealAdd(const int &id, const Rect &area);
	void DestAdd(const std::string &name, const int &x, const int &y);
	void DestDel(const std::string &name);
	void SelectDest(const std::string &name);

	void Update(pyrodactyl::event::Info &info);
	void SetImage(const unsigned int &val, const bool &force = false);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl
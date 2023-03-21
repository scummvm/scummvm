//=============================================================================
// Author:   Arvind
// Purpose:  Cursor object
//=============================================================================
#pragma once

#include "common_header.h"
#include "vectors.h"
#include "loaders.h"
#include "Image.h"

namespace pyrodactyl
{
	namespace input
	{
		class Cursor
		{
			//Mouse images
			pyrodactyl::image::Image img, img_s, img_hover, img_hover_s;

			//The hover mouse cursor is drawn at a slight offset to the normal cursor
			Vector2i hover_offset;

			//Mouse image changes slightly if left click button is pressed
			bool pressed;

		public:
			//Various coordinates
			Vector2i motion, button, rel;

			//Is the mouse inside the HUD? Used to disable level mouse movement if true
			bool inside_hud;

			//Was the last click on an NPC?
			bool hover;

			Cursor(){ pressed = false; inside_hud = false; hover = false; }
			~Cursor(){}

			void Quit() { img.Delete(); img_s.Delete(); }
			void Reset();

			void Load(rapidxml::xml_node<char> *node);
			void HandleEvents(const SDL_Event &Event);

			void Draw();
			const bool Pressed(){ return pressed; }
		};

		extern Cursor gMouse;
	}
}
#pragma once

#include "common_header.h"
#include "slider.h"
#include "ImageData.h"
#include "StateButton.h"
#include "ToggleButton.h"
#include "ResolutionMenu.h"

namespace pyrodactyl
{
	namespace ui
	{
		class GfxSettingMenu
		{
			//The brightness slider
			Slider brightness;

			//The button to toggle between full and windowed, and turn vsync on/off, window borders or not, game quality
			ToggleButton fullscreen, vsync, border, quality;

			//The buttons and menus for changing resolution
			ResolutionMenu resolution;

			//Notice that quality setting can only be changed outside the game
			HoverInfo notice_quality;

		public:
			GfxSettingMenu(){}
			~GfxSettingMenu(){}

			void Load(rapidxml::xml_node<char> *node);
			int HandleEvents(const SDL_Event &Event);
			void InternalEvents();

			void Draw();
			void SetUI();

			void SetInfo() { resolution.SetInfo(); }
		};
	}
}
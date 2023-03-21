//=============================================================================
// Author:   Arvind
// Purpose:  The tray where you have inventory, map and journal icons
//=============================================================================
#pragma once

#include "common_header.h"
#include "input.h"
#include "SlideShow.h"
#include "FileMenu.h"
#include "GameOverMenu.h"
#include "PauseMenu.h"
#include "HealthIndicator.h"
#include "GameEventInfo.h"
#include "StateButton.h"
#include "talknotify.h"
#include "level_objects.h"

namespace pyrodactyl
{
	namespace ui
	{
		enum HUDSignal { HS_NONE = -1, HS_MAP, HS_PAUSE, HS_CHAR, HS_JOURNAL, HS_INV };

		//The world map, inventory and objective buttons are in the button menu
		class HUD
		{
			//The background image
			ImageData bg;

			//The launcher menu for stuff like inventory, character, pause etc
			Menu<StateButton> menu;

			//The health gem thingy
			//HealthIndicator health;

			//Sprite sheet for animated notify icon
			ImageKey notify_anim;

			//Animated notification icon needs a clip sign
			Rect clip;

			//The amount of time to wait before incrementing clip
			Timer timer;

			//The original tooltips as provided in the xml
			std::vector<std::string> tooltip;

		public:

			GameOverMenu gom;
			PauseMenu pause;
			pyrodactyl::input::HotKey pausekey;
			Button back;

			HUD(){ pausekey.Set(pyrodactyl::input::IG_PAUSE); notify_anim = 0; }
			~HUD(){}

			void InternalEvents(bool ShowMap);
			void PlayerImg(const StateButtonImage &img){ menu.element.at(HS_CHAR).Img(img); }

			void State(const int &val);

			void Load(const std::string &filename, pyrodactyl::level::TalkNotify &tn, pyrodactyl::level::PlayerDestMarker &pdm);
			HUDSignal HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);

			void Draw(pyrodactyl::event::Info &info, const std::string &id);

			//Set the tooltips for the buttons in the menu
			//The tooltips are of the style <Name> (<Hotkey>), with Name being provided by the xml
			void SetTooltip();

			void SetUI();
		};
	}
}
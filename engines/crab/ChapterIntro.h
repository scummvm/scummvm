#pragma once

#include "button.h"
#include "dialogbox.h"
#include "GameEventInfo.h"
#include "sprite.h"

namespace pyrodactyl
{
	namespace ui
	{
		class ChapterIntro
		{
			//This contains the background image info and start button
			GameDialogBox dialog;

			//This is where the sprite is drawn
			Element pos;

			//The traits button
			Button traits;

		public:
			//Should we show the traits screen
			bool show_traits;

			ChapterIntro(){ show_traits = false; }

			void Load(rapidxml::xml_node<char> *node);
			bool HandleEvents(SDL_Event &Event);
			void Draw(pyrodactyl::event::Info &info, std::string &text,
				pyrodactyl::anim::Sprite *cur_sp, const pyrodactyl::people::PersonState &state);
		};
	}
}

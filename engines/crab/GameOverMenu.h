#pragma once

#include "common_header.h"
#include "menu.h"
#include "TextData.h"
#include "ImageData.h"

namespace pyrodactyl
{
	namespace ui
	{
		class GameOverMenu
		{
			//Background image
			ImageData bg;

			//The selection of quotes, we pick one out of these
			std::vector<std::string> quote;

			//The current picked quote
			unsigned int cur;

			//How to draw the quote
			TextData title;

			//The menu for actions we can take
			ButtonMenu menu;

		public:
			GameOverMenu(void){ cur = 0; }
			~GameOverMenu(void){}

			void Reset() { cur = gRandom.Num() % quote.size(); }

			void Load(rapidxml::xml_node<char> *node);
			int HandleEvents(const SDL_Event &Event);
			void Draw();

			void SetUI();
		};
	}
}
#pragma once

#include "common_header.h"
#include "ImageData.h"
#include "ParagraphData.h"
#include "quest.h"
#include "button.h"

namespace pyrodactyl
{
	namespace ui
	{
		class QuestText : public ParagraphData
		{
		protected:
			//How much the text and bullet positions change per line
			Vector2i inc;

			//Color of the highlighted quest
			int col_s;

			//The coordinates for drawing image, which is like bullet points in the form of <Bullet> <Text>
			ImageData img;

			//The lines per page, we split the quest text into multiple pages if we have to draw more than that
			unsigned int lines_per_page;

			//Keep track of which page we are at, and total pages
			unsigned int current_page, total_page;

			//The quest entries we start and stop the drawing at
			int start, stop;

			//The buttons for cycling between pages of the menu
			Button prev, next;

			//Display "Page 1 of 3" style information for the menu
			HoverInfo status;

		public:
			QuestText() { col_s = 0; current_page = 0; start = 0; stop = 0; total_page = 1; lines_per_page = 10; }

			void Load(rapidxml::xml_node<char> *node);

			//Reset the value of current page
			void Reset() { current_page = 0; }

			void HandleEvents(pyrodactyl::event::Quest &q, const SDL_Event &Event);
			void Draw(pyrodactyl::event::Quest &q);

			void SetUI();
		};
	}
}
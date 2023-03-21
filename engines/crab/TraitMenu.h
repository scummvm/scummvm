#pragma once

#include "common_header.h"
#include "TraitButton.h"
#include "ParagraphData.h"
#include "menu.h"
#include "person.h"

namespace pyrodactyl
{
	namespace ui
	{
		class TraitMenu
		{
			//The menu for displaying all the traits
			Menu<TraitButton> menu;

			//The reference button (from which all buttons are initialized)
			TraitButton ref;

			//This vector stores the increments in x,y for each new button
			Vector2i inc;

			//How to draw the selected trait description
			ParagraphData desc;

			//The selected trait, and size of the menu
			int select;

			//The size and dimensions of the menu
			unsigned int size, rows, cols;

		public:
			TraitMenu(){ select = -1; size = 1; rows = 1; cols = 1; }
			~TraitMenu(){}

			void Reset() { select = -1; }
			void Clear();

			void Load(rapidxml::xml_node<char> *node);
			void Draw(const pyrodactyl::people::Person *obj);
			void HandleEvents(pyrodactyl::people::Person *obj, const SDL_Event &Event);

			void Cache(const pyrodactyl::people::Person &obj);
			void SetUI();
		};
	}
}
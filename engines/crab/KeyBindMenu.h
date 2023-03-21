#pragma once

#include "common_header.h"
#include "menu.h"
#include "ImageData.h"
#include "OptionSelect.h"

namespace pyrodactyl
{
	namespace ui
	{
		class KeyBindMenu
		{
			//The keyboard controls menu has 2 types of inputs
			enum Controls { CON_GAME, CON_UI, CON_TOTAL };

			//Each menu can be in these 2 states
			enum States { STATE_NORMAL, STATE_KEY } state;

			//This button swaps between sub-sections "Gameplay" and "Interface"
			OptionSelect sel_controls;

			//These two buttons are the template buttons for the menu
			Button prim, alt;

			//This is the template text info
			TextData desc;

			//inc tells us what to add to the reference buttons to get multiple buttons
			//Divide is the space between two columns
			Vector2i inc, divide;

			//The number of rows and columns
			Vector2i dim;

			//The menu for the keyboard options in both sub categories
			//all control types have equal entries so we just need to change the text displayed
			ButtonMenu menu[CON_TOTAL];

			//The selected button in the current menu
			int choice;

			struct PromptInfo
			{
				int col, col_prev;
				std::string text;

				PromptInfo(){ col = 0; col_prev = 0; }

				void Load(rapidxml::xml_node<char> *node)
				{
					if(NodeValid(node))
					{
						LoadStr(text, "text", node);
						LoadNum(col,"color",node);
					}
				}

				void Swap(Caption &c)
				{
					col_prev = c.col;
					c.text = text;
					c.col = col;
				}
			} prompt;

			void StartAndSize(const int &type, int &start, int &size);
			void InitMenu(const int &type);
			void DrawDesc(const int &type);

		public:
			KeyBindMenu(){ Reset(); choice = -1; }
			~KeyBindMenu(){}

			void Reset(){ state = STATE_NORMAL; }
			void SwapKey(const SDL_Scancode &find);
			bool DisableHotkeys() { return state != STATE_NORMAL; }

			void Load(rapidxml::xml_node<char> *node);
			void HandleEvents(const SDL_Event &Event);
			void SetCaption();

			void Draw();
			void SetUI();
		};
	}
}
#pragma once

#include "common_header.h"
#include "ImageData.h"
#include "TextData.h"
#include "TextManager.h"
#include "timer.h"
#include "button.h"

namespace pyrodactyl
{
	namespace ui
	{
		class CreditScreen
		{
			//Text and formatting information
			struct CreditText
			{
				//Title of the section (stuff like programmer, designer etc)
				std::string text;

				//The style it should be drawn in
				bool heading;
			};

			//The background image and company logo
			pyrodactyl::ui::ImageData bg, logo;

			//The names displayed in the credits
			std::vector<CreditText> list;

			//The starting position
			Vector2i start;

			//The current position
			Vector2D<long> cur;

			//Text parameters
			struct TextParam
			{
				int inc, color;
				FontKey font;
				Align align;

				TextParam(){ inc = 30; color = 0; font = 1; align = ALIGN_CENTER; }

				void Load(rapidxml::xml_node<char> *node)
				{
					LoadNum(inc, "inc", node);
					LoadNum(color, "color", node);
					LoadFontKey(font, "font", node);
					LoadAlign(align, node);
				}
			} heading, paragraph;

			//All speed levels at which credits can scroll through
			struct ScrollSpeed
			{
				//The current speed
				float cur;

				//Various levels
				float slow, fast, reverse;

				ScrollSpeed(){ slow = 1.0f; fast = 4.0f; reverse = -2.0f; cur = slow; }
			} speed;

			//Speed controls for credits
			Button fast, slow, reverse, pause;

			//The back button, website and twitter buttons
			Button back, website, twitter;

		public:
			CreditScreen(){ Reset(); }
			~CreditScreen(){}

			void Load(const std::string &filename);
			void Reset();

			bool HandleEvents(SDL_Event& Event);
			void Draw();

			void SetUI();
		};
	}
}
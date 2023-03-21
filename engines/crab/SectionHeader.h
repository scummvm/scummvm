#pragma once

#include "common_header.h"
#include "TextManager.h"
#include "ImageManager.h"
#include "TextData.h"

namespace pyrodactyl
{
	namespace ui
	{
		class SectionHeader : public TextData
		{
			//The content of the header
			std::string text;

			//This image surrounds the text like <img> text <img>, with the right image being flipped horizontally
			ImageKey img;

			//The coordinates for drawing image
			Vector2i left, right;

			//Should we draw one or both or none of the images
			bool draw_l, draw_r;

		public:
			SectionHeader() { img = 0; draw_l = false; draw_r = false; }
			~SectionHeader(){}

			void Load(rapidxml::xml_node<char> *node);

			void Draw(const int &XOffset = 0, const int &YOffset = 0);
			void Draw(const std::string &str, const int &XOffset = 0, const int &YOffset = 0);
		};
	}
}
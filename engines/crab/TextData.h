#pragma once

#include "common_header.h"
#include "TextManager.h"
#include "element.h"

namespace pyrodactyl
{
	namespace ui
	{
		class TextData : public Element
		{
		public:
			int col;
			FontKey font;
			Align align;
			bool background;

			TextData(void){ col = 0; font = 0; align = ALIGN_LEFT; background = false; }
			~TextData(void){}

			bool Load(rapidxml::xml_node<char> *node, Rect *parent = NULL, const bool &echo = true);

			//Plain drawing
			void Draw(const std::string &val, const int &XOffset = 0, const int &YOffset = 0);

			//Draw with a different color
			void DrawColor(const std::string &val, const int &color, const int &XOffset = 0, const int &YOffset = 0);
		};
	}
}
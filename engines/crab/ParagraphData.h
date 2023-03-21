#pragma once

#include "common_header.h"
#include "TextData.h"

namespace pyrodactyl
{
	namespace ui
	{
		class ParagraphData : public TextData
		{
		public:
			Vector2i line;

			ParagraphData() : line(1, 1){}
			~ParagraphData(){}

			bool Load(rapidxml::xml_node<char> *node, Rect *parent = NULL, const bool &echo = true);

			void Draw(const std::string &val, const int &XOffset = 0, const int &YOffset = 0);
		};
	}
}
#pragma once
#include "common_header.h"
#include "button.h"

namespace pyrodactyl
{
	namespace ui
	{
		//Sometimes we need to display a clipped version of the button image
		class ClipButton : public Button
		{
		public:
			//The clip rectangle
			Rect clip;

			ClipButton(){}
			~ClipButton(){}

			void Load(rapidxml::xml_node<char> * node, const bool &echo = true);
			void Draw(const int &XOffset = 0, const int &YOffset = 0);
		};
	}
}

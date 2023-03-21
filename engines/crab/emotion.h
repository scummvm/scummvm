#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "eventstore.h"
#include "textarea.h"

namespace pyrodactyl
{
	namespace ui
	{
		struct EmotionIndicator
		{
			//The info for drawing the description
			TextData text;

			//This array is used to store the corresponding tone values to a reply
			std::vector<unsigned int> value;

			EmotionIndicator(){}

			void Load(rapidxml::xml_node<char> *node);
			void Draw(const int &select);
			void SetUI();
		};
	}
}
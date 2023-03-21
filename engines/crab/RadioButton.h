#pragma once

#include "ToggleButton.h"

namespace pyrodactyl
{
	namespace ui
	{
		class RadioButton : public ToggleButton
		{
		public:
			//The value associated with the radio button
			float val;

			RadioButton(){ val = 0.0f; }
			~RadioButton(){}

			void Load(rapidxml::xml_node<char> * node)
			{
				ToggleButton::Load(node);
				LoadNum(val, "val", node);
			}
		};
	}
}
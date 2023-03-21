#pragma once

#include "common_header.h"
#include "StateButton.h"
#include "trait.h"

namespace pyrodactyl
{
	namespace ui
	{
		class TraitButton : public StateButton
		{
			//The offset for drawing the trait image
			Vector2i offset;

			//The trait image
			ImageKey trait_img;

		public:

			TraitButton(){ trait_img = 0; }
			~TraitButton(){}

			void Init(const TraitButton &ref, const int &XOffset = 0, const int &YOffset = 0);
			void Load(rapidxml::xml_node<char> * node, const bool &echo = true);

			void Draw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);

			void Cache(const pyrodactyl::people::Trait &trait);
			void Empty();
		};
	}
}

#pragma once

#include "common_header.h"
#include "ScreenSettings.h"
#include "ImageManager.h"
#include "vectors.h"

namespace pyrodactyl
{
	namespace ui
	{
		class Element : public Rect
		{
			//The position loaded directly from xml
			Vector2i raw;

			//Which side of the screen is this object aligned to?
			struct { Align x, y; } align;

			void BasicLoad(rapidxml::xml_node<char> *node, const bool &echo = true);

		public:
			Element() { align.x = ALIGN_LEFT; align.y = ALIGN_LEFT; }
			~Element() {}

			//Initialize an element without loading it from file
			void Init(const int &X, const int &Y, const Align &align_x, const Align &align_y,
				const ImageKey image = 0, const int &W = 0, const int &H = 0);

			//Initialize an element from another
			void Init(const Element &e, ImageKey img = 0, const int &XOffset = 0, const int &YOffset = 0)
			{
				raw.x = e.raw.x + XOffset;
				raw.y = e.raw.y + YOffset;
				Init(e.x + XOffset, e.y + YOffset, e.align.x, e.align.y, img, e.w, e.h);
			}

			//The parent is the object inside which the element exists
			void Load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo = true);

			//The parent is the object inside which the element exists
			void Load(rapidxml::xml_node<char> *node, Rect *parent = NULL, const bool &echo = true);

			void SetUI(Rect *parent = NULL);
		};
	}
}

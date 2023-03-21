#pragma once

#include "common_header.h"
#include "vectors.h"
#include "ImageManager.h"

namespace pyrodactyl
{
	namespace anim
	{
		struct ShadowData
		{
			//The image of the sprite's shadow
			ImageKey img;

			//Size of image
			Vector2i size;

			//The default shadow offset
			Vector2i offset;

			ShadowData() : size(1, 1) { img = 0; }

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadImgKey(img, "img", node);
				offset.Load(node);

				using namespace pyrodactyl::image;
				Image dat;
				gImageManager.GetTexture(img, dat);
				size.x = dat.W() / 2;
				size.y = dat.H() / 2;
			}
		};

		//Used when a set of animation frames needs a specific shadow offset
		class ShadowOffset : public Vector2i
		{
		public:
			//Only use this offset if this is true
			bool valid;

			ShadowOffset() { valid = false; }
		};
	}
}
#pragma once

#include "common_header.h"
#include "ScreenSettings.h"

namespace pyrodactyl
{
	namespace image
	{
		//Image data used in Asset Manager
		class Image
		{
			//The dimensions of the image
			int w, h;

			//The actual hardware texture
			SDL_Texture* texture;

		public:
			Image() : texture(nullptr), w(0), h(0){}
			~Image(){}

			//Set color modulation
			void Color(const Uint8 &r, const Uint8 &g, const Uint8 &b){ SDL_SetTextureColorMod(texture, r, g, b); }

			//Set blending
			void BlendMode(const SDL_BlendMode &mode){ SDL_SetTextureBlendMode(texture, mode); }

			//Set alpha modulation
			int Alpha(const Uint8 &alpha){ return SDL_SetTextureAlphaMod(texture, alpha); }

			//Get alpha modulation
			Uint8 Alpha(){ Uint8 res = 255; SDL_GetTextureAlphaMod(texture, &res); return res; }

			//Load the image
			bool Load(const std::string &path);
			bool Load(rapidxml::xml_node<char> *node, const char* name);
			bool Load(SDL_Surface *surface);

			//Draw the openGL texture
			void Draw(const int &x, const int &y, SDL_Rect *clip = NULL, const TextureFlipType &flip = FLIP_NONE);

			//Delete the openGL texture
			void Delete();

			const int W() { return w; }
			const int H() { return h; }
			const bool Valid() { return texture != nullptr; }
		};
	}
}
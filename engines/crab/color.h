#pragma once

#include "common_header.h"
#include "XMLDoc.h"
#include "loaders.h"

namespace pyrodactyl
{
	namespace text
	{
		//Storage pool for saving colors from one file, then using them
		class ColorPool
		{
			std::vector<SDL_Color> pool;

			//Default invalid color
			SDL_Color invalid;

		public:
			ColorPool() { pool.clear(); invalid.r = 255; invalid.g = 0; invalid.b = 220; }

			SDL_Color& Get(const int &num);
			void Load(const std::string &filename);
		};
	}
}
#pragma once

#include "common_header.h"
#include "ImageManager.h"

namespace pyrodactyl
{
	namespace people
	{
		struct Trait
		{
			//The id of the trait
			int id;

			//Used for achievements
			std::string id_str;

			//The name of the trait
			std::string name;

			//The description of the trait
			std::string desc;

			//The image used to draw the trait
			ImageKey img;

			//Indicator for traits the player hasn't read before
			bool unread;

			Trait(){ id = -1;  img = 0;  unread = true; }
			Trait(rapidxml::xml_node<char> *node){ Load(node); }
			~Trait(){}

			void Clear();
			void Load(rapidxml::xml_node<char> *node);
			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* rootname);
		};
	}
}
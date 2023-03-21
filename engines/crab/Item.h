#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "person.h"
#include "bonus.h"

namespace pyrodactyl
{
	namespace item
	{
		struct Item
		{
			//The id, name and description of the item
			std::string id, name, desc;

			//The image for the item
			ImageKey img;

			//The type of item
			std::string type;

			//The stat bonuses provided by the item
			std::vector<pyrodactyl::stat::Bonus> bonus;

			//The price
			unsigned int value;

			Item(){ Clear(); }
			~Item(){}

			void Clear();
			void StatChange(pyrodactyl::people::Person &obj, bool increase);

			void Load(rapidxml::xml_node<char> *node);
			void Draw(const int &x, const int &y);

			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
		};
	}
}
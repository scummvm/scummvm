#pragma once

#include "ImageManager.h"
#include "bonus.h"
#include "common_header.h"
#include "person.h"

namespace pyrodactyl {
namespace item {
struct Item {
	// The id, name and description of the item
	std::string id, name, desc;

	// The image for the item
	ImageKey img;

	// The type of item
	std::string type;

	// The stat bonuses provided by the item
	std::vector<pyrodactyl::stat::Bonus> bonus;

	// The price
	unsigned int value;

	Item() { Clear(); }
	~Item() {}

	void Clear();
	void StatChange(pyrodactyl::people::Person &obj, bool increase);

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &x, const int &y);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace item
} // End of namespace pyrodactyl
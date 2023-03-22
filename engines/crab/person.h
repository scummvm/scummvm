#pragma once

#include "ImageManager.h"
#include "Stat.h"
#include "StatTemplate.h"
#include "common_header.h"
#include "opinion.h"
#include "personbase.h"
#include "trait.h"

namespace pyrodactyl {
namespace people {
struct Person {
	// The id of the object
	std::string id;

	// Opinion of the person towards the player
	Opinion opinion;

	// The state of the object, defines what behavior it is doing right now
	PersonState state;

	// The stats of the character
	pyrodactyl::stat::StatGroup stat;

	// Name of object
	std::string name;

	// Type of object
	PersonType type;

	// Sometimes a person's journal entry isn't the same as their name
	std::string journal_name;

	// If this is true, use the alternate journal name instead
	bool alt_journal_name;

	// The picture of the object - DISABLED DUE TO LOW BUDGET
	// ImageKey pic;

	// The trigger areas the person is in right now
	std::vector<int> trig;

	// The traits of a person
	std::vector<Trait> trait;

	Person();
	void Load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem);

	void Reset();
	void Validate();

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);
};

typedef std::unordered_map<std::string, Person> PersonMap;
} // End of namespace people
} // End of namespace pyrodactyl
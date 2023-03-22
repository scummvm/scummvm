#pragma once
#include "common_header.h"

#include "GameParam.h"

namespace pyrodactyl {
namespace people {
// What each opinion type is
// like - how much a person likes you
// intimidate - how much a person fears you
// respect - how much a person respects you
enum OpinionType { OPI_LIKE,
				   OPI_RESPECT,
				   OPI_FEAR,
				   OPI_TOTAL };

// The limits on opinion values
extern int OPINION_MIN, OPINION_MAX;

struct Opinion {
	// The opinion of the character about the player
	// Range 0 to 100 , 100 = absolutely adore you, and 0 = really hate you
	int val[OPI_TOTAL];

	Opinion();
	~Opinion() {}

	void Change(const OpinionType &type, const int &change);
	void Set(const OpinionType &type, const int &val);
	void Validate(const OpinionType &type);

	void Load(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace people
} // End of namespace pyrodactyl
#pragma once

#include "common_header.h"
#include "opinion.h"

namespace pyrodactyl {
namespace event {
struct ChangeVal {
	// The person whose opinion is changed
	std::string id;

	// How much does opinion change?
	int val[pyrodactyl::people::OPI_TOTAL];

	ChangeVal() {
		using namespace pyrodactyl::people;
		val[OPI_LIKE] = 0;
		val[OPI_RESPECT] = 0;
		val[OPI_FEAR] = 0;
	}

	ChangeVal(rapidxml::xml_node<char> *node) : ChangeVal() { Load(node); }

	void Load(rapidxml::xml_node<char> *node) {
		using namespace pyrodactyl::people;
		LoadStr(id, "id", node);
		LoadNum(val[OPI_LIKE], "like", node);
		LoadNum(val[OPI_RESPECT], "respect", node);
		LoadNum(val[OPI_FEAR], "fear", node);
	}
};
} // End of namespace event
} // End of namespace pyrodactyl
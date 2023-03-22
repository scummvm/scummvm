#pragma once
#include "common_header.h"

#include "ImageManager.h"
#include "TextManager.h"

namespace pyrodactyl {
namespace stat {
struct Stat {
	// The current, default, minimum and maximum value of the stat
	int cur, def, min, max;

	Stat() {
		min = 0;
		max = 1;
		cur = 1;
		def = 1;
	}

	void Reset() { cur = def; }
	void Validate();

	void Load(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name);
};

struct StatGroup {
	// We group all stats a single person can have
	Stat val[STAT_TOTAL];

	StatGroup() {}
	StatGroup(rapidxml::xml_node<char> *node) { Load(node); }

	void Change(const pyrodactyl::stat::StatType &type, const int &change);
	void Set(const pyrodactyl::stat::StatType &type, const int &val);

	void Load(rapidxml::xml_node<char> *node);
};

StatType StringToStatType(const std::string &val);
const char *StatTypeToString(const StatType &val);
} // End of namespace stat
} // End of namespace pyrodactyl
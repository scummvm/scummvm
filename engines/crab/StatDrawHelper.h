#pragma once
#include "common_header.h"

#include "TextData.h"
#include "person.h"

namespace pyrodactyl {
namespace stat {
struct StatInfo {
	// Used to draw stat value and description
	pyrodactyl::ui::TextData desc;
	std::string text;

	Rect dim;
	ImageKey full, empty;
	bool active;

	StatInfo() {
		active = false;
		full = 0;
		empty = 0;
	}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &val, const int &max);
};

class StatDrawHelper {
	StatInfo info[STAT_TOTAL];

public:
	StatDrawHelper() {}
	~StatDrawHelper() {}

	void Load(rapidxml::xml_node<char> *node);
	void DrawInfo(const pyrodactyl::people::Person &obj);

	const std::string &Name(const StatType &type) { return info[type].text; }
};
} // End of namespace stat
} // End of namespace pyrodactyl
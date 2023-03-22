#pragma once
#include "common_header.h"

#include "TextData.h"

namespace pyrodactyl {
namespace ui {
// Caption for buttons
class Caption : public TextData {
public:
	bool enabled;
	int col_s;

	std::string text;

	Caption(void) {
		col_s = 0;
		enabled = false;
	}
	~Caption(void) {}

	void Init(const Caption &c, const int &XOffset = 0, const int &YOffset = 0);

	void Load(rapidxml::xml_node<char> *node, Rect *parent = NULL);
	void Draw(bool selected, const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl
#pragma once
#include "common_header.h"

#include "TextData.h"

namespace pyrodactyl {
namespace ui {
// Tooltip for buttons
class HoverInfo : public TextData {
public:
	bool enabled;
	std::string text;

	HoverInfo(void) { enabled = false; }
	~HoverInfo(void) {}

	void Init(const HoverInfo &h, const int &XOffset = 0, const int &YOffset = 0);

	void Load(rapidxml::xml_node<char> *node, Rect *parent = NULL);
	void Draw(const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl
#pragma once

#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
// This button is used to draw multiple lines of text instead of an image
class ReplyButton : public Button {
	// Information about drawing reply options
	int col_b, col_s, col_h;
	FontKey font;
	Align align;
	Vector2D<unsigned int> line_size;

	// Reply options get moved around a lot, this remembers their actual position
	Rect orig;

	// The text for this button
	std::string text;

public:
	// The object it points to
	int index;

	ReplyButton() {
		index = 0;
		col_b = 0;
		col_s = 0;
		col_h = 0;
		font = 0;
		align = ALIGN_LEFT;
	}
	~ReplyButton() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &XOffset = 0, const int &YOffset = 0);

	// Used to calculate size and set the string
	// Spacing is the minimum space between buttons added in case of overflow
	// Bottom edge is the y+h value of the previous choice
	void Cache(const std::string &val, const int &spacing, const int &bottom_edge, Rect *parent);
};
} // End of namespace ui
} // End of namespace pyrodactyl

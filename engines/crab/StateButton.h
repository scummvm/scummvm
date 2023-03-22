#pragma once

#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
struct StateButtonImage {
	ButtonImage normal, select;

	StateButtonImage() {}
	StateButtonImage(rapidxml::xml_node<char> *node) {
		if (NodeValid("normal", node))
			normal.Load(node->first_node("normal"));

		if (NodeValid("select", node, false))
			select.Load(node->first_node("select"));
		else
			select = normal;
	}
};

struct StateButtonColor {
	int col, col_s;

	StateButtonColor() {
		col = 0;
		col_s = 0;
	}
};

// This button has two sets of images (b, h, s) and switching is done by clicking it
// Similar to the checkbox UI element in windows/web
class StateButton : public Button {
	// The two images
	StateButtonImage img_set;

	// The color for the caption when the image is selected
	StateButtonColor col_normal, col_select;

public:
	StateButton() {}
	~StateButton() {}

	void Init(const StateButton &ref, const int &XOffset = 0, const int &YOffset = 0);
	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);

	// The state of the button - false is original image, true is second image
	void State(const bool val);
	bool State() { return (img == img_set.select); }

	// Set the image
	void Img(const StateButtonImage &sbi);
};
} // End of namespace ui
} // End of namespace pyrodactyl
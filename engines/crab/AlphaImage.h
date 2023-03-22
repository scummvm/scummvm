#pragma once

#include "ImageManager.h"
#include "common_header.h"
#include "element.h"

namespace pyrodactyl {
namespace ui {
class AlphaImage {
	// The image used - we don't use the image manager
	// because we don't change the alpha for those images
	ImageKey img;

	// The information for drawing the image
	Element pos;

	// The information related to alpha modulation of the image
	struct AlphaVal {
		int cur, min, max;

		// Are we increasing or decreasing the alpha
		bool inc;

		// By how much do we change the alpha every update
		int change;

		AlphaVal() {
			cur = 255;
			min = 255;
			max = 255;
			inc = true;
			change = 0;
		}
	} alpha;

public:
	AlphaImage() {}
	AlphaImage(rapidxml::xml_node<char> *node) { Load(node); }

	~AlphaImage() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);

	void Draw(const int &XOffset = 0, const int &YOffset = 0);

	// This is used to vary the alpha
	void InternalEvents();

	void SetUI() { pos.SetUI(); }
};
} // End of namespace ui
} // End of namespace pyrodactyl
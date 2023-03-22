#pragma once

#include "ImageManager.h"
#include "common_header.h"
#include "element.h"

namespace pyrodactyl {
namespace ui {
class ImageData : public Element {
	// The image
	ImageKey key;

	// Should we clip the image? (usually used for large background images)
	bool crop;

	// The clip rectangle, used only when clip is true
	Rect clip;

public:
	ImageData() {
		key = 0;
		crop = false;
	}
	~ImageData() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void Draw(const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl
#pragma once
#include "common_header.h"

#include "ImageManager.h"

namespace pyrodactyl {
namespace anim {
struct ImageEffect : public Vector2i {
	ImageKey img;
	bool visible;

	ImageEffect() {
		visible = false;
		img = 0;
	}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &XOffset, const int &YOffset);
};
} // End of namespace anim
} // End of namespace pyrodactyl
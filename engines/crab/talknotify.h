#pragma once

#include "GameEventInfo.h"
#include "ImageManager.h"
#include "common_header.h"
#include "sprite.h"

namespace pyrodactyl {
namespace level {
class TalkNotify {
	// The offset to the sprite at which the name is drawn - along with alignment, font and color used to draw the name
	Vector2i offset;
	FontKey font;
	int col;
	Align align;

public:
	TalkNotify() {
		font = 0;
		col = 0;
		align = ALIGN_CENTER;
	}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s, const Rect &camera);
};
} // End of namespace level
} // End of namespace pyrodactyl
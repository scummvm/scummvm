#pragma once

#include "AnimationEffect.h"
#include "HoverInfo.h"
#include "ImageManager.h"
#include "TextManager.h"
#include "common_header.h"

namespace pyrodactyl {
namespace anim {
struct AnimationFrame : public Vector2i {
	// The image drawn in this frame
	ImageKey img;

	// This is the time in we draw the frame milliseconds relative to the start of the entire animation
	Uint32 start, finish;

	// The effect applied to the image
	AnimationEffect eff;

	// In case we want to display any words during the animation
	pyrodactyl::ui::HoverInfo text;

	// The color drawn on the screen
	SDL_Color col;

	AnimationFrame() {
		img = 0;
		start = 0;
		finish = 0;
		col.r = 0;
		col.g = 0;
		col.b = 0;
		col.a = 255;
	}
	AnimationFrame(rapidxml::xml_node<char> *node);

	void Reset();
	void Draw(const Uint32 &timestamp);
	DrawType InternalEvents(const Uint32 &timestamp);
};
} // End of namespace anim
} // End of namespace pyrodactyl
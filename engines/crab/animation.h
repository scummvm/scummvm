#pragma once

#include "AnimationEffect.h"
#include "AnimationFrame.h"
#include "common_header.h"
#include "timer.h"

namespace pyrodactyl {
namespace anim {
class Animation {
	// All the frames are updated simultaneously rather than sequentially
	std::vector<AnimationFrame> frame;

	// Length of the entire animation in milliseconds
	Uint32 length;

	// Keep track of the time
	Timer timer;

public:
	Animation() { length = 0; }
	Animation(rapidxml::xml_node<char> *node);

	void Draw();
	void Reset();

	bool InternalEvents(DrawType &game_draw);
	void Start() {
		Reset();
		timer.Start();
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl

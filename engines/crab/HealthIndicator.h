#pragma once
#include "common_header.h"

#include "ImageManager.h"

namespace pyrodactyl {
namespace ui {
class HealthIndicator {
	struct HealthImage {
		ImageKey normal, glow;
		int val;
	};

	int x, y;
	std::vector<HealthImage> img;

	// Related to the pulse effect
	Uint8 alpha;
	bool inc;

public:
	HealthIndicator() {
		x = 0;
		y = 0;
		alpha = 0;
		inc = true;
	}
	~HealthIndicator() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(int num);
};
} // End of namespace ui
} // End of namespace pyrodactyl
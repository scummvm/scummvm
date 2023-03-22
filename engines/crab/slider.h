#pragma once

#include "Caption.h"
#include "ImageData.h"
#include "ImageManager.h"
#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
class Slider {
	// The value of the slider and the backup
	int value, backup;

	// The slider bar position and dimensions
	ImageData bar;

	// The slider knob
	Button knob;

	// Caption for the slider
	Caption caption;

	// The maximum and minimum values for the slider
	int max, min;

public:
	Slider() {
		max = 100;
		min = 0;
		Value((max - min) / 2);
		backup = value;
	}
	~Slider() {}

	void Load(rapidxml::xml_node<char> *node, const int &Min, const int &Max, const int &Val);

	// Return true if volume changed
	bool HandleEvents(const SDL_Event &Event);
	void Draw();

	int Value() { return value; }
	void Value(const int val);

	void CreateBackup() { backup = value; }
	void RestoreBackup() { Value(backup); }

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl
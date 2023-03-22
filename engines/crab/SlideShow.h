#pragma once

#include "Image.h"
#include "ImageData.h"
#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
class SlideShow {
	// The list of images and where they are located, and the current image
	std::vector<std::string> path;

	// We only load the current image in memory
	pyrodactyl::image::Image img;

	// The index of our current image
	unsigned int index;

	// The position at which map image has to be drawn
	Element pos;

	// Background image of the slide show
	ImageData bg;

	Button prev, next;
	bool usekeyboard;

public:
	SlideShow() {
		index = 0;
		usekeyboard = false;
	}
	~SlideShow() { Clear(); }

	void Refresh();
	void Clear() { img.Delete(); }

	void Load(rapidxml::xml_node<char> *node);
	void HandleEvents(const SDL_Event &Event);
	void Draw();

	void SetUI();
};

extern SlideShow gHelpScreen;
} // End of namespace ui
} // End of namespace pyrodactyl
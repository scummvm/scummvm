#pragma once

#include "ClipButton.h"
#include "common_header.h"
#include "timer.h"

namespace pyrodactyl {
namespace ui {
class ProgressBar : public ClipButton {
	// Whenever the progress bar value is changed, we display a glowing effect
	Timer timer;

	// The total time for which the change effect must be shown
	Uint32 notify_rate;

	// Are we currently displaying the effect?
	bool changed;

	// The effect also depends on if the change was positive or negative, so store the previous value
	int old;

	// If we are drawing an animation, we need to smoothly transition from old->value
	// This stores the current progress
	int cur;

	// The type of effect being drawn
	enum { NONE,
		   INCREASE,
		   DECREASE } type;

	// We reuse the button images for the 2 types of effect
	ImageKey inc, dec;

	// Where to draw the effect
	Vector2i offset;

	// The caption text changes depending on the value of the progress bar - we store all possible text here
	struct CaptionText {
		// The text to be drawn
		std::string text;

		// The above text is drawn only if the progress bar value is greater than this val
		int val;

		CaptionText() { val = 0; }
		CaptionText(rapidxml::xml_node<char> *node) {
			if (!LoadNum(val, "val", node))
				val = 0;

			if (!LoadStr(text, "text", node))
				text = "";
		}
	};

	std::vector<CaptionText> ct;

public:
	ProgressBar() {
		old = 0;
		cur = 0;
		inc = 0;
		dec = 0;
		notify_rate = 5;
		Reset();
	}
	~ProgressBar() {}

	// Reset the effect
	void Reset() {
		changed = false;
		type = NONE;
	}
	void Load(rapidxml::xml_node<char> *node);

	void Draw(const int &value, const int &max);
	void Effect(const int &value, const int &prev);
};
} // End of namespace ui
} // End of namespace pyrodactyl

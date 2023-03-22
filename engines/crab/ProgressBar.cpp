#include "ProgressBar.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ProgressBar::Load(rapidxml::xml_node<char> *node) {
	ClipButton::Load(node);
	LoadNum(notify_rate, "notify", node);

	if (NodeValid("effect", node)) {
		rapidxml::xml_node<char> *effnode = node->first_node("effect");
		LoadImgKey(inc, "inc", effnode);
		LoadImgKey(dec, "dec", effnode);
		offset.Load(effnode);
	}

	if (NodeValid("desc", node)) {
		rapidxml::xml_node<char> *descnode = node->first_node("desc");
		for (rapidxml::xml_node<char> *n = descnode->first_node("above"); n != NULL; n = n->next_sibling("above"))
			ct.push_back(n);
	}
}

void ProgressBar::Draw(const int &value, const int &max) {
	// We don't want divide by zero errors
	if (max == 0)
		return;

	// Figure out which text to draw as caption
	for (auto &i : ct)
		if (value > i.val) {
			caption.text = i.text;
			break;
		}

	// If we don't have to draw animations for changing value, just draw the bar
	if (!changed) {
		clip.w = (gImageManager.GetTexture(img.normal).W() * value) / max;
		ClipButton::Draw();
	} else {
		clip.w = (gImageManager.GetTexture(img.normal).W() * cur) / max;
		ClipButton::Draw();

		switch (type) {
		case INCREASE:
			gImageManager.Draw(x + clip.w + offset.x, y + offset.y, inc);
			if (timer.TargetReached()) {
				cur++;
				timer.Start();
			}
			break;
		case DECREASE:
			gImageManager.Draw(x + clip.w + offset.x, y + offset.y, dec);
			if (timer.TargetReached()) {
				cur--;
				timer.Start();
			}
			break;
		default:
			break;
		}

		if (cur == value)
			changed = false;
	}
}

void ProgressBar::Effect(const int &value, const int &prev) {
	old = prev;
	cur = prev;

	if (value > prev) {
		changed = true;
		type = INCREASE;
		timer.Target(notify_rate * (value - prev));
	} else if (value < prev) {
		changed = true;
		type = DECREASE;
		timer.Target(notify_rate * (prev - value));
	} else {
		changed = false;
		type = NONE;
	}
}
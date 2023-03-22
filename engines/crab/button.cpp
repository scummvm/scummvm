//=============================================================================
// Author:   Arvind
// Purpose:  Contains the button functions
//=============================================================================
#include "button.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;
using namespace pyrodactyl::music;
using namespace pyrodactyl::text;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Button::Button() {
	visible = false;
	canmove = false;
	se_click = -1;
	se_hover = -1;
	hover_prev = false;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button from a file
//------------------------------------------------------------------------
void Button::Load(rapidxml::xml_node<char> *node, const bool &echo) {
	img.Load(node, echo);
	Element::Load(node, img.normal, echo);

	LoadNum(se_click, "click", node, echo);
	LoadNum(se_hover, "hover", node, echo);

	if (NodeValid("hotkey", node, false))
		hotkey.Load(node->first_node("hotkey"));

	tooltip.Load(node->first_node("tooltip"), this);
	caption.Load(node->first_node("caption"), this);

	visible = true;
	canmove = false;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button
//------------------------------------------------------------------------
void Button::Init(const Button &ref, const int &XOffset, const int &YOffset) {
	img = ref.img;
	Element::Init(ref, img.normal, XOffset, YOffset);
	se_click = ref.se_click;
	se_hover = ref.se_hover;

	caption.Init(ref.caption, XOffset, YOffset);
	tooltip.Init(ref.tooltip, XOffset, YOffset);

	visible = true;
	canmove = false;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Reset the button
//------------------------------------------------------------------------
void Button::Reset() {
	mousepressed = false;
	hover_mouse = false;
	hover_key = false;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Button::Draw(const int &XOffset, const int &YOffset, Rect *clip) {
	if (visible) {
		if (mousepressed) {
			gImageManager.Draw(x + XOffset, y + YOffset, img.select, clip);

			tooltip.Draw(XOffset, YOffset);
			caption.Draw(true, XOffset, YOffset);
		} else if (hover_mouse || hover_key) {
			gImageManager.Draw(x + XOffset, y + YOffset, img.hover, clip);

			tooltip.Draw(XOffset, YOffset);
			caption.Draw(true, XOffset, YOffset);
		} else {
			gImageManager.Draw(x + XOffset, y + YOffset, img.normal, clip);
			caption.Draw(false, XOffset, YOffset);
		}
	}
}

void Button::ImageCaptionOnlyDraw(const int &XOffset, const int &YOffset, Rect *clip) {
	if (visible) {
		if (mousepressed) {
			gImageManager.Draw(x + XOffset, y + YOffset, img.select, clip);
			caption.Draw(true, XOffset, YOffset);
		} else if (hover_mouse || hover_key) {
			gImageManager.Draw(x + XOffset, y + YOffset, img.hover, clip);
			caption.Draw(true, XOffset, YOffset);
		} else {
			gImageManager.Draw(x + XOffset, y + YOffset, img.normal, clip);
			caption.Draw(false, XOffset, YOffset);
		}
	}
}

void Button::HoverInfoOnlyDraw(const int &XOffset, const int &YOffset, Rect *clip) {
	if (visible) {
		if (mousepressed || hover_mouse || hover_key)
			tooltip.Draw(XOffset, YOffset);
	}
}

//------------------------------------------------------------------------
// Purpose: Handle input and stuff
//------------------------------------------------------------------------
ButtonAction Button::HandleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset) {
	Rect dim = *this;
	dim.x += XOffset;
	dim.y += YOffset;

	if (visible) {
		if (dim.Contains(gMouse.motion.x, gMouse.motion.y)) {
			hover_mouse = true;

			if (!hover_prev) {
				hover_prev = true;
				gMusicManager.PlayEffect(se_hover, 0);
			}
		} else {
			hover_prev = false;
			hover_mouse = false;
		}

		if (Event.type == SDL_MOUSEMOTION) {
			if (canmove && mousepressed) {
				x += gMouse.rel.x;
				y += gMouse.rel.y;
				return BUAC_GRABBED;
			}
		} else if (Event.type == SDL_MOUSEBUTTONDOWN) {
			// The gMouse button pressed, then released, comprises of a click action
			if (dim.Contains(gMouse.button.x, gMouse.button.y))
				mousepressed = true;
		} else if (Event.type == SDL_MOUSEBUTTONUP && mousepressed) {
			Reset();
			if (dim.Contains(gMouse.button.x, gMouse.button.y)) {
				mousepressed = false;
				if (Event.button.button == SDL_BUTTON_LEFT) {
					gMusicManager.PlayEffect(se_click, 0);
					return BUAC_LCLICK;
				} else if (Event.button.button == SDL_BUTTON_RIGHT)
					return BUAC_RCLICK;
			}
		} else if (hotkey.HandleEvents(Event)) {
			gMusicManager.PlayEffect(se_click, 0);
			return BUAC_LCLICK;
		}
	}

	return BUAC_IGNORE;
}

void Button::SetUI(Rect *parent) {
	Element::SetUI(parent);

	tooltip.SetUI(this);
	caption.SetUI(this);
}
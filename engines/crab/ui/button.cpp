/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

//=============================================================================
// Author:   Arvind
// Purpose:  Contains the button functions
//=============================================================================
#include "crab/crab.h"
#include "crab/ui/button.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;
using namespace pyrodactyl::music;
using namespace pyrodactyl::text;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Button::Button() {
	_visible = false;
	_canmove = false;
	_seClick = -1;
	_seHover = -1;
	_hoverPrev = false;
	reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button from a file
//------------------------------------------------------------------------
void Button::load(rapidxml::xml_node<char> *node, const bool &echo) {
	_img.load(node, echo);
	Element::load(node, _img._normal, echo);

	loadNum(_seClick, "click", node, echo);
	loadNum(_seHover, "hover", node, echo);

	if (nodeValid("hotkey", node, false))
		_hotkey.load(node->first_node("hotkey"));

	_tooltip.load(node->first_node("tooltip"), this);
	_caption.load(node->first_node("caption"), this);

	_visible = true;
	_canmove = false;
	reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button
//------------------------------------------------------------------------
void Button::init(const Button &ref, const int &xOffset, const int &yOffset) {
	_img = ref._img;
	Element::init(ref, _img._normal, xOffset, yOffset);
	_seClick = ref._seClick;
	_seHover = ref._seHover;

	_caption.Init(ref._caption, xOffset, yOffset);
	_tooltip.Init(ref._tooltip, xOffset, yOffset);

	_visible = true;
	_canmove = false;
	reset();
}
//------------------------------------------------------------------------
// Purpose: Reset the button
//------------------------------------------------------------------------
void Button::reset() {
	_mousePressed = false;
	_hoverMouse = false;
	_hoverKey = false;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Button::draw(const int &xOffset, const int &yOffset, Rect *clip) {
	if (_visible) {
		if (_mousePressed) {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._select, clip);

			_tooltip.draw(xOffset, yOffset);
			_caption.draw(true, xOffset, yOffset);
		} else if (_hoverMouse || _hoverKey) {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._hover, clip);

			_tooltip.draw(xOffset, yOffset);
			_caption.draw(true, xOffset, yOffset);
		} else {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._normal, clip);
			_caption.draw(false, xOffset, yOffset);
		}
	}
}

void Button::imageCaptionOnlyDraw(const int &xOffset, const int &yOffset, Rect *clip) {
	if (_visible) {
		if (_mousePressed) {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._select, clip);
			_caption.draw(true, xOffset, yOffset);
		} else if (_hoverMouse || _hoverKey) {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._hover, clip);
			_caption.draw(true, xOffset, yOffset);
		} else {
			g_engine->_imageManager->draw(x + xOffset, y + yOffset, _img._normal, clip);
			_caption.draw(false, xOffset, yOffset);
		}
	}
}

void Button::hoverInfoOnlyDraw(const int &xOffset, const int &yOffset, Rect *clip) {
	if (_visible) {
		if (_mousePressed || _hoverMouse || _hoverKey)
			_tooltip.draw(xOffset, yOffset);
	}
}

//------------------------------------------------------------------------
// Purpose: Handle input and stuff
//------------------------------------------------------------------------
ButtonAction Button::handleEvents(const Common::Event &Event, const int &xOffset, const int &yOffset) {
	Rect dim = *this;
	dim.x += xOffset;
	dim.y += yOffset;

	if (_visible) {
		if (dim.Contains(g_engine->_mouse->_motion.x, g_engine->_mouse->_motion.y)) {
			_hoverMouse = true;

			if (!_hoverPrev) {
				_hoverPrev = true;
				g_engine->_musicManager->PlayEffect(_seHover, 0);
			}
		} else {
			_hoverPrev = false;
			_hoverMouse = false;
		}

		if (Event.type == Common::EVENT_MOUSEMOVE) {
			if (_canmove && _mousePressed) {
				x += g_engine->_mouse->_rel.x;
				y += g_engine->_mouse->_rel.y;
				return BUAC_GRABBED;
			}
		} else if (Event.type == Common::EVENT_LBUTTONDOWN || Event.type == Common::EVENT_RBUTTONDOWN) {
			// The g_engine->_mouse button pressed, then released, comprises of a click action
			if (dim.Contains(g_engine->_mouse->_button.x, g_engine->_mouse->_button.y))
				_mousePressed = true;
		} else if ((Event.type == Common::EVENT_LBUTTONUP || Event.type == Common::EVENT_RBUTTONUP) && _mousePressed) {
			reset();
			if (dim.Contains(g_engine->_mouse->_button.x, g_engine->_mouse->_button.y)) {
				_mousePressed = false;
				if (Event.type == Common::EVENT_LBUTTONUP) {
					g_engine->_musicManager->PlayEffect(_seClick, 0);
					return BUAC_LCLICK;
				} else if (Event.type == Common::EVENT_RBUTTONUP)
					return BUAC_RCLICK;
			}
		} else if (_hotkey.handleEvents(Event)) {
			g_engine->_musicManager->PlayEffect(_seClick, 0);
			return BUAC_LCLICK;
		}
	}

	return BUAC_IGNORE;
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle input and stuff
//------------------------------------------------------------------------
ButtonAction Button::handleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset) {
	Rect dim = *this;
	dim.x += XOffset;
	dim.y += YOffset;

	if (visible) {
		if (dim.Contains(g_engine->_mouse->motion.x, g_engine->_mouse->motion.y)) {
			hover_mouse = true;

			if (!hover_prev) {
				hover_prev = true;
				g_engine->_musicManager->PlayEffect(se_hover, 0);
			}
		} else {
			hover_prev = false;
			hover_mouse = false;
		}

		if (Event.type == SDL_MOUSEMOTION) {
			if (canmove && mousepressed) {
				x += g_engine->_mouse->rel.x;
				y += g_engine->_mouse->rel.y;
				return BUAC_GRABBED;
			}
		} else if (Event.type == SDL_MOUSEBUTTONDOWN) {
			// The g_engine->_mouse button pressed, then released, comprises of a click action
			if (dim.Contains(g_engine->_mouse->button.x, g_engine->_mouse->button.y))
				mousepressed = true;
		} else if (Event.type == SDL_MOUSEBUTTONUP && mousepressed) {
			reset();
			if (dim.Contains(g_engine->_mouse->button.x, g_engine->_mouse->button.y)) {
				mousepressed = false;
				if (Event.button.button == SDL_BUTTON_LEFT) {
					g_engine->_musicManager->PlayEffect(se_click, 0);
					return BUAC_LCLICK;
				} else if (Event.button.button == SDL_BUTTON_RIGHT)
					return BUAC_RCLICK;
			}
		} else if (hotkey.handleEvents(Event)) {
			g_engine->_musicManager->PlayEffect(se_click, 0);
			return BUAC_LCLICK;
		}
	}

	return BUAC_IGNORE;
}
#endif

void Button::setUI(Rect *parent) {
	Element::setUI(parent);

	_tooltip.setUI(this);
	_caption.setUI(this);
}

} // End of namespace Crab

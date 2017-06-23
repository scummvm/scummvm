/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on code by omergilad.

#ifndef GUI_ANIMATION_ANIMATION_H
#define GUI_ANIMATION_ANIMATION_H

#include "gui/animation/Animation.h"

namespace GUI {

class AlphaAnimation: public Animation {
public:
	AlphaAnimation() {}
	virtual ~AlphaAnimation() {}
	float getEndAlpha() const { return _endAlpha; }
	void setEndAlpha(float endAlpha) { _endAlpha = endAlpha; }
	float getStartAlpha() const { return _startAlpha; }
	void setStartAlpha(float startAlpha) { _startAlpha = startAlpha; }

protected:
	virtual void updateInternal(Drawable* drawable, float interpolation) {
		// Calculate alpha value based on properties and interpolation
		drawable->setAlpha(_startAlpha * (1 - interpolation) + _endAlpha * interpolation);
	}

	float _startAlpha;
	float _endAlpha;
};

} // End of namespace GUI

#endif /* GUI_ANIMATION_ANIMATION_H */

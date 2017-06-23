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

#ifndef GUI_ANIMATION_DRAWABLE_H
#define GUI_ANIMATION_DRAWABLE_H

#include "common/ptr.h"
#include "graphics/transparent_surface.h"

#include "gui/animation/Animation.h"

namespace GUI {

class Animation;
typedef Common::SharedPtr<Animation> AnimationPtr;

class Drawable {
public:
	Drawable() :
		_bitmap(NULL), _positionX(0), _positionY(0), _width(0), _height(0), _alpha(1),
		_usingSnapshot(false), _shouldCenter(false) {
		_displayRatio = 1.0;
	}

	virtual ~Drawable() {
		if (_usingSnapshot)
			delete _bitmap;
	}

	void updateAnimation(long currentTime) {
		if (_animation.get() != NULL) {
			_animation->update(this, currentTime);
		}
	}

	bool isAnimationFinished() {
		if (_animation.get() != NULL)
			return _animation->isFinished();

		return false;
	}

	float getAlpha() const { return _alpha; }
	void setAlpha(float alpha) { _alpha = alpha; }
	AnimationPtr getAnimation() const { return _animation; }
	void setAnimation(AnimationPtr animation) { _animation = animation; }
	Graphics::TransparentSurface *getBitmap() const { return _bitmap; }
	void setBitmap(Graphics::TransparentSurface *bitmap) { _bitmap = bitmap; }
	float getPositionX() const { return _positionX; }
	void setPositionX(float positionX) { _positionX = positionX; }
	float getPositionY() const { return _positionY; }
	void setPositionY(float positionY) { _positionY = positionY; }
	virtual float getWidth() const { return _width; }
	void setWidth(float width) { _width = width; }

	virtual float getHeight() const {
		if (_height == 0)
			return getWidth() * _bitmap->getRatio() * _displayRatio;

		return _height;
	}

	void setHeight(float height) { _height = height; }
	void setDisplayRatio(float ratio) { _displayRatio = ratio; }
	inline bool shouldCenter() const { return _shouldCenter; }
	void setShouldCenter(bool shouldCenter) { _shouldCenter = shouldCenter; }

protected:
	bool _usingSnapshot;

private:
	Graphics::TransparentSurface *_bitmap;
	float _positionX;
	float _positionY;
	float _width;
	float _height;
	float _alpha;
	bool _shouldCenter;
	AnimationPtr _animation;

	float _displayRatio;
};

typedef Common::SharedPtr<Drawable> DrawablePtr;

} // End of namespace GUI

#endif /* GUI_ANIMATION_DRAWABLE_H */

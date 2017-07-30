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

#include "bladerunner/ui_image_picker.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/shape.h"

#include "common/rect.h"
#include "graphics/surface.h"

namespace BladeRunner {

struct UIImagePickerImage {
	int          active;
	Common::Rect rect;
	Shape       *shapeUp;
	Shape       *shapeHovered;
	Shape       *shapeDown;
	const char  *tooltip;
};

UIImagePicker::UIImagePicker(BladeRunnerEngine *vm, int imageCount) : _vm(vm) {
	reset();
	_images = new UIImagePickerImage[imageCount];
	_imageCount = imageCount;
	resetImages();
}

UIImagePicker::~UIImagePicker() {
	delete[] _images;
	_images = nullptr;
	reset();
}

void UIImagePicker::resetImages() {
	for (int i = 0; i != _imageCount; i++) {
		resetImage(i);
	}
}

bool UIImagePicker::defineImage(int i, int left, int top, int right, int bottom, Shape *shapeUp, Shape *shapeHovered, Shape *shapeDown, const char *tooltip) {
	if (i < 0 || i >= _imageCount || _images[i].active)
		return false;

	UIImagePickerImage &img = _images[i];

	img.rect.left = left;
	img.rect.top = top;
	img.rect.right = right + 1;
	img.rect.bottom = bottom + 1;
	img.shapeUp = shapeUp;
	img.shapeHovered = shapeHovered;
	img.shapeDown = shapeDown;
	img.active = true;
	img.tooltip = tooltip;

	return true;
}

bool UIImagePicker::setImageTop(int i, int top) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	UIImagePickerImage &img = _images[i];

	img.rect.moveTo(img.rect.left, top);

	return true;
}

bool UIImagePicker::setImageLeft(int i, int left) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	UIImagePickerImage &img = _images[i];

	img.rect.moveTo(left, img.rect.top);

	return true;
}

bool UIImagePicker::setImageShapeUp(int i, Shape *shapeUp) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	_images[i].shapeUp = shapeUp;

	return true;
}

bool UIImagePicker::setImageShapeHovered(int i, Shape *shapeHovered) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	_images[i].shapeHovered = shapeHovered;

	return true;
}

bool UIImagePicker::setImageShapeDown(int i, Shape *shapeDown) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	_images[i].shapeDown = shapeDown;

	return true;
}

bool UIImagePicker::setImageTooltip(int i, const char *tooltip) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	_images[i].tooltip = tooltip;

	return true;
}

bool UIImagePicker::resetActiveImage(int i) {
	if (i < 0 || i >= _imageCount || !_images[i].active)
		return false;

	resetImage(i);
	return true;
}

void UIImagePicker::setCallbacks(UIImagePickerCallback *mouseInCallback,
                                 UIImagePickerCallback *mouseOutCallback,
                                 UIImagePickerCallback *mouseDownCallback,
                                 UIImagePickerCallback *mouseUpCallback,
                                 void *callbackData)
{
	_isButtonDown = false;
	_mouseInCallback   = mouseInCallback;
	_mouseOutCallback  = mouseOutCallback;
	_mouseDownCallback = mouseDownCallback;
	_mouseUpCallback   = mouseUpCallback;
	_callbackData      = callbackData;
	_hoverStartTimestamp = 0;
	_isVisible = true;
	_hoveredImageIndex = -1;
	_pressedImageIndex = -1;
}

void UIImagePicker::resetCallbacks() {}

// TODO
void UIImagePicker::drawTooltip() {}

void UIImagePicker::draw(Graphics::Surface &surface) {
	if (!_isVisible)
		return;

	for (int i = 0; i != _imageCount; ++i) {
		UIImagePickerImage &img = _images[i];
		if (!img.active)
			continue;

		// TODO: Check interaction with Mouse::isDisabled
		if (i == _hoveredImageIndex && i == _pressedImageIndex && _isButtonDown) {
			if (img.shapeDown) {
				img.shapeDown->draw(surface, img.rect.left, img.rect.top);
			}
		} else if (i == _hoveredImageIndex && !_isButtonDown) {
			if (img.shapeHovered) {
				img.shapeHovered->draw(surface, img.rect.left, img.rect.top);
			}
		} else {
			if (img.shapeUp) {
				img.shapeUp->draw(surface, img.rect.left, img.rect.top);
			}
		}
	}
}

void UIImagePicker::handleMouseAction(int x, int y, bool down, bool up, bool ignore) {
	if (!_isVisible || ignore) {
		return;
	}

	int hoveredImageIndex = -1;
	for (int i = 0; i != _imageCount; ++i) {
		if (_images[i].rect.contains(x, y)) {
			hoveredImageIndex = i;
			break;
		}
	}

	// If mouse moved to a new image
	if (hoveredImageIndex != _hoveredImageIndex) {
		if (!_isButtonDown) {
			if (hoveredImageIndex == -1) {
				if (_mouseOutCallback)
					_mouseOutCallback(hoveredImageIndex, _callbackData);
			} else {
				if (_mouseInCallback)
					_mouseInCallback(hoveredImageIndex, _callbackData);
			}
		}
		_hoveredImageIndex = hoveredImageIndex;
	}

	// If mouse button changed to pressed
	if (down && !_isButtonDown) {
		_isButtonDown = true;
		_pressedImageIndex = _hoveredImageIndex;
		if (_mouseDownCallback)
			_mouseDownCallback(_hoveredImageIndex, _callbackData);
	}

	// If mouse button changed to released
	if (up) {
		if (_isButtonDown) {
			if (_hoveredImageIndex == _pressedImageIndex && _pressedImageIndex != -1) {
				if (_mouseUpCallback)
					_mouseUpCallback(_hoveredImageIndex, _callbackData);
			}
		}
		_isButtonDown = false;
		_pressedImageIndex = -1;
	}
}

void UIImagePicker::resetImage(int i) {
	assert(i >= 0 && i < _imageCount);
	UIImagePickerImage &img = _images[i];

	img.active = false;
	img.rect.left = -1;
	img.rect.top = -1;
	img.rect.right = -1;
	img.rect.bottom = -1;
	img.shapeUp = nullptr;
	img.shapeHovered = nullptr;
	img.shapeDown = nullptr;
	img.tooltip = nullptr;
}

bool UIImagePicker::hasHoveredImage() {
	return _hoveredImageIndex >= 0;
}

void UIImagePicker::reset() {}

} // End of namespace BladeRunner

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

#include "bladerunner/ui/ui_image_picker.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/debugger.h"
#include "bladerunner/font.h"
#include "bladerunner/mouse.h"
#include "bladerunner/shape.h"
#include "bladerunner/time.h"

#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace BladeRunner {

UIImagePicker::UIImagePicker(BladeRunnerEngine *vm, int imageCount) {
	_vm = vm;
	reset();
	_images.resize(imageCount);
	_imageCount = imageCount;
	resetImages();
}

UIImagePicker::~UIImagePicker() {
	_images.clear();
	reset();
}

void UIImagePicker::resetImages() {
	for (int i = 0; i != _imageCount; ++i) {
		resetImage(i);
	}
}

bool UIImagePicker::defineImage(int i, Common::Rect rect, const Shape *shapeUp, const Shape *shapeHovered, const Shape *shapeDown, const char *tooltip) {
	if (i < 0 || i >= _imageCount || _images[i].active) {
		return false;
	}

	Image &img = _images[i];

	img.rect = rect;
	// for rect to be inclusive
	++(img.rect.right);
	++(img.rect.bottom);
	img.shapeUp = shapeUp;
	img.shapeHovered = shapeHovered;
	img.shapeDown = shapeDown;
	img.active = true;

	if (tooltip != nullptr) {
		img.tooltip = tooltip;
	} else {
		img.tooltip.clear();
	}

	return true;
}

bool UIImagePicker::setImageTop(int i, int top) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	img.rect.moveTo(img.rect.left, top);

	return true;
}

bool UIImagePicker::setImageLeft(int i, int left) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	img.rect.moveTo(left, img.rect.top);

	return true;
}

bool UIImagePicker::setImageWidth(int i, int16 width) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	img.rect.setWidth(width);

	return true;
}

bool UIImagePicker::setImageShapeUp(int i, const Shape *shapeUp) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	_images[i].shapeUp = shapeUp;

	return true;
}

bool UIImagePicker::setImageShapeHovered(int i, const Shape *shapeHovered) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	_images[i].shapeHovered = shapeHovered;

	return true;
}

bool UIImagePicker::setImageShapeDown(int i, const Shape *shapeDown) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	_images[i].shapeDown = shapeDown;

	return true;
}

bool UIImagePicker::setImageTooltip(int i, const char *tooltip) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	if (tooltip != nullptr) {
		_images[i].tooltip = tooltip;
	} else {
		_images[i].tooltip.clear();
	}

	return true;
}

int UIImagePicker::getImageTop(int i) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	return img.rect.top;
}

int UIImagePicker::getImageLeft(int i) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	return img.rect.left;
}

int UIImagePicker::getImageWidth(int i) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	Image &img = _images[i];

	return img.rect.width();
}

bool UIImagePicker::resetActiveImage(int i) {
	if (i < 0 || i >= _imageCount || !_images[i].active) {
		return false;
	}

	resetImage(i);
	return true;
}

void UIImagePicker::activate(UIImagePickerCallback *mouseInCallback,
                             UIImagePickerCallback *mouseOutCallback,
                             UIImagePickerCallback *mouseDownCallback,
                             UIImagePickerCallback *mouseUpCallback,
                             void *callbackData) {
	_isButtonDown        = false;
	_mouseInCallback     = mouseInCallback;
	_mouseOutCallback    = mouseOutCallback;
	_mouseDownCallback   = mouseDownCallback;
	_mouseUpCallback     = mouseUpCallback;
	_callbackData        = callbackData;
	_hoverStartTimestamp = 0u;
	_isVisible           = true;
	_hoveredImageIndex   = -1;
	_pressedImageIndex   = -1;
}

void UIImagePicker::deactivate() {
	_isButtonDown        = false;
	_mouseInCallback     = nullptr;
	_mouseOutCallback    = nullptr;
	_mouseDownCallback   = nullptr;
	_mouseUpCallback     = nullptr;
	_callbackData        = nullptr;
	_hoverStartTimestamp = 0u;
	_isVisible           = false;
	_hoveredImageIndex   = -1;
	_pressedImageIndex   = -1;
}

void UIImagePicker::draw(Graphics::Surface &surface) {
	if (!_isVisible) {
		return;
	}

	for (int i = 0; i != _imageCount; ++i) {
		Image &img = _images[i];
		if (!img.active) {
			continue;
		}

		if (i == _hoveredImageIndex && i == _pressedImageIndex && _isButtonDown
			&& !_vm->_mouse->isDisabled()
			&& img.shapeDown) {
			img.shapeDown->draw(surface, img.rect.left, img.rect.top);
		} else if (i == _hoveredImageIndex && !_isButtonDown
			&& !_vm->_mouse->isDisabled()
			&& img.shapeHovered) {
			img.shapeHovered->draw(surface, img.rect.left, img.rect.top);
		} else {
			// this shape should always be the fall back shape to prevent blinking
			if (img.shapeUp) {
				img.shapeUp->draw(surface, img.rect.left, img.rect.top);
			}
		}

		if (_vm->_debugger->_viewUI) {
			surface.frameRect(img.rect, surface.format.RGBToColor(255, 255, 255));
			_vm->_mainFont->drawString(&surface, Common::String::format("%d", i), (img.rect.left + img.rect.right) / 2, (img.rect.top + img.rect.bottom) / 2, surface.w, surface.format.RGBToColor(255, 255, 255));
		}
	}
}

void UIImagePicker::drawTooltip(Graphics::Surface &surface, int x, int y) {
	if (!_isVisible) {
		return;
	}

	if (
		(_hoveredImageIndex == -1) ||
		(_vm->_mouse->isDisabled()) ||
		(!_images[_hoveredImageIndex].active) ||
		(_vm->_time->current() - _hoverStartTimestamp < 1000u)
	) { // unsigned difference is intentional (time difference)
		return;
	}

	Common::String &tooltip = _images[_hoveredImageIndex].tooltip;

	if (tooltip.empty()) {
		return;
	}

	int width = _vm->_mainFont->getStringWidth(tooltip) + 1;
	int height = _vm->_mainFont->getFontHeight() + 1;

	Common::Rect rect;
	rect.left = x - ((width / 2) + 1);
	if (rect.left < 0) {
		rect.left = 0;
	}

	rect.top = y - 10;
	if (rect.top < 0) {
		rect.top = 0;
	}

	rect.right = width + rect.left + 3;
	if (rect.right >= 640) {
		rect.right = 639;
		rect.left = 636 - width;
	}

	rect.bottom = height + rect.top + 2;
	if (rect.bottom >= 480) {
		rect.bottom = 479;
		rect.top = 478 - height;
	}

	surface.fillRect(rect, surface.format.RGBToColor(0, 0, 0));
	surface.frameRect(rect, surface.format.RGBToColor(255, 255, 255));
	_vm->_mainFont->drawString(&surface, tooltip, rect.left + 2, rect.top, surface.w, surface.format.RGBToColor(255, 255, 255));
}

bool UIImagePicker::handleMouseAction(int x, int y, bool down, bool up, bool ignore) {
	if (!_isVisible || ignore) {
		return false;
	}
	bool actionHandled = false;
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
				if (_mouseOutCallback) {
					_mouseOutCallback(hoveredImageIndex, _callbackData);
				}
			} else {
				if (_mouseInCallback) {
					_mouseInCallback(hoveredImageIndex, _callbackData);
				}
			}
		}
		_hoverStartTimestamp = _vm->_time->current();
		_hoveredImageIndex = hoveredImageIndex;
	}

	// If mouse button changed to pressed
	if (down && !_isButtonDown) {
		_isButtonDown = true;
		_pressedImageIndex = _hoveredImageIndex;
		if (_hoveredImageIndex != -1) {
			if (_mouseDownCallback) {
				_mouseDownCallback(_hoveredImageIndex, _callbackData);
				actionHandled = true;
			}
		}
	}

	// If mouse button changed to released
	if (up) {
		if (_isButtonDown) {
			if (_hoveredImageIndex == _pressedImageIndex && _pressedImageIndex != -1) {
				if (_mouseUpCallback) {
					_mouseUpCallback(_hoveredImageIndex, _callbackData);
					actionHandled = true;
				}
			}
		}
		_isButtonDown = false;
		_pressedImageIndex = -1;
	}

	return actionHandled;
}

void UIImagePicker::resetImage(int i) {
	assert(i >= 0 && i < _imageCount);
	Image &img = _images[i];

	img.active = false;
	img.rect.left = -1;
	img.rect.top = -1;
	img.rect.right = -1;
	img.rect.bottom = -1;
	img.shapeUp = nullptr;
	img.shapeHovered = nullptr;
	img.shapeDown = nullptr;
	img.tooltip.clear();
}

bool UIImagePicker::hasHoveredImage() {
	return _hoveredImageIndex >= 0;
}

void UIImagePicker::reset() {
	_isVisible           = false;
	_hoveredImageIndex   = -1;
	_pressedImageIndex   = -1;
	_hoverStartTimestamp = 0u;
	_isButtonDown        = false;
	_mouseInCallback     = nullptr;
	_mouseOutCallback    = nullptr;
	_mouseDownCallback   = nullptr;
	_mouseUpCallback     = nullptr;
	_callbackData        = nullptr;
}

} // End of namespace BladeRunner

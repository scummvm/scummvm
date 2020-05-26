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

#ifndef BLADERUNNER_UI_IMAGE_PICKER_H
#define BLADERUNNER_UI_IMAGE_PICKER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;
class Shape;

typedef void UIImagePickerCallback(int, void *);

class UIImagePicker {
	struct Image {
		int             active;
		Common::Rect    rect;
		const Shape    *shapeUp;
		const Shape    *shapeHovered;
		const Shape    *shapeDown;
		Common::String  tooltip;
	};

	BladeRunnerEngine *_vm;

	int    _isVisible;
	int    _imageCount;
	int    _hoveredImageIndex;
	int    _pressedImageIndex;
	uint32 _hoverStartTimestamp;
	int    _isButtonDown;
	Common::Array<Image> _images;

	UIImagePickerCallback *_mouseInCallback;
	UIImagePickerCallback *_mouseOutCallback;
	UIImagePickerCallback *_mouseDownCallback;
	UIImagePickerCallback *_mouseUpCallback;
	void *_callbackData;

public:
	UIImagePicker(BladeRunnerEngine *vm, int imageCount);
	~UIImagePicker();

	void resetImages();
	bool defineImage(int i, Common::Rect rect, const Shape *shapeUp, const Shape *shapeHovered, const Shape *shapeDown, const char *tooltip);

	bool setImageTop(int i, int top);
	bool setImageLeft(int i, int left);
	bool setImageWidth(int i, int16 width);
	bool setImageShapeUp(int i, const Shape *shapeUp);
	bool setImageShapeHovered(int i, const Shape *shapeHovered);
	bool setImageShapeDown(int i, const Shape *shapeDown);
	bool setImageTooltip(int i, const char *tooltip);

	int getImageTop(int i);
	int getImageLeft(int i);
	int getImageWidth(int i);

	bool resetActiveImage(int i);

	void activate(UIImagePickerCallback *mouseInCallback,
	              UIImagePickerCallback *mouseOutCallback,
	              UIImagePickerCallback *mouseDownCallback,
	              UIImagePickerCallback *mouseUpCallback,
	              void *callbackData);

	void deactivate();

	void draw(Graphics::Surface &surface);
	void drawTooltip(Graphics::Surface &surface, int x, int y);

	bool handleMouseAction(int x, int y, bool down, bool up, bool ignore = false);

	void resetImage(int i);
	bool hasHoveredImage();

	void reset();
};

} // End of namespace BladeRunner

#endif

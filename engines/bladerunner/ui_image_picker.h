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

namespace Graphics {
struct Surface;
}

namespace BladeRunner {

class BladeRunnerEngine;
class Shape;
struct UIImagePickerImage;

typedef void UIImagePickerCallback(int, void*);

class UIImagePicker {
	BladeRunnerEngine *_vm;

	int _isVisible;
	int _imageCount;
	int _hoveredImageIndex;
	int _pressedImageIndex;
	int _hoverStartTimestamp;
	int _isButtonDown;
	UIImagePickerImage *_images;

	UIImagePickerCallback *_mouseInCallback;
	UIImagePickerCallback *_mouseOutCallback;
	UIImagePickerCallback *_mouseDownCallback;
	UIImagePickerCallback *_mouseUpCallback;
	void *_callbackData;

public:
	UIImagePicker(BladeRunnerEngine *vm, int imageCount);
	~UIImagePicker();

	void resetImages();
	bool defineImage(int i, int left, int top, int right, int bottom, Shape *shapeUp, Shape *shapeHovered, Shape *shapeDown, const char *tooltip);

	bool setImageTop(int i, int top);
	bool setImageLeft(int i, int left);
	bool setImageShapeUp(int i, Shape *shapeUp);
	bool setImageShapeHovered(int i, Shape *shapeHovered);
	bool setImageShapeDown(int i, Shape *shapeDown);
	bool setImageTooltip(int i, const char *tooltip);

	bool resetActiveImage(int i);

	void setCallbacks(UIImagePickerCallback *mouseInCallback,
	                  UIImagePickerCallback *mouseOutCallback,
	                  UIImagePickerCallback *mouseDownCallback,
	                  UIImagePickerCallback *mouseUpCallback,
	                  void *callbackData);

	void resetCallbacks();

	void drawTooltip();
	void draw(Graphics::Surface &surface);

	void handleMouseAction(int x, int y, bool down, bool up, bool ignore = false);

	void resetImage(int i);
	bool hasHoveredImage();

	void reset();
};

} // End of namespace BladeRunner

#endif

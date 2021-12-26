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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/views/resviewer.h"

#include "asylum/system/cursor.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

#define SCROLL_STEP 10

#define NOPALETTE {0, 0, 0, 0, 0, 0, 0, 0}
static int paletteIds[][8] {
	NOPALETTE,
	{0x0011, 0x001A, 0x001F, 0x003B, 0x003C},
	NOPALETTE,
	NOPALETTE,
	NOPALETTE,
	{0x0014, 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x00B7, 0x00CD},
	{0x0014, 0x0020, 0x026C, 0x0277, 0x02A6},
	{0x0014, 0x0018, 0x0019, 0x001A, 0x0163, 0x0191},
	{0x0014, 0x0018, 0x01BC},
	{0x0014, 0x008C, 0x00AB, 0x00D4},
	{0x0014, 0x001E, 0x001F, 0x0171, 0x0185, 0x01C0},
	{0x0012, 0x001F, 0x011B, 0x011E, 0x0120, 0x0135, 0x0159},
	{0x0014, 0x018F, 0x01A3, 0x01B0, 0x01C1},
	{0x0014, 0x019D, 0x019E, 0x019F, 0x01A7},
	{0x0014, 0x001E, 0x001F, 0x0121, 0x0124},
	{0x0014, 0x00AD},
	{0x0014, 0x0018, 0x0095},
	{0x0014, 0x006B},
	{0x000B, 0x000C, 0x000D}
};

static const int resPackSizes[] = {
	4112, 61, 28, 534, 1, 215, 701, 418, 458, 230, 475, 354, 461, 459, 310, 176, 168, 121, 22
};

ResourceViewer::ResourceViewer(AsylumEngine *engine) : _vm(engine), _resource(_vm) {
	_handler = nullptr;
	_resourceId = kResourceNone;
	_frameIndex = _frameCount = 0;
	_frameIncrement = 1;
	_x = _y = 0;
	_width = _height = 0;
	_scroll = false;
	_resPack = -1;
	_paletteIndex = 0;
	_animate = true;
}

bool ResourceViewer::setResourceId(ResourceId resourceId) {
	if (resourceId == kResourceNone ||
		!getResource()->get(resourceId) ||
		strncmp((const char *)getResource()->get(resourceId)->data, "D3GR", 4))

		return false;

	_resourceId = resourceId;
	_frameIndex = 0;
	_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);

	_resource.load(_resourceId);

	_frameIncrement = 1;
	_x = _y = 0;

	if (isPalette(_resourceId)) {
		_width  = 0;
		_height = 0;
	} else {
		_width  = _resource.getFrame(0)->getWidth();
		_height = _resource.getFrame(0)->getHeight();
	}

	_scroll = _width > 640 || _height > 480;
	_resPack = RESOURCE_PACK(_resourceId);
	_paletteIndex = 0;

	int fontIndex = 13;
	if (_resPack == 1)
		fontIndex = 16;
	else if (_resPack == 18)
		fontIndex = 19;
	getText()->loadFont(MAKE_RESOURCE(_resPack, fontIndex));

	return true;
}

void ResourceViewer::drawResource() {
	int16 x, y;
	GraphicFrame *frame = _resource.getFrame(_frameIndex);

	if (_scroll) {
		x = _x;
		y = _y;
	} else {
		x = (640 - frame->getWidth())  / 2 - frame->x;
		y = (480 - frame->getHeight()) / 2 - frame->y;
	}

	getScreen()->setPalette(MAKE_RESOURCE(_resPack, paletteIds[_resPack][_paletteIndex]));
	getScreen()->draw(_resourceId, _frameIndex, Common::Point(x, y));

	if (_frameCount > 1 && _animate) {
		if (_frameIndex + 1 >= (int)_frameCount)
			_frameIncrement = -1;
		else if (_frameIndex == 0)
			_frameIncrement =  1;

		_frameIndex += _frameIncrement;
	}
}

bool ResourceViewer::isPalette(ResourceId resourceId) {
	return getResource()->get(resourceId)->size == 800;
}

void ResourceViewer::drawPalette() {
	const int size = 20;
	const int x0 = (640 - size * 16) / 2, y0 = (480 - size * 16) / 2;

	getScreen()->setPalette(_resourceId);
	for (int i = 0, color = 0; i < 16; i++)
		for (int j = 0; j < 16; j++, color++)
			getScreen()->fillRect(x0 + j * size, y0 + i * size, size, size, color);
}

void ResourceViewer::update() {
	getCursor()->hide();
	getScreen()->clear();

	if (isPalette(_resourceId))
		drawPalette();
	else
		drawResource();

	getText()->draw(Common::Point(615, 440), Common::String::format("%X", _resourceId).c_str());
	getScreen()->copyBackBufferToScreen();
}

void ResourceViewer::key(const AsylumEvent &evt) {
	switch (evt.kbd.keycode) {
	default:
		break;

	case Common::KEYCODE_ESCAPE:
		_vm->switchEventHandler(_handler);
		break;

	case Common::KEYCODE_SPACE:
		if (RESOURCE_INDEX(_resourceId) < resPackSizes[_resPack] - 1) {
			int i = 1;
			do {
				if (setResourceId(_resourceId + i))
					break;
				i++;
			} while (RESOURCE_INDEX(_resourceId + i) < resPackSizes[_resPack] - 1);
		}
		break;

	case Common::KEYCODE_BACKSPACE:
		if (RESOURCE_INDEX(_resourceId)) {
			int i = 0;
			do {
				i++;
				if (setResourceId(_resourceId - i))
					break;
			} while (RESOURCE_INDEX(_resourceId - i));
		}
		break;

	case Common::KEYCODE_RETURN:
		_animate = !_animate;
		break;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_LEFT:
		if (_scroll) {
			int16 x = _x, y = _y;
			int dir = (int)(evt.kbd.keycode - Common::KEYCODE_UP);

			if (dir < 2)
				y -= SCROLL_STEP * (2 * dir - 1);
			else
				x -= SCROLL_STEP * (1 - 2 * (dir - 2));

			if (640 - x <= _width && x <= 0 && 480 - y <= _height && y <= 0) {
				_x = x;
				_y = y;
			}
		}
		break;

	case Common::KEYCODE_PAGEUP:
		if (_paletteIndex)
			_paletteIndex = _paletteIndex - 1;
		break;

	case Common::KEYCODE_PAGEDOWN:
		if (_paletteIndex < 8 && paletteIds[_resPack][_paletteIndex + 1])
			_paletteIndex = _paletteIndex + 1;
		break;
	}
}

bool ResourceViewer::handleEvent(const AsylumEvent &evt) {
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_UPDATE:
		update();
		return true;

	case Common::EVENT_KEYDOWN:
		key(evt);
		return true;
	}

	return false;
}

} // End of namespace Asylum

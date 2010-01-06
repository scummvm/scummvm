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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/gfx.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/robot.h"

namespace Sci {

#ifdef ENABLE_SCI32
Robot::Robot(ResourceManager *resMan, Screen *screen, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

Robot::~Robot() {
	_resMan->unlockResource(_resource);
}

void Robot::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeRobot, resourceId), true);
	if (!_resource) {
		error("robot resource %d not found", resourceId);
	}
	_resourceData = _resource->data;

	_width = READ_LE_UINT16(_resourceData + 10);
	_height = READ_LE_UINT16(_resourceData + 12);
}

// TODO: just trying around in here...

void Robot::draw() {
	byte *bitmapData = _resourceData + 0x48;
	int x, y;
	//int frame;

	//for (frame = 0; frame < 30; frame++) {
	for (y = 0; y < _height; y++) {
		for (x = 0; x < _width; x++) {
			_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, *bitmapData, 0, 0);
			bitmapData++;
		}
	}
	//}
	_screen->copyToScreen();
}
#endif

} // End of namespace Sci

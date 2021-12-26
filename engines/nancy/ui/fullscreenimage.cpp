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

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {
namespace UI {

void FullScreenImage::init(const Common::String &imageName) {
	g_nancy->_resource->loadImage(imageName, _drawSurface);

	Common::Rect srcBounds = Common::Rect(0,0, _drawSurface.w, _drawSurface.h);
	_screenPosition = srcBounds;

	RenderObject::init();
}

} // End of namespace UI
} // End of namespace Nancy

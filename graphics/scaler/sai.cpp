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
 */

#include "graphics/scaler/sai.h"
#include "graphics/scaler.h"

SAIPlugin::SAIPlugin() {
	_factor = 1;
	_factors.push_back(1);
	_factors.push_back(2);
}

void SAIPlugin::initialize(Graphics::PixelFormat format) {
	_format = format;
}

void SAIPlugin::deinitialize() {
	//DestroyScalers();
}

void SAIPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (!_doScale) {
		scale1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height, _format.bytesPerPixel);
		return;
	}
	switch (_factor) {
	case 1:
		scale1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height, _format.bytesPerPixel);
		break;
	case 2:
		_2xSaI(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	}
}

uint SAIPlugin::increaseFactor() {
	if (_factor < 2)
		++_factor;
	return _factor;
}

uint SAIPlugin::decreaseFactor() {
	if (_factor > 1)
		--_factor;
	return _factor;
}

const char *SAIPlugin::getName() const {
	return "SAI";
}


REGISTER_PLUGIN_STATIC(SAI, PLUGIN_TYPE_SCALER, SAIPlugin);

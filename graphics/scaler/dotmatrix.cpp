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

#include "graphics/scaler/dotmatrix.h"
#include "graphics/scaler.h"

void DotMatrixPlugin::initialize(Graphics::PixelFormat format) {
	// TODO: initialize dotmatrix array
}

void DotMatrixPlugin::scale(const uint8 *srcPtr, uint32 srcPitch,
							uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (!_doScale) {
		Normal1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		return;
	}
	switch (_factor) {
	case 1:
		Normal1x(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	case 2:
		DotMatrix(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		break;
	}
}

uint DotMatrixPlugin::increaseFactor() {
	if (_factor < 2)
		++_factor;
	return _factor;
}

uint DotMatrixPlugin::decreaseFactor() {
	if (_factor > 1)
		--_factor;
	return _factor;
}

const char *DotMatrixPlugin::getName() const {
	return "DotMatrix";
}


REGISTER_PLUGIN_STATIC(DOTMATRIX, PLUGIN_TYPE_SCALER, DotMatrixPlugin);

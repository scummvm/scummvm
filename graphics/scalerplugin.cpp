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

#include "graphics/scalerplugin.h"

SourceScaler::SourceScaler() {
	for (int i = 0; i < SRC_MAX; ++i) {
		oldSrcs[i] = NULL;
	}
}

void SourceScaler::setSource(const byte *src, uint pitch, int width, int height, int padding, SourceType type) {
	widths[type] = width;
	heights[type] = height;
	pitches[type] = pitch;
	newSrcs[type] = src;
	paddings[type] = padding;

	if (oldSrcs[type] != NULL)
		free(oldSrcs[type]);

	int size = (height + _format.bytesPerPixel * padding) * 2 * pitch;
	oldSrcs[type] = new byte[size];
	memset(oldSrcs[type], 0, size);
}

void SourceScaler::oldSrcScale(byte *dst, uint dstPitch, SourceType type) {
	// Call user defined scale function
	internScale(newSrcs[type] + paddings[type] * 2 + pitches[type] * paddings[type], pitches[type],
	            dst, dstPitch,
	            oldSrcs[type] + paddings[type] * 2 + pitches[type] * paddings[type], pitches[type],
				widths[type], heights[type]);
	// Update old src
	memcpy(oldSrcs[type], newSrcs[type], (heights[type] + _format.bytesPerPixel * paddings[type])  * 2 * pitches[type]);
}


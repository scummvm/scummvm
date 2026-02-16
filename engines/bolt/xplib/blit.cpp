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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

void XpLib::blit(void *src, uint16 srcStride, void *dst, uint16 dstStride, uint16 width, uint16 height) {

}

void XpLib::dirtyBlit(void *src, void *dst, void *dirtyFlags, uint16 width, uint16 height) {

}

void XpLib::maskBlit(void *src, uint16 srcStride, void *dst, uint16 dstStride, uint16 width, uint16 height) {

}

uint32 XpLib::compositeBlit(void *src, void *background, void *dst, uint16 stride, uint16 width, uint16 height) {
	return 0;
}

void XpLib::rleBlit(void *src, void *dst, uint16 dstStride, uint16 width, uint16 height) {

}

void XpLib::rleMaskBlit(void *src, void *dst, uint16 dstStride, uint16 width, uint16 height) {

}

uint32 XpLib::rleCompositeBlit(void *rle, void *background, void *dst, uint16 width, uint16 height, void *dirtyFlags) {
	return 0;
}

uint16 XpLib::rleDataSize(void *rleData, uint16 height) {
	return 0;
}

void XpLib::markCursorPixels(void *buffer, uint32 count) {

}

} // End of namespace Bolt

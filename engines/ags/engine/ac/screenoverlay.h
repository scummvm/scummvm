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

#ifndef AGS_ENGINE_AC_SCREENOVERLAY_H
#define AGS_ENGINE_AC_SCREENOVERLAY_H

#include <stdint.h>

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Bitmap;
class Stream;
} // namespace Shared
} // namespace AGS

namespace AGS {
namespace Engine {
class IDriverDependantBitmap;
} // namespace Engine
} // namespace AGS

using namespace AGS; // FIXME later


struct ScreenOverlay {
	Engine::IDriverDependantBitmap *bmp = nullptr;
	Common::Bitmap *pic = nullptr;
	int type = 0, x = 0, y = 0, timeout = 0;
	int bgSpeechForChar = 0;
	int associatedOverlayHandle = 0;
	bool hasAlphaChannel = false;
	bool positionRelativeToScreen = false;
	bool hasSerializedBitmap = false;
	int _offsetX = 0, _offsetY = 0;

	void ReadFromFile(Common::Stream *in, int32_t cmp_ver);
	void WriteToFile(Common::Stream *out) const;
};

} // namespace AGS3

#endif

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

//=============================================================================
//
// Driver-dependant bitmap interface
//
//=============================================================================

#ifndef AGS_ENGINE_GFX_DDB_H
#define AGS_ENGINE_GFX_DDB_H

namespace AGS3 {
namespace AGS {
namespace Engine {

class IDriverDependantBitmap {
public:
	virtual ~IDriverDependantBitmap() = default;

	virtual void SetTransparency(int transparency) = 0;  // 0-255
	virtual void SetFlippedLeftRight(bool isFlipped) = 0;
	virtual void SetStretch(int width, int height, bool useResampler = true) = 0;
	virtual void SetLightLevel(int light_level) = 0;   // 0-255
	virtual void SetTint(int red, int green, int blue, int tintSaturation) = 0;  // 0-255

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual int GetColorDepth() = 0;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif

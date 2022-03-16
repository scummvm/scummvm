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

//=============================================================================
//
// Driver-dependant bitmap interface
//
// TODO: split into texture object that has only tex data
// and object describing a drawing operation, with ref to texture and
// drawing parameters (modes, shaders, etc).
// Then we will also be able to share one texture among multiple game entities.
//=============================================================================

#ifndef AGS_ENGINE_GFX_DDB_H
#define AGS_ENGINE_GFX_DDB_H

namespace AGS3 {
namespace AGS {
namespace Engine {

class IDriverDependantBitmap {
public:
	virtual int  GetTransparency() const = 0;
	virtual void SetTransparency(int transparency) = 0;  // 0-255
	virtual void SetFlippedLeftRight(bool isFlipped) = 0;
	virtual void SetStretch(int width, int height, bool useResampler = true) = 0;
	virtual void SetLightLevel(int light_level) = 0;   // 0-255
	virtual void SetTint(int red, int green, int blue, int tintSaturation) = 0;  // 0-255

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual int GetColorDepth() const = 0;

protected:
	IDriverDependantBitmap() {}
	virtual ~IDriverDependantBitmap() {}
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif

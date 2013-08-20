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
 */

#ifndef ZVISION_RLF_ANIMATION_H
#define ZVISION_RLF_ANIMATION_H


namespace Common {
class String;
}

namespace ZVision {

class RlfAnimation {
public:
	RlfAnimation(const Common::String &fileName);
	~RlfAnimation();

private:
private:
	uint _frameCount;
	uint _width;
	uint _height;
	uint32 _frameTime; // In milliseconds
	uint16 **_frames;

public:
	uint frameCount() { return _frameCount; }
	uint width() { return _width; }
	uint height() { return _height; }
	uint32 frameTime() { return _frameTime; }
	const uint16 *getFrameData(uint frameNumber) const { return _frames[frameNumber]; }

private:
	void decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
	void decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const;
};

} // End of namespace ZVision

#endif

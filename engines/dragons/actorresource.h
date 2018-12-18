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
#ifndef SCUMMVM_ACTORRESOURCE_H
#define SCUMMVM_ACTORRESOURCE_H

#include "common/system.h"

namespace Dragons {

class BigfileArchive;
class ActorResource;

struct ActorFrame {
	int16 field_0;
	int16 field_2;
	uint8 width;
	uint8 height;
	byte *frameDataOffset;
	uint16 flags;
	uint16 field_c;
};

class ActorResourceLoader {
private:
	BigfileArchive *_bigFileArchive;

public:
	ActorResourceLoader(BigfileArchive *bigFileArchive);
	ActorResource *load(uint32 resourceId);
};

class ActorResource {
private:
	byte *_data;
	ActorFrame *_frames;
	uint16 _framesCount;
	byte _palette[512];
	uint16 _sequenceTableOffset;
	uint16 _sequenceCount;

public:
	bool load(byte *dataStart, Common::SeekableReadStream &stream);
	Graphics::Surface *loadFrame(uint16 frameNumber);
	ActorFrame *getFrameHeader(uint16 frameNumber);
	byte *getSequenceData(int16 sequenceId);

private:
	void writePixelBlock(byte *pixels, byte *data);
};

} // End of namespace Dragons

#endif //SCUMMVM_ACTORRESOURCE_H

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
#ifndef DRAGONS_ACTORRESOURCE_H
#define DRAGONS_ACTORRESOURCE_H

#include "common/system.h"

namespace Dragons {

class BigfileArchive;
class ActorResource;

struct ActorFrame {
	int16 xOffset;
	int16 yOffset;
	uint16 width;
	uint16 height;
	byte *frameDataOffset;
	uint16 flags;
	uint16 field_c;
	int16 field_e;
	int16 field_10;
};

enum FrameFlags {
	FRAME_FLAG_FLIP_X = 0x800
};

class ActorResourceLoader {
private:
	BigfileArchive *_bigFileArchive;

public:
	ActorResourceLoader(BigfileArchive *bigFileArchive);
	ActorResource *load(uint32 resourceId);
};

class ActorResource {
public:
	uint32 _id;
private:
	byte *_data;
	int32 _fileSize;
	ActorFrame *_frames;
	uint16 _framesCount;
	byte _palette[512];
	uint16 _sequenceTableOffset;
	//uint16 _sequenceCount;

public:
	ActorResource();
	~ActorResource();
	bool load(uint32 id, byte *dataStart, Common::SeekableReadStream &stream);
	Graphics::Surface *loadFrame(ActorFrame &frameNumber, byte *palette);
	ActorFrame *loadFrameHeader(uint16 frameOffset);

	ActorFrame *getFrameHeader(uint16 frameNumber);
	byte *getSequenceData(int16 sequenceId);
	byte *getSequenceDataAtOffset(uint32 offset);
	const char *getFilename();
	byte *getPalette();

private:
};

} // End of namespace Dragons

#endif //DRAGONS_ACTORRESOURCE_H

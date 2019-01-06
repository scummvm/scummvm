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
#include "common/debug.h"
#include "common/memstream.h"
#include "graphics/surface.h"
#include "actorfiles.h"
#include "bigfile.h"
#include "actorresource.h"

namespace Dragons {


ActorResourceLoader::ActorResourceLoader(BigfileArchive *bigFileArchive) : _bigFileArchive(bigFileArchive) {

}

ActorResource *ActorResourceLoader::load(uint32 resourceId) {
	assert (resourceId < DRAGONS_NUM_ACTOR_FILES);
	ActorResource *actorResource = new ActorResource();
	const char *filename = actorResourceFiles[resourceId];
	uint32 size;
	byte *scrData = _bigFileArchive->load(filename, size);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(scrData, size, DisposeAfterUse::NO);

	debug("Loading '%s'", filename);
	actorResource->load(resourceId, scrData, *readStream);
	return actorResource;
}

bool ActorResource::load(uint32 id, byte *dataStart, Common::SeekableReadStream &stream) {
	_id = id;
	_data = dataStart;
	_fileSize = stream.size();
	stream.seek(0x6);
	_sequenceTableOffset = stream.readUint16LE();
	uint16 frameOffset = stream.readUint16LE();
	uint16 paletteOffset = stream.readUint16LE();

	stream.seek(paletteOffset);
	stream.read(_palette, 512);
	_palette[1] = 0x80; // set alpha (bit 15) on first palette entry.

	stream.seek(frameOffset);

	_framesCount = (paletteOffset - stream.readUint16LE()) / 0xe;

	debug("Frame Count: %d", _framesCount);

	_frames = new ActorFrame[_framesCount];
	for (int i=0; i < _framesCount; i++) {
		stream.seek(frameOffset + i * 2);

		uint16 offset = stream.readUint16LE();
		stream.seek(offset);
		_frames[i].xOffset = stream.readSint16LE();
		_frames[i].yOffset = stream.readSint16LE();
		_frames[i].width = stream.readByte() * 2; //FIXME the original checks actor->frame_flags bit 0 here at 0x80018438
		_frames[i].height = stream.readByte();
		uint32 frameDataOffset = stream.readUint32LE();
		_frames[i].frameDataOffset = &dataStart[frameDataOffset];
		_frames[i].flags = stream.readUint16LE();
		_frames[i].field_c = stream.readUint16LE();
//		debug("Frame[%d] field_0: %d field_2: %d (%d, %d) offset: %X, flags: %X field_c: %d",
//				i, _frames[i].field_0, _frames[i].field_2, _frames[i].width, _frames[i].height, frameDataOffset, _frames[i].flags, _frames[i].field_c);
	}

	return false;
}

void ActorResource::writePixelBlock(byte *pixels, byte *data) {
	pixels[0] = _palette[data[0] * 2];
	pixels[1] = _palette[data[0] * 2 + 1];
	pixels[2] = _palette[data[1] * 2];
	pixels[3] = _palette[data[1] * 2 + 1];
	pixels[4] = _palette[data[2] * 2];
	pixels[5] = _palette[data[2] * 2 + 1];
	pixels[6] = _palette[data[3] * 2];
	pixels[7] = _palette[data[3] *2 + 1];
}

Graphics::Surface *ActorResource::loadFrame(uint16 frameNumber) {
	assert (frameNumber < _framesCount);
	return loadFrame(_frames[frameNumber]);
}

Graphics::Surface *ActorResource::loadFrame(ActorFrame &actorFrame) {

	Graphics::Surface *surface = new Graphics::Surface();
	Graphics::PixelFormat pixelFormat16(2, 5, 5, 5, 1, 10, 5, 0, 15); //TODO move this to a better location.
	surface->create(actorFrame.width, actorFrame.height, pixelFormat16);

	byte *pixels = (byte *)surface->getPixels();

	int32 blockSize = ((actorFrame.width / 2) * actorFrame.height * 2) / 4;

	debug("Frame blockSize: %d width: %d height: %d", blockSize, actorFrame.width, actorFrame.height);

	byte *data = actorFrame.frameDataOffset;

	while(blockSize > 0) {
		int32 size = READ_BE_INT32(data);
		data += 4;
		if (size >= 0) {
			if (blockSize < size) {
				size = blockSize;
			}
			blockSize -= size;

			if (size != 0) {
				for(int32 i = size; i != 0; i--) {
					//TODO clean up this copy.
					writePixelBlock(pixels, data);

					data += 4;
					pixels += 8;
				}
			}
		} else {
			size = size & 0x7fffffff;
			if (blockSize < size) {
				size = blockSize;
			}
			blockSize -= size;
			if (size != 0) {
				for(int32 i = size; i != 0; i--) {
					//TODO write bytes to pixel data.
					writePixelBlock(pixels, data);
					pixels += 8;
				}
				data += 4;
			}
		}
	}

	return surface;
}

ActorFrame *ActorResource::getFrameHeader(uint16 frameNumber) {
	assert (frameNumber < _framesCount);

	return &_frames[frameNumber];
}

byte *ActorResource::getSequenceData(int16 sequenceId)
{
	uint16 offset = READ_LE_UINT16(_data + _sequenceTableOffset + (sequenceId * 2));
	return &_data[offset];
}

ActorFrame *ActorResource::loadFrameHeader(uint16 frameOffset) {
	Common::SeekableReadStream *stream = new Common::MemoryReadStream(_data + frameOffset, sizeof(ActorFrame), DisposeAfterUse::NO);

	ActorFrame *frame = new ActorFrame;
	frame->xOffset = stream->readSint16LE();
	frame->yOffset = stream->readSint16LE();
	frame->width = stream->readByte() * 2; //FIXME the original checks actor->frame_flags bit 0 here at 0x80018438
	frame->height = stream->readByte();
	uint32 frameDataOffset = stream->readUint32LE();
	frame->frameDataOffset = &_data[frameDataOffset];
	frame->flags = stream->readUint16LE();
	frame->field_c = stream->readUint16LE();

	delete stream;
	return frame;
}

const char *ActorResource::getFilename() {
	return actorResourceFiles[_id];
}

byte *ActorResource::getSequenceDataAtOffset(uint32 offset) {
	assert(offset < _fileSize);
	return &_data[offset];
}

} // End of namespace Dragons

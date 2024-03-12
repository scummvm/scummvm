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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"
#include "common/substream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/video.h"
#include "engines/nancy/decompress.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

namespace Nancy {

class VideoCacheLoader : public DeferredLoader {
public:
	VideoCacheLoader(AVFDecoder::AVFVideoTrack &owner) : _owner(owner) {}
	virtual ~VideoCacheLoader() {}

private:
	bool loadInner() override;

	AVFDecoder::AVFVideoTrack &_owner;
};

bool VideoCacheLoader::loadInner() {
	AVFDecoder::CacheHint hint = _owner._cacheHint;
	int frameID = _owner._curFrame;
	int frameCount = _owner._frameCount;

	for (int i = 0; i < frameCount; ++i) {
		if (frameID < 0) {
			frameID += frameCount;
		}

		if (frameID >= frameCount) {
			frameID -= frameCount;
		}

		if (!_owner._frameCache[frameID].getPixels()) {
			_owner.decodeFrame(frameID);
			return false;
		}

		// Select next frame based on hint and play direction
		if (hint != AVFDecoder::kLoadBidirectional) {
			frameID = _owner._reversed ? frameID - 1 : frameID + 1;
		} else {
			frameID = _owner._curFrame + (i % 2 ? i >> 1 : -(i >> 1));
		}
	}

	return true;
}

AVFDecoder::~AVFDecoder() {
	close();
}

bool AVFDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	char id[15];
	stream->read(id, 15);
	id[14] = 0;
	Common::String idString = id;

	bool earlyHeaderFormat = false;

	if (idString == "AVF WayneSikes") {
		stream->skip(1); // Unknown
	} else if (idString.hasPrefix("ALG")) {
		earlyHeaderFormat = true;
		stream->seek(10, SEEK_SET);
	}

	uint32 chunkFileFormat;
	chunkFileFormat = stream->readUint16LE() << 16;
	chunkFileFormat |= stream->readUint16LE();

	if (chunkFileFormat != 0x00020000 && chunkFileFormat != 0x00010000) {
		warning("Unsupported version %d.%d found in AVF", chunkFileFormat >> 16, chunkFileFormat & 0xffff);
		return false;
	}

	if (!earlyHeaderFormat) {
		stream->skip(1); // Unknown
	}

	addTrack(new AVFVideoTrack(stream, chunkFileFormat, _cacheHint));

	return true;
}

const Graphics::Surface *AVFDecoder::decodeFrame(uint frameNr) {
	return ((AVFDecoder::AVFVideoTrack *)getTrack(0))->decodeFrame(frameNr);
}

void AVFDecoder::addFrameTime(const uint16 timeToAdd) {
	((AVFDecoder::AVFVideoTrack *)getTrack(0))->_frameTime += timeToAdd;
}

// Custom function to allow the last frame of the video to play correctly
bool AVFDecoder::atEnd() const {
	const AVFDecoder::AVFVideoTrack *track = ((const AVFDecoder::AVFVideoTrack *)getTrack(0));
	if (!track) {
		return true;
	}
	return !track->isReversed() && track->endOfTrack() && track->getFrameTime(track->getFrameCount()) <= getTime();
}

AVFDecoder::AVFVideoTrack::AVFVideoTrack(Common::SeekableReadStream *stream, uint32 chunkFileFormat, CacheHint cacheHint) {
	assert(stream);
	_fileStream = stream;
	_curFrame = -1;
	_reversed = false;
	_dec = new Decompressor;

	_frameCount = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_depth = stream->readByte();
	_frameTime = stream->readUint32LE();

	byte comp = stream->readByte();
	_compressed = comp == 2;

	uint formatHi = chunkFileFormat >> 16;

	if (formatHi == 1) {
		stream->skip(1);
	}

	if (comp != 1 && comp != 2)
		error("Unknown compression type %d found in AVF", comp);

	_pixelFormat = g_nancy->_graphics->getInputPixelFormat();
	_frameSize = _width * _height * _pixelFormat.bytesPerPixel;

	_chunkInfo.reserve(_frameCount);
	for (uint i = 0; i < _frameCount; i++) {
		ChunkInfo info;

		if (formatHi == 1) {
			char buf[9];
			stream->read(buf, 9);
			buf[8] = '\0';
			info.name = buf;
			info.index = stream->readUint32LE();

			stream->skip(4); // unknown

			info.offset = stream->readUint32LE();
			info.compressedSize = stream->readUint32LE();
			info.size = _frameSize;
			info.type = 0;
		} else if (formatHi == 2) {
			info.index = stream->readUint16LE();
			info.offset = stream->readUint32LE();
			info.compressedSize = stream->readUint32LE();
			info.size = stream->readUint32LE();
			info.type = stream->readByte();
			stream->skip(4); // Unknown;
		}

		_chunkInfo.push_back(info);
	}

	_frameCache.resize(_frameCount);
	_cacheHint = cacheHint;
	_loaderPtr.reset(new VideoCacheLoader(*this));
	auto castedPtr = _loaderPtr.dynamicCast<DeferredLoader>();
	g_nancy->addDeferredLoader(castedPtr);
}

AVFDecoder::AVFVideoTrack::~AVFVideoTrack() {
	delete _fileStream;
	delete _dec;

	for (Graphics::Surface &surf : _frameCache) {
		surf.free();
	}
}

bool AVFDecoder::AVFVideoTrack::seek(const Audio::Timestamp &time) {
	_curFrame = getFrameAtTime(time);

	// Offset by 1 to ensure decodeNextFrame() actually decodes the frame we want
	if (!_reversed) {
		--_curFrame;
	} else {
		++_curFrame;
	}

	return true;
}

bool AVFDecoder::AVFVideoTrack::setReverse(bool reverse) {
	_reversed = reverse;
	return true;
}

bool AVFDecoder::AVFVideoTrack::endOfTrack() const {
	if (_reversed)
		return _curFrame <= 0;

	return _curFrame >= getFrameCount();
}

bool AVFDecoder::AVFVideoTrack::decode(byte *outBuf, uint32 frameSize, Common::ReadStream &inBuf) const {
	byte cmd = inBuf.readByte();
	while (!inBuf.eos()) {
		uint32 len, offset;
		switch (cmd) {
		case 0x20:
			// Write literal block
			offset = inBuf.readUint32LE() * 2;
			len = inBuf.readUint32LE() * 2;
			if (offset + len > frameSize)
				return false;
			inBuf.read(outBuf + offset, len);
			break;
		case 0x40: {
			// Write literal value 'n' times
			uint16 val = inBuf.readUint16LE();
			offset = inBuf.readUint32LE() * 2;
			len = inBuf.readUint32LE() * 2;
			if (offset + len > frameSize)
				return false;
			for (uint i = 0; i < len; i += 2)
				WRITE_LE_UINT16(outBuf + offset + i, val);
			break;
		}
		case 0x80: {
			// Write literal block 'n' times
			len = inBuf.readByte() * 2;
			uint32 offsetCount = inBuf.readUint32LE();
			byte buf[510];

			inBuf.read(buf, len);
			for (uint i = 0; i < offsetCount; ++i) {
				offset = inBuf.readUint32LE() * 2;
				if (offset + len > frameSize)
					return false;
				memcpy(outBuf + offset, buf, len);
			}
			break;
		}
		default:
			break;
		}
		cmd = inBuf.readByte();
	}
	return true;
}

const Graphics::Surface *AVFDecoder::AVFVideoTrack::decodeFrame(uint frameNr) {
	if (frameNr < _frameCache.size() && _frameCache[frameNr].getPixels()) {
		// Frame is cached, return a pointer to it
		return &_frameCache[frameNr];
	}

	if (frameNr >= _chunkInfo.size()) {
		debugC(kDebugVideo, "Frame %d doesn't exist, returning last frame %d", frameNr, _chunkInfo.size() - 1);
		return decodeFrame(_chunkInfo.size() - 1);
	}

	const ChunkInfo &info = _chunkInfo[frameNr];

	if (!info.size && !info.compressedSize) {
		if (info.type != 2) {
			warning("Found empty frame %d of type %d", frameNr, info.type);
			return nullptr;
		}

		// Type 2 empty frames are valid. We recursively call decodeFrame until
		// we find a valid previous frame, or arrive at the beginning of the video
		if (frameNr != 0) {
			return decodeFrame(frameNr - 1);
		} else {
			return nullptr;
		}
	}

	Graphics::Surface &frameInCache = _frameCache[frameNr];
	frameInCache.create(_width, _height, _pixelFormat);

	byte *decompBuf = nullptr;
	if (info.type == 0) {
		// For type 0 we decompress straight to the surface, make sure we don't go out of bounds
		if (info.size > _frameSize) {
			warning("Decompressed size %d exceeds frame size %d", info.size, _frameSize);
			return nullptr;
		}

		decompBuf = (byte *)frameInCache.getPixels();
	} else {
		// For types 1 and 2, we decompress to a temp buffer for decoding
		decompBuf = new byte[info.size];
	}

	Common::SeekableSubReadStream input(_fileStream, info.offset, info.offset + info.compressedSize);

	if (_compressed) {
		Common::MemoryWriteStream output(decompBuf, info.size);

		if (!_dec->decompress(input, output)) {
			warning("Failed to decompress frame %d", frameNr);
			// Make sure we don't delete data we don't own
			if (info.type != 0) {
				delete[] decompBuf;
			}

			return nullptr;
		}
	} else {
		// No compression, just copy the data
		input.read(decompBuf, info.size);
	}

	if (info.type != 0) {
		if (info.type == 2 && frameNr != 0) {
			// Type 2 frames are incomplete, and only contain the pixels
			// that are different from the last valid frame. Thus, we need
			// to decode the previous frame and copy its contents to the new one's
			const Graphics::Surface *refFrame = decodeFrame(frameNr - 1);
			if (refFrame) {
				Graphics::copyBlit((byte *)frameInCache.getPixels(), (const byte *)refFrame->getPixels(),
					frameInCache.pitch, refFrame->pitch, frameInCache.w, frameInCache.h, frameInCache.format.bytesPerPixel);

#ifdef SCUMM_BIG_ENDIAN
				// Convert from BE back to LE so the decode step below works correctly
				byte *buf = (byte *)frameInCache.getPixels();
				if (g_nancy->_graphics->getInputPixelFormat().bytesPerPixel == 2) {
					for (int i = 0; i < frameInCache.pitch * frameInCache.h / 2; ++i) {
						((uint16 *)buf)[i] = SWAP_BYTES_16(((uint16 *)buf)[i]);
					}
				}
#endif
			}
		}

		Common::MemoryReadStream decompStr(decompBuf, info.size);
		decode((byte *)frameInCache.getPixels(), _frameSize, decompStr);
	}

	if (info.type != 0) {
		delete[] decompBuf;
	}

#ifdef SCUMM_BIG_ENDIAN
	byte *buf = (byte *)frameInCache.getPixels();
	if (g_nancy->_graphics->getInputPixelFormat().bytesPerPixel == 2) {
		for (int i = 0; i < frameInCache.pitch * frameInCache.h / 2; ++i) {
			((uint16 *)buf)[i] = SWAP_BYTES_16(((uint16 *)buf)[i]);
		}
	}
#endif

	return &frameInCache;
}

const Graphics::Surface *AVFDecoder::AVFVideoTrack::decodeNextFrame() {
	return decodeFrame(_reversed ? --_curFrame : ++_curFrame);
}

} // End of namespace Nancy

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

#include <common/stream.h>
#include "cel_decoder.h"

namespace Pink {

bool CelDecoder::loadStream(Common::SeekableReadStream *stream) {
    close();

    /* uint32 frameSize = */ stream->readUint32LE();
    uint16 frameType = stream->readUint16LE();

    // Check FLC magic number
    if (frameType != 0xAF12) {
        warning("FlicDecoder::loadStream(): attempted to load non-FLC data (type = 0x%04X)", frameType);
        return false;
    }

    uint16 frameCount = stream->readUint16LE();
    uint16 width = stream->readUint16LE();
    uint16 height = stream->readUint16LE();
    uint16 colorDepth = stream->readUint16LE();
    if (colorDepth != 8) {
        warning("FlicDecoder::loadStream(): attempted to load an FLC with a palette of color depth %d. Only 8-bit color palettes are supported", colorDepth);
        return false;
    }

    addTrack(new CelVideoTrack(stream, frameCount, width, height));
    return true;
}

uint32 CelDecoder::getX(){
    CelVideoTrack *track = (CelVideoTrack*) getTrack(0);
    if (!track)
        return -1;
    return track->getX();
}

uint32 CelDecoder::getY() {
    CelVideoTrack *track = (CelVideoTrack*) getTrack(0);
    if (!track)
        return -1;
    return track->getY();
}


uint16 CelDecoder::getTransparentColourIndex() {
    CelVideoTrack *track = (CelVideoTrack*) getTrack(0);
    if (!track)
        return 0;
    return track->getTransparentColourIndex();
}

const Graphics::Surface *CelDecoder::getCurrentFrame() {
    CelVideoTrack *track = (CelVideoTrack*) getTrack(0);
    if (!track)
        return 0;
    return track->getCurrentFrame();
}

CelDecoder::CelVideoTrack::CelVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, bool skipHeader)
        : FlicVideoTrack(stream, frameCount, width, height, 1), _center(0,0), _transparentColourIndex(0){
    readHeader();
}

#define PREFIX_TYPE 0xF100
#define CEL_DATA 3

void CelDecoder::CelVideoTrack::readPrefixChunk() {
    _fileStream->seek(0x80);
    uint32 chunkSize = _fileStream->readUint32LE();
    uint16 chunkType = _fileStream->readUint16LE();
    if (chunkType != PREFIX_TYPE)
        return;
    uint32 offset = 6;

    uint32 subchunkSize = _fileStream->readUint32LE();
    uint16 subchunkType = _fileStream->readUint16LE();

    switch (subchunkType) {
        case CEL_DATA:
            debug("%u", _fileStream->readUint16LE());
            _center.x = _fileStream->readUint16LE();
            _center.y = _fileStream->readUint16LE();
            debug("stretch x: %u", _fileStream->readUint16LE());
            debug("stretch y: %u", _fileStream->readUint16LE());
            debug("rotation x: %u", _fileStream->readUint16LE());
            debug("rotation y: %u", _fileStream->readUint16LE());
            debug("rotation z: %u", _fileStream->readUint16LE());
            debug("current Frame: %u", _fileStream->readUint16LE());
            debug("next frame offset: %u",_fileStream->readUint32LE());
            debug("tcolor: %u", _transparentColourIndex = _fileStream->readUint16LE());
            for (int j = 0; j < 18; ++j) {
                debug("%u", _fileStream->readUint16LE());
            }
            break;
        default:
            error("Unknown subchunk type");
            _fileStream->skip(subchunkSize - 6);
            break;
    }

}

void CelDecoder::CelVideoTrack::readHeader() {
    _fileStream->readUint16LE();	// flags
    // Note: The normal delay is a 32-bit integer (dword), whereas the overridden delay is a 16-bit integer (word)
    // the frame delay is the FLIC "speed", in milliseconds.
    _frameDelay = _startFrameDelay = _fileStream->readUint32LE();

    _fileStream->seek(80);
    _offsetFrame1 = _fileStream->readUint32LE();
    _offsetFrame2 = _fileStream->readUint32LE();

    if (_offsetFrame1 > 0x80) {
        readPrefixChunk();
    }

    // Seek to the first frame
    _fileStream->seek(_offsetFrame1);
}

uint32 CelDecoder::CelVideoTrack::getX() const {
    return _center.x - getWidth() / 2;
}

uint32 CelDecoder::CelVideoTrack::getY() const {
    return _center.y - getHeight() / 2;
}

uint16 CelDecoder::CelVideoTrack::getTransparentColourIndex() {
    return _transparentColourIndex;
}

const Graphics::Surface *CelDecoder::CelVideoTrack::getCurrentFrame() {
    return _surface;
}

} // End of namepsace Pink
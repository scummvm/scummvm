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
 *
 */

#include "common/stream.h"
#include "audio/audiostream.h"

#include "engines/zvision/zork_avi_decoder.h"
#include "engines/zvision/zork_raw.h"

namespace ZVision {

// Redefinitions from avi_decoder.cpp
#define ID_VIDS MKTAG('v','i','d','s')
#define ID_TXTS MKTAG('t','x','t','s')
#define ID_STRF MKTAG('s','t','r','f')
#define ID_AUDS MKTAG('a','u','d','s')
#define ID_MIDS MKTAG('m','i','d','s')

// Copied off AVIDecoder::handleStreamHeader()
void ZorkAVIDecoder::handleStreamHeader() {
	AVIStreamHeader sHeader;
	sHeader.size = _fileStream->readUint32LE();
	sHeader.streamType = _fileStream->readUint32BE();

	if (sHeader.streamType == ID_MIDS || sHeader.streamType == ID_TXTS)
		error("Unhandled MIDI/Text stream");

	sHeader.streamHandler = _fileStream->readUint32BE();
	sHeader.flags = _fileStream->readUint32LE();
	sHeader.priority = _fileStream->readUint16LE();
	sHeader.language = _fileStream->readUint16LE();
	sHeader.initialFrames = _fileStream->readUint32LE();
	sHeader.scale = _fileStream->readUint32LE();
	sHeader.rate = _fileStream->readUint32LE();
	sHeader.start = _fileStream->readUint32LE();
	sHeader.length = _fileStream->readUint32LE();
	sHeader.bufferSize = _fileStream->readUint32LE();
	sHeader.quality = _fileStream->readUint32LE();
	sHeader.sampleSize = _fileStream->readUint32LE();

	_fileStream->skip(sHeader.size - 48); // Skip over the remainder of the chunk (frame)

	if (_fileStream->readUint32BE() != ID_STRF)
		error("Could not find STRF tag");

	uint32 strfSize = _fileStream->readUint32LE();
	uint32 startPos = _fileStream->pos();

	if (sHeader.streamType == ID_VIDS) {
		BitmapInfoHeader bmInfo;
		bmInfo.size = _fileStream->readUint32LE();
		bmInfo.width = _fileStream->readUint32LE();
		bmInfo.height = _fileStream->readUint32LE();
		bmInfo.planes = _fileStream->readUint16LE();
		bmInfo.bitCount = _fileStream->readUint16LE();
		bmInfo.compression = _fileStream->readUint32BE();
		bmInfo.sizeImage = _fileStream->readUint32LE();
		bmInfo.xPelsPerMeter = _fileStream->readUint32LE();
		bmInfo.yPelsPerMeter = _fileStream->readUint32LE();
		bmInfo.clrUsed = _fileStream->readUint32LE();
		bmInfo.clrImportant = _fileStream->readUint32LE();

		if (bmInfo.clrUsed == 0)
			bmInfo.clrUsed = 256;

		if (sHeader.streamHandler == 0)
			sHeader.streamHandler = bmInfo.compression;

		AVIVideoTrack *track = new AVIVideoTrack(_header.totalFrames, sHeader, bmInfo);

		if (bmInfo.bitCount == 8) {
			byte *palette = const_cast<byte *>(track->getPalette());
			for (uint32 i = 0; i < bmInfo.clrUsed; i++) {
				palette[i * 3 + 2] = _fileStream->readByte();
				palette[i * 3 + 1] = _fileStream->readByte();
				palette[i * 3] = _fileStream->readByte();
				_fileStream->readByte();
			}

			track->markPaletteDirty();
		}

		addTrack(track);
	} else if (sHeader.streamType == ID_AUDS) {
		PCMWaveFormat wvInfo;
		wvInfo.tag = _fileStream->readUint16LE();
		wvInfo.channels = _fileStream->readUint16LE();
		wvInfo.samplesPerSec = _fileStream->readUint32LE();
		wvInfo.avgBytesPerSec = _fileStream->readUint32LE();
		wvInfo.blockAlign = _fileStream->readUint16LE();
		wvInfo.size = _fileStream->readUint16LE();

		// AVI seems to treat the sampleSize as including the second
		// channel as well, so divide for our sake.
		if (wvInfo.channels == 2)
			sHeader.sampleSize /= 2;

		addTrack(new ZorkAVIAudioTrack(sHeader, wvInfo, _soundType));
	}

	// Ensure that we're at the end of the chunk
	_fileStream->seek(startPos + strfSize);
}

void ZorkAVIDecoder::ZorkAVIAudioTrack::queueSound(Common::SeekableReadStream *stream) {
	if (_audStream) {
		if (_wvInfo.tag == kWaveFormatZorkPCM) {
			assert(_wvInfo.size == 8);
			_audStream->queueAudioStream(makeRawZorkStream(stream, _wvInfo.samplesPerSec, DisposeAfterUse::YES), DisposeAfterUse::YES);
		}
	} else {
		delete stream;
	}
}

} // End of namespace ZVision

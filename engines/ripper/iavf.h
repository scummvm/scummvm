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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_IAVF_H
#define RIPPER_IAVF_H

#include "common/array.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Ripper {

struct IavfSegment {
	Common::Array<byte> setup;
	Common::Array<uint32> frameSizes;
	Common::Array<Common::Array<byte> > framePayloads;
	Common::Array<uint32> frameAudioOffsets;
	uint32 expectedFrames;
	int x;
	int y;
	bool clearDisplayBefore;

	IavfSegment() : expectedFrames(0), x(-1), y(-1), clearDisplayBefore(false) {}
};

struct IavfMovie {
	uint32 declaredGateCount;
	uint16 sampleRate;
	byte channels;
	byte bitsPerSample;
	uint32 audioByteRate;
	Common::Array<byte> audio;
	uint32 audioPayloadBytes;
	Common::Array<IavfSegment> segments;
	uint presentationWidth;
	uint presentationHeight;
	bool clearDisplayAfter;

	IavfMovie() : declaredGateCount(0), sampleRate(0), channels(0), bitsPerSample(0),
		audioByteRate(0), audioPayloadBytes(0), presentationWidth(0),
		presentationHeight(0), clearDisplayAfter(false) {}
};

bool parseIavf(Common::SeekableReadStream &stream, const Common::String &name,
	IavfMovie &movie);
Common::SeekableReadStream *rebuildSmackerStream(const IavfSegment &segment);

} // End of namespace Ripper

#endif // RIPPER_IAVF_H

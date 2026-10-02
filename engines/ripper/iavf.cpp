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

#include "ripper/iavf.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"

#include "ripper/detection.h"

namespace Ripper {

namespace {

struct IavfDescriptor {
	uint16 opcode;
	uint32 arg0;
	uint32 arg1;
	uint32 arg2;
};

static bool readExact(Common::SeekableReadStream &stream, void *data, uint32 size) {
	return stream.read(data, size) == size;
}

static bool readBlob(Common::SeekableReadStream &stream, uint32 size,
		Common::Array<byte> &data) {
	if ((uint64)stream.pos() + size > (uint64)stream.size())
		return false;
	data.resize(size);
	return size == 0 || readExact(stream, data.data(), size);
}

static bool validateSmackerSetup(const Common::Array<byte> &setup, uint32 &frameCount) {
	if (setup.size() < 104 || READ_BE_UINT32(setup.data()) != MKTAG('S', 'M', 'K', '2'))
		return false;
	frameCount = READ_LE_UINT32(setup.data() + 12);
	const uint32 flags = READ_LE_UINT32(setup.data() + 20);
	const uint32 treesSize = READ_LE_UINT32(setup.data() + 52);
	if ((flags & 1) != 0)
		return false;
	return (uint64)104 + frameCount + treesSize == setup.size();
}

static bool readDescriptor(Common::SeekableReadStream &stream, IavfDescriptor &descriptor) {
	if ((uint64)stream.pos() + 14 > (uint64)stream.size())
		return false;
	descriptor.opcode = stream.readUint16LE();
	descriptor.arg0 = stream.readUint32LE();
	descriptor.arg1 = stream.readUint32LE();
	descriptor.arg2 = stream.readUint32LE();
	return true;
}

} // End of anonymous namespace

static bool parseIavfInternal(Common::SeekableReadStream &stream, const Common::String &name,
		IavfMovie &movie) {
	if (stream.size() < 0x91 || (uint64)stream.size() > 0xffffffffULL)
		return false;

	Common::Array<byte> header;
	if (!readBlob(stream, 0x91, header) || memcmp(header.data(), "IAVF2.00", 8) != 0)
		return false;
	movie.declaredGateCount = READ_LE_UINT32(header.data() + 0x10);
	movie.sampleRate = READ_LE_UINT16(header.data() + 0x1c);
	movie.channels = header[0x1e];
	movie.bitsPerSample = header[0x1f];
	const uint32 headerByteRate = READ_LE_UINT32(header.data() + 0x20);
	const uint32 headerBlockAlign = READ_LE_UINT32(header.data() + 0x24);
	// Shipped IAVF assets store height before width.
	movie.presentationHeight = READ_LE_UINT16(header.data() + 0x2f);
	movie.presentationWidth = READ_LE_UINT16(header.data() + 0x31);
	if (movie.channels != 1 || movie.bitsPerSample != 16 || movie.sampleRate == 0) {
		warning("Ripper: unsupported IAVF audio format rate=%u channels=%u bits=%u in '%s'",
			movie.sampleRate, movie.channels, movie.bitsPerSample, name.c_str());
		return false;
	}
	const uint32 expectedBlockAlign = movie.channels * movie.bitsPerSample / 8;
	movie.audioByteRate = movie.sampleRate * expectedBlockAlign;
	if (headerBlockAlign != expectedBlockAlign || headerByteRate != movie.audioByteRate) {
		warning("Ripper: inconsistent IAVF audio header rate=%u byteRate=%u blockAlign=%u in '%s'",
			movie.sampleRate, headerByteRate, headerBlockAlign, name.c_str());
	}
	if (movie.presentationWidth == 0 || movie.presentationHeight == 0)
		return false;
	movie.audio.reserve((uint32)stream.size());

	Common::HashMap<uint32, Common::Array<byte> > setupCache;
	Common::HashMap<uint32, uint32> audioEndOffsets;
	IavfSegment *activeSegment = nullptr;
	Common::Array<byte> pendingFramePayload;
	bool reachedEnd = false;
	bool hasPendingAudioOffset = false;
	bool hasPendingFrame = false;
	bool pendingDisplayClear = false;
	uint32 descriptorIndex = 0;
	uint32 observedGateCount = 0;
	uint32 pendingAudioOffset = 0;
	uint32 pendingFrameAudioOffset = 0;
	while (stream.pos() < stream.size() && !reachedEnd) {
		IavfDescriptor descriptor;
		if (!readDescriptor(stream, descriptor))
			return false;
		debugC(11, kDebugVideo,
			"Ripper: IAVF '%s' descriptor=%u opcode=0x%02x arg0=%u arg1=%u arg2=%u offset=%lld",
			name.c_str(), descriptorIndex++, descriptor.opcode, descriptor.arg0, descriptor.arg1,
			descriptor.arg2, stream.pos() - 14);

		switch (descriptor.opcode) {
		case 0x66: {
			if (descriptor.arg2 > descriptor.arg1 ||
				(uint64)movie.audio.size() + descriptor.arg1 > 0xffffffffULL ||
				(uint64)movie.audioPayloadBytes + descriptor.arg2 > 0xffffffffULL)
				return false;
			const uint32 outputOffset = movie.audio.size();
			movie.audio.resize(outputOffset + descriptor.arg1);
			if (descriptor.arg2 != 0 &&
					!readExact(stream, movie.audio.data() + outputOffset, descriptor.arg2))
				return false;
			if (descriptor.arg1 > descriptor.arg2) {
				memset(movie.audio.data() + outputOffset + descriptor.arg2, 0,
					descriptor.arg1 - descriptor.arg2);
				debugC(11, kDebugVideo,
					"Ripper: IAVF '%s' audio tag=%u payload=%u timeline=%u silence=%u",
					name.c_str(), descriptor.arg0, descriptor.arg2, descriptor.arg1,
					descriptor.arg1 - descriptor.arg2);
			}
			movie.audioPayloadBytes += descriptor.arg2;
			audioEndOffsets[descriptor.arg0] = movie.audio.size();
			break;
		}

		case 0x67: {
			Common::HashMap<uint32, uint32>::const_iterator audioOffset =
				audioEndOffsets.find(descriptor.arg0);
			if (audioOffset == audioEndOffsets.end() || audioOffset->_value != descriptor.arg1)
				return false;
			pendingAudioOffset = audioOffset->_value;
			hasPendingAudioOffset = true;
			++observedGateCount;
			break;
		}

		case 0x68:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0)
				return false;
			pendingDisplayClear = true;
			break;

		case 0x75:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0)
				return false;
			// RunPacketizedMediaPlaybackCore at 0x5b592 prebuffers the packet
			// stream here. playIavf reconstructs every parsed branch before it
			// starts the shared audio timeline.
			debugC(2, kDebugVideo, "Ripper: IAVF '%s' reached prebuffer command 0x75", name.c_str());
			break;

		case 0x78: {
			if (descriptor.arg0 == 0 || descriptor.arg1 == 0 || descriptor.arg2 != 0)
				return false;
			Common::Array<byte> setup;
			uint32 frameCount = 0;
			if (!readBlob(stream, descriptor.arg1, setup) ||
					!validateSmackerSetup(setup, frameCount))
				return false;
			setupCache[descriptor.arg0] = Common::move(setup);
			break;
		}

		case 0x6a: {
			if (hasPendingFrame ||
				(activeSegment && activeSegment->frameSizes.size() != activeSegment->expectedFrames))
				return false;
			int32 branch[5];
			for (uint i = 0; i < ARRAYSIZE(branch); ++i)
				branch[i] = stream.readSint32LE();
			if (stream.err())
				return false;

			IavfSegment segment;
			segment.x = branch[0];
			segment.y = branch[1];
			segment.clearDisplayBefore = pendingDisplayClear;
			pendingDisplayClear = false;
			if (descriptor.arg1 == 0) {
				if (!readBlob(stream, descriptor.arg0, segment.setup))
					return false;
			} else {
				Common::HashMap<uint32, Common::Array<byte> >::const_iterator cached =
					setupCache.find(descriptor.arg1);
				if (cached == setupCache.end() || cached->_value.size() != descriptor.arg0)
					return false;
				segment.setup = cached->_value;
			}
			if (!validateSmackerSetup(segment.setup, segment.expectedFrames))
				return false;
			movie.segments.push_back(Common::move(segment));
			activeSegment = &movie.segments.back();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' segment=%u frames=%u x=%d y=%d setup=%u",
				name.c_str(), movie.segments.size() - 1, activeSegment->expectedFrames,
				activeSegment->x, activeSegment->y, activeSegment->setup.size());
			break;
		}

		case 0x6c:
			if (!activeSegment || descriptor.arg1 != 0 || descriptor.arg2 != 0 ||
				activeSegment->frameSizes.size() >= activeSegment->expectedFrames ||
				!hasPendingAudioOffset || hasPendingFrame)
				return false;
			if (!readBlob(stream, descriptor.arg0, pendingFramePayload))
				return false;
			pendingFrameAudioOffset = pendingAudioOffset;
			hasPendingAudioOffset = false;
			hasPendingFrame = true;
			break;

		case 0x77:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0 ||
				!activeSegment || !hasPendingFrame ||
				activeSegment->frameSizes.size() >= activeSegment->expectedFrames)
				return false;
			// LoadCustomPacketPaletteStateBlock at 0x6c430 consumes opcode 0x6c,
			// while 0x77 reaches RenderCustomPacketFrameAndOverlays at 0x6c486.
			activeSegment->frameSizes.push_back(pendingFramePayload.size());
			activeSegment->framePayloads.push_back(Common::move(pendingFramePayload));
			activeSegment->frameAudioOffsets.push_back(pendingFrameAudioOffset);
			hasPendingFrame = false;
			break;

		case 0x70:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0 ||
				hasPendingFrame)
				return false;
			movie.clearDisplayAfter = pendingDisplayClear;
			reachedEnd = true;
			break;

		default:
			warning("Ripper: unsupported IAVF opcode 0x%02x at 0x%llx in '%s'",
				descriptor.opcode, stream.pos() - 14, name.c_str());
			return false;
		}
	}

	if (!reachedEnd || movie.segments.empty())
		return false;
	if (observedGateCount != movie.declaredGateCount) {
		warning("Ripper: IAVF '%s' declares %u playback gates but contains %u",
			name.c_str(), movie.declaredGateCount, observedGateCount);
	}
	for (uint i = 0; i < movie.segments.size(); ++i) {
		if (movie.segments[i].frameSizes.size() != movie.segments[i].expectedFrames ||
			movie.segments[i].frameAudioOffsets.size() != movie.segments[i].expectedFrames)
			return false;
		for (uint frame = 0; frame < movie.segments[i].frameAudioOffsets.size(); ++frame) {
			if (movie.segments[i].frameAudioOffsets[frame] > movie.audio.size())
				return false;
		}
	}
	debugC(1, kDebugVideo,
		"Ripper: parsed IAVF '%s' canvas=%ux%u segments=%u gates=%u audioPayloadBytes=%u audioTimelineBytes=%u audioMs=%u",
		name.c_str(), movie.presentationWidth, movie.presentationHeight,
		movie.segments.size(), observedGateCount, movie.audioPayloadBytes, movie.audio.size(),
		(uint32)((uint64)movie.audio.size() * 1000 / movie.audioByteRate));
	return true;
}

bool parseIavf(Common::SeekableReadStream &stream, const Common::String &name,
		IavfMovie &movie) {
	IavfMovie parsedMovie;
	if (!parseIavfInternal(stream, name, parsedMovie))
		return false;
	movie = Common::move(parsedMovie);
	return true;
}

Common::SeekableReadStream *rebuildSmackerStream(const IavfSegment &segment) {
	uint32 setupFrameCount = 0;
	if (!validateSmackerSetup(segment.setup, setupFrameCount) ||
			setupFrameCount != segment.expectedFrames ||
			segment.frameSizes.size() != segment.expectedFrames ||
			segment.framePayloads.size() != segment.expectedFrames ||
			segment.frameAudioOffsets.size() != segment.expectedFrames)
		return nullptr;
	for (uint i = 0; i < segment.frameSizes.size(); ++i) {
		if (segment.frameSizes[i] != segment.framePayloads[i].size())
			return nullptr;
	}

	uint64 totalSize = 104 + (uint64)segment.frameSizes.size() * 4 +
		segment.setup.size() - 104;
	for (uint i = 0; i < segment.framePayloads.size(); ++i)
		totalSize += segment.framePayloads[i].size();
	if (totalSize > 0xffffffffULL)
		return nullptr;

	// MemoryReadStream disposes owned buffers with free().
	byte *output = (byte *)malloc((uint32)totalSize);
	if (!output)
		return nullptr;
	uint32 cursor = 0;
	memcpy(output + cursor, segment.setup.data(), 104);
	cursor += 104;
	for (uint i = 0; i < segment.frameSizes.size(); ++i) {
		WRITE_LE_UINT32(output + cursor, segment.frameSizes[i]);
		cursor += 4;
	}
	memcpy(output + cursor, segment.setup.data() + 104, segment.setup.size() - 104);
	cursor += segment.setup.size() - 104;
	for (uint i = 0; i < segment.framePayloads.size(); ++i) {
		memcpy(output + cursor, segment.framePayloads[i].data(), segment.framePayloads[i].size());
		cursor += segment.framePayloads[i].size();
	}
	return new Common::MemoryReadStream(output, cursor, DisposeAfterUse::YES);
}

} // End of namespace Ripper

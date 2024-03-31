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

#include "common/memstream.h"
#include "common/stream.h"
#include "common/substream.h"

#include "image/ani.h"

namespace Image {

AniDecoder::Metadata::Metadata()
	: numFrames(0), numSteps(0), width(0), height(0), bitCount(0),
	  numPlanes(0), perFrameDelay(0), haveSeqData(false), isCURFormat(false) {
}

AniDecoder::FrameDef::FrameDef() : delay(0), imageIndex(0) {
}

AniDecoder::AniDecoder() : _stream(nullptr), _disposeAfterUse(DisposeAfterUse::NO) {
}

AniDecoder::~AniDecoder() {
	close();
}

void AniDecoder::close() {
	if (_disposeAfterUse == DisposeAfterUse::YES && _stream != nullptr)
		delete _stream;

	_stream = nullptr;
}

const AniDecoder::Metadata &AniDecoder::getMetadata() const {
	return _metadata;
}

AniDecoder::FrameDef AniDecoder::getSequenceFrame(uint sequenceIndex) const {
	FrameDef frameDef;

	if (sequenceIndex >= _rateData.size())
		frameDef.delay = _metadata.perFrameDelay;
	else
		frameDef.delay = _rateData[sequenceIndex];

	if (sequenceIndex >= _seqData.size())
		frameDef.imageIndex = sequenceIndex;
	else
		frameDef.imageIndex = _seqData[sequenceIndex];

	return frameDef;
}

Common::SeekableReadStream *AniDecoder::openImageStream(uint imageIndex) const {
	if (imageIndex >= _frameDataLocations.size())
		error("Invalid ANI image index");

	const FrameDataRange &frameDataRange = _frameDataLocations[imageIndex];

	return new Common::SafeSeekableSubReadStream(_stream, frameDataRange.pos, frameDataRange.pos + frameDataRange.size);
}

bool AniDecoder::open(Common::SeekableReadStream &stream, DisposeAfterUse::Flag disposeAfterUse) {
	close();

	_stream = &stream;
	_disposeAfterUse = disposeAfterUse;

	bool loadedOK = load();
	if (!loadedOK)
		close();

	return loadedOK;
}

bool AniDecoder::load() {
	if (!parseRIFFChunks(*_stream, Common::Functor2Mem<const RIFFChunkDef &, Common::SeekableReadStream &, bool, AniDecoder>(this, &AniDecoder::parseTopLevelChunk))) {
		warning("AniDecoder::load: Failed to load ANI container");
		return false;
	}

	return true;
}

bool AniDecoder::parseRIFFChunks(Common::SeekableReadStream &stream, const RIFFChunkParseFunc_t &callback) {
	int64 nextChunkStartPos = 0;
	int64 endPos = stream.size();

	while (nextChunkStartPos < endPos) {
		if (!stream.seek(nextChunkStartPos)) {
			warning("AniDecoder::parseRIFFChunks: Failed to reset to start of RIFF chunk");
			return false;
		}

		byte riffChunkHeader[8];

		if (stream.read(riffChunkHeader, 8) != 8) {
			warning("AniDecoder::parseRIFFChunks: Failed to read RIFF chunk header");
			return false;
		}

		uint32 chunkSize = READ_LE_UINT32(riffChunkHeader + 4);

		int64 actualChunkSize = chunkSize;
		if (chunkSize & 1)
			actualChunkSize++;

		int64 chunkAvailable = stream.size() - stream.pos();
		if (chunkAvailable < actualChunkSize) {
			warning("AniDecoder::parseRIFFChunk: RIFF chunk is too large");
			return false;
		}

		RIFFChunkDef chunkDef;
		chunkDef.id = READ_BE_UINT32(riffChunkHeader);
		chunkDef.size = chunkSize;

		Common::SeekableSubReadStream substream(&stream, static_cast<uint32>(stream.pos()), static_cast<uint32>(stream.pos()) + chunkSize);
		if (!callback(chunkDef, substream))
			return false;

		nextChunkStartPos += actualChunkSize + 8;
	}

	return true;
}

bool AniDecoder::parseRIFFContainer(Common::SeekableReadStream &chunkStream, const RIFFChunkDef &chunkDef, const RIFFContainerParseFunc_t &callback) {
	if (chunkDef.size < 4) {
		warning("AniDecoder::parseRIFFContainer: RIFF container is too small");
		return false;
	}

	byte containerTypeID[4];
	if (chunkStream.read(containerTypeID, 4) != 4) {
		warning("AniDecoder::parseRIFFContainer: Failed to read RIFF container type");
		return false;
	}

	RIFFContainerDef containerDef;
	containerDef.id = READ_BE_UINT32(containerTypeID);
	containerDef.size = chunkDef.size - 4;

	Common::SeekableSubReadStream substream(&chunkStream, 4, chunkDef.size);
	return callback(containerDef, substream);
}

bool AniDecoder::parseTopLevelChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	if (chunk.id != MKTAG('R', 'I', 'F', 'F')) {
		warning("AniDecoder::parseTopLevelChunk: Top-level chunk isn't RIFF");
		return false;
	}

	return parseRIFFContainer(stream, chunk, Common::Functor2Mem<const RIFFContainerDef &, Common::SeekableReadStream &, bool, AniDecoder>(this, &AniDecoder::parseTopLevelContainer));
}

bool AniDecoder::parseTopLevelContainer(const RIFFContainerDef &container, Common::SeekableReadStream &stream) {
	if (container.id == MKTAG('A', 'C', 'O', 'N'))
		return parseRIFFChunks(stream, Common::Functor2Mem<const RIFFChunkDef &, Common::SeekableReadStream &, bool, AniDecoder>(this, &AniDecoder::parseSecondLevelChunk));

	warning("AniDecoder::parseTopLevelContainer: Top-level container isn't ACON");
	return false;
}

bool AniDecoder::parseSecondLevelChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	if (chunk.id == MKTAG('L', 'I', 'S', 'T'))
		return parseRIFFContainer(stream, chunk, Common::Functor2Mem<const RIFFContainerDef &, Common::SeekableReadStream &, bool, AniDecoder>(this, &AniDecoder::parseListContainer));

	if (chunk.id == MKTAG('a', 'n', 'i', 'h'))
		return parseAnimHeaderChunk(chunk, stream);

	if (chunk.id == MKTAG('s', 'e', 'q', ' '))
		return parseSeqChunk(chunk, stream);

	if (chunk.id == MKTAG('r', 'a', 't', 'e'))
		return parseRateChunk(chunk, stream);

	return true;
}

bool AniDecoder::parseListContainer(const RIFFContainerDef &container, Common::SeekableReadStream &stream) {
	if (container.id == MKTAG('f', 'r', 'a', 'm'))
		return parseRIFFChunks(stream, Common::Functor2Mem<const RIFFChunkDef &, Common::SeekableReadStream &, bool, AniDecoder>(this, &AniDecoder::parseIconChunk));

	return true;
}

bool AniDecoder::parseAnimHeaderChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	const uint32 expectedStructSize = 36;

	if (chunk.size < expectedStructSize) {
		warning("AniDecoder::parseAnimHeaderChunk: Chunk is too small");
		return false;
	}

	uint32 structSize = 0;
	uint32 flags = 0;
	if (!stream.readMultipleLE(structSize, _metadata.numFrames, _metadata.numSteps, _metadata.width, _metadata.height,
		_metadata.bitCount, _metadata.numPlanes, _metadata.perFrameDelay, flags) || structSize < expectedStructSize) {
		warning("AniDecoder::parseAnimHeaderChunk: Read failed");
		return false;
	}

	_metadata.isCURFormat = ((flags & 1) != 0);
	_metadata.haveSeqData = ((flags & 2) != 0);

	return true;
}

bool AniDecoder::parseSeqChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	uint32 numFrames = chunk.size / 4u;

	if (numFrames > 1000u) {
		warning("AniDecoder::parseRateChunk: Too many frames");
		return false;
	}

	if (numFrames > _seqData.size())
		_seqData.resize(numFrames);

	for (uint i = 0; i < numFrames; i++) {
		byte seqData[4];

		if (stream.read(seqData, 4) != 4) {
			warning("AniDecoder::parseRateChunk: Failed to read sequence information");
			return false;
		}

		_seqData[i] = READ_LE_UINT32(seqData);
	}

	return true;
}

bool AniDecoder::parseRateChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	uint32 numFrames = chunk.size / 4u;

	if (numFrames > 1000u) {
		warning("AniDecoder::parseRateChunk: Too many frames");
		return false;
	}

	if (numFrames > _rateData.size())
		_rateData.resize(numFrames);

	for (uint i = 0; i < numFrames; i++) {
		byte rateData[4];

		if (stream.read(rateData, 4) != 4) {
			warning("AniDecoder::parseRateChunk: Failed to read rate information");
			return false;
		}

		_rateData[i] = READ_LE_UINT32(rateData);
	}

	return true;
}

bool AniDecoder::parseIconChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream) {
	FrameDataRange frameDataRange;

	// Get the global stream position
	frameDataRange.pos = static_cast<uint32>(_stream->pos());
	frameDataRange.size = chunk.size;

	_frameDataLocations.push_back(frameDataRange);

	return true;
}


} // End of namespace Image

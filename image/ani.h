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

#ifndef IMAGE_ANI_H
#define IMAGE_ANI_H

#include "common/array.h"
#include "common/types.h"
#include "common/func.h"

namespace Common {

class SeekableReadStream;
struct IFFChunk;

} // End of namespace Common

namespace Graphics {

class Cursor;
struct Surface;

} // End of namespace Graphics

namespace Image {

class AniDecoder {
public:
	struct Metadata {
		Metadata();

		uint32 numFrames;	// Number of images
		uint32 numSteps;	// Number of frames (use the FrameDef to determine which frame)
		uint32 width;
		uint32 height;
		uint32 bitCount;
		uint32 numPlanes;
		uint32 perFrameDelay;
		bool haveSeqData;
		bool isCURFormat;
	};

	struct FrameDef {
		FrameDef();

		uint32 imageIndex;
		uint32 delay;	// In 1/60 sec
	};

	AniDecoder();
	~AniDecoder();

	bool open(Common::SeekableReadStream &stream, DisposeAfterUse::Flag = DisposeAfterUse::NO);
	void close();

	const Metadata &getMetadata() const;
	FrameDef getSequenceFrame(uint sequenceIndex) const;

	/**
	 * Opens a substream for an image.  If the metadata field
	 * "isCURFormat" is set, you can pass the stream to IcoCurDecoder to
	 * read it.  Otherwise, you must determine the format.  The stream
	 * is valid for as long as the stream used to construct the AniDecoder
	 * is valid.
	 * 
	 * @param imageIndex The index of the image in the ANI file.
	 * @return A substream for the image.
	 */
	Common::SeekableReadStream *openImageStream(uint imageIndex) const;

private:
	struct RIFFContainerDef {
		uint32 id;
		uint32 size;
	};

	struct RIFFChunkDef {
		uint32 id;
		uint32 size;
	};

	struct FrameDataRange {
		uint32 pos;
		uint32 size;
	};

	typedef Common::Functor2<const RIFFContainerDef &, Common::SeekableReadStream &, bool> RIFFContainerParseFunc_t;
	typedef Common::Functor2<const RIFFChunkDef &, Common::SeekableReadStream &, bool> RIFFChunkParseFunc_t;

	bool load();

	static bool parseRIFFChunks(Common::SeekableReadStream &stream, const RIFFChunkParseFunc_t &callback);
	static bool parseRIFFContainer(Common::SeekableReadStream &stream, const RIFFChunkDef &chunkDef, const RIFFContainerParseFunc_t &callback);

	bool parseTopLevelChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);
	bool parseTopLevelContainer(const RIFFContainerDef &container, Common::SeekableReadStream &stream);

	bool parseSecondLevelChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);

	bool parseListContainer(const RIFFContainerDef &container, Common::SeekableReadStream &stream);

	bool parseAnimHeaderChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);
	bool parseSeqChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);
	bool parseRateChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);
	bool parseIconChunk(const RIFFChunkDef &chunk, Common::SeekableReadStream &stream);

	Metadata _metadata;
	Common::Array<uint32> _rateData;
	Common::Array<uint32> _seqData;
	Common::Array<FrameDataRange> _frameDataLocations;

	Common::SeekableReadStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;
};

} // End of namespace Image

#endif

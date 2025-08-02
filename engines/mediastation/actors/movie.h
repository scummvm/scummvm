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

#ifndef MEDIASTATION_MOVIE_H
#define MEDIASTATION_MOVIE_H

#include "common/array.h"

#include "mediastation/actor.h"
#include "mediastation/audio.h"
#include "mediastation/datafile.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

enum MovieBlitType {
	kInvalidMovieBlit = 0,
	kUncompressedMovieBlit = 1,
	kUncompressedDeltaMovieBlit = 2,
	kCompressedDeltaMovieBlit = 3,
};

class MovieFrameHeader : public BitmapHeader {
public:
	MovieFrameHeader(Chunk &chunk);

	uint _index = 0;
	uint _keyframeEndInMilliseconds = 0;
};

class MovieFrameImage : public Bitmap {
public:
	MovieFrameImage(Chunk &chunk, MovieFrameHeader *header);
	virtual ~MovieFrameImage() override;

	uint32 index() { return _bitmapHeader->_index; }

private:
	MovieFrameHeader *_bitmapHeader = nullptr;
};

enum MovieSectionType {
	kMovieRootSection = 0x06a8,
	kMovieImageDataSection = 0x06a9,
	kMovieFrameDataSection = 0x06aa
};

struct MovieFrame {
	MovieFrame(Chunk &chunk);
	uint unk3 = 0;
	uint unk4 = 0;
	uint layerId = 0;
	uint startInMilliseconds = 0;
	uint endInMilliseconds = 0;
	Common::Point leftTop;
	Common::Point diffBetweenKeyframeAndFrame;
	MovieBlitType blitType = kInvalidMovieBlit;
	int16 zIndex = 0;
	uint keyframeIndex = 0;
	bool keepAfterEnd = false;
	uint index = 0;
	MovieFrameImage *image = nullptr;
	MovieFrameImage *keyframeImage = nullptr;
};

class StreamMovieActor : public SpatialEntity {
public:
	StreamMovieActor() : _framesOnScreen(StreamMovieActor::compareFramesByZIndex), SpatialEntity(kActorTypeMovie) {}
	virtual ~StreamMovieActor() override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void readSubfile(Subfile &subfile, Chunk &chunk) override;

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void process() override;

	virtual void draw(const Common::Array<Common::Rect> &dirtyRegion) override;

	virtual bool isVisible() const override { return _isVisible; }

	uint32 _audioChunkReference = 0;
	uint32 _animationChunkReference = 0;

private:
	AudioSequence _audioSequence;
	uint _audioChunkCount = 0;
	uint _fullTime = 0;

	uint _loadType = 0;
	bool _isPlaying = false;
	bool _hasStill = false;

	Common::Array<MovieFrame *> _frames;
	Common::Array<MovieFrameImage *> _images;

	Common::Array<MovieFrame *> _framesNotYetShown;
	Common::SortedArray<MovieFrame *, const MovieFrame *> _framesOnScreen;

	// Script method implementations.
	void timePlay();
	void timeStop();

	void setVisibility(bool visibility);
	void updateFrameState();
	void invalidateRect(const Common::Rect &rect);
	void decompressIntoAuxImage(MovieFrame *frame);

	void readImageData(Chunk &chunk);
	void readFrameData(Chunk &chunk);

	Common::Rect getFrameBoundingBox(MovieFrame *frame);
	static int compareFramesByZIndex(const MovieFrame *a, const MovieFrame *b);
};

} // End of namespace MediaStation

#endif

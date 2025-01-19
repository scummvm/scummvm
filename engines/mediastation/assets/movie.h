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
#include "audio/audiostream.h"

#include "mediastation/subfile.h"
#include "mediastation/chunk.h"
#include "mediastation/assetheader.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class MovieFrameHeader : public BitmapHeader {
public:
	MovieFrameHeader(Chunk &chunk);

	uint _index = 0;
	uint _keyframeEndInMilliseconds = 0;
};

class MovieFrameFooter {
public:
	MovieFrameFooter(Chunk &chunk);

	uint _unk1 = 0;
	uint _unk2 = 0;
	uint _startInMilliseconds = 0;
	uint _endInMilliseconds = 0;
	uint _left = 0;
	uint _top = 0;
	uint _unk3 = 0;
	uint _unk4 = 0;
	uint _zIndex = 0; // TODO: This is still unconfirmed but seems likely.
	uint _diffBetweenKeyframeAndFrameX = 0;
	uint _diffBetweenKeyframeAndFrameY = 0;
	uint _keyframeIndex = 0;
	uint _unk9 = 0;
	uint _index = 0;
};

class MovieFrame : public Bitmap {
public:
	MovieFrame(Chunk &chunk, MovieFrameHeader *header);
	~MovieFrame();

	void setFooter(MovieFrameFooter *footer);
	uint32 left();
	uint32 top();
	Common::Point topLeft();
	Common::Rect boundingBox();
	uint32 index();
	uint32 startInMilliseconds();
	uint32 endInMilliseconds();
	uint32 keyframeEndInMilliseconds();
	// This is called zCoordinate because zIndex is too close to "index" and
	// that could be confusing.
	uint32 zCoordinate();

	bool _showing = false;

private:
	MovieFrameHeader *_bitmapHeader = nullptr;
	MovieFrameFooter *_footer = nullptr;
};

enum MovieSectionType {
	kMovieRootSection = 0x06a8,
	kMovieFrameSection = 0x06a9,
	kMovieFooterSection = 0x06aa
};

class Movie : public Asset {
public:
	Movie(AssetHeader *header) : Asset(header) {};
	virtual ~Movie() override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void readSubfile(Subfile &subfile, Chunk &chunk) override;

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;
	virtual void process() override;

private:
	Common::Array<MovieFrame *> _frames;
	Common::Array<MovieFrame *> _stills;
	Common::Array<MovieFrameFooter *> _footers;
	Common::Array<Audio::SeekableAudioStream *> _audioStreams;

	// Method implementations. These should be called from callMethod.
	void timePlay();
	void timeStop();

	// Internal helper functions.
	bool drawNextFrame();
	void processTimeEventHandlers();
};

} // End of namespace MediaStation

#endif
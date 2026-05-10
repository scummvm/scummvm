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

class MovieFrameImage : public PixMapImage {
public:
	MovieFrameImage(Chunk &chunk, uint index, uint keyframeEndInMilliseconds, const ImageInfo &imageInfo);

	uint _index = 0;
	uint _keyframeEndInMilliseconds = 0;
};

enum MovieSectionType {
	kMovieRootSection = 0x06a8,
	kMovieImageDataSection = 0x06a9,
	kMovieFrameDataSection = 0x06aa,
	kMovieChunkMarkerSection = 0x06ab
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

class StreamMovieActor;

// Represents an individually controllable layer of a stream movie that is its own spatial entity
// and can be in a different stage than its parent stream movie, but which references the parent's
// frames at all times. Only frames with a layer ID matching a proxy's layer ID will be drawn when
// that proxy is drawn. For example, this is used in the last section of the Dalmatians hide-and-seek
// minigame to show a "light" layer that only appears when the player shines a flashlight on an area,
// while a "dark" layer seamlessly displays otherwise.
class StreamMovieProxy : public SpatialEntity {
friend class StreamMovieActor;

public:
	StreamMovieProxy(Chunk &chunk, StreamMovieActor *parent);
	virtual void draw(DisplayContext &displayContext) override;
	virtual bool isVisible() const override;

	uint _layerId = 0;
	uint _scriptId = 0;

private:
	StreamMovieActor *_parent = nullptr;
};

// This is called `RT_stmvFrames` in the original.
class StreamMovieActorFrames : public ChannelClient {
public:
	StreamMovieActorFrames(StreamMovieActor *parent) : ChannelClient(), _parent(parent) {}
	~StreamMovieActorFrames();

	virtual void readChunk(Chunk &chunk) override;

	Common::Array<MovieFrame *> _frames;
	Common::Array<MovieFrameImage *> _images;

private:
	StreamMovieActor *_parent = nullptr;

	void readImageData(Chunk &chunk);
	void readFrameData(Chunk &chunk);
};

// This is called `RT_stmvSound` in the original.
class StreamMovieActorSound : public ChannelClient, public SoundClient {
public:
	StreamMovieActorSound(StreamMovieActor *parent) : ChannelClient(), _audioSequence(this), _parent(parent) {}
	~StreamMovieActorSound();
	virtual void readChunk(Chunk &chunk) override;
	virtual void soundPlayStateChanged(SoundPlayState state, SoundStopReason why) override;

	AudioSequence &getAudioSequence() { return _audioSequence; }

private:
	AudioSequence _audioSequence;
	StreamMovieActor *_parent = nullptr;
};

class StreamMovieActor : public SpatialEntity, public ChannelClient, public PreDisplaySyncClient {
friend class StreamMovieActorFrames;
friend class StreamMovieActorSound;

public:
	StreamMovieActor();
	virtual ~StreamMovieActor() override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void loadIsComplete() override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual PreDisplaySyncState preDisplaySync() override;
	virtual void onEvent(const ActorEvent &event) override;
	virtual void timerEvent(const TimerEvent &event) override;

	virtual void draw(DisplayContext &displayContext) override;
	void drawLayer(DisplayContext &displayContext, uint layerId);
	virtual void invalidateLocalBounds() override;
	bool isLayerInSeparateZPlane(uint layerId);

private:
	ImtStreamFeed *_streamFeed = nullptr;
	uint _fullTime = 0;
	uint _chunkCount = 0;
	double _frameRate = 0;
	uint _disableScreenAutoUpdateToken = 0;

	bool _shouldCache = false;
	bool _isPlaying = false;
	bool _hasStill = false;

	StreamMovieActorFrames *_streamFrames = nullptr;
	StreamMovieActorSound *_streamSound = nullptr;

	Common::Array<MovieFrame *> _framesNotYetShown;
	Common::Array<StreamMovieProxy *> _proxies;
	Common::SortedArray<MovieFrame *, const MovieFrame *> _framesOnScreen;

	// Script method implementations.
	void timePlay();
	void timeStop(bool isMovieEnd);

	void setVisibility(bool visibility);
	void updateFrameState();
	void invalidateRect(const Common::Rect &rect);
	void decompressIntoAuxImage(MovieFrame *frame);

	void parseMovieHeader(Chunk &chunk);
	void parseMovieChunkMarker(Chunk &chunk);

	Common::Rect getFrameBoundingBox(MovieFrame *frame);
	StreamMovieProxy *proxyOfId(uint layerId);
	StreamMovieProxy *proxyOfScriptId(uint scriptId);
	static int compareFramesByZIndex(const MovieFrame *a, const MovieFrame *b);
};

} // End of namespace MediaStation

#endif

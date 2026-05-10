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

#ifndef MEDIASTATION_SPRITE_H
#define MEDIASTATION_SPRITE_H

#include "common/rect.h"
#include "common/array.h"
#include "common/ptr.h"

#include "mediastation/actor.h"
#include "mediastation/datafile.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

struct SpriteMovieClip {
	SpriteMovieClip() = default;
	SpriteMovieClip(uint clipId, int first, int last);
	Common::String getDebugString() const;

	uint id = 0;
	int firstFrameIndex = 0;
	int lastFrameIndex = 0;
};

class SpriteFrame : public PixMapImage {
public:
	SpriteFrame(Chunk &chunk, uint index, Common::Point origin, const ImageInfo &imageInfo, bool decompressInPlace);

	int _index = 0;
	Common::Point _origin;
};

// The original had a separate class that did reference counting,
// for sharing an asset across actors, but we can just use a SharedPtr.
struct SpriteAsset {
	~SpriteAsset();

	uint frameCount = 0;
	Common::Array<SpriteFrame *> frames;
};

// Sprites are somewhat like movies, but they strictly show one frame at a time
// and don't have sound. They are intended for background/recurrent animations.
class SpriteMovieActor : public SpatialEntity, public ChannelClient {
public:
	SpriteMovieActor() : SpatialEntity(kActorTypeSprite) {};
	~SpriteMovieActor();

	virtual void draw(DisplayContext &displayContext) override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual void loadIsComplete() override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual void onEvent(const ActorEvent &event) override;
	virtual void timerEvent(const TimerEvent &event) override;

private:
	const uint DEFAULT_FORWARD_CLIP_ID = 0x4B0;
	const uint DEFAULT_BACKWARD_CLIP_ID = 0x4B1;

	bool _decompressInPlace = false;
	uint _frameRate = 0;
	uint _actorReference = 0;
	Common::HashMap<uint, SpriteMovieClip> _clips;
	Common::SharedPtr<SpriteAsset> _asset;
	bool _isPlaying = false;
	int _currentFrameIndex = 0;
	uint _nextFrameTime = 0;
	uint _defaultClipId = DEFAULT_FORWARD_CLIP_ID;
	SpriteMovieClip _activeClip;

	void play();
	void stop();
	void setCurrentClip(uint clipId);

	bool activateNextFrame();
	bool activatePreviousFrame();

	void dirtyIfVisible();
	void setCurrentFrameToInitial();
	void setCurrentFrameToFinal();

	void scheduleNextFrame();
	void scheduleNextTimerEvent();
	void postMovieEndEventIfNecessary();
	void setVisibility(bool visibility);
};

} // End of namespace MediaStation

#endif

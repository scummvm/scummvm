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

#include "mediastation/asset.h"
#include "mediastation/datafile.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

struct SpriteClip {
	uint id = 0;
	uint firstFrameIndex = 0;
	uint lastFrameIndex = 0;
};

class SpriteFrameHeader : public BitmapHeader {
public:
	SpriteFrameHeader(Chunk &chunk);

	uint _index;
	Common::Point _boundingBox;
};

class SpriteFrame : public Bitmap {
public:
	SpriteFrame(Chunk &chunk, SpriteFrameHeader *header);
	virtual ~SpriteFrame() override;

	uint32 left();
	uint32 top();
	Common::Point topLeft();
	Common::Rect boundingBox();
	uint32 index();

private:
	SpriteFrameHeader *_bitmapHeader = nullptr;
};

// Sprites are somewhat like movies, but they strictly show one frame at a time
// and don't have sound. They are intended for background/recurrent animations.
class Sprite : public SpatialEntity {
friend class Context;

public:
	Sprite() : SpatialEntity(kAssetTypeSprite) {};
	~Sprite();

	virtual void process() override;
	virtual void draw(const Common::Array<Common::Rect> &dirtyRegion) override;

	virtual void readParameter(Chunk &chunk, AssetHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual bool isVisible() const override { return _isVisible; }

	virtual void readChunk(Chunk &chunk) override;

private:
	static const uint DEFAULT_CLIP_ID = 1200;
	uint _loadType = 0;
	uint _frameRate = 0;
	uint _frameCount = 0;
	Common::HashMap<uint, SpriteClip> _clips;
	Common::Array<SpriteFrame *> _frames;
	bool _isPlaying = false;
	uint _currentFrameIndex = 0;
	uint _nextFrameTime = 0;
	SpriteClip _activeClip;

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

	void updateFrameState();
	void timerEvent();
};

} // End of namespace MediaStation

#endif

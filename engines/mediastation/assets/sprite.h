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

#include "mediastation/asset.h"
#include "mediastation/assetheader.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/operand.h"

namespace MediaStation {

class SpriteFrameHeader : public BitmapHeader {
public:
	SpriteFrameHeader(Chunk &chunk);
	~SpriteFrameHeader();

	uint _index;
	Common::Point *_boundingBox;
};

class SpriteFrame : public Bitmap {
public:
	SpriteFrame(Chunk &chunk, SpriteFrameHeader *header);
	~SpriteFrame();

	uint32 left();
	uint32 top();
	Common::Point topLeft();
	Common::Rect boundingBox();
	uint32 index();

private:
	SpriteFrameHeader *_bitmapHeader = nullptr;
};

class Sprite : public Asset {
public:
	Sprite(AssetHeader *header) : Asset(header) {};
	~Sprite();

	virtual Operand callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) override;
	virtual void process() override;

	virtual void readChunk(Chunk &chunk) override;

private:
	Common::Array<SpriteFrame *> _frames;
	SpriteFrame *_persistFrame = nullptr;
	uint _currentFrameIndex = 0;
	uint _nextFrameTime = 0;

	// Method implementations.
	void spatialShow();
	void timePlay();
	void movieReset();

	// Helper functions.
	void drawNextFrame();
	void drawFrame(SpriteFrame *frame);
};

} // End of namespace MediaStation

#endif
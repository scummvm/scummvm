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

#ifndef MEDIASTATION_IMAGE_H
#define MEDIASTATION_IMAGE_H

#include "common/ptr.h"

#include "mediastation/actor.h"
#include "mediastation/datafile.h"
#include "mediastation/bitmap.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

// The original had a separate class that did reference counting,
// for sharing an asset across actors, but we can just use a SharedPtr.
struct ImageAsset {
	~ImageAsset();

	Bitmap *bitmap = nullptr;
};

class ImageActor : public SpatialEntity, public ChannelClient {
public:
	ImageActor() : SpatialEntity(kActorTypeImage) {};
	virtual ~ImageActor() override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void draw(DisplayContext &displayContext) override;
	virtual Common::Rect getBbox() const override;

private:
	Common::SharedPtr<ImageAsset> _asset;
	uint _loadType = 0;
	int _xOffset = 0;
	int _yOffset = 0;
	uint _actorReference = 0;

	// Script method implementations.
	void spatialShow();
	void spatialHide();
};

} // End of namespace MediaStation

#endif

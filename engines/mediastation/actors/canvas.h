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

#ifndef MEDIASTATION_CANVAS_H
#define MEDIASTATION_CANVAS_H

#include "common/rect.h"

#include "mediastation/actor.h"
#include "mediastation/graphics.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

struct ImageAsset;

class CanvasActor : public SpatialEntity, public ChannelClient {
public:
	CanvasActor() : SpatialEntity(kActorTypeCanvas) {
		_dissolveFactor = 1.0;
		_isVisible = true;
		_hasTransparency = true;
	}

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void loadIsComplete() override;
	virtual void draw(DisplayContext &displayContext) override;

private:
	Common::Point _offset;
	Common::SharedPtr<ImageAsset> _image;
	DisplayContext _displayContext;

	void setVisibility(bool visibility);
	void fillCanvas(uint paletteIndex);
	void clearToTransparency();
	void stampImage(const Common::Point &dest, uint actorId);
	void copyScreenTo(const Common::Point &dest);
	void clearToPalette(uint colorIndex);
};

} // End of namespace MediaStation

#endif

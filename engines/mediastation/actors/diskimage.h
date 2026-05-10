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

#ifndef MEDIASTATION_ACTORS_DISKIMAGE_H
#define MEDIASTATION_ACTORS_DISKIMAGE_H

#include "mediastation/actor.h"
#include "mediastation/bitmap.h"
#include "mediastation/datafile.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

struct StripInfoNode {
	bool isLoaded = false;
	bool isLoadScheduled = false;
	Common::Rect rect;
	uint streamId = 0;
	uint lengthInBytes = 0;
};

struct StripImageNode {
	bool isLoaded = false;
	PixMapImage *image = nullptr;
	uint lastDrawTime = 0;
};

// Despite the name from the original, this is not a "disk image" but
// a set of graphics (like a large background) that are streamed from
// disk in a very particular way.
class DiskImageActor : public SpatialEntity, public ChannelClient {
public:
	DiskImageActor() : SpatialEntity(kActorTypeDiskImage) {};
	~DiskImageActor();

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual void onEvent(const ActorEvent &event) override;
	virtual void timerEvent(const TimerEvent &event) override;

	virtual void readChunk(Chunk &chunk) override;
	virtual void draw(DisplayContext &displayContext) override;
	virtual void preload(const Common::Rect &rect, bool fireStepEvent = true) override;
	virtual bool isReadyToDraw(DisplayContext &displayContext) override;
	virtual bool isRectInMemory(const Common::Rect &rectToCheck) override;
	virtual void setAdjustedBounds(CylindricalWrapMode wrapMode) override;

private:
	void setStripInfo(Chunk &chunk);
	void setStripsToLoad(const Common::Rect &rectToLoad);
	int getStripToLoad();
	void startStripLoad(uint stripIndex);

	void purge();
	void stopLoad();
	void unloadLeastRecentlyDrawnStrip();
	void debugPrintNodes();

	Common::HashMap<uint, StripInfoNode> _stripInfoNodes;
	Common::HashMap<uint, StripImageNode> _stripImageNodes;

	Common::Rect _rectToLoad;
	bool _shouldDecompressInPlace = false;
	bool _isLoading = false;
	bool _isLoadingStrips = false;
	bool _firePreloadEvent = false;
	uint _maxStripsInMemory = 0;
	int16 _stripThickness = 0;
	uint _maxImagesInMemory = 0;
	bool _useVerticalStrips = false;
};

} // End of namespace MediaStation

#endif

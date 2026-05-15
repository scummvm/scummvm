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

#ifndef MEDIASTATION_DOTGAME_H
#define MEDIASTATION_DOTGAME_H

#include "mediastation/actors/stage.h"

namespace MediaStation {

class DotGameActor : public SpatialEntity {
public:
	DotGameActor() : SpatialEntity(kActorTypeDotGame) {};

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void draw(DisplayContext &displayContext) override;
	virtual void loadIsComplete() override;

private:
	uint16 _totalDots = 0;
	Common::Array<Common::Point> _dotPositions;
	uint16 _startHotspotId = 0;
	uint16 _endHotspotId = 0;
	uint16 _markerActorId = 0;
	uint16 _currentDotIndex = 0;
	uint16 _speed = 1;
	uint8 _lineThickness = 3;
	byte _lineColorR = 0;
	byte _lineColorG = 0;
	byte _lineColorB = 0;
	byte _linePaletteIndex = 0;
	Common::Point _currentPosition;
	uint16 _animationProgress = 100;

	void activateHelpers();
	void deActivateHelpers();
	void doHit();
	void doReset(int16 targetDot);
	void updateHelpers();
};

} // End namespace MediaStation

#endif

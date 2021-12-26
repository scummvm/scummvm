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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ILLUSIONS_BBDOU_BBDOU_FOODCTL_H
#define ILLUSIONS_BBDOU_BBDOU_FOODCTL_H

#include "illusions/specialcode.h"
#include "illusions/thread.h"

namespace Illusions {

class IllusionsEngine_BBDOU;

const uint kFoodMaxPropertyIdsCount = 15;
const uint kFoodCount = 16;

struct FoodItem {
	uint32 objectId;
	int value;
};

class BbdouFoodCtl {
public:
	BbdouFoodCtl(IllusionsEngine_BBDOU *vm);
	~BbdouFoodCtl();
	void placeFood(uint totalRoundsCount, uint maxRequestedFoodCount);
	void addFood(uint32 propertyId);
	void requestFirstFood();
	void requestNextFood();
	void nextRound();
	bool hasReachedRequestedFoodCount();
	bool hasRoundFinished();
	void serveFood();
	void resetFood();
protected:
	IllusionsEngine_BBDOU *_vm;
	uint _totalRoundsCount, _maxRequestedFoodCount;
	uint32 _foodPropertyIds[kFoodMaxPropertyIdsCount];
	uint _requestedFoodCount;
	uint _requestedFoodIndex;
	FoodItem _foodItems[kFoodCount];
	uint _servedFoodCount;
	void placeActors();
	void resetFoodControls();
	uint32 getFoodSequenceId();
	uint32 getCurrentStudentObjectId();
};

} // End of namespace Illusions

#endif // ILLUSIONS_BBDOU_BBDOU_FOODCTL_H

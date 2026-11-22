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

#ifndef NGI_ANIHANDLER_H
#define NGI_ANIHANDLER_H

namespace NGI {

class ExCommand2;
class Movement;
class Statics;

struct MGMSubItem {
	Movement *movement;
	int staticsIndex;
	int movementCount;
	int frameCount;
	int x;
	int y;

	MGMSubItem();

	void reset() {
		movement = nullptr;
		staticsIndex = 0;
		movementCount = 0;
		frameCount = 0;
		x = 0;
		y = 0;
	}
};

struct MGMItem {
	int16 objId;
	Common::Array<MGMSubItem> transitions;
	Common::Array<Statics *> statics;
	Common::Array<Movement *> movements;
	Common::Array<int> visitedMovements;

	MGMItem();
};

struct MakeQueueStruct {
	AniObject *ani;
	int staticsId1;
	int staticsId2;
	int movementId;
	int cycleCount;
	int x1;
	int y1;
	int destZ;
	int x2;
	int y2;
	int flags;

	MakeQueueStruct() { reset(); }

	void reset() {
		ani = nullptr;
		staticsId1 = 0;
		staticsId2 = 0;
		movementId = 0;
		cycleCount = 0;
		x1 = 0;
		y1 = 0;
		destZ = 0;
		x2 = 0;
		y2 = 0;
		flags = 0;
	}
};

class AniHandler : public CObject {
protected:
	Common::Array<MGMItem> _items;

public:
	void detachAllObjects();
	void attachObject(int objId);
	void resetData(int objId);
	int getIndex(int objId);

	MessageQueue *makeRunQueue(MakeQueueStruct *mkQueue);
	void putObjectToStatic(AniObject *ani, int staticsId);
	Common::Point getTransitionSize(int aniId, int staticsId1, int staticsId2);
	int getStaticIndex(int idx, int16 id);
	int getStaticIndex(int idx, Statics *st);
	void clearVisitsList(int idx);
	int seekWay(int idx, int st1idx, int st2idx, bool allowReverse, bool linkedOnly);
	Common::Point getNumCycles(Movement *mov, int x, int y, int *mult, int *len, int flag);
	ExCommand2 *createCommand(Movement *mov, int objId, int x1, int y1, Common::Point &x2, Common::Point &y2, int len);
	MessageQueue *makeQueue(AniObject *ani, int staticsIndex, int staticsId, int *resStatId, Common::Point **pointArr);
	int getFramesCount(int idx, int subIdx, int subOffset, int flag);
	int getNumMovements(int objectId, int idx1, int idx2);
};

} // End of namespace NGI

#endif /* NGI_ANIHANDLER_H */

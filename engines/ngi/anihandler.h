/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	int field_8;
	int field_C;
	int x;
	int y;

	MGMSubItem();

	void reset() {
		movement = nullptr;
		staticsIndex = 0;
		field_8 = 0;
		field_C = 0;
		x = 0;
		y = 0;
	}
};

struct MGMItem {
	int16 objId;
	Common::Array<MGMSubItem> subItems;
	Common::Array<Statics *> statics;
	Common::Array<Movement *> movements1;
	Common::Array<int> movements2;

	MGMItem();
};

struct MakeQueueStruct {
	StaticANIObject *ani;
	int staticsId1;
	int staticsId2;
	int movementId;
	int field_10;
	int x1;
	int y1;
	int field_1C;
	int x2;
	int y2;
	int flags;

	MakeQueueStruct() { reset(); }

	void reset() {
		ani = nullptr;
		staticsId1 = 0;
		staticsId2 = 0;
		movementId = 0;
		field_10 = 0;
		x1 = 0;
		y1 = 0;
		field_1C = 0;
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
	void putObjectToStatics(StaticANIObject *ani, int staticsId);
	Common::Point getTransitionSize(int aniId, int staticsId1, int staticsId2);
	int getStaticsIndexById(int idx, int16 id);
	int getStaticsIndex(int idx, Statics *st);
	void clearVisitsList(int idx);
	int seekWay(int idx, int st1idx, int st2idx, bool flip, bool flop);
	Common::Point getNumCycles(Movement *mov, int x, int y, int *mult, int *len, int flag);
	ExCommand2 *createCommand(Movement *mov, int objId, int x1, int y1, Common::Point &x2, Common::Point &y2, int len);
	MessageQueue *makeQueue(StaticANIObject *ani, int staticsIndex, int staticsId, int *resStatId, Common::Point **pointArr);
	int getFramesCount(int idx, int subIdx, int subOffset, int flag);
	int getNumMovements(int objectId, int idx1, int idx2);
};

} // End of namespace NGI

#endif /* NGI_ANIHANDLER_H */

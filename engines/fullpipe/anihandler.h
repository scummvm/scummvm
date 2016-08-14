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

#ifndef FULLPIPE_MGM_H
#define FULLPIPE_MGM_H

namespace Fullpipe {

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
};

struct MGMItem {
	int16 objId;
	Common::Array<MGMSubItem *> subItems;
	Common::Array<Statics *> statics;
	Common::Array<Movement *> movements1;
	Common::Array<int> movements2;

	MGMItem();
};

struct MGMInfo {
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

	MGMInfo() { memset(this, 0, sizeof(MGMInfo)); }
};

class MGM : public CObject {
public:
	Common::Array<MGMItem *> _items;

public:
	void clear();
	void addItem(int objId);
	void rebuildTables(int objId);
	int getItemIndexById(int objId);

	MessageQueue *genMovement(MGMInfo *mgminfo);
	void updateAnimStatics(StaticANIObject *ani, int staticsId);
	Common::Point *getPoint(Common::Point *point, int aniId, int staticsId1, int staticsId2);
	int getStaticsIndexById(int idx, int16 id);
	int getStaticsIndex(int idx, Statics *st);
	void clearMovements2(int idx);
	int recalcOffsets(int idx, int st1idx, int st2idx, bool flip, bool flop);
	Common::Point *calcLength(Common::Point *point, Movement *mov, int x, int y, int *mult, int *len, int flag);
	ExCommand2 *buildExCommand2(Movement *mov, int objId, int x1, int y1, Common::Point *x2, Common::Point *y2, int len);
	MessageQueue *genMQ(StaticANIObject *ani, int staticsIndex, int staticsId, int *resStatId, Common::Point **pointArr);
	int countPhases(int idx, int subIdx, int subOffset, int flag);
	int refreshOffsets(int objectId, int idx1, int idx2);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_MGM_H */

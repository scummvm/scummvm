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

#ifndef FULLPIPE_BEHAVIOR_H
#define FULLPIPE_BEHAVIOR_H

namespace Fullpipe {

struct BehaviorEntryInfo {
	MessageQueue *_messageQueue;
	int _delay;
	uint32 _percent;
	int _flags;

	BehaviorEntryInfo(GameVar *subvar, Scene *sc, int *delay);
};

struct BehaviorEntry {
	int _staticsId;
	int _itemsCount;
	int _flags;
	BehaviorEntryInfo **_items;

	BehaviorEntry();
	BehaviorEntry(GameVar *var, Scene *sc, StaticANIObject *ani, int *minDelay);
};

struct BehaviorInfo {
	StaticANIObject *_ani;
	int _staticsId;
	int _counter;
	int _counterMax;
	int _flags;
	int _subIndex;
	int _itemsCount;
	Common::Array<BehaviorEntry *> _bheItems;

	BehaviorInfo() { clear(); }

	void clear();
	void initAmbientBehavior(GameVar *var, Scene *sc);
	void initObjectBehavior(GameVar *var, Scene *sc, StaticANIObject *ani);
};

class BehaviorManager : public CObject {
	Common::Array<BehaviorInfo *> _behaviors;
	Scene *_scene;
	bool _isActive;

  public:
	BehaviorManager();
	~BehaviorManager();

	void clear();

	void initBehavior(Scene *scene, GameVar *var);

	void updateBehaviors();
	void updateBehavior(BehaviorInfo *behaviorInfo, BehaviorEntry *entry);
	void updateStaticAniBehavior(StaticANIObject *ani, int delay, BehaviorEntry *beh);

	bool setBehaviorEnabled(StaticANIObject *obj, int aniId, int quId, int flag);

	void setFlagByStaticAniObject(StaticANIObject *ani, int flag);

	BehaviorEntryInfo *getBehaviorEntryInfoByMessageQueueDataId(StaticANIObject *ani, int id1, int id2);
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_BEHAVIOR_H */

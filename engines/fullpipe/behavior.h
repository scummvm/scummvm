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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef FULLPIPE_BEHAVIOR_H
#define FULLPIPE_BEHAVIOR_H

namespace Fullpipe {

class BehaviorManager : public CObject {
	CObArray _behaviors;
	Scene *_scene;
	bool _isActive;

  public:
	BehaviorManager();

	void initBehavior(Scene *scene, CGameVar *var);
};

struct BehaviorEntryInfo {
	int _messageQueue;
	int _delay;
	int _percent;
	int _flags;
};

struct BehaviorEntry {
	int _staticsId;
	int _itemsCount;
	int _flags;
	BehaviorEntryInfo *_items;
};

struct BehaviorInfo {
	StaticANIObject *_ani;
	int _staticsId;
	int _counter;
	int _counterMax;
	int _flags;
	int _subIndex;
	int _itemsCount;
	BehaviorEntryInfo *_items;
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_BEHAVIOR_H */

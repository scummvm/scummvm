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

#ifndef ILLUSIONS_DICTIONARY_H
#define ILLUSIONS_DICTIONARY_H

#include "common/hashmap.h"

namespace Illusions {

class ActorType;
class Control;
class Sequence;
class TalkEntry;

template<class T>
class DictionaryHashMap {
public:

	void add(uint32 id, T *value) {
		_map[id] = value;
	}

	void remove(uint32 id) {
		_map.erase(id);
	}

	T *find(uint32 id) {
		typename Common::HashMap<uint32, T*>::iterator it = _map.find(id);
		if (it != _map.end())
			return it->_value;
		return 0;
	}

protected:
	Common::HashMap<uint32, T*> _map;
};

class Dictionary {
public:

	void addActorType(uint32 id, ActorType *actorType);
	void removeActorType(uint32 id);
	ActorType *findActorType(uint32 id);

    void addSequence(uint32 id, Sequence *sequence);
	void removeSequence(uint32 id);
	Sequence *findSequence(uint32 id);

    void addTalkEntry(uint32 id, TalkEntry *talkEntry);
	void removeTalkEntry(uint32 id);
	TalkEntry *findTalkEntry(uint32 id);

    void setObjectControl(uint32 objectId, Control *control);
    Control *getObjectControl(uint32 objectId);

protected:
	DictionaryHashMap<ActorType> _actorTypes;
	DictionaryHashMap<Sequence> _sequences;
	DictionaryHashMap<TalkEntry> _talkEntries;
	DictionaryHashMap<Control> _controls;
};

} // End of namespace Illusions

#endif // ILLUSIONS_DICTIONARY_H

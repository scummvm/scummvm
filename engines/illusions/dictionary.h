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

struct ActorType;
class Control;
class FontResource;
struct Sequence;
struct TalkEntry;

template<class T>
class DictionaryHashMap {
protected:
	typedef Common::List<T*> List;
	typedef typename List::iterator ListIterator;
	typedef Common::HashMap<uint32, List*> Map;
	typedef typename Map::iterator MapIterator;
	Map _map;
public:

	~DictionaryHashMap() {
		for (MapIterator it = _map.begin(); it != _map.end(); ++it) {
			delete it->_value;
		}
	}

	void add(uint32 id, T *value) {
		MapIterator it = _map.find(id);
		List *list;
		if (it != _map.end())
			list = it->_value;
		else {
			 list = new List();
			 _map[id] = list;
		}
		list->push_back(value);
	}

	void remove(uint32 id) {
		MapIterator it = _map.find(id);
		List *list;
		if (it != _map.end()) {
			list = it->_value;
			list->pop_back();
			if (list->empty()) {
				_map.erase(id);
				delete list;
			}
		}
	}

	T *find(uint32 id) {
		MapIterator it = _map.find(id);
		if (it != _map.end())
			return it->_value->back();
		return 0;
	}

};

class Dictionary {
public:

	void addActorType(uint32 id, ActorType *actorType);
	void removeActorType(uint32 id);
	ActorType *findActorType(uint32 id);

	void addFont(uint32 id, FontResource *fontResource);
	void removeFont(uint32 id);
	FontResource *findFont(uint32 id);

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
	DictionaryHashMap<Control> _controls;
	DictionaryHashMap<FontResource> _fontResources;
	DictionaryHashMap<Sequence> _sequences;
	DictionaryHashMap<TalkEntry> _talkEntries;
};

} // End of namespace Illusions

#endif // ILLUSIONS_DICTIONARY_H

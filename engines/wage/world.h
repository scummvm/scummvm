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
 * $URL$
 * $Id$
 *
 */

#ifndef WAGE_WORLD_H
#define WAGE_WORLD_H

#include "wage/scene.h"
#include "wage/script.h"
#include "wage/obj.h"
#include "wage/chr.h"
#include "wage/sound.h"

namespace Wage {

#define STORAGESCENE "STORAGE@"

class World {
public:
	World();
	~World();
 
	bool loadWorld(MacResManager *resMan);

	String _name;
	String _aboutMessage;
	String _soundLibrary1;
	String _soundLibrary2;

	bool _weaponMenuDisabled;
	Script *_globalScript;
	Common::HashMap<String, Scene *> _scenes;
	Common::HashMap<String, Obj *> _objs;
	Common::HashMap<String, Chr *> _chrs;
	Common::HashMap<String, Sound *> _sounds;
	Common::List<Scene *> _orderedScenes;
	Common::List<Obj *> _orderedObjs;
	Common::List<Chr *> _orderedChrs;
	Common::List<Sound *> _orderedSounds;
	Common::List<byte *> _patterns;
	Scene _storageScene;
	Chr _player;
	//List<MoveListener> moveListeners;

	void addScene(Scene *room) {
		if (room->_name.size() != 0) {
			String s = room->_name;
			s.toLowercase();
			_scenes[s] = room;
		}
		_orderedScenes.push_back(room);
	}

	void addObj(Obj *obj) {
		String s = obj->_name;
		s.toLowercase();
		_objs[s] = obj;
		obj->_index = _orderedObjs.size();
		_orderedObjs.push_back(obj);
	}

	void addChr(Chr *chr) {
		String s = chr->_name;
		s.toLowercase();
		_chrs[s] = chr;
		chr->_index = _orderedChrs.size();
		_orderedChrs.push_back(chr);
	}

	void addSound(Sound *sound) {
		String s = sound->_name;
		s.toLowercase();
		_sounds[s] = sound;
		_orderedSounds.push_back(sound);
	}
};
 
} // End of namespace Wage
 
#endif

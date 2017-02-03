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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WAGE_WORLD_H
#define WAGE_WORLD_H

#include "wage/entities.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Wage {

// Import the enum definitions
using Graphics::MacPatterns;

class Script;
class Sound;

class World {
public:
	World(WageEngine *engine);
	~World();

	bool loadWorld(Common::MacResManager *resMan);
	void loadExternalSounds(Common::String fname);
	Common::String *loadStringFromDITL(Common::MacResManager *resMan, int resourceId, int itemIndex);
	void move(Obj *obj, Chr *chr);
	void move(Obj *obj, Scene *scene, bool skipSort = false);
	void move(Chr *chr, Scene *scene, bool skipSort = false);
	Scene *getRandomScene();
	Scene *getSceneAt(int x, int y);
	bool scenesAreConnected(Scene *scene1, Scene *scene2);
	const char *getAboutMenuItemName();

	WageEngine *_engine;

	Common::String _name;
	Common::String _aboutMessage;
	Common::String _soundLibrary1;
	Common::String _soundLibrary2;

	bool _weaponMenuDisabled;
	Script *_globalScript;
	Common::HashMap<Common::String, Scene *> _scenes;
	Common::HashMap<Common::String, Obj *> _objs;
	Common::HashMap<Common::String, Chr *> _chrs;
	Common::HashMap<Common::String, Sound *> _sounds;
	Common::Array<Scene *> _orderedScenes;
	ObjArray _orderedObjs;
	ChrArray _orderedChrs;
	Common::Array<Sound *> _orderedSounds;
	Graphics::MacPatterns *_patterns;
	Scene *_storageScene;
	Chr *_player;
	int _signature;
	//List<MoveListener> moveListeners;

	Common::String *_gameOverMessage;
	Common::String *_saveBeforeQuitMessage;
	Common::String *_saveBeforeCloseMessage;
	Common::String *_revertMessage;

	Common::String _aboutMenuItemName;
	Common::String _commandsMenuName;
	Common::String _commandsMenu;
	Common::String _weaponsMenuName;

	void addScene(Scene *room) {
		if (!room->_name.empty()) {
			Common::String s = room->_name;
			s.toLowercase();
			_scenes[s] = room;
		}
		_orderedScenes.push_back(room);
	}

	void addObj(Obj *obj) {
		Common::String s = obj->_name;
		s.toLowercase();
		_objs[s] = obj;
		obj->_index = _orderedObjs.size();
		_orderedObjs.push_back(obj);
	}

	void addChr(Chr *chr) {
		Common::String s = chr->_name;
		s.toLowercase();
		_chrs[s] = chr;
		chr->_index = _orderedChrs.size();
		_orderedChrs.push_back(chr);
	}

	void addSound(Sound *sound);
};

} // End of namespace Wage

#endif

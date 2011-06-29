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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/globals.h"
#include "tsage/tsage.h"
#include "tsage/blueforce_logic.h"
#include "tsage/ringworld_demo.h"
#include "tsage/ringworld_logic.h"

namespace tSage {

Globals *_globals = NULL;
ResourceManager *_resourceManager = NULL;

/*--------------------------------------------------------------------------*/

/**
 * Instantiates a saved object that can be instantiated
 */
static SavedObject *classFactoryProc(const Common::String &className) {
	if (className == "ObjectMover") return new ObjectMover();
	if (className == "NpcMover") return new NpcMover();
	if (className == "ObjectMover2") return new ObjectMover2();
	if (className == "ObjectMover3") return new ObjectMover3();
	if (className == "PlayerMover") return new PlayerMover();
	if (className == "SceneObjectWrapper") return new SceneObjectWrapper();
	if (className == "PaletteRotation") return new PaletteRotation();
	if (className == "PaletteFader") return new PaletteFader();
	return NULL;
}

/*--------------------------------------------------------------------------*/

Globals::Globals() : _dialogCenter(160, 140), _gfxManagerInstance(_screenSurface),
		_randomSource("tsage"), _unkColor1(0), _unkColor2(255), _unkColor3(255) {
	reset();
	_stripNum = 0;
	_gfxEdgeAdjust = 3;

	if (_vm->getFeatures() & GF_DEMO) {
		_gfxFontNumber = 0;
		_gfxColors.background = 6;
		_gfxColors.foreground = 0;
		_fontColors.background = 255;
		_fontColors.foreground = 6;
		_dialogCenter.y = 80;
		// Workaround in order to use later version of the engine
		_unkColor1 = _gfxColors.foreground;
		_unkColor2 = _gfxColors.foreground;
		_unkColor3 = _gfxColors.foreground;
	} else if ((_vm->getGameID() == GType_Ringworld) &&  (_vm->getFeatures() & GF_CD)) {
		_gfxFontNumber = 50;
		_gfxColors.background = 53;
		_gfxColors.foreground = 0;
		_fontColors.background = 51;
		_fontColors.foreground = 54;
		_unkColor1 = 18;
		_unkColor2 = 18;
		_unkColor3 = 18;
	} else {
		_gfxFontNumber = 50;
		_gfxColors.background = 53;
		_gfxColors.foreground = 18;
		_fontColors.background = 51;
		_fontColors.foreground = 54;
		// Workaround in order to use later version of the engine
		_unkColor1 = _gfxColors.foreground;
		_unkColor2 = _gfxColors.foreground;
		_unkColor3 = _gfxColors.foreground;
	}
	_screenSurface.setScreenSurface();
	_gfxManagers.push_back(&_gfxManagerInstance);

	_sceneObjects = &_sceneObjectsInstance;
	_sceneObjects_queue.push_front(_sceneObjects);

	_prevSceneOffset = Common::Point(-1, -1);
	_sounds.push_back(&_soundHandler);
	_sounds.push_back(&_sequenceManager._soundHandler);

	_scrollFollower = NULL;
	_inventory = NULL;

	switch (_vm->getGameID()) {
	case GType_Ringworld:
		if (!(_vm->getFeatures() & GF_DEMO)) {
			_inventory = new RingworldInvObjectList();
			_game = new RingworldGame();
		} else {
			_game = new RingworldDemoGame();
		}
		break;

	case GType_BlueForce:
		_game = new BlueForceGame();
		break;
	}
}

Globals::~Globals() {
	delete _inventory;
	delete _game;
	_globals = NULL;
}

void Globals::reset() {
	Common::set_to(&_flags[0], &_flags[MAX_FLAGS], false);
	_saver->addFactory(classFactoryProc);
}

void Globals::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);
	assert(_gfxManagers.size() == 1);

	_sceneItems.synchronize(s);
	SYNC_POINTER(_sceneObjects);
	_sceneObjects_queue.synchronize(s);
	s.syncAsSint32LE(_gfxFontNumber);
	s.syncAsSint32LE(_gfxColors.background);
	s.syncAsSint32LE(_gfxColors.foreground);
	s.syncAsSint32LE(_fontColors.background);
	s.syncAsSint32LE(_fontColors.foreground);

	if (s.getVersion() >= 4) {
		s.syncAsByte(_unkColor1);
		s.syncAsByte(_unkColor2);
		s.syncAsByte(_unkColor3);
	}

	s.syncAsSint16LE(_dialogCenter.x); s.syncAsSint16LE(_dialogCenter.y);
	_sounds.synchronize(s);
	for (int i = 0; i < 256; ++i)
		s.syncAsByte(_flags[i]);

	s.syncAsSint16LE(_sceneOffset.x); s.syncAsSint16LE(_sceneOffset.y);
	s.syncAsSint16LE(_prevSceneOffset.x); s.syncAsSint16LE(_prevSceneOffset.y);
	SYNC_POINTER(_scrollFollower);
	s.syncAsSint32LE(_stripNum);
}

void Globals::dispatchSound(ASound *obj) {
	obj->dispatch();
}

void Globals::dispatchSounds() {
	Common::for_each(_sounds.begin(), _sounds.end(), Globals::dispatchSound);
}


} // end of namespace tSage

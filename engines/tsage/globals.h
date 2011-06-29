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

#ifndef TSAGE_GLOBALS_H
#define TSAGE_GLOBALS_H

#include "common/random.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/scenes.h"
#include "tsage/events.h"
#include "tsage/sound.h"
#include "tsage/saveload.h"

namespace tSage {

class Globals : public SavedObject {
private:
	static void dispatchSound(ASound *obj);
public:
	GfxSurface _screenSurface;
	GfxManager _gfxManagerInstance;
	Common::List<GfxManager *> _gfxManagers;
	SceneHandler _sceneHandler;
	Game *_game;
	EventsClass _events;
	SceneManager _sceneManager;
	ScenePalette _scenePalette;
	SceneRegions _sceneRegions;
	SceneItemList _sceneItems;
	SceneObjectList _sceneObjectsInstance;
	SceneObjectList *_sceneObjects;
	SynchronizedList<SceneObjectList *> _sceneObjects_queue;
	SceneText _sceneText;
	int _gfxFontNumber;
	GfxColors _gfxColors;
	GfxColors _fontColors;
	byte _unkColor1, _unkColor2, _unkColor3;
	SoundManager _soundManager;
	Common::Point _dialogCenter;
	WalkRegions _walkRegions;
	SynchronizedList<ASound *> _sounds;
	bool _flags[256];
	Player _player;
	ASound _soundHandler;
	InvObjectList *_inventory;
	Region _paneRegions[2];
	int _paneRefreshFlag[2];
	Common::Point _sceneOffset;
	Common::Point _prevSceneOffset;
	SceneObject *_scrollFollower;
	SequenceManager _sequenceManager;
	Common::RandomSource _randomSource;
	int _stripNum;
	int _gfxEdgeAdjust;
public:
	Globals();
	~Globals();

	void reset();
	void setFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = true;
	}
	void clearFlag(int flagNum) {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		_flags[flagNum] = false;
	}
	bool getFlag(int flagNum) const {
		assert((flagNum >= 0) && (flagNum < MAX_FLAGS));
		return _flags[flagNum];
	}

	GfxManager &gfxManager() { return **_gfxManagers.begin(); }
	virtual Common::String getClassName() { return "Globals"; }
	virtual void synchronize(Serializer &s);
	void dispatchSounds();
};

extern Globals *_globals;

// Note: Currently this can't be part of the _globals structure, since it needs to be constructed
// prior to many of the fields in Globals execute their constructors
extern ResourceManager *_resourceManager;

} // End of namespace tSage

#endif

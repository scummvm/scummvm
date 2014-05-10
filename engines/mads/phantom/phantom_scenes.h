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

#ifndef MADS_PHANTOM_SCENES_H
#define MADS_PHANTOM_SCENES_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/game_phantom.h"
//#include "mads/phantom/globals_phantom.h"


namespace MADS {

namespace Phantom {

enum Noun {
	VERB_LOOK_AT		= 0x0D1
	// TODO
};

class SceneFactory {
public:
	static SceneLogic *createScene(MADSEngine *vm);
};

/**
 * Specialized base class for Dragonsphere game scenes
 */
class PhantomScene : public SceneLogic {
protected:
	PhantomGlobals &_globals;
	GamePhantom &_game;
	MADSAction &_action;

	/**
	 * Forms an animation resource name
	 */
	Common::String formAnimName(char sepChar, int suffixNum);

	/**
	 * Plays appropriate sound for entering varous rooms
	 */
	void lowRoomsEntrySound();
public:
	/**
	 * Constructor
	 */
	PhantomScene(MADSEngine *vm);

	void sub7178C();
};

class SceneInfoPhantom : public SceneInfo {
	friend class SceneInfo;
protected:
	virtual void loadCodes(MSurface &depthSurface, int variant);

	virtual void loadCodes(MSurface &depthSurface, Common::SeekableReadStream *stream);

	/**
	* Constructor
	*/
	SceneInfoPhantom(MADSEngine *vm) : SceneInfo(vm) {}
};

// TODO: Temporary, remove once implemented properly
class Scene1xx : public PhantomScene {
protected:
	/**
	 * Plays an appropriate sound when entering a scene
	 */
	void sceneEntrySound() {}

	/**
	 *Sets the AA file to use for the scene
	 */
	void setAAName() {}

	/**
	 * Updates the prefix used for getting player sprites for the scene
	 */
	void setPlayerSpritesPrefix() {}
public:
	Scene1xx(MADSEngine *vm) : PhantomScene(vm) {}
};

// TODO: Temporary, remove once implemented properly
class DummyScene: public PhantomScene {
public:
	DummyScene(MADSEngine *vm) : PhantomScene(vm) {
		warning("Unimplemented scene");
	}

	virtual void setup() {}
	virtual void enter() {}
	virtual void actions() {}
};

} // End of namespace Phantom

} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES_H */

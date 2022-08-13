/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_GAME_IN_GAME_SCENE_H
#define TETRAEDGE_GAME_IN_GAME_SCENE_H

#include "common/array.h"
#include "common/str.h"
#include "common/hashmap.h"

#include "tetraedge/game/object3d.h"
#include "tetraedge/game/billboard.h"
#include "tetraedge/te/te_scene.h"
#include "tetraedge/te/te_lua_gui.h"

namespace Tetraedge {

class Character;
class TeLayout;

class InGameScene : public TeScene {
public:
	InGameScene();

	struct AnimObject {
		bool onFinished();

		Common::String _name;
		TeSpriteLayout *_layout;
	};

	struct SoundStep {
		Common::String _stepSound1;
		Common::String _stepSound2;
	};

	class AnchorZone {
	};

	class TeMarker {
	};

	void activateAnchorZone(const Common::String &name, bool param_2);
	void addAnchorZone(const Common::String &param_1, const Common::String &param_2, float param_3);
	void addBlockingObject(const Common::String &obj) {
		_blockingObjects.push_back(obj);
	}
	void addCallbackAnimation2D(const Common::String &param_1, const Common::String &param_2, float param_3);
	void addMarker(const Common::String &name, const Common::String &param_2, float param_3, float param_4, const Common::String &param_5, const Common::String &param_6);
	static float angularDistance(float a1, float a2);
	bool aroundAnchorZone(const AnchorZone *zone);
	TeLayout *background();
	void loadBackground(const Common::Path &path);
	void loadInteractions(const Common::Path &path);
	void initScroll();

	void draw();
	Character *character(const Common::String &name);
	bool loadCharacter(const Common::String &name);
	bool loadPlayerCharacter(const Common::String &name);
	bool changeBackground(const Common::String &name);
	void unloadPlayerCharacter(const Common::String &character);
	void unloadCharacter(const Common::String &name);

	void close();
	void reset();
	void freeSceneObjects();
	void unloadSpriteLayouts();
	void deleteAllCallback();

	void setStep(const Common::String &scene, const Common::String &step1, const Common::String &step2);

	void loadBlockers();
	Common::Path getBlockersFileName();

	// Does nothing, but to keep calls from original..
	static void updateScroll() {};

	bool findKate();

	Character *_character;
	Common::Array<Character *> _characters;

	TeLuaGUI &bgGui() { return _bgGui; }

private:
	struct TeBlocker {};
	struct TeRectBlocker {};

	Common::Array<TeBlocker> _blockers;
	Common::Array<TeRectBlocker> _rectBlockers;
	Common::Array<TeMarker *> _markers;
	Common::Array<AnchorZone *> _anchorZones;
	Common::Array<AnimObject *> _animObjects;
	Common::Array<Object3D *> _object3Ds;
	Common::Array<Billboard *> _billboards;
	Common::Array<TeSpriteLayout *> _sprites;

	Common::HashMap<Common::String, SoundStep> _soundSteps;

	TeIntrusivePtr<TeModel> _playerCharacterModel;
	Common::Array<Common::String> _blockingObjects;
	TeLuaGUI _bgGui;
	TeLuaGUI _gui2; // TODO: find a better name.
	TeLuaGUI _gui3; // TODO: find a better name.
	// TODO add private members

	TeVector2f32 _someScrollVector;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_IN_GAME_SCENE_H

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

#ifndef MADS_NEBULAR_SCENES5_H
#define MADS_NEBULAR_SCENES5_H

#include "common/scummsys.h"
#include "mads/core/game.h"
#include "mads/core/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene5xx : public NebularScene {
protected:
	/**
	 * Plays an appropriate sound when entering a scene
	 */
	void setAAName();

	/**
	 * Updates the prefix used for getting player sprites for the scene
	 */
	void setPlayerSpritesPrefix();

	void sceneEntrySound();

public:
	Scene5xx(RexNebularEngine *vm) : NebularScene(vm) {}
};

class Scene501 : public Scene5xx{
private:
	int _mainSequenceId;
	int _mainSpriteId;
	int _doorHotspotid;
	bool _rexPunched;

	void handleSlotActions();

public:
	Scene501(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene502 : public SceneTeleporter {
public:
	Scene502(RexNebularEngine *vm) : SceneTeleporter(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene503 : public Scene5xx{
private:
	int _detonatorHotspotId;

public:
	Scene503(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene504 : public Scene5xx{
private:
	int _carAnimationMode;
	int _carFrame;

public:
	Scene504(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene505 : public Scene5xx{
private:
	int _frame;
	int _nextButtonId;
	int _homeSelectedId;
	int _selectedId;
	int _activeCars;

	int _carLocations[9];

public:
	Scene505(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene506 : public Scene5xx{
private:
	Common::Point _doorPos;
	Facing _heroFacing;

	int _doorDepth;
	int _doorSpriteIdx;
	int _doorSequenceIdx;
	int _doorWord;

	bool _labDoorFl;
	bool _firstDoorFl;
	bool _actionFl;

	void handleDoorSequences();

public:
	Scene506(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene507 : public Scene5xx{
private:
	int _penlightHotspotId;

public:
	Scene507(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene508 : public Scene5xx{
private:
	int _chosenObject;

	void handlePedestral();

public:
	Scene508(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void preActions() override;
	void actions() override;
};

class Scene511 : public Scene5xx{
private:
	bool _handingLine;
	bool _lineMoving;

	int _lineAnimationMode;
	int _lineFrame;
	int _lineAnimationPosition;

public:
	Scene511(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene512 : public Scene5xx{
private:
  int _fishingRodHotspotId;
  int _keyHotspotId;

public:
	Scene512(RexNebularEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene513 : public Scene5xx{
public:
	Scene513(RexNebularEngine *vm) : Scene5xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene515 : public Scene5xx{
public:
	Scene515(RexNebularEngine *vm) : Scene5xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override {};
};

class Scene551 : public Scene5xx{
public:
	Scene551(RexNebularEngine *vm) : Scene5xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};
} // namespace Nebular
} // namespace MADS

#endif /* MADS_NEBULAR_SCENES5_H */

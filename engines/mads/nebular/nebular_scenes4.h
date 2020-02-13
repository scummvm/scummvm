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
 */

#ifndef MADS_NEBULAR_SCENES4_H
#define MADS_NEBULAR_SCENES4_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene4xx : public NebularScene {
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
	Scene4xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene401 : public Scene4xx {
private:
	bool _northFl;
	Common::Point _destPos;
	uint32 _timer;

public:
	Scene401(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene402 : public Scene4xx {
private:
	bool _lightOn;
	bool _blowingSmoke;
	bool _leftWomanMoving;
	bool _rightWomanMoving;
	bool _firstTalkToGirlInChair;
	bool _waitingGinnyMove;
	bool _ginnyLooking;
	bool _bigBeatFl;
	bool _roxOnStool;
	bool _bartenderSteady;
	bool _bartenderHandsHips;
	bool _bartenderLooksLeft;
	bool _bartenderReady;
	bool _bartenderTalking;
	bool _bartenderCalled;
	bool _conversationFl;
	bool _activeTeleporter;
	bool _activeArrows;
	bool _activeArrow1;
	bool _activeArrow2;
	bool _activeArrow3;
	bool _cutSceneReady;
	bool _cutSceneNeeded;
	bool _helgaReady;
	bool _refuseAlienLiquor;

	int _drinkTimer;
	int _beatCounter;
	int _bartenderMode;
	int _bartenderDialogNode;
	int _bartenderCurrentQuestion;
	int _helgaTalkMode;
	int _roxMode;
	int _rexMode;
	int _talkTimer;

	Conversation _dialog1;
	Conversation _dialog2;
	Conversation _dialog3;
	Conversation _dialog4;

	void setDialogNode(int node);
	void handleConversation1();
	void handleConversation2();
	void handleConversation3();
	void handleConversation4();
	void handleDialogs();

public:
	Scene402(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene405 : public Scene4xx {
public:
	Scene405(MADSEngine *vm) : Scene4xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene406 : public Scene4xx {
private:
	bool _hitStorageDoor;

public:
	Scene406(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene407 : public Scene4xx {
private:
	bool _fromNorth;
	Common::Point _destPos;

public:
	Scene407(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene408 : public Scene4xx {
public:
	Scene408(MADSEngine *vm) : Scene4xx(vm) {}

	void setup() override;
	void enter() override;
	void preActions() override;
	void actions() override;
};

class Scene409 : public SceneTeleporter {
public:
	Scene409(MADSEngine *vm) : SceneTeleporter(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene410 : public Scene4xx {
public:
	Scene410(MADSEngine *vm) : Scene4xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene411 : public Scene4xx {
private:
	int _curAnimationFrame;
	int _newIngredient;
	int _newQuantity;
	int _resetFrame;
	int _badThreshold;

	bool _killRox;
	bool _makeMushroomCloud;

	Conversation _dialog1;
	Conversation _dialog2;
	Conversation _dialog3;
	Conversation _dialog4;

	void giveToRex(int object);
	void handleDialog();
	void handleKettleAction();

	int computeQuoteAndQuantity();

	bool addQuantity();
	bool addIngredient();

public:
	Scene411(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene413 : public Scene4xx {
private:
	int _rexDeath;
	int _canMove;

public:
	Scene413(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};
} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES4_H */

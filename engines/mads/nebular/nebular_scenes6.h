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

#ifndef MADS_NEBULAR_SCENES6_H
#define MADS_NEBULAR_SCENES6_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene6xx : public NebularScene {
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
	Scene6xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene601 : public Scene6xx{
public:
	Scene601(MADSEngine *vm) : Scene6xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene602 : public Scene6xx{
private:
	int _lastSpriteIdx;
	int _lastSequenceIdx;
	int _cycleIndex;
	int _safeMode;

	void handleSafeActions();

public:
	Scene602(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene603 : public Scene6xx{
private:
	int _compactCaseHotspotId;
	int _noteHotspotId;

public:
	Scene603(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene604 : public Scene6xx{
private:
	int _timebombHotspotId;
	int _bombMode;
	int _monsterFrame;

	uint32 _monsterTimer;

	bool _monsterActive;
	bool _animationActiveFl;

	void handleBombActions();

public:
	Scene604(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene605 : public Scene6xx{
public:
	Scene605(MADSEngine *vm) : Scene6xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene607 : public Scene6xx{
private:
	uint32 _dogTimer;
	uint32 _lastFrameTime;

	bool _dogLoop;
	bool _dogEatsRex;
	bool _dogBarking;
	bool _shopAvailable;

	int _animationMode;
	int _animationActive;
	int _counter;

	void handleThrowingBone();

public:
	Scene607(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene608 : public Scene6xx{
private:
	int _carMode;
	int _carFrame;
	int _carMoveMode;
	int _dogDeathMode;
	int _carHotspotId;
	int _barkCount;
	int _polycementHotspotId;
	int _animationMode;
	int _nextTrigger;
	int _throwMode;

	bool _resetPositionsFl;
	bool _dogActiveFl;
	bool _dogBarkingFl;
	bool _dogFirstEncounter;
	bool _rexBeingEaten;
	bool _dogHitWindow;
	bool _checkFl;
	bool _dogSquashFl;
	bool _dogSafeFl;
	bool _buttonPressedonTimeFl;
	bool _dogUnderCar;
	bool _dogYelping;

	long _dogWindowTimer;
	long _dogRunTimer;

	uint32 _dogTimer1;
	uint32 _dogTimer2;

	void resetDogVariables();
	void restoreAnimations();
	void setCarAnimations();
	void handleThrowingBone();

public:
	Scene608(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene609 : public Scene6xx{
private:
	int _videoDoorMode;

	void enterStore();

public:
	Scene609(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene610 : public Scene6xx{
private:
	int _handsetHotspotId;
	int _checkVal;

	bool _cellCharging;

	long _cellChargingTimer;
	uint32 _lastFrameTimer;

public:
	Scene610(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene611 : public Scene6xx{
private:
	bool _seenRatFl;
	bool _eyesRunningFl;
	bool _shouldRemoveEyes;
	bool _ratPresentFl;
	bool _duringDialogFl;
	bool _resetBatterieText;
	bool _hermitTalkingFl;
	bool _hermitMovingFl;
	bool _alreadyTalkingFl;
	bool _giveBatteriesFl;
	bool _startTradingFl;
	bool _check1Fl;
	bool _stickFingerFl;

	int _randVal;
	int _ratHotspotId;
	int _hermitDialogNode;
	int _hermitDisplayedQuestion;
	int _nextFrame;
	int _hermitMode;

	uint32 _ratTimer;

	Conversation _dialog1;
	Conversation _dialog2;

	Common::Point _defaultDialogPos;

	void handleTrading();
	void handleRatMoves();
	void handleDialog();
	void handleSubDialog1();
	void handleSubDialog2();
	void handleTalking(int delay);
	void setDialogNode(int node);
	void displayHermitQuestions(int question);

	bool check2ChargedBatteries();
	bool check4ChargedBatteries();

public:
	Scene611(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene612 : public Scene6xx{
private:
	int _actionMode;
	int _cycleIndex;

	void handleWinchMovement();

public:
	Scene612(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene620 : public Scene6xx{
public:
	Scene620(MADSEngine *vm) : Scene6xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override {};
};
} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES6_H */

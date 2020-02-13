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

#ifndef MADS_NEBULAR_SCENES2_H
#define MADS_NEBULAR_SCENES2_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene2xx : public NebularScene {
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
	Scene2xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene201 : public Scene2xx {
private:
	bool _pterodactylFlag;

public:
	Scene201(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene202 : public Scene2xx {
private:
	bool _activeMsgFl, _ladderTopFl, _waitingMeteoFl, _toStationFl, _toTeleportFl;
	int _ladderHotspotId, _lastRoute, _stationCounter, _meteoFrame;
	uint32 _meteoClock1, _meteoClock2, _startTime;
	bool _meteorologistSpecial;

	int subStep1(int randVal);
	int subStep2(int randVal);
	int subStep3(int randVal);
	int subStep4(int randVal);

public:
	Scene202(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;

	void setRandomKernelMessage();
};

class Scene203 : public Scene2xx {
private:
	bool _rhotundaEat2Fl, _rhotundaEatFl;

public:
	Scene203(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene205 : public Scene2xx {
private:
	uint32 _lastFishTime, _chickenTime;
	bool _beingKicked;
	int _kernelMessage;
	Conversation _dialog1;

	void handleWomanSpeech(int quoteId);

public:
	Scene205(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene207 : public Scene2xx {
private:
	bool _vultureFl, _spiderFl, _eyeFl;
	int _spiderHotspotId, _vultureHotspotId;
	int32 _spiderTime, _vultureTime;

	void moveVulture();
	void moveSpider();

public:
	Scene207(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene208 : public Scene2xx {
private:
	bool _rhotundaTurnFl, _boundingFl;
	int32 _rhotundaTime;

	void updateTrap();
	void subAction(int mode);

public:
	Scene208(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene209 : public Scene2xx {
private:
	bool _dodgeFl, _forceDodgeFl, _shouldDodgeFl;
	bool _pitchFl;
	bool _fallFl, _forceFallFl, _shouldFallFl;
	bool _playingAnimFl, _playingDialogFl;
	int _pauseMode, _pauseCounterThreshold, _pauseCounter;
	bool _removeMonkeyFl;
	int _monkeyPosition;
	bool _shootReadyFl, _startShootingInTimerFl, _shootMissedLastFl;
	bool _binocularsDroppedFl;
	int _dialogAbortVal;
	int _counter;

	void handlePause();
	void initPauseCounterThreshold();
	void handlePeek();
	void handleVerticalMove();
	void handleLookStay();
	void handleLookRight();
	void handleBlink();
	void handleGetBinoculars();
	void handleStandFromPeek();
	void handleDodge();
	void handleBinocularBlink();
	void handleBinocularScan();
	void handleJumpInTree();
	void handleTongue();
	void handleMonkeyFall();
	void handleJumpAndHide();
	void handleMonkeyEating();
	void handleMonkey1();
	void handleStandBlink();
	void handleMonkey2();

public:
	Scene209(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene210 : public Scene2xx {
private:
	int _curDialogNode;
	int _nextHandsPlace;
	int _twinkleAnimationType;
	int _twinklesCurrentFrame;
	bool _shouldTalk, _shouldFaceRex, _shouldMoveHead;
	bool _stopWalking;
	bool _twinklesTalking;
	bool _twinklesTalk2;
	int _doorway;
	Common::String _subQuote2;
	Conversation _conv1, _conv2, _conv3;
	Conversation _conv5, _conv6, _conv7, _conv8;

	void handleConversations();
	void handleConversation1();
	void handleConversation2();
	void handleConversation3();
	void handleConversation5();
	void handleConversation6();
	void handleConversation7();
	void handleConversation8();
	void setDialogNode(int node);
	void handleTwinklesSpeech(int quoteId, int shiftX, uint32 delay);
	void newNode(int node);
	void restoreDialogNode(int node, int msgId, int posY);

public:
	Scene210(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene211 : public Scene2xx {
private:
	bool _ambushFl, _wakeFl;
	int _monkeyFrame, _scrollY;
	uint32 _monkeyTime;

public:
	Scene211(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene212 : public Scene2xx {
public:
	Scene212(MADSEngine *vm) : Scene2xx(vm) {}
	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene213 : public SceneTeleporter {
public:
	Scene213(MADSEngine *vm) : SceneTeleporter(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene214 : public Scene2xx {
private:
	uint32 _devilTime;
	bool _devilRunningFl;

public:
	Scene214(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene215 : public Scene2xx {
public:
	Scene215(MADSEngine *vm) : Scene2xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene216 : public Scene2xx {
public:
	Scene216(MADSEngine *vm) : Scene2xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override {};
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES2_H */

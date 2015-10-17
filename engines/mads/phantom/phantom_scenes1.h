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

#ifndef MADS_PHANTOM_SCENES1_H
#define MADS_PHANTOM_SCENES1_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

class Scene1xx : public PhantomScene {
protected:
	/**
	 * Plays an appropriate sound when entering a scene
	 */
	void sceneEntrySound();

	/**
	 *Sets the AA file to use for the scene
	 */
	void setAAName();

	/**
	 * Updates the prefix used for getting player sprites for the scene
	 */
	void setPlayerSpritesPrefix();
public:
	Scene1xx(MADSEngine *vm) : PhantomScene(vm) {}
};

class Scene101 : public Scene1xx {
private:
	int _chanStatus;
	int _wipeStatus;
	int _callingStatus;
	int _chandelierStatus;
	int _callingFrame;
	int _chandelierFrame;
	int _talkCounter;
	int _convCounter;
	int _brieAnimId;
	bool _startWalkingFl;
	bool _startWalking0Fl;
	bool _anim0Running;
	bool _anim1Running;
	bool _startSittingFl;
public:
	Scene101(MADSEngine *vm);
	virtual void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();

	void handleConversation0();
	void handleConversation1();
	void handleAnimation0();
	void handleAnimation1();
};

class Scene102 : public Scene1xx {
private:
	bool _anim0Running;

public:
	Scene102(MADSEngine *vm);
	virtual void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

class Scene103 : public Scene1xx {
private:
	int _jacquesAction;
	int _lastRandom;
	int _standPosition;
	int _hotspotPrompt1;
	int _hotspotPrompt2;
	int _hotspotPrompt3;
	int _hotspotPrompt4;
	int _hotspotPrompt5;
	int _hotspotRightFloor1;
	int _hotspotRightFloor2;
	int _hotspotLeftFloor1;
	int _hotspotLeftFloor2;
	int _hotspotGentleman;
	int _convCount;
	int _lastStairFrame;
	int _lastJacquesFrame;
	int _talkCount;

	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _anim4ActvFl;
	bool _anim5ActvFl;
	bool _anim6ActvFl;
	bool _climbThroughTrapFl;
	bool _guardFrameFl;
	bool _sitFl;

	void adjustRails(int variant);
	void handleJacquesAnim();
	void climbRightStairs();
	void climbLeftStairs();
	void descendRightStairs();
	void descendLeftStairs();
	void process_conv_jacques();

public:
	Scene103(MADSEngine *vm);
	virtual void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

class Scene104 : public Scene1xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _needToGetUp;
	bool _needToStandUp;
	bool _needToTalk;
	bool _sittingUp;
	bool _beforeSheLeaves;
	bool _beforeHeLeaves;

	int _walkStatus;
	int _walkFrame;
	int _coupleStatus;
	int _coupleFrame;
	int _richStatus;
	int _richFrame;
	int _manTalkCount;
	int _womanTalkCount;
	int _lookCount;
	int _richTalkCount;
	int _lastPlayerFrame;

	void cleanInventory();
	void processConversations();
	void handleWalkAnimation();
	void handleCoupleAnimations();
	void handleRichAnimations();
	void handlePlayerWalk();

public:
	Scene104(MADSEngine *vm);
	virtual void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

class Scene105 : public Scene1xx {
private:

public:
	Scene105(MADSEngine *vm);
	virtual void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

} // End of namespace Phantom
} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES1_H */

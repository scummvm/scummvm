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

#ifndef MADS_DRAGON_SCENES1_H
#define MADS_DRAGON_SCENES1_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/dragonsphere/dragonsphere_scenes.h"

namespace MADS {

namespace Dragonsphere {

class Scene1xx : public DragonsphereScene {
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
	Scene1xx(MADSEngine *vm) : DragonsphereScene(vm) {}
};

class Scene101 : public Scene1xx {
public:
	Scene101(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene102 : public Scene1xx {
private:
	int _diaryHotspotIdx1;
	int _diaryHotspotIdx2;
	int _diaryFrame;
	int _animRunning;

public:
	Scene102(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene103 : public Scene1xx {
public:
	Scene103(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene104 : public Scene1xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _anim4ActvFl;
	bool _anim5ActvFl;
	bool _anim6ActvFl;
	bool _activateTimerFl;
	bool _wasBearFl;
	bool _amuletWorksFl;
	bool _pidDrawnSword;
	int _animationRunning;
	int _deathTimer;
	int _deathFrame;
	int _doorwayHotspotId;

	int _kingStatus;
	int _kingFrame;
	int _kingCount;
	int _queenStatus;
	int _queenFrame;
	int _queenCount;
	int _pidStatus;
	int _pidFrame;
	int _pidCount;
	int _macStatus;
	int _macFrame;
	int _macCount;
	int _twinklesStatus;
	int _twinklesFrame;
	int _twinklesCount;
	int _tapestryFrame;

	int32 _clock;

	void handleFinalConversation();
	void handleKingAnimation();
	void handleMacAnimation1();
	void handleMacAnimation2();
	void handleQueenAnimation();
	void handleTwinklesAnimation();
	void handleDeathAnimation();
	void handlePidAnimation();

public:
	Scene104(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene105 : public Scene1xx {
private:
	bool _maidTalkingFl;
	bool _sitUpFl;
	bool _goodNumberFl;

	int _maidFrame;
	int _maidHotspotId1;
	int _maidHotspotId2;
	int _newStatus;
	int _previousStatus;
	int _bucketHotspotId;
	int _boneHotspotId;
	int _gobletHotspotId;

	void setRandomStatus();
	void setRandomWipebrow();
	void handleConversation();

public:
	Scene105(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

} // End of namespace Dragonsphere
} // End of namespace MADS

#endif /* MADS_DRAGON_SCENES1_H */

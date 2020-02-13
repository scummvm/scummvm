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

#ifndef MADS_PHANTOM_SCENES5_H
#define MADS_PHANTOM_SCENES5_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

class Scene5xx : public PhantomScene {
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
	Scene5xx(MADSEngine *vm) : PhantomScene(vm) {}
};

class Scene501 : public Scene5xx {
private:
	bool _anim0ActvFl;
	bool _skipFl;

public:
	Scene501(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene502 : public Scene5xx {
private:
	bool _fire1ActiveFl;
	bool _fire2ActiveFl;
	bool _fire3ActiveFl;
	bool _fire4ActiveFl;
	bool _panelTurningFl;
	bool _trapDoorHotspotEnabled;
	bool _acceleratedFireActivationFl;

	int _panelPushedNum;
	int _puzzlePictures[16];
	int _puzzleSprites[16];
	int _puzzleSequences[16];
	int _messageLevel;
	int _cycleStage;

	Common::Point _nextPos;

	uint32 _lastFrameTime;
	uint32 _timer;
	uint32 _deathTimer;

	byte *_cyclePointer;

	void room_502_initialize_panels();
	void loadCyclingInfo();
	void animateFireBursts();
	void setPaletteCycle();
	void handlePanelAnimation();
	void getPanelInfo(Common::Point *walkToPos, int *panel, Common::Point mousePos, Common::Point *interimPos);

public:
	Scene502(MADSEngine *vm);
	~Scene502() override;

	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene504 : public Scene5xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _anim4ActvFl;
	bool _anim5ActvFl;
	bool _playingMusicFl;
	bool _chairDialogDoneFl;
	bool _fireBreathFl;

	int _songNum;
	int _input3Count;
	int _playCount;
	int _listenStatus;
	int _listenFrame;
	int _chairStatus;
	int _chairFrame;
	int _playStatus;
	int _playFrame;
	int _phantomStatus;
	int _phantomFrame;
	int _christineTalkCount;
	int _deathCounter;

	void handleListenAnimation();
	void handleOrganAnimation();
	void handleChairAnimation();
	void handlePhantomAnimation1();
	void handlePhantomAnimation2();
	void handlePhantomAnimation3();
	void handleListenConversation();
	void handlePlayConversation();
	void handleFightConversation();

public:
	Scene504(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene505 : public Scene5xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _checkFrame106;
	bool _leaveRoomFl;
	bool _partedFl;

	int _raoulStatus;
	int _raoulFrame;
	int _raoulCount;
	int _bothStatus;
	int _bothFrame;
	int _bothCount;
	int _partStatus;
	int _partFrame;
	int _partCount;

	void handleRaoulAnimation();
	void handleBothanimation();
	void handlePartedAnimation();
	void handleCoffinDialog();

public:
	Scene505(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene506 : public Scene5xx {
private:
	bool _anim0ActvFl;
	bool _skipFl;
	bool _ascendingFl;

public:
	Scene506(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

} // End of namespace Phantom
} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES5_H */

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

#ifndef MADS_PHANTOM_SCENES3_H
#define MADS_PHANTOM_SCENES3_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

class Scene3xx : public PhantomScene {
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
	Scene3xx(MADSEngine *vm) : PhantomScene(vm) {}
};

class Scene301 : public Scene3xx {
private:
	bool _anim0ActvFl;
	bool _skip1Fl;
	bool _skip2Fl;

	int _lightingHotspotId;
	int _sandbagHotspotId;

public:
	Scene301(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene302 : public Scene3xx {
public:
	Scene302(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene303 : public Scene3xx {
private:
	bool _anim0ActvFl;
	int _hempHotspotId;
	int _skipFrameCheckFl;
public:
	Scene303(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene304 : public Scene3xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;

	int _raoulFrame;
	int _raoulStatus;
	int _fightFrame;
	int _fightStatus;
	int _fightCount;
	int _phantomFrame;
	int _phantomStatus;

	void handleConversation23();
	void handleRaoulAnimation();
	void handlePhantomAnimation();
	void handleFightAnimation();

public:
	Scene304(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene305 : public Scene3xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _skipFl;
	bool _unmaskFl;

	int _unmaskFrame;

	void handle_animation_unmask();

public:
	Scene305(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene306 : public Scene3xx {
private:
	bool _speechDoneFl;

public:
	Scene306(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene307 : public Scene3xx {
public:
	Scene307(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene308 : public Scene3xx {
private:
	bool _anim0ActvFl;
	bool _anim1ActvFl;
	bool _anim2ActvFl;
	bool _anim3ActvFl;
	bool _skip1Fl;
	bool _skip2Fl;
	int _currentFloor;

public:
	Scene308(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene309 : public Scene3xx {
private:
	bool _anim0ActvFl;

	int _boatStatus;
	int _boatFrame;
	int _talkCount;

	void handleBoatAnimation();

public:
	Scene309(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene310 : public Scene3xx {
private:
	int _raoulMessageColor;
	int _chrisMessageColor;
	int _multiplanePosX[4];
	int _lakeFrame;

	void setMultiplanePos(int x_new);
	void handleLakeAnimation();

public:
	Scene310(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};
} // End of namespace Phantom
} // End of namespace MADS

#endif /* MADS_PHANTOM_SCENES3_H */

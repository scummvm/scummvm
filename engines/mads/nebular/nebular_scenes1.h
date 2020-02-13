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

#ifndef MADS_NEBULAR_SCENES1_H
#define MADS_NEBULAR_SCENES1_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene1xx : public NebularScene {
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
	Scene1xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene101 : public Scene1xx {
private:
	bool _sittingFl;
	bool _panelOpened;

	int _messageNum;
	int _posY;
	int _shieldSpriteIdx;
	int _chairHotspotId;
	int _oldSpecial;

	void sayDang();

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
	bool _fridgeOpenedFl;
	bool _fridgeOpenedDescr;
	bool _fridgeFirstOpenFl;
	bool _chairDescrFl;
	bool _drawerDescrFl;
	bool _activeMsgFl;

	int _fridgeCommentCount;

	void addRandomMessage();

public:
	Scene102(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
	void postActions() override;
};

class Scene103 : public Scene1xx {
private:
	uint32 _updateClock;

public:
	Scene103(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
	void postActions() override;
};

class Scene104 : public Scene1xx {
private:
	bool _kargShootingFl;
	bool _loseFl;

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
	bool _explosionFl;

public:
	Scene105(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene106 : public Scene1xx {
private:
	bool _backToShipFl;
	bool _shadowFl;
	bool _firstEmergingFl;

	int _positionY;

public:
	Scene106(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene107 : public Scene1xx {
private:
	bool _shootingFl;

public:
	Scene107(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene108 : public Scene1xx {
public:
	Scene108(MADSEngine *vm) : Scene1xx(vm) {}

	void setup() override;
	void enter() override;
	void preActions() override;
	void actions() override;
};

class Scene109 : public Scene1xx {
private:
	bool _rexThrowingObject;
	bool _hoovicDifficultFl;
	bool _beforeEatingRex;
	bool _eatingRex;
	bool _hungryFl;
	bool _eatingFirstFish;

	int _throwingObjectId;
	int _hoovicTrigger;

public:
	Scene109(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene110 : public Scene1xx {
private:
	bool _crabsFl;

public:
	Scene110(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene111 : public Scene1xx {
private:
	bool _stampedFl;
	bool _launch1Fl;
	bool _launched2Fl;
	bool _rexDivingFl;

public:
	Scene111(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene112 : public Scene1xx {
public:
	Scene112(MADSEngine *vm) : Scene1xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override {};
};

} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES1_H */

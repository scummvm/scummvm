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

#ifndef MADS_NEBULAR_SCENES7_H
#define MADS_NEBULAR_SCENES7_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene7xx : public NebularScene {
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
	Scene7xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene701 : public Scene7xx {
private:
	int _fishingLineId;

public:
	Scene701(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void preActions();
	virtual void actions();
	virtual void step();
};

class Scene702 : public Scene7xx {
public:
	Scene702(MADSEngine *vm) : Scene7xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void preActions();
	virtual void actions();
};

class Scene703 : public Scene7xx{
private:
	int _monsterMode;
	int _boatFrame;
	int _curSequence;
	int _boatDir;

	bool _useBomb;
	bool _startMonsterTimer;
	bool _rexDeathFl;
	bool _restartTrigger70Fl;

	uint32 _lastFrameTime;
	uint32 _monsterTime;

	Conversation _dialog1;

	void handleBottleInterface();
	void setBottleSequence();
	void handleFillBottle(int quote);

public:
	Scene703(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene704 : public Scene7xx{
private:
	int _bottleHotspotId;
	int _boatCurrentFrame;
	int _animationMode;
	int _boatDirection;

	bool _takeBottleFl;

	Conversation _dialog1;

	void handleFillBottle(int quote);
	void setBottleSequence();
	void handleBottleInterface();

public:
	Scene704(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene705 : public Scene7xx{
private:
	Conversation _dialog1;

	void handleFillBottle(int quote);
	void setBottleSequence();
	void handleBottleInterface();

public:
	Scene705(MADSEngine *vm) : Scene7xx(vm) {}
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene706 : public Scene7xx{
private:
	int _vaseHotspotId;
	int _vaseMode;
	int _animationMode;
	int _animationFrame;

	bool _emptyPedestral;

	void handleTakeVase();
	void handleRexDeath();

public:
	Scene706(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

class Scene707 : public SceneTeleporter {
public:
	Scene707(MADSEngine *vm) : SceneTeleporter(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene710 : public Scene7xx {
public:
	Scene710(MADSEngine *vm) : Scene7xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene711 : public SceneTeleporter {
public:
	Scene711(MADSEngine *vm) : SceneTeleporter(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void actions();
};

class Scene751 : public Scene7xx{
private:
	bool _rexHandingLine;

public:
	Scene751(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

class Scene752 : public Scene7xx {
private:
	int _cardId;

public:
	Scene752(MADSEngine *vm);
	void synchronize(Common::Serializer &s);

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
};

} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES7_H */

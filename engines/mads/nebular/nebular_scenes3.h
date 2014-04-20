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

#ifndef MADS_NEBULAR_SCENES3_H
#define MADS_NEBULAR_SCENES3_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

typedef struct {
	bool _flag;
	int _vertical;
	int _horizontal;
	int _seqId[40];
	uint32 _timer;
} ForceField;

class Scene3xx : public NebularScene {
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

	void initForceField(ForceField *force, bool flag);
	void handleForceField(ForceField *force, int *sprites);
	int computeScale(int low, int high, int id);

public:
	Scene3xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene301: public Scene3xx {
public:
	Scene301(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene302: public Scene3xx {
private:
	int _oldFrame;

public:
	Scene302(MADSEngine *vm) : Scene3xx(vm), _oldFrame(0) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene303: public Scene3xx {
public:
	Scene303(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene304: public Scene3xx {
private:
	int _explosionSpriteId;

public:
	Scene304(MADSEngine *vm) : Scene3xx(vm), _explosionSpriteId(-1) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene307: public Scene3xx {
private:
	ForceField _forceField;

	bool _afterPeeingFl;
	bool _duringPeeingFl;
	bool _grateOpenedFl;
	bool _activePrisonerFl;

	int _animationMode;
	int  _prisonerMessageId;
	int _fieldCollisionCounter;

	uint32 _lastFrameTime;
	uint32 _guardTime;
	uint32 _prisonerTimer;

	Common::String _subQuote2;

	//	dialog1, dialog2;

	void handleDialog();
	void handleRexDialog(int quote);
	void handlePrisonerDialog();
	void handlePrisonerEncounter();
	void setDialogNode(int node);
	void handlePrisonerSpeech(int firstQuoteId, int number, long time);

public:
	Scene307(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions();
	virtual void postActions() {};
};

class Scene308: public Scene3xx {
private:
	ForceField _forceField;

public:
	Scene308(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene309: public Scene3xx {
private:
	ForceField _forceField;
	int _characterSpriteIndexes[3];
	int _messagesIndexes[3];
	int _lastFrame;
	
public:
	Scene309(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene310: public Scene3xx {
private:
	ForceField _forceField;

public:
	Scene310(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions() {};
	virtual void postActions() {};
};

class Scene311: public Scene3xx {
private:
	bool _checkGuardFl;

public:
	Scene311(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions();
	virtual void postActions() {};
};

class Scene313: public Scene3xx {
public:
	Scene313(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step() {};
	virtual void preActions() {};
	virtual void actions();
	virtual void postActions() {};
};

class Scene316: public Scene3xx {
private:
	void handleRexInGrate();
	void handleRoxInGrate();

public:
	Scene316(MADSEngine *vm) : Scene3xx(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions();
	virtual void actions();
	virtual void postActions() {};
};
} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES3_H */

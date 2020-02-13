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

#ifndef MADS_NEBULAR_SCENES8_H
#define MADS_NEBULAR_SCENES8_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene8xx : public NebularScene {
protected:
	/**
	 * Initial setup code shared by several scenes
	 */
	void setPlayerSpritesPrefix();

	/**
	* Initial setup code shared by several scenes
	*/
	void setAAName();

	/**
	 * Common scene enter code used by multiple scenes
	 */
	void sceneEntrySound();
public:
	Scene8xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene801 : public Scene8xx{
private:
	bool _walkThroughDoor;

public:
	Scene801(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene802 : public Scene8xx{
public:
	Scene802(MADSEngine *vm) : Scene8xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene803 : public Scene8xx{
public:
	Scene803(MADSEngine *vm) : Scene8xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene804 : public Scene8xx {
private:
	bool _messWithThrottle;
	bool _movingThrottle;
	bool _throttleGone;
	bool _dontPullThrottleAgain;
	bool _pullThrottleReally;
	bool _alreadyOrgan;
	bool _alreadyPop;
	uint32 _throttleCounter;
	int _resetFrame;

public:
	Scene804(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene805 : public Scene8xx{
public:
	Scene805(MADSEngine *vm) : Scene8xx(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void preActions() override;
	void actions() override;
};

class Scene807 : public SceneTeleporter {
public:
	Scene807(MADSEngine *vm) : SceneTeleporter(vm) {}

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override;
};

class Scene808 : public Scene8xx{
private:
	bool _goingTo803;

public:
	Scene808(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void actions() override;
};

class Scene810 : public Scene8xx{
private:
	bool _moveAllowed;

public:
	Scene810(MADSEngine *vm);
	void synchronize(Common::Serializer &s) override;

	void setup() override;
	void enter() override;
	void step() override;
	void actions() override {};
};

} // End of namespace Nebular
} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES8_H */

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

#ifndef TSAGE_RINGWORLD_DEMO_H
#define TSAGE_RINGWORLD_DEMO_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class RingworldDemoGame: public Game {
private:
	void pauseGame();
public:
	void start() override;
	Scene *createScene(int sceneNumber) override;
	void quitGame() override;
	void processEvent(Event &event) override;
	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;
};

class RingworldDemoScene: public Scene {
public:
	SequenceManager _sequenceManager;
	SceneObject _actor1, _actor2, _actor3;
	SceneObject _actor4, _actor5, _actor6;
	ASound _soundHandler;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void process(Event &event) override;
	void signal() override;
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif

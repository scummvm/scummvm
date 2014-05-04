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

#ifndef MADS_NEBULAR_SCENES5_H
#define MADS_NEBULAR_SCENES5_H

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/scene.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

class Scene5xx : public NebularScene {
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
	Scene5xx(MADSEngine *vm) : NebularScene(vm) {}
};

class Scene502: public SceneTeleporter {
public:
	Scene502(MADSEngine *vm) : SceneTeleporter(vm) {}

	virtual void setup();
	virtual void enter();
	virtual void step();
	virtual void preActions() {};
	virtual void actions();
	virtual void postActions() {};
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_NEBULAR_SCENES5_H */

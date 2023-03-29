/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_LOWLEVELGAMESETUP_H
#define HPL_LOWLEVELGAMESETUP_H

#include "hpl1/engine/ai/AI.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/physics/Physics.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/system/System.h"

namespace hpl {

class LowLevelGameSetup {
public:
	LowLevelGameSetup();
	~LowLevelGameSetup();

	cInput *createInput(cGraphics *graphics);
	cSystem *createSystem();
	cGraphics *createGraphics();
	cResources *createResources(cGraphics *graphics);
	cScene *createScene(cGraphics *graphics, cResources *resources, cSound *sound,
						cPhysics *physics, cSystem *system, cAI *ai);
	cSound *createSound();
	cPhysics *createPhysics();
	cAI *createAi();

private:
	LowLevelSystem *_lowLevelSystem;
	iLowLevelGraphics *_lowLevelGraphics;
	iLowLevelInput *_lowLevelInput;
	LowLevelResources *_lowLevelResources;
	iLowLevelSound *_lowLevelSound;
	iLowLevelPhysics *_lowLevelPhysics;
};

} // namespace hpl

#endif // HPL_LOWLEVELGAMESETUP_H

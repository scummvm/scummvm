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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HPL_SDL_GAMESETUP_H
#define HPL_SDL_GAMESETUP_H

#include "hpl1/engine/game/LowLevelGameSetup.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/haptic/LowLevelHaptic.h"
#include "hpl1/engine/input/LowLevelInput.h"
#include "hpl1/engine/physics/LowLevelPhysics.h"
#include "hpl1/engine/resources/LowLevelResources.h"
#include "hpl1/engine/sound/LowLevelSound.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

class cSDLGameSetup : public iLowLevelGameSetup {
public:
	cSDLGameSetup();
	~cSDLGameSetup();

	cInput *CreateInput(cGraphics *apGraphics);
	cSystem *CreateSystem();
	cGraphics *CreateGraphics();
	cResources *CreateResources(cGraphics *apGraphics);
	cScene *CreateScene(cGraphics *apGraphics, cResources *apResources, cSound *apSound,
						cPhysics *apPhysics, cSystem *apSystem, cAI *apAI, cHaptic *apHaptic);
	cSound *CreateSound();
	cPhysics *CreatePhysics();
	cAI *CreateAI();
	cHaptic *CreateHaptic();

private:
	iLowLevelSystem *mpLowLevelSystem;
	iLowLevelGraphics *mpLowLevelGraphics;
	iLowLevelInput *mpLowLevelInput;
	iLowLevelResources *mpLowLevelResources;
	//		#ifdef WIN32
	//			iLowLevelSound* mpLowLevelSound;
	//		#else
	iLowLevelSound *mpLowLevelSound;
	//		#endif
	iLowLevelPhysics *mpLowLevelPhysics;
	iLowLevelHaptic *mpLowLevelHaptic;
};
};     // namespace hpl
#endif // HPL_LOWLEVELGAMESETUP_SDL_H

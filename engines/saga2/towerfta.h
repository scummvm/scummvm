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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TOWERFTA_H
#define SAGA2_TOWERFTA_H

#include "saga2/tower.h"

namespace Saga2 {

INITIALIZER(initSystemConfig);
// uses null cleanup

INITIALIZER(initPlayIntro);
TERMINATOR(termPlayOutro);

TERMINATOR(termSystemTimer);

TERMINATOR (termAudio);

INITIALIZER(initResourceFiles);
TERMINATOR(termResourceFiles);

INITIALIZER(initResourceServers);
TERMINATOR(termResourceServers);

INITIALIZER(initPathFinders);
TERMINATOR(termPathFinders);

INITIALIZER(initSAGAInterpreter);
TERMINATOR(termSAGAInterpreter);

INITIALIZER(initAudioChannels);
TERMINATOR(termAudioChannels);

INITIALIZER(initResourceHandles);
TERMINATOR(termResourceHandles);

INITIALIZER(initPalettes);
TERMINATOR(termPalettes);

INITIALIZER(initDisplayPort);
TERMINATOR(termDisplayPort);

INITIALIZER(initPanelSystem);
TERMINATOR(termPanelSystem);

INITIALIZER(initMainWindow);
TERMINATOR(termMainWindow);

INITIALIZER(initGUIMessagers);
TERMINATOR(termGUIMessagers);

INITIALIZER(initMousePointer);
TERMINATOR(termMousePointer);

INITIALIZER(initDisplay);
TERMINATOR(termDisplay);

INITIALIZER(initGameMaps);
TERMINATOR(termGameMaps);

INITIALIZER(initRouteData);
TERMINATOR(termRouteData);

INITIALIZER(initActorSprites);
TERMINATOR(termActorSprites);

INITIALIZER(initWeaponData);
TERMINATOR(termWeaponData);

INITIALIZER(initSpellData);
TERMINATOR(termSpellData);

INITIALIZER(initObjectSoundFX);
TERMINATOR(termObjectSoundFX);

INITIALIZER(initObjectPrototypes);
TERMINATOR(termObjectPrototypes);

INITIALIZER(initDynamicGameData);
TERMINATOR(termDynamicGameData);

INITIALIZER(initGameMode);
TERMINATOR(termGameMode);

INITIALIZER(initTop);
TERMINATOR(termTop);

INITIALIZER(initProcessResources);
TERMINATOR(termProcessResources);

} // end of namespace Saga2

#endif

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

#include "hpl1/engine/impl/LowLevelInputSDL.h"
#include "common/events.h"
#include "common/system.h"
#include "hpl1/engine/impl/KeyboardSDL.h"
#include "hpl1/engine/impl/MouseSDL.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//-----------------------------------------------------------------------

cLowLevelInputSDL::cLowLevelInputSDL(iLowLevelGraphics *apLowLevelGraphics) {
	_lowLevelGraphics = apLowLevelGraphics;
	LockInput(true);
}

//-----------------------------------------------------------------------

cLowLevelInputSDL::~cLowLevelInputSDL() {
}

//-----------------------------------------------------------------------

void cLowLevelInputSDL::LockInput(bool abX) {
	g_system->lockMouse(abX);
}

//-----------------------------------------------------------------------

void cLowLevelInputSDL::BeginInputUpdate() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		_events.push_back(event);
	}
}

//-----------------------------------------------------------------------

void cLowLevelInputSDL::EndInputUpdate() {
	_events.clear();
}

//-----------------------------------------------------------------------

iMouse *cLowLevelInputSDL::CreateMouse() {
	return hplNew(cMouseSDL, (this, _lowLevelGraphics));
}

//-----------------------------------------------------------------------

iKeyboard *cLowLevelInputSDL::CreateKeyboard() {
	return hplNew(cKeyboardSDL, (this));
}

//-----------------------------------------------------------------------

} // namespace hpl

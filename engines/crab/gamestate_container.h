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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

//=============================================================================
// Author:   Arvind
// Purpose:  The game state virtual class
//=============================================================================
#pragma once

#include "common_header.h"
#include "gamestates.h"

//------------------------------------------------------------------------
// Purpose: Game State base class
//------------------------------------------------------------------------
class GameState {
public:
	virtual void HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) = 0;
	virtual void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID) = 0;
	virtual void Draw() = 0;
	virtual void SetUI() = 0;
	virtual void AutoSave() = 0;
	virtual ~GameState(){};
};

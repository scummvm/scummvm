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

#ifndef CRAB_GAMESTATES_H
#define CRAB_GAMESTATES_H
#include "common_header.h"

//------------------------------------------------------------------------
// Purpose: Game State Enumerators
//------------------------------------------------------------------------
enum GameStateID {
	GAMESTATE_NULL = -2,
	GAMESTATE_TITLE = -1,
	GAMESTATE_EXIT = -3,
	GAMESTATE_MAIN_MENU = 0,
	GAMESTATE_NEW_GAME,
	GAMESTATE_LOAD_GAME,
	TOTAL_GAMESTATES // ALWAYS THE LAST VALUE
};

#endif // CRAB_GAMESTATES_H

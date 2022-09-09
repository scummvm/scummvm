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

#ifndef WATCHMAKER_DO_PLAYER_H
#define WATCHMAKER_DO_PLAYER_H

#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

void SetCurPlayerPosTo_9x(GameVars &gameVars, Init &init);
void doPlayer(WGame &game);
void ChangePlayer(WGame &game, uint8 oc);
void ProcessCharacters(WGame &game);
void UpdatePlayerStand(WGame &game, uint8 oc);
int PlayerCanSwitch(GameVars &gameVars, char no_inventory);
int PlayerCanCall(GameVars &gameVars);
int PlayerCanSave();
int IsPlayerInPool();

} // End of namespace Watchmaker

#endif // WATCHMAKER_DO_PLAYER_H

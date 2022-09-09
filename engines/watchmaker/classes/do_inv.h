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

#ifndef WATCHMAKER_DO_INV_H
#define WATCHMAKER_DO_INV_H

#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

void AddIcon(Init &init, uint8 icon);
void doInventory(WGame &game);
void doUseWith(WGame &game);
void KillIcon(Init &init, uint8 icon);
uint8 IconInInv(Init &init, uint8 icon);
void ReplaceIcon(Init &init, uint8 oldicon, uint8 newicon);
void ClearUseWith();
void InventoryUp();
void InventoryDown();
uint8 WhatIcon(Renderer &renderer, int16 invmx, int16 invmy);
void SelectCurBigIcon(uint8 icon);
} // End of namespace Watchmaker

#endif // WATCHMAKER_DO_INV_H

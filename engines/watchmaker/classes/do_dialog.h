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

#ifndef WATCHMAKER_DO_DIALOG_H
#define WATCHMAKER_DO_DIALOG_H

#include "watchmaker/types.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

extern int16 NextDlg;

void doDialog(WGame &game);
void PaintDialog(WGame &game);
void UpdateDialogMenu(WGame &game, int16 dmx, int16 dmy, uint8 db);
int32 GetAlternateAnim(Init &init, int32 an);
uint8 GetAlternateCamera(Init &init, uint8 cam);
uint8 GetAlternatePosition(Init &init, uint8 pos);

} // End of namespace Watchmaker

#endif // WATCHMAKER_DO_DIALOG_H

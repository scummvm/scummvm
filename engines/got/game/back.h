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

#ifndef GOT_GAME_BACK_H
#define GOT_GAME_BACK_H

#include "got/data/defines.h"

namespace Got {

extern const char *OBJECT_NAMES[];
extern const char *ITEM_NAMES[];

/**
 * Shows a new level
 * In the original this did some initial actors setup and rendering.
 * Now in ScummVM, it only does the setup portions, since the
 * GameContent view takes care of the scene rendering.
*/
extern void showLevel(int newLevel);
extern void showLevelDone();
extern void odinSpeaks(int index, int item, const char *endMessage = nullptr);
extern int switchIcons();
extern int rotateArrows();
extern void killEnemies(int iy, int ix);
extern void removeObjects(int y, int x);
extern void placeTile(int x, int y, int tile);
extern int backgroundTile(int x, int y);
extern void selectItem();
extern void actorSpeaks(const Actor *actor, int index, int item);

} // namespace Got

#endif

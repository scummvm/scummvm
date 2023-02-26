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

#ifndef AGS_ENGINE_MAIN_GAME_RUN_H
#define AGS_ENGINE_MAIN_GAME_RUN_H

#include "ags/shared/ac/keycode.h"

namespace AGS3 {

namespace AGS {
namespace Engine {
class IDriverDependantBitmap;
}
}
using namespace AGS::Engine; // FIXME later

// Loops game frames until certain event takes place (for blocking actions)
void GameLoopUntilValueIsZero(const int8 *value);
void GameLoopUntilValueIsZero(const short *value);
void GameLoopUntilValueIsZero(const int *value);
void GameLoopUntilValueIsZeroOrLess(const short *move);
void GameLoopUntilValueIsNegative(const short *value);
void GameLoopUntilValueIsNegative(const int *value);
void GameLoopUntilNotMoving(const short *move);
void GameLoopUntilNoOverlay();
void GameLoopUntilButAnimEnd(int guin, int objn);

// Run the actual game until it ends, or aborted by player/error; loops GameTick() internally
void RunGameUntilAborted();
// Update everything game related
void UpdateGameOnce(bool checkControls = false, IDriverDependantBitmap *extraBitmap = nullptr, int extraX = 0, int extraY = 0);
// Update minimal required game state: audio, loop counter, etc
void UpdateGameAudioOnly();
// Gets current logical game FPS, this is normally a fixed number set in script;
// in case of "maxed fps" mode this function returns real measured FPS.
float get_current_fps();
// Runs service key controls, returns false if no key was pressed or key input was claimed by the engine,
// otherwise returns true and provides a keycode.
bool run_service_key_controls(KeyInput &kgn);
// Runs service mouse controls, returns false if mouse input was claimed by the engine,
// otherwise returns true and provides mouse button code.
bool run_service_mb_controls(eAGSMouseButton &mbut, int &mwheelz);
// Polls few things (exit flag and debugger messages)
// TODO: refactor this
void update_polled_stuff();

} // namespace AGS3

#endif

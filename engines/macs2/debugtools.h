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

#ifndef MACS2_DEBUGTOOLS_H
#define MACS2_DEBUGTOOLS_H

namespace Macs2 {
void onImGuiInit();
void onImGuiRender();
void onImGuiCleanup();
bool shouldDrawPathfindingOverlay();
bool shouldDrawDebugOutputOverlay();
bool shouldDrawPathOverlay();
// Returns true if the script debugger wants the executor to pause before the next opcode
bool scriptDebuggerShouldPause();
// Direct access to debugger state for tick() resume logic
extern bool _scriptDebugPaused;
extern bool _scriptDebugStepRequested;
} // namespace Macs2

#endif // MACS2_DEBUGTOOLS_H

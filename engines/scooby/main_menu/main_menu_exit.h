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

#ifndef SCOOBY_MAIN_MENU_EXIT_H
#define SCOOBY_MAIN_MENU_EXIT_H

// Describes the three control-flow outcomes that can leave the main-menu owner.

namespace Scooby {
enum class MainMenuExit {
	// The selected path hands control to room initialization.
	ContinueToRoom,

	// The selected path requests a complete restart from the cartridge entry sequence.
	RestartRequested,

	// The native host requested process shutdown during a recovered wait.
	HostClosed
};
} // namespace Scooby

#endif // SCOOBY_MAIN_MENU_EXIT_H

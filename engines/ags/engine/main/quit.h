/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_MAIN_QUIT_H
#define AGS_ENGINE_MAIN_QUIT_H

enum QuitReason {
	kQuitKind_NormalExit        = 0x01,
	kQuitKind_DeliberateAbort   = 0x02,
	kQuitKind_GameException     = 0x04,
	kQuitKind_EngineException   = 0x08,

	// user closed the window or script command QuitGame was executed
	kQuit_GameRequest   = kQuitKind_NormalExit | 0x10,

	// user pressed abort game key
	kQuit_UserAbort     = kQuitKind_DeliberateAbort | 0x20,

	// script command AbortGame was executed
	kQuit_ScriptAbort   = kQuitKind_GameException | 0x10,
	// game logic has generated a warning and warnings are treated as error
	kQuit_GameWarning   = kQuitKind_GameException | 0x20,
	// game logic has generated an error (often script error)
	kQuit_GameError     = kQuitKind_GameException | 0x30,

	// any kind of a fatal engine error
	kQuit_FatalError    = kQuitKind_EngineException
};

#endif

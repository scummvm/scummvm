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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Original code from EcWin7 - Copyright (C) 2010 Emanuele Colombo
 * https://code.google.com/p/dukto/
 */

#include "common/scummsys.h"

#include "common/textconsole.h"

#include "backends/taskbar/win32/win32-taskbar.h"

Win32TaskbarManager::Win32TaskbarManager() {
}

Win32TaskbarManager::~Win32TaskbarManager() {
}

void Win32TaskbarManager::setOverlayIcon(const Common::String &name, const Common::String &description) {
	warning("[Win32TaskbarManager::setOverlayIcon] Not implemented");
}

void Win32TaskbarManager::setProgressValue(int val, int max) {
	warning("[Win32TaskbarManager::setProgressValue] Not implemented");
}

void Win32TaskbarManager::setProgressState(TaskbarProgressState state) {
	warning("[Win32TaskbarManager::setProgressState] Not implemented");
}

void Win32TaskbarManager::addRecent(const Common::String &name, const Common::String &description) {
	warning("[Win32TaskbarManager::addRecent] Not implemented");
}
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
 */

#if defined(UNIX)

#include "backends/taskbar/unity/unity-taskbar.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/file.h"

UnityTaskbarManager::UnityTaskbarManager() {
}

UnityTaskbarManager::~UnityTaskbarManager() {
}

void UnityTaskbarManager::setOverlayIcon(Common::String name, Common::String description) {
	warning("[UnityTaskbarManager::setOverlayIcon] Not implemented");
}

void UnityTaskbarManager::setProgressValue(int completed, int total) {
	warning("[UnityTaskbarManager::setProgressValue] Not implemented");
}

void UnityTaskbarManager::setProgressState(TaskbarProgressState state) {
	warning("[UnityTaskbarManager::setProgressState] Not implemented");
}

void UnityTaskbarManager::addRecent(Common::String name, Common::String description) {
	warning("[UnityTaskbarManager::addRecent] Not implemented");
}

#endif

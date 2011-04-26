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
 * $URL$
 * $Id$
 */

#ifndef COMMON_TASKBAR_MANAGER_H
#define COMMON_TASKBAR_MANAGER_H

#include "common/str.h"

namespace Common {

class TaskbarManager {
public:
	/**
	 * 	Values representing the taskbar progress state
	 */
	enum TaskbarProgressState {
		NoProgress = 0,
		Indeterminate = 1,
		Normal = 2,
		Error = 4,
		Paused = 8
	};

	TaskbarManager() {}
	virtual ~TaskbarManager() {}

	/**
	 * Sets an overlay icon on the taskbar icon.
	 *
	 * When an empty name is given, no icon is shown
	 * and the current overlay icon (if any) is removed
	 *
	 * @param	name        Path to the icon
	 * @param	description The description
	 *
	 * @note on Windows, the icon should be an ICO file
	 */
	 virtual void setOverlayIcon(const String &name, const String &description) {}

    /**
     * Sets a progress value on the taskbar icon
     *
     * @param	val The current progress value
     * @param	max The maximum progress value
     */
    virtual void setProgressValue(int val, int max) {}

    /**
     * Sets the progress state on the taskbar icon
     *
     * 	State can be any of the following:
	 * 	 - NoProgress: disable display of progress state
	 *   - Indeterminate
	 *   - Normal
	 *   - Error
	 *   - Paused
     *
     * @param	state	The progress state
     */
     virtual void setProgressState(TaskbarProgressState state) {}

	 /**
	  * Adds an engine to the recent items list
	  *
	  * Path is automatically set to the current executable path,
	  * an icon name is generated (with fallback to default icon)
	  * and the command line is set to start the engine on click.
	  *
	  * @param	name	   	The target name.
	  * @param	description	The description.
	  */
	 virtual void addRecent(const String &name, const String &description) {}
};

}	// End of namespace Common

#endif // COMMON_TASKBAR_MANAGER_H

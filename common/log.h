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

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include "common/scummsys.h"

namespace LogMessageType {
/**
 * Enumeration for log message types.
 * @ingroup common_system
 *
 */
enum Type {
	kInfo,    /**< Info logs. */
	kError,   /**< Error logs. */
	kWarning, /**< Warning logs. */
	kDebug    /**< Debug logs. */
};

} // End of namespace LogMessageType

namespace Common {

/**
 * A callback that is invoked by debug, warning and error methods.
 *
 * A typical example would be a function that shows a debug
 * console and displays the given message in it.
 */
typedef void (*LogWatcher)(LogMessageType::Type type, int level, uint32 debugChannels, const char *message);

/**
 * Set the watcher used by debug, error and warning methods.
 */
void setLogWatcher(LogWatcher f);

/**
 * Get the watcher used by debug, error and warning methods.
 */
LogWatcher getLogWatcher();

} // namespace Common

#endif

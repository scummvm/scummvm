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

#ifndef GUI_ERROR_H
#define GUI_ERROR_H

#include "common/error.h"

namespace GUI {

/**
 * Displays an error dialog for some error code.
 *
 * @param error error code
 * @param extraText extra text to be displayed in addition to default string description(optional)
 */
void displayErrorDialog(const Common::Error &error, const Common::U32String &extraText = Common::U32String());

/**
 * Displays an error dialog for a given message.
 *
 * @param text message to be displayed
 */
void displayErrorDialog(const Common::U32String &text);

} // End of namespace GUI

#endif //GUI_ERROR_H

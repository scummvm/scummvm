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
#ifndef SLUDGE_PLATEFORM_DEPENDENT_H
#define SLUDGE_PLATEFORM_DEPENDENT_H
/* These are the functions which have different versions for
 * the different operating systems.
 */
#include <stdint.h>

namespace Sludge {

char *grabFileName();
int showSetupWindow();

void msgBox(const char *head, const char *msg);
int msgBoxQuestion(const char *head, const char *msg);

void changeToUserDir();
uint32_t launch(char *filename);

bool defaultUserFullScreen();

} // End of namespace Sludge

#endif

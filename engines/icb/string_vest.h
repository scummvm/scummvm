/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_STRING_VEST_H
#define ICB_STRING_VEST_H

#include "engines/icb/common/px_common.h"
#include "engines/icb/debug.h"

namespace ICB {

#define ENGINE_STRING_LEN 256

// This is the maximum length of a complete URL (clustering might remove the need for this).
#define MAXLEN_URL ENGINE_STRING_LEN

// This is the maximum length of a complete URL to a cluster.
#define MAXLEN_CLUSTER_URL 128

// The maximum length of an icon name.
#define MAXLEN_ICON_NAME 32

// Maximum length of an event name.
#define MAXLEN_EVENT_NAME 16

// Maximum length of a game object name.
#define MAXLEN_OBJECT_NAME 64

// Maximum length of a name given to an icon list (e.g. "inventory") is one.
#define MAXLEN_ICON_LIST_NAME 20

// Don't know what this one is used for.
#define TINY_NAME_LEN 32

// stop initing these in multiple routines and use this instead
extern char temp_buf[ENGINE_STRING_LEN];

void Set_string_and_len(const char *from, char *to, uint32 *length);
void Set_string(const char *from, char *to);
void Set_string(const char *from, char *to, uint32 length);

} // End of namespace ICB

#endif // #ifndef   __STRING_VEST_H

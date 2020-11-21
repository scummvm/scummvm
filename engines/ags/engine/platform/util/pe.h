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

#ifndef __AGS_EE_PLATFORM_UTIL_PE_H
#define __AGS_EE_PLATFORM_UTIL_PE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char version[15];
  char description[100];
  char internal_name[100];
} version_info_t;

int getVersionInformation(char* filename, version_info_t* version_info);

#ifdef __cplusplus
}
#endif

#endif // __AGS_EE_PLATFORM_UTIL_PE_H

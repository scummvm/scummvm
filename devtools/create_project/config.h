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
 *
 */

#ifndef TOOLS_CREATE_PROJECT_CONFIG_H
#define TOOLS_CREATE_PROJECT_CONFIG_H

#define PROJECT_DESCRIPTION "Residual"    // Used in console output and build configuration
#define PROJECT_NAME "residual"           // Used for folders, icons, resources and project/solution name
#define LIBS_DEFINE "RESIDUAL_LIBS"       // Name of the include environment variable
#define REVISION_DEFINE "RESIDUAL_INTERNAL_REVISION"

#define HAS_VIDEO_FOLDER 0
#define ADDITIONAL_LIBRARY "glu32"
#define NEEDS_RTTI 1

#endif // TOOLS_CREATE_PROJECT_CONFIG_H

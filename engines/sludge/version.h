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
#define MAJOR_VERSION 2
#define MINOR_VERSION 2
#define RELEASE_VERSION 1
#define BUILD_VERSION 208
#define TEXT_VERSION "2.2.1"
#define WHOLE_VERSION (MAJOR_VERSION * 256 + MINOR_VERSION) // This version
#define MINIM_VERSION (1             * 256 + 2)             // Earliest version of games the engine can run

#define COPYRIGHT_TEXT "\251 Hungry Software and contributors 2000-2014"

#define VERSION(a,b)    (a * 256 + b)

namespace Sludge {

extern int gameVersion;

} // End of namespace Sludge

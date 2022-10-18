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

/*************************************
 * 
 * Xobject to play PACo video files
 *
 * USED IN:
 * Hell Cab, Journeyman Mac, Iron Helix Mac
 *
 *************************************/

#ifndef DIRECTOR_LINGO_XLIBS_XPLAYANIMXOBJ_H
#define DIRECTOR_LINGO_XLIBS_XPLAYANIMXOBJ_H

namespace Director {

namespace XPlayAnim {

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

void b_xplayanim(int nargs);

} // End of namespace XPlayAnim

} // End of namespace Director

#endif

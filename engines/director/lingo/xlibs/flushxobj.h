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

#ifndef DIRECTOR_LINGO_XOBJECT_FLUSHXOBJ_H
#define DIRECTOR_LINGO_XOBJECT_FLUSHXOBJ_H

namespace Director {

class FlushXObject : public Object<FlushXObject> {
public:
	FlushXObject(ObjectType objType);
};

namespace FlushXObj {

void initialize(int type);

void m_new(int nargs);
void m_clearMask(int nargs);
void m_addToMask(int nargs);
void m_flush(int nargs);
void m_flushEvents(int nargs);

} // End of namespace FlushXObj

} // End of namespace Director

#endif

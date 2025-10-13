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

#ifndef DIRECTOR_LINGO_XLIBS_EVENTQ_H
#define DIRECTOR_LINGO_XLIBS_EVENTQ_H

namespace Director {
class EventQXObject : public Object<EventQXObject> {
public:
	EventQXObject(ObjectType objType);
};

namespace EventQXObj {

extern const char *const xlibNames[];
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_bufferEvents(int nargs);
void m_flushEvents(int nargs);
void m_postEvents(int nargs);
void m_bufferStatus(int nargs);
void m_getNextEvent(int nargs);

} // End of namespace EventQXObj

} // End of namespace Director

#endif

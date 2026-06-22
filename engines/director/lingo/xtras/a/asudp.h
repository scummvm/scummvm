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

#ifndef DIRECTOR_LINGO_XTRAS_A_ASUDP_H
#define DIRECTOR_LINGO_XTRAS_A_ASUDP_H

namespace Director {

class AsudpXtraObject : public Object<AsudpXtraObject> {
public:
	AsudpXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AsudpXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_free(int nargs);
void m_getLocalIP(int nargs);
void m_getLocalIPs(int nargs);
void m_startListen(int nargs);
void m_stopListen(int nargs);
void m_bytesReceived(int nargs);
void m_getBytes(int nargs);
void m_getBytesWithSenderInfo(int nargs);
void m_sendBytes(int nargs);
void m_setSendingPort(int nargs);
void m_getError(int nargs);
void m_"(int nargs);

} // End of namespace AsudpXtra

} // End of namespace Director

#endif

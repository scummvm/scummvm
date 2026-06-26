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

#ifndef DIRECTOR_LINGO_XTRAS_D_DIRECTMEDIA_H
#define DIRECTOR_LINGO_XTRAS_D_DIRECTMEDIA_H

namespace Director {

class DirectMediaXtraObject : public Object<DirectMediaXtraObject> {
public:
	DirectMediaXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace DirectMediaXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_videoplay(int nargs);
void m_videopause(int nargs);
void m_videoseek(int nargs);
void m_videoplaysegment(int nargs);
void m_setvolume(int nargs);
void m_getvolume(int nargs);
void m_setbalance(int nargs);
void m_getbalance(int nargs);
void m_isPastCuePoint(int nargs);
void m_mostRecentCuePoint(int nargs);
void m_isDirectShowInstalled(int nargs);

} // End of namespace DirectMediaXtra

} // End of namespace Director

#endif

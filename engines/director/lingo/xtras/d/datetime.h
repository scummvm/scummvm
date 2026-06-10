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

#ifndef DIRECTOR_LINGO_XTRAS_D_DATETIME_H
#define DIRECTOR_LINGO_XTRAS_D_DATETIME_H

namespace Director {

class DateTimeXtraObject : public Object<DateTimeXtraObject> {
public:
	DateTimeXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace DateTimeXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_GetSeconds(int nargs);
void m_GetMinutes(int nargs);
void m_GetHours(int nargs);
void m_GetDay(int nargs);
void m_GetMonth(int nargs);
void m_GetYear(int nargs);
void m_GetWeekDay(int nargs);

} // End of namespace DateTimeXtra

} // End of namespace Director

#endif

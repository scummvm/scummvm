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

#ifndef DIRECTOR_LINGO_XLIBS_W_WIDGET_H
#define DIRECTOR_LINGO_XLIBS_W_WIDGET_H

namespace Director {

class WidgetXObject : public Object<WidgetXObject> {
public:
	WidgetXObject(ObjectType objType);

	Common::HashMap<Common::String, Common::Array<byte>> _lipSyncData;
};

namespace WidgetXObj {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_name(int nargs);
void m_status(int nargs);
void m_error(int nargs);
void m_lastError(int nargs);
void m_add(int nargs);
void m_askQuit(int nargs);
void m_first(int nargs);
void m_lipSync(int nargs);
void m_loadData(int nargs);
void m_saveData(int nargs);
void m_hideCursor(int nargs);
void m_showCursor(int nargs);
void m_mul(int nargs);
void m_globals(int nargs);
void m_symbols(int nargs);
void m_sendPerform(int nargs);
void m_factory(int nargs);

void m_getPro(int nargs);

} // End of namespace WidgetXObj

} // End of namespace Director

#endif

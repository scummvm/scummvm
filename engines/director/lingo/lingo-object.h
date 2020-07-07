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

#ifndef DIRECTOR_LINGO_OBJECT_H
#define DIRECTOR_LINGO_OBJECT_H

namespace Director {

struct Object {
	Common::String *name;
	ObjectType type;
	bool disposed;

	DatumHash properties;
	int *refCount;
	ScriptContext *ctx;
	int inheritanceLevel; // 1 for original object

	// used only for factories
	Common::HashMap<uint32, Datum> *objArray;

	Object(const Common::String &objName, ObjectType objType, ScriptContext *objCtx = nullptr);
	Object(const Object &obj);
	virtual ~Object();

	virtual Object *clone();
	Symbol getMethod(const Common::String &methodName);
	bool hasProp(const Common::String &propName);
	Datum getProp(const Common::String &propName);
	bool setProp(const Common::String &propName, const Datum &value);
};

namespace LM {
	// predefined methods
	void m_describe(int nargs);
	void m_dispose(int nargs);
	void m_get(int nargs);
	void m_instanceRespondsTo(int nargs);
	void m_messageList(int nargs);
	void m_name(int nargs);
	void m_new(int nargs);
	void m_perform(int nargs);
	void m_put(int nargs);
	void m_respondsTo(int nargs);

} // End of namespace LM

} // End of namespace Director

#endif

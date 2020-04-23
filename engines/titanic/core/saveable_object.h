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

#ifndef TITANIC_SAVEABLE_OBJECT_H
#define TITANIC_SAVEABLE_OBJECT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "titanic/support/simple_file.h"

namespace Titanic {

class CSaveableObject;

class ClassDef {
public:
	const char *_className;
	ClassDef *_parent;
public:
	ClassDef(const char *className, ClassDef *parent) :
		_className(className), _parent(parent) {}
	virtual ~ClassDef() {}
	virtual CSaveableObject *create();
};

template<typename T>
class TypeTemplate : public ClassDef {
public:
	TypeTemplate(const char *className, ClassDef *parent) :
		ClassDef(className, parent) {}
	CSaveableObject *create() override { return new T(); }
};

#define CLASSDEF \
	static ClassDef *_type; \
	virtual ClassDef *getType() const override { return _type; }

class CSaveableObject {
	typedef CSaveableObject *(*CreateFunction)();
private:
	typedef Common::List<ClassDef *> ClassDefList;
	typedef Common::HashMap<Common::String, CreateFunction> ClassListMap;
	static ClassDefList *_classDefs;
	static ClassListMap *_classList;
public:
	/**
	 * Sets up the list of saveable object classes
	 */
	static void initClassList();

	/**
	 * Free the list of saveable object classes
	 */
	static void freeClassList();

	/**
	 * Creates a new instance of a saveable object class
	 */
	static CSaveableObject *createInstance(const Common::String &name);
public:
	static ClassDef *_type; \
	virtual ClassDef *getType() const { return _type; }

	virtual ~CSaveableObject() {}

	bool isInstanceOf(const ClassDef *classDef) const;

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Write out a header definition for the class to file
	 * prior to saving the actual data for the class
	 */
	virtual void saveHeader(SimpleFile *file, int indent);

	/**
	 * Writes out a footer for the class after it's data has
	 * been written to file
	 */
	virtual void saveFooter(SimpleFile *file, int indent);
};

} // End of namespace Titanic

#endif /* TITANIC_SAVEABLE_OBJECT_H */

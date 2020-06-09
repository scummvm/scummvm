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

#include "gui/filebrowser-dialog.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xobject/fileio.h"

namespace Director {

static struct MethodProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	{ "mDelete",				FileIO::m_delete,			 0, 0, kXObj,				2 },	// D2
	{ "mDispose",				FileIO::m_dispose,			 0, 0, kXObj,				2 },	// D2
	{ "mFileName",				FileIO::m_fileName,			 0, 0, kXObj,				2 },	// D2
	{ "mGetLength",				FileIO::m_getLength,		 0, 0, kXObj,				2 },	// D2
	{ "mGetPosition",			FileIO::m_getPosition,		 0, 0, kXObj,				2 },	// D2
	{ "mNew",					FileIO::m_new,				 2, 2, kXObj,				2 },	// D2
	{ "mReadChar",				FileIO::m_readChar,			 0, 0, kXObj,				2 },	// D2
	{ "mReadLine",				FileIO::m_readLine,			 0, 0, kXObj,				2 },	// D2
	{ "mReadToken",				FileIO::m_readToken,		 2, 2, kXObj,				2 },	// D2
	{ "mReadWord",				FileIO::m_readWord,			 0, 0, kXObj,				2 },	// D2
	{ "mSetPosition",			FileIO::m_setPosition,		 1, 1, kXObj,				2 },	// D2
	{ "mWriteChar",				FileIO::m_writeChar,		 1, 1, kXObj,				2 },	// D2
	{ "mWriteString",			FileIO::m_writeString,		 1, 1, kXObj,				2 },	// D2
	{ 0, 0, 0, 0, 0, 0 }
};

void FileIO::b_openXLib(int nargs) {
	const Common::String name = "FileIO";

	if (g_lingo->_globalvars.contains(name)) {
		warning("FileIO already initialized");
		return;
	}

	FileXObject *obj = new FileXObject();

	for (MethodProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > g_lingo->_vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = FBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.methodType = mtd->type;
		sym.u.bltin = mtd->func;
		obj->methods[mtd->name] = sym;
	}

	g_lingo->_globalvars[name] = Symbol();
	g_lingo->_globalvars[name].name = new Common::String(name);
	g_lingo->_globalvars[name].global = true;
	g_lingo->_globalvars[name].type = OBJECT;
	g_lingo->_globalvars[name].u.obj = obj;
}

// Initialization/disposal

Object *FileXObject::clone() {
	FileXObject *res = new FileXObject();
	res->disposed = disposed;
	res->prototype = this;
	res->properties = properties;
	res->methods = methods;
	res->inheritanceLevel = inheritanceLevel + 1;
	return res;
}

void FileIO::m_new(int nargs) {
	FileXObject *me = static_cast<FileXObject *>(g_lingo->_currentMeObj);

	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String option = *d1.u.s;
	Common::String filename = *d2.u.s;

	if (option.hasPrefix("?")) {
		option = option.substr(1);
		GUI::FileBrowserDialog browser(0, filename.c_str(), option.equalsIgnoreCase("read") ? GUI::kFBModeLoad : GUI::kFBModeSave);
		if (browser.runModal() <= 0) {
			g_lingo->push(Datum(kErrorFileNotFound));
			return;
		}
		filename = browser.getResult();
	}

	if (option.equalsIgnoreCase("read")) {
		me->infile = g_system->getSavefileManager()->openForLoading(filename);
		if (!me->infile) {
			g_lingo->push(Datum(kErrorIO));
			return;
		}
	} else if (option.equalsIgnoreCase("write")) {
		me->outfile = g_system->getSavefileManager()->openForSaving(filename);
		if (!me->outfile) {
			g_lingo->push(Datum(kErrorIO));
			return;
		}
	} else if (option.equalsIgnoreCase("append")) {
		warning("FileIO: append is unimplemented");
	} else {
		error("Unsupported FileIO option: '%s'", option.c_str());
	}

	Datum res;
	res.type = OBJECT;
	res.u.obj = me;
	g_lingo->push(res);
}

void FileIO::m_dispose(int nargs) {
	FileXObject *me = static_cast<FileXObject *>(g_lingo->_currentMeObj);

	me->disposed = true;

	if (me->infile) {
		delete me->infile;
		me->infile = nullptr;
	}
	if (me->outfile) {
		me->outfile->finalize();
		delete me->outfile;
		me->outfile = nullptr;
	}
}

// Read

void FileIO::m_readChar(int nargs) {
	g_lingo->printSTUBWithArglist("m_readChar", 0);
	g_lingo->push(Datum(0));
}

void FileIO::m_readLine(int nargs) {
	FileXObject *me = static_cast<FileXObject *>(g_lingo->_currentMeObj);

	if (me->infile) {
		Common::String line = me->infile->readLine();
		g_lingo->push(Datum(line));
	} else {
		g_lingo->push(Datum(""));
	}
}

void FileIO::m_readWord(int nargs) {
	g_lingo->printSTUBWithArglist("m_readWord", 0);
	g_lingo->push(Datum(""));
}

void FileIO::m_readToken(int nargs) {
	g_lingo->printSTUBWithArglist("m_readToken", 2);
	g_lingo->push(Datum(""));
}

// Write

void FileIO::m_writeChar(int nargs) {
	g_lingo->printSTUBWithArglist("m_writeChar", 1);
	g_lingo->push(Datum(0));
}

void FileIO::m_writeString(int nargs) {
	FileXObject *me = static_cast<FileXObject *>(g_lingo->_currentMeObj);
	Datum d = g_lingo->pop();

	if (me->outfile) {
		me->outfile->writeString(d.asString());
		g_lingo->push(Datum(kErrorNone));
	} else {
		g_lingo->push(Datum(kErrorReadOnly));
	}
}

// Other

void FileIO::m_getPosition(int nargs) {
	g_lingo->printSTUBWithArglist("m_getPosition", 0);
	g_lingo->push(Datum(0));
}

void FileIO::m_setPosition(int nargs) {
	g_lingo->printSTUBWithArglist("m_setPosition", 1);
}

void FileIO::m_getLength(int nargs) {
	g_lingo->printSTUBWithArglist("m_getLength", 0);
	g_lingo->push(Datum(0));
}

void FileIO::m_fileName(int nargs) {
	g_lingo->printSTUBWithArglist("m_fileName", 0);
	g_lingo->push(Datum(""));
}

void FileIO::m_delete(int nargs) {
	g_lingo->printSTUBWithArglist("m_delete", 0);
}

} // End of namespace Director

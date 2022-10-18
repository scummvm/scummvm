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

/*
 * Use the SerialPort XObject to send and receive data over the Macintosh’s
 * two standard serial ports (commonly called the modem and printer ports).
 * This XObject is built into Macromedia Director, so you don’t have to open
 * an XLibrary to use it.
 *
 * Reference: Director 4 Using Lingo, pages 315-320
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/serialportxobj.h"

namespace Director {

const char *SerialPortXObj::xlibName = "SerialPort";
const char *SerialPortXObj::fileNames[] = {
	"SerialPort",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",				SerialPortXObj::m_new,			 1, 1,	200 },	// D2
	{ "GetPortNum",			SerialPortXObj::m_getPortNum,	 0, 0,	200 },	// D2
	{ "WriteString",		SerialPortXObj::m_writeString,	 1, 1,	200 },	// D2
	{ "WriteChar",			SerialPortXObj::m_writeChar,	 1, 1,	200 },	// D2
	{ "ReadString",			SerialPortXObj::m_readString,	 0, 0,  200 },	// D2
	{ "ReadChar",			SerialPortXObj::m_readChar,		 0, 0,  200 },	// D2
	{ "ReadCount",			SerialPortXObj::m_readCount,	 0, 0,  200 },	// D2
	{ "ReadFlush",			SerialPortXObj::m_readFlush,	 0, 0,  200 },	// D2
	{ "ConfigChan",			SerialPortXObj::m_configChan,	 2, 2,  200 },	// D2
	{ "HShakeChan",			SerialPortXObj::m_hShakeChan,	 3, 3,  200 },	// D2
	{ "SetUp",				SerialPortXObj::m_setUp,		 3, 3,  200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void SerialPortXObj::open(int type) {
	if (type == kXObj) {
		SerialPortXObject::initMethods(xlibMethods);
		SerialPortXObject *xobj = new SerialPortXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void SerialPortXObj::close(int type) {
	if (type == kXObj) {
		SerialPortXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


SerialPortXObject::SerialPortXObject(ObjectType ObjectType) :Object<SerialPortXObject>("SerialPort") {
	_objType = ObjectType;
}

void SerialPortXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_currentMe);
}

void SerialPortXObj::m_getPortNum(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_getPortNum", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_writeString(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_writeString", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_writeChar(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_writeChar", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_readString(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_readString", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_readChar(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_readChar", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_readCount(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_readCount", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_readFlush(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_readFlush", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_configChan(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_configChan", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_hShakeChan(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_hShakeChan", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SerialPortXObj::m_setUp(int nargs) {
	g_lingo->printSTUBWithArglist("SerialPortXObj::m_setUp", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}



} // End of namespace Director

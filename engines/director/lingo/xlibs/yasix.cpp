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

/*************************************
 *
 * USED IN:
 * Yearn2Learn: The Flintstones Coloring Book
 *
 *************************************/

/*
 * v0.2, ©1992 Clark Brady, 317/839-0442 - YASIX - Yet Another System Information XFCN
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/yasix.h"

namespace Director {

const char *const Yasix::xlibName = "YASIX";
const XlibFileDesc Yasix::fileNames[] = {
	{ "YASIX", nullptr },
	{ nullptr, nullptr },
};

static const BuiltinProto builtins[] = {
	{ "YASIX", Yasix::m_yasix, 0, 0, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void Yasix::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void Yasix::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void Yasix::m_yasix(int nargs) {
	g_lingo->push(Datum("Machine_Type   Macintosh\r\n\
Keyboard_Type ExtISOADBKbd\r\n\
Processor_Type 68030\r\n\
MMU_Type NoMMU\r\n\
FPU_Type NoFPU\r\n\
Physical_RAM 16 Meg\r\n\
System_Version 7.6.1\r\n\
Screen_0 0,0,640,480 Color 32 bit\r\n\
Volume_1 ScummVM 500M 500M"));
}

} // End of namespace Director

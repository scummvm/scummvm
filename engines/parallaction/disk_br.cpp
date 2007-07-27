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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "parallaction/parallaction.h"


namespace Parallaction {


void DosDisk_br::errorFileNotFound(const char *s) {
	error("File '%s' not found", s);
}

Common::String DosDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDisk_br::selectArchive");
	return "";
}

void DosDisk_br::setLanguage(uint16 language) {
	debugC(5, kDebugDisk, "DosDisk_br::setLanguage");

	return;
}

DosDisk_br::DosDisk_br(Parallaction* vm) : _vm(vm) {

}

DosDisk_br::~DosDisk_br() {
}

Cnv* DosDisk_br::loadTalk(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTalk");

	return 0;
}

Script* DosDisk_br::loadLocation(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadLocation");
	return 0;
}

Script* DosDisk_br::loadScript(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScript");
	return 0;
}

//	there are no Head resources in Big Red Adventure
StaticCnv* DosDisk_br::loadHead(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadHead");
	return 0;
}


StaticCnv* DosDisk_br::loadPointer() {
	debugC(5, kDebugDisk, "DosDisk_br::loadPointer");
	return 0;
}


Font* DosDisk_br::loadFont(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFont");
	return 0;
}


Cnv* DosDisk_br::loadObjects(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadObjects");
	return 0;
}


StaticCnv* DosDisk_br::loadStatic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadStatic");
	return 0;
}

Cnv* DosDisk_br::loadFrames(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFrames");
	return 0;
}

//	there are no Slide resources in Big Red Adventure
void DosDisk_br::loadSlide(const char *filename) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSlide");
	return;
}

void DosDisk_br::loadScenery(const char *name, const char *mask) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScenery");
	return;
}

Table* DosDisk_br::loadTable(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTable");
	return 0;
}

Common::ReadStream* DosDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadMusic");
	return 0;
}


Common::ReadStream* DosDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSound");
	return 0;
}


} // namespace Parallaction

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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
 */

#include "common/winexe_ne.h"
#include "common/winexe_pe.h"
#include "graphics/wincursor.h"

#include "buried/buried.h"
#include "buried/database.h"

namespace Buried {

DatabaseNE::DatabaseNE() {
	_exe = new Common::NEResources();
}

DatabaseNE::~DatabaseNE() {
	delete _exe;
}

bool DatabaseNE::load(const Common::String &fileName) {
	return _exe->loadFromEXE(fileName);
}

void DatabaseNE::close() {
	_exe->clear();
}

Common::String DatabaseNE::loadString(uint32 stringID) {
	bool continueReading = true;
	Common::String result;

	while (continueReading) {
		Common::String string = _exe->loadString(stringID);

		if (string.empty())
			return "";

		if (string[0] == '!') {
			string.deleteChar(0);
			stringID++;
		} else {
			continueReading = false;
		}

		result += string;
	}

	// Change any \r to \n
	for (uint32 i = 0; i < result.size(); i++)
		if (result[i] == '\r')
			result.setChar('\n', i);

	return result;
}

Common::SeekableReadStream *DatabaseNE::getBitmapStream(uint32 bitmapID) {
	return _exe->getResource(Common::kNEBitmap, bitmapID);
}

Graphics::WinCursorGroup *DatabaseNE::getCursorGroup(uint32 cursorGroupID) {
	return Graphics::WinCursorGroup::createCursorGroup(*_exe, cursorGroupID);
}

Common::SeekableReadStream *DatabaseNE::getResourceStream(const Common::String &resourceType, uint32 resourceID) {
	return _exe->getResource(resourceType, resourceID);
}

uint32 DatabaseNE::getVersion() {
	Common::NEResources::VersionInfo versionInfo = _exe->getVersionInfo();
	return MAKEVERSION(versionInfo.fileVersion[0], versionInfo.fileVersion[1], versionInfo.fileVersion[2], versionInfo.fileVersion[3]);
}

bool DatabaseNECompressed::load(const Common::String &fileName) {
	return _exe->loadFromCompressedEXE(fileName);
}

DatabasePE::DatabasePE() {
	_exe = new Common::PEResources();
}

DatabasePE::~DatabasePE() {
	delete _exe;
}

bool DatabasePE::load(const Common::String &fileName) {
	return _exe->loadFromEXE(fileName);
}

void DatabasePE::close() {
	_exe->clear();
}

Common::String DatabasePE::loadString(uint32 stringID) {
	bool continueReading = true;
	Common::String result;

	while (continueReading) {
		Common::String string = _exe->loadString(stringID);

		if (string.empty())
			return "";

		if (string[0] == '!') {
			string.deleteChar(0);
			stringID++;
		} else {
			continueReading = false;
		}

		result += string;
	}

	// Change any \r to \n
	for (uint32 i = 0; i < result.size(); i++)
		if (result[i] == '\r')
			result.setChar('\n', i);

	return result;
}

Common::SeekableReadStream *DatabasePE::getBitmapStream(uint32 bitmapID) {
	return _exe->getResource(Common::kPEBitmap, bitmapID);
}

Graphics::WinCursorGroup *DatabasePE::getCursorGroup(uint32 cursorGroupID) {
	return Graphics::WinCursorGroup::createCursorGroup(*_exe, cursorGroupID);
}

Common::SeekableReadStream *DatabasePE::getResourceStream(const Common::String &resourceType, uint32 resourceID) {
	return _exe->getResource(resourceType, resourceID);
}

uint32 DatabasePE::getVersion() {
	// Not really needed, it should only be 1.1
	return MAKEVERSION(1, 1, 0, 0);
}

} // End of namespace Buried

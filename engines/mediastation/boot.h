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

#ifndef MEDIASTATION_BOOT_H
#define MEDIASTATION_BOOT_H

#include "common/path.h"
#include "common/str.h"
#include "common/array.h"
#include "common/hashmap.h"

#include "mediastation/datafile.h"

namespace MediaStation {

enum ContextDeclarationSectionType {
	kContextDeclarationEmptySection = 0x0000,
	kContextDeclarationPlaceholder = 0x0003,
	kContextDeclarationContextId = 0x0004,
	kContextDeclarationStreamId = 0x0005,
	kContextDeclarationParentContextId = 0x0006,
	kContextDeclarationName = 0x0bb8
};

class ContextDeclaration {
public:
	ContextDeclaration(Chunk &chunk);
	ContextDeclaration() {};

	uint _contextId = 0;
	uint _streamId = 0;
	Common::String _name;
	Common::Array<uint> _parentContextIds;

private:
	ContextDeclarationSectionType getSectionType(Chunk &chunk);
};

enum ScreenDeclarationSectionType {
	kScreenDeclarationEmpty = 0x0000,
	kScreenDeclarationAssetId = 0x0009,
	kScreenDeclarationScreenId = 0x0004
};

class ScreenDeclaration {
public:
	ScreenDeclaration(Chunk &chunk);
	ScreenDeclaration() {};

	uint _assetId = 0;
	uint _screenId = 0;

private:
	ScreenDeclarationSectionType getSectionType(Chunk &chunk);
};

enum FileDeclarationSectionType {
	kFileDeclarationEmptySection = 0x0000,
	kFileDeclarationFileId = 0x002b,
	kFileDeclarationFileNameAndType = 0x002d
};

// Indicates where a file is intended to be stored.
// NOTE: This might not be correct and this might be a more general "file type".
enum IntendedFileLocation {
	kFileLocationEmpty = 0x0000,
	// Usually all files that have numbers remain on the CD-ROM.
	kFileIntendedOnCdRom = 0x0007,
	// These UNKs only appear in George Shrinks.
	kFileIntendedForUnk1 = 0x0008,
	kFileIntendedForUnk2 = 0x0009,
	// Usually only INSTALL.CXT is copied to the hard disk.
	kFileIntendedOnHardDisk = 0x000b
};

class FileDeclaration {
public:
	FileDeclaration(Chunk &chunk);
	FileDeclaration() {};

	uint _id = 0;
	IntendedFileLocation _intendedLocation = kFileLocationEmpty;
	Common::String _name;

private:
	FileDeclarationSectionType getSectionType(Chunk &chunk);
};

enum SubfileDeclarationSectionType {
	kSubfileDeclarationEmptySection = 0x0000,
	kSubfileDeclarationAssetId = 0x002a,
	kSubfileDeclarationFileId = 0x002b,
	kSubfileDeclarationStartOffset = 0x002c
};

class SubfileDeclaration {
public:
	SubfileDeclaration(Chunk &chunk);
	SubfileDeclaration() {};

	uint _assetId = 0;
	uint _fileId = 0;
	uint _startOffsetInFile = 0;

private:
	SubfileDeclarationSectionType getSectionType(Chunk &chunk);
};

// Declares a cursor, which is stored as a cursor resource in the game executable.
class CursorDeclaration {
public:
	CursorDeclaration(Chunk &chunk);
	CursorDeclaration() {};

	uint _id = 0;
	uint _unk = 0;
	Common::String _name;
};

class EngineResourceDeclaration {
public:
	EngineResourceDeclaration(Common::String resourceName, int resourceId) : _name(resourceName), _id(resourceId) {};
	EngineResourceDeclaration() {};

	Common::String _name;
	int _id = 0;
};

enum BootSectionType {
	kBootLastSection = 0x0000,
	kBootEmptySection = 0x002e,
	kBootContextDeclaration = 0x0002,
	kBootVersionInformation = 0x0190,
	kBootUnk1 = 0x0191,
	kBootUnk2 = 0x0192,
	kBootUnk3 = 0x0193,
	kBootEngineResource = 0x0bba,
	kBootEngineResourceId = 0x0bbb,
	kBootScreenDeclaration = 0x0007,
	kBootFileDeclaration = 0x000a,
	kBootSubfileDeclaration = 0x000b,
	kBootUnk5 = 0x000c,
	kBootCursorDeclaration = 0x0015,
	kBootEntryScreen = 0x002f,
	kBootAllowMultipleSounds = 0x0035,
	kBootAllowMultipleStreams = 0x0036,
	kBootUnk4 = 0x057b
};

class Boot : Datafile {
private:
	BootSectionType getSectionType(Chunk &chunk);

public:
	Common::String _gameTitle;
	VersionInfo _versionInfo;
	Common::String _engineInfo;
	Common::String _sourceString;
	Common::HashMap<uint32, ContextDeclaration> _contextDeclarations;
	Common::HashMap<uint32, ScreenDeclaration> _screenDeclarations;
	Common::HashMap<uint32, FileDeclaration> _fileDeclarations;
	Common::HashMap<uint32, SubfileDeclaration> _subfileDeclarations;
	Common::HashMap<uint32, CursorDeclaration> _cursorDeclarations;
	Common::HashMap<uint32, EngineResourceDeclaration> _engineResourceDeclarations;

	uint32 _entryContextId = 0;
	bool _allowMultipleSounds = false;
	bool _allowMultipleStreams = false;

	Boot(const Common::Path &path);
	~Boot();
};

} // End of namespace MediaStation

#endif

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

#include "mediastation/datafile.h"
#include "mediastation/subfile.h"

namespace MediaStation {

// Contains information about the engine (also called
//  "title compiler") used in this particular game.
// Engine version information is not present in early games.
class VersionInfo {
public:
	VersionInfo(Chunk &chunk);
	~VersionInfo();

	// The version number of this engine,
	// in the form 4.0r8 (major . minor r revision).
	uint32 _majorVersion;
	uint32 _minorVersion;
	uint32 _revision;

	// A textual description of this engine.
	// Example: "Title Compiler T4.0r8 built Feb 13 1998 10:16:52"
	//           ^^^^^^^^^^^^^^  ^^^^^
	//           | Engine name   | Version number
	Common::String *string;
};

enum ContextDeclarationSectionType {
	kContextDeclarationEmptySection = 0x0000,
	kContextDeclarationPlaceholder = 0x0003,
	kContextDeclarationFileNumber1 = 0x0004,
	kContextDeclarationFileNumber2 = 0x0005,
	kContextDeclarationFileReference = 0x0006,
	kContextDeclarationName = 0x0bb8
};

class ContextDeclaration {
public:
	ContextDeclaration(Chunk &chunk);
	~ContextDeclaration();

	Common::Array<uint32> _fileReferences;
	uint32 _fileNumber;
	Common::String *_contextName;
	// Signal that there are no more declarations to read.
	bool _isLast;

private:
	ContextDeclarationSectionType getSectionType(Chunk &chunk);
};

enum UnknownDeclarationSectionType {
	kUnknownDeclarationEmptySection = 0x0000,
	kUnknownDeclarationUnk1 = 0x0009,
	kUnknownDeclarationUnk2 = 0x0004
};

class UnknownDeclaration {
public:
	uint16 _unk;
	// Signal that there are no more declarations to read.
	bool _isLast;

	UnknownDeclaration(Chunk &chunk);

private:
	UnknownDeclarationSectionType getSectionType(Chunk& chunk);
};

enum FileDeclarationSectionType {
	kFileDeclarationEmptySection = 0x0000,
	kFileDeclarationFileId = 0x002b,
	kFileDeclarationFileNameAndType = 0x002d
};

// Indicates where a file is intended to be stored.
// NOTE: This might not be correct and this might be a more general "file type".
enum IntendedFileLocation {
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
	~FileDeclaration();

	uint32 _id;
	IntendedFileLocation _intendedLocation;
	Common::String *_name;
	// Signal that there are no more declarations to read.
	bool _isLast;

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

	uint16 _assetId;
	uint16 _fileId;
	uint32 _startOffsetInFile;
	// Signal that there are no more context declarations to read.
	bool _isLast;

private:
	SubfileDeclarationSectionType getSectionType(Chunk &chunk);
};

// Declares a cursor, which is stored as a cursor resource in the game executable.
class CursorDeclaration {
public:
	CursorDeclaration(Chunk &chunk);
	~CursorDeclaration();

	uint16 _id;
	uint16 _unk;
	Common::String *_name;
};

class EngineResourceDeclaration {
public:
	Common::String *_resourceName;
	int _resourceId;

	EngineResourceDeclaration(Common::String *resourceName, int resourceId);
	~EngineResourceDeclaration();
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
	kBootUnknownDeclaration = 0x0007,
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
	Subfile subfile;

	BootSectionType getSectionType(Chunk &chunk);

public:
	Common::String *_gameTitle = nullptr;
	VersionInfo *_versionInfo = nullptr;
	Common::String *_sourceString = nullptr;
	Common::HashMap<uint32, ContextDeclaration *> _contextDeclarations;
	Common::Array<UnknownDeclaration *> _unknownDeclarations;
	Common::HashMap<uint32, FileDeclaration *> _fileDeclarations;
	Common::HashMap<uint32, SubfileDeclaration *> _subfileDeclarations;
	Common::HashMap<uint32, CursorDeclaration *> _cursorDeclarations;
	Common::HashMap<uint32, EngineResourceDeclaration *> _engineResourceDeclarations;

	uint32 _entryContextId = 0;
	bool _allowMultipleSounds = false;
	bool _allowMultipleStreams = false;

	Boot(const Common::Path &path);
	~Boot();

	uint32 getRootContextId();
};

} // End of namespace MediaStation

#endif
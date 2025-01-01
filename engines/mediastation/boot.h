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

#include "mediastation/datafile.h"
#include "mediastation/subfile.h"

#ifndef MEDIASTATION_BOOT_H
#define MEDIASTATION_BOOT_H

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
	uint32 majorVersion;
	uint32 minorVersion;
	uint32 revision;

	// A textual description of this engine.
	// Example: "Title Compiler T4.0r8 built Feb 13 1998 10:16:52"
	//           ^^^^^^^^^^^^^^  ^^^^^
	//           | Engine name   | Version number
	Common::String *string;
};

class ContextDeclaration {
public:
	enum class SectionType {
		EMPTY = 0x0000,
		PLACEHOLDER = 0x0003,
		FILE_NUMBER_1 = 0x0004,
		FILE_NUMBER_2 = 0x0005,
		FILE_REFERENCE = 0x0006,
		CONTEXT_NAME = 0x0bb8
	};

	ContextDeclaration(Chunk &chunk);
	~ContextDeclaration();

	Common::Array<uint32> fileReferences;
	uint32 fileNumber;
	Common::String *contextName;
	// Signal that there are no more declarations to read.
	bool isLast;

private:
	ContextDeclaration::SectionType getSectionType(Chunk &chunk);
};

class UnknownDeclaration {
public:
	enum class SectionType {
		EMPTY = 0x0000,
		UNK_1 = 0x0009,
		UNK_2 = 0x0004
	};

	uint16 _unk;
	// Signal that there are no more declarations to read.
	bool _isLast;

	UnknownDeclaration(Chunk &chunk);

private:
	UnknownDeclaration::SectionType getSectionType(Chunk& chunk);
};

class FileDeclaration {
public:
	enum class SectionType {
		EMPTY = 0x0000,
		FILE_ID = 0x002b,
		FILE_NAME_AND_TYPE = 0x002d
	};

	// Indicates where this file is intended to be stored.
	// NOTE: This might not be correct and this might be a more general "file type".
	enum class IntendedLocation {
		// Usually all files that have numbers remain on the CD-ROM.
		CD_ROM = 0x0007,
		// These UNKs only appear in George Shrinks.
		UNK1 = 0x0008,
		UNK2 = 0x0009,
		// Usually only INSTALL.CXT is copied to the hard disk.
		HARD_DISK = 0x000b
	};

	FileDeclaration(Chunk &chunk);
	~FileDeclaration();

	uint32 _id;
	IntendedLocation _intendedLocation;
	Common::String *_name;
	// Signal that there are no more declarations to read.
	bool _isLast;

private:
	FileDeclaration::SectionType getSectionType(Chunk &chunk);
};

class SubfileDeclaration {
public:
	enum class SectionType {
		EMPTY = 0x0000,
		ASSET_ID = 0x002a,
		FILE_ID = 0x002b,
		START_OFFSET = 0x002c
	};

	SubfileDeclaration(Chunk &chunk);

	uint16 _assetId;
	uint16 _fileId;
	uint32 _startOffsetInFile;
	// Signal that there are no more context declarations to read.
	bool _isLast;

private:
	SubfileDeclaration::SectionType getSectionType(Chunk &chunk);
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

class Boot : Datafile {
private:
	enum class SectionType {
		LAST = 0x0000,
		EMPTY = 0x002e,
		CONTEXT_DECLARATION = 0x0002,
		VERSION_INFORMATION = 0x0190,
		UNK1 = 0x0191,
		UNK2 = 0x0192,
		UNK3 = 0x0193,
		ENGINE_RESOURCE = 0x0bba,
		ENGINE_RESOURCE_ID = 0x0bbb,
		UNKNOWN_DECLARATION = 0x0007,
		FILE_DECLARATION = 0x000a,
		SUBFILE_DECLARATION = 0x000b,
		UNK5 = 0x000c,
		CURSOR_DECLARATION = 0x0015,
		ENTRY_SCREEN = 0x002f,
		ALLOW_MULTIPLE_SOUNDS = 0x0035,
		ALLOW_MULTIPLE_STREAMS = 0x0036,
		UNK4 = 0x057b
	};

	Subfile subfile;

	Boot::SectionType getSectionType(Chunk &chunk);

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
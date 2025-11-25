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

enum ContextReferenceSectionType {
	kContextReferencePlaceholder = 0x0003,
	kContextReferenceContextId = 0x0004,
	kContextReferenceStreamId = 0x0005,
	kContextReferenceParentContextId = 0x0006,
	kContextReferenceName = 0x0bb8
};

class ContextReference {
public:
	ContextReference(Chunk &chunk);
	ContextReference() {};

	uint _contextId = 0;
	uint _streamId = 0;
	Common::String _name;
	Common::Array<uint> _parentContextIds;

private:
	ContextReferenceSectionType getSectionType(Chunk &chunk);
};

enum ScreenReferenceSectionType {
	kScreenReferenceScreenId = 0x0009,
	kScreenReferenceContextId = 0x0004
};

class ScreenReference {
public:
	ScreenReference(Chunk &chunk);
	ScreenReference() {};

	uint _screenActorId = 0;
	uint _contextId = 0;

private:
	ScreenReferenceSectionType getSectionType(Chunk &chunk);
};

enum FileInfoSectionType {
	kFileInfoEmptySection = 0x0000,
	kFileInfoFileId = 0x002b,
	kFileInfoFileNameAndType = 0x002d
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

class FileInfo {
public:
	FileInfo(Chunk &chunk);
	FileInfo() {};

	uint _id = 0;
	IntendedFileLocation _intendedLocation = kFileLocationEmpty;
	Common::String _name;

private:
	FileInfoSectionType getSectionType(Chunk &chunk);
};

enum StreamInfoSectionType {
	kStreamInfoEmptySection = 0x0000,
	kStreamInfoActorId = 0x002a,
	kStreamInfoFileId = 0x002b,
	kStreamInfoStartOffset = 0x002c
};

class StreamInfo {
public:
	StreamInfo(Chunk &chunk);
	StreamInfo() {};

	uint _actorId = 0;
	uint _fileId = 0;
	uint _startOffsetInFile = 0;

private:
	StreamInfoSectionType getSectionType(Chunk &chunk);
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
	kBootContextReference = 0x0002,
	kBootVersionInformation = 0x0190,
	kBootUnk1 = 0x0191,
	kBootFunctionTableSize = 0x0192,
	kBootUnk3 = 0x0193,
	kBootEngineResource = 0x0bba,
	kBootEngineResourceId = 0x0bbb,
	kBootScreenReference = 0x0007,
	kBootFileInfo = 0x000a,
	kBootStreamInfo = 0x000b,
};

} // End of namespace MediaStation

#endif

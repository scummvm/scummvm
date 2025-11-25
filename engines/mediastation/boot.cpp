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

#include "mediastation/boot.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

#pragma region ContextReference
ContextReference::ContextReference(Chunk &chunk) {
	// Read the file number.
	ContextReferenceSectionType sectionType = getSectionType(chunk);
	if (kContextReferenceContextId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_contextId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kContextReferenceStreamId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_streamId = chunk.readTypedUint16();

	// Read the context name. Only some titles have context names,
	// and unfortunately we can't determine which just by relying
	// on the title compiler version number.
	sectionType = getSectionType(chunk);
	if (kContextReferenceName == sectionType) {
		_name = chunk.readTypedString();
		sectionType = getSectionType(chunk);
	}

	// Read the parent context IDs. We don't know how many file
	// references there are beforehand, so we'll just read until
	// we get something else.
	uint rewindOffset = chunk.pos();
	while (kContextReferenceParentContextId == sectionType) {
		int fileReference = chunk.readTypedUint16();
		_parentContextIds.push_back(fileReference);
		rewindOffset = chunk.pos();
		sectionType = getSectionType(chunk);
	}
	chunk.seek(rewindOffset);
}

ContextReferenceSectionType ContextReference::getSectionType(Chunk &chunk) {
	return static_cast<ContextReferenceSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region ScreenReference
ScreenReference::ScreenReference(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	ScreenReferenceSectionType sectionType = getSectionType(chunk);
	if (kScreenReferenceScreenId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_screenActorId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kScreenReferenceContextId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_contextId = chunk.readTypedUint16();
}

ScreenReferenceSectionType ScreenReference::getSectionType(Chunk &chunk) {
	return static_cast<ScreenReferenceSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region FileInfo
FileInfo::FileInfo(Chunk &chunk) {
	// Read the file ID.
	FileInfoSectionType sectionType = getSectionType(chunk);
	if (kFileInfoFileId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_id = chunk.readTypedUint16();

	// Read the intended file location.
	sectionType = getSectionType(chunk);
	if (kFileInfoFileNameAndType != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_intendedLocation = static_cast<IntendedFileLocation>(chunk.readTypedUint16());

	// Since the platforms that Media Station originally targeted were case-insensitive,
	// the case of these filenames might not match the case of the files actually in
	// the directory. All files should be matched case-insensitively.
	_name = chunk.readTypedFilename();
}

FileInfoSectionType FileInfo::getSectionType(Chunk &chunk) {
	return static_cast<FileInfoSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region StreamInfo
StreamInfo::StreamInfo(Chunk &chunk) {
	// Read the actor ID.
	StreamInfoSectionType sectionType = getSectionType(chunk);
	if (kStreamInfoActorId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_actorId = chunk.readTypedUint16();

	// Read the file ID.
	sectionType = getSectionType(chunk);
	if (kStreamInfoFileId != sectionType) {
		error("%s: Expected section type FILE_ID, got 0x%x", __func__, static_cast<uint>(sectionType));
	}
	_fileId = chunk.readTypedUint16();

	// Read the start offset from the absolute start of the file.
	sectionType = getSectionType(chunk);
	if (kStreamInfoStartOffset != sectionType) {
		error("%s: Expected section type START_OFFSET, got 0x%x", __func__, static_cast<uint>(sectionType));
	}
	_startOffsetInFile = chunk.readTypedUint32();
}

StreamInfoSectionType StreamInfo::getSectionType(Chunk &chunk) {
	return static_cast<StreamInfoSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region CursorDeclaration
CursorDeclaration::CursorDeclaration(Chunk &chunk) {
	uint16 unk1 = chunk.readTypedUint16(); // Always 0x0001
	_id = chunk.readTypedUint16();
	_unk = chunk.readTypedUint16();
	_name = chunk.readTypedFilename();
	debugC(5, kDebugLoading, " - CursorDeclaration(): unk1 = 0x%x, id = 0x%x, unk = 0x%x, name = %s", unk1, _id, _unk, _name.c_str());
}
#pragma endregion

#pragma region Boot
void MediaStationEngine::readDocumentDef(Chunk &chunk) {
	BootSectionType sectionType = kBootLastSection;
	while (true) {
		sectionType = static_cast<BootSectionType>(chunk.readTypedUint16());
		if (sectionType == kBootLastSection) {
			break;
		}
		readDocumentInfoFromStream(chunk, sectionType);
	}
}

void MediaStationEngine::readDocumentInfoFromStream(Chunk &chunk, BootSectionType sectionType) {
	switch (sectionType) {
	case kBootVersionInformation:
		readVersionInfoFromStream(chunk);
		break;

	case kBootContextReference:
		readContextReferencesFromStream(chunk);
		break;

	case kBootScreenReference:
		readScreenReferencesFromStream(chunk);
		break;

	case kBootFileInfo:
		readAndAddFileMaps(chunk);
		break;

	case kBootStreamInfo:
		readAndAddStreamMaps(chunk);
		break;

	case kBootUnk1:
		_unk1 = chunk.readTypedUint16();
		break;

	case kBootFunctionTableSize:
		_functionTableSize = chunk.readTypedUint16();
		break;

	case kBootUnk3:
		_unk3 = chunk.readTypedUint16();
		break;

	default:
		// See if any registered parameter clients know how to
		// handle this parameter.
		readUnrecognizedFromStream(chunk, static_cast<uint>(sectionType));
	}
}

void MediaStationEngine::readVersionInfoFromStream(Chunk &chunk) {
	_gameTitle = chunk.readTypedString();
	_versionInfo = chunk.readTypedVersion();
	_engineInfo = chunk.readTypedString();
	_sourceString = chunk.readTypedString();
}

void MediaStationEngine::readContextReferencesFromStream(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		ContextReference contextReference(chunk);
		_contextReferences.setVal(contextReference._contextId, contextReference);
		flag = chunk.readTypedUint16();
	}
}

void MediaStationEngine::readScreenReferencesFromStream(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		ScreenReference screenDeclaration(chunk);
		_screenReferences.setVal(screenDeclaration._screenActorId, screenDeclaration);
		flag = chunk.readTypedUint16();
	}
}

void MediaStationEngine::readAndAddFileMaps(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		FileInfo fileDeclaration(chunk);
		_fileMap.setVal(fileDeclaration._id, fileDeclaration);
		flag = chunk.readTypedUint16();
	}
}

void MediaStationEngine::readAndAddStreamMaps(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		StreamInfo subfileDeclaration(chunk);
		_streamMap.setVal(subfileDeclaration._actorId, subfileDeclaration);
		flag = chunk.readTypedUint16();
	}
}

#pragma endregion

} // End of namespace MediaStation

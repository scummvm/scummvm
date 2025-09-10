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

#pragma region ContextDeclaration
ContextDeclaration::ContextDeclaration(Chunk &chunk) {
	// Read the file number.
	ContextDeclarationSectionType sectionType = getSectionType(chunk);
	if (kContextDeclarationContextId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_contextId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kContextDeclarationStreamId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_streamId = chunk.readTypedUint16();

	// Read the context name. Only some titles have context names,
	// and unfortunately we can't determine which just by relying
	// on the title compiler version number.
	sectionType = getSectionType(chunk);
	if (kContextDeclarationName == sectionType) {
		_name = chunk.readTypedString();
		sectionType = getSectionType(chunk);
	}

	// Read the parent context IDs. We don't know how many file
	// references there are beforehand, so we'll just read until
	// we get something else.
	uint rewindOffset = chunk.pos();
	while (kContextDeclarationParentContextId == sectionType) {
		int fileReference = chunk.readTypedUint16();
		_parentContextIds.push_back(fileReference);
		rewindOffset = chunk.pos();
		sectionType = getSectionType(chunk);
	}
	chunk.seek(rewindOffset);
}

ContextDeclarationSectionType ContextDeclaration::getSectionType(Chunk &chunk) {
	return static_cast<ContextDeclarationSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region ScreenDeclaration
ScreenDeclaration::ScreenDeclaration(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	ScreenDeclarationSectionType sectionType = getSectionType(chunk);
	if (kScreenDeclarationActorId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_actorId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kScreenDeclarationScreenId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_screenId = chunk.readTypedUint16();
}

ScreenDeclarationSectionType ScreenDeclaration::getSectionType(Chunk &chunk) {
	return static_cast<ScreenDeclarationSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region FileDeclaration
FileDeclaration::FileDeclaration(Chunk &chunk) {
	// Read the file ID.
	FileDeclarationSectionType sectionType = getSectionType(chunk);
	if (kFileDeclarationFileId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_id = chunk.readTypedUint16();

	// Read the intended file location.
	sectionType = getSectionType(chunk);
	if (kFileDeclarationFileNameAndType != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_intendedLocation = static_cast<IntendedFileLocation>(chunk.readTypedUint16());

	// Since the platforms that Media Station originally targeted were case-insensitive,
	// the case of these filenames might not match the case of the files actually in
	// the directory. All files should be matched case-insensitively.
	_name = chunk.readTypedFilename();
}

FileDeclarationSectionType FileDeclaration::getSectionType(Chunk &chunk) {
	return static_cast<FileDeclarationSectionType>(chunk.readTypedUint16());
}
#pragma endregion

#pragma region SubfileDeclaration
SubfileDeclaration::SubfileDeclaration(Chunk &chunk) {
	// Read the actor ID.
	SubfileDeclarationSectionType sectionType = getSectionType(chunk);
	if (kSubfileDeclarationActorId != sectionType) {
		error("%s: Got unexpected section type %d", __func__, static_cast<uint>(sectionType));
	}
	_actorId = chunk.readTypedUint16();

	// Read the file ID.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationFileId != sectionType) {
		error("%s: Expected section type FILE_ID, got 0x%x", __func__, static_cast<uint>(sectionType));
	}
	_fileId = chunk.readTypedUint16();

	// Read the start offset from the absolute start of the file.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationStartOffset != sectionType) {
		error("%s: Expected section type START_OFFSET, got 0x%x", __func__, static_cast<uint>(sectionType));
	}
	_startOffsetInFile = chunk.readTypedUint32();
}

SubfileDeclarationSectionType SubfileDeclaration::getSectionType(Chunk &chunk) {
	return static_cast<SubfileDeclarationSectionType>(chunk.readTypedUint16());
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
Boot::Boot(const Common::Path &path) : Datafile(path) {
	Subfile subfile = getNextSubfile();
	Chunk chunk = subfile.nextChunk();

	// TODO: This is really analogous to RT_ImtGod::notifyBufferFilled.
	// But we are currently only handling the DocumentDef part of it.
	BootStreamType streamType = static_cast<BootStreamType>(chunk.readTypedUint16());
	switch (streamType) {
		case kBootDocumentDef:
			readDocumentDef(chunk);
			break;

		case kBootControlCommands:
			error("%s: STUB: readControlComments", __func__);
			break;

		default:
			error("%s: Unhandled section type 0x%x", __func__, static_cast<uint>(streamType));
	}
}

BootSectionType Boot::getSectionType(Chunk &chunk) {
	return static_cast<BootSectionType>(chunk.readTypedUint16());
}

Boot::~Boot() {
	_contextDeclarations.clear();
	_streamMap.clear();
	_engineResourceDeclarations.clear();
	_screenDeclarations.clear();
	_fileMap.clear();
}

void Boot::readDocumentDef(Chunk &chunk) {
	BootSectionType sectionType = kBootLastSection;
	while (true) {
		sectionType = getSectionType(chunk);
		if (sectionType == kBootLastSection) {
			break;
		}
		readDocumentInfoFromStream(chunk, sectionType);
	}
}

void Boot::readDocumentInfoFromStream(Chunk &chunk, BootSectionType sectionType) {
	switch (sectionType) {
	case kBootVersionInformation:
		readVersionInfoFromStream(chunk);
		break;

	case kBootContextDeclaration:
		readContextReferencesFromStream(chunk);
		break;

	case kBootScreenDeclaration:
		readScreenDeclarationsFromStream(chunk);
		break;

	case kBootFileDeclaration:
		readAndAddFileMaps(chunk);
		break;

	case kBootSubfileDeclaration:
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
		g_engine->readUnrecognizedFromStream(chunk, static_cast<uint>(sectionType));
	}
}

void Boot::readVersionInfoFromStream(Chunk &chunk) {
	_gameTitle = chunk.readTypedString();
	_versionInfo = chunk.readTypedVersion();
	_engineInfo = chunk.readTypedString();
	_sourceString = chunk.readTypedString();
}

void Boot::readContextReferencesFromStream(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		ContextDeclaration contextDeclaration(chunk);
		_contextDeclarations.setVal(contextDeclaration._contextId, contextDeclaration);
		flag = chunk.readTypedUint16();
	}
}

void Boot::readScreenDeclarationsFromStream(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		ScreenDeclaration screenDeclaration(chunk);
		_screenDeclarations.setVal(screenDeclaration._screenId, screenDeclaration);
		flag = chunk.readTypedUint16();
	}
}

void Boot::readAndAddFileMaps(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		FileDeclaration fileDeclaration(chunk);
		_fileMap.setVal(fileDeclaration._id, fileDeclaration);
		flag = chunk.readTypedUint16();
	}
}

void Boot::readAndAddStreamMaps(Chunk &chunk) {
	uint flag = chunk.readTypedUint16();
	while (flag != 0) {
		SubfileDeclaration subfileDeclaration(chunk);
		_streamMap.setVal(subfileDeclaration._actorId, subfileDeclaration);
		flag = chunk.readTypedUint16();
	}
}

#pragma endregion

} // End of namespace MediaStation

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
#include "mediastation/datum.h"
#include "mediastation/subfile.h"
#include "mediastation/chunk.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

#pragma region VersionInfo
VersionInfo::VersionInfo(Chunk &chunk) {
	majorVersion = Datum(chunk, DatumType::UINT16_1).u.i;
	minorVersion = Datum(chunk, DatumType::UINT16_1).u.i;
	revision = Datum(chunk, DatumType::UINT16_1).u.i;
	string = Datum(chunk, DatumType::STRING).u.string;
}

VersionInfo::~VersionInfo() {
	if (string != nullptr) {
		delete string;
	}
}
#pragma endregion

#pragma region ContextDeclaration
ContextDeclaration::ContextDeclaration(Chunk &chunk) {
	// ENSURE WE HAVEN'T REACHED THE END OF THE DECLARATIONS.
	ContextDeclaration::SectionType sectionType = getSectionType(chunk);
	if (ContextDeclaration::SectionType::EMPTY == sectionType) {
		isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		isLast = false;
	}

	// READ THE FILE REFERENCES.
	while (ContextDeclaration::SectionType::FILE_REFERENCE == sectionType) {
		int fileReference = Datum(chunk).u.i;
		fileReferences.push_back(fileReference);
		sectionType = getSectionType(chunk);
	}

	// READ THE OTHER CONTEXT METADATA.
	if (ContextDeclaration::SectionType::PLACEHOLDER == sectionType) {
		// READ THE FILE NUMBER.
		sectionType = getSectionType(chunk);
		if (ContextDeclaration::SectionType::FILE_NUMBER_1 == sectionType) {
			fileNumber = Datum(chunk).u.i;
		} else {
			error("ContextDeclaration(): Expected section type FILE_NUMBER_1, got 0x%x", sectionType);
		}
		// I don't know why the file number is always repeated.
		// Is it just for data integrity, or is there some other reason?
		sectionType = getSectionType(chunk);
		if (ContextDeclaration::SectionType::FILE_NUMBER_2 == sectionType) {
			uint32 repeatedFileNumber = Datum(chunk).u.i;
			if (repeatedFileNumber != fileNumber) {
				warning("ContextDeclaration(): Expected file numbers to match, but 0x%d != 0x%d", fileNumber, repeatedFileNumber);
			}
		} else {
			error("ContextDeclaration(): Expected section type FILE_NUMBER_2, got 0x%x", sectionType);
		}

		// READ THE CONTEXT NAME.
		// Only some titles have context names, and unfortunately we can't
		// determine which just by relying on the title compiler version
		// number.
		// TODO: Find a better way to read the context name without relying
		// on reading and rewinding.
		int rewindOffset = chunk.pos();
		sectionType = getSectionType(chunk);
		if (ContextDeclaration::SectionType::CONTEXT_NAME == sectionType) {
			contextName = Datum(chunk, DatumType::STRING).u.string;
		} else {
			// THERE IS NO CONTEXT NAME.
			// We have instead read into the next declaration, so let's undo that.
			chunk.seek(rewindOffset);
		}
	} else if (ContextDeclaration::SectionType::EMPTY == sectionType) {
		isLast = true;
	} else {
		error("ContextDeclaration::ContextDeclaration(): Unknown section type 0x%x", sectionType);
	}
}

ContextDeclaration::SectionType ContextDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	ContextDeclaration::SectionType sectionType = static_cast<ContextDeclaration::SectionType>(datum.u.i);
	return sectionType;
}

ContextDeclaration::~ContextDeclaration() {
	delete contextName;
	contextName = nullptr;
}
#pragma endregion

#pragma region UnknownDeclaration
UnknownDeclaration::UnknownDeclaration(Chunk &chunk) {
	// ENSURE THIS DECLARATION IS NOT EMPTY.
	UnknownDeclaration::SectionType sectionType = getSectionType(chunk);
	if (UnknownDeclaration::SectionType::EMPTY == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	// READ THE UNKNOWN VALUE.
	sectionType = getSectionType(chunk);
	if (UnknownDeclaration::SectionType::UNK_1 == sectionType) {
		_unk = Datum(chunk, DatumType::UINT16_1).u.i;
	} else {
		error("UnknownDeclaration(): Expected section type UNK_1, got 0x%x", sectionType);
	}
	sectionType = getSectionType(chunk);
	if (UnknownDeclaration::SectionType::UNK_2 == sectionType) {
		uint16 repeatedUnk = Datum(chunk, DatumType::UINT16_1).u.i;
		if (repeatedUnk != _unk) {
			warning("UnknownDeclaration(): Expected unknown values to match, but 0x%x != 0x%x", _unk, repeatedUnk);
		}
	} else {
		error("UnknownDeclaration(): Expected section type UNK_2, got 0x%x", sectionType);
	}
}

UnknownDeclaration::SectionType UnknownDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	UnknownDeclaration::SectionType sectionType = static_cast<UnknownDeclaration::SectionType>(datum.u.i);
	return sectionType;
}
#pragma endregion

#pragma region FileDeclaration
FileDeclaration::FileDeclaration(Chunk &chunk) {
	// ENSURE THIS DECLARATION IS NOT EMPTY.
	FileDeclaration::SectionType sectionType = getSectionType(chunk);
	if (FileDeclaration::SectionType::EMPTY == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	// READ THE FILE ID.
	sectionType = getSectionType(chunk);
	if (FileDeclaration::SectionType::FILE_ID == sectionType) {
		_id = Datum(chunk, DatumType::UINT16_1).u.i;
	} else {
		error("FileDeclaration(): Expected section type FILE_ID, got 0x%x", sectionType);
	}

	// READ THE INTENDED LOCATION OF THE FILE.
	sectionType = getSectionType(chunk);
	if (FileDeclaration::SectionType::FILE_NAME_AND_TYPE == sectionType) {
		Datum datum = Datum(chunk, DatumType::UINT16_1);
		// TODO: Verify we actually read a valid enum member.
		_intendedLocation = static_cast<FileDeclaration::IntendedLocation>(datum.u.i);
	} else {
		error("FileDeclaration(): Expected section type FILE_NAME_AND_TYPE, got 0x%x", sectionType);
	}

	// READ THE CASE-INSENSITIVE FILENAME.
	// Since the platforms that Media Station originally targeted were case-insensitive,
	// the case of these filenames might not match the case of the files actually in
	// the directory. All files should be matched case-insensitively.
	_name = Datum(chunk, DatumType::FILENAME).u.string;
}

FileDeclaration::SectionType FileDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	FileDeclaration::SectionType sectionType = static_cast<FileDeclaration::SectionType>(datum.u.i);
	return sectionType;
}

FileDeclaration::~FileDeclaration() {
	delete _name;
	_name = nullptr;
}
#pragma endregion

#pragma region SubfileDeclaration
SubfileDeclaration::SubfileDeclaration(Chunk &chunk) {
	// ENSURE THIS DECLARATION IS NOT EMPTY.
	SubfileDeclaration::SectionType sectionType = getSectionType(chunk);
	if (SubfileDeclaration::SectionType::EMPTY == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	// READ THE ASSET ID.
	sectionType = getSectionType(chunk);
	if (SubfileDeclaration::SectionType::ASSET_ID == sectionType) {
		_assetId = Datum(chunk, DatumType::UINT16_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type ASSET_ID, got 0x%x", sectionType);
	}

	// READ THE FILE ID.
	sectionType = getSectionType(chunk);
	if (SubfileDeclaration::SectionType::FILE_ID == sectionType) {
		_fileId = Datum(chunk, DatumType::UINT16_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type FILE_ID, got 0x%x", sectionType);
	}

	// READ THE START OFFSET IN THE GIVEN FILE.
	// This is from the absolute start of the given file.
	sectionType = getSectionType(chunk);
	if (SubfileDeclaration::SectionType::START_OFFSET == sectionType) {
		_startOffsetInFile = Datum(chunk, DatumType::UINT32_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type START_OFFSET, got 0x%x", sectionType);
	}
}

SubfileDeclaration::SectionType SubfileDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	SubfileDeclaration::SectionType sectionType = static_cast<SubfileDeclaration::SectionType>(datum.u.i);
	return sectionType;
}
#pragma endregion

#pragma region CursorDeclaration
CursorDeclaration::CursorDeclaration(Chunk& chunk) {
	// READ THE CURSOR RESOURCE.
	uint16 unk1 = Datum(chunk, DatumType::UINT16_1).u.i; // Always 0x0001
	_id = Datum(chunk, DatumType::UINT16_1).u.i;
	_unk = Datum(chunk, DatumType::UINT16_1).u.i;
	_name = Datum(chunk, DatumType::FILENAME).u.string;
	debugC(5, kDebugLoading, " - CursorDeclaration(): unk1 = 0x%x, id = 0x%x, unk = 0x%x, name = %s", unk1, _id, _unk, _name->c_str());
}

CursorDeclaration::~CursorDeclaration() {
	delete _name;
	_name = nullptr;
}
#pragma endregion

#pragma region Engine Resource Declaration
EngineResourceDeclaration::EngineResourceDeclaration(Common::String *resourceName, int resourceId) : _resourceName(resourceName), _resourceId(resourceId) {}

EngineResourceDeclaration::~EngineResourceDeclaration() {
	delete _resourceName;
	_resourceName = nullptr;
}
#pragma endregion

#pragma region Boot
Boot::Boot(const Common::Path &path) : Datafile(path) {
	// OPEN THE FILE FOR READING.
	subfile = Subfile(_stream);
	Chunk chunk = subfile.nextChunk();

	uint32 beforeSectionTypeUnk = Datum(chunk, DatumType::UINT16_1).u.i; // Usually 0x0001
	debugC(5, kDebugLoading, "Boot::Boot(): unk1 = 0x%x", beforeSectionTypeUnk);

	Boot::SectionType sectionType = getSectionType(chunk);
	bool notLastSection = (Boot::SectionType::LAST != sectionType);
	while (notLastSection) {
		debugC(5, kDebugLoading, "Boot::Boot(): sectionType = 0x%x", sectionType);
		switch (sectionType) {
		case Boot::SectionType::VERSION_INFORMATION: {
			_gameTitle = Datum(chunk, DatumType::STRING).u.string;
			debugC(5, kDebugLoading, " - gameTitle = %s", _gameTitle->c_str());
			uint32 unk = chunk.readUint16LE();
			debugC(5, kDebugLoading, " - unk = 0x%x", unk);
			_versionInfo = new VersionInfo(chunk);
			debugC(5, kDebugLoading, " - versionInfo = %d.%d.%d (%s)", _versionInfo->majorVersion, _versionInfo->minorVersion, _versionInfo->revision, _versionInfo->string->c_str());
			_sourceString = Datum(chunk, DatumType::STRING).u.string;
			debugC(5, kDebugLoading, " - sourceString = %s", _sourceString->c_str());
			break;
		}

		case Boot::SectionType::UNK1:
		case Boot::SectionType::UNK2:
		case Boot::SectionType::UNK3: {
			uint32 unk = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - unk = 0x%x", unk);
			break;
		}

		case Boot::SectionType::UNK4: {
			double unk = Datum(chunk).u.f;
			debugC(5, kDebugLoading, " - unk = %f", unk);
			break;
		}

		case Boot::SectionType::ENGINE_RESOURCE: {
			Common::String *resourceName = Datum(chunk, DatumType::STRING).u.string;
			sectionType = getSectionType(chunk);
			if (sectionType == Boot::SectionType::ENGINE_RESOURCE_ID) {
				int resourceId = Datum(chunk).u.i;
				EngineResourceDeclaration *resourceDeclaration = new EngineResourceDeclaration(resourceName, resourceId);
				_engineResourceDeclarations.setVal(resourceId, resourceDeclaration);
			} else {
				error("Boot::Boot(): Got section type 0x%x when expecting ENGINE_RESOURCE_ID", sectionType);
			}
			break;
		}

		case Boot::SectionType::CONTEXT_DECLARATION: {
			ContextDeclaration *contextDeclaration = new ContextDeclaration(chunk);
			while (!contextDeclaration->isLast) {
				_contextDeclarations.setVal(contextDeclaration->fileNumber, contextDeclaration);
				contextDeclaration = new ContextDeclaration(chunk);
			}
			break;
		}

		case Boot::SectionType::UNKNOWN_DECLARATION: {
			UnknownDeclaration *unknownDeclaration = new UnknownDeclaration(chunk);
			while (!unknownDeclaration->_isLast) {
				_unknownDeclarations.push_back(unknownDeclaration);
				unknownDeclaration = new UnknownDeclaration(chunk);
			}
			break;
		}

		case Boot::SectionType::FILE_DECLARATION: {
			FileDeclaration *fileDeclaration = new FileDeclaration(chunk);
			while (!fileDeclaration->_isLast) {
				_fileDeclarations.setVal(fileDeclaration->_id, fileDeclaration);
				fileDeclaration = new FileDeclaration(chunk);
			}
			break;
		}

		case Boot::SectionType::SUBFILE_DECLARATION: {
			SubfileDeclaration *subfileDeclaration = new SubfileDeclaration(chunk);
			while (!subfileDeclaration->_isLast) {
				_subfileDeclarations.setVal(subfileDeclaration->_assetId, subfileDeclaration);
				subfileDeclaration = new SubfileDeclaration(chunk);
			}
			break;
		}

		case Boot::SectionType::CURSOR_DECLARATION: {
			CursorDeclaration *cursorDeclaration = new CursorDeclaration(chunk);
			_cursorDeclarations.setVal(cursorDeclaration->_id, cursorDeclaration);
			break;
		}

		case Boot::SectionType::EMPTY: {
			// This semems to separate the cursor declarations from whatever comes
			// after it (what I formerly called the "footer"), but it has no data
			// itself.
			break;
		}

		case Boot::SectionType::ENTRY_SCREEN: {
			_entryContextId = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - _entryContextId = %d", _entryContextId);
			break;
		}

		case Boot::SectionType::ALLOW_MULTIPLE_SOUNDS: {
			_allowMultipleSounds = (Datum(chunk).u.i == 1);
			debugC(5, kDebugLoading, " - _allowMultipleSounds = %d", _allowMultipleSounds);
			break;
		}

		case Boot::SectionType::ALLOW_MULTIPLE_STREAMS: {
			_allowMultipleStreams = (Datum(chunk).u.i == 1);
			debugC(5, kDebugLoading, " - _allowMultipleStreams = %d", _allowMultipleStreams);
			break;
		}

		case Boot::SectionType::UNK5: {
			uint32 unk1 = Datum(chunk).u.i;
			uint32 unk2 = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - unk1 = 0x%x, unk2 = 0x%x", unk1, unk2);
			break;
		}

		default: {
			warning("Boot::Boot(): Unknown section type 0x%x", sectionType);
			break;
		}
		}

		sectionType = getSectionType(chunk);
		notLastSection = Boot::SectionType::LAST != sectionType;
	}
}

Boot::SectionType Boot::getSectionType(Chunk& chunk) {
	Datum datum = Datum(chunk, DatumType::UINT16_1);
	Boot::SectionType sectionType = static_cast<Boot::SectionType>(datum.u.i);
	return sectionType;
}

uint32 Boot::getRootContextId() {
	// TODO: Is the ID of the root context actually stored somewhere so
	// we don't need to find it ourselves? Maybe it is always the
	for (auto &declaration : _contextDeclarations) {
		if (declaration._value->fileReferences.empty()) {
			return declaration._value->fileNumber;
		}
	}
	return 0;
}

Boot::~Boot() {
	delete _gameTitle;
	_gameTitle = nullptr;

	_contextDeclarations.clear();
	_subfileDeclarations.clear();
	_cursorDeclarations.clear();
	_engineResourceDeclarations.clear();
	_unknownDeclarations.clear();
}
#pragma endregion

} // End of namespace MediaStation

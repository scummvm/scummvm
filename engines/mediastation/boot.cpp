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
#include "mediastation/debugchannels.h"

namespace MediaStation {

#pragma region VersionInfo
VersionInfo::VersionInfo(Chunk &chunk) {
	_majorVersion = Datum(chunk, kDatumTypeUint16_1).u.i;
	_minorVersion = Datum(chunk, kDatumTypeUint16_1).u.i;
	_revision = Datum(chunk, kDatumTypeUint16_1).u.i;
	string = Datum(chunk, kDatumTypeString).u.string;
}

VersionInfo::~VersionInfo() {
	if (string != nullptr) {
		delete string;
	}
}
#pragma endregion

#pragma region ContextDeclaration
ContextDeclaration::ContextDeclaration(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	ContextDeclarationSectionType sectionType = getSectionType(chunk);
	if (kContextDeclarationEmptySection == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	if (kContextDeclarationPlaceholder == sectionType) {
		// Read the file number.
		sectionType = getSectionType(chunk);
		if (kContextDeclarationFileNumber1 == sectionType) {
			_fileNumber = Datum(chunk).u.i;
		} else {
			error("ContextDeclaration(): Expected section type FILE_NUMBER_1, got 0x%x", static_cast<uint>(sectionType));
		}
		// I don't know why the file number is always repeated.
		// Is it just for data integrity, or is there some other reason?
		sectionType = getSectionType(chunk);
		if (kContextDeclarationFileNumber2 == sectionType) {
			uint32 repeatedFileNumber = Datum(chunk).u.i;
			if (repeatedFileNumber != _fileNumber) {
				warning("ContextDeclaration(): Expected file numbers to match, but 0x%d != 0x%d", _fileNumber, repeatedFileNumber);
			}
		} else {
			error("ContextDeclaration(): Expected section type FILE_NUMBER_2, got 0x%x", static_cast<uint>(sectionType));
		}

		// Read the context name.
		// Only some titles have context names, and unfortunately we can't
		// determine which just by relying on the title compiler version
		// number.
		// TODO: Find a better way to read the context name without relying
		// on reading and rewinding.
		int rewindOffset = chunk.pos();
		sectionType = getSectionType(chunk);
		if (kContextDeclarationName == sectionType) {
			_contextName = Datum(chunk, kDatumTypeString).u.string;
		} else {
			// There is no context name.
			// We have instead read into the next declaration, so let's undo that.
			chunk.seek(rewindOffset);
		}
	} else if (kContextDeclarationEmptySection == sectionType) {
		_isLast = true;
	} else {
		error("ContextDeclaration::ContextDeclaration(): Unknown section type 0x%x", static_cast<uint>(sectionType));
	}

	// Read the file references.
	// We don't know how many file references there are beforehand, so we'll
	// just read until we get something else.
	int rewindOffset = 0;
	sectionType = getSectionType(chunk);
	while (kContextDeclarationFileReference == sectionType) {
		int fileReference = Datum(chunk).u.i;
		_fileReferences.push_back(fileReference);
		rewindOffset = chunk.pos();
		sectionType = getSectionType(chunk);
	}
	chunk.seek(rewindOffset);
}

ContextDeclarationSectionType ContextDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	ContextDeclarationSectionType sectionType = static_cast<ContextDeclarationSectionType>(datum.u.i);
	return sectionType;
}

ContextDeclaration::~ContextDeclaration() {
	delete _contextName;
	_contextName = nullptr;
}
#pragma endregion

#pragma region UnknownDeclaration
UnknownDeclaration::UnknownDeclaration(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	UnknownDeclarationSectionType sectionType = getSectionType(chunk);
	if (kUnknownDeclarationEmptySection == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	sectionType = getSectionType(chunk);
	if (kUnknownDeclarationUnk1 == sectionType) {
		_unk = Datum(chunk, kDatumTypeUint16_1).u.i;
	} else {
		error("UnknownDeclaration(): Expected section type UNK_1, got 0x%x", static_cast<uint>(sectionType));
	}
	sectionType = getSectionType(chunk);
	if (kUnknownDeclarationUnk2 == sectionType) {
		uint16 repeatedUnk = Datum(chunk, kDatumTypeUint16_1).u.i;
		if (repeatedUnk != _unk) {
			warning("UnknownDeclaration(): Expected unknown values to match, but 0x%x != 0x%x", _unk, repeatedUnk);
		}
	} else {
		error("UnknownDeclaration(): Expected section type UNK_2, got 0x%x", static_cast<uint>(sectionType));
	}
}

UnknownDeclarationSectionType UnknownDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	UnknownDeclarationSectionType sectionType = static_cast<UnknownDeclarationSectionType>(datum.u.i);
	return sectionType;
}
#pragma endregion

#pragma region FileDeclaration
FileDeclaration::FileDeclaration(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	FileDeclarationSectionType sectionType = getSectionType(chunk);
	if (kFileDeclarationEmptySection == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	// Read the file ID.
	sectionType = getSectionType(chunk);
	if (kFileDeclarationFileId == sectionType) {
		_id = Datum(chunk, kDatumTypeUint16_1).u.i;
	} else {
		error("FileDeclaration(): Expected section type FILE_ID, got 0x%x", static_cast<uint>(sectionType));
	}

	// Read the intended file location.
	sectionType = getSectionType(chunk);
	if (kFileDeclarationFileNameAndType == sectionType) {
		Datum datum = Datum(chunk, kDatumTypeUint16_1);
		// TODO: Verify we actually read a valid enum member.
		_intendedLocation = static_cast<IntendedFileLocation>(datum.u.i);
	} else {
		error("FileDeclaration(): Expected section type FILE_NAME_AND_TYPE, got 0x%x", static_cast<uint>(sectionType));
	}

	// Since the platforms that Media Station originally targeted were case-insensitive,
	// the case of these filenames might not match the case of the files actually in
	// the directory. All files should be matched case-insensitively.
	_name = Datum(chunk, kDatumTypeFilename).u.string;
}

FileDeclarationSectionType FileDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	FileDeclarationSectionType sectionType = static_cast<FileDeclarationSectionType>(datum.u.i);
	return sectionType;
}

FileDeclaration::~FileDeclaration() {
	delete _name;
	_name = nullptr;
}
#pragma endregion

#pragma region SubfileDeclaration
SubfileDeclaration::SubfileDeclaration(Chunk &chunk) {
	// Make sure this declaration isn't empty.
	SubfileDeclarationSectionType sectionType = getSectionType(chunk);
	if (kSubfileDeclarationEmptySection == sectionType) {
		_isLast = true;
		return;
	} else {
		// There may be more declarations in the stream.
		_isLast = false;
	}

	// Read the asset ID.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationAssetId == sectionType) {
		_assetId = Datum(chunk, kDatumTypeUint16_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type ASSET_ID, got 0x%x", static_cast<uint>(sectionType));
	}

	// Read the file ID.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationFileId == sectionType) {
		_fileId = Datum(chunk, kDatumTypeUint16_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type FILE_ID, got 0x%x", static_cast<uint>(sectionType));
	}

	// Read the start offset from the absolute start of the file.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationStartOffset == sectionType) {
		_startOffsetInFile = Datum(chunk, kDatumTypeUint32_1).u.i;
	} else {
		error("SubfileDeclaration(): Expected section type START_OFFSET, got 0x%x", static_cast<uint>(sectionType));
	}
}

SubfileDeclarationSectionType SubfileDeclaration::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	SubfileDeclarationSectionType sectionType = static_cast<SubfileDeclarationSectionType>(datum.u.i);
	return sectionType;
}
#pragma endregion

#pragma region CursorDeclaration
CursorDeclaration::CursorDeclaration(Chunk& chunk) {
	uint16 unk1 = Datum(chunk, kDatumTypeUint16_1).u.i; // Always 0x0001
	_id = Datum(chunk, kDatumTypeUint16_1).u.i;
	_unk = Datum(chunk, kDatumTypeUint16_1).u.i;
	_name = Datum(chunk, kDatumTypeFilename).u.string;
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
Boot::Boot(const Common::Path &path) : Datafile(path){
	Subfile subfile = getNextSubfile();
	Chunk chunk = subfile.nextChunk();

	uint32 beforeSectionTypeUnk = Datum(chunk, kDatumTypeUint16_1).u.i; // Usually 0x0001
	debugC(5, kDebugLoading, "Boot::Boot(): unk1 = 0x%x", beforeSectionTypeUnk);

	BootSectionType sectionType = getSectionType(chunk);
	bool notLastSection = (kBootLastSection != sectionType);
	while (notLastSection) {
		debugC(5, kDebugLoading, "Boot::Boot(): sectionType = 0x%x", static_cast<uint>(sectionType));
		switch (sectionType) {
		case kBootVersionInformation: {
			_gameTitle = Datum(chunk, kDatumTypeString).u.string;
			debugC(5, kDebugLoading, " - gameTitle = %s", _gameTitle->c_str());
			uint32 unk = chunk.readUint16LE();
			debugC(5, kDebugLoading, " - unk = 0x%x", unk);
			_versionInfo = new VersionInfo(chunk);
			debugC(5, kDebugLoading, " - versionInfo = %d.%d.%d (%s)", _versionInfo->_majorVersion, _versionInfo->_minorVersion, _versionInfo->_revision, _versionInfo->string->c_str());
			_sourceString = Datum(chunk, kDatumTypeString).u.string;
			debugC(5, kDebugLoading, " - sourceString = %s", _sourceString->c_str());
			break;
		}

		case kBootUnk1:
		case kBootUnk2:
		case kBootUnk3: {
			uint32 unk = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - unk = 0x%x", unk);
			break;
		}

		case kBootUnk4: {
			double unk = Datum(chunk).u.f;
			debugC(5, kDebugLoading, " - unk = %f", unk);
			break;
		}

		case kBootEngineResource: {
			Common::String *resourceName = Datum(chunk, kDatumTypeString).u.string;
			sectionType = getSectionType(chunk);
			if (sectionType == kBootEngineResourceId) {
				int resourceId = Datum(chunk).u.i;
				EngineResourceDeclaration *resourceDeclaration = new EngineResourceDeclaration(resourceName, resourceId);
				_engineResourceDeclarations.setVal(resourceId, resourceDeclaration);
			} else {
				error("Boot::Boot(): Got section type 0x%x when expecting ENGINE_RESOURCE_ID", static_cast<uint>(sectionType));
			}
			break;
		}

		case kBootContextDeclaration: {
			ContextDeclaration *contextDeclaration = new ContextDeclaration(chunk);
			while (!contextDeclaration->_isLast) {
				_contextDeclarations.setVal(contextDeclaration->_fileNumber, contextDeclaration);
				contextDeclaration = new ContextDeclaration(chunk);
			}
			// The very last declaration is just an empty flag, so delete it
			// since it's not put in the map.
			delete contextDeclaration;
			break;
		}

		case kBootUnknownDeclaration: {
			UnknownDeclaration *unknownDeclaration = new UnknownDeclaration(chunk);
			while (!unknownDeclaration->_isLast) {
				_unknownDeclarations.push_back(unknownDeclaration);
				unknownDeclaration = new UnknownDeclaration(chunk);
			}
			// The very last declaration is just an empty flag, so delete it
			// since it's not put in the map.
			delete unknownDeclaration;
			break;
		}

		case kBootFileDeclaration: {
			FileDeclaration *fileDeclaration = new FileDeclaration(chunk);
			while (!fileDeclaration->_isLast) {
				_fileDeclarations.setVal(fileDeclaration->_id, fileDeclaration);
				fileDeclaration = new FileDeclaration(chunk);
			}
			// The very last declaration is just an empty flag, so delete it
			// since it's not put in the map.
			delete fileDeclaration;
			break;
		}

		case kBootSubfileDeclaration: {
			SubfileDeclaration *subfileDeclaration = new SubfileDeclaration(chunk);
			while (!subfileDeclaration->_isLast) {
				_subfileDeclarations.setVal(subfileDeclaration->_assetId, subfileDeclaration);
				subfileDeclaration = new SubfileDeclaration(chunk);
			}
			// The very last declaration is just an empty flag, so delete it
			// since it's not put in the map.
			delete subfileDeclaration;
			break;
		}

		case kBootCursorDeclaration: {
			CursorDeclaration *cursorDeclaration = new CursorDeclaration(chunk);
			_cursorDeclarations.setVal(cursorDeclaration->_id, cursorDeclaration);
			break;
		}

		case kBootEmptySection: {
			// This semems to separate the cursor declarations from whatever comes
			// after it (what I formerly called the "footer"), but it has no data
			// itself.
			break;
		}

		case kBootEntryScreen: {
			_entryContextId = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - _entryContextId = %d", _entryContextId);
			break;
		}

		case kBootAllowMultipleSounds: {
			_allowMultipleSounds = (Datum(chunk).u.i == 1);
			debugC(5, kDebugLoading, " - _allowMultipleSounds = %d", _allowMultipleSounds);
			break;
		}

		case kBootAllowMultipleStreams: {
			_allowMultipleStreams = (Datum(chunk).u.i == 1);
			debugC(5, kDebugLoading, " - _allowMultipleStreams = %d", _allowMultipleStreams);
			break;
		}

		case kBootUnk5: {
			uint32 unk1 = Datum(chunk).u.i;
			uint32 unk2 = Datum(chunk).u.i;
			debugC(5, kDebugLoading, " - unk1 = 0x%x, unk2 = 0x%x", unk1, unk2);
			break;
		}

		default:
			warning("Boot::Boot(): Unknown section type %d", static_cast<uint>(sectionType));
		}

		sectionType = getSectionType(chunk);
		notLastSection = kBootLastSection != sectionType;
	}
}

BootSectionType Boot::getSectionType(Chunk &chunk) {
	Datum datum = Datum(chunk, kDatumTypeUint16_1);
	BootSectionType sectionType = static_cast<BootSectionType>(datum.u.i);
	return sectionType;
}

Boot::~Boot() {
	delete _gameTitle;
	_gameTitle = nullptr;

	delete _versionInfo;
	_versionInfo = nullptr;

	delete _sourceString;
	_sourceString = nullptr;

	for (auto it = _contextDeclarations.begin(); it != _contextDeclarations.end(); ++it) {
		delete it->_value;
	}
	_contextDeclarations.clear();

	for (auto it = _subfileDeclarations.begin(); it != _subfileDeclarations.end(); ++it) {
		delete it->_value;
	}
	_subfileDeclarations.clear();

	for (auto it = _cursorDeclarations.begin(); it != _cursorDeclarations.end(); ++it) {
		delete it->_value;
	}
	_cursorDeclarations.clear();

	for (auto it = _engineResourceDeclarations.begin(); it != _engineResourceDeclarations.end(); ++it) {
		delete it->_value;
	}
	_engineResourceDeclarations.clear();

	for (auto unknownDeclaration : _unknownDeclarations) {
		delete unknownDeclaration;
	}
	_unknownDeclarations.clear();

	for (auto it = _fileDeclarations.begin(); it != _fileDeclarations.end(); ++it) {
		delete it->_value;
	}
	_fileDeclarations.clear();
}
#pragma endregion

} // End of namespace MediaStation

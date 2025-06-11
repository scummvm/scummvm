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

namespace MediaStation {

#pragma region ContextDeclaration
ContextDeclaration::ContextDeclaration(Chunk &chunk) {
	// Read the file number.
	ContextDeclarationSectionType sectionType = getSectionType(chunk);
	if (kContextDeclarationContextId != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
	}
	_contextId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kContextDeclarationStreamId != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
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
	if (kScreenDeclarationAssetId != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
	}
	_assetId = chunk.readTypedUint16();

	sectionType = getSectionType(chunk);
	if (kScreenDeclarationScreenId != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
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
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
	}
	_id = chunk.readTypedUint16();

	// Read the intended file location.
	sectionType = getSectionType(chunk);
	if (kFileDeclarationFileNameAndType != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
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
	// Read the asset ID.
	SubfileDeclarationSectionType sectionType = getSectionType(chunk);
	if (kSubfileDeclarationAssetId != sectionType) {
		error("Got unexpected section type %d", static_cast<uint>(sectionType));
	}
	_assetId = chunk.readTypedUint16();

	// Read the file ID.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationFileId != sectionType) {
		error("SubfileDeclaration(): Expected section type FILE_ID, got 0x%x", static_cast<uint>(sectionType));
	}
	_fileId = chunk.readTypedUint16();

	// Read the start offset from the absolute start of the file.
	sectionType = getSectionType(chunk);
	if (kSubfileDeclarationStartOffset != sectionType) {
		error("SubfileDeclaration(): Expected section type START_OFFSET, got 0x%x", static_cast<uint>(sectionType));
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

	uint32 beforeSectionTypeUnk = chunk.readTypedUint16(); // Usually 0x0001
	debugC(5, kDebugLoading, "Boot::Boot(): unk1 = 0x%x", beforeSectionTypeUnk);

	BootSectionType sectionType = getSectionType(chunk);
	bool notLastSection = (kBootLastSection != sectionType);
	while (notLastSection) {
		debugC(5, kDebugLoading, "Boot::Boot(): sectionType = 0x%x", static_cast<uint>(sectionType));
		switch (sectionType) {
		case kBootVersionInformation: {
			_gameTitle = chunk.readTypedString();
			debugC(5, kDebugLoading, " - gameTitle = %s", _gameTitle.c_str());
			_versionInfo = chunk.readTypedVersion();
			_engineInfo = chunk.readTypedString();
			debugC(5, kDebugLoading, " - versionInfo = %d.%d.%d (%s)",
				_versionInfo.major, _versionInfo.minor, _versionInfo.patch, _engineInfo.c_str());
			_sourceString = chunk.readTypedString();
			debugC(5, kDebugLoading, " - sourceString = %s", _sourceString.c_str());
			break;
		}

		case kBootUnk1:
		case kBootUnk2:
		case kBootUnk3: {
			uint unk = chunk.readTypedUint16();
			debugC(5, kDebugLoading, " - unk = 0x%x", unk);
			break;
		}

		case kBootUnk4: {
			double unk = chunk.readTypedTime();
			debugC(5, kDebugLoading, " - unk = %f", unk);
			break;
		}

		case kBootEngineResource: {
			Common::String resourceName = chunk.readTypedString();
			sectionType = getSectionType(chunk);
			if (sectionType == kBootEngineResourceId) {
				int resourceId = chunk.readTypedUint16();
				EngineResourceDeclaration resourceDeclaration = EngineResourceDeclaration(resourceName, resourceId);
				_engineResourceDeclarations.setVal(resourceId, resourceDeclaration);
			} else {
				error("Boot::Boot(): Got section type 0x%x when expecting ENGINE_RESOURCE_ID", static_cast<uint>(sectionType));
			}
			break;
		}

		case kBootContextDeclaration: {
			uint flag = chunk.readTypedUint16();
			while (flag != 0) {
				ContextDeclaration contextDeclaration = ContextDeclaration(chunk);
				_contextDeclarations.setVal(contextDeclaration._contextId, contextDeclaration);
				flag = chunk.readTypedUint16();
			}
			break;
		}

		case kBootScreenDeclaration: {
			uint flag = chunk.readTypedUint16();
			while (flag != 0) {
				ScreenDeclaration screenDeclaration = ScreenDeclaration(chunk);
				_screenDeclarations.setVal(screenDeclaration._assetId, screenDeclaration);
				flag = chunk.readTypedUint16();
			}
			break;
		}

		case kBootFileDeclaration: {
			uint flag = chunk.readTypedUint16();
			while (flag != 0) {
				FileDeclaration fileDeclaration = FileDeclaration(chunk);
				_fileDeclarations.setVal(fileDeclaration._id, fileDeclaration);
				flag = chunk.readTypedUint16();
			};
			break;
		}

		case kBootSubfileDeclaration: {
			uint flag = chunk.readTypedUint16();
			while (flag != 0) {
				SubfileDeclaration subfileDeclaration = SubfileDeclaration(chunk);
				_subfileDeclarations.setVal(subfileDeclaration._assetId, subfileDeclaration);
				flag = chunk.readTypedUint16();
			}
			break;
		}

		case kBootCursorDeclaration: {
			CursorDeclaration cursorDeclaration = CursorDeclaration(chunk);
			_cursorDeclarations.setVal(cursorDeclaration._id, cursorDeclaration);
			break;
		}

		case kBootEmptySection: {
			// This semems to separate the cursor declarations from whatever comes
			// after it (what I formerly called the "footer"), but it has no data
			// itself.
			break;
		}

		case kBootEntryScreen: {
			_entryContextId = chunk.readTypedUint16();
			debugC(5, kDebugLoading, " - _entryContextId = %d", _entryContextId);
			break;
		}

		case kBootAllowMultipleSounds: {
			_allowMultipleSounds = (chunk.readTypedByte() == 1);
			debugC(5, kDebugLoading, " - _allowMultipleSounds = %d", _allowMultipleSounds);
			break;
		}

		case kBootAllowMultipleStreams: {
			_allowMultipleStreams = (chunk.readTypedByte() == 1);
			debugC(5, kDebugLoading, " - _allowMultipleStreams = %d", _allowMultipleStreams);
			break;
		}

		case kBootUnk5: {
			uint32 unk1 = chunk.readTypedUint16();
			uint32 unk2 = chunk.readTypedUint16();
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
	return static_cast<BootSectionType>(chunk.readTypedUint16());
}

Boot::~Boot() {
	_contextDeclarations.clear();
	_subfileDeclarations.clear();
	_cursorDeclarations.clear();
	_engineResourceDeclarations.clear();
	_screenDeclarations.clear();
	_fileDeclarations.clear();
}
#pragma endregion

} // End of namespace MediaStation

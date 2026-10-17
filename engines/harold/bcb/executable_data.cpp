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

#include "common/algorithm.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "harold/bcb/bcb.h"

namespace BCB {

namespace {

struct PeSection {
	uint32 virtualAddress;
	uint32 size;
	uint32 rawOffset;
	uint32 rawSize;
};

struct ExecutableDataProfile {
	Common::Language language;
	const char *executableName;
	Common::CodePage encoding;
	uint32 sceneChoiceTextTableVa;
	uint32 sceneChoiceTextTableEndVa;
	uint32 mapButtonTextPointerTableVa;
	uint32 gameOverTextPointerTableVa;
	uint32 saveSlotOccupiedTextFormatVa;
	uint32 saveSlotEmptyTextFormatVa;
	uint32 noSaveDataMessageVa;
	uint32 sceneChoiceMoveTextVa;
	uint32 loadSlotConfirmTextFormatVa;
	uint32 saveSlotOverwriteConfirmTextFormatVa;
	uint32 saveSlotEmptyConfirmTextFormatVa;
	uint32 returnToTitleConfirmTextVa;
	uint32 titleExitConfirmTextVa;
};

static const ExecutableDataProfile kJapaneseExecutableDataProfile = {
	Common::JA_JPN,
	"bcb.exe",
	Common::kWindows932,
	0x00412000,
	0x00412d20,
	0x00412cd4,
	0x00413648,
	0x00416394,
	0x004163b4,
	0x004163bb,
	0x004163d0,
	0x004163d8,
	0x00416418,
	0x00416444,
	0x00416460,
	0x00416474};

static const ExecutableDataProfile kKoreanExecutableDataProfile = {
	Common::KO_KOR,
	"bcb.exe",
	Common::kWindows949,
	0x00421878,
	0x00422598,
	0x0041e404,
	0x0041f2a8,
	0x00423a10,
	0x004239f4,
	0x00423980,
	0x004239dc,
	0x004239a0,
	0x0042394c,
	0x00423934,
	0x004238fc,
	0x004238cc};

static const ExecutableDataProfile *getExecutableDataProfile(Common::Language language) {
	if (language == Common::KO_KOR)
		return &kKoreanExecutableDataProfile;

	return &kJapaneseExecutableDataProfile;
}

static const uint kSceneChoiceTextSlotsPerSet = 8;
static const uint kMapButtonTextCount = 19;
static const uint kGameOverTextCount = 10;

static bool readExecutableBytes(Common::SeekableReadStream *stream, Common::Array<byte> &data) {
	Common::ScopedPtr<Common::SeekableReadStream> executableStream(stream);
	if (!executableStream)
		return false;

	const int32 size = executableStream->size();
	if (size <= 0)
		return false;

	data.resize(size);
	return executableStream->read(&data[0], size) == (uint32)size;
}

static uint16 readUint16LE(const Common::Array<byte> &data, uint offset) {
	return data[offset] | (data[offset + 1] << 8);
}

static uint32 readUint32LE(const Common::Array<byte> &data, uint offset) {
	return data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16) | (data[offset + 3] << 24);
}

static bool virtualAddressToFileOffset(uint32 virtualAddress, uint32 imageBase, const Common::Array<PeSection> &sections, uint &fileOffset) {
	const uint32 relativeAddress = virtualAddress - imageBase;
	for (uint i = 0; i < sections.size(); ++i) {
		const PeSection &section = sections[i];
		if (relativeAddress < section.virtualAddress || relativeAddress >= section.virtualAddress + section.size)
			continue;

		fileOffset = section.rawOffset + relativeAddress - section.virtualAddress;
		return fileOffset < section.rawOffset + section.rawSize;
	}

	return false;
}

static bool readExecutableString(const Common::Array<byte> &executableData, uint32 virtualAddress, uint32 imageBase, const Common::Array<PeSection> &sections, Common::String &sjisText) {
	uint textOffset = 0;
	if (!virtualAddressToFileOffset(virtualAddress, imageBase, sections, textOffset))
		return false;

	uint textEnd = textOffset;
	while (textEnd < executableData.size() && executableData[textEnd] != 0)
		++textEnd;

	if (textEnd == textOffset)
		return false;

	sjisText = Common::String(reinterpret_cast<const char *>(&executableData[textOffset]), textEnd - textOffset);
	return true;
}

static bool readExecutableString(const Common::Array<byte> &executableData, uint32 virtualAddress, uint32 imageBase, const Common::Array<PeSection> &sections, Common::CodePage encoding, Common::U32String &text) {
	Common::String sjisText;
	if (!readExecutableString(executableData, virtualAddress, imageBase, sections, sjisText))
		return false;

	text = Common::U32String(sjisText.c_str(), sjisText.size(), encoding);
	return true;
}

} // End of anonymous namespace

bool BCBEngine::loadExecutableDataTables() {
	const ExecutableDataProfile &profile = *getExecutableDataProfile(_gameDescription->desc.language);
	_executableTextEncoding = profile.encoding;
	Common::Array<byte> executableData;
	if (!readExecutableBytes(createProgramReadStream(profile.executableName), executableData))
		return false;

	if (executableData.size() < 0x40 || executableData[0] != 'M' || executableData[1] != 'Z')
		return false;

	const uint32 peHeaderOffset = readUint32LE(executableData, 0x3c);
	if (peHeaderOffset + 0x18 >= executableData.size() || executableData[peHeaderOffset] != 'P' || executableData[peHeaderOffset + 1] != 'E')
		return false;

	const uint sectionCount = readUint16LE(executableData, peHeaderOffset + 6);
	const uint optionalHeaderSize = readUint16LE(executableData, peHeaderOffset + 20);
	const uint optionalHeaderOffset = peHeaderOffset + 24;
	const uint sectionTableOffset = optionalHeaderOffset + optionalHeaderSize;
	if (sectionTableOffset + sectionCount * 40 > executableData.size())
		return false;

	const uint32 imageBase = readUint32LE(executableData, optionalHeaderOffset + 28);
	Common::Array<PeSection> sections;
	for (uint i = 0; i < sectionCount; ++i) {
		const uint sectionOffset = sectionTableOffset + i * 40;
		PeSection section;
		const uint32 virtualSize = readUint32LE(executableData, sectionOffset + 8);
		section.virtualAddress = readUint32LE(executableData, sectionOffset + 12);
		section.rawSize = readUint32LE(executableData, sectionOffset + 16);
		section.rawOffset = readUint32LE(executableData, sectionOffset + 20);
		section.size = MAX<uint32>(virtualSize, section.rawSize);
		sections.push_back(section);
	}

	uint tableOffset = 0;
	uint tableEndOffset = 0;
	if (!virtualAddressToFileOffset(profile.sceneChoiceTextTableVa, imageBase, sections, tableOffset))
		return false;
	if (!virtualAddressToFileOffset(profile.sceneChoiceTextTableEndVa, imageBase, sections, tableEndOffset))
		return false;

	const uint setCount = (tableEndOffset - tableOffset) / (kSceneChoiceTextSlotsPerSet * 4);
	_sceneChoiceText.clear();
	_sceneChoiceText.resize(setCount + 1);

	for (uint setId = 1; setId <= setCount; ++setId) {
		for (uint slot = 0; slot < kSceneChoiceTextSlotsPerSet; ++slot) {
			const uint pointerOffset = tableOffset + ((setId - 1) * kSceneChoiceTextSlotsPerSet + slot) * 4;
			const uint32 textVa = readUint32LE(executableData, pointerOffset);
			uint textOffset = 0;
			if (textVa == 0 || !virtualAddressToFileOffset(textVa, imageBase, sections, textOffset))
				break;

			uint textEnd = textOffset;
			while (textEnd < executableData.size() && executableData[textEnd] != 0)
				++textEnd;
			if (textEnd == textOffset)
				break;

			const char *textStart = reinterpret_cast<const char *>(&executableData[textOffset]);
			_sceneChoiceText[setId].push_back(Common::U32String(textStart, textEnd - textOffset, profile.encoding));
		}
	}

	if (!readExecutableString(executableData, profile.sceneChoiceMoveTextVa, imageBase, sections, _sceneChoiceMoveTextSjis))
		return false;
	_sceneChoiceMoveText = Common::U32String(_sceneChoiceMoveTextSjis.c_str(), _sceneChoiceMoveTextSjis.size(), profile.encoding);

	uint mapTextTableOffset = 0;
	if (!virtualAddressToFileOffset(profile.mapButtonTextPointerTableVa, imageBase, sections, mapTextTableOffset))
		return false;

	_mapButtonTextSjis.clear();
	_mapButtonText.clear();
	_mapButtonTextSjis.resize(kMapButtonTextCount);
	_mapButtonText.resize(kMapButtonTextCount);
	for (uint i = 0; i < kMapButtonTextCount; ++i) {
		const uint pointerOffset = mapTextTableOffset + i * 4;
		const uint32 textVa = readUint32LE(executableData, pointerOffset);
		uint textOffset = 0;
		if (textVa == 0 || !virtualAddressToFileOffset(textVa, imageBase, sections, textOffset))
			continue;

		uint textEnd = textOffset;
		while (textEnd < executableData.size() && executableData[textEnd] != 0)
			++textEnd;

		if (textEnd == textOffset)
			continue;

		const char *textStart = reinterpret_cast<const char *>(&executableData[textOffset]);
		_mapButtonTextSjis[i] = Common::String(textStart, textEnd - textOffset);
		_mapButtonText[i] = Common::U32String(textStart, textEnd - textOffset, profile.encoding);
	}

	uint gameOverTextTableOffset = 0;
	if (!virtualAddressToFileOffset(profile.gameOverTextPointerTableVa, imageBase, sections, gameOverTextTableOffset))
		return false;

	_gameOverText.clear();
	_gameOverText.resize(kGameOverTextCount);
	for (uint i = 0; i < kGameOverTextCount; ++i) {
		const uint pointerOffset = gameOverTextTableOffset + i * 4;
		const uint32 textVa = readUint32LE(executableData, pointerOffset);
		uint textOffset = 0;
		if (textVa == 0 || !virtualAddressToFileOffset(textVa, imageBase, sections, textOffset))
			continue;

		uint textEnd = textOffset;
		while (textEnd < executableData.size() && executableData[textEnd] != 0)
			++textEnd;

		if (textEnd == textOffset)
			continue;

		const char *textStart = reinterpret_cast<const char *>(&executableData[textOffset]);
		_gameOverText[i] = Common::U32String(textStart, textEnd - textOffset, profile.encoding);
	}

	if (!readExecutableString(executableData, profile.titleExitConfirmTextVa, imageBase, sections, profile.encoding, _titleExitConfirmText))
		return false;
	if (!readExecutableString(executableData, profile.saveSlotOccupiedTextFormatVa, imageBase, sections, _saveSlotOccupiedTextFormatSjis))
		return false;
	if (!readExecutableString(executableData, profile.saveSlotEmptyTextFormatVa, imageBase, sections, _saveSlotEmptyTextFormatSjis))
		return false;
	if (!readExecutableString(executableData, profile.loadSlotConfirmTextFormatVa, imageBase, sections, _loadSlotConfirmTextFormatSjis))
		return false;
	if (!readExecutableString(executableData, profile.saveSlotOverwriteConfirmTextFormatVa, imageBase, sections, _saveSlotOverwriteConfirmTextFormatSjis))
		return false;
	if (!readExecutableString(executableData, profile.saveSlotEmptyConfirmTextFormatVa, imageBase, sections, _saveSlotEmptyConfirmTextFormatSjis))
		return false;
	if (!readExecutableString(executableData, profile.noSaveDataMessageVa, imageBase, sections, _noSaveDataMessageSjis))
		return false;
	if (!readExecutableString(executableData, profile.returnToTitleConfirmTextVa, imageBase, sections, _returnToTitleConfirmTextSjis))
		return false;

	return true;
}

Common::U32String BCBEngine::decodeExecutableTextForDisplay(const Common::String &text) const {
	return Common::U32String(text.c_str(), text.size(), _executableTextEncoding);
}

} // End of namespace BCB

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

#include "common/file.h"

#include "hollywood/monsters/detection.h"
#include "hollywood/monsters/graphics.h"

namespace Hollywood {
namespace {

const ADGameDescription fallbackWindowsDescription = {
	"hollywood",
	"",
	AD_ENTRY1(nullptr, nullptr),
	Common::UNK_LANG,
	Common::kPlatformWindows,
	ADGF_UNSTABLE,
	GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
};

const ADGameDescription fallbackDosDescription = {
	"hollywood",
	"Unknown DOS version",
	AD_ENTRY1(nullptr, nullptr),
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_UNSTABLE,
	GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
};


enum Resource000FallbackEntry {
	kFallbackOptionsFramebufferEntry = 0x2a,
	kFallbackInventoryPagesEntry = 0x2b,
	kFallbackBottomPanelEntry = 0x2c,
	kFallbackDialoguePanelEntry = 0x2f,
	kFallbackObjectPaletteEntry = 0x31
};

const uint kFallbackResource000HeaderByteCount = 1;
const uint kFallbackResource000OffsetTableSize = 400;
const uint kFallbackResource000SizeTableSize = 400;
const uint kFallbackMinimumInventoryPagesSize = 0x7e000;
const uint kFallbackMinimumBottomPanelSize = 0x41d8;
const uint kFallbackMinimumDialoguePanelSourceSize = 45 * 1024;
const uint kFallbackObjectPaletteSize = 0x60;

bool readResource000Entry(Common::File &file, uint entryIndex, uint32 &offset, uint32 &size) {
	const uint offsetTablePosition = kFallbackResource000HeaderByteCount + entryIndex * 4;
	const uint sizeTablePosition = kFallbackResource000HeaderByteCount +
		kFallbackResource000OffsetTableSize + entryIndex * 4;
	if (sizeTablePosition + 4 > (uint32)file.size())
		return false;

	file.seek(offsetTablePosition);
	offset = file.readUint32LE();
	file.seek(sizeTablePosition);
	size = file.readUint32LE();

	return !file.err() && size != 0 && offset <= (uint32)file.size() &&
		size <= (uint32)file.size() - offset;
}

bool hasResource000Entry(Common::File &file, uint entryIndex, uint32 minimumSize) {
	uint32 offset = 0;
	uint32 size = 0;
	return readResource000Entry(file, entryIndex, offset, size) && size >= minimumSize;
}

bool hasValidResource000StartupLayout(const AdvancedMetaEngineBase::FileMap &allFiles) {
	if (!allFiles.contains("RESOURCE.000"))
		return false;

	Common::File file;
	if (!file.open(allFiles["RESOURCE.000"]))
		return false;

	const uint32 startupTablesSize = kFallbackResource000HeaderByteCount +
		kFallbackResource000OffsetTableSize + kFallbackResource000SizeTableSize;
	if ((uint32)file.size() < startupTablesSize)
		return false;

	if (!hasResource000Entry(file, kFallbackOptionsFramebufferEntry, kSceneBufferByteCount))
		return false;
	if (!hasResource000Entry(file, kFallbackInventoryPagesEntry, kFallbackMinimumInventoryPagesSize))
		return false;
	if (!hasResource000Entry(file, kFallbackBottomPanelEntry, kFallbackMinimumBottomPanelSize))
		return false;
	if (!hasResource000Entry(file, kFallbackDialoguePanelEntry, kFallbackMinimumDialoguePanelSourceSize))
		return false;
	if (!hasResource000Entry(file, kFallbackObjectPaletteEntry, kFallbackObjectPaletteSize))
		return false;

	return true;
}

Common::Platform detectExecutablePlatform(const AdvancedMetaEngineBase::FileMap &allFiles) {
	if (!allFiles.contains("MONSTERS.EXE"))
		return Common::kPlatformUnknown;

	Common::File file;
	if (!file.open(allFiles["MONSTERS.EXE"]) || file.size() < 0x40)
		return Common::kPlatformUnknown;

	file.seek(0);
	const byte mzHeader[2] = { file.readByte(), file.readByte() };
	if (mzHeader[0] != 'M' || mzHeader[1] != 'Z')
		return Common::kPlatformUnknown;

	file.seek(0x3c);
	const uint32 peHeaderOffset = file.readUint32LE();
	if (peHeaderOffset + 4 <= (uint32)file.size()) {
		file.seek(peHeaderOffset);
		if (file.readByte() == 'P' && file.readByte() == 'E' &&
				file.readByte() == 0 && file.readByte() == 0)
			return Common::kPlatformWindows;
	}

	return Common::kPlatformDOS;
}

} // End of anonymous namespace

const ADFileBasedFallback monstersFileBasedFallback[] = {
	{ &fallbackDosDescription, { "MONSTERS.EXE", "RESOURCE.000", "RESOURCE.003", "RESOURCE.004", nullptr } },
	{ nullptr, { nullptr } }
};

const ADGameDescription *detectMonstersVariant(const AdvancedMetaEngineBase::FileMap &allFiles) {
	if (!hasValidResource000StartupLayout(allFiles))
		return nullptr;

	const Common::Platform platform = detectExecutablePlatform(allFiles);
	if (platform == Common::kPlatformWindows)
		return &fallbackWindowsDescription;
	if (platform == Common::kPlatformDOS)
		return &fallbackDosDescription;
	return nullptr;
}

} // End of namespace Hollywood

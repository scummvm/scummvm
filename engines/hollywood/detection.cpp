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

#include "base/plugins.h"

#include "common/file.h"

#include "engines/advancedDetector.h"
#include "engines/game.h"

#include "hollywood/detection.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const PlainGameDescriptor plainGameDescriptors[] = {
	{"hollywood", "Hollywood Monsters"},
	{nullptr, nullptr}
};

const DebugChannelDef debugFlagList[] = {
	{Hollywood::kDebugGeneral, "general", "General debug level"},
	{Hollywood::kDebugResources, "resources", "Resource archive loading"},
	{Hollywood::kDebugScene, "scene", "Scene state and presentation"},
	{Hollywood::kDebugPath, "path", "Pathfinding and actor route tracing"},
	DEBUG_CHANNEL_END
};

const ADGameDescription gameDescriptions[] = {
	// Spanish Windows first edition full game, installed MONSTERS directory.
	{
		"hollywood",
		"1st edition",
		AD_ENTRY4s("MONSTERS.EXE", "1129cea40124683fc4aabaeaf6478f5a", 1167360,
			"RESOURCE.000", "b16a8300ca1af75a0322e443a8971159", 3662162,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153,
			"RESOURCE.004", "a79d937879665b899c255957c7310d42", 265002432),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Spanish Windows full game, installed MONSTERS directory.
	{
		"hollywood",
		nullptr,
		AD_ENTRY3s("MONSTERS.EXE", "5b097e04e6ca7225d9dea6cd99909f6d", 1431552,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Spanish DOS CD full game, MONSTERS directory from the CD.
	{
		"hollywood",
		nullptr,
		AD_ENTRY3s("MONSTERS.EXE", "54a1e8749448e08086a1929510ec4b6a", 2513867,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

static const ADGameDescription fallbackWindowsDescription = {
	"hollywood",
	"",
	AD_ENTRY1(nullptr, nullptr),
	Common::UNK_LANG,
	Common::kPlatformWindows,
	ADGF_UNSTABLE,
	GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
};

static const ADGameDescription fallbackDosDescription = {
	"hollywood",
	"Unknown DOS version",
	AD_ENTRY1(nullptr, nullptr),
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_UNSTABLE,
	GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &fallbackDosDescription, { "MONSTERS.EXE", "RESOURCE.000", "RESOURCE.003", "RESOURCE.004", nullptr } },
	{ nullptr, { nullptr } }
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
const uint kFallbackOptionsFramebufferSize = 0x78000;
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

	if (!hasResource000Entry(file, kFallbackOptionsFramebufferEntry, kFallbackOptionsFramebufferSize))
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

class HollywoodMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	HollywoodMetaEngineDetection() : AdvancedMetaEngineDetection(Hollywood::gameDescriptions, plainGameDescriptors) {
		_flags = kADFlagPreferFallbackDetection;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist,
			ADDetectedGameExtraInfo **extra) const override {
		ADDetectedGame game = detectGameFilebased(allFiles, Hollywood::fileBasedFallback);
		if (!game.desc)
			return ADDetectedGame();

		if (!hasValidResource000StartupLayout(allFiles))
			return ADDetectedGame();

		const Common::Platform platform = detectExecutablePlatform(allFiles);
		if (platform == Common::kPlatformWindows)
			game.desc = &Hollywood::fallbackWindowsDescription;
		else if (platform == Common::kPlatformDOS)
			game.desc = &Hollywood::fallbackDosDescription;
		else
			return ADDetectedGame();

		return game;
	}

	const char *getName() const override {
		return "hollywood";
	}

	const char *getEngineName() const override {
		return "Hollywood Monsters";
	}

	const char *getOriginalCopyright() const override {
		return "Hollywood Monsters (C) 1997 Pendulo Studios";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace Hollywood

REGISTER_PLUGIN_STATIC(HOLLYWOOD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hollywood::HollywoodMetaEngineDetection);

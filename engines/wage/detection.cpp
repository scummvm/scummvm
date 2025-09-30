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
#include "common/macresman.h"

#include "engines/advancedDetector.h"

static const PlainGameDescriptor wageGames[] = {
	{"afm", "Another Fine Mess"},
	{"amot", "A Mess O' Trouble"},
	{"cantitoe", "Camp Cantitoe"},
	{"drakmythcastle", "Drakmyth Castle"},
	{"grailquest", "GrailQuest: Adventure in the Age of King Arthur"},
	{"raysmaze", "Ray's Maze"},
	{"scepters", "Enchanted Scepters"},
	{"twisted", "Twisted!"},
	{"worldbuilder", "World Builder"},
	{"wage", "WAGE"},
	{0, 0}
};

#include "wage/detection_tables.h"
#include "wage/detection.h"

static const DebugChannelDef debugFlagList[] = {
	{Wage::kDebugImGui, "imgui", "Show ImGui debug window (if available)"},
	DEBUG_CHANNEL_END
};

static ADGameDescription s_fallbackDesc = {
	"wage",
	"",
	AD_ENTRY1(0, 0),
	Common::EN_ANY,
	Common::kPlatformMacintosh,
	ADGF_NO_FLAGS,
	GUIO0()
};

class WageMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	mutable Common::String _filenameStr;

public:
	WageMetaEngineDetection() : AdvancedMetaEngineDetection(Wage::gameDescriptions, wageGames) {
		_md5Bytes = 2 * 1024 * 1024;
		_guiOptions = GUIO3(GUIO_NOSPEECH, GUIO_NOMIDI, GAMEOPTION_TTS);
	}

	const char *getName() const override {
		return "wage";
	}

	const char *getEngineName() const override {
		return "World Adventure Game Engine";
	}

	const char *getOriginalCopyright() const override {
		return "World Builder (C) Silicon Beach Software";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;
};

ADDetectedGame WageMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	SearchMan.addDirectory("WageMetaEngineDetection::fallbackDetect", fslist.begin()->getParent());

	for (Common::FSList::const_iterator fs = fslist.begin(); fs != fslist.end(); ++fs) {
		if (fs->isDirectory())
			continue;

		Common::Path filePath = Common::Path(fs->getPathInArchive());
		Common::MacResManager resManager;
		if (!resManager.open(filePath)) {
			continue;
		}

		Common::MacFinderInfo finderInfo;
		if (resManager.getFileFinderInfo(filePath, finderInfo)) {
			if (READ_BE_UINT32(finderInfo.type) != MKTAG('A', 'P', 'P', 'L')) {
				continue;
			}
			if (READ_BE_UINT32(finderInfo.creator) != MKTAG('W', 'E', 'D', 'T')) {
				continue;
			}

			Common::Path outPath(fs->getFileName());
			_filenameStr = outPath.toString();

			s_fallbackDesc.filesDescriptions[0].fileName = _filenameStr.c_str();

			ADDetectedGame game;
			game.desc = &s_fallbackDesc;

			FileProperties tmp;
			if (getFileProperties(allFiles, kMD5MacResFork, filePath, tmp)) {
				game.hasUnknownFiles = true;
				game.matchedFiles[filePath] = tmp;
			}

			SearchMan.remove("WageMetaEngineDetection::fallbackDetect");
			return game;
		}
	}

	SearchMan.remove("WageMetaEngineDetection::fallbackDetect");
	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(WAGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, WageMetaEngineDetection);

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

#include "engines/advancedDetector.h"

#include "common/file.h"
#include "common/winexe.h"

#include "director/detection.h"
#include "director/director.h"

#include "director/detection_tables.h"
#include "director/detection_paths.h"

static struct CustomTarget {
	const char *name;
	const char *platform;
	const char *version;
} customTargetList[] = {
	{"d2-mac", "mac", "200" },
	{"d3-mac", "mac", "300" },
	{"d4-mac", "mac", "400" },
	{"d3-win", "win", "300" },
	{"d4-win", "win", "400" },
	{"director-movie", "win", "400" },
	{ nullptr, nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{Director::kDebug32bpp, "32bpp", "Work in 32bpp mode"},
	{Director::kDebugCompile, "compile", "Lingo Compilation"},
	{Director::kDebugCompileOnly, "compileonly", "Skip Lingo code execution"},
	{Director::kDebugDesktop, "desktop", "Show the Classic Mac desktop"},
	{Director::kDebugEndVideo, "endvideo", "Fake that the end of video is reached setting"},
	{Director::kDebugEvents, "events", "Event processing"},
	{Director::kDebugFast, "fast", "Fast (no delay) playback"},
	{Director::kDebugFewFramesOnly, "fewframesonly", "Only run the first 10 frames"},
	{Director::kDebugImages, "images", "Image drawing"},
	{Director::kDebugLingoExec, "lingoexec", "Lingo Execution"},
	{Director::kDebugLingoStrict, "lingostrict", "Drop into debugger on Lingo error"},
	{Director::kDebugLoading, "loading", "Loading"},
	{Director::kDebugNoBytecode, "nobytecode", "Do not execute Lscr bytecode"},
	{Director::kDebugNoLoop, "noloop", "Do not loop the playback"},
	{Director::kDebugParse, "parse", "Lingo code parsing"},
	{Director::kDebugPreprocess, "preprocess", "Lingo preprocessing"},
	{Director::kDebugScreenshot, "screenshot", "screenshot each frame"},
	{Director::kDebugSlow, "slow", "Slow playback"},
	{Director::kDebugText, "text", "Text rendering"},
	DEBUG_CHANNEL_END
};

class DirectorMetaEngineDetection : public AdvancedMetaEngineDetection {
private:
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _customTarget;

public:
	DirectorMetaEngineDetection() : AdvancedMetaEngineDetection(Director::gameDescriptions, sizeof(Director::DirectorGameDescription), directorGames) {
		_maxScanDepth = 5;
		_directoryGlobs = Director::directoryGlobs;
		_flags = kADFlagMatchFullPaths;

		// initialize customTarget hashmap here
		for (int i = 0; customTargetList[i].name != nullptr; i++)
			_customTarget[customTargetList[i].name] = true;
	}

	const char *getEngineId() const override {
		return "director";
	}

	const char *getName() const override {
		return "Macromedia Director";
	}

	const char *getOriginalCopyright() const override {
		return "Macromedia Director (C) 1990-1995 Macromedia";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	bool canPlayUnknownVariants() const override { return true; }

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extraInfo) const override;
};

static Director::DirectorGameDescription s_fallbackDesc = {
	{
		"director",
		"",
		AD_ENTRY1(nullptr, nullptr),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_TAILMD5,	// We calculate tail of the projector
		GUIO0()
	},
	Director::GID_GENERIC,
	0
};

static char s_fallbackFileNameBuffer[51];
static char s_fallbackExtraBuf[256];

ADDetectedGame DirectorMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extraInfo) const {
	// TODO: Handle Mac fallback

	// reset fallback description
	Director::DirectorGameDescription *desc = &s_fallbackDesc;
	desc->desc.gameId = "director";
	desc->desc.extra = "";
	desc->desc.language = Common::UNK_LANG;
	desc->desc.flags = ADGF_TAILMD5;
	desc->desc.platform = Common::kPlatformWindows;
	desc->desc.guiOptions = GUIO0();
	desc->desc.filesDescriptions[0].fileName = nullptr;
	desc->version = 0;
	desc->gameGID = Director::GID_GENERIC;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();

		// first we check the custom target, check whether the filename is in the list of custom target
		// then we read 4 string from it, targetID, gameName, platform and version
		if (_customTarget.contains(fileName)) {
			Common::File f;
			if (!f.open(*file))
				continue;

			Common::String targetID, gameName, platform, version, tmp;

			// First, fill the info based on the filename
			for (int i = 0; customTargetList[i].name != nullptr; i++) {
				if (fileName.equalsIgnoreCase(customTargetList[i].name)) {
					targetID = "director-fallback";
					platform = customTargetList[i].platform;
					version = customTargetList[i].version;
					gameName = Common::String::format("Director Movie %s/v%s", platform.c_str(), version.c_str());
				}
			}

			// Now try to read info from the file
			if (!(tmp = f.readString('\n')).empty())
				targetID = tmp;

			if (!(tmp = f.readString('\n')).empty())
				gameName = tmp;

			if (!(tmp = f.readString('\n')).empty())
				platform = tmp;

			if (!(tmp = f.readString('\n')).empty())
				version = tmp;

			desc->version = atoi(version.c_str());
			desc->desc.platform = Common::parsePlatform(platform);

			// if we have extra info slots
			if (extraInfo != nullptr) {
				*extraInfo = new ADDetectedGameExtraInfo;
				(*extraInfo)->targetID = targetID;
				(*extraInfo)->gameName = gameName;
			}

			ADDetectedGame game(&desc->desc);
			return game;
		}

		if (!fileName.hasSuffix(".exe"))
			continue;

		Common::File f;
		if (!f.open(*file))
			continue;

		f.seek(-4, SEEK_END);

		uint32 offset = f.readUint32LE();

		if (f.eos() || offset == 0 || offset >= (uint32)(f.size() - 4))
			continue;

		f.seek(offset);

		uint32 tag = f.readUint32LE();

		switch (tag) {
		case MKTAG('P', 'J', '9', '3'):
		case MKTAG('3', '9', 'J', 'P'):
			desc->version = 400;
			break;
		case MKTAG('P', 'J', '9', '5'):
			desc->version = 500;
			break;
		case MKTAG('P', 'J', '9', '7'):
			desc->version = 600;
			break;
		case MKTAG('P', 'J', '0', '0'):
			desc->version = 700;
			break;
		case MKTAG('P', 'J', '0', '1'):
			desc->version = 800;
			break;
		default:
			// Prior to version 4, there was no tag here. So we'll use a bit of a
			// heuristic to detect. The first field is the entry count, of which
			// there should only be one.
			if ((tag & 0xFFFF) != 1)
				continue;

			f.skip(3);

			uint32 mmmSize = f.readUint32LE();

			if (f.eos() || mmmSize == 0)
				continue;

			byte fileNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(fileNameSize);
			byte directoryNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(directoryNameSize);

			if (f.pos() != f.size() - 4)
				continue;

			// Assume v3 at this point (for now at least)
			desc->version = 300;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		desc->desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		Common::String extra;
		Common::WinResources *exe = Common::WinResources::createFromEXE(&f);
		if (exe) {
			Common::WinResources::VersionInfo *versionInfo = exe->getVersionResource(1);
			if (versionInfo) {
				extra = Common::String::format("v%d.%d.%dr%d", versionInfo->fileVersion[0], versionInfo->fileVersion[1], versionInfo->fileVersion[2], versionInfo->fileVersion[3]);
				delete versionInfo;
			}
			delete exe;
		}
		if (extra.empty()) {
			extra = Common::String::format("v%d.%02d", desc->version / 100, desc->version % 100);
		}
		Common::strlcpy(s_fallbackExtraBuf, extra.c_str(), sizeof(s_fallbackExtraBuf) - 1);
		desc->desc.extra = s_fallbackExtraBuf;

		warning("Director fallback detection %s (uses tail MD5)", extra.c_str());

		ADDetectedGame game(&desc->desc);

		FileProperties tmp;
		if (getFileProperties(allFiles, desc->desc, file->getName(), tmp)) {
			game.hasUnknownFiles = true;
			game.matchedFiles[file->getName()] = tmp;
		}

		return game;
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(DIRECTOR_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DirectorMetaEngineDetection);

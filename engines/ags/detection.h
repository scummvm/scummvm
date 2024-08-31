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

#ifndef AGS_DETECTION_H
#define AGS_DETECTION_H

#include "engines/advancedDetector.h"

namespace AGS {

enum AGSDebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath     = 1 << 1,
	kDebugScan     = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript   = 1 << 4
};

enum GameFlag {
	GAMEFLAG_PLUGINS_MASK = 0xff,
	GAMEFLAG_PLUGINS_NONE = 0,
	GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE = 1,
	GAMEFLAG_PLUGINS_AGS_FLASHLIGHT = 2,
	GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP = 3,

	GAMEFLAG_FORCE_AA = 1 << 8,
};

struct AGSGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;

	uint32 features;
};

extern const PlainGameDescriptor GAME_NAMES[];

extern const AGSGameDescription GAME_DESCRIPTIONS[];

#define GAMEOPTION_NO_SAVE_THUMBNAIL GUIO_GAMEOPTIONS1
#define GAMEOPTION_NO_AUTOSAVE		 GUIO_GAMEOPTIONS2
#define GAMEOPTION_NO_SAVELOAD		 GUIO_GAMEOPTIONS3

} // namespace AGS


class AGSMetaEngineDetection : public AdvancedMetaEngineDetection<AGS::AGSGameDescription> {
	mutable Common::String _gameid;
	mutable Common::String _extra;
	mutable Common::String _filenameStr;
	mutable Common::String _md5;

	static const DebugChannelDef debugFlagList[];

public:
	AGSMetaEngineDetection();
	~AGSMetaEngineDetection() override {}

	const char *getName() const override {
		return "ags";
	}

	const char *getEngineName() const override {
		return "Adventure Game Studio";
	}

	const char *getOriginalCopyright() const override {
		return "AGS Engine (C) Chris Jones";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra = nullptr) const override;
};

#endif

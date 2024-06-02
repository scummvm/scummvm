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

#ifndef TWP_DETECTION_H
#define TWP_DETECTION_H

#include "engines/advancedDetector.h"

namespace Twp {

enum TwpDebugChannels {
	kDebugText = 1,
	kDebugGGPack,
	kDebugRes,
	kDebugDialog,
	kDebugGenScript,
	kDebugObjScript,
	kDebugSysScript,
	kDebugRoomScript,
	kDebugActScript,
	kDebugSndScript,
	kDebugGame,
	kDebugConsole,
	kDebugSave
};

enum GameXorKey {
	GAME_XORKEY_56AD,
	GAME_XORKEY_566D,
	GAME_XORKEY_5B6D,
	GAME_XORKEY_5BAD,
};

enum LanguageSupported {
	LS_WITH_RUSSIAN,
	LS_WITHOUT_RUSSIAN
};

extern const PlainGameDescriptor twpGames[];
struct TwpGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;
	GameXorKey xorKey;
	LanguageSupported languageSupported;
};

extern const TwpGameDescription gameDescriptions[];

} // End of namespace Twp

class TwpMetaEngineDetection : public AdvancedMetaEngineDetection<Twp::TwpGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	TwpMetaEngineDetection();
	~TwpMetaEngineDetection() override = default;

	const char *getName() const override {
		return "twp";
	}

	const char *getEngineName() const override {
		return "Thimbleweed Park";
	}

	const char *getOriginalCopyright() const override {
		return "Thimbleweed Park (C) Terrible Toybox";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	/** Convert an AD game description into the shared game description format. */
	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override;
};

#endif // TWP_DETECTION_H

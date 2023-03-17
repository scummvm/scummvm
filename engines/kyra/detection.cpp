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

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "kyra/detection.h"
#include "kyra/detection_tables.h"
#include "kyra/kyra_v1.h"

namespace {

static const DebugChannelDef debugFlagList[] = {
	{Kyra::kDebugLevelScriptFuncs, "ScriptFuncs", "Script function debug level"},
	{Kyra::kDebugLevelScript, "Script", "Script interpreter debug level"},
	{Kyra::kDebugLevelSprites, "Sprites", "Sprite debug level"},
	{Kyra::kDebugLevelScreen, "Screen", "Screen debug level"},
	{Kyra::kDebugLevelSound, "Sound", "Sound debug level"},
	{Kyra::kDebugLevelAnimator, "Animator", "Animator debug level"},
	{Kyra::kDebugLevelMain, "Main", "Generic debug level"},
	{Kyra::kDebugLevelGUI, "GUI", "GUI debug level"},
	{Kyra::kDebugLevelSequence, "Sequence", "Sequence debug level"},
	{Kyra::kDebugLevelMovie, "Movie", "Movie debug level"},
	{Kyra::kDebugLevelTimer, "Timer", "Timer debug level"},
	DEBUG_CHANNEL_END
};

const char *const directoryGlobs[] = {
	"malcolm",
	"data", // LOL GOG release
	"runtime",
	nullptr
};

} // End of anonymous namespace

class KyraMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	KyraMetaEngineDetection() : AdvancedMetaEngineDetection(adGameDescs, sizeof(KYRAGameDescription), gameList) {
		_md5Bytes = 1024 * 1024;
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "kyra";
	}

	const char *getEngineName() const override {
		return "Kyra";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const char *getOriginalCopyright() const override {
		return "The Legend of Kyrandia (C) Westwood Studios"
#ifdef ENABLE_LOL
		       "\nLands of Lore (C) Westwood Studios"
#endif
#ifdef ENABLE_EOB
		       "\nEye of the Beholder (C) TSR, Inc., (C) Strategic Simulations, Inc."
#endif
		       ;
	}
};

REGISTER_PLUGIN_STATIC(KYRA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, KyraMetaEngineDetection);

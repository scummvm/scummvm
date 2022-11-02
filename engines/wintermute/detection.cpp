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

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/util.h"
#include "common/translation.h"

#include "engines/metaengine.h"

#include "wintermute/detection.h"
#include "wintermute/detection_tables.h"
#include "wintermute/wintermute.h"

static const DebugChannelDef debugFlagList[] = {
	{Wintermute::kWintermuteDebugLog, "enginelog", "Covers the same output as the log-file in WME"},
	{Wintermute::kWintermuteDebugSaveGame, "savegame", "Savegames"},
	{Wintermute::kWintermuteDebugFont, "font", "Text-drawing-related messages"},
	{Wintermute::kWintermuteDebugFileAccess, "file-access", "Non-critical problems like missing files"},
	{Wintermute::kWintermuteDebugAudio, "audio", "audio-playback-related issues"},
	{Wintermute::kWintermuteDebugGeneral, "general", "various issues not covered by any of the above"},
	DEBUG_CHANNEL_END
};

namespace Wintermute {

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_SHOW_FPS,
		{
			_s("Show FPS-counter"),
			_s("Show the current number of frames per second in the upper left corner"),
			"show_fps",
			false,
			0,
			0
		},
	},

	{
		GAMEOPTION_BILINEAR,
		{
			_s("Sprite bilinear filtering (SLOW)"),
			_s("Apply bilinear filtering to individual sprites"),
			"bilinear_filtering",
			false,
			0,
			0
		}
	},

#ifdef ENABLE_WME3D
	{
		GAMEOPTION_FORCE_2D_RENDERER,
		{
			_s("Force to use 2D renderer (2D games only)"),
			_s("This setting forces ScummVM to use 2D renderer while running 2D games"),
			"force_2d_renderer",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const char *directoryGlobs[] = {
	"language", // To detect the various languages
	"languages", // To detect the various languages
	"localization", // To detect the various languages
	0
};

class WintermuteMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	WintermuteMetaEngineDetection() : AdvancedMetaEngineDetection(Wintermute::gameDescriptions, sizeof(WMEGameDescription), Wintermute::wintermuteGames, gameGuiOptions) {
		// Use kADFlagUseExtraAsHint to distinguish between SD and HD versions
		// of J.U.L.I.A. when their datafiles sit in the same directory (e.g. in Steam distribution).
		_flags = kADFlagUseExtraAsHint;
#ifdef ENABLE_WME3D
		_guiOptions = GUIO4(GUIO_NOMIDI, GAMEOPTION_SHOW_FPS, GAMEOPTION_BILINEAR, GAMEOPTION_FORCE_2D_RENDERER);
#else
		_guiOptions = GUIO3(GUIO_NOMIDI, GAMEOPTION_SHOW_FPS, GAMEOPTION_BILINEAR);
#endif
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "wintermute";
	}

	const char *getEngineName() const override {
		return "Wintermute";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) 2011 Jan Nedoma";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override {
		/**
		 * Fallback detection for Wintermute heavily depends on engine resources, so it's not possible
		 * to use them without the engine present in a clean way.
		 */

		if (ConfMan.hasKey("always_run_fallback_detection_extern")) {
			if (ConfMan.getBool("always_run_fallback_detection_extern") == false) {
				warning("WINTERMUTE: Fallback detection is disabled.");
				return ADDetectedGame();
			}
		}

		const Plugin *metaEnginePlugin = EngineMan.findPlugin(getName());

		if (metaEnginePlugin) {
			const Plugin *enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
			if (enginePlugin) {
				return enginePlugin->get<AdvancedMetaEngine>().fallbackDetectExtern(_md5Bytes, allFiles, fslist);
			} else {
				static bool warn = true;
				if (warn) {
					warning("Engine plugin for Wintermute not present. Fallback detection is disabled.");
					warn = false;
				}
			}
		}
		return ADDetectedGame();
	}

};

} // End of namespace Wintermute

REGISTER_PLUGIN_STATIC(WINTERMUTE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Wintermute::WintermuteMetaEngineDetection);

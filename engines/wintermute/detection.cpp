/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

namespace Wintermute {

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_SHOW_FPS,
		{
			_s("Show FPS-counter"),
			_s("Show the current number of frames per second in the upper left corner"),
			"show_fps",
			false
		},
	},

	{
		GAMEOPTION_BILINEAR,
		{
			_s("Sprite bilinear filtering (SLOW)"),
			_s("Apply bilinear filtering to individual sprites"),
			"bilinear_filtering",
			false
		}
	},

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
		_guiOptions = GUIO3(GUIO_NOMIDI, GAMEOPTION_SHOW_FPS, GAMEOPTION_BILINEAR);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "wintermute";
	}

	const char *getName() const override {
		return "Wintermute";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) 2011 Jan Nedoma";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override {
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

		const Plugin *metaEnginePlugin = EngineMan.findPlugin(getEngineId());

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

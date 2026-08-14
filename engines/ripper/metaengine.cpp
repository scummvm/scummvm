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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/plugins.h"

#include "common/translation.h"

#include "ripper/detection.h"
#include "ripper/metaengine.h"
#include "ripper/ripper.h"

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_SKIP_INTRO,
		{
			_s("Skip intro video"),
			_s("Skip the startup logo video before loading a saved game or showing the main menu."),
			"skip_intro",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SUBTITLES,
		{
			_s("Subtitles"),
			_s("Show authored subtitle text during supported videos and audio presentations."),
			"ripper_subtitles",
			true,
			0,
			1
		}
	},
	{
		GAMEOPTION_SUBTITLE_AUTOSCROLL,
		{
			_s("Automatically scroll subtitles"),
			_s("Automatically advance authored subtitle text while supported videos and audio presentations play."),
			"ripper_subtitle_autoscroll",
			true,
			1,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *RipperMetaEngine::getName() const {
	return "ripper";
}

const ADExtraGuiOptionsMap *RipperMetaEngine::getAdvancedExtraGuiOptions() const {
	return gameGuiOptions;
}

Common::Error RipperMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Ripper::RipperEngine(syst, desc);
	return Common::kNoError;
}

bool RipperMetaEngine::hasFeature(MetaEngineFeature feature) const {
	return checkExtendedSaves(feature) || feature == kSupportsLoadingDuringStartup;
}

#if PLUGIN_ENABLED_DYNAMIC(RIPPER)
REGISTER_PLUGIN_DYNAMIC(RIPPER, PLUGIN_TYPE_ENGINE, RipperMetaEngine);
#else
REGISTER_PLUGIN_STATIC(RIPPER, PLUGIN_TYPE_ENGINE, RipperMetaEngine);
#endif

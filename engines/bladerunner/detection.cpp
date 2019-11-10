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


#include "bladerunner/bladerunner.h"
#include "bladerunner/detection_tables.h"
#include "bladerunner/savefile.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

namespace BladeRunner {

static const PlainGameDescriptor bladeRunnerGames[] = {
	{"bladerunner", "Blade Runner"},
	{"bladerunner-final", "Blade Runner with restored content"},
	{0, 0}
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_SITCOM,
		{
			_s("Sitcom mode"),
			_s("Game will add laughter after actor's line or narration"),
			"sitcom",
			false
		}
	},
	{
		GAMEOPTION_SHORTY,
		{
			_s("Shorty mode"),
			_s("Game will shrink the actors and make their voices high pitched"),
			"shorty",
			false
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_NODELAYMILLIS,
		{
			_s("Frame limiter high performance mode"),
			_s("This mode may result in high CPU usage! It avoids use of delayMillis() function."),
			"nodelaymillisfl",
			false
		}
	},
	{
		GAMEOPTION_FRAMELIMITER_FPS,
		{
			_s("Max frames per second limit"),
			_s("This mode targets a maximum of 120 fps. When disabled, the game targets 60 fps"),
			"frames_per_secondfl",
			false
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace BladeRunner

class BladeRunnerMetaEngine : public AdvancedMetaEngine {
public:
	BladeRunnerMetaEngine();

	const char *getName() const override;
	const char *getOriginalCopyright() const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

BladeRunnerMetaEngine::BladeRunnerMetaEngine()
	: AdvancedMetaEngine(
		BladeRunner::gameDescriptions,
		sizeof(BladeRunner::gameDescriptions[0]),
		BladeRunner::bladeRunnerGames,
		BladeRunner::optionsList) {}


const char *BladeRunnerMetaEngine::getName() const {
	return "Blade Runner";
}

const char *BladeRunnerMetaEngine::getOriginalCopyright() const {
	return "Blade Runner (C) 1997 Westwood Studios";
}

bool BladeRunnerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new BladeRunner::BladeRunnerEngine(syst, desc);

	return true;
}

bool BladeRunnerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		f == kSupportsListSaves ||
		f == kSupportsLoadingDuringStartup ||
		f == kSupportsDeleteSave ||
		f == kSavesSupportMetaInfo ||
		f == kSavesSupportThumbnail ||
		f == kSavesSupportCreationDate ||
		f == kSavesSupportPlayTime ||
		f == kSimpleSavesNames;
}

SaveStateList BladeRunnerMetaEngine::listSaves(const char *target) const {
	return BladeRunner::SaveFileManager::list(target);
}

int BladeRunnerMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void BladeRunnerMetaEngine::removeSaveState(const char *target, int slot) const {
	BladeRunner::SaveFileManager::remove(target, slot);
}

SaveStateDescriptor BladeRunnerMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return BladeRunner::SaveFileManager::queryMetaInfos(target, slot);
}

#if PLUGIN_ENABLED_DYNAMIC(BLADERUNNER)
	REGISTER_PLUGIN_DYNAMIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BLADERUNNER, PLUGIN_TYPE_ENGINE, BladeRunnerMetaEngine);
#endif

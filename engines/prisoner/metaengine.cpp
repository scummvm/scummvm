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

#include "common/translation.h"

#include "prisoner/metaengine.h"
#include "prisoner/detection.h"
#include "prisoner/prisoner.h"

namespace Prisoner {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original menus"),
			_s("Use the original save/load, sound volume and text and music screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_MIDI_MUSIC,
		{
			_s("Use MIDI music"),
			_s("Play the MIDI soundtrack through the selected music device instead of the digital one"),
			"midi_music",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Prisoner

const char *PrisonerMetaEngine::getName() const {
	return "prisoner";
}

const ADExtraGuiOptionsMap *PrisonerMetaEngine::getAdvancedExtraGuiOptions() const {
	return Prisoner::optionsList;
}

Common::Error PrisonerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Prisoner::PrisonerEngine(syst, desc);
	return Common::kNoError;
}

bool PrisonerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesUseExtendedFormat);
}

bool Prisoner::PrisonerEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

int PrisonerMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void PrisonerMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	((Prisoner::PrisonerEngine *)g_engine)->getSavegameThumbnail(thumb);
}

#if PLUGIN_ENABLED_DYNAMIC(PRISONER)
REGISTER_PLUGIN_DYNAMIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#endif

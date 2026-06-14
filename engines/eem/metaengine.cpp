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

#include "common/translation.h"

#include "eem/detection.h"
#include "eem/eem.h"

#include "common/system.h"

namespace EEM {

const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_HIDE_HIGHLIGHT_BOXES,
		{
			_s("Hide the highlight boxes"),
			_s("Hide the boxes that highlight searchable clue locations."),
			"hide_highlight_boxes",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FIT_DIALOG_BALLOONS,
		{
			_s("Better fit for dialog balloons"),
			_s("Pick a smaller speech-bubble graphic when the wrapped "
			   "text doesn't need the original size."),
			"fit_dialog_balloons",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SKIP_REPEATED_CASES,
		{
			_s("Skip repeated cases"),
			_s("Skip all Book 2 cases and jump to Book 3 once Book 1 is complete."),
			"skip_repeated_cases",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_RESTORED_CONTENT,
		{
			_s("Enable restored content"),
			_s("Restore floppy release extras in the CD version, including pre-scrapbook conversations and first-try badges."),
			"restored_content",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *EEMEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Platform EEMEngine::getPlatform() const {
	return _gameDescription->platform;
}

} // End of namespace EEM

class EEMMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "eem";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return EEM::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new EEM::EEMEngine(syst, desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return checkExtendedSaves(f) ||
			   f == kSupportsLoadingDuringStartup;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(EEM)
REGISTER_PLUGIN_DYNAMIC(EEM, PLUGIN_TYPE_ENGINE, EEMMetaEngine);
#else
REGISTER_PLUGIN_STATIC(EEM, PLUGIN_TYPE_ENGINE, EEMMetaEngine);
#endif

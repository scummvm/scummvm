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

#include "freescape/freescape.h"
#include "freescape/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_PRERECORDED_SOUNDS,
		{
			_s("Prerecorded sounds"),
			_s("Use high-quality pre-recorded sounds instead of pc speaker emulation"),
			"prerecorded_sounds",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class FreescapeMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "freescape";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
};

Common::Error FreescapeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (Common::String(gd->gameId) == "driller" || Common::String(gd->gameId) == "spacestationoblivion") {
		*engine = (Engine *)new Freescape::DrillerEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "darkside") {
		*engine = (Engine *)new Freescape::DarkEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "totaleclipse") {
		*engine = (Engine *)new Freescape::EclipseEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "castlemaster") {
		*engine = (Engine *)new Freescape::CastleEngine(syst, gd);
	} else
		*engine = new Freescape::FreescapeEngine(syst, gd);

	return Common::kNoError;
}

namespace Freescape {

bool FreescapeEngine::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // namespace Freescape

#if PLUGIN_ENABLED_DYNAMIC(FREESCAPE)
REGISTER_PLUGIN_DYNAMIC(FREESCAPE, PLUGIN_TYPE_ENGINE, FreescapeMetaEngine);
#else
REGISTER_PLUGIN_STATIC(FREESCAPE, PLUGIN_TYPE_ENGINE, FreescapeMetaEngine);
#endif

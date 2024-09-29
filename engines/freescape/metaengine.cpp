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
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "graphics/thumbnail.h"
#include "graphics/scaler.h"


#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/games/dark/dark.h"
#include "freescape/games/driller/driller.h"
#include "freescape/games/eclipse/eclipse.h"
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
	{
		GAMEOPTION_EXTENDED_TIMER,
		{
			_s("Extended timer"),
			_s("Start the game timer at 99:59:59"),
			"extended_timer",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_AUTOMATIC_DRILLING,
		{
			_s("Automatic drilling"),
			_s("Allow to successfully drill in any part of the area in Driller"),
			"automatic_drilling",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_DEMO_MODE,
		{
			_s("Disable demo mode"),
			_s("Never activate demo mode"),
			"disable_demo_mode",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_SENSORS,
		{
			_s("Disable sensors"),
			_s("Sensors will not shoot the player"),
			"disable_sensors",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_SENSORS,
		{
			_s("Disable falling"),
			_s("Player cannot fall over edges"),
			"disable_falling",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_INVERT_Y,
		{
			_s("Invert Y-axis on mouse"),
			_s("Use alternative camera controls"),
			"invert_y",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_AUTHENTIC_GRAPHICS,
		{
			_s("Authentic graphics"),
			_s("Keep graphics as close as possible to the original"),
			"authentic_graphics",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_TRAVEL_ROCK,
		{
			_s("Enable rock travel"),
			_s("Enable traveling using a rock shoot at start"),
			"rock_travel",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class FreescapeMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "freescape";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error FreescapeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (Common::String(gd->gameId) == "driller" || Common::String(gd->gameId) == "spacestationoblivion") {
		*engine = (Engine *)new Freescape::DrillerEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "darkside") {
		*engine = (Engine *)new Freescape::DarkEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "totaleclipse" || Common::String(gd->gameId) == "totaleclipse2") {
		*engine = (Engine *)new Freescape::EclipseEngine(syst, gd);
	} else if (Common::String(gd->gameId) == "castlemaster") {
		*engine = (Engine *)new Freescape::CastleEngine(syst, gd);
	} else
		*engine = new Freescape::FreescapeEngine(syst, gd);

	return Common::kNoError;
}

Common::KeymapArray FreescapeMetaEngine::initKeymaps(const char *target) const {
	using namespace Freescape;

	FreescapeEngine *engine = (Freescape::FreescapeEngine *)g_engine;
	Common::Keymap *engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "freescape", "Freescape game");
	Common::Keymap *infoScreenKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "infoscreen-keymap", "Information screen keymapping");

	if (engine)
		engine->initKeymaps(engineKeyMap, infoScreenKeyMap, target);

	Common::KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = infoScreenKeyMap;

	return keymaps;
}

void FreescapeMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	Freescape::FreescapeEngine *engine = (Freescape::FreescapeEngine *)g_engine;
	assert(engine->_savedScreen);
	Graphics::Surface *scaledSavedScreen = scale(*engine->_savedScreen, kThumbnailWidth, kThumbnailHeight2);
	assert(scaledSavedScreen);
	thumb.copyFrom(*scaledSavedScreen);

	scaledSavedScreen->free();
	delete scaledSavedScreen;
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

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

#include "engines/advancedDetector.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "graphics/managed_surface.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"

#include "mtropolis/actions.h"
#include "mtropolis/debug.h"
#include "mtropolis/detection.h"
#include "mtropolis/mtropolis.h"

#include "mtropolis/mtropolis.h"

namespace Graphics {

struct Surface;

} // End of namespace Graphics

namespace MTropolis {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_WIDESCREEN_MOD,
		{
			_s("16:9 widescreen mod"),
			_s("Removes letterboxing and moves some display elements, improving coverage on widescreen displays"),
			"mtropolis_mod_obsidian_widescreen",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DYNAMIC_MIDI,
		{
			_s("Improved music mixing"),
			_s("Enables dynamic MIDI mixer, improving quality, but behaving less like mTropolis Player."),
			"mtropolis_mod_dynamic_midi",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS,
		{
			_s("Autosave at progress points"),
			_s("Automatically saves the game after completing puzzles and chapters."),
			"mtropolis_mod_auto_save_at_checkpoints",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_ENABLE_SHORT_TRANSITIONS,
		{
			_s("Enable short transitions"),
			_s("Enables transitions that are set to maximum rate instead of skipping them."),
			"mtropolis_mod_minimum_transition_duration",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_SOUND_EFFECT_SUBTITLES,
		{
			_s("Enable subtitles for important sound effects"),
			_s("Enables subtitles for important sound effects.  This may reduce the difficulty of sound recognition puzzles and minigames."),
			"mtropolis_mod_sound_gameplay_subtitles",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_LAUNCH_DEBUG,
		{
			_s("Start with debugger"),
			_s("Starts with the debugger dashboard active."),
			"mtropolis_debug_at_start",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 MTropolisEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform MTropolisEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace MTropolis

class MTropolisMetaEngine : public AdvancedMetaEngine<MTropolis::MTropolisGameDescription> {
public:
	const char *getName() const override {
		return "mtropolis";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return MTropolis::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const MTropolis::MTropolisGameDescription *desc) const override;

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

bool MTropolisMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f);
}

Common::Error MTropolisMetaEngine::createInstance(OSystem *syst, Engine **engine, const MTropolis::MTropolisGameDescription *desc) const {
	*engine = new MTropolis::MTropolisEngine(syst, reinterpret_cast<const MTropolis::MTropolisGameDescription *>(desc));
	return Common::kNoError;
}


Common::Array<Common::Keymap *> MTropolisMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "mtropolis", "mTropolis");
	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keymap->addAction(act);

	act = new Common::Action(Common::kStandardActionMiddleClick, _("Middle Click"));
	act->setMiddleClickEvent();
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->addDefaultInputMapping("JOY_X");
	keymap->addAction(act);

	act = new Common::Action("DEBUG_TOGGLE_OVERLAY", _("Toggle debug overlay"));
	act->setCustomEngineActionEvent(MTropolis::Actions::kDebugToggleOverlay);
	act->addDefaultInputMapping("F10");
	keymap->addAction(act);

	act = new Common::Action("DEBUG_SKIP_MOVIES", _("Force any playing movies to end"));
	act->setCustomEngineActionEvent(MTropolis::Actions::kDebugSkipMovies);
	keymap->addAction(act);

	return Common::Keymap::arrayOf(keymap);
}

void MTropolisMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	const Graphics::Surface *savegameScreenshot = static_cast<MTropolis::MTropolisEngine *>(g_engine)->getSavegameScreenshot();

	if (savegameScreenshot) {
		int thumbnailWidth = kThumbnailWidth;
		int thumbnailHeight = thumbnailWidth * savegameScreenshot->h / savegameScreenshot->w;
		if (thumbnailHeight > kThumbnailHeight2) {
			thumbnailHeight = kThumbnailHeight2;
			thumbnailWidth = thumbnailHeight * savegameScreenshot->w / savegameScreenshot->h;
		}

		Common::SharedPtr<Graphics::ManagedSurface> outSurface(new Graphics::ManagedSurface());
		outSurface->create(savegameScreenshot->w, savegameScreenshot->h, Graphics::createPixelFormat<888>());

		for (int y = 0; y < savegameScreenshot->h; y++) {
			for (int x = 0; x < savegameScreenshot->w; x++) {
				uint8 r, g, b;
				savegameScreenshot->format.colorToRGB(savegameScreenshot->getPixel(x, y), r, g, b);
				outSurface->setPixel(x, y, outSurface->format.RGBToColor(r, g, b));
			}
		}

		while (outSurface->w >= thumbnailWidth * 2) {
			Common::SharedPtr<Graphics::ManagedSurface> temp(new Graphics::ManagedSurface());
			temp->create(outSurface->w / 2, outSurface->h, Graphics::createPixelFormat<888>());

			for (int y = 0; y < temp->h; y++) {
				for (int x = 0; x < temp->w; x++) {
					uint32 px1 = outSurface->getPixel(x * 2, y);
					uint32 px2 = outSurface->getPixel(x * 2 + 1, y);

					uint8 r1, g1, b1;
					outSurface->format.colorToRGB(px1, r1, g1, b1);

					uint8 r2, g2, b2;
					outSurface->format.colorToRGB(px2, r2, g2, b2);

					temp->setPixel(x, y, temp->format.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));
				}
			}

			outSurface = temp;
		}

		while (outSurface->h >= thumbnailHeight * 2) {
			Common::SharedPtr<Graphics::ManagedSurface> temp(new Graphics::ManagedSurface());
			temp->create(outSurface->w, outSurface->h / 2, Graphics::createPixelFormat<888>());

			for (int y = 0; y < temp->h; y++) {
				for (int x = 0; x < temp->w; x++) {
					uint32 px1 = outSurface->getPixel(x, y * 2);
					uint32 px2 = outSurface->getPixel(x, y * 2 + 1);

					uint8 r1, g1, b1;
					outSurface->format.colorToRGB(px1, r1, g1, b1);

					uint8 r2, g2, b2;
					outSurface->format.colorToRGB(px2, r2, g2, b2);

					temp->setPixel(x, y, temp->format.RGBToColor((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));
				}
			}

			outSurface = temp;
		}

		// TODO: Fix this for weird sizes
		Common::SharedPtr<Graphics::ManagedSurface> changeTo16Temp = outSurface;
		outSurface.reset(new Graphics::ManagedSurface());
		outSurface->create(changeTo16Temp->w, changeTo16Temp->h, Graphics::createPixelFormat<565>());

		for (int y = 0; y < outSurface->h; y++) {
			for (int x = 0; x < outSurface->w; x++) {
				uint32 px = changeTo16Temp->getPixel(x, y);

				uint8 r, g, b;
				changeTo16Temp->format.colorToRGB(px, r, g, b);

				outSurface->setPixel(x, y, outSurface->format.RGBToColor(r, g, b));
			}
		}

		changeTo16Temp.reset();

		thumb.copyFrom(*outSurface);
	} else {
		AdvancedMetaEngine::getSavegameThumbnail(thumb);
	}
}

#if PLUGIN_ENABLED_DYNAMIC(MTROPOLIS)
REGISTER_PLUGIN_DYNAMIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#else
REGISTER_PLUGIN_STATIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#endif

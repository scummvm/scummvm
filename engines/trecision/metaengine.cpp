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
#include "graphics/surface.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "trecision/trecision.h"
#include "trecision/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class TrecisionMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
	const char *getName() const override {
		return "trecision";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error TrecisionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Trecision::TrecisionEngine(syst, desc);
	return Common::kNoError;
}

void TrecisionMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	// We are referencing g_engine here, but this should be safe, as this
	// method is only used while the engine is running.
	// TODO: Is there a better way to do this?

	Trecision::TrecisionEngine *engine = (Trecision::TrecisionEngine *)g_engine;

	if (engine->_controlPanelSave)
		thumb.copyFrom(engine->_thumbnail);
	else
		MetaEngine::getSavegameThumbnail(thumb);
}

SaveStateDescriptor TrecisionMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> saveFile(g_system->getSavefileManager()->openForLoading(
		getSavegameFile(slot, target)));

	if (saveFile) {
		const byte version = saveFile->readByte();

		if (version >= SAVE_VERSION_ORIGINAL_MIN && version <= SAVE_VERSION_ORIGINAL_MAX) {
			// Original saved game, convert
			Common::String saveName = saveFile->readString(0, 40);

			SaveStateDescriptor desc(this, slot, saveName);

			// This is freed inside SaveStateDescriptor
			const Graphics::PixelFormat kImageFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
			Graphics::Surface *thumbnail = new Graphics::Surface();
			thumbnail->create(ICONDX, ICONDY, kImageFormat);
			saveFile->read(thumbnail->getPixels(), ICONDX * ICONDY * kImageFormat.bytesPerPixel);
			desc.setThumbnail(thumbnail);

			return desc;
		} else if (version >= SAVE_VERSION_SCUMMVM_MIN) {
			saveFile->seek(0);
			return MetaEngine::querySaveMetaInfos(target, slot);
		}
	}

	return SaveStateDescriptor();
}

Common::KeymapArray TrecisionMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Trecision;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "trecision-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip video"));
	act->setCustomEngineActionEvent(kActionSkipVideo);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	// I18N: Toggles walking speed of actor
	act = new Action("FASTWALK", _("Toggle fast walk"));
	act->setCustomEngineActionEvent(kActionFastWalk);
	act->addDefaultInputMapping("CAPSLOCK");
	act->addDefaultInputMapping("JOY_CENTER");
	gameKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause game"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("q");
	act->addDefaultInputMapping("Q");
	act->addDefaultInputMapping("JOY_LEFT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("SYSMENU", _("Open system menu"));
	act->setCustomEngineActionEvent(kActionSystemMenu);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("SAVEGAME", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action("LOADGAME", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Action("YESKEY", _("Press \"Yes\" key"));
	act->setCustomEngineActionEvent(kActionYes);
	act->addDefaultInputMapping("y");
	act->addDefaultInputMapping("j");
	act->addDefaultInputMapping("JOY_RIGHT_STICK");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

bool Trecision::TrecisionEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsSubtitleOptions) ||
		   (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime) ||
		   (f == kSupportsChangingOptionsDuringRuntime);
}

#if PLUGIN_ENABLED_DYNAMIC(TRECISION)
	REGISTER_PLUGIN_DYNAMIC(TRECISION, PLUGIN_TYPE_ENGINE, TrecisionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TRECISION, PLUGIN_TYPE_ENGINE, TrecisionMetaEngine);
#endif

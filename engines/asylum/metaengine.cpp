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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/translation.h"

#include "engines/achievements.h"
#include "engines/advancedDetector.h"

#include "graphics/scaler.h"

#include "asylum/system/savegame.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/shared.h"

class AsylumMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "asylum";
	}

	const char *getOriginalCopyright() const {
		return "Sanitarium (c) ASC Games";
	}

	int getMaximumSaveSlot() const override { return 24; }
	int getAutosaveSlot()    const override { return getMaximumSaveSlot(); }
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
	const Common::AchievementDescriptionList *getAchievementDescriptionList() const override;
};

bool Asylum::AsylumEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void AsylumMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	Asylum::AsylumEngine *engine = (Asylum::AsylumEngine *)g_engine;

	if (engine->isMenuVisible()) {
		const Graphics::Surface &savedScreen = engine->scene()->getSavedScreen();
		::createThumbnail(&thumb, (const byte *)savedScreen.getPixels(), savedScreen.w, savedScreen.h, engine->scene()->getSavedPalette());
	} else {
		::createThumbnailFromScreen(&thumb);
	}
}

SaveStateDescriptor AsylumMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc = MetaEngine::querySaveMetaInfos(target, slot);

	if (desc.getSaveSlot() == -1) {
		Common::InSaveFile *in(g_system->getSavefileManager()->openForLoading(getSavegameFile(slot, target)));

		if (in) {
			if (in->size() > 60) {
				(void)(uint32)Asylum::Savegame::read(in, "Chapter");
				desc.setSaveSlot(slot);
				desc.setDescription(Asylum::Savegame::read(in, 45, "Game Name"));
			}

			delete in;
		}
	}

	return desc;
}

Common::Error AsylumMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Asylum::AsylumEngine(syst, desc);
	}
	return desc ? Common::kNoError : Common::kUnsupportedGameidError;
}

Common::KeymapArray AsylumMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Asylum;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "asylum", "Sanitarium");

	Action *act;

	act = new Action("VERSION", _("Show version"));
	act->setCustomEngineActionEvent(kAsylumActionShowVersion);
	act->addDefaultInputMapping("v");
	engineKeyMap->addAction(act);

	act = new Action("LOAD", _("Quick load"));
	act->setCustomEngineActionEvent(kAsylumActionQuickLoad);
	act->addDefaultInputMapping("S+l");
	engineKeyMap->addAction(act);

	act = new Action("SAVE", _("Quick save"));
	act->setCustomEngineActionEvent(kAsylumActionQuickSave);
	act->addDefaultInputMapping("S+s");
	engineKeyMap->addAction(act);

	act = new Action("SARAH", _("Switch to Sarah"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToSarah);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	act = new Action("GRIMWALL", _("Switch to Grimwall"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToGrimwall);
	act->addDefaultInputMapping("g");
	engineKeyMap->addAction(act);

	act = new Action("OLMEC", _("Switch to Olmec"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToOlmec);
	act->addDefaultInputMapping("o");
	engineKeyMap->addAction(act);

	act = new Action("MENU", _("Bring up the In-Game Menu"));
	act->setCustomEngineActionEvent(kAsylumActionShowMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Open character inventory"));
	act->setCustomEngineActionEvent(kAsylumActionOpenInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Move up"));
	act->setCustomEngineActionEvent(kAsylumActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Move down"));
	act->setCustomEngineActionEvent(kAsylumActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Move left"));
	act->setCustomEngineActionEvent(kAsylumActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Move right"));
	act->setCustomEngineActionEvent(kAsylumActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	Keymap *resviewerKeyMap = new Keymap(Keymap::kKeymapTypeGame, "asylum-resviewer", "Sanitarium - Resource viewer");
	resviewerKeyMap->setEnabled(false);

	act = new Action(kStandardActionMoveUp, _("Move up"));
	act->setCustomEngineActionEvent(kAsylumActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Move down"));
	act->setCustomEngineActionEvent(kAsylumActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Move left"));
	act->setCustomEngineActionEvent(kAsylumActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Move right"));
	act->setCustomEngineActionEvent(kAsylumActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	act = new Action("ANIMATE", _("Toggle animation"));
	act->setCustomEngineActionEvent(kAsylumActionAnimate);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	resviewerKeyMap->addAction(act);

	act = new Action("CANCEL", _("Return to gameplay"));
	act->setCustomEngineActionEvent(kAsylumActionShowMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	resviewerKeyMap->addAction(act);

	// I18N: Switch to previous image in internal game resource viewer
	act = new Action("PREVRESOURCE", _("Previous resource"));
	act->setCustomEngineActionEvent(kAsylumActionPreviousResource);
	act->addDefaultInputMapping("BACKSPACE");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	// I18N: Switch to next image in internal game resource viewer
	act = new Action("NEXTRESOURCE", _("Next resource"));
	act->setCustomEngineActionEvent(kAsylumActionNextResource);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	// I18N: Switch to previous palette in internal game resource viewer
	act = new Action("PREVPALETTE", _("Previous palette"));
	act->setCustomEngineActionEvent(kAsylumActionPreviousPalette);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	// I18N: Switch to next palette in internal game resource viewer
	act = new Action("NEXTPALETTE", _("Next palette"));
	act->setCustomEngineActionEvent(kAsylumActionNextPalette);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	act->allowKbdRepeats();
	resviewerKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = resviewerKeyMap;

	return keymaps;
}

const Common::AchievementDescriptionList *AsylumMetaEngine::getAchievementDescriptionList() const {
	static const Common::AchievementDescriptionList achievementDescriptionList[] = {
		{"asylum", Common::STEAM_ACHIEVEMENTS, "284050"},

		ACHIEVEMENT_DESC_TABLE_END_MARKER
	};

	return achievementDescriptionList;
}

#if PLUGIN_ENABLED_DYNAMIC(ASYLUM)
	REGISTER_PLUGIN_DYNAMIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#endif

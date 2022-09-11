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

#include "common/achievements.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "graphics/scaler.h"

#include "asylum/system/savegame.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/shared.h"

class AsylumMetaEngine : public AdvancedMetaEngine {
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
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Open character inventory"));
	act->setCustomEngineActionEvent(kAsylumActionOpenInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

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

	return Keymap::arrayOf(engineKeyMap);
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

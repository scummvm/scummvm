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

#include "common/savefile.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "phoenixvr/detection.h"
#include "phoenixvr/game_state.h"
#include "phoenixvr/metaengine.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

static const ADExtraGuiOptionsMap optionsList[] = {
	{GAMEOPTION_ORIGINAL_SAVELOAD,
	 {_s("Use original save/load screens"),
	  _s("Use the original save/load screens instead of the ScummVM ones"),
	  "original_menus",
	  false,
	  0,
	  0}},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR};

} // End of namespace PhoenixVR

const char *PhoenixVRMetaEngine::getName() const {
	return "phoenixvr";
}

const ADExtraGuiOptionsMap *PhoenixVRMetaEngine::getAdvancedExtraGuiOptions() const {
	return PhoenixVR::optionsList;
}

Common::Error PhoenixVRMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new PhoenixVR::PhoenixVREngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray PhoenixVRMetaEngine::initKeymaps(const char *target) const {
	auto *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "phoenixvr", "Default keymap for PhoenixVR");

	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Action"));
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("RETURN");
	act->setLeftClickEvent();
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Inventory"));
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_B");
	act->setRightClickEvent();
	keyMap->addAction(act);

	act = new Common::Action("QUIT", _("Quit game"));
	act->addDefaultInputMapping("C+q");
	act->setEvent(Common::EventType::EVENT_QUIT);
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionSkip, _("Skip cutscene"));
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	act->setKeyEvent(Common::KeyState{Common::KEYCODE_SPACE, ' '});
	keyMap->addAction(act);

	act = new Common::Action("QUICKSAVE", _("Quick save"));
	act->addDefaultInputMapping("F5");
	act->setKeyEvent(Common::KeyState{Common::KEYCODE_F5});
	keyMap->addAction(act);

	act = new Common::Action("QUICKLOAD", _("Quick load"));
	act->addDefaultInputMapping("F8");
	act->setKeyEvent(Common::KeyState{Common::KEYCODE_F8});
	keyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionOpenMainMenu, _("Menu"));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_START");
	act->setKeyEvent(Common::KeyState{Common::KEYCODE_ESCAPE});
	keyMap->addAction(act);

	act = new Common::Action("HINT", _("Show hints"));
	act->addDefaultInputMapping("h");
	act->setKeyEvent(Common::KeyState{Common::KEYCODE_h});
	keyMap->addAction(act);

	return Common::Keymap::arrayOf(keyMap);
}

SaveStateList PhoenixVRMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (const auto &file : filenames) {
		auto dotPos = file.rfind('.');
		if (dotPos == file.npos)
			continue;
		int slotNum = atoi(file.c_str() + dotPos + 1);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			SaveStateDescriptor desc = querySaveMetaInfos(target, slotNum);
			saveList.push_back(desc);
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor PhoenixVRMetaEngine::querySaveMetaInfos(const char *target, int slotIdx) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	auto fname = getSavegameFile(slotIdx, target);
	Common::ScopedPtr<Common::InSaveFile> slot(saveFileMan->openForLoading(fname));
	if (!slot)
		return {};

	auto state = PhoenixVR::GameState::load(*slot);

	SaveStateDescriptor desc;
	desc.setSaveSlot(slotIdx);
	desc.setDeletableFlag(true);
	desc.setDescription(state.game + " " + state.info);
	Graphics::PixelFormat rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0);
	desc.setThumbnail(state.getThumbnail(rgb565, 160));
	return desc;
}

bool PhoenixVRMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSimpleSavesNames:
	case kSupportsListSaves:
	case kSupportsLoadingDuringStartup:
	case kSavesSupportThumbnail:
	case kSavesSupportMetaInfo:
		return true;
	default:
		return false;
	}
}

#if PLUGIN_ENABLED_DYNAMIC(PHOENIXVR)
REGISTER_PLUGIN_DYNAMIC(PHOENIXVR, PLUGIN_TYPE_ENGINE, PhoenixVRMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PHOENIXVR, PLUGIN_TYPE_ENGINE, PhoenixVRMetaEngine);
#endif

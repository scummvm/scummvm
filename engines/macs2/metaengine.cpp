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

#include "macs2/metaengine.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "engines/enhancements.h"
#include "macs2/detection.h"
#include "macs2/dialogs.h"
#include "macs2/macs2.h"

const char *Macs2MetaEngine::getName() const {
	return "macs2";
}

Common::KeymapArray Macs2MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Macs2;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "macs2", "Macs2");

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Interact"));
	act->setCustomEngineActionEvent(kMacs2ActionInteract);
	// Mouse buttons are handled via raw EVENT_LBUTTON* in Events::processEvent so
	// press/release pairs work (panel close on release). Gamepad only here.
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Change cursor mode"));
	act->setCustomEngineActionEvent(kMacs2ActionCursorMode);
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kMacs2ActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kMacs2ActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("MENU", _("Menu"));
	act->setCustomEngineActionEvent(kMacs2ActionMenu);
	act->addDefaultInputMapping("n");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("GAMESPEED", _("Cycle game speed"));
	act->setCustomEngineActionEvent(kMacs2ActionGameSpeed);
	act->addDefaultInputMapping("C+t");
	engineKeyMap->addAction(act);

	act = new Action("GMM", _("Open options menu"));
	act->setCustomEngineActionEvent(kMacs2ActionOpenGMM);
	act->addDefaultInputMapping("F5");
	engineKeyMap->addAction(act);

	act = new Action("HELP", _("Help / map"));
	act->setCustomEngineActionEvent(kMacs2ActionHelp);
	act->addDefaultInputMapping("F1");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionToggleHotspots, _("Show hotspots"));
	act->setEvent(Common::EVENT_HOTSPOTS_SHOW);
	act->addDefaultInputMapping("h");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::Error Macs2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Macs2::Macs2Engine(syst, desc);
	return Common::kNoError;
}

bool Macs2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		   (f == kSupportsLoadingDuringStartup);
}

GUI::OptionsContainerWidget *Macs2MetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new Macs2::Macs2OptionsWidget(boss, name, target);
}

void Macs2MetaEngine::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("original_menus", false);
	ConfMan.registerDefault("enhancements", kEnhGameBreakingBugFixes | kEnhGrp1);
	ConfMan.registerDefault("macs2_game_speed_mode", 0);
#ifdef USE_TTS
	ConfMan.registerDefault("tts_enabled", false);
#endif
}

SaveStateList Macs2MetaEngine::listSaves(const char *target) const {
	// Get standard ScummVM saves first
	SaveStateList saves = AdvancedMetaEngine::listSaves(target);

	// Also look for original DOS saves (SAVEGAME.0 through SAVEGAME.9)
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	for (int slot = 0; slot < 10; slot++) {
		Common::String origName = Common::String::format("SAVEGAME.%d", slot);
		Common::InSaveFile *f = saveFileMan->openForLoading(origName);
		if (!f) {
			// Also try lowercase
			origName = Common::String::format("savegame.%d", slot);
			f = saveFileMan->openForLoading(origName);
		}
		if (f) {
			// Validate magic
			char magic[12];
			f->read(magic, 12);
			if (memcmp(magic, "AHFFMSGM0100", 12) == 0) {
				// Read slot name (Pascal string: 1 byte length + up to 20 chars)
				byte nameLen = f->readByte();
				if (nameLen > 20)
					nameLen = 20;
				char name[21];
				f->read(name, nameLen);
				name[nameLen] = '\0';
				// Use slots 100+ for original saves to avoid conflicts
				int scummSlot = 100 + slot;
				// Check if this slot is already in the list
				bool found = false;
				for (const auto &s : saves) {
					if (s.getSaveSlot() == scummSlot) {
						found = true;
						break;
					}
				}
				if (!found) {
					Common::String desc = Common::String::format("[DOS] %s", name);
					saves.push_back(SaveStateDescriptor(this, scummSlot, desc));
				}
			}
			delete f;
		}
	}

	return saves;
}

#if PLUGIN_ENABLED_DYNAMIC(MACS2)
REGISTER_PLUGIN_DYNAMIC(MACS2, PLUGIN_TYPE_ENGINE, Macs2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(MACS2, PLUGIN_TYPE_ENGINE, Macs2MetaEngine);
#endif

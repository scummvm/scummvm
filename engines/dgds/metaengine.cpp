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
#include "common/system.h"
#include "common/translation.h"
#include "common/savefile.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"

#include "dgds/dgds.h"

class DgdsMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "dgds";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool DgdsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		   (f == kSupportsListSaves) ||
		   (f == kSupportsLoadingDuringStartup) ||
		   (f == kSupportsDeleteSave) ||
		   (f == kSavesSupportMetaInfo) ||
		   (f == kSavesSupportThumbnail) ||
		   (f == kSimpleSavesNames);
}

Common::Error DgdsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Dgds::DgdsEngine(syst, desc);

	return (desc != nullptr) ? Common::kNoError : Common::kUnknownError;
}

struct KeybindingRecord {
	Dgds::DgdsKeyEvent _action;
	const char *_id;
	const Common::U32String _desc;
	const char *_key;
	const char *_altKey;
};

Common::KeymapArray DgdsMetaEngine::initKeymaps(const char *target) const {
	const KeybindingRecord DGDS_KEYS[] = {
		{ Dgds::kDgdsKeyLoad, "LOAD", _("Load Game"), "F7", "C+l" },
		{ Dgds::kDgdsKeySave, "SAVE", _("Save Game"), "F5", "C+s" },
		{ Dgds::kDgdsKeyToggleMenu, "TOGGLE_MENU", _("Toggle Menu"), "ESCAPE", nullptr },
		{ Dgds::kDgdsKeyToggleClock, "TOGGLE_CLOCK", _("Toggle Clock"), "c", nullptr },
		{ Dgds::kDgdsKeyNextChoice, "NEXT_CHOICE", _("Next dialog / menu item"), "DOWN", "s" },
		{ Dgds::kDgdsKeyPrevChoice, "PREV_CHOICE", _("Previous dialog / menu item"), "UP", "a" },
		{ Dgds::kDgdsKeyNextItem, "NEXT_ITEM", _("Next object"), "TAB", "w" },
		{ Dgds::kDgdsKeyPrevItem, "PREV_ITEM", _("Previous object"), "S+TAB", "q" },
		{ Dgds::kDgdsKeyPickUp, "PICK_UP", _("Pick up / Operate"), "SPACE", "KP5" },
		{ Dgds::kDgdsKeyLook, "LOOK", _("Look"), "RETURN", "," },
		{ Dgds::kDgdsKeyActivate, "ACTIVATE", _("Activate Inventory Object"), "BACKSPACE", nullptr },
	};

	Common::Keymap *map = new Common::Keymap(Common::Keymap::kKeymapTypeGame, target, _("Game Keys"));

	for (const auto &k : DGDS_KEYS) {
		Common::Action *act = new Common::Action(k._id, k._desc);
		act->setCustomEngineActionEvent(k._action);
		act->addDefaultInputMapping(k._key);
		if (k._altKey)
			act->addDefaultInputMapping(k._altKey);
		map->addAction(act);
	}

	return Common::Keymap::arrayOf(map);
}

//
// Used for detecting original game saves
//
// Ideally save file magic should be compared to the one in the GDS file,
// but when loading from the launcher the game engine isn't created,
// so use a static list.
//
static const uint32 GAME_MAGICS[] {
	0x53E83426, // Rise of the Dragon
	0xFF553726, // Heart of China
	0x7ADA2628, // Adventures of Willy Beamish
};

SaveStateDescriptor DgdsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc = AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	if (!desc.isValid() && slot > 0) {
		const Common::String filename = getSavegameFile(slot, target);
		Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(filename));

		if (f) {
			uint16 origSlotNum = f->readUint16LE();
			const Common::String origName = f->readString();
			uint32 magic = f->readUint32BE();

			for (uint32 game_magic : GAME_MAGICS) {
				if (magic == game_magic && origSlotNum < 4096 && !origName.empty()) {
					desc.setDescription(origName);
					desc.setSaveSlot(slot);
					break;
				}
			}
		}
	}

	return desc;
}


#if PLUGIN_ENABLED_DYNAMIC(DGDS)
	REGISTER_PLUGIN_DYNAMIC(DGDS, PLUGIN_TYPE_ENGINE, DgdsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DGDS, PLUGIN_TYPE_ENGINE, DgdsMetaEngine);
#endif

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/algorithm.h"

#include "dune/detection.h"
#include "dune/dune.h"

class DuneMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "dune";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Dune::DuneEngine(syst, desc);
		return Common::kNoError;
	}

	// Periodic/pre-load autosaving must not replace an ordinary log. The
	// original's two entry checkpoints have their own filenames and triggers.
	int getAutosaveSlot() const override { return -1; }

	int getMaximumSaveSlot() const override { return Dune::kNewSietchSaveSlot; }

	Common::String getSavegameFile(int slot, const char *target = nullptr) const override {
		if (!target)
			target = getName();
		if (slot == Dune::kLastPlaceSaveSlot)
			return Common::String::format("%s.place-entry", target);
		if (slot == Dune::kNewSietchSaveSlot)
			return Common::String::format("%s.new-sietch-entry", target);
		return MetaEngine::getSavegameFile(slot, target);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		SaveStateDescriptor save = MetaEngine::querySaveMetaInfos(target, slot);
		if (slot == Dune::kLastPlaceSaveSlot || slot == Dune::kNewSietchSaveSlot)
			save.setWriteProtectedFlag(true);
		return save;
	}

	SaveStateList listSaves(const char *target) const override {
		SaveStateList saves = MetaEngine::listSaves(target);
		// Numbered logs 000..099 retain their existing names. Exclude any
		// higher numbered file before adding the separately named checkpoints.
		for (int i = (int)saves.size() - 1; i >= 0; i--) {
			if (saves[i].getSaveSlot() >= Dune::kLastPlaceSaveSlot)
				saves.remove_at(i);
		}
		for (int slot = Dune::kLastPlaceSaveSlot; slot <= Dune::kNewSietchSaveSlot; slot++) {
			SaveStateDescriptor save = querySaveMetaInfos(target, slot);
			if (!save.isValid()) {
				// Empty checkpoint rows must not become writable manual slots.
				save = SaveStateDescriptor(this, slot, slot == Dune::kLastPlaceSaveSlot
					? "Last entering into a place" : "Last entering new sietch");
				save.setLocked(true);
			}
			saves.push_back(save);
		}
		Common::sort(saves.begin(), saves.end(), SaveStateDescriptorSlotComparator());
		return saves;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return f == kSupportsListSaves
			|| f == kSupportsLoadingDuringStartup
			|| f == kSupportsDeleteSave
			|| f == kSavesSupportMetaInfo
			|| f == kSavesSupportThumbnail
			|| f == kSavesSupportCreationDate
			|| f == kSavesSupportPlayTime
			|| f == kSimpleSavesNames
			// The extended header is what appendExtendedSave writes and
			// querySaveMetaInfos reads; without it the save slots are
			// numbered from nought and no save is ever listed, so the
			// load dialog has nothing to load and the save dialog
			// believes every slot is taken.
			|| f == kSavesUseExtendedFormat;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(DUNE)
	REGISTER_PLUGIN_DYNAMIC(DUNE, PLUGIN_TYPE_ENGINE, DuneMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DUNE, PLUGIN_TYPE_ENGINE, DuneMetaEngine);
#endif

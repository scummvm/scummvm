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

#include "mutationofjb/mutationofjb.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "engines/advancedDetector.h"

class MutationOfJBMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "mutationofjb";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new MutationOfJB::MutationOfJBEngine(syst, desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		if (f == kSupportsListSaves || f == kSimpleSavesNames || f == kSupportsLoadingDuringStartup) {
			return true;
		}

		return false;
	}

	int getMaximumSaveSlot() const override {
		return 999;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::SaveFileManager *const saveFileMan = g_system->getSavefileManager();
		Common::StringArray filenames;
		Common::String pattern = target;
		pattern += ".###";

		filenames = saveFileMan->listSavefiles(pattern);

		SaveStateList saveList;
		int slotNo = 0;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			// Obtain the last 3 digits of the filename, since they correspond to the save slot
			slotNo = atoi(file->c_str() + file->size() - 3);

			Common::InSaveFile *const in = saveFileMan->openForLoading(*file);
			if (in) {
				Common::Serializer sz(in, nullptr);

				MutationOfJB::SaveHeader saveHdr;
				if (saveHdr.sync(sz)) {
					saveList.push_back(SaveStateDescriptor(slotNo, saveHdr._description));
				}
			}
		}
		return saveList;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(MUTATIONOFJB)
	REGISTER_PLUGIN_DYNAMIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MUTATIONOFJB, PLUGIN_TYPE_ENGINE, MutationOfJBMetaEngine);
#endif

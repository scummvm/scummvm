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

#include "ags/metaengine.h"
#include "ags/detection.h"
#include "ags/ags.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/filestream.h"
#include "ags/engine/ac/richgamemedia.h"
#include "ags/engine/game/savegame.h"
#include "common/savefile.h"

const char *AGSMetaEngine::getName() const {
	return "ags";
}

Common::Error AGSMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const AGS::AGSGameDescription *gd = (const AGS::AGSGameDescription *)desc;

	*engine = new AGS::AGSEngine(syst, gd);
	return Common::kNoError;
}

SaveStateList AGSMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		Common::String filename = Common::String::format("%s%s",
			::AGS3::AGS::Shared::SAVE_FOLDER_PREFIX, file->c_str());

		::AGS3::AGS::Shared::FileStream saveFile(filename, ::AGS3::AGS::Shared::kFile_Open,
			::AGS3::AGS::Shared::kFile_Read);
		if (saveFile.IsValid()) {
			AGS3::RICH_GAME_MEDIA_HEADER rich_media_header;
			rich_media_header.ReadFromFile(&saveFile);

			if (rich_media_header.dwMagicNumber == RM_MAGICNUMBER) {
				int slotNum = atoi(file->c_str() + file->size() - 3);

				SaveStateDescriptor desc;
				desc.setSaveSlot(slotNum);
				desc.setDescription(rich_media_header.getSaveName());

				if (slotNum == getAutosaveSlot())
					desc.setWriteProtectedFlag(true);

				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

#if PLUGIN_ENABLED_DYNAMIC(AGS)
REGISTER_PLUGIN_DYNAMIC(AGS, PLUGIN_TYPE_ENGINE, AGSMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AGS, PLUGIN_TYPE_ENGINE, AGSMetaEngine);
#endif

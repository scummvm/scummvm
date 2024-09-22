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

#include "ags/metaengine.h"
#include "ags/detection.h"
#include "ags/achievements_tables.h"
#include "ags/ags.h"
#include "ags/globals.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/file_stream.h"
#include "ags/engine/ac/rich_game_media.h"
#include "ags/engine/game/savegame.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "engines/achievements.h"
#include "image/bmp.h"

const char *AGSMetaEngine::getName() const {
	return "ags";
}

Common::Error AGSMetaEngine::createInstance(OSystem *syst, Engine **engine, const AGS::AGSGameDescription *gd) const {
	*engine = new AGS::AGSEngine(syst, gd);
	return Common::kNoError;
}

SaveStateList AGSMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

	filenames = saveFileMan->listSavefiles(pattern);

	int maxSlot = getMaximumSaveSlot();
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
				if (slotNum > maxSlot)
					continue;

				SaveStateDescriptor desc(this, slotNum, rich_media_header.getSaveName());
				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool AGSMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSupportsLoadingDuringStartup);
}

Common::String AGSMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	if (saveGameIdx == kSavegameFilePattern) {
		// Pattern requested
		return Common::String::format("%s.###", target == nullptr ? getName() : target);
	} else {
		// Specific filename requested
		return Common::String::format("%s.%03d", target == nullptr ? getName() : target, saveGameIdx);
	}
}

SaveStateDescriptor AGSMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s%s",
	                          ::AGS3::AGS::Shared::SAVE_FOLDER_PREFIX,
	                          getSavegameFile(slot, target).c_str());

	::AGS3::AGS::Shared::FileStream saveFile(filename, ::AGS3::AGS::Shared::kFile_Open,
	        ::AGS3::AGS::Shared::kFile_Read);
	if (saveFile.IsValid()) {
		AGS3::RICH_GAME_MEDIA_HEADER rich_media_header;
		rich_media_header.ReadFromFile(&saveFile);

		if (rich_media_header.dwMagicNumber == RM_MAGICNUMBER) {
			SaveStateDescriptor desc(this, slot, rich_media_header.getSaveName());

			// Thumbnail handling
			if (rich_media_header.dwThumbnailOffsetLowerDword != 0 &&
			        rich_media_header.dwThumbnailSize != 0) {
				// Read in the thumbnail data
				byte *thumbData = (byte *)malloc(rich_media_header.dwThumbnailSize);
				saveFile.Seek(rich_media_header.dwThumbnailOffsetLowerDword,
				              AGS3::AGS::Shared::kSeekCurrent);
				saveFile.Read(thumbData, rich_media_header.dwThumbnailSize);
				Common::MemoryReadStream thumbStream(thumbData,
				                                     rich_media_header.dwThumbnailSize, DisposeAfterUse::YES);

				// Decode the thumbnail
				Image::BitmapDecoder decoder;
				if (decoder.loadStream(thumbStream)) {
					const Graphics::Surface *src = decoder.getSurface();
					Graphics::Surface *dest;

					if (src->w == 160 && src->h == 100) {
						dest = new Graphics::Surface();
						dest->copyFrom(*src);
					} else {
						Graphics::ManagedSurface temp(160, 100, src->format);
						temp.blitFrom(*src, Common::Rect(0, 0, src->w, src->h),
							Common::Rect(0, 0, 160, 100));

						dest = new Graphics::Surface();
						dest->copyFrom(temp);
					}

					desc.setThumbnail(dest);
				}
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}

void AGSMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}

int AGSMetaEngine::getAutosaveSlot() const {
	if (!g_engine || !_G(noScummAutosave))
		return 0;
	else
		return -1;
}

const Common::AchievementDescriptionList* AGSMetaEngine::getAchievementDescriptionList() const {
	return AGS::achievementDescriptionList;
}

Common::StringArray AGSMetaEngine::getGameTranslations(const Common::String &domain) {
	Common::Path path = ConfMan.getPath("path", domain);
	Common::FSDirectory dir(path);
	Common::ArchiveMemberList traFileList;
	dir.listMatchingMembers(traFileList, "*.tra");
	Common::StringArray traFileNames;

	for (Common::ArchiveMemberList::iterator iter = traFileList.begin(); iter != traFileList.end(); ++iter) {
		Common::String traFileName = (*iter)->getName();
		traFileName.erase(traFileName.size() - 4); // remove .tra extension
		traFileNames.push_back(traFileName);
	}

	return traFileNames;
}

#if PLUGIN_ENABLED_DYNAMIC(AGS)
REGISTER_PLUGIN_DYNAMIC(AGS, PLUGIN_TYPE_ENGINE, AGSMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AGS, PLUGIN_TYPE_ENGINE, AGSMetaEngine);
#endif

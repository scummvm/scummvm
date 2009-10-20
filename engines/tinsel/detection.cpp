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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"

#include "tinsel/bmv.h"
#include "tinsel/cursor.h"
#include "tinsel/tinsel.h"
#include "tinsel/savescn.h"	// needed by TinselMetaEngine::listSaves

namespace Tinsel {

struct TinselGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 TinselEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TinselEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TinselEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform TinselEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 TinselEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor tinselGames[] = {
	{"tinsel", "Tinsel engine game"},
	{"dw", "Discworld"},
	{"dw2", "Discworld 2: Missing Presumed ...!?"},
	{0, 0}
};


namespace Tinsel {

using Common::GUIO_NONE;
using Common::GUIO_NOSPEECH;
using Common::GUIO_NOSFX;
using Common::GUIO_NOMUSIC;

static const TinselGameDescription gameDescriptions[] = {

	// Note: The following is the (hopefully) definitive list of version details:
	// TINSEL_V0: Used only by the Discworld 1 demo - this used a more primitive version
	//   of the Tinsel engine and graphics compression
	// TINSEL_V1: There were two versions of the Discworld 1 game - the first used .GRA
	//   files, and the second used .SCN files. The second also provided some fixes to
	//   various script bugs and coding errors, but is still considered TINSEL_V1,
	//   as both game versions work equally well with the newer code.
	// TINSEL_V2: The Discworld 2 game used this updated version of the Tinsel 1 engine,
	//   and as far as we know there aren't any variations of this engine.

	{	// Floppy Demo V0 from http://www.adventure-treff.de/specials/dl_demos.php
		{
			"dw",
			"Floppy Demo",
			AD_ENTRY1s("dw.gra", "ce1b57761ba705221bcf70955b827b97", 441192),
			//AD_ENTRY1s("dw.scn", "ccd72f02183d0e96b6e7d8df9492cda8", 23308),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO_NOSPEECH | GUIO_NOSFX | GUIO_NOMUSIC
		},
		GID_DW1,
		0,
		GF_DEMO,
		TINSEL_V0,
	},

	{	// CD Demo V1 version, with *.gra files
		{
			"dw",
			"CD Demo",
			{
				{"dw.gra", 0, "ef5a2518c9e205f786f5a4526396e661", 781676},
				{"english.smp", 0, NULL, -1},
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD,
		TINSEL_V1,
	},

	{	// Floppy V1 version, with *.gra files
		{
			"dw",
			"Floppy",
			AD_ENTRY1s("dw.gra", "c8808ccd988d603dd35dff42013ae7fd", 781656),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		GID_DW1,
		0,
		GF_FLOPPY | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// CD V1 version, with *.gra files (same as the floppy one, with english.smp)
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Italian CD with english speech and *.gra files.
		// Note: It contains only italian subtitles, but inside english.txt
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.txt", 0, "15f0703f85477d7fab4280bf938b61c1", 237774},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Multilingual CD with english speech and *.gra files.
		// Note: It contains no english subtitles.
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, -1},
				{"french.txt", 0, NULL, -1},
				{"german.txt", 0, NULL, -1},
				{"italian.txt", 0, NULL, -1},
				{"spanish.txt", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_DROPLANGUAGE,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// English CD v2
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "70955425870c7720d6eebed903b2ef41", 776188},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Hebrew CD v2
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "759d1374b4f02af6d52fc07c96679936", 770780},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::HB_ISR,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Discworld PSX CD
		{
			"dw",
			"CD",
			{
				{"english.txt", 0, "7526cfc3a64e00f223795de476b4e2c9", 230326},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// multilanguage PSX demo
		{
			"dw",
			"CD demo",
			{
				{"french.txt", 0, "e7020d35f58d0d187052ac406d86cc87", 273914},
				{"german.txt", 0, "52f0a01e0ff0d340b02a36fd5109d705", 263942},
				{"italian.txt", 0, "15f0703f85477d7fab4280bf938b61c1", 239834},
				{"spanish.txt", 0, "c324170c3f1922c605c5cc09ba265aa5", 236702},
				{"english.txt", 0, "7526cfc3a64e00f223795de476b4e2c9", 230326},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V1,
	},

#if 0
	{	// English Saturn CD
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "6803f293c88758057cc685b9437f7637", 382248},
				{"english.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},
#endif

#if 0
	{	// Mac multilanguage CD
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "cfc40a8d5d476a1c9d3abf826fa46f8c", 1265532},
				{"english.smp", 0, NULL, -1},				
				{NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

#endif

	{	// German CD re-release "Neon Edition"
		// Note: This release has ENGLISH.TXT (with german content) instead of GERMAN.TXT
		{
			"dw",
			"CD",
			AD_ENTRY1s("dw.scn", "6182c7986eaec893c62fb6ea13a9f225", 774556),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_CD | GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// European/Australian Discworld 2 release
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_GRB,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{	// US Discworld 2 release
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"us1.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{	// French version of Discworld 2
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"french1.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{	// German Discworld 2 re-release "Neon Edition"
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"german1.smp", 0, NULL, -1},
				{NULL, 0, NULL, 0}
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{	// Italian/Spanish Discworld 2
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, -1},
				{"italian1.txt", 0, "d443249f8b55489b5888c227b9096f4e", 246495},
				{NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},
	{
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, -1},
				{"spanish1.txt", 0, "bc6e147c5f542db228ac577357e4d897", 230323},
				{NULL, 0, NULL, 0}
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{	// Russian Discworld 2 release by Fargus
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, -1},
				{"english1.txt", 0, "b522e19d7b2cd7b85e50e36fe48e36a9", 274444},
				{NULL, 0, NULL, 0}
			},
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		GID_DW2,
		0,
		GF_CD | GF_SCNFILES,
		TINSEL_V2,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

} // End of namespace Tinsel

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Tinsel::gameDescriptions,
	// Size of that superset structure
	sizeof(Tinsel::TinselGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	tinselGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"tinsel",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE
};

class TinselMetaEngine : public AdvancedMetaEngine {
public:
	TinselMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Tinsel Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Tinsel (C) Psygnosis";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool TinselMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Tinsel::TinselEngine::hasFeature(EngineFeature f) const {
	return
#if 0
		// FIXME: tinsel does not exit cleanly yet
		(f == kSupportsRTL) ||
#endif
		(f == kSupportsLoadingDuringRuntime);
}

namespace Tinsel {
extern int getList(Common::SaveFileManager *saveFileMan, const Common::String &target);
}

SaveStateList TinselMetaEngine::listSaves(const char *target) const {
	Common::String pattern = target;
	pattern = pattern + ".???";
	Common::StringList files = g_system->getSavefileManager()->listSavefiles(pattern);
	sort(files.begin(), files.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringList::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		const Common::String &fname = *file;
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);
		if (in) {
			in->readUint32LE();		// skip id
			in->readUint32LE();		// skip size
			in->readUint32LE();		// skip version
			char saveDesc[Tinsel::SG_DESC_LEN];
			in->read(saveDesc, sizeof(saveDesc));

			saveDesc[Tinsel::SG_DESC_LEN - 1] = 0;

			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
			delete in;
		}
	}

	return saveList;
}

bool TinselMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Tinsel::TinselGameDescription *gd = (const Tinsel::TinselGameDescription *)desc;
	if (gd) {
		*engine = new Tinsel::TinselEngine(syst, gd);
	}
	return gd != 0;
}

struct SizeMD5 {
	int size;
	char md5[32+1];
};
typedef Common::HashMap<Common::String, SizeMD5, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SizeMD5Map;
typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
typedef Common::Array<const ADGameDescription*> ADGameDescList;

/**
 * Fallback detection scans the list of Discworld 2 targets to see if it can detect an installation
 * where the files haven't been renamed (i.e. don't have the '1' just before the extension)
 */
const ADGameDescription *TinselMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	Common::String extra;
	FileMap allFiles;
	SizeMD5Map filesSizeMD5;

	const ADGameFileDescription *fileDesc;
	const Tinsel::TinselGameDescription *g;

	if (fslist.empty())
		return NULL;

	// First we compose a hashmap of all files in fslist.
	// Includes nifty stuff like removing trailing dots and ignoring case.
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			if (!scumm_stricmp(file->getName().c_str(), "dw2")) {
				// Probably Discworld 2 subfolder on CD, so add it's contents as well
				Common::FSList files;
				if (file->getChildren(files, Common::FSNode::kListAll)) {
					Common::FSList::const_iterator file2;
					for (file2 = files.begin(); file2 != files.end(); ++file2) {
						if (file2->isDirectory())
							continue;

						Common::String fname = file2->getName();
						allFiles[fname] = *file2;
					}
				}
			}
			continue;
		}

		Common::String tstr = file->getName();

		allFiles[tstr] = *file;	// Record the presence of this file
	}

	// Check which files are included in some dw2 ADGameDescription *and* present
	// in fslist without a '1' suffix character. Compute MD5s and file sizes for these files.
	for (g = &Tinsel::gameDescriptions[0]; g->desc.gameid != 0; ++g) {
		if (strcmp(g->desc.gameid, "dw2") != 0)
			continue;

		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			strcpy(tempFilename, fileDesc->fileName);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) strcpy(pOne, pOne + 1);	// FIXME: Valgrind complains about this

			Common::String fname(tempFilename);
			if (allFiles.contains(fname) && !filesSizeMD5.contains(fname)) {
				SizeMD5 tmp;
				if (!md5_file_string(allFiles[fname], tmp.md5, detectionParams.md5Bytes))
					tmp.md5[0] = 0;

				Common::File testFile;
				if (testFile.open(allFiles[fname]))
					tmp.size = (int32)testFile.size();
				else
					tmp.size = -1;

				filesSizeMD5[fname] = tmp;
			}
		}
	}

	ADGameDescList matched;
	int maxFilesMatched = 0;
	bool gotAnyMatchesWithAllFiles = false;

	// MD5 based matching
	uint i;
	for (i = 0, g = &Tinsel::gameDescriptions[0]; g->desc.gameid != 0; ++g) {
		if (strcmp(g->desc.gameid, "dw2") != 0)
			continue;

		bool fileMissing = false;

		if ((detectionParams.flags & kADFlagUseExtraAsHint) && !extra.empty() && g->desc.extra != extra)
			continue;

		bool allFilesPresent = true;

		// Try to match all files for this game
		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			strcpy(tempFilename, fileDesc->fileName);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) strcpy(pOne, pOne + 1);

			Common::String tstr(tempFilename);

			if (!filesSizeMD5.contains(tstr)) {
				fileMissing = true;
				allFilesPresent = false;
				break;
			}

			if (fileDesc->md5 != NULL && 0 != strcmp(fileDesc->md5, filesSizeMD5[tstr].md5)) {
				fileMissing = true;
				break;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesSizeMD5[tstr].size) {
				fileMissing = true;
				break;
			}
		}

		if (allFilesPresent)
			gotAnyMatchesWithAllFiles = true;

		if (!fileMissing) {
			// Count the number of matching files. Then, only keep those
			// entries which match a maximal amount of files.
			int curFilesMatched = 0;
			for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++)
				curFilesMatched++;

			if (curFilesMatched > maxFilesMatched) {
				maxFilesMatched = curFilesMatched;

				for (uint j = 0; j < matched.size();) {
					if (matched[j]->flags & ADGF_KEEPMATCH)
						 ++j;
					else
						matched.remove_at(j);
				}
				matched.push_back((const ADGameDescription *)g);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back((const ADGameDescription *)g);
			}
		}
	}

	// We didn't find a match
	if (matched.empty())
		return NULL;

	return *matched.begin();
}

int TinselMetaEngine::getMaximumSaveSlot() const { return 99; }

void TinselMetaEngine::removeSaveState(const char *target, int slot) const {
	Tinsel::setNeedLoad();
	Tinsel::getList(g_system->getSavefileManager(), target);

	g_system->getSavefileManager()->removeSavefile(Tinsel::ListEntry(slot, Tinsel::LE_NAME));
	Tinsel::setNeedLoad();
	Tinsel::getList(g_system->getSavefileManager(), target);
}

#if PLUGIN_ENABLED_DYNAMIC(TINSEL)
	REGISTER_PLUGIN_DYNAMIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#endif

namespace Tinsel {

Common::Error TinselEngine::loadGameState(int slot) {
	// FIXME: Hopefully this is only used when loading games via
	// the launcher, since we do a hacky savegame slot to savelist
	// entry mapping here.
	//
	// You might wonder why is needed and here is the answer:
	// The save/load dialog of the GMM operates with the physical
	// savegame slots, while Tinsel internally uses entry numbers in
	// a savelist (which is sorted latest to first). Now to allow
	// proper loading of (especially Discworld2) saves we need to
	// get a savelist entry number instead of the physical slot.
	//
	// There are different possible solutions:
	//
	// One way to fix this would be to pass the filename instead of
	// the savelist entry number to RestoreGame, though it could make
	// problems how DW2 handles CD switches. Normally DW2 would pass
	// '-2' as slot when it changes CDs.
	//
	// Another way would be to convert all of Tinsel to use physical
	// slot numbers instead of savelist entry numbers for loading.
	// This would also allow '-2' as slot for CD changes without
	// any major hackery.

	int listSlot = -1;
	const int numStates = Tinsel::getList();
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	if (listSlot == -1)
		return Common::kUnknownError;	// TODO: proper error code

	RestoreGame(listSlot);
	return Common::kNoError;	// TODO: return success/failure
}

#if 0
Common::Error TinselEngine::saveGameState(int slot, const char *desc) {
	Common::String saveName = _vm->getSavegameFilename((int16)(slot + 1));
	char saveDesc[SG_DESC_LEN];
	strncpy(saveDesc, desc, SG_DESC_LEN);
	// Make sure that saveDesc is 0-terminated
	saveDesc[SG_DESC_LEN - 1] = '\0';
	SaveGame((char *)saveName.c_str(), saveDesc);
	ProcessSRQueue();			// This shouldn't be needed, but for some reason it is...
	return Common::kNoError;	// TODO: return success/failure
}
#endif

bool TinselEngine::canLoadGameStateCurrently() { return !_bmv->MoviePlaying(); }

#if 0
bool TinselEngine::canSaveGameStateCurrently() { return isCursorShown(); }
#endif

} // End of namespace Tinsel

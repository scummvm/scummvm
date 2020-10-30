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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"
#include "common/md5.h"

#include "tinsel/detection.h"

static const PlainGameDescriptor tinselGames[] = {
	{"dw", "Discworld"},
	{"dw2", "Discworld 2: Missing Presumed ...!?"},
	{"noir", "Discworld Noir"},
	{0, 0}
};

#include "tinsel/detection_tables.h"

class TinselMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TinselMetaEngineDetection() : AdvancedMetaEngineDetection(Tinsel::gameDescriptions, sizeof(Tinsel::TinselGameDescription), tinselGames) {
	}

	const char *getEngineId() const  override{
		return "tinsel";
	}

	const char *getName() const override {
		return "Tinsel";
	}

	const char *getOriginalCopyright() const override {
		return "Tinsel (C) Psygnosis";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
};

struct SizeMD5 {
	int size;
	Common::String md5;
};
typedef Common::HashMap<Common::String, SizeMD5, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SizeMD5Map;
typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
typedef Common::Array<const ADGameDescription *> ADGameDescList;

/**
 * Fallback detection scans the list of Discworld 2 targets to see if it can detect an installation
 * where the files haven't been renamed (i.e. don't have the '1' just before the extension)
 */
ADDetectedGame TinselMetaEngineDetection::fallbackDetect(const FileMap &allFilesXXX, const Common::FSList &fslist) const {
	Common::String extra;
	FileMap allFiles;
	SizeMD5Map filesSizeMD5;

	const ADGameFileDescription *fileDesc;
	const Tinsel::TinselGameDescription *g;

	if (fslist.empty())
		return ADDetectedGame();

	// TODO: The following code is essentially a slightly modified copy of the
	// complete code of function detectGame() in engines/advancedDetector.cpp.
	// That quite some hefty and undesirable code duplication. Its only purpose
	// seems to be to treat filenames of the form "foo1.ext" as "foo.ext".
	// It would be nice to avoid this code duplication.

	// First we compose a hashmap of all files in fslist.
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
	for (g = &Tinsel::gameDescriptions[0]; g->desc.gameId != 0; ++g) {
		if (strcmp(g->desc.gameId, "dw2") != 0)
			continue;

		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			Common::strlcpy(tempFilename, fileDesc->fileName, 50);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) {
				do {
					*pOne = *(pOne + 1);
					pOne++;
				} while (*pOne);
			}

			Common::String fname(tempFilename);
			if (allFiles.contains(fname) && !filesSizeMD5.contains(fname)) {
				SizeMD5 tmp;
				Common::File testFile;

				if (testFile.open(allFiles[fname])) {
					tmp.size = (int32)testFile.size();
					tmp.md5 = computeStreamMD5AsString(testFile, _md5Bytes);
				} else {
					tmp.size = -1;
				}

				filesSizeMD5[fname] = tmp;
			}
		}
	}

	ADDetectedGame matched;
	int maxFilesMatched = 0;

	// MD5 based matching
	for (g = &Tinsel::gameDescriptions[0]; g->desc.gameId != 0; ++g) {
		if (strcmp(g->desc.gameId, "dw2") != 0)
			continue;

		bool fileMissing = false;

		// Try to match all files for this game
		for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++) {
			// Get the next filename, stripping off any '1' suffix character
			char tempFilename[50];
			Common::strlcpy(tempFilename, fileDesc->fileName, 50);
			char *pOne = strchr(tempFilename, '1');
			if (pOne) {
				do {
					*pOne = *(pOne + 1);
					pOne++;
				} while (*pOne);
			}

			Common::String tstr(tempFilename);

			if (!filesSizeMD5.contains(tstr)) {
				fileMissing = true;
				break;
			}

			if (fileDesc->md5 != NULL && fileDesc->md5 != filesSizeMD5[tstr].md5) {
				fileMissing = true;
				break;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesSizeMD5[tstr].size) {
				fileMissing = true;
				break;
			}
		}

		if (!fileMissing) {
			// Count the number of matching files. Then, only keep those
			// entries which match a maximal amount of files.
			int curFilesMatched = 0;
			for (fileDesc = g->desc.filesDescriptions; fileDesc->fileName; fileDesc++)
				curFilesMatched++;

			if (curFilesMatched >= maxFilesMatched) {
				maxFilesMatched = curFilesMatched;

				matched = ADDetectedGame(&g->desc);
			}
		}
	}

	return matched;
}

REGISTER_PLUGIN_STATIC(TINSEL_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TinselMetaEngineDetection);

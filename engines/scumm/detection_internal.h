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

#ifndef SCUMM_DETECTION_INTERNAL_H
#define SCUMM_DETECTION_INTERNAL_H

#include "common/debug.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/punycode.h"
#include "common/translation.h"

#include "gui/error.h"

#include "scumm/detection_tables.h"
#include "scumm/scumm-md5.h"
#include "scumm/file_nes.h"

// Includes some shared functionalities, which is required by multiple TU's.
// Mark it as static in the header, so visibility for function is limited by the TU, and we can use it wherever required.
// This is being done, because it's necessary in detection, creating an instance, as well as in initiliasing the ScummEngine.
#include "scumm/detection_steam.h"

namespace Scumm {

enum {
	// We only compute the MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

static int compareMD5Table(const void *a, const void *b) {
	const char *key = (const char *)a;
	const MD5Table *elem = (const MD5Table *)b;
	return strcmp(key, elem->md5);
}

static const MD5Table *findInMD5Table(const char *md5) {
	uint32 arraySize = ARRAYSIZE(md5table) - 1;
	return (const MD5Table *)bsearch(md5, md5table, arraySize, sizeof(MD5Table), compareMD5Table);
}


static Common::String generateFilenameForDetection(const char *pattern, FilenameGenMethod genMethod, Common::Platform platform) {
	Common::String result;

	switch (genMethod) {
	case kGenDiskNum:
	case kGenRoomNum:
		result = Common::String::format(pattern, 0);
		break;

	case kGenDiskNumSteam:
	case kGenRoomNumSteam: {
		const SteamIndexFile *indexFile = lookUpSteamIndexFile(pattern, platform);
		if (!indexFile) {
			error("Unable to find Steam executable from detection pattern");
		} else {
			result = indexFile->executableName;
		}
		} break;

	case kGenHEPC:
	case kGenHEIOS:
		result = Common::String::format("%s.he0", pattern);
		break;

	case kGenHEMac:
		result = Common::String::format("%s (0)", pattern);
		break;

	case kGenHEMacNoParens:
		result = Common::String::format("%s 0", pattern);
		break;

	case kGenUnchanged:
		result = pattern;
		break;

	default:
		error("generateFilenameForDetection: Unsupported genMethod");
	}

	return result;
}

struct DetectorDesc {
	Common::FSNode node;
	Common::String md5;
	const MD5Table *md5Entry;	// Entry of the md5 table corresponding to this file, if any.
};

typedef Common::HashMap<Common::String, DetectorDesc, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DescMap;

static bool testGame(const GameSettings *g, const DescMap &fileMD5Map, const Common::String &file);


// Search for a node with the given "name", inside fslist. Ignores case
// when performing the matching. The first match is returned, so if you
// search for "resource" and two nodes "RESOURCE" and "resource" are present,
// the first match is used.
static bool searchFSNode(const Common::FSList &fslist, const Common::String &name, Common::FSNode &result) {
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!scumm_stricmp(file->getName().c_str(), name.c_str())) {
			result = *file;
			return true;
		}
	}
	return false;
}

static BaseScummFile *openDiskImage(const Common::FSNode &node, const GameFilenamePattern *gfp) {
	Common::String disk1 = node.getName();
	BaseScummFile *diskImg;

	SearchMan.addDirectory("tmpDiskImgDir", node.getParent());

	if (disk1.hasSuffix(".prg")) { // NES
		diskImg = new ScummNESFile();
	} else { // C64 or Apple //gs
		// setup necessary game settings for disk image reader
		GameSettings gs;
		memset(&gs, 0, sizeof(GameSettings));
		gs.gameid = gfp->gameid;
		gs.id = (Common::String(gfp->gameid) == "maniac" ? GID_MANIAC : GID_ZAK);
		gs.platform = gfp->platform;
		if (strcmp(gfp->pattern, "maniacdemo.d64") == 0)
			gs.features |= GF_DEMO;

		// Determine second disk file name.
		Common::String disk2(disk1);
		for (Common::String::iterator it = disk2.begin(); it != disk2.end(); ++it) {
			// replace "xyz1.(d64|dsk)" by "xyz2.(d64|dsk)"
			if (*it == '1') {
				*it = '2';
				break;
			}
		}

		// Open image.
		diskImg = new ScummDiskImage(disk1.c_str(), disk2.c_str(), gs);
	}

	if (diskImg->open(disk1.c_str()) && diskImg->openSubFile("00.LFL")) {
		debugC(0, kDebugGlobalDetection, "Success");
		return diskImg;
	}
	delete diskImg;
	return 0;
}

static void closeDiskImage(ScummDiskImage *img) {
	if (img)
		img->close();
	SearchMan.remove("tmpDiskImgDir");
}

/*
 * This function tries to detect if a speech file exists.
 * False doesn't necessarily mean there are no speech files.
 */
static bool detectSpeech(const Common::FSList &fslist, const GameSettings *gs) {
	if (gs->id == GID_MONKEY || gs->id == GID_MONKEY2) {
		// FM-TOWNS monkey and monkey2 games don't have speech but may have .sou files.
		if (gs->platform == Common::kPlatformFMTowns)
			return false;

		const char *const basenames[] = { gs->gameid, "monster", 0 };
		static const char *const extensions[] = { "sou",
#ifdef USE_FLAC
		 "sof",
#endif
#ifdef USE_VORBIS
		 "sog",
#endif
#ifdef USE_MAD
		 "so3",
#endif
		 0 };

		for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (file->isDirectory())
				continue;

			for (int i = 0; basenames[i]; ++i) {
				Common::String basename = Common::String(basenames[i]) + ".";

				for (int j = 0; extensions[j]; ++j) {
					if ((basename + extensions[j]).equalsIgnoreCase(file->getName()))
						return true;
				}
			}
		}
	}
	return false;
}

// The following function tries to detect the language.
static Common::Language detectLanguage(const Common::FSList &fslist, byte id, const char *variant, Common::Language originalLanguage = Common::UNK_LANG) {
	// First try to detect Chinese translation.
	Common::FSNode fontFile;

	if (searchFSNode(fslist, "chinese_gb16x12.fnt", fontFile) || (searchFSNode(fslist, "video", fontFile) && fontFile.getChild("chinese_gb16x12.fnt").exists())) {
		debugC(0, kDebugGlobalDetection, "Chinese detected");
		return Common::ZH_CHN;
	}

	for (uint i = 0; ruScummPatcherTable[i].patcherName; i++) {
		Common::FSNode patchFile;
		if (ruScummPatcherTable[i].gameid == id && (variant == nullptr || strcmp(variant, ruScummPatcherTable[i].variant) == 0)
		    && searchFSNode(fslist, Common::punycode_decode(ruScummPatcherTable[i].patcherName), patchFile)) {
			debugC(0, kDebugGlobalDetection, "Russian detected");
			return Common::RU_RUS;
		}
	}

	if (id != GID_CMI && id != GID_DIG) {
		// Detect Korean fan translated games
		Common::FSNode langFile;
		if (searchFSNode(fslist, "korean.trs", langFile)) {
			debugC(0, kDebugGlobalDetection, "Korean fan translation detected");
			return Common::KO_KOR;
		}

		return originalLanguage;
	}

	// Now try to detect COMI and Dig by language files.
	// Check for LANGUAGE.BND (Dig) resp. LANGUAGE.TAB (CMI).
	// These are usually inside the "RESOURCE" subdirectory.
	// If found, we match based on the file size (should we
	// ever determine that this is insufficient, we can still
	// switch to MD5 based detection).
	const char *filename = (id == GID_CMI) ? "LANGUAGE.TAB" : "LANGUAGE.BND";
	Common::File tmp;
	Common::FSNode langFile;
	if (searchFSNode(fslist, filename, langFile))
		tmp.open(langFile);
	if (!tmp.isOpen()) {
		// Try loading in RESOURCE sub dir.
		Common::FSNode resDir;
		Common::FSList tmpList;
		if (searchFSNode(fslist, "RESOURCE", resDir)
			&& resDir.isDirectory()
			&& resDir.getChildren(tmpList, Common::FSNode::kListFilesOnly)
			&& searchFSNode(tmpList, filename, langFile)) {
			tmp.open(langFile);
		}
		// The Steam version of Dig has the LANGUAGE.BND in the DIG sub dir.
		if (!tmp.isOpen()
			&& id == GID_DIG
			&& searchFSNode(fslist, "DIG", resDir)
			&& resDir.isDirectory()
			&& resDir.getChildren(tmpList, Common::FSNode::kListFilesOnly)
			&& searchFSNode(tmpList, filename, langFile)) {
			tmp.open(langFile);
		}
		// The Chinese version of Dig has the LANGUAGE.BND in the VIDEO sub dir.
		if (!tmp.isOpen()
			&& id == GID_DIG
			&& searchFSNode(fslist, "VIDEO", resDir)
			&& resDir.isDirectory()
			&& resDir.getChildren(tmpList, Common::FSNode::kListFilesOnly)
			&& searchFSNode(tmpList, filename, langFile)) {
			tmp.open(langFile);
		}
	}
	if (tmp.isOpen()) {
		uint size = tmp.size();
		if (id == GID_CMI) {
			switch (size) {
			case 439080:	// 2daf3db71d23d99d19fc9a544fcf6431
				return Common::EN_ANY;
			case 322602:	// caba99f4f5a0b69963e5a4d69e6f90af
				return Common::ZH_TWN;
			case 493252:	// 5d59594b24f3f1332e7d7e17455ed533
				return Common::DE_DEU;
			case 461746:	// 35bbe0e4d573b318b7b2092c331fd1fa
				return Common::FR_FRA;
			case 443439:	// 4689d013f67aabd7c35f4fd7c4b4ad69
				return Common::IT_ITA;
			case 398613:	// d1f5750d142d34c4c8f1f330a1278709
				return Common::KO_KOR;
			case 440586:	// 5a1d0f4fa00917bdbfe035a72a6bba9d
				return Common::PT_BRA;
			case 454457:	// 0e5f450ec474a30254c0e36291fb4ebd
			case 394083:	// ad684ca14c2b4bf4c21a81c1dbed49bc
				return Common::RU_RUS;
			case 449787:	// 64f3fe479d45b52902cf88145c41d172
				return Common::ES_ESP;
			default:
				break;
			}
		} else { // The DIG
			switch (size) {
			case 248627:	// 1fd585ac849d57305878c77b2f6c74ff
				return Common::DE_DEU;
			case 257460:	// 04cf6a6ba6f57e517bc40eb81862cfb0
				return Common::FR_FRA;
			case 231402:	// 93d13fcede954c78e65435592182a4db
				return Common::IT_ITA;
			case 228772:	// 5d9ad90d3a88ea012d25d61791895ebe
				return Common::PT_BRA;
			case 229884:	// d890074bc15c6135868403e73c5f4f36
				return Common::ES_ESP;
			case 223107:	// 64f3fe479d45b52902cf88145c41d172
				return Common::JA_JPN;
			case 180730:	// 424fdd60822722cdc75356d921dad9bf
				return Common::ZH_TWN;
			default:
				break;
			}
		}
	}

	return originalLanguage;
}


static void computeGameSettingsFromMD5(const Common::FSList &fslist, const GameFilenamePattern *gfp, const MD5Table *md5Entry, DetectorResult &dr) {
	dr.language = md5Entry->language;
	dr.extra = md5Entry->extra;

	// Compute the precise game settings using gameVariantsTable.
	for (const GameSettings *g = gameVariantsTable; g->gameid; ++g) {
		if (g->gameid[0] == 0 || !scumm_stricmp(md5Entry->gameid, g->gameid)) {
			// The gameid either matches, or is empty. The latter indicates
			// a generic entry, currently used for some generic HE settings.
			if (g->variant == 0 || !scumm_stricmp(md5Entry->variant, g->variant)) {

				// The English EGA release of Monkey Island 1 sold by Limited Run Games in the
				// Monkey Island Anthology in late 2021 contains several corrupted files, making
				// the game unplayable (see bug #14500). It's possible to recover working files
				// from the raw KryoFlux resources also provided by LRG, but this requires
				// dedicated tooling, and so we can just detect the corrupted resources and
				// report the problem to users before they report weird crashes in the game.
				// https://dwatteau.github.io/scummfixes/corrupted-monkey1-ega-files-limitedrungames.html
				if (g->id == GID_MONKEY_EGA && g->platform == Common::kPlatformDOS) {
					Common::String md5Disk03, md5Disk04, md5Lfl903;
					Common::FSNode resFile;
					Common::File f;

					if (searchFSNode(fslist, "903.LFL", resFile))
						f.open(resFile);
					if (f.isOpen()) {
						md5Lfl903 = Common::computeStreamMD5AsString(f, kMD5FileSizeLimit);
						f.close();
					}

					if (searchFSNode(fslist, "DISK03.LEC", resFile))
						f.open(resFile);
					if (f.isOpen()) {
						md5Disk03 = Common::computeStreamMD5AsString(f, kMD5FileSizeLimit);
						f.close();
					}

					if (searchFSNode(fslist, "DISK04.LEC", resFile))
						f.open(resFile);
					if (f.isOpen()) {
						md5Disk04 = Common::computeStreamMD5AsString(f, kMD5FileSizeLimit);
						f.close();
					}

					if ((!md5Lfl903.empty() && md5Lfl903 == "54d4e17df08953b483d17416043345b9") ||
					    (!md5Disk03.empty() && md5Disk03 == "a8ab7e8eaa322d825beb6c5dee28f17d") ||
					    (!md5Disk04.empty() && md5Disk04 == "f338cc1d3117c1077a3a9d0c1d70b1e8")) {
						::GUI::displayErrorDialog(_("This version of Monkey Island can't be played, because Limited Run Games "
						    "provided corrupted DISK03.LEC, DISK04.LEC and 903.LFL files.\n\nPlease contact their technical "
						    "support for replacement files, or look online for some guides which can help you recover valid "
						    "files from the KryoFlux dumps that Limited Run Games also provided."));
						continue;
					}
				}

				// Perfect match found, use it and stop the loop.
				dr.game = *g;
				dr.game.gameid = md5Entry->gameid;

				// Set the platform value. The value from the MD5 record has
				// highest priority; if missing (i.e. set to unknown) we try
				// to use that from the filename pattern record instead.
				if (md5Entry->platform != Common::kPlatformUnknown) {
					dr.game.platform = md5Entry->platform;
				} else if (gfp->platform != Common::kPlatformUnknown) {
					dr.game.platform = gfp->platform;
				}

				// HACK: Special case to distinguish the V1 demo from the full version
				// (since they have identical MD5).
				if (dr.game.id == GID_MANIAC && !strcmp(gfp->pattern, "%02d.MAN")) {
					dr.extra = "V1 Demo";
					dr.game.features = GF_DEMO;
				}

				// HACK: Try to detect languages for translated games.
				if (dr.language == UNK_LANG || dr.language == Common::EN_ANY) {
					dr.language = detectLanguage(fslist, dr.game.id, g->variant, dr.language);
				}

				// HACK: Detect between 68k and PPC versions.
				if (dr.game.platform == Common::kPlatformMacintosh && dr.game.version >= 5 && dr.game.heversion == 0 && strstr(gfp->pattern, "Data"))
					dr.game.features |= GF_MAC_CONTAINER;

				break;
			}
		}
	}
}

static void composeFileHashMap(DescMap &fileMD5Map, const Common::FSList &fslist, int depth, const char *const *globs) {
	if (depth <= 0)
		return;

	if (fslist.empty())
		return;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (!file->isDirectory()) {
			DetectorDesc d;
			d.node = *file;
			d.md5Entry = 0;
			fileMD5Map[file->getName()] = d;
		} else {
			if (!globs)
				continue;

			bool matched = false;
			for (const char *const *glob = globs; *glob; glob++)
				if (file->getName().matchString(*glob, true)) {
					matched = true;
					break;
				}

			if (!matched)
				continue;

			Common::FSList files;
			if (file->getChildren(files, Common::FSNode::kListAll)) {
				composeFileHashMap(fileMD5Map, files, depth - 1, globs);
			}
		}
	}
}

static void detectGames(const Common::FSList &fslist, Common::List<DetectorResult> &results, const char *gameid) {
	DescMap fileMD5Map;
	DetectorResult dr;

	// Dive one level down since mac indy3/loom have their files split into directories. See Bug #2507.
	// Dive two levels down for Mac Steam games.
	composeFileHashMap(fileMD5Map, fslist, 3, directoryGlobs);

	// Iterate over all filename patterns.
	for (const GameFilenamePattern *gfp = gameFilenamesTable; gfp->gameid; ++gfp) {
		// If a gameid was specified, we only try to detect that specific game,
		// so we can just skip over everything with a differing gameid.
		if (gameid && scumm_stricmp(gameid, gfp->gameid))
			continue;

		// Generate the detectname corresponding to the gfp. If the file doesn't
		// exist in the directory we are looking at, we can skip to the next
		// one immediately.
		Common::String file(generateFilenameForDetection(gfp->pattern, gfp->genMethod, gfp->platform));
		Common::Platform platform = gfp->platform;
		if (!fileMD5Map.contains(file)) {
			if (fileMD5Map.contains(file + ".bin") && (platform == Common::Platform::kPlatformMacintosh || platform == Common::Platform::kPlatformUnknown)) {
				file += ".bin";
				platform = Common::Platform::kPlatformMacintosh;
			} else
				continue;
		}

		// Reset the DetectorResult variable.
		dr.fp.pattern = gfp->pattern;
		dr.fp.genMethod = gfp->genMethod;
		dr.game.gameid = 0;
		dr.language = gfp->language;
		dr.md5.clear();
		dr.extra = 0;

		//  ____            _     _
		// |  _ \ __ _ _ __| |_  / |
		// | |_) / _` | '__| __| | |
		// |  __/ (_| | |  | |_  | |
		// |_|   \__,_|_|   \__| |_|
		//
		// PART 1: Trying to find an exact match using MD5.
		//
		//
		// Background: We found a valid detection file. Check if its MD5
		// checksum occurs in our MD5 table. If it does, try to use that
		// to find an exact match.
		//
		// We only do that if the MD5 hadn't already been computed (since
		// we may look at some detection files multiple times).
		DetectorDesc &d = fileMD5Map[file];
		if (d.md5.empty()) {
			Common::SeekableReadStream *tmp = 0;
			bool isDiskImg = (file.hasSuffix(".d64") || file.hasSuffix(".dsk") || file.hasSuffix(".prg"));

			if (isDiskImg) {
				tmp = openDiskImage(d.node, gfp);

				debugC(2, kDebugGlobalDetection, "Falling back to disk-based detection");
			} else {
				tmp = d.node.createReadStream();
			}

			Common::String md5str;
			if (tmp)
				md5str = computeStreamMD5AsString(*tmp, kMD5FileSizeLimit);
			if (!md5str.empty()) {
				int64 filesize = tmp->size();

				d.md5 = md5str;
				d.md5Entry = findInMD5Table(md5str.c_str());

				if (!d.md5Entry && (platform == Common::Platform::kPlatformMacintosh || platform == Common::Platform::kPlatformUnknown)) {
					Common::SeekableReadStream *dataStream = Common::MacResManager::openDataForkFromMacBinary(tmp);
					if (dataStream) {
						Common::String dataMD5 = computeStreamMD5AsString(*dataStream, kMD5FileSizeLimit);
						const MD5Table *dataMD5Entry = findInMD5Table(dataMD5.c_str());
						if (dataMD5Entry) {
							d.md5 = dataMD5;
							d.md5Entry = dataMD5Entry;
							filesize = dataStream->size();
							platform = Common::Platform::kPlatformMacintosh;
						}
						delete dataStream;
					}
				}

				dr.md5 = d.md5;

				if (d.md5Entry) {
					// Exact match found. Compute the precise game settings.
					computeGameSettingsFromMD5(fslist, gfp, d.md5Entry, dr);

					// Print some debug info.
					debugC(1, kDebugGlobalDetection, "SCUMM detector found matching file '%s' with MD5 %s, size %" PRId64 "\n",
						file.c_str(), md5str.c_str(), filesize);

					// Sanity check: We *should* have found a matching gameid/variant at this point.
					// If not, we may have #ifdef'ed the entry out in our detection_tables.h, because we
					// don't have the required stuff compiled in, or there's a bug in our data tables.
					if (dr.game.gameid != 0)
						// Add it to the list of detected games.
						results.push_back(dr);
				}
			}

			if (isDiskImg)
				closeDiskImage((ScummDiskImage *)tmp);
			delete tmp;
		}

		// If an exact match for this file has already been found, don't bother
		// looking at it anymore.
		if (d.md5Entry)
			continue;

		// Prevent executables being detected as Steam variant. If we don't
		// know the md5, then it's just the regular executable. Otherwise we
		// will most likely fail on trying to read the index from the executable.
		// Fixes bug #10290.
		if (gfp->genMethod == kGenRoomNumSteam || gfp->genMethod == kGenDiskNumSteam)
			continue;

		//  ____            _     ____
		// |  _ \ __ _ _ __| |_  |___ \ *
		// | |_) / _` | '__| __|   __) |
		// |  __/ (_| | |  | |_   / __/
		// |_|   \__,_|_|   \__| |_____|
		//
		// PART 2: Fuzzy matching for files with unknown MD5.
		//
		//
		// We loop over the game variants matching the gameid associated to
		// the gfp record. We then try to decide for each whether it could be
		// appropriate or not.
		dr.md5 = d.md5;
		for (const GameSettings *g = gameVariantsTable; g->gameid; ++g) {
			// Skip over entries with a different gameid.
			if (g->gameid[0] == 0 || scumm_stricmp(gfp->gameid, g->gameid))
				continue;

			dr.game = *g;
			dr.extra = g->variant; // FIXME: We (ab)use 'variant' for the 'extra' description for now.

			if (platform != Common::kPlatformUnknown)
				dr.game.platform = platform;


			// If a variant has been specified, use that!
			if (gfp->variant) {
				if (!scumm_stricmp(gfp->variant, g->variant)) {
					// Perfect match found.
					results.push_back(dr);
					break;
				}
				continue;
			}

			// HACK: Perhaps it is some modified translation?
			dr.language = detectLanguage(fslist, g->id, g->variant);

			// Detect if there are speech files in this unknown game.
			if (detectSpeech(fslist, g)) {
				if (strchr(dr.game.guioptions, GUIO_NOSPEECH[0]) != NULL) {
					if (g->id == GID_MONKEY || g->id == GID_MONKEY2)
						// TODO: This may need to be updated if something important gets added
						// in the top detection table for these game ids.
						dr.game.guioptions = GUIO0();
					else
						warning("FIXME: fix NOSPEECH fallback");
				}
			}

			// Add the game/variant to the candidates list if it is consistent
			// with the file(s) we are seeing.
			if (testGame(g, fileMD5Map, file))
				results.push_back(dr);
		}
	}
}

static bool testGame(const GameSettings *g, const DescMap &fileMD5Map, const Common::String &file) {
	const DetectorDesc &d = fileMD5Map[file];

	// At this point, we know that the gameid matches, but no variant
	// was specified, yet there are multiple ones. So we try our best
	// to distinguish between the variants.
	// To do this, we take a close look at the detection file and
	// try to filter out some cases.

	Common::File tmp;
	if (!tmp.open(d.node)) {
		warning("SCUMM testGame: failed to open '%s' for read access", d.node.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	if (file == "maniac1.d64" || file == "maniac1.dsk" || file == "zak1.d64") {
		// TODO
	} else if (file == "00.LFL") {
		// Used in V1, V2, V3 games.
		if (g->version > 3)
			return false;

		// Read a few bytes to narrow down the game.
		byte buf[6];
		tmp.read(buf, 6);

		if (buf[0] == 0xbc && buf[1] == 0xb9) {
			// The NES version of MM.
			if (g->id == GID_MANIAC && g->platform == Common::kPlatformNES) {
				// Perfect match.
				return true;
			}
		} else if ((buf[0] == 0xCE && buf[1] == 0xF5) || // PC
			(buf[0] == 0xCD && buf[1] == 0xFE)) {    // Commodore 64
			// Could be V0 or V1.
			// Candidates: maniac classic, zak classic.

			if (g->version >= 2)
				return false;

			// Zak has 58.LFL, Maniac doesn't.
			const bool has58LFL = fileMD5Map.contains("58.LFL");
			if (g->id == GID_MANIAC && !has58LFL) {
			} else if (g->id == GID_ZAK && has58LFL) {
			} else
				return false;
		} else if (buf[0] == 0xFF && buf[1] == 0xFE) {
			// GF_OLD_BUNDLE: could be V2 or old V3.
			// Note that GF_OLD_BUNDLE is true if and only if GF_OLD256 is false.
			// Candidates: maniac enhanced, zak enhanced, indy3ega, loom.

			if ((g->version != 2 && g->version != 3)  || (g->features & GF_OLD256))
				return false;

			/* We distinguish the games by the presence/absence of
			   certain files. In the following, '+' means the file
			   present, '-' means the file is absent.

			   maniac:    -58.LFL, -84.LFL,-86.LFL, -98.LFL

			   zak:       +58.LFL, -84.LFL,-86.LFL, -98.LFL
			   zakdemo:   +58.LFL, -84.LFL,-86.LFL, -98.LFL

			   loom:      +58.LFL, -84.LFL,+86.LFL, -98.LFL
			   loomdemo:  -58.LFL, +84.LFL,-86.LFL, -98.LFL

			   indy3:     +58.LFL, +84.LFL,+86.LFL, +98.LFL
			   indy3demo: -58.LFL, +84.LFL,-86.LFL, +98.LFL
			*/
			const bool has58LFL = fileMD5Map.contains("58.LFL");
			const bool has84LFL = fileMD5Map.contains("84.LFL");
			const bool has86LFL = fileMD5Map.contains("86.LFL");
			const bool has98LFL = fileMD5Map.contains("98.LFL");

			if (g->id == GID_INDY3         && has98LFL && has84LFL) {
			} else if (g->id == GID_ZAK    && !has98LFL && !has86LFL && !has84LFL && has58LFL) {
			} else if (g->id == GID_MANIAC && !has98LFL && !has86LFL && !has84LFL && !has58LFL) {
			} else if (g->id == GID_LOOM   && !has98LFL && (has86LFL != has84LFL)) {
			} else
				return false;
		} else if (buf[4] == '0' && buf[5] == 'R') {
			// Newer V3 game.
			// Candidates: indy3, indy3Towns, zakTowns, loomTowns.

			if (g->version != 3 || !(g->features & GF_OLD256))
				return false;

			/*
			Considering that we know about *all* TOWNS versions, and
			know their MD5s, we could simply rely on this and if we find
			something which has an unknown MD5, assume that it is an (so
			far unknown) version of Indy3. However, there are also fan
			translations of the TOWNS versions, so we can't do that.

			But we could at least look at the resource headers to distinguish
			TOWNS versions from regular games:

			Indy3:
			_numGlobalObjects 1000
			_numRooms 99
			_numCostumes 129
			_numScripts 139
			_numSounds 84

			Indy3Towns, ZakTowns, ZakLoom demo:
			_numGlobalObjects 1000
			_numRooms 99
			_numCostumes 199
			_numScripts 199
			_numSounds 199

			Assuming that all the town variants look like the latter, we can
			do the check like this:
			  if (numScripts == 139)
				assume Indy3
			  else if (numScripts == 199)
				assume towns game
			  else
				unknown, do not accept it
			*/

			// We now try to exclude various possibilities by the presence of certain
			// LFL files. Note that we only exclude something based on the *presence*
			// of a LFL file here; compared to checking for the absence of files, this
			// has the advantage that we are less likely to accidentally exclude demos
			// (which, after all, are usually missing many LFL files present in the
			// full version of the game).

			// No version of Indy3 has 05.LFL but MM, Loom and Zak all have it.
			if (g->id == GID_INDY3 && fileMD5Map.contains("05.LFL"))
				return false;

			// All versions of Indy3 have 93.LFL, but no other game does.
			if (g->id != GID_INDY3 && fileMD5Map.contains("93.LFL"))
				return false;

			// No version of Loom has 48.LFL.
			if (g->id == GID_LOOM && fileMD5Map.contains("48.LFL"))
				return false;

			// No version of Zak has 60.LFL, but most (non-demo) versions of Indy3 have it.
			if (g->id == GID_ZAK && fileMD5Map.contains("60.LFL"))
				return false;

			// All versions of Indy3 and ZakTOWNS have 98.LFL, but no other game does.
			if (g->id == GID_LOOM && g->platform != Common::kPlatformPCEngine && fileMD5Map.contains("98.LFL"))
				return false;


		} else {
			// TODO: Unknown file header, deal with it. Maybe an unencrypted
			// variant...
			// Anyway, we don't know how to deal with the file, so we
			// just skip it.
		}
	} else if (file == "000.LFL") {
		// Used in V4.
		// Candidates: monkeyEGA, pass, monkeyVGA, loomcd.

		if (g->version != 4)
			return false;

		/*
		For all of them, we have:
		_numGlobalObjects 1000
		_numRooms 99
		_numCostumes 199
		_numScripts 199
		_numSounds 199

		Any good ideas to distinguish those? Maybe by the presence/absence
		of some files?
		At least PASS and the monkeyEGA demo differ by 903.LFL missing.
		And the count of DISK??.LEC files differ depending on what version
		you have (4 or 8 floppy versions).
		loomcd of course shipped on only one "disc".

		pass: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
		monkeyEGA:  000.LFL, 901-904.LFL, DISK01-09.LEC
		monkeyEGA DEMO: 000.LFL, 901.LFL, 902.LFL, 904.LFL, disk01.lec
		monkeyVGA: 000.LFL, 901-904.LFL, DISK01-04.LEC
		loomcd: 000.LFL, 901-904.LFL, DISK01.LEC
		*/

		const bool has903LFL = fileMD5Map.contains("903.LFL");
		const bool hasDisk02 = fileMD5Map.contains("DISK02.LEC");

		// There is not much we can do based on the presence/absence
		// of files. Only that if 903.LFL is present, it can't be PASS;
		// and if DISK02.LEC is present, it can't be LoomCD.
		if (g->id == GID_PASS              && !has903LFL && !hasDisk02) {
		} else if (g->id == GID_LOOM       &&  has903LFL && !hasDisk02) {
		} else if (g->id == GID_MONKEY_VGA) {
		} else if (g->id == GID_MONKEY_EGA) {
		} else
			return false;
	} else {
		// Must be a V5+ game.
		if (g->version < 5)
			return false;

		// At this point the gameid is determined, but not necessarily
		// the variant!

		// TODO: Add code that handles this, at least for the non-HE games.
		// Not sure how realistic it is to correctly detect HE game
		// variants, would require me to look at a sufficiently large
		// sample collection of HE games (assuming I had the time :).

		// TODO: For Mac versions in container file, we can sometimes
		// distinguish the demo from the regular version by looking
		// at the content of the container file and then looking for
		// the *.000 file in there.
	}

	return true;
}

static Common::String customizeGuiOptions(const DetectorResult &res) {
	Common::String guiOptions = res.game.guioptions;

	static const uint mtypes[] = {MT_PCSPK, MT_CMS, MT_PCJR, MT_ADLIB, MT_C64, MT_AMIGA, MT_APPLEIIGS, MT_TOWNS, MT_PC98, MT_SEGACD, 0, 0, 0, 0, MT_MACINTOSH};
	int midiflags = res.game.midi;

	// These games often have no detection entries of their own and therefore come with all the DOS audio options.
	// We clear them here to avoid confusion and add the appropriate default sound option below.
	if (res.game.platform == Common::kPlatformAmiga || (res.game.platform == Common::kPlatformMacintosh && strncmp(res.extra, "Steam", 6)) || res.game.platform == Common::kPlatformC64) {
		midiflags = MDT_NONE;
		// Remove invalid types from options string
		for (int i = 0; i < ARRAYSIZE(mtypes); ++i) {
			if (!mtypes[i])
				continue;
			uint pos = guiOptions.findFirstOf(MidiDriver::musicType2GUIO(mtypes[i]));
			if (pos != Common::String::npos)
				guiOptions.erase(pos, 1);
		}
	}

	for (int i = 0; i < ARRAYSIZE(mtypes); ++i) {
		if (mtypes[i] && (midiflags & (1 << i)))
			guiOptions += MidiDriver::musicType2GUIO(mtypes[i]);
	}

	if (midiflags & MDT_MIDI) {
		guiOptions += MidiDriver::musicType2GUIO(MT_GM);
		guiOptions += MidiDriver::musicType2GUIO(MT_MT32);
	}

	// Amiga versions often have no detection entries of their own and therefore come with all the DOS render modes.
	// We remove them if we find any.
	static const char *const rmodes[] = { GUIO_RENDERHERCGREEN, GUIO_RENDERHERCAMBER, GUIO_RENDERCGABW, GUIO_RENDERCGACOMP, GUIO_RENDERCGA };
	if (res.game.platform == Common::kPlatformAmiga) {
		for (int i = 0; i < ARRAYSIZE(rmodes); ++i) {
			uint pos = guiOptions.findFirstOf(rmodes[i][0]);
			if (pos != Common::String::npos)
				guiOptions.erase(pos, 1);
		}
	}

	Common::String defaultRenderOption = "";
	Common::String defaultSoundOption = "";

	// Add default rendermode and sound option for target. We don't always put the default modes
	// into the detection tables, due to the amount of targets we have. It it more convenient to
	// add the option here.
	switch (res.game.platform) {
	case Common::kPlatformC64:
		defaultRenderOption = GUIO_RENDERC64;
		defaultSoundOption = GUIO_MIDIC64;
		break;
	case Common::kPlatformAmiga:
		defaultRenderOption = GUIO_RENDERAMIGA;
		defaultSoundOption = GUIO_MIDIAMIGA;
		break;
	case Common::kPlatformApple2GS:
		defaultRenderOption = GUIO_RENDERAPPLE2GS;
		// No default sound here, since we don't support it.
		break;
	case Common::kPlatformMacintosh:
		if (!strncmp(res.extra, "Steam", 6)) {
			defaultRenderOption = GUIO_RENDERVGA;
		} else {
			defaultRenderOption = GUIO_RENDERMACINTOSH;
			defaultSoundOption = GUIO_MIDIMAC;
		}
		break;
	case Common::kPlatformFMTowns:
		defaultRenderOption = GUIO_RENDERFMTOWNS;
		// No default sound here, it is all in the detection tables.
		break;
	case Common::kPlatformAtariST:
		defaultRenderOption = GUIO_RENDERATARIST;
		// No default sound here, since we don't support it.
		break;
	case Common::kPlatformDOS:
		defaultRenderOption = (!strncmp(res.extra, "EGA", 4) || !strncmp(res.extra, "V1", 3) || !strncmp(res.extra, "V2", 3)) ? GUIO_RENDEREGA : GUIO_RENDERVGA;
		break;
	case Common::kPlatformUnknown:
		// For targets that don't specify the platform (often happens with SCUMM6+ games) we stick with default VGA.
		defaultRenderOption = GUIO_RENDERVGA;
		break;
	default:
		// Leave this as nullptr for platforms that don't have a specific render option (SegaCD, NES, ...).
		// These targets will then have the full set of render mode options in the launcher options dialog.
		break;
	}

	// If the render option is already part of the string (specified in the
	// detection tables) we don't add it again.
	if (!guiOptions.contains(defaultRenderOption))
		guiOptions += defaultRenderOption;
	// Same for sound...
	if (!defaultSoundOption.empty() && !guiOptions.contains(defaultSoundOption))
		guiOptions += defaultSoundOption;

	return guiOptions;
}

} // End of namespace Scumm

#endif // SCUMM_DETECTION_INTERNAL_H

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

#include "gob/dataio.h"
#include "gob/detection/detection.h"
#include "gob/detection/tables.h"

class GobMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	GobMetaEngineDetection();

	const char *getEngineId() const override {
		return "gob";
	}

	const char *getName() const override;
	const char *getOriginalCopyright() const override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;

private:
	/**
	 * Inspect the game archives to detect which Once Upon A Time game this is.
	 */
	static const Gob::GOBGameDescription *detectOnceUponATime(const Common::FSList &fslist);
};

GobMetaEngineDetection::GobMetaEngineDetection() :
	AdvancedMetaEngineDetection(Gob::gameDescriptions, sizeof(Gob::GOBGameDescription), gobGames) {

	_guiOptions = GUIO1(GUIO_NOLAUNCHLOAD);
}

ADDetectedGame GobMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	ADDetectedGame detectedGame = detectGameFilebased(allFiles, Gob::fileBased);
	if (!detectedGame.desc) {
		return ADDetectedGame();
	}

	const Gob::GOBGameDescription *game = (const Gob::GOBGameDescription *)detectedGame.desc;

	if (game->gameType == Gob::kGameTypeOnceUponATime) {
		game = detectOnceUponATime(fslist);
		if (game) {
			detectedGame.desc = &game->desc;
		}
	}

	return detectedGame;
}

const Gob::GOBGameDescription *GobMetaEngineDetection::detectOnceUponATime(const Common::FSList &fslist) {
	// Add the game path to the search manager
	SearchMan.clear();
	SearchMan.addDirectory(fslist.begin()->getParent().getPath(), fslist.begin()->getParent());

	// Open the archives
	Gob::DataIO dataIO;
	if (!dataIO.openArchive("stk1.stk", true) ||
	    !dataIO.openArchive("stk2.stk", true) ||
	    !dataIO.openArchive("stk3.stk", true)) {

		SearchMan.clear();
		return 0;
	}

	Gob::OnceUponATime gameType         = Gob::kOnceUponATimeInvalid;
	Gob::OnceUponATimePlatform platform = Gob::kOnceUponATimePlatformInvalid;

	// If these animal files are present, it's Abracadabra
	if (dataIO.hasFile("arai.anm") &&
	    dataIO.hasFile("crab.anm") &&
	    dataIO.hasFile("crap.anm") &&
	    dataIO.hasFile("drag.anm") &&
	    dataIO.hasFile("guep.anm") &&
	    dataIO.hasFile("loup.anm") &&
	    dataIO.hasFile("mous.anm") &&
	    dataIO.hasFile("rhin.anm") &&
	    dataIO.hasFile("saut.anm") &&
	    dataIO.hasFile("scor.anm"))
		gameType = Gob::kOnceUponATimeAbracadabra;

	// If these animal files are present, it's Baba Yaga
	if (dataIO.hasFile("abei.anm") &&
	    dataIO.hasFile("arai.anm") &&
	    dataIO.hasFile("drag.anm") &&
	    dataIO.hasFile("fauc.anm") &&
	    dataIO.hasFile("gren.anm") &&
	    dataIO.hasFile("rena.anm") &&
	    dataIO.hasFile("sang.anm") &&
	    dataIO.hasFile("serp.anm") &&
	    dataIO.hasFile("tort.anm") &&
	    dataIO.hasFile("vaut.anm"))
		gameType = Gob::kOnceUponATimeBabaYaga;

	// Detect the platform by endianness and existence of a MOD file
	Common::SeekableReadStream *villeDEC = dataIO.getFile("ville.dec");
	if (villeDEC && (villeDEC->size() > 6)) {
		byte data[6];

		if (villeDEC->read(data, 6) == 6) {
			if        (!memcmp(data, "\000\000\000\001\000\007", 6)) {
				// Big endian -> Amiga or Atari ST

				if (dataIO.hasFile("mod.babayaga"))
					platform = Gob::kOnceUponATimePlatformAmiga;
				else
					platform = Gob::kOnceUponATimePlatformAtariST;

			} else if (!memcmp(data, "\000\000\001\000\007\000", 6))
				// Little endian -> DOS
				platform = Gob::kOnceUponATimePlatformDOS;
		}

		delete villeDEC;
	}

	SearchMan.clear();

	if ((gameType == Gob::kOnceUponATimeInvalid) || (platform == Gob::kOnceUponATimePlatformInvalid)) {
		warning("GobMetaEngineDetection::detectOnceUponATime(): Detection failed (%d, %d)",
		        (int)gameType, (int)platform);
		return 0;
	}

	return &Gob::fallbackOnceUpon[gameType][platform];
}

const char *GobMetaEngineDetection::getName() const {
	return "Gob";
}

const char *GobMetaEngineDetection::getOriginalCopyright() const {
	return "Goblins Games (C) Coktel Vision";
}

REGISTER_PLUGIN_STATIC(GOB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GobMetaEngineDetection);

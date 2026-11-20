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

#ifndef HOLLYWOOD_RUNAWAY_STRINGS_H
#define HOLLYWOOD_RUNAWAY_STRINGS_H

#include "common/hash-str.h"
#include "common/language.h"
#include "common/ustr.h"

namespace Common {
class Archive;
}

namespace Hollywood {

// Original executable labels are compiled from PO catalogs into hollywood.dat.
// Dialogue and subtitles remain in RESOURCE.003.
class RunawayStrings {
public:
	enum TextId {
		kLook, kTake, kUse, kPutAway, kWith, kInventory, kResume,
		kOpen, kClose, kMove, kLookThrough, kUsePhone, kTakeFromShelf,
		kUnplugCooler, kShirt, kWetCloth, kDirtyCloth,
		kOptions, kLoad, kSave, kDelete, kPlay, kQuit, kMusic, kAntialias,
		kScrolling, kPointerSound, kSubtitles, kOn, kOff, kFast, kSlow,
		kVolume, kBrightness, kCredits, kSaveSettings, kDefaultSettings,
		kConfirmQuit, kYes, kNo, kQuitKey, kLeaveHatch,
		kEnterSaveName, kNoSaves, kNewSave, kGameSaved, kSettingsSaved, kDefaultsRestored,
		kSaveFailed, kLoadFailed, kDeleteFailed, kNoFreeSlots, kDeleteKey, kCreditsFailed,
		kPlayBeginning, kPlayChapter,
		kDryMarkerPen, kSyringeWithAlcohol, kFakeMedicalChart, kHeadWithWig, kGlassWithWater,
		kBoxOfSyringes, kMedicalChart, kFakeChart,
		kMuseumDemoCondom, kFrozenBattery, kMayanPryingTool, kGroundCoffeeBowl, kEmptyBowl, kLadleWithBattery,
		kAnthropologyBooks, kDocuments, kAlteredLaser, kVarnishBriefcase, kWilly,
		kMayanBowlsAndJars, kExhibitBucket, kTeapot, kExhibitBowl,
		kThermalChamber, kKey, kNitrogenTank, kElectronMicroscope,
		kMakeshiftBullet, kPileOfMakeshiftBullets,
		kMendedStrap, kBottleWithWater, kBottleWithGasoline, kFilledMeasuringTube,
		kPolishedStone, kStoneOfLight, kMakeshiftMortar, kSteamOutletLever, kPressureWheel,
		kMakeshiftMiningTool, kHotScalpel, kOpenPods, kTextCount
	};
	explicit RunawayStrings(Common::Language language) : _language(language) {}
	bool load();
	Common::U32String text(TextId id) const;
	Common::U32String itemName(byte chapter, byte item) const;
	Common::U32String hotspotName(uint16 scene, byte hotspot) const;
	Common::U32String sceneActionCaption(uint16 scene, byte hotspot, byte item, bool look, const char *variant = nullptr) const;
	Common::U32String combinationCaption(byte chapter, byte item, byte target) const;
	static Common::U32String formatActionCaption(const Common::U32String &pattern, const Common::U32String &item, const Common::U32String &target);
	Common::U32String shackHotspotName(byte hotspot) const;

private:
	bool loadCatalog(const Common::Archive &archive, const char *language);
	Common::U32String lookup(const Common::String &key) const;
	Common::Language _language;
	Common::HashMap<Common::String, Common::U32String> _text;
};

} // End of namespace Hollywood

#endif

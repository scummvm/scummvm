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

#ifndef ZOOMBINI_STATE_H
#define ZOOMBINI_STATE_H

#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/str.h"

namespace Common {

class WriteStream;

} // namespace Common

namespace Mohawk {

class MohawkEngine_Zoombini;
class ZmbFeature;

/**
 * Transient SFX-selection outcomes returned while a page flag is updated.
 *
 * These values are complete selector codes, not a difficulty ordinal or a
 * set of independently combinable bits. Values 5 and 12 are returned as
 * literals; bits 4 and 8 do not have standalone meanings.
 *
 * The Easy group represents normal Puzzle Level 1. The Hard group represents
 * normal Puzzle Levels 2-4. These names identify the source difficulty family,
 * not a guarantee that every consumer will play a literally easy or hard SFX.
 * The fixed Hard selectors 2 and 12 are emitted only for the first two recorded
 * Hard-group triggers while the internal route level is 1. Later Level 2
 * visits and all Level 3-4 visits return @ref ZmbSfxGroupFlags::kRandom_00,
 * allowing each consumer to make a route-level-dependent selection.
 *
 * First- and second-trigger terminology refers to the two persistent
 * Hard-group marker bits, not to the page's total visit count. A page may have
 * any number of earlier Easy visits, and XFER may evaluate a copied page flag
 * without committing either marker.
 *
 * The exact return conditions and page-flag updates are:
 * @code
 * Mode      Puzzle  routeLevel  First  Second  Return  Page-flag update
 * Normal    L1      0           *      *       1       Increment visit count
 * Normal    L2      1           0      *       2       Increment count; set First
 * Normal    L2      1           1      0       12      Increment count; set Second
 * Normal    L2      1           1      1       0       Increment visit count
 * Normal    L3      2           *      *       0       Increment visit count
 * Normal    L4      3           *      *       0       Increment visit count
 * Practice  *       *           *      *       5       Do not change page flag
 * @endcode
 *
 * First and Second denote @ref ZmbStateFile::PageFlag::kHardGroupFirstTrigger and
 * @ref ZmbStateFile::PageFlag::kHardGroupSecondTrigger. An asterisk means that the value is
 * not evaluated. A normally formed page flag has Second clear while First is
 * clear, although the first Level 2 branch does not require that invariant.
 * Incrementing the visit count is a saturating increment of the low 12 bits.
 */
enum class ZmbSfxGroupFlags : int16 {
	/**
	 * Use random or route-level-dependent SFX selection.
	 *
	 * Normal Level 2 returns this after both Hard-group markers are recorded.
	 * Normal Levels 3-4 always return this selector. Consumers may then combine
	 * it with the active puzzle level to choose an Easy, Hard, random, or silent
	 * SFX as appropriate for that page.
	 */
	kRandom_00 = 0,
	/**
	 * Use the fixed Easy-group selection.
	 *
	 * Normal mode returns this only for Puzzle Level 1, when the internal route
	 * level is 0. Easy describes the originating difficulty family rather than
	 * requiring every consumer to play an SFX explicitly labeled as easy.
	 */
	kEasy_01 = 1,
	/**
	 * Use the first fixed Hard-group selection.
	 *
	 * Normal mode returns this while the internal route level is 1 and
	 * @ref ZmbStateFile::PageFlag::kHardGroupFirstTrigger is clear. The state helper then sets
	 * that marker. FirstTrigger therefore means the first recorded Hard-group
	 * trigger, not the first lifetime visit to the page.
	 */
	kHardFirstTrigger_02 = 2,
	/**
	 * Use the practice-mode SFX selection.
	 *
	 * Practice returns this before updating the visit count or either Hard-group
	 * marker. Consumers may inspect the selected practice level to choose between
	 * their Easy- and Hard-family SFX behavior.
	 */
	kPractice_05 = 5,
	/**
	 * Use the second fixed Hard-group selection.
	 *
	 * Normal mode returns this only while the internal route level is 1,
	 * @ref ZmbStateFile::PageFlag::kHardGroupFirstTrigger is set, and
	 * @ref ZmbStateFile::PageFlag::kHardGroupSecondTrigger is clear. The state helper then sets
	 * the second marker. This is not a generic not-first-visit value: once both
	 * markers are set, later visits return @ref ZmbSfxGroupFlags::kRandom_00.
	 * SecondTrigger describes the marker state rather than an exact helper-call
	 * count, because callers such as XFER may evaluate a copied page flag.
	 */
	kHardSecondTrigger_12 = 12
};

/**
 * Source puzzle index used only to select an XFER scene and destination.
 *
 * XFER derives a destination ZmbDestPageKind from this value, then clears it after the transition.
 * SI is therefore a source-puzzle index, not the persisted current-page ID.
 * The enum ordering supports XFER arithmetic without a translation table.
 */
enum class ZmbSrcPageKind : int16 {
	/** No pending XFER source. */
	kMinus1 = -1,
	/** Reset value after XFER cleanup. */
	kTown_00 = 0,
	/** Zoombini Isle to Bridge. */
	kPicker_01 = 1,
	/** Bridge to Stone Cold Caves. */
	kBridge_02 = 2,
	/** Stone Cold Caves to Pizza. */
	kTunnels_03 = 3,
	/** Pizza to Basecamp 1. */
	kPizza_04 = 4,
	/** Basecamp 1 north exit to Ferry. */
	kBC1North_05 = 5,
	/** Basecamp 1 south exit to Fleens. */
	kBC1South_06 = 6,
	/** Ferry to Lilly. */
	kFerry_07 = 7,
	/** Lilly to Slides. */
	kLilly_08 = 8,
	/** Slides to Basecamp 2. */
	kSlides_09 = 9,
	/** Fleens to Hotel. */
	kFleens_10 = 10,
	/** Hotel to Net. */
	kHotel_11 = 11,
	/** Net to Basecamp 2. */
	kNet_12 = 12,
	/** Basecamp 2 to The Lion's Lair. */
	kBC2_13 = 13,
	/** The Lion's Lair to Smoke. */
	kCaves_14 = 14,
	/** Smoke to Maze. */
	kSmoke_15 = 15,
	/** Maze to Zoombiniville. */
	kMaze_16 = 16,
};

/**
 * Direct page ID used by the page dispatcher and saved as the current page.
 *
 * During an XFER it is also the destination page ID.
 * Outside an XFER it is not inherently a destination.
 */
enum class ZmbDestPageKind : int16 {
	/** Sentinel used by transient XFER state */
	kUnk_M1 = -1,
	/** No active page / default value */
	kUnk_00 = 0,
	/** Roadmap */
	kMap_01 = 1,
	/** Zoombini Isle */
	kIsle_03 = 3,
	/** Base Camp 1 - Shelter Rock */
	kBC1_04 = 4,
	/** Base Camp 2 - Shade Tree */
	kBC2_05 = 5,
	/** Zoombiniville */
	kTown_06 = 6,
	/** Allergic Cliffs */
	kBridge_07 = 7,
	/** Stone Cold Caves */
	kTunnels_08 = 8,
	/** Pizza Pass */
	kPizza_09 = 9,
	/** Captain Cajun's Ferryboat */
	kFerry_10 = 10,
	/** Titanic Tattooed Toads */
	kLilly_11 = 11,
	/** Stone Rise */
	kSlides_12 = 12,
	/** Fleens! */
	kFleens_13 = 13,
	/** Hotel Dimensia */
	kHotel_14 = 14,
	/** Mudball Wall */
	kNet_15 = 15,
	/** The Lion's Lair */
	kCaves_16 = 16,
	/** Mirror Machine */
	kSmoke_17 = 17,
	/** Bubblewonder Abyss */
	kMaze_18 = 18,
};

/** Canonical route index used by @ref ZmbStateFile::_routeLevels and completion counters. */
enum class ZmbRouteId : uint16 {
	/** Allergic Cliffs to Basecamp 1. */
	kBigBadHungry = 0,
	/** Basecamp 1 north route to Basecamp 2. */
	kWhosBayou = 1,
	/** Basecamp 1 south route to Basecamp 2. */
	kDeepDarkForest = 2,
	/** Basecamp 2 route to Zoombiniville. */
	kMontDespair = 3,
};

/** Four route bands, each with four map-progress states. */
enum class ZmbSubRouteid : uint16 {
	/** Big Bad Hungry, level 1 map state. */
	kBigBadHungry_0 = 0,
	/** Big Bad Hungry, level 2 map state. */
	kBigBadHungry_1 = 1,
	/** Big Bad Hungry, level 3 map state. */
	kBigBadHungry_2 = 2,
	/** Big Bad Hungry, level 4 map state. */
	kBigBadHungry_3 = 3,
	/** Who's Bayou, level 1 map state. */
	kWhosBayou_0 = 4,
	/** Who's Bayou, level 2 map state. */
	kWhosBayou_1 = 5,
	/** Who's Bayou, level 3 map state. */
	kWhosBayou_2 = 6,
	/** Who's Bayou, level 4 map state. */
	kWhosBayou_3 = 7,
	/** Deep Dark Forest, level 1 map state. */
	kDeepDarkForest_0 = 8,
	/** Deep Dark Forest, level 2 map state. */
	kDeepDarkForest_1 = 9,
	/** Deep Dark Forest, level 3 map state. */
	kDeepDarkForest_2 = 10,
	/** Deep Dark Forest, level 4 map state. */
	kDeepDarkForest_3 = 11,
	/** Mountain of Despair, level 1 map state. */
	kMontDespair_0 = 12,
	/** Mountain of Despair, level 2 map state. */
	kMontDespair_1 = 13,
	/** Mountain of Despair, level 3 map state. */
	kMontDespair_2 = 14,
	/** Mountain of Despair, level 4 map state. */
	kMontDespair_3 = 15,
};

/**
 * Shared identity and debug labels for one XFER route transition.
 *
 * srcPuzzlePage is ZmbDestPageKind::kUnk_00 for departures from the Isle or a
 * basecamp, where no puzzle completion flag is recorded.
 *
 * The debugger builds its route description from routeName, srcPageName,
 * and destName instead of storing a completed sentence here.
 * routeId is the canonical zero-based route used by persisted route state.
 */
struct ZmbXferRouteInfo {
	static constexpr const char *kXferPageNameAllergicCliffs = "Allergic Cliffs";
	static constexpr const char *kXferPageNameStoneColdCaves = "Stone Cold Caves";
	static constexpr const char *kXferPageNamePizzaPass = "Pizza Pass";
	static constexpr const char *kXferPageNameShelterRock = "Shelter Rock";
	static constexpr const char *kXferPageNameCaptainCajunsFerryboat = "Captain Cajun's Ferryboat";
	static constexpr const char *kXferPageNameTitanicTattooedToads = "Titanic Tattooed Toads";
	static constexpr const char *kXferPageNameStoneRise = "Stone Rise";
	static constexpr const char *kXferPageNameShadeTree = "Shade Tree";
	static constexpr const char *kXferPageNameFleens = "Fleens!";
	static constexpr const char *kXferPageNameHotelDimensia = "Hotel Dimensia";
	static constexpr const char *kXferPageNameMudballWall = "Mudball Wall";
	static constexpr const char *kXferPageNameLionsLair = "The Lion's Lair";
	static constexpr const char *kXferPageNameMirrorMachine = "Mirror Machine";
	static constexpr const char *kXferPageNameBubblewonderAbyss = "Bubblewonder Abyss";
	static constexpr const char *kXferPageNameZoombiniville = "Zoombiniville";

	static constexpr const char *kXferRouteNameFromIsle = "From Isle";
	static constexpr const char *kXferRouteNameBigBadHungry = "The Big, the Bad, and the Hungry";
	static constexpr const char *kXferRouteNameWhosBayou = "Who's Bayou?";
	static constexpr const char *kXferRouteNameDeepDarkForest = "The Deep, Dark Forest";
	static constexpr const char *kXferRouteNameMountainsOfDespair = "Mountains of Despair";
	static constexpr const char *kXferRouteNameToTown = "To Town";

	/** Shared XFER route table. */
	static const ZmbXferRouteInfo kXferRouteInfos[];

	/** Return the shared XFER route table and its entry count. */
	static const ZmbXferRouteInfo *getZmbXferRouteInfos(uint32 &routeCount);

	/** Find the shared XFER route entry for a source SI page. */
	static const ZmbXferRouteInfo *getZmbXferRouteInfo(ZmbSrcPageKind sourcePage);

	/** Return whether this route arrives at a Basecamp or Town container. */
	bool entersContainer() const;

	const char *name;
	ZmbDestPageKind destPage;
	const char *destName;
	ZmbSrcPageKind srcPage;
	ZmbDestPageKind srcPuzzlePage;
	const char *srcPageName;
	/** Canonical route owning this transition. */
	ZmbRouteId routeId;
	const char *routeName;
};

struct ZmbTrait {
	/** Sentinel returned when one of the four trait slots is empty. */
	static constexpr int16 SNOID_INCOMPLETE = -1;
	/** Number of canonical four-trait combinations. */
	static constexpr int16 SNOID_MAX = 625; // 5^4 combinations
	/** Serialized value used when a trait slot is empty. */
	static constexpr byte TRAIT_NONE = 0;

	/** Unified trait category index in save-file trait order. */
	enum TraitKind : byte {
		/** Hair trait byte. */
		kTraitHair = 0,
		/** Eye trait byte. */
		kTraitEyes = 1,
		/** Nose trait byte. */
		kTraitNose = 2,
		/** Feet trait byte. */
		kTraitFeet = 3,
	};

	/** Hair trait (1-5 valid, 0 = none). */
	enum HairKind : byte {
		/** @remark Male voice SFX */
		kHairSpiked = 1,
		/** @remark Female voice SFX */
		kHairPonytail = 2,
		/** @remark Male voice SFX */
		kHairFlattop = 3,
		/** @remark Male voice SFX */
		kHairBalding = 4,
		/** @remark Female voice SFX */
		kHairGreenCap = 5,
	};

	/** Eye trait (1-5 valid, 0 = none). */
	enum EyeKind : byte {
		kEyeNormal = 1,
		kEyeCyclops = 2,
		kEyeSleepy = 3,
		kEyeGlasses = 4,
		kEyeSunglasses = 5,
	};

	/** Nose trait (1-5 valid, 0 = none). */
	enum NoseKind : byte {
		kNoseGreen = 1,
		kNoseYellow = 2,
		kNoseRed = 3,
		kNosePurple = 4,
		kNoseBlue = 5,
	};
	/** Feet trait (1-5 valid, 0 = none). */
	enum FeetKind : byte {
		kFeetSneakers = 1,
		kFeetSkates = 2,
		kFeetSpring = 3,
		kFeetWheels = 4,
		kFeetPropeller = 5,
	};

	/** 0=none, 1=Spikey, 2=Ponytail, 3=Flattop, 4=Balding, 5=GreenCap. */
	byte _hair = TRAIT_NONE;
	/** 0=none, 1=NormalEyed, 2=Cyclops, 3=SleepyEyed, 4=Glasses, 5=Sunglasses. */
	byte _eyes = TRAIT_NONE;
	/** 0=none, 1=Green, 2=Yellow, 3=Red, 4=Purple, 5=Blue. */
	byte _nose = TRAIT_NONE;
	/** 0=none, 1=Sneakers, 2=Skates, 3=Spring, 4=Wheels, 5=Propeller. */
	byte _feet = TRAIT_NONE;

	/** Construct an empty trait record. */
	ZmbTrait() = default;
	ZmbTrait(HairKind hair, EyeKind eyes, NoseKind nose, FeetKind feet) : _hair(hair), _eyes(eyes), _nose(nose), _feet(feet) {}

	/** Convert the four trait values to the canonical roster ID. */
	int16 snoidId() const;
	/** Return whether every serialized trait byte is within its valid range. */
	bool hasValidValues() const { return _hair <= 5 && _eyes <= 5 && _nose <= 5 && _feet <= 5; }
	/** Return whether all four trait slots contain valid nonzero values. */
	bool isComplete() const {
		return hasValidValues() && 0 < _hair && 0 < _eyes && 0 < _nose && 0 < _feet;
	}
	/** Replace out-of-range serialized trait bytes with empty slots. */
	void recoverSerializedValues();
	/** Serialize the four trait bytes in save-file order. */
	void sync(Common::Serializer &s);
	/** Return whether every canonical trait slot equals @p other. */
	bool operator==(const ZmbTrait &other) const {
		return _hair == other._hair && _eyes == other._eyes &&
			   _nose == other._nose && _feet == other._feet;
	}
	/** Return whether any canonical trait slot differs from @p other. */
	bool operator!=(const ZmbTrait &other) const { return !(*this == other); }

	// ---------- Trait category enum and debug name helpers ----------

	/**
	 * Get a trait value by its zero-based category.
	 * @param kind TraitKind (hair/eyes/nose/feet).
	 */
	byte getTraitValue(TraitKind kind) const {
		switch (kind) {
		case kTraitHair:
			return _hair;
		case kTraitEyes:
			return _eyes;
		case kTraitNose:
			return _nose;
		case kTraitFeet:
			return _feet;
		default:
			error("ZmbTrait::getTraitValue: invalid kind %d", static_cast<int>(kind));
			return TRAIT_NONE;
		}
	}

	/**
	 * Access a trait byte by its zero-based category in save-file order.
	 * @param index  0=hair, 1=eyes, 2=nose, 3=feet.
	 */
	byte &operator[](int index) {
		switch (index) {
		case kTraitHair:
			return _hair;
		case kTraitEyes:
			return _eyes;
		case kTraitNose:
			return _nose;
		case kTraitFeet:
			return _feet;
		default:
			error("ZmbTrait::operator[]: invalid index %d", index);
			return _hair; // Must not arrive here
		}
	}

	/**
	 * Access a trait byte by its zero-based category in save-file order.
	 * @param index  0=hair, 1=eyes, 2=nose, 3=feet.
	 */
	const byte &operator[](int index) const {
		return const_cast<ZmbTrait *>(this)->operator[](index);
	}

	/**
	 * Get the human-readable name for a trait category.
	 * @param kind Trait category in hair/eyes/nose/feet order.
	 */
	static const char *debugTraitKindName(TraitKind kind) {
		switch (kind) {
		case kTraitHair:
			return "HAIR";
		case kTraitEyes:
			return "EYES";
		case kTraitNose:
			return "NOSE";
		case kTraitFeet:
			return "FEET";
		default:
			return "?";
		}
	}

	/**
	 * Convert a zero-based trait index to its category.
	 * @param index Zero-based index in hair/eyes/nose/feet order.
	 */
	static TraitKind traitKindFromIndex(int index) {
		if (0 <= index && index < 4)
			return static_cast<TraitKind>(index);
		error("ZmbTrait::getTraitKindIndex: invalid index %d", index);
		return kTraitHair;
	}

	/**
	 * Get human-readable trait value name.
	 * @param kind      TraitKind (hair/eyes/nose/feet).
	 * @param value     1-5 trait value.
	 */
	static const char *debugTraitValueName(TraitKind kind, int value) {
		static const char *kZoombiniTraitNames[4][5] = {
			{"Spikey", "Ponytail", "Flattop", "Balding", "GreenCap"},
			{"NormalEyed", "Cyclops", "SleepyEyed", "Glasses", "Sunglasses"},
			{"Green", "Yellow", "Red", "Purple", "Blue"},
			{"Sneakers", "Skates", "Spring", "Wheels", "Propeller"}};
		const byte traitIndex = static_cast<byte>(kind);
		if (traitIndex < 4 && 1 <= value && value <= 5)
			return kZoombiniTraitNames[traitIndex][value - 1];
		return "?";
	}

	/** Get human-readable trait value names in hair/eyes/nose/feet order. */
	Common::String toStr() const {
		return Common::String::format("%s, %s, %s, %s",
									  debugTraitValueName(kTraitHair, _hair),
									  debugTraitValueName(kTraitEyes, _eyes),
									  debugTraitValueName(kTraitNose, _nose),
									  debugTraitValueName(kTraitFeet, _feet));
	}
};

// For Stored Zoombinis (the ones which are on Rest or Ville)
struct ZmbStateStoredEntry {
	/** Fixed byte width of a stored Zoombini's serialized name. */
	static const uint32 kNameByteCount = 10;

private:
	/** Trait payload serialized for this stored entry. */
	ZmbTrait _traits;

public:
	/** Stored position of this Town/Basecamp Zoombini. */
	Common::Rect _rect;

private:
	byte _name[kNameByteCount] = {0};

public:
	const ZmbTrait &getTraits() const { return _traits; }
	void setTraits(const ZmbTrait &traits) { _traits = traits; }

	/** Serialize the stored entry using the selected release layout. */
	void sync(Common::Serializer &s, bool isTlcLayout);
	/** Decode the stored name, or return empty when its NUL terminator is missing. */
	Common::U32String getName(MohawkEngine_Zoombini *vm) const;
	/** Encode and assign a stored name through the active engine text codec. */
	void setName(MohawkEngine_Zoombini *vm, const Common::U32String &name);
	/** Return whether the serialized traits and NUL-terminated name are valid. */
	bool isSerializedStateValid() const;
	/** Normalize serialized traits and terminate the name within its fixed field. */
	void recoverSerializedState();
	/** Return whether the serialized name field contains a non-empty name. */
	bool hasSerializedName() const;
	/** Return whether the serialized name contains a byte outside ASCII. */
	bool hasNonAsciiName() const;

private:
	/** Return the bounded serialized name length, or the field size when unterminated. */
	uint32 getNameLength() const;
};

struct ZmbStateStoredChunk {
private:
	/** Range: 0 ~ 120, each column has 5 entries, so max 600 entries in total. */
	int16 _leftmostColumnIdx = 0;
	/**
	 * Serialized belt bookkeeping count.
	 * Original saves can contain a lower value after a failed shelter drop
	 * restored its slot without restoring this header. Complete entry traits,
	 * rather than this field, are therefore authoritative for physical occupancy.
	 */
	int16 _storedCount = 0;
	/** Fixed-capacity stored-entry array. */
	ZmbStateStoredEntry _entries[625];

public:
	int16 getLeftmostColumnIdx() const { return _leftmostColumnIdx; }
	void setLeftmostColumnIdx(int16 columnIdx) { _leftmostColumnIdx = columnIdx; }
	int16 getStoredCount() const { return _storedCount; }
	void setStoredCount(int16 count) { _storedCount = count; }
	bool increaseStoredCount(int16 count) {
		if (getEntryCapacity() < _storedCount + count)
			return false;

		_storedCount += count;
		return true;
	}
	bool decreaseStoredCount(int16 count) {
		if (_storedCount < count)
			return false;

		_storedCount -= count;
		return true;
	}
	int16 getEntryCapacity() const { return static_cast<int16>(ARRAYSIZE(_entries)); }
	ZmbStateStoredEntry &getEntry(int16 index) { return _entries[index]; }
	const ZmbStateStoredEntry &getEntry(int16 index) const { return _entries[index]; }
	/** Count physically populated storage slots from complete trait records. */
	int16 getPopulatedEntryCount() const;

	/** Return whether serialized headers, traits, and names are safe to consume. */
	bool isSerializedStateValid() const;
	/** Normalize serialized headers, traits, and names for safe loading. */
	void recoverSerializedState();
	/** Serialize the stored chunk using the selected release layout. */
	void sync(Common::Serializer &s, bool isTlcLayout);
};

/** Represents a snoid in an active pack. */
struct ZmbStateActiveEntry {
private:
	/**
	 * 0x00: Zoombini traits (4 bytes)
	 * The four trait bytes are stored in the same order as the ZmbTrait struct.
	 */
	ZmbTrait _traits;

	/**
	 * 0x04: X coordinate of unoccupied slot position (WORD)
	 * When the slot is unoccupied, this is the X screen position used for animation.
	 */
	uint16 _posX = 0;
	/**
	 * 0x06: Y coordinate of unoccupied slot position (WORD)
	 * When the slot is unoccupied, this is the Y screen position used for animation.
	 */
	uint16 _posY = 0;

	/**
	 * 0x08: Occupied flag (BYTE)
	 * 0 = unoccupied, 1 = occupied.
	 */
	byte _bIsOccupied = 0;

	/** 0x09-0x12: Zoombini name (10 bytes) */
	byte _name[10] = {0};

public:
	const ZmbTrait &getTraits() const { return _traits; }
	void setTraits(const ZmbTrait &traits) { _traits = traits; }

	Common::Point getPos() const { return Common::Point(_posX, _posY); }
	void setPos(const Common::Point &position) {
		_posX = static_cast<uint16>(position.x);
		_posY = static_cast<uint16>(position.y);
	}

	bool getIsOccupied() const { return _bIsOccupied != 0; }
	void setIsOccupied(bool occupied) { _bIsOccupied = occupied ? 1 : 0; }

	/** Return whether serialized traits, occupancy, and name bytes are valid. */
	bool isSerializedStateValid() const;
	/** Normalize serialized traits, occupancy, and name bytes for safe loading. */
	void recoverSerializedState();
	/** Serialize one active-pack entry using the selected release layout. */
	void sync(Common::Serializer &s, bool isTlcLayout);
	/** Decode the active-pack name, or return empty when its NUL terminator is missing. */
	Common::U32String getU32Name(MohawkEngine_Zoombini *vm) const;
	/** Encode and assign an active-pack name through the active engine text codec. */
	void setU32Name(MohawkEngine_Zoombini *vm, const Common::U32String &name);
	/** Return whether the serialized name field contains a non-empty name. */
	bool hasSerializedName() const;
	/** Return whether the serialized name contains a byte outside ASCII. */
	bool hasNonAsciiName() const;

private:
	/** Return the bounded serialized name length, or the field size when unterminated. */
	uint32 getNameLength() const;
};

/**
 * The serialized pack contains 6 header bytes and 32 entries.
 * It occupies 0x266 bytes in v1.x and 0x286 bytes in TLC.
 * Each TLC entry has one trailing pad byte.
 */
struct ZmbStateActivePack {
private:
	/** Number of valid active-pack entries. */
	int16 _wPackZmbCount = 0;
	/**
	 * +0x02: Skip occupied entries while materializing this pack as runners.
	 * Together with @ref _bSkipUnoccupiedEntries, 0/0 loads all entries, 1/0
	 * loads only non-occupied entries, 0/1 loads only occupied entries, and
	 * 1/1 loads none. The flags describe snapshot filtering, not ownership.
	 */
	int16 _bSkipOccupiedEntries = 0;
	/**
	 * +0x04: Skip non-occupied entries while materializing this pack.
	 * A shelter departure snapshot can retain both departing occupied entries
	 * and resident non-occupied entries; the skip pair selects the live subset.
	 */
	int16 _bSkipUnoccupiedEntries = 0;
	/** Fixed-capacity active-pack entry array. */
	ZmbStateActiveEntry _entries[32];

public:
	int16 getPackZmbCount() const { return _wPackZmbCount; }
	int16 getEntryCapacity() const { return static_cast<int16>(ARRAYSIZE(_entries)); }
	ZmbStateActiveEntry &getEntry(int16 index) { return _entries[index]; }
	const ZmbStateActiveEntry &getEntry(int16 index) const { return _entries[index]; }

	bool appendEntry(const ZmbStateActiveEntry &entry) {
		if (_wPackZmbCount < 0 || getEntryCapacity() <= _wPackZmbCount)
			return false;

		_entries[_wPackZmbCount] = entry;
		_wPackZmbCount += 1;
		return true;
	}

	/**
	 * Adds one logical entry at a caller-selected physical slot without shifting
	 * the existing slots. The route transfer path uses this for an existing
	 * empty slot inside the destination pack prefix.
	 */
	bool appendEntryAt(int16 index, const ZmbStateActiveEntry &entry) {
		if (index < 0 || getEntryCapacity() <= index ||
			_wPackZmbCount < index || getEntryCapacity() <= _wPackZmbCount)
			return false;

		_entries[index] = entry;
		_wPackZmbCount += 1;
		return true;
	}

	/**
	 * Writes one physical serialized slot without changing the logical count.
	 * This is used for the original save-mode duplicate tail.
	 */
	bool writeEntryAt(int16 index, const ZmbStateActiveEntry &entry) {
		if (index < 0 || getEntryCapacity() <= index)
			return false;

		_entries[index] = entry;
		return true;
	}

	bool removeEntryAt(int16 index) {
		if (index < 0 || _wPackZmbCount <= index)
			return false;

		for (int16 i = index; i + 1 < _wPackZmbCount; i++)
			_entries[i] = _entries[i + 1];
		_wPackZmbCount -= 1;
		return true;
	}

	bool removeEntriesFromBack(int16 count) {
		if (count < 0 || _wPackZmbCount < count)
			return false;

		_wPackZmbCount -= count;
		return true;
	}

	/** Clears the logical entry count without erasing serialized entry slots. */
	void clearEntries() { _wPackZmbCount = 0; }

	bool getSkipOccupiedEntries() const { return _bSkipOccupiedEntries != 0; }
	void setSkipOccupiedEntries(bool skip) { _bSkipOccupiedEntries = skip ? 1 : 0; }
	bool getSkipUnoccupiedEntries() const { return _bSkipUnoccupiedEntries != 0; }
	void setSkipUnoccupiedEntries(bool skip) { _bSkipUnoccupiedEntries = skip ? 1 : 0; }

	/** Return whether the serialized header and entries are safe to consume. */
	bool isSerializedStateValid() const;
	/** Normalize the serialized header and entries for safe loading. */
	void recoverSerializedState();
	/** Serialize the active pack header, flags, and entries. */
	void sync(Common::Serializer &s, bool isTlcLayout);
	void copyTo(ZmbStateActivePack &dest) {
		dest = *this;
	}
};

/**
 * Represents the Zoombini game-state.
 *
 * The savefile has three layouts: v1.0BR (44549 bytes), v1.1US (44559
 * bytes), and v2.0TLC (48430-byte small or 48440-byte current). Europe
 * v1.1 retains the v1.0BR layout. Each serialized member below identifies
 * its byte address as (v1.0BR), (v1.1US), and (v2.0TLC). An address labelled
 * v2.0TLC applies to both TLC lengths unless the comment distinguishes the
 * 48430-byte small layout from the current 48440-byte layout.
 */
struct ZmbStateFile {
	/** Construct a state file with its release-independent defaults. */
	ZmbStateFile();

	/**
	 * Persisted two-byte page state.
	 *
	 * The low 12 bits hold the saturating visit count.
	 * The next two bits record the first and second normal Hard-group triggers.
	 * The serialized value remains one little-endian uint16.
	 */
	struct PageFlag {
		/** Bit masks for the visit count and normal Hard-group trigger state. */
		enum FlagMask : uint16 {
			/** Low 12 bits that hold the saturating visit count. */
			kVisitCountMask = 0x0FFF,
			/** First normal Hard-group trigger bit. */
			kHardGroupFirstTrigger = 0x1000,
			/** Second normal Hard-group trigger bit. */
			kHardGroupSecondTrigger = 0x2000
		};

		/** Return the unmodified serialized value for diagnostics and legacy comparisons. */
		uint16 getRaw() const { return _raw; }
		/** Return the visit count represented by one unwrapped serialized value. */
		static constexpr uint16 getVisitCountFromRaw(uint16 raw) { return raw & kVisitCountMask; }
		/** Return the visit count stored in the low 12 bits. */
		uint16 getVisitCount() const { return getVisitCountFromRaw(_raw); }
		/** Return whether the state has no recorded visit or trigger flag. */
		bool isZero() const { return _raw == 0; }
		/** Return whether the first normal Hard-group trigger was recorded. */
		bool hasFirstHardGroupTrigger() const { return (_raw & kHardGroupFirstTrigger) != 0; }
		/** Return whether the second normal Hard-group trigger was recorded. */
		bool hasSecondHardGroupTrigger() const { return (_raw & kHardGroupSecondTrigger) != 0; }
		/** Return whether either normal Hard-group trigger was recorded. */
		bool hasHardGroupTrigger() const { return hasFirstHardGroupTrigger() || hasSecondHardGroupTrigger(); }
		/** Increment the visit count unless it already occupies every visit-count bit. */
		void incVisitCount() {
			if (getVisitCount() < kVisitCountMask)
				_raw += 1;
		}
		/** Record the first normal Hard-group trigger. */
		void setFirstHardGroupTrigger() { _raw |= kHardGroupFirstTrigger; }
		/** Record the second normal Hard-group trigger. */
		void setSecondHardGroupTrigger() { _raw |= kHardGroupSecondTrigger; }
		/** Clear both normal Hard-group trigger bits without changing the visit count. */
		void clearHardGroupTriggers() {
			_raw &= static_cast<uint16>(~(kHardGroupFirstTrigger |
										  kHardGroupSecondTrigger));
		}
		/** Serialize the stored uint16 without changing the page-flag byte order. */
		void sync(Common::Serializer &s) { s.syncAsUint16LE(_raw); }

	private:
		uint16 _raw = 0;
	};

	/**
	 * Retrieve the persisted page flag corresponding to one page type.
	 * @param pageType The target page identifier.
	 * @return A reference to the corresponding `_pageFlag...` member.
	 */
	PageFlag &getPageFlagFromPageType(ZoombiniPageType pageType);

	/**
	 * (v1.0BR) 0x0000
	 * (v1.1US) 0x0000
	 * (v2.0TLC) 0x0000
	 * Magic value 0x006B, stored big-endian in the state file (on-disk bytes
	 * 00 6B).
	 * The roster stores the same value little-endian instead.
	 */
	uint16 _magic006B;
	/**
	 * (v1.0BR) 0x0002
	 * (v1.1US) 0x0002
	 * (v2.0TLC) 0x0002
	 * Auto-sticky mouse delay threshold (big-endian in file), default 0x1E (30).
	 */
	uint16 _autoStickyDelay = 0x001E;

	// Flags begin at (v1.0BR/v1.1US/v2.0TLC) 0x0004.
private:
	/**
	 * (v1.0BR) 0x0004
	 * (v1.1US) 0x0004
	 * (v2.0TLC) 0x0004
	 * SFX enable flag, toggled by Ctrl+D and options menu button 5.
	 * It gates both ScummVM SFX and Speech mixer types.
	 */
	byte _flagSfxEnable = 1;
	/**
	 * (v1.0BR) 0x0005
	 * (v1.1US) 0x0005
	 * (v2.0TLC) 0x0005
	 * BGM enable flag, toggled by Ctrl+B and options menu button 6.
	 * It gates both PCM Music and MIDI background music.
	 */
	byte _flagBgmEnable = 1;
	/**
	 * (v1.0BR) 0x0006
	 * (v1.1US) 0x0006
	 * (v2.0TLC) 0x0006
	 * Sticky Mouse enable flag, toggled by Ctrl+J and options menu button 7.
	 */
	byte _flagStickyMouseEnable = 1;
	/**
	 * (v1.0BR) 0x0007
	 * (v1.1US) 0x0007
	 * (v2.0TLC) 0x0007
	 * Cursor Visible Flag.
	 * Runtime global: toggled by Ctrl+H
	 */
	byte _flagCursorVisible = 1;
	/**
	 * (v1.0BR) 0x0008
	 * (v1.1US) 0x0008
	 * (v2.0TLC) 0x0008
	 * Built-in debug mode flag.
	 * Set by the hidden typed code and serialized with game state.
	 */
	byte _flagDebug = 0;
	/**
	 * (v1.0BR) 0x0009
	 * (v1.1US) 0x0009
	 * (v2.0TLC) 0x0009
	 * Auto-Sticky Mouse Flag.
	 * Runtime global: toggled by Ctrl+U
	 */
	byte _flagAutoStickyMouse = 0;
	/**
	 * (v1.0BR)  0x000A-0x000B
	 * (v1.1US)  0x000A-0x000B
	 * v1.x transition-disable flag.
	 */
	uint16 _v1TransitionsDisable = 0;
	/**
	 * (v2.0TLC) 0x000A
	 * TouchSense enable flag. This field has no v1.x equivalent.
	 * ScummVM preserves this setting but does not implement its hardware feedback.
	 */
	byte _tlcTouchSenseEnable = 1;
	/**
	 * (v2.0TLC) 0x000B
	 * Help Audio enable flag. This field has no v1.x equivalent.
	 */
	byte _tlcHelpAudioEnable = 1;

public:
	bool getSfxEnabled() const { return _flagSfxEnable != 0; }
	void setSfxEnabled(bool enabled) { _flagSfxEnable = enabled ? 1 : 0; }
	bool getBgmEnabled() const { return _flagBgmEnable != 0; }
	void setBgmEnabled(bool enabled) { _flagBgmEnable = enabled ? 1 : 0; }
	bool getStickyMouseEnabled() const { return _flagStickyMouseEnable != 0; }
	void setStickyMouseEnabled(bool enabled) { _flagStickyMouseEnable = enabled ? 1 : 0; }
	bool getCursorVisible() const { return _flagCursorVisible != 0; }
	void setCursorVisible(bool visible) { _flagCursorVisible = visible ? 1 : 0; }
	bool getDebugEnabled() const { return _flagDebug != 0; }
	void setDebugEnabled(bool enabled) { _flagDebug = enabled ? 1 : 0; }
	bool getAutoStickyMouseEnabled() const { return _flagAutoStickyMouse != 0; }
	void setAutoStickyMouseEnabled(bool enabled) { _flagAutoStickyMouse = enabled ? 1 : 0; }
	bool getV1TransitionsDisabled() const { return _v1TransitionsDisable != 0; }
	void setV1TransitionsDisabled(bool disabled) { _v1TransitionsDisable = disabled ? 1 : 0; }
	bool getTouchSenseEnabled() const { return _tlcTouchSenseEnable != 0; }
	void setTouchSenseEnabled(bool enabled) { _tlcTouchSenseEnable = enabled ? 1 : 0; }
	bool getHelpAudioEnabled() const { return _tlcHelpAudioEnable != 0; }
	void setHelpAudioEnabled(bool enabled) { _tlcHelpAudioEnable = enabled ? 1 : 0; }
	/**
	 * (v1.0BR)  0x000C-0x000F
	 * (v1.1US)  0x000C-0x000F
	 * Fleens trait value rotations in v1.x.
	 * The Fleens setup adds the matching value in this array to each Zoombini trait modulo five.
	 * Difficulty levels 2 and 4 regenerate the values; levels 1 and 3 preserve them.
	 * These bytes are serialized state, not runtime-only page data.
	 * TLC stores the corresponding state in @ref _v2FleensTraitValueRotations.
	 */
	byte _v1FleensTraitValueRotations[4] = {0, 0, 0, 0};
	/**
	 * (v2.0TLC) 0x000C-0x000D
	 * Transition-disable flag. This field has no v1.x equivalent.
	 */
private:
	/** TLC-only persisted flag that disables transition scenes. */
	uint16 _v2TransitionsDisable = 0;

public:
	bool getV2TransitionsDisabled() const { return _v2TransitionsDisable != 0; }
	void setV2TransitionsDisabled(bool disabled) { _v2TransitionsDisable = disabled ? 1 : 0; }
	/**
	 * (v2.0TLC) 0x000E-0x0011
	 * Fleens trait value rotations in TLC.
	 * The Fleens setup adds the matching value in this array to each Zoombini trait modulo five.
	 * A zero first byte means that the table has not been initialized.
	 * Difficulty levels 2 and 4 regenerate the values; levels 1 and 3 preserve them.
	 */
	byte _v2FleensTraitValueRotations[4] = {0, 0, 0, 0};
	/**
	 * (v1.0BR)  0x0010-0x0013
	 * (v1.1US)  0x0010-0x0013
	 * Fleens destination slots in v1.x.
	 * A zero uses the unrotated attribute position; values 1-4 redirect it to that one-based Fleens body slot.
	 * Levels 1 and 2 contain all zeroes.
	 * The slots are persisted with the rotations above.
	 * TLC stores the corresponding state in @ref _v2FleensTraitDestSlots.
	 */
	byte _v1FleensTraitDestSlots[4] = {0, 0, 0, 0};
	/**
	 * (v2.0TLC) 0x0012-0x0015
	 * Fleens destination slots in TLC.
	 * A zero uses the unrotated attribute position; values 1-4 redirect it to that one-based Fleens body slot.
	 * Levels 1 and 2 contain all zeroes.
	 * Levels 3 and 4 retain a non-repeating slot permutation, regenerated at level 4.
	 */
	byte _v2FleensTraitDestSlots[4] = {0, 0, 0, 0};
	/**
	 * (v1.0BR) 0x0014-0x001D
	 * (v1.1US) 0x0014-0x001D
	 * (v2.0TLC) 0x0016-0x001F
	 * Basecamp1 mushroom color state (0-4).
	 */
	uint16 _bcOneMushroomColors[5] = {0, 0, 0, 0, 0};
	/**
	 * (v1.0BR) 0x001E
	 * (v1.1US) 0x001E
	 * (v2.0TLC) 0x0020
	 * Town zoombini grid scroll column (0-5).
	 * Restored on town load: scrolls the grid left by the stored column times 320 pixels.
	 * Updated by left/right click on the scroll arrows, wraps 0<->5.
	 */
	uint16 _townScrollCol = 0;
	/**
	 * (v1.0BR) 0x0020
	 * (v1.1US) 0x0020
	 * (v2.0TLC) 0x0022
	 * Less/More Action Mode Flag.
	 * Less action is 1 and suppresses some features.
	 * More action is 0.
	 */
private:
	/** Persisted reduced-action setting used by TLC controls. */
	uint16 _lessActionFlag = 0;

public:
	bool getLessActionEnabled() const { return _lessActionFlag != 0; }
	void setLessActionEnabled(bool enabled) { _lessActionFlag = enabled ? 1 : 0; }
	/**
	 * (v1.0BR) 0x0022
	 * (v1.1US) 0x0022
	 * (v2.0TLC) 0x0024
	 * (Unused) Fleens puzzle best score (0-99).
	 * Tracks the highest number of zoombinis successfully placed in the Fleens minigame.
	 * ScummVM does not use this serialized field for gameplay.
	 */
	uint16 _fleensHighScore = 0;
	/**
	 * (v1.0BR) 0x0024
	 * (v1.1US) 0x0024
	 * (v2.0TLC) 0x0026
	 * (Unused) Mudball Wall puzzle best score (0-999).
	 * Tracks the highest score achieved across all Mudball Wall sessions.
	 * ScummVM does not use this serialized field for gameplay.
	 */
	uint16 _mudballHighScore = 0;
	/**
	 * (v1.0BR) 0x0026
	 * (v1.1US) 0x0026
	 * (v2.0TLC) 0x0028
	 * Picker wave and boat animation state.
	 * Values 0-3 select both running, both stopped, waves running, or boat running.
	 */
	uint16 _pickerWaveBoatAnimationState = 0;

	// Page flags: v1.0BR/v1.1US 0x0028-0x0047; v2.0TLC 0x002A-0x0049.
	/**
	 * (v1.0BR) 0x0028
	 * (v1.1US) 0x0028
	 * (v2.0TLC) 0x002A
	 */
	PageFlag _pageFlagIsle;
	/**
	 * (v1.0BR) 0x002A
	 * (v1.1US) 0x002A
	 * (v2.0TLC) 0x002C
	 */
	PageFlag _pageFlagBridge;
	/**
	 * (v1.0BR) 0x002C
	 * (v1.1US) 0x002C
	 * (v2.0TLC) 0x002E
	 */
	PageFlag _pageFlagTunnels;
	/**
	 * (v1.0BR) 0x002E
	 * (v1.1US) 0x002E
	 * (v2.0TLC) 0x0030
	 */
	PageFlag _pageFlagPizza;
	/**
	 * (v1.0BR) 0x0030
	 * (v1.1US) 0x0030
	 * (v2.0TLC) 0x0032
	 */
	PageFlag _pageFlagBasecamp1;
	/**
	 * (v1.0BR) 0x0032
	 * (v1.1US) 0x0032
	 * (v2.0TLC) 0x0034
	 */
	PageFlag _pageFlagFerry;
	/**
	 * (v1.0BR) 0x0034
	 * (v1.1US) 0x0034
	 * (v2.0TLC) 0x0036
	 */
	PageFlag _pageFlagLilly;
	/**
	 * (v1.0BR) 0x0036
	 * (v1.1US) 0x0036
	 * (v2.0TLC) 0x0038
	 */
	PageFlag _pageFlagSlides;
	/**
	 * (v1.0BR) 0x0038
	 * (v1.1US) 0x0038
	 * (v2.0TLC) 0x003A
	 */
	PageFlag _pageFlagFleens;
	/**
	 * (v1.0BR) 0x003A
	 * (v1.1US) 0x003A
	 * (v2.0TLC) 0x003C
	 */
	PageFlag _pageFlagHotel;
	/**
	 * (v1.0BR) 0x003C
	 * (v1.1US) 0x003C
	 * (v2.0TLC) 0x003E
	 */
	PageFlag _pageFlagNet;
	/**
	 * (v1.0BR) 0x003E
	 * (v1.1US) 0x003E
	 * (v2.0TLC) 0x0040
	 */
	PageFlag _pageFlagBasecamp2;
	/**
	 * (v1.0BR) 0x0040
	 * (v1.1US) 0x0040
	 * (v2.0TLC) 0x0042
	 */
	PageFlag _pageFlagCaves;
	/**
	 * (v1.0BR) 0x0042
	 * (v1.1US) 0x0042
	 * (v2.0TLC) 0x0044
	 */
	PageFlag _pageFlagSmoke;
	/**
	 * (v1.0BR) 0x0044
	 * (v1.1US) 0x0044
	 * (v2.0TLC) 0x0046
	 */
	PageFlag _pageFlagMaze;
	/**
	 * (v1.0BR) 0x0046
	 * (v1.1US) 0x0046
	 * (v2.0TLC) 0x0048
	 */
	PageFlag _pageFlagTown;

	// Generated and stored Zoombini counts: v1.0BR/v1.1US 0x0048-0x004F; v2.0TLC 0x004A-0x0051.
	/**
	 * (v1.0BR) 0x0048
	 * (v1.1US) 0x0048
	 * (v2.0TLC) 0x004A
	 */
	int16 _zmbGeneratedCount = 0;
	/**
	 * (v1.0BR) 0x004A
	 * (v1.1US) 0x004A
	 * (v2.0TLC) 0x004C
	 */
	int16 _zmbStoredBC1Count = 0;
	/**
	 * (v1.0BR) 0x004C
	 * (v1.1US) 0x004C
	 * (v2.0TLC) 0x004E
	 */
	int16 _zmbStoredBC2Count = 0;
	/**
	 * (v1.0BR) 0x004E
	 * (v1.1US) 0x004E
	 * (v2.0TLC) 0x0050
	 */
	int16 _zmbStoredTownCount = 0;

	// Level flags: v1.0BR/v1.1US 0x0050-0x0061; v2.0TLC 0x0052-0x0063.
	/**
	 * (v1.0BR) 0x0050
	 * (v1.1US) 0x0050
	 * (v2.0TLC) 0x0052
	 */
	byte _levelFlagRouteBigBadHungry = 0;
	/**
	 * (v1.0BR) 0x0051
	 * (v1.1US) 0x0051
	 * (v2.0TLC) 0x0053
	 */
	byte _levelFlagRouteMontDespair = 0;
	/**
	 * (v1.0BR) 0x0052
	 * (v1.1US) 0x0052
	 * (v2.0TLC) 0x0054
	 */
	byte _levelFlagLoWhosBayouHiDeepDarkForest = 0;
	/**
	 * (v1.0BR) 0x0053-0x0061
	 * (v1.1US) 0x0053-0x0061
	 * (v2.0TLC) 0x0055-0x0063
	 */
	byte _pageLevelFlags[15] = {
		0,
	};

	// Memorial Stone records: v1.0BR/v1.1US 0x0062-0x00C1; v2.0TLC 0x0064-0x00C3.
	/**
	 * (v1.0BR) 0x0062-0x0081
	 * (v1.1US) 0x0062-0x0081
	 * (v2.0TLC) 0x0064-0x0083
	 * Gregorian achievement year for each memorial slot.
	 * Active records use 1-9999; inactive slots store 0.
	 */
	uint16 _memorialYears[16] = {
		0,
	};
	/**
	 * (v1.0BR) 0x0082-0x0091
	 * (v1.1US) 0x0082-0x0091
	 * (v2.0TLC) 0x0084-0x0093
	 * Month of achievement, stored as 1-12 for active records.
	 * Inactive slots store 0.
	 */
	byte _memorialMonths[16] = {
		0,
	};
	/**
	 * (v1.0BR) 0x0092-0x00A1
	 * (v1.1US) 0x0092-0x00A1
	 * (v2.0TLC) 0x0094-0x00A3
	 * Day of achievement, stored as 1-31 and constrained by the month.
	 * Inactive slots store 0.
	 */
	byte _memorialDays[16] = {
		0,
	};
	/**
	 * (v1.0BR) 0x00A2-0x00B1
	 * (v1.1US) 0x00A2-0x00B1
	 * (v2.0TLC) 0x00A4-0x00B3
	 * One-based route number, 1-4, for an active memorial.
	 * A value of 0 marks the slot as inactive.
	 */
	byte _memorialRoutes[16] = {
		0,
	};
	/**
	 * (v1.0BR) 0x00B2-0x00C1
	 * (v1.1US) 0x00B2-0x00C1
	 * (v2.0TLC) 0x00B4-0x00C3
	 * One-based completed route level, 1-4, for an active memorial.
	 * Inactive slots store 0.
	 */
	byte _memorialLevels[16] = {
		0,
	};

	// Route levels: v1.0BR/v1.1US 0x00C2-0x00C9; v2.0TLC 0x00C4-0x00CB.
	/**
	 * (v1.0BR) 0x00C2-0x00C9
	 * (v1.1US) 0x00C2-0x00C9
	 * (v2.0TLC) 0x00C4-0x00CB
	 * 0 ~ 3 (Level 1 ~ 4)
	 */
	int16 _routeLevels[4] = {0, 0, 0, 0};
	/**
	 * (v1.0BR) 0x00CA
	 * (v1.1US) 0x00CA
	 * (v2.0TLC) not serialized
	 * Last interactive page before the current transition.
	 * Picker uses value 1 to distinguish a return from RodMap.
	 * TLC derives this compatibility value from @ref _tlcPreviousPage after loading.
	 * @remarks 0 ~ 18 (0: launch state, 1: RodMap, 3: Isle, 18: Maze)
	 */
	int16 _currentRoute = 0;
	/**
	 * (v1.0BR) 0x00CC
	 * (v1.1US) 0x00CC
	 * (v2.0TLC) 0x00CE (+0x0002)
	 * Current interactive page.
	 * @remarks 3 ~ 18 (3: ISLE, 4: BC1, 5: BC2, 6: TOWN, 18: MAZE)
	 */
	ZmbDestPageKind _currentPage = ZmbDestPageKind::kIsle_03;
	/**
	 * (v1.0BR) absent
	 * (v1.1US) absent
	 * (v2.0TLC) 0x00CC
	 * TLC/v2.0 previous interactive page; the current page is at 0x00CE.
	 */
	ZmbDestPageKind _v2PreviousPage = ZmbDestPageKind::kUnk_00;
	/** Return the current interactive page as a runtime page type. */
	ZoombiniPageType getCurrentPageType() const;
	/** Return whether the serialized current page can be restored. */
	bool hasLoadableCurrentPage() const { return isLoadablePage(_currentPage); }
	/** Return whether the TLC previous-page sentinel or page value is valid. */
	bool hasValidV2PreviousPage() const { return _v2PreviousPage == ZmbDestPageKind::kUnk_00 || isLoadablePage(_v2PreviousPage); }
	/** Return whether bounded scalar fields and generation bookkeeping are valid. */
	bool hasValidSerializedScalars() const;
	/** Normalize bounded scalar fields and generation bookkeeping for loading. */
	void recoverSerializedScalars();
	/** Return whether RodMap can convert persisted progress values to shape frames. */
	bool hasValidRodMapProgress() const;
	/** Normalize contradictory RodMap progress values to a bounded route level. */
	void recoverRodMapProgress();
	/** Set the current interactive page from a runtime page type. */
	void setCurrentPageType(ZoombiniPageType pageType);
	/** Set the completion bit for a route at its zero-based difficulty level. */
	void setRouteCompletionFlag(ZmbRouteId routeId, int16 routeLevel);

	/**
	 * (v1.0BR) 0x00CE
	 * (v1.1US) 0x00CE
	 * (v2.0TLC) 0x00D0 (+0x0002)
	 * Stored Zoombinis on Basecamp 1.
	 * The hair, eye, nose, and feet traits are zeroed in its 16 finished-game entries.
	 */
	ZmbStateStoredChunk _storedChunkBC1;

	/**
	 * (v1.0BR) 0x3688
	 * (v1.1US) 0x3688
	 * (v2.0TLC) 0x3B6C (+0x04E4)
	 * Stored Zoombinis on Basecamp 2. At some of its 16 finished-game entries,
	 * the hair/eye/nose/feet traits are zeroed.
	 */
	ZmbStateStoredChunk _storedChunkBC2;

	/**
	 * (v1.0BR) 0x6C42
	 * (v1.1US) 0x6C42
	 * (v2.0TLC) 0x7608 (+0x09C6)
	 * Stored Zoombinis on Town.
	 */
	ZmbStateStoredChunk _storedChunkTown;

	/**
	 * (v1.0BR) 0xA1FC
	 * (v1.1US) 0xA1FC
	 * (v2.0TLC) 0xB0A4 (+0x0EA8)
	 * Active Zoombini pack on the Isle.
	 */
	ZmbStateActivePack _zmbPackIsle = {};
	/**
	 * (v1.0BR) 0xA462
	 * (v1.1US) 0xA462
	 * (v2.0TLC) 0xB32A (+0x0EC8)
	 */
	ZmbStateActivePack _zmbPackBC1 = {};
	/**
	 * (v1.0BR) 0xA6C8
	 * (v1.1US) 0xA6C8
	 * (v2.0TLC) 0xB5B0 (+0x0EE8)
	 */
	ZmbStateActivePack _zmbPackBC2 = {};
	/**
	 * (v1.0BR) 0xA92E
	 * (v1.1US) 0xA92E
	 * (v2.0TLC) 0xB836 (+0x0F08)
	 */
	ZmbStateActivePack _zmbPackActive = {};
	/**
	 * (v1.0BR) 0xAB94-0xAE04
	 * (v1.1US) 0xAB94-0xAE04
	 * (v2.0TLC) 0xBABC-0xBD2C (+0x0F28)
	 * Zoombini twin-generation status.
	 */
	byte _twinGenStatus[625] = {
		0,
	};
	/**
	 * (v1.0BR) absent
	 * (v1.1US) absent
	 * (v2.0TLC) 0xBD2D
	 * TLC-only byte after @ref ZmbStateFile::_twinGenStatus.
	 */
	byte _v2TwinGenStatusPad = 0;

	/**
	 * (v1.0BR) absent
	 * (v1.1US) 0xAE05-0xAE0C
	 * (v2.0TLC small 48430) absent
	 * (v2.0TLC current 48440) 0xBD2E-0xBD35 (+0x0F29)
	 * Per-route perfect completion counters (4 x int16).
	 * Stores one counter for each route in array order.
	 * Increments when a route is completed with all Snoids surviving.
	 * When a counter reaches three, it resets and advances the corresponding @ref ZmbStateFile::_routeLevels entry.
	 * Added in newer save formats; zeroed when loading older shorter saves.
	 */
	int16 _routePerfectCounters[4] = {0, 0, 0, 0};
	/**
	 * (v1.0BR) absent
	 * (v1.1US) 0xAE0D
	 * (v2.0TLC small) absent
	 * (v2.0TLC) 0xBD36 (+0x0F29)
	 * Town Develop Level (0-6).
	 */
	int16 _townDevelopLevel = 0;

	/**
	 * EOF:
	 * (v1.0BR) 0xAE05
	 * (v1.1US) 0xAE0F
	 * (v2.0TLC small) 0xBD2E
	 * (v2.0TLC) 0xBD38 (+0x0F29)
	 */

	/** Serialize the complete game state using the selected release layout. */
	void sync(Common::Serializer &s, bool isTlcLayout, bool hasCompletionCounters);

	/**
	 * ScummVM-only runtime marker for serialized-state changes not reflected in a save file.
	 * Gameplay, new-game setup, post-load conversion, and debugger state writes can set it.
	 * Debugger-specific save confirmation is tracked separately by @ref ZoombiniGameState::_debugStateMutationFlag.
	 * This marker is never serialized.
	 */
	bool _isDirty = false;

private:
	/** Return whether a serialized page can be restored as an interactive page. */
	static bool isLoadablePage(ZmbDestPageKind page);
};

struct ZmbRosterEntry {
	/** 22 bytes plus a NUL terminator. */
	byte _saveName[23] = {
		0,
	};
	/** 8 bytes plus a NUL terminator. */
	byte _fileName[9] = {
		0,
	};

	/** Serialize the roster entry's fixed-width byte fields. */
	void sync(Common::Serializer &r);
	/** Decode the user-visible save name, or return empty when unterminated. */
	Common::U32String getSaveName(MohawkEngine_Zoombini *vm) const;
	/** Decode the user-visible save name through an explicitly selected code page. */
	Common::U32String getSaveName(Common::CodePage codePage) const;
	/** Decode the legacy state-file stem, or return empty when unterminated. */
	Common::U32String getFileName(MohawkEngine_Zoombini *vm) const;
	/** Return the bounded byte length of the save name, or the field capacity when unterminated. */
	uint16 getSaveNameLength() const;
	/** Return the validated ASCII ZOOM#### state-file stem. */
	Common::String getSaveFileStem() const;
	/** Return the four ASCII decimal digits from a valid ZOOM#### state-file stem. */
	Common::String getSaveFileNumStr() const;
	/** Return whether both fixed-width fields are structurally valid. */
	bool isSerializedStateValid() const;
	/** Check whether a save name round-trips through the code page without substitution. */
	static bool isSaveNameEncodingValid(const Common::U32String &uSaveName, Common::CodePage codePage);
	/** Encode a save name without substitution and check the 22-byte name limit. */
	static bool encodeSaveName(const Common::U32String &uSaveName, Common::CodePage codePage, Common::String &encodedSaveName);
	/** Check if the given save name can be encoded and fit in the save name field. */
	static bool checkSaveNameSize(MohawkEngine_Zoombini *vm, const Common::U32String &uSaveName);

private:
	/** Return the bounded state-file stem length, or the field size when unterminated. */
	uint16 getFileNameLength() const;
};

struct ZmbRosterFile {
	/** Construct an empty roster with the current roster magic. */
	ZmbRosterFile();

	/** Endian-aware roster magic value. */
	uint16 _magic006B;
	/** Next monotonic ZOOM#### filename counter. */
	uint16 _nextSaveFileNameCounter = 0;
	/** Number of valid save entries. */
private:
	friend class ZoombiniGameState;
	/** Number of valid entries in @ref _entries. */
	uint16 _saveEntryCount = 0;
	/** Fifty entries fixed by the 1606-byte file and 32-byte entry layout. */
	ZmbRosterEntry _entries[50] = {};

public:
	/** Serialized roster header size before the fixed-width entries. */
	static constexpr uint32 kSerializedHeaderSize = 6;
	/** Serialized size of one 23-byte name and 9-byte filename entry. */
	static constexpr uint32 kSerializedEntrySize = 32;
	/** Return how many complete entries fit in @p fileSize and in the model. */
	uint16 getEntryCapacityForFileSize(uint32 fileSize) const {
		if (fileSize < kSerializedHeaderSize)
			return 0;
		const uint32 serializedCapacity = (fileSize - kSerializedHeaderSize) / kSerializedEntrySize;
		return static_cast<uint16>(MIN<uint32>(serializedCapacity, getEntryCapacity()));
	}
	/** Return whether the serialized entry count fits the exact roster file. */
	bool hasValidEntryCount() const;
	/** Return whether the next filename counter is reachable by the roster writer. */
	bool hasValidNextSaveFileNameCounter() const { return _nextSaveFileNameCounter <= 10000; }
	/** Return the entry count clamped to the entries fitting the exact roster size. */
	uint16 getEntryCount() const;
	uint16 getEntryCapacity() const { return static_cast<uint16>(ARRAYSIZE(_entries)); }
	/** Return whether an index addresses a serialized roster entry safely. */
	bool hasEntry(int index) const { return hasValidEntryCount() && 0 <= index && index < getEntryCount(); }
	/** Return whether the header and every used entry are structurally valid. */
	bool isSerializedStateValid() const;
	ZmbRosterEntry &getEntry(int16 index) { return _entries[index]; }
	const ZmbRosterEntry &getEntry(int16 index) const { return _entries[index]; }
	/** Swap two valid roster entries without changing the entry count. */
	bool swapEntries(int16 firstIndex, int16 secondIndex) {
		if (!hasEntry(firstIndex) || !hasEntry(secondIndex) || firstIndex == secondIndex)
			return false;

		const ZmbRosterEntry entry = _entries[firstIndex];
		_entries[firstIndex] = _entries[secondIndex];
		_entries[secondIndex] = entry;
		return true;
	}

	/** Assign the next monotonic ZOOM#### stem and append an entry at the roster tail. */
	bool appendNewSaveEntry(ZmbRosterEntry entry);

	bool removeEntryAt(int16 index) {
		if (!hasEntry(index))
			return false;

		for (int16 i = index; i + 1 < _saveEntryCount; i++)
			_entries[i] = _entries[i + 1];
		_entries[_saveEntryCount - 1] = ZmbRosterEntry();
		_saveEntryCount -= 1;
		return true;
	}

	void clearEntries() { _saveEntryCount = 0; }

	/** Serialize the roster header and valid entries. */
	void sync(Common::Serializer &r);
};

struct ZoombiniSaveSummary;

/**
 * Transfers Zoombinis saves between a ScummVM target's save namespace and an installed or DOSBox game folder
 * using raw ZOOMBINI.WHO and ZOOM####.TXT files.
 *
 * Each entry point is a self-contained, modal UI flow: it prompts for a folder,
 * copies the roster-referenced saves, and reports the result.
 */
class ZoombiniSaveTransfer {
public:
	/**
	 * Import the installed game's ZOOMBINI.WHO roster and ZOOM####.TXT saves from a chosen folder
	 * into the given ScummVM target's save namespace.
	 */
	static void importFromOriginalFolder(const Common::String &target);

	/** Export the given ScummVM target's saves into a chosen folder in raw game format. */
	static void exportToOriginalFolder(const Common::String &target);

	/** Import one raw .TXT or ScummVM .ZMB state file into the target. */
	static bool importOneSave(const Common::String &target);

	/** Export one target save slot as an uncompressed original-engine .TXT file. */
	static void exportOneSave(const Common::String &target, int slot);

private:
	/** Validate an uncompressed state-file size for all supported variants. */
	static bool zmbIsValidStateSize(uint32 size);
	/** Read a bounded stream into @p out. */
	static bool zmbReadAll(Common::SeekableReadStream *stream, Common::Array<byte> &out, uint32 maximumSize);
	/** Find a named child under a folder without changing the filesystem. */
	static Common::FSNode zmbFindChild(const Common::FSNode &dir, const Common::String &name);
	/** Return whether a target save file exists. */
	static bool zmbSaveFileExists(Common::SaveFileManager *saveFileMan, const Common::String &name);
	/** Write raw bytes to a ScummVM save file. */
	static bool zmbWriteToSaveFile(Common::SaveFileManager *saveFileMan, const Common::String &name, const Common::Array<byte> &bytes);
	/** Write raw bytes to a file in an original-game folder. */
	static bool zmbWriteToFolder(const Common::FSNode &node, const Common::Array<byte> &bytes);
};

class ZoombiniGameState {
public:
	/**
	 * On-disk game-state layouts, identified by exact file length.
	 *
	 * Zoombini state files have no explicit format tag. The original engines
	 * identify a compatible layout by its exact byte length. The Europe v1.1
	 * executable retains the Europe v1.0 layout, so kEuV10 covers both of those
	 * releases. kUsV11 identifies the later 44559-byte v1.1-family layout,
	 * including the Korean release.
	 */
	enum class ZmbSaveFormat {
		/** Unknown size; do not load. */
		kInvalid = 0,
		/** 44549-byte Europe v1.0/v1.1 layout. */
		kEuV10,
		/** 44559-byte US v1.1 layout, also used by the Korean release. */
		kUsV11,
		/**
		 * 48430-byte small TLC v2.0 layout.
		 *
		 * The Europe v1.0/v1.1 files are 44549 bytes and the US v1.1/Korean files
		 * are 44559 bytes, so this is already a TLC layout rather than an EU v1
		 * layout with a renamed version. The original TLC v2.0 reader accepts it
		 * by zero-filling the missing 10-byte tail (four route-perfect counters and
		 * Town Develop Level), then always writes the 48440-byte layout. No evidence
		 * establishes a direct EU v1.0-to-TLC conversion or a separately shipped
		 * retail producer for this shorter file. It is not dead in ScummVM because
		 * the compatibility loader still accepts it.
		 */
		kTlcV20Small,
		/** 48440-byte current TLC v2.0 layout. */
		kTlcV20
	};

	/** Result of compacting roster file stems and the next filename counter. */
	enum class ZmbSaveCompactResult {
		kSuccess,
		kNoChanges,
		kInvalidRoster,
		kMissingSaveFile,
		kFileNameConflict,
		kFileMoveFailed,
		kRosterWriteFailed,
		kRollbackFailed
	};

	/** Result of recovering one corrupted state file. */
	enum class ZmbSaveRecoverResult {
		kSuccess,
		kNotRecoverable,
		kTemporaryWriteFailed,
		kBackupFailed,
		kReplaceFailed,
		kRollbackFailed
	};

	/** Structural problems that can require rebuilding ZOOMBINI.WHO. */
	enum ZmbRosterIssueFlags : uint32 {
		kRosterIssueNone = 0,
		kRosterIssueMissing = 1 << 0,
		kRosterIssueSize = 1 << 1,
		kRosterIssueMagic = 1 << 2,
		kRosterIssueEntryCount = 1 << 3,
		kRosterIssueNextCounter = 1 << 4,
		kRosterIssueSaveName = 1 << 5,
		kRosterIssueFileStem = 1 << 6,
		kRosterIssueMissingState = 1 << 7,
		kRosterIssueOrphanState = 1 << 8,
		kRosterIssueRead = 1 << 9,
		kRosterIssueDuplicateFileStem = 1 << 10
	};

	/** Result of rebuilding a corrupted ZOOMBINI.WHO roster. */
	enum class ZmbRosterRecoverResult {
		kSuccess,
		kNotNeeded,
		kBackupFailed,
		kWriteFailed,
		kRollbackFailed
	};

	/** Primary structural problem detected while validating one state file. */
	enum class ZmbSaveIssue {
		kNone,
		kMagic,
		kPage,
		kScalarFields,
		kBasecamp1Storage,
		kBasecamp2Storage,
		kTownStorage,
		kIslePack,
		kBasecamp1Pack,
		kBasecamp2Pack,
		kActivePack,
		kPuzzlePack,
		kProgressFields,
		kRouteLevel,
		kRodMapProgress,
		kPerfectCounter,
		kLogicalLocationCount,
		kIsleActiveCount,
		kPhysicalLocationCount
	};

	static constexpr uint16 kEndianMagic = 0x006B; // 6B 00 in LE, 00 6B in BE.
	/** Raw little-endian bytes for the roster magic. */
	static constexpr byte kLittleEndianMagicBytes[2]{
		0x6B,
		0x00,
	};
	/** Raw big-endian bytes for the roster magic. */
	static constexpr byte kBigEndianMagicBytes[2]{
		0x00,
		0x6B,
	};

	/** Exact Bridge v1.0/European v1.1 state-file size. */
	static constexpr int32 kStateFileSizeBrV10 = 44549;
	/** Exact US v1.1 state-file size. */
	static constexpr int32 kStateFileSizeUsV11 = 44559;
	/** Small TLC v2.0 state-file size without the final five-word completion tail. */
	static constexpr int32 kStateFileSizeTlcV20Small = 48430;
	/** Exact TLC v2.0 state-file size. */
	static constexpr int32 kStateFileSizeTlcV20 = 48440;
	/** Exact uncompressed ZOOMBINI.WHO size in every release family. */
	static constexpr uint32 kZmbRosterFileSize = 1606;
	/** Return the compact display identifier for a serialized save format. */
	static const char *getSaveFormatName(ZmbSaveFormat format);

	/**
	 * Runtime-only ambient scheduler state shared by every interactive page instance.
	 * The deadline remains page-local because every page switch resets it, while the
	 * last sound, release cadence, and per-page non-repeat pools survive page replacement.
	 * This state is not serialized and remains stable for the lifetime of one engine.
	 */
	struct AmbientSoundState {
		/** Number of page identifiers that can own an authored ambient pool. */
		static constexpr uint16 kPoolStateCount = 19;
		/** Resource ID of the most recently selected system ambient sound. */
		int16 _lastSoundId = 0;
		/** Retained modulo-16 ambient resource-release cadence. */
		uint16 _preloadCounter = 0;
		/** Non-repeat random-pool state indexed by @ref ZoombiniPageType. */
		uint32 _poolStates[kPoolStateCount] = {};

		/** Return the retained non-repeat pool for @p pageType. */
		uint32 &getPoolState(ZoombiniPageType pageType) {
			const uint16 pageIndex = static_cast<uint16>(pageType);
			assert(pageIndex < kPoolStateCount);
			return _poolStates[pageIndex];
		}
	};

	/**
	 * Runtime-only celebration scheduling shared by every Slides page instance.
	 *
	 * The state belongs to one engine session rather than a class static so
	 * parallel ScummVM engine instances never share a scheduler.
	 * It is not serialized and survives new-game and save-load operations.
	 */
	struct SlidesCelebrationState {
		/** Number of successful celebration starts in the current cycle. */
		int16 _visitCount = 0;
		/** Non-repeat random-pool state for selecting celebration Snoids. */
		uint32 _poolState = 0;
		/** Frame counter at which the most recent celebration started. */
		uint32 _lastFrame = 0;
	};

	/** Net selection history retained across page visits, new games, and save loads, without serialization. */
	struct NetCelebrationHistory {
		/** Non-repeat pool for celebration candidates, including skipped Snoids. */
		uint32 _poolState = 0;
		/** Frame of the most recent selection attempt, whether or not it succeeded. */
		uint32 _lastFrame = 0;
	};

	/** Maze selection history retained across page visits, new games, and save loads, without serialization. */
	struct MazeCelebrationHistory {
		/** Non-repeat pool for celebration candidates, including skipped Snoids. */
		uint32 _poolState = 0;
		/** Frame of the most recent selection attempt, whether or not it succeeded. */
		uint32 _lastFrame = 0;
	};

	/**
	 * Runtime-only Ferry selector state shared by every Ferry page instance.
	 *
	 * Most selectors remain stable for the lifetime of one engine. New Game resets
	 * only the visit count and reject-destination pool through @ref resetForNewGame().
	 * None of these fields is serialized.
	 */
	struct FerryRuntimeState {
		/** Number of Ferry page opens in this engine session. */
		uint16 _visitCount = 0;
		/** Non-repeat pool for the subsequent-visit raft SCRBs 1800-1803. */
		uint32 _raftPoolState = 0;
		/** Non-repeat pool for the idle Captain SCRBs 1823-1827. */
		uint32 _idleFidgetPoolState = 0;
		/** Non-repeat pool for the good-reaction SCRBs 1817-1818. */
		uint32 _goodReactionPoolState = 0;
		/** Non-repeat pool for the bad-reaction SCRBs 1804-1814. */
		uint32 _badReactionPoolState = 0;
		/** Non-repeat pool for the post-flight Captain SCRBs 1828-1832. */
		uint32 _postFlightFidgetPoolState = 0;
		/** Non-repeat pool for the move-reaction SCRBs 1820-1822. */
		uint32 _moveReactionPoolState = 0;
		/** Non-repeat pool for reject destinations 0-9. */
		uint32 _rejectDestinationPoolState = 0;

		/** Reset the two Ferry selectors owned by clean game-state initialization. */
		void resetForNewGame() {
			_visitCount = 0;
			_rejectDestinationPoolState = 0;
		}
	};

	/**
	 * Runtime-only Maze layout selectors shared by every Maze page instance.
	 * They are not serialized and remain stable for the lifetime of one engine.
	 * The initial values are latched before the first Maze layout is generated.
	 */
	struct MazeLayoutVariantState {
		/** Next layout variant for level 1. */
		int16 _level1 = 0;
		/** Next layout variant for level 2. */
		int16 _level2 = 0;
		/** Next layout variant for level 3. */
		int16 _level3 = 0;
		/** Next layout variant for level 4. */
		int16 _level4 = 0;
		/** Prevent later page loads and option changes from replacing the first selection. */
		bool _initialVariantsSelected = false;
	};

	/**
	 * Runtime-only Tunnels entry-actor selector shared by every Tunnels page instance.
	 * It is not serialized and remains stable for the lifetime of one engine.
	 */
	struct TunnelsEntryActorState {
		/** Number of Tunnels page opens in this engine session. */
		uint16 _pageOpenCount = 0;
	};

	/** Construct game-state storage attached to the engine save manager. */
	ZoombiniGameState(MohawkEngine_Zoombini *vm, Common::SaveFileManager *saveFileMan);
	/** Release roster, save, and practice-state storage. */
	~ZoombiniGameState();

	/** Decode an exact-size ZOOMBINI.WHO image after checking its magic; callers must validate its fields. */
	static bool parseRoster(const Common::Array<byte> &bytes, ZmbRosterFile &roster);
	/** Decode an exact-size roster stream after checking its magic; callers must validate its fields. */
	static bool parseRoster(Common::SeekableReadStream *stream, ZmbRosterFile &roster);
	/** Load and decode one roster file without trusting its entry fields. */
	static bool loadRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, ZmbRosterFile &roster);
	/** Serialize and write one roster file. */
	static bool saveRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster);

	/** Load the target's roster file into memory. */
	void loadRoster();
	/** Persist the in-memory roster file. */
	bool saveRoster();
	/** Load one game state by roster slot. */
	bool loadGame(int slot);
	/** Save the current game state to an existing slot. */
	bool saveGame(int slot);
	/**
	 * Serialize the live in-memory game state into a stream using the
	 * active release's target layout.
	 *
	 * Unlike @ref saveGame, this serializes the current model directly:
	 * it does not run page-side pack hooks and it leaves runtime markers
	 * such as the dirty flag untouched.
	 */
	void dumpCurrentState(Common::WriteStream &out);
	/** Create a new save slot and persist the current game state. */
	bool saveNewGame(const Common::U32String &saveName);
	/** Rename an existing save slot without changing its state-file stem. */
	bool renameGame(int slot, const Common::U32String &saveName);
	/** Move one save entry to an adjacent roster slot. */
	bool moveGame(int slot, int destinationSlot);
	/** Renumber all state-file stems in roster order and reset the next counter. */
	ZmbSaveCompactResult compactSaveFiles();
	/** Return whether the most recent load operation was cancelled by the user. */
	bool wasLastLoadCancelled() const { return _lastLoadCancelledFlag; }
	/** Mark the game state changed by a debugger command. */
	void markDebugStateMutation();
	/** Clear the debugger-specific save-confirmation flag for the current state. */
	void clearDebugStateMutation() { _debugStateMutationFlag = false; }
	/** Mark a debugger-created state that cannot be represented safely in a save file. */
	void markUnsafeSyntheticDebugState();
	/** Return whether saving a debugger-modified state requires confirmation. */
	bool requiresSaveConfirmationForDebugStateMutation() const { return _debugStateMutationFlag; }
	/** Return whether saving is blocked by a debugger-created synthetic state. */
	bool hasUnsafeSyntheticDebugState() const { return _debugUnsafeSyntheticStateFlag; }
	/** Delete one roster entry, shift later entries, and remove its file. */
	bool deleteGameAndShiftRoster(int slot);

	/** List the roster entries and summarize each referenced state file. */
	static Common::Array<ZoombiniSaveSummary> listSaveSummaries(const Common::String &target);

	/**
	 * Delete one roster entry and its state file without requiring a running
	 * Zoombini engine.
	 */
	static bool deleteSaveForTarget(const Common::String &target, int slot);
	/** Rename one roster entry without requiring a running Zoombini engine. */
	static bool renameSaveForTarget(const Common::String &target, int slot, const Common::U32String &saveName);
	/** Move one roster entry to an adjacent slot without requiring a running engine. */
	static bool moveSaveForTarget(const Common::String &target, int slot, int destinationSlot);
	/** Recover one corrupted state file without requiring a running engine. */
	static ZmbSaveRecoverResult recoverSaveForTarget(const Common::String &target, int slot);
	/** Compact one target's state-file stems without requiring a running engine. */
	static ZmbSaveCompactResult compactSaveFilesForTarget(const Common::String &target);
	/** Return whether Slot Compact is available and would change one target. */
	static bool isSaveCompactionNeededForTarget(const Common::String &target);
	/** Return the detected structural problems in one target's ZOOMBINI.WHO. */
	static uint32 getRosterIssueFlagsForTarget(const Common::String &target);
	/** Rebuild one target's roster from bounded metadata and existing state files. */
	static ZmbRosterRecoverResult recoverRosterForTarget(const Common::String &target);
	/** Append an entry with a roster- and disk-unique ZOOM#### state-file stem. */
	static bool appendNewSaveEntryForTarget(Common::SaveFileManager *saveFileMan,
											const Common::String &target, ZmbRosterFile &roster,
											ZmbRosterEntry entry);
	/** Remove leading and trailing ASCII spaces from a save name. */
	static Common::U32String trimSaveName(const Common::U32String &name);
	/** Return the executable code page used for save names in a target. */
	static Common::CodePage getSaveNameCodePage(const Common::String &target);
	/** Encode and zero-fill a roster save-name field. */
	static bool encodeRosterSaveName(const Common::U32String &saveName, Common::CodePage codePage, byte *output, uint32 outputSize);
	/** Rename one entry in a decoded roster. */
	static bool renameRosterEntry(const Common::String &target, ZmbRosterFile &roster, int slot, const Common::U32String &saveName);

	/**
	 * Build the per-target save filename "{targetId}-####.ZMB"
	 * from the legacy roster base name stored in @ref ZmbRosterEntry::_fileName.
	 */
	static Common::String makeSaveFilename(const Common::String &targetName, const byte *baseName);

	/**
	 * Scan all living Zoombini entries in active packs and stored chunks.
	 * Mark their names in @ref ZoombiniGameState::_zoombiniNameGeneratedTable.
	 * This prevents new names from duplicating names assigned to existing Zoombinis.
	 */
	void buildNameGeneratedTable();

	/** Return whether serialized-state changes are not reflected in a save file. */
	bool isStateDirty() const { return _f._isDirty; }
	/** Return whether quitting should currently offer a save. */
	bool needsSaveBeforeQuit() const { return _saveBeforeQuitPending; }
	/** Mark the current state for a save-before-quit prompt. */
	void markSaveBeforeQuitPending() { _saveBeforeQuitPending = true; }
	/** Clear the save-before-quit prompt request. */
	void clearSaveBeforeQuitPending() { _saveBeforeQuitPending = false; }
	/** Suppress the next save-before-quit mark caused by loading a game. */
	void suppressSaveBeforeQuitMarkForLoad() { _suppressSaveBeforeQuitMarkForLoad = true; }
	/** Consume and clear the load-induced save-prompt suppression flag. */
	bool consumeSaveBeforeQuitMarkSuppression() {
		const bool suppress = _suppressSaveBeforeQuitMarkForLoad;
		_suppressSaveBeforeQuitMarkForLoad = false;
		return suppress;
	}
	/** Return whether the active game state has completed its initial setup. */
	bool isGameStateReady() const { return _gameStateReadyFlag; }
	/** Mark the active game state ready for normal page transitions. */
	void markGameStateReady() { _gameStateReadyFlag = true; }
	/** Return true once, then clear the first-launch marker. */
	bool isFirstLaunch() {
		bool ret = _isFirstLaunch;
		_isFirstLaunch = false;
		return ret;
	}
	/** Return the runtime state shared by all Slides page instances. */
	SlidesCelebrationState &getSlidesCelebrationState() { return _slidesCelebrationState; }
	/** Return the retained Net celebration selection history for this engine session. */
	NetCelebrationHistory &getNetCelebrationHistory() { return _netCelebrationHistory; }
	/** Return the retained Maze celebration selection history for this engine session. */
	MazeCelebrationHistory &getMazeCelebrationHistory() { return _mazeCelebrationHistory; }
	/** Return the runtime selector state shared by all Ferry page instances. */
	FerryRuntimeState &getFerryRuntimeState() { return _ferryRuntimeState; }
	/** Return the runtime ambient scheduler state shared by all interactive pages. */
	AmbientSoundState &getAmbientSoundState() { return _ambientSoundState; }
	/** Return the runtime layout selector shared by all Maze page instances. */
	MazeLayoutVariantState &getMazeLayoutVariantState() { return _mazeLayoutVariantState; }
	/** Return the runtime entry-actor selector shared by all Tunnels page instances. */
	TunnelsEntryActorState &getTunnelsEntryActorState() { return _tunnelsEntryActorState; }

	/**
	 * Selects a transient SFX group from the route level and page flag.
	 * As a side effect, updates a page's visit count and trigger flags.
	 * @param pageFlag The state variable to update and read from.
	 * @return A value from @ref ZmbSfxGroupFlags.
	 */
	ZmbSfxGroupFlags getSfxGroupFlagsFromPageFlag(ZmbStateFile::PageFlag &pageFlag);
	/**
	 * Read the saturating visit count packed into a page flag.
	 *
	 * This does not modify @p pageFlag or interpret either SFX trigger bit.
	 */
	static uint16 getPageVisitCountFromPageFlag(const ZmbStateFile::PageFlag &pageFlag) {
		return pageFlag.getVisitCount();
	}
	/**
	 * Selects a transient SFX group for an explicit page's route.
	 * @param pageFlag The state variable to update and read from.
	 * @param pageType The page whose route level controls the selection.
	 * @return A value from @ref ZmbSfxGroupFlags.
	 */
	ZmbSfxGroupFlags getSfxGroupFlagsFromPageFlag(ZmbStateFile::PageFlag &pageFlag, ZoombiniPageType pageType);

	/**
	 * Convenience method that retrieves a page's flag and updates it via
	 * @ref getSfxGroupFlagsFromPageFlag to return the page's transient SFX group.
	 * @param pageType The target page identifier.
	 * @return A value from @ref ZmbSfxGroupFlags.
	 */
	ZmbSfxGroupFlags getSfxGroupFlagsFromPageType(ZoombiniPageType pageType);

	/**
	 * Reads the current progress level (0-3) of the active route.
	 * @return The current route level, 0-based
	 */
	int16 readActivePageRouteLevel();

	/**
	 * Reads the progress level (0-3) for a specific page's route.
	 * @param pageType The page whose route level should be queried.
	 * @return The page's route level, 0-based.
	 */
	int16 readPageRouteLevel(ZoombiniPageType pageType);

	/**
	 * Maps the serialized current page to its owning route.
	 * Puzzle pages 7-18 map in groups of three: pages 7-9 use route 0,
	 * 10-12 route 1, 13-15 route 2, and 16-18 route 3.
	 * @return The owning route for the current page, or -1 when the current
	 *         page is not a puzzle page (DI 7-18).
	 */
	int16 readActivePageRouteId();

	/**
	 * Reads the progress level for a specific route.
	 * @param routeId The ID of the route to query.
	 * @return The stored route level.
	 */
	int16 readRouteLevel(ZmbRouteId routeId);

	/**
	 * Advance one route level using the same state side effects as normal gameplay.
	 *
	 * The persisted route perfect counter is cleared in releases that store it,
	 * and the transient departure marker is set when the level actually advances.
	 * The marker is consumed by the normal departure path when it records the
	 * completed level.
	 * @param routeId The zero-based route to advance.
	 * @return True when the route advanced, otherwise false at level 4 or for an invalid route.
	 */
	bool advanceRouteLevel(ZmbRouteId routeId);

	/**
	 * Set a route's player-facing difficulty level for debugger state control.
	 *
	 * Increasing the level uses @ref advanceRouteLevel() for each step so the
	 * normal advancement cleanup is retained. A level change resets the
	 * persisted perfect counter because that progress belongs to the old level.
	 * Route-path completion and perfect-clear flags are rebuilt so every lower
	 * difficulty is complete and the selected difficulty has not been played.
	 * When lowering, memorials at or above the target level are removed. Missing
	 * prerequisite memorials below the target are added with the current date.
	 * A debugger level change is not a puzzle departure, so its one-shot
	 * departure marker is cleared before returning.
	 * @param routeId The zero-based route to update.
	 * @param difficultyLevel The player-facing difficulty level, from 1 through 4.
	 * @return True when the arguments are valid, otherwise false.
	 */
	bool setRouteDifficultyLevel(ZmbRouteId routeId, int16 difficultyLevel);

	/**
	 * Record a first-clear memorial for a completed route level.
	 *
	 * Existing records for the same route and level are not duplicated. Normal
	 * route completion calls this before route advancement, so the level is the
	 * player-facing level that was just completed.
	 * @param routeId The zero-based route that was completed.
	 * @param difficultyLevel The player-facing level that was completed, from 1 through 4.
	 * @return True when a new memorial was stored, otherwise false.
	 */
	bool recordFirstClearMemorial(ZmbRouteId routeId, int16 difficultyLevel);

	/**
	 * Return whether a first-clear memorial is active for a route and level.
	 * @param routeId The zero-based route to query.
	 * @param difficultyLevel The one-based completed level, from 1 through 4.
	 * @return True when the matching memorial slot is active.
	 */
	bool readMemorialActive(ZmbRouteId routeId, int16 difficultyLevel) const;

	/**
	 * Read the date of an active first-clear memorial.
	 * @param routeId The zero-based route to query.
	 * @param difficultyLevel The one-based completed level, from 1 through 4.
	 * @param year Receives the Gregorian year, from 1 through 9999.
	 * @param month Receives the month, from 1 through 12.
	 * @param day Receives the valid day for the selected month.
	 * @return True when the matching memorial slot is active.
	 */
	bool readMemorialDate(ZmbRouteId routeId, int16 difficultyLevel, uint16 &year, byte &month, byte &day) const;

	/**
	 * Activate or deactivate a first-clear memorial for a route and level.
	 * Activating a missing record creates it with the command-time date.
	 * Deactivating a record clears all serialized fields in its slot.
	 * @param routeId The zero-based route to update.
	 * @param difficultyLevel The one-based completed level, from 1 through 4.
	 * @param active True to create or retain the memorial, false to clear it.
	 * @return True when the arguments are valid and the requested state fits in the memorial slots.
	 */
	bool setMemorialActive(ZmbRouteId routeId, int16 difficultyLevel, bool active);

	/**
	 * Set the date of a first-clear memorial.
	 * A missing record is created and activated with the supplied date.
	 * @param routeId The zero-based route to update.
	 * @param difficultyLevel The one-based completed level, from 1 through 4.
	 * @param year The Gregorian year, from 1 through 9999.
	 * @param month The month, from 1 through 12.
	 * @param day The valid day for the selected month.
	 * @return True when the date and target memorial are valid.
	 */
	bool setMemorialDate(ZmbRouteId routeId, int16 difficultyLevel, uint16 year, byte month, byte day);

	/**
	 * Calculates the sequence index of the current page within its active route.
	 * @return The zero-based index of the page in the route progression.
	 */
	uint16 getPageIdxInRoute();

	/**
	 * Determines if the next logical destination in the game flow is a container
	 * page (e.g., Basecamp 1, Basecamp 2, or Zoombiniville).
	 * @return True if the next page is a container; otherwise false.
	 */
	bool isNextPageContainer();

	/**
	 * Remember an XFER source when its route arrives at a container page.
	 * The Basecamp and Town arrival handlers consume the transient marker.
	 */
	void markXferContainerArrival(ZmbSrcPageKind sourcePage, const ZmbXferRouteInfo &routeInfo);

	/**
	 * Runtime (non-persisted): set to the puzzle page DI when a puzzle that leads
	 * to a container page (BC1, BC2, Town) completes successfully.
	 * Read and cleared during BC1, BC2, and Town page initialization.
	 */
	uint16 _lastPageBeforeContainer = 0;
	bool inPracticeMode() { return _practiceLevel != 0; }

	/**
	 * "Perfect streak" flag.
	 * Set to true when entering the first puzzle of a route (Bridge/Ferry/Fleens/Caves).
	 * Cleared to false when any snoid is lost (non-occupied) at a puzzle.
	 * Checked at container puzzles for route level advancement.
	 * Runtime only; not serialized.
	 */
	bool _perfectStreakFlag = false;

	/**
	 * Set when a route level advances.
	 * Used by route completion flag setting to record the PREVIOUS level's
	 * completion rather than the newly advanced level.
	 * Once set, the stored route level is one higher than the level just
	 * completed. The departure completion-flag path subtracts one while this
	 * marker is set, so it records the completed level instead of the newly
	 * entered level.
	 * Runtime only, reset each departure cycle.
	 */
	bool _routeLevelJustAdvanced = false;

	/**
	 * Generate random snoids in the active pack.
	 * Used for practice mode and debug jump commands.
	 * @param count Number of Snoids to place in the active pack, from 1 through 16.
	 * @return Number of active-pack slots that were empty before the debug fill.
	 */
	int16 generateRandomPack(int16 count = 16);
	/**
	 * Generate a debug pack using the same availability rules as Picker.
	 *
	 * When the Isle still has a partial pack, the generated count is limited to
	 * the number that can be removed from that pack. This keeps a debug-created
	 * pack from increasing the live Zoombini population unexpectedly.
	 * @return Number of Snoids generated for the debug pack.
	 */
	int16 generatePickerRandomPack();
	/** Remove debug-generated Snoids from the Isle after a temporary pack fill. */
	int16 subtractDebugGeneratedSnoidsFromIsle(int16 generatedCount);

	/** Return the active roster entry, if a saved slot is selected. */
	ZmbRosterEntry *getActiveSaveRosterEntry();
	/** Return the active save's localized display name. */
	Common::U32String getActiveSaveName();
	/** Return the currently selected save slot, or @ref kUnsavedNewGame. */
	int getActiveSaveSlot() { return _currentSaveSlot; }
	/** Find a roster slot by localized save name. */
	int searchSaveSlotByName(const Common::U32String &saveName);
	/** Return the first available roster slot. */
	int getAvailableSaveSlot();

	// Options
	/** Reset to a new-game baseline, optionally running the shortcut-only save preflight. */
	void startNewGame(bool askSaveCurrentGame);
	/** Return whether sound effects are enabled. */
	bool getEnableSound() { return _f.getSfxEnabled(); }
	/** Return whether background music is enabled. */
	bool getEnableMusic() { return _f.getBgmEnabled(); }
	/** Return whether sticky-mouse behavior is enabled. */
	bool getEnableStickyMouse() { return _f.getStickyMouseEnabled(); }
	/** Return whether automatic sticky-mouse activation is enabled. */
	bool getEnableAutoStickyMouse() { return _f.getAutoStickyMouseEnabled(); }
	/** Return the delay before automatic sticky-mouse activation. */
	uint16 getAutoStickyThreshold() { return _f._autoStickyDelay; }
	/** Return whether transition scenes are enabled. */
	bool getEnableTransitions();
	/** Return whether touch-sense behavior is enabled. */
	bool getEnableTouchSense();
	/** Return whether help audio is enabled. */
	bool getEnableHelpAudio();
	/** Return whether reduced-action mode is enabled. */
	bool isLessActionEnabled() { return _f.getLessActionEnabled(); }
	/** Return whether the in-game cursor is visible. */
	bool isCursorVisible() { return _flagCursorVisible; }
	/** Set the global sound-enabled option and optionally show its page notification. */
	void setEnableSound(bool val, bool showNotification = true);
	/** Set the global music-enabled option and optionally show its page notification. */
	void setEnableMusic(bool val, bool showNotification = true);
	/** Set the sticky-mouse option and optionally show its page notification. */
	void setEnableStickyMouse(bool val, bool showNotification = true);
	/** Set the automatic sticky-mouse option. */
	void setEnableAutoStickyMouse(bool val);
	/** Set the transition-effects option and optionally show its page notification. */
	void setEnableTransitions(bool val, bool showNotification = true);
	/** Set the touch-sense option and optionally show its page notification. */
	void setEnableTouchSense(bool val, bool showNotification = true);
	/** Set the help-audio option and optionally show its page notification. */
	void setEnableHelpAudio(bool val, bool showNotification = true);
	/** Set the reduced-action option. */
	void setLessActionEnabled(bool val);
	/** Set visibility of the in-game cursor. */
	void setCursorVisible(bool val);
	/** Toggle sound effects and return the new value. */
	bool toggleSound(bool showNotification = true) {
		setEnableSound(!getEnableSound(), showNotification);
		return getEnableSound();
	}
	/** Toggle background music and return the new value. */
	bool toggleMusic(bool showNotification = true) {
		setEnableMusic(!getEnableMusic(), showNotification);
		return getEnableMusic();
	}
	/** Toggle sticky-mouse behavior and return the new value. */
	bool toggleStickyMouse(bool showNotification = true) {
		setEnableStickyMouse(!getEnableStickyMouse(), showNotification);
		return getEnableStickyMouse();
	}
	/** Toggle automatic sticky-mouse behavior and return the new value. */
	bool toggleAutoStickyMouse() {
		setEnableAutoStickyMouse(!getEnableAutoStickyMouse());
		return getEnableAutoStickyMouse();
	}
	/** Toggle transition effects and return the new value. */
	bool toggleTransitions(bool showNotification = true) {
		setEnableTransitions(!getEnableTransitions(), showNotification);
		return getEnableTransitions();
	}
	/** Toggle touch-sense behavior and return the new value. */
	bool toggleTouchSense(bool showNotification = true) {
		setEnableTouchSense(!getEnableTouchSense(), showNotification);
		return getEnableTouchSense();
	}
	/** Toggle help audio and return the new value. */
	bool toggleHelpAudio(bool showNotification = true) {
		setEnableHelpAudio(!getEnableHelpAudio(), showNotification);
		return getEnableHelpAudio();
	}
	/** Toggle reduced-action mode and return the new value. */
	bool toggleLessMoreAction() {
		setLessActionEnabled(!isLessActionEnabled());
		return isLessActionEnabled();
	}
	/** Toggle cursor visibility and return the new value. */
	bool toggleCursorVisibility() {
		setCursorVisible(!isCursorVisible());
		return isCursorVisible();
	}

	/** Current serialized game state. */
	ZmbStateFile _f;
	/** Current serialized save roster. */
	ZmbRosterFile _r;
	/** Feature pointers retained while a page is active. */
	Common::Array<ZmbFeature *> _loadedZmbFeatures;

	/** Current practice difficulty, or zero outside practice mode. */
	uint16 _practiceLevel = 0;
	/** Used-name table for canonical Zoombini names. */
	byte _zoombiniNameGeneratedTable[625] = {
		0,
	};

	/**
	 * Practice-mode state snapshot.
	 * Practice launches capture the active game state in memory and restore it on RodMap re-entry.
	 * This prevents practice play from modifying the user's pack or progression flags.
	 *
	 * @ref ZoombiniGameState::_practiceStateBackupActive is set when the snapshot is captured and cleared on restore.
	 */
	ZmbStateFile _practiceStateBackup;
	/** Whether @ref _practiceStateBackup contains a snapshot awaiting restoration. */
	bool _practiceStateBackupActive = false;
	/** Runtime-only ambient scheduler state for this engine session. */
	AmbientSoundState _ambientSoundState;
	/** Runtime-only Slides celebration scheduler state for this engine session. */
	SlidesCelebrationState _slidesCelebrationState;
	/** Runtime-only Net celebration selection history for this engine session. */
	NetCelebrationHistory _netCelebrationHistory;
	/** Runtime-only Maze celebration selection history for this engine session. */
	MazeCelebrationHistory _mazeCelebrationHistory;
	/** Runtime-only Ferry selector state for this engine session. */
	FerryRuntimeState _ferryRuntimeState;
	/** Runtime-only Maze layout selector state for this engine session. */
	MazeLayoutVariantState _mazeLayoutVariantState;
	/** Runtime-only Tunnels entry-actor selector state for this engine session. */
	TunnelsEntryActorState _tunnelsEntryActorState;

	enum PredefinedSaveSlot {
		/** First user-selectable save slot. */
		kNormal = 0,
		/** Current game was created but has not been saved. */
		kUnsavedNewGame = -1,
	};
	/** Currently selected roster slot, or @ref kUnsavedNewGame for a new game. */
	int32 _currentSaveSlot = kUnsavedNewGame;

private:
	/** Return the save-manager error, or the operation-specific fallback. */
	static Common::Error getSaveFileWriteError(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode);
	/** Build the localized write-failure reason, including backend details when available. */
	static Common::U32String getSaveFileWriteFailureReason(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode);
	/** Report a state-file write failure to the player. */
	static void showStateFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode);
	/** Report a roster-file write failure to the player. */
	static void showRosterFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode);
	/** Write one roster while returning the operation-specific fallback error category. */
	static bool writeRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster,
								Common::ErrorCode *fallbackErrorCode);
	/** Mark one generated name as already used. */
	void markGeneratedName(const Common::U32String &name);
	/** Return whether any living Zoombini has a serialized non-ASCII name. */
	bool hasNonAsciiZoombiniName(const ZmbStateFile &state) const;
	/** Replace every living Zoombini name through the active release's standard name generator. */
	void replaceZoombiniNamesWithStandardNames(ZmbStateFile &state);
	/** Find the active memorial slot for a route and level, or return -1. */
	int16 findFirstClearMemorialSlot(ZmbRouteId routeId, int16 difficultyLevel) const;
	/** Return whether a Gregorian memorial date is within its valid calendar range. */
	static bool isValidMemorialDate(uint16 year, byte month, byte day);
	/** Add a memorial with an already captured date if the route/level is missing. */
	bool addFirstClearMemorial(ZmbRouteId routeId, int16 difficultyLevel, uint16 year, byte month, byte day);
	/** Clear all serialized fields belonging to one memorial slot. */
	void clearFirstClearMemorial(uint16 slotIdx);
	/** Make route-path completion flags consistent with a debugger-selected route level. */
	void syncRouteProgressFlags(ZmbRouteId routeId, int16 difficultyLevel);
	/** Make memorial records consistent with a debugger-selected route level. */
	void syncRouteFirstClearMemorials(ZmbRouteId routeId, int16 difficultyLevel, bool removeAtOrAboveTarget);
	/** Select a save format from an exact serialized file size. */
	static ZmbSaveFormat getSaveFormatFromSize(int32 size);
	/** Return whether a save format is a TLC layout. */
	static bool isTlcSaveFormat(ZmbSaveFormat format);
	/** Return whether a save format stores route completion counters. */
	static bool hasCompletionCounters(ZmbSaveFormat format);
	/** Select the write format for the active engine release. */
	static ZmbSaveFormat getTargetSaveFormat(const MohawkEngine_Zoombini *vm);
	/** Return whether the engine can load a source save format. */
	static bool canLoadSaveFormat(ZmbSaveFormat sourceFormat);
	/** Return the major version represented by a save format. */
	static int getSaveFormatMajorVersion(ZmbSaveFormat format);
	/** Return whether a source format is newer than the target format. */
	static bool isNewerThanTargetSaveFormat(ZmbSaveFormat sourceFormat, ZmbSaveFormat targetFormat);
	/** Initialize route completion counters for a newly created state. */
	static void initializeCompletionCounters(ZmbStateFile &state);
	/** Upgrade source-layout fields into the target release layout. */
	static void upgradeStateForVariant(ZmbStateFile &state, ZmbSaveFormat sourceFormat, ZmbSaveFormat targetFormat);
	/** Select the serialized layout read by a target for the available file length. */
	static ZmbSaveFormat getLoadSaveFormatFromSize(int32 size, ZmbSaveFormat targetFormat);
	/** Read and validate the big-endian magic at the stream's current position. */
	static bool hasValidStateFileMagic(Common::SeekableReadStream &stream);
	/** Count entries in the used prefix of a serialized active pack. */
	static int16 countSnoidsInPack(const ZmbStateActivePack &pack);
	/** Count populated trait records in a serialized storage chunk. */
	static int16 countStoredSnoidsInChunk(const ZmbStateStoredChunk &chunk);
	/** Count entries that remain residents of a shelter pack. */
	static int16 countShelterSnoidsInPack(const ZmbStateActivePack &pack);
	/** Count occupied entries in an active pack. */
	static int16 countOccupiedSnoidsInPack(const ZmbStateActivePack &pack);
	/** Limit populated entries in a storage chunk while preserving entry order. */
	static void limitStoredSnoidsInChunk(ZmbStateStoredChunk &chunk, int16 maximumCount);
	/** Limit shelter residents in a pack while preserving nonresident entries. */
	static void limitShelterSnoidsInPack(ZmbStateActivePack &pack, int16 maximumCount);
	/** Limit occupied entries in a pack while preserving unoccupied entries. */
	static void limitOccupiedSnoidsInPack(ZmbStateActivePack &pack, int16 maximumCount);
	/** Normalize a parsed state file into a validated, loadable state. */
	static bool recoverState(ZmbStateFile &state);
	/** Validate untrusted state and populate the save-management location summary. */
	static bool summarizeState(const ZmbStateFile &state, ZoombiniSaveSummary &summary);
	/** Compact occupied active-pack entries toward the front of the pack. */
	static void compactActivePack(ZmbStateActivePack &pack);
	/** Return whether a filename is already present in a case-insensitive list. */
	static bool containsSaveFilename(const Common::Array<Common::String> &filenames, const Common::String &filename);
	/** Copy one save file without overwriting an existing destination. */
	static bool copySaveFile(Common::SaveFileManager *saveFileMan, const Common::String &source, const Common::String &destination);
	/** Move one save file without overwriting an existing destination. */
	static bool moveSaveFile(Common::SaveFileManager *saveFileMan, const Common::String &source, const Common::String &destination);
	/** Restore files staged by @ref compactRosterSaveFiles after a failed operation. */
	static bool restoreCompactedSaveFiles(Common::SaveFileManager *saveFileMan, const Common::Array<Common::String> &originalFilenames,
										  const Common::Array<Common::String> &temporaryFilenames, const Common::Array<Common::String> &compactedFilenames,
										  int stagedCount, int compactedCount);
	/** Compact a decoded roster and all state files referenced by it. */
	static ZmbSaveCompactResult compactRosterSaveFiles(Common::SaveFileManager *saveFileMan, const Common::String &target, ZmbRosterFile &roster);
	/** Generate a pack using either standard or Picker availability rules. */
	int16 generateRandomPack(int16 maxCount, bool usePickerGenerationRules);

	/** Engine instance that owns this game-state model. */
	MohawkEngine_Zoombini *_vm;
	/** Save manager used for target-scoped roster and state files. */
	Common::SaveFileManager *_saveFileMan;

	/** Whether the first-launch notification has not yet been consumed. */
	bool _isFirstLaunch = true;

	/** Volatile runtime flag, not stored in save file. */
	bool _flagCursorVisible = true;
	/** Whether a debugger state write requires confirmation before saving. */
	bool _debugStateMutationFlag = false;
	/** Whether the live debugger state cannot be serialized as a coherent game. */
	bool _debugUnsafeSyntheticStateFlag = false;
	/** Whether the current state has completed initial loading. */
	bool _gameStateReadyFlag = false;
	/** Whether the most recent load dialog was cancelled. */
	bool _lastLoadCancelledFlag = false;
	/** Whether a save prompt is pending before quit. */
	bool _saveBeforeQuitPending = false;
	/** Whether the next load should suppress save-before-quit marking. */
	bool _suppressSaveBeforeQuitMarkForLoad = false;

	/** Initialize release-specific runtime defaults. */
	void initVariantDefaults();
	/** Serialize the current state and release-specific optional fields. */
	void syncGameState(Common::Serializer &s, bool isTlcLayout, bool hasCompletionCounters);

	/**
	 * The active ScummVM target id, used to namespace the roster and save files
	 * so every Zoombini target keeps its own independent set of saves.
	 */
	Common::String getTargetName() const;
	/** Build the target-scoped state filename for a roster slot. */
	Common::String getSaveFilename(int slot) const;
	/** Build the target-scoped roster filename. */
	Common::String getRosterFilename() const;

	/** Load one target-scoped save state. */
	bool loadState(int slot);
	/** Save one target-scoped save state. */
	bool saveState(int slot);
};

/**
 * Summary data shown by the ScummVM save-management page.
 *
 * The name and filename retain the original executable encoding. The options
 * widget decodes the name for display.
 */
struct ZoombiniSaveSummary {
	/** User-visible save name bytes as stored in ZOOMBINI.WHO. */
	Common::String _saveName;
	/** Four ASCII decimal digits from the ZOOMBINI.WHO state-file stem. */
	Common::String _saveFileNumber;
	/** Target-scoped ScummVM state filename derived from the roster stem. */
	Common::String _fileName;
	/** Detected serialized state-file layout. */
	ZoombiniGameState::ZmbSaveFormat _saveFormat = ZoombiniGameState::ZmbSaveFormat::kInvalid;
	/** RodMap's Isle/on-road count after separating the occupied active pack. */
	int16 _isleCount = -1;
	/** RodMap's logical Basecamp 1 ownership count. */
	int16 _basecamp1Count = -1;
	/** RodMap's logical Basecamp 2 ownership count. */
	int16 _basecamp2Count = -1;
	/** RodMap's logical Zoombiniville ownership count. */
	int16 _townCount = -1;
	/** Number of occupied Zoombinis in the persisted active pack. */
	int16 _activePackCount = -1;
	/** Whether the roster row and its state file passed structural and count validation. */
	bool _stateValid = false;
	/** Whether the parsed state can be normalized into a loadable save. */
	bool _stateRecoverable = false;
	/** Primary structural problem found before best-effort recovery. */
	ZoombiniGameState::ZmbSaveIssue _stateIssue = ZoombiniGameState::ZmbSaveIssue::kNone;
};

} // End of namespace Mohawk

#endif

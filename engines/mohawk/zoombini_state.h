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

#include "common/array.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/language.h"
#include "common/noncopyable.h"
#include "common/platform.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/ustr.h"

#include "mohawk/zoombini_resource.h"

namespace Common {

class SaveFileManager;
class WriteStream;

} // namespace Common

namespace Mohawk {

class MohawkEngine_Zoombini;
class ZmbFeature;
class ZoombiniText;

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
	static const ZmbXferRouteInfo *getZmbXferRouteInfo(ZmbSrcPageKind srcPage);

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
	ZmbTrait(HairKind hair, EyeKind eyes, NoseKind nose, FeetKind feet)
		: _hair(hair), _eyes(eyes), _nose(nose), _feet(feet) {}

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
	/**
	 * Serialize the four trait bytes in save-file order.
	 * @param s Serializer reading or writing hair, eyes, nose and feet, in that order.
	 */
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
	/** Fixed encoded name field, including its NUL terminator; serialized verbatim. */
	char _name[kNameByteCount] = {};

public:
	const ZmbTrait &getTraits() const { return _traits; }
	void setTraits(const ZmbTrait &traits) { _traits = traits; }

	/**
	 * Serialize the stored entry using the selected release layout and native platform.
	 * @param s Serializer whose loading or saving mode selects the transfer direction.
	 * @param hasAlignedStoredEntries Whether the entry has two trailing padding bytes, as in TLCv2.0 and MacV11EU.
	 * @param platform Native save platform, selecting the rectangle's field order and word byte order.
	 */
	void sync(Common::Serializer &s, bool hasAlignedStoredEntries, Common::Platform platform);
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
	/** Check the bounded serialized name using the target text service's generation rules. */
	bool hasSupportedName(ZoombiniText &text) const;

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
	/**
	 * Serialize the stored chunk using the selected release layout.
	 * @param s Serializer whose loading or saving mode selects the transfer direction.
	 * @param hasAlignedStoredEntries Whether each stored entry has two trailing padding bytes (TLCv2.0 or MacV11EU).
	 * @param platform Native save platform used for the chunk's header words and entry rectangles.
	 */
	void sync(Common::Serializer &s, bool hasAlignedStoredEntries, Common::Platform platform);
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

	/** 0x09-0x12: Fixed encoded name field, including its NUL terminator; serialized verbatim (10 bytes). */
	char _name[10] = {};

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
	/**
	 * Serialize one active-pack entry using the selected release layout.
	 * @param s Serializer whose loading or saving mode selects the transfer direction.
	 * @param isTlcV20Layout Whether to use TLCv2.0's 20-byte entry, including one trailing padding byte.
	 * @param platform Native save platform, selecting coordinate byte order; Macintosh also requires a 20-byte entry.
	 */
	void sync(Common::Serializer &s, bool isTlcV20Layout, Common::Platform platform);
	/** Decode the active-pack name, or return empty when its NUL terminator is missing. */
	Common::U32String getU32Name(MohawkEngine_Zoombini *vm) const;
	/** Encode and assign an active-pack name through the active engine text codec. */
	void setU32Name(MohawkEngine_Zoombini *vm, const Common::U32String &name);
	/** Return whether the serialized name field contains a non-empty name. */
	bool hasSerializedName() const;
	/** Check the bounded serialized name using the target text service's generation rules. */
	bool hasSupportedName(ZoombiniText &text) const;

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
	/**
	 * Serialize the active pack header, flags, and entries for the native platform.
	 * @param s Serializer whose loading or saving mode selects the transfer direction.
	 * @param isTlcV20Layout Whether entries use TLCv2.0's trailing padding byte; Macintosh entries always have it.
	 * @param platform Native save platform used for header words and each active entry.
	 */
	void sync(Common::Serializer &s, bool isTlcV20Layout, Common::Platform platform);
	void copyTo(ZmbStateActivePack &dest) {
		dest = *this;
	}
};

/**
 * Native state-file layouts recognized by length and platform signature.
 * Files contain a platform signature but no explicit version tag.
 * The optional 10-byte completion tail contains four 16-bit route-perfect counters
 * and one 16-bit Town development level. Release numbering alone does not identify the layout.
 *
 * "TLC header" means the v2.0 state prefix before the first stored chunk at 0x00D0.
 * - 0x000A (byte): TouchSense
 * - 0x000B (byte): Help Audio
 * - 0x000C (word): Transition-disable flag
 * - 0x000E-0x0015: Fleens Transform Tables
 * - 0x00CC (word): Previous pageId
 * - 0x00CE (word): Current pageId
 */
enum class ZmbSaveFormat {
	/** Unrecognized layout; as a conversion destination, preserve the exact source bytes. */
	kInvalid = 0,
	/**
	 * 44549-byte Windows layout used by USv1.0-family and Europe-v1 releases, including EUv1.1.
	 *
	 * - Stored entries:  22 bytes
	 * - Active entries:  19 bytes
	 * - Completion tail: absent
	 */
	kWinV10,
	/**
	 * 44678-byte Macintosh layout used by USv1.0, EUv1.0 and JPv1.0 releases.
	 *
	 * - Stored entries:  22 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: absent
	 *
	 * One alignment byte follows the twin-generation status array.
	 */
	kMacV10,
	/**
	 * 48432-byte EUv1.1 Macintosh layout retaining the v1.0 header and progression fields.
	 *
	 * - Stored entries:  24 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: absent
	 *
	 * Two padding bytes precede the first stored chunk, and three follow the twin-generation status array.
	 * The v1.1 release number does not imply the presence of the completion tail.
	 */
	kMacV11EU,
	/**
	 * 44559-byte Windows layout used by USv1.1 and KRv1.11.
	 *
	 * - Stored entries:  22 bytes
	 * - Active entries:  19 bytes
	 * - Completion tail: 10 bytes
	 *
	 * Extends kWinV10 with the completion tail.
	 */
	kWinV11,
	/**
	 * 44688-byte USv1.1 Macintosh layout.
	 *
	 * - Stored entries:  22 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: 10 bytes
	 *
	 * Extends kMacV10 with the completion tail, retaining one alignment byte after the twin-generation status array.
	 */
	kMacV11US,
	/**
	 * 48430-byte Windows TLC v2.0 compatibility input.
	 *
	 * - Stored entries:  24 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: absent
	 *
	 * Already uses the TLC header.
	 * In the original TLC v2.0 family, support for this shorter form remains only in the reader.
	 * The reader zero-fills the missing five words; the writer always emits kWinV20, never this form.
	 * The original producer of this shorter form has not been established.
	 * ScummVM accepts this layout only as input; new TLC saves use kWinV20.
	 */
	kWinV20Compat,
	/**
	 * 48440-byte Windows TLC v2.0 writer layout.
	 *
	 * - Stored entries:  24 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: 10 bytes
	 *
	 * Uses the TLC header.
	 */
	kWinV20,
	/**
	 * 48430-byte Macintosh TLC v2.0 compatibility input.
	 *
	 * - Stored entries:  24 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: absent
	 *
	 * Uses the TLC header with Macintosh signature, scalar byte order and rectangle order.
	 * In the original TLC v2.0 family, support for this shorter form remains only in the reader.
	 * The reader zero-fills the missing five words; the writer always emits kMacV20, never this form.
	 * As with kWinV20Compat, the original producer of this shorter form has not been established.
	 * ScummVM accepts this layout only as input; new TLC saves use kMacV20.
	 */
	kMacV20Compat,
	/**
	 * 48440-byte Macintosh TLC v2.0 writer layout.
	 *
	 * - Stored entries:  24 bytes
	 * - Active entries:  20 bytes
	 * - Completion tail: 10 bytes
	 *
	 * Has the same TLC fields as kWinV20, but uses Macintosh signature, scalar byte order and rectangle order.
	 */
	kMacV20
};

/**
 * Represents the Zoombini game-state.
 *
 * Recognized native formats (sizes in bytes):
 * - WinV10 (44549)
 * - MacV10 (44678)
 * - MacV11EU (48432): retains MacV10 fields with different native alignment.
 * - WinV11 (44559)
 * - MacV11US (44688)
 * - WinV20/MacV20 (48440)
 * - WinV20Compat/MacV20Compat (48430).
 *
 * Member offsets below describe the Windows layouts.
 * Platform conventions and layout differences:
 * - The state signature is 0x006A on Macintosh and 0x006B on Windows.
 * - Both platforms store the signature and auto-sticky delay as big-endian 16-bit fields.
 *   Subsequent 16-bit fields are little-endian on Windows and big-endian on Macintosh.
 * - Active entries use 19 bytes in Windows v1 layouts and 20 bytes in all Macintosh and TLC layouts.
 * - MacV11EU uses 24-byte stored entries, inserts two padding bytes before the first stored chunk,
 *   and ends with three padding bytes after the twin-generation status array.
 * - Other Macintosh layouts have one alignment byte after the twin-generation status array.
 * - Stored rectangles use left/top/right/bottom order on Macintosh and bottom/right/top/left order on Windows.
 *
 * MacV10 and MacV11EU have three stored chunks (Basecamp 1, Basecamp 2 and Town),
 * each containing 625 serialized entries, including unused slots:
 * @code
 * MacV10:   [traits: 4][rectangle: 8][name: 10]             = 22 bytes per entry
 * MacV11EU: [traits: 4][rectangle: 8][name: 10][padding: 2] = 24 bytes per entry
 *
 * Extra stored-entry padding:       3 * 625 * 2 = 3750 bytes
 * Padding before the first stored chunk:        +    2 bytes
 * Extra padding after the twin-status array:    +    2 bytes (3 instead of 1)
 *                                               ------------
 * Total size difference:                          3754 bytes
 * MacV11EU file size:             44678 + 3754 = 48432 bytes
 * @endcode
 * Both layouts use 20-byte active entries and have no completion tail;
 * the entire 3754-byte difference comes from alignment padding.
 *
 * The shared in-memory state normalizes these differences. Signatures and alignment
 * padding are handled by the serializer and are not retained as game-state members.
 */
struct ZmbStateFile {
	/** Construct a state file with its release-independent defaults. */
	ZmbStateFile() {}

	/** Native Windows signature: BE16 in state files, LE16 in roster files. */
	static constexpr uint16 kWindowsMagic = 0x006B;
	/** Native Macintosh signature: BE16 in both state and roster files. */
	static constexpr uint16 kMacintoshMagic = 0x006A;

	/** Return the native platform whose signature, word order and geometry the save format uses. */
	static Common::Platform getSavePlatform(ZmbSaveFormat format);
	/** Return the writable counterpart, upgrading read-only TLC compatibility layouts. */
	static ZmbSaveFormat getWriteSaveFormat(ZmbSaveFormat format);
	/** EUv1.1 Macintosh retains v1.0 fields with four-byte rectangle alignment. */
	static bool usesMacV11EUAlignment(ZmbSaveFormat format);
	/** Return whether a save format is a TLC layout. */
	static bool isTlcSaveFormat(ZmbSaveFormat format);
	/** Return whether a save format stores route completion counters. */
	static bool hasCompletionCounters(ZmbSaveFormat format);

	/**
	 * Persisted two-byte page state.
	 *
	 * The low 12 bits hold the saturating visit count.
	 * The next two bits record the first and second normal Hard-group triggers.
	 * The serialized value remains one uint16 in the selected platform's byte order.
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
		/**
		 * Serialize the stored uint16 using the native platform's byte order.
		 * @param s Serializer reading or writing the packed visit count and trigger flags.
		 * @param platform Native save platform selecting the word byte order.
		 */
		void sync(Common::Serializer &s, Common::Platform platform) {
			const bool isLE = platform == Common::kPlatformWindows;
			s.syncMultipleEndian(isLE, _raw);
		}

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
	/** Record the selected transfer's puzzle progress and required shelter discoveries. */
	void markDebugXferProgress(const ZmbXferRouteInfo &routeInfo);

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

	/**
	 * Serialize game state, validating or writing the platform's native signature and skipping padding.
	 * Decode the body even after a signature mismatch so recovery can inspect its fields;
	 * the caller decides whether to reject or repair it.
	 * V20Compat layouts are read-only and are rejected before writing any fields.
	 * @param s Serializer whose loading or saving mode selects the transfer direction.
	 * @param format Native save format selecting the signature, byte order, fields and alignment.
	 * @return Whether the format supports this transfer, the signature matches and serializer I/O succeeded.
	 */
	bool sync(Common::Serializer &s, ZmbSaveFormat format);

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

	/**
	 * Serialize the roster entry's fixed-width byte fields.
	 * @param r Serializer reading or writing the save-name and file-stem byte arrays.
	 */
	void sync(Common::Serializer &r);
	/** Decode the user-visible save name, or return empty when unterminated. */
	Common::U32String getSaveName(MohawkEngine_Zoombini *vm) const;
	/** Decode the user-visible save name through an explicitly selected code page. */
	Common::U32String getSaveName(Common::CodePage codePage) const;
	/** Return the bounded byte length of the save name, or the field capacity when unterminated. */
	uint16 getSaveNameLength() const;
	/**
	 * Return the validated ASCII ZOOM#### state-file stem from the fixed-width roster field.
	 * Read at most the field's nine bytes; return empty if its terminator, prefix or digits are invalid.
	 */
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

/** Native ZOOMBINI.WHO roster with a fixed-capacity entry array and filename counter. */
struct ZmbRosterFile {
	/** Construct an empty roster. */
	ZmbRosterFile() {}

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
	/** Return the number of slots in the fixed-capacity roster array. */
	uint16 getEntryCapacity() const { return static_cast<uint16>(ARRAYSIZE(_entries)); }
	/** Return whether an index addresses a serialized roster entry safely. */
	bool hasEntry(int index) const { return hasValidEntryCount() && 0 <= index && index < getEntryCount(); }
	/** Return whether the header and every used entry are structurally valid. */
	bool isSerializedStateValid() const;
	/**
	 * Return a mutable slot from the fixed-capacity roster array.
	 * @param index Zero-based slot; must be in [0, getEntryCapacity()).
	 */
	ZmbRosterEntry &getEntry(int16 index) { return _entries[index]; }
	/**
	 * Return a read-only slot from the fixed-capacity roster array.
	 * @param index Zero-based slot; must be in [0, getEntryCapacity()).
	 */
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

	/**
	 * Remove a used roster entry, shift later entries left and clear the vacated tail slot.
	 * @param index Zero-based index of the used entry to remove.
	 * @return Whether the index was valid and the entry was removed.
	 */
	bool removeEntryAt(int16 index) {
		if (!hasEntry(index))
			return false;

		for (int16 i = index; i + 1 < _saveEntryCount; i++)
			_entries[i] = _entries[i + 1];
		_entries[_saveEntryCount - 1] = ZmbRosterEntry();
		_saveEntryCount -= 1;
		return true;
	}

	/** Mark the roster empty while retaining its entry bytes and next filename counter. */
	void clearEntries() { _saveEntryCount = 0; }

	/**
	 * Serialize the roster, detecting its native signature on load and selecting it from platform on save.
	 * Decode available fields even for a bad signature to permit recovery.
	 * @param r Serializer whose loading or saving mode selects the transfer direction.
	 * @param platform Native output platform when saving; ignored on load, where the signature selects the byte order.
	 * @return Whether the signature and serializer I/O are valid.
	 */
	bool sync(Common::Serializer &r, Common::Platform platform);
};

struct ZoombiniSaveSummary;

/**
 * Transfers Zoombinis saves between a ScummVM target's save namespace and an installed or DOSBox game folder
 * using raw ZOOMBINI.WHO and ZOOM####.TXT files.
 *
 * Each entry point is a self-contained, modal UI flow: it prompts for a folder,
 * copies the roster-referenced saves, and reports the result.
 * Folder transfers preserve the .TXT payloads without converting their native formats.
 * Imported rosters use the dest target's native platform; exported rosters retain their stored bytes.
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

class ZoombiniGameState : private Common::NonCopyable {
public:
	/** Result of compacting roster file stems and the next filename counter. */
	enum class ZmbSaveCompactResult {
		/** Sequential file stems and the matching next counter were written successfully. */
		kSuccess,
		/** File stems are already sequential and the next counter equals the entry count. */
		kNoChanges,
		/** The roster could not be loaded or its serialized data is invalid. */
		kInvalidRoster,
		/** A state file referenced by the roster does not exist. */
		kMissingSaveFile,
		/** A compacted filename or the next filename is occupied by a file outside this roster. */
		kFileNameConflict,
		/** Staging, copying a duplicate reference, or renaming a state file failed. */
		kFileMoveFailed,
		/** Backing up the original roster or writing the updated roster failed. */
		kRosterWriteFailed,
		/** A failed operation could not restore or preserve all original files. */
		kRollbackFailed
	};

	/** Result of recovering one corrupted state file. */
	enum class ZmbSaveRecoverResult {
		/** The repaired state replaced the original, passed validation, and retains an original-file backup. */
		kSuccess,
		/** The roster or slot is invalid, or the state is unreadable, unsupported, already valid, or cannot be repaired. */
		kNotRecoverable,
		/** Selecting a temporary filename, writing the repaired state, or validating that temporary file failed. */
		kTemporaryWriteFailed,
		/** Selecting a backup filename, copying the original, or checking the backup format failed. */
		kBackupFailed,
		/** Removing the original, installing the repaired file, or validating the installed file failed. */
		kReplaceFailed,
		/** Replacement failed and the original could not be copied back from its backup. */
		kRollbackFailed
	};

	/** Combinable structural problems that can require rebuilding ZOOMBINI.WHO. */
	enum ZmbRosterIssueFlags : uint32 {
		/** No roster problem was detected, including when neither roster nor state files exist. */
		kRosterIssueNone = 0,
		/** State files exist, but their roster could not be opened. */
		kRosterIssueMissing = 1 << 0,
		/** The roster length differs from the expected 1606 bytes. */
		kRosterIssueSize = 1 << 1,
		/** The roster's native signature is invalid. */
		kRosterIssueMagic = 1 << 2,
		/** The declared entry count exceeds the entries available in the roster file. */
		kRosterIssueEntryCount = 1 << 3,
		/** The next filename counter is outside the writer's range 0..10000. */
		kRosterIssueNextCounter = 1 << 4,
		/** A roster entry has an empty save title or lacks a NUL terminator within its title field. */
		kRosterIssueSaveName = 1 << 5,
		/** A roster entry does not contain a valid ZOOM#### state-file stem. */
		kRosterIssueFileStem = 1 << 6,
		/** A roster entry references a state file that was not found. */
		kRosterIssueMissingState = 1 << 7,
		/** State files lack usable roster references; unmatched files are reported when roster capacity remains. */
		kRosterIssueOrphanState = 1 << 8,
		/** The roster header and available entries could not be read or decoded. */
		kRosterIssueRead = 1 << 9,
		/** Multiple roster entries reference the same existing state file. */
		kRosterIssueDuplicateFileStem = 1 << 10,
	};

	/** Result of rebuilding a corrupted ZOOMBINI.WHO roster. */
	enum class ZmbRosterRecoverResult {
		/** The rebuilt roster was installed, validated, and checked against the available state files. */
		kSuccess,
		/** No roster issue was detected, so rebuilding was skipped. */
		kNotNeeded,
		/** Selecting a backup filename, copying the original roster, or checking its backup failed. */
		kBackupFailed,
		/** Creating, validating, or installing the rebuilt roster failed. */
		kWriteFailed,
		/** Installing the rebuilt roster failed and its original backup could not be restored. */
		kRollbackFailed,
	};

	/** Primary structural problem detected while validating one state file. */
	enum class ZmbSaveIssue {
		/** No state validation problem has been recorded. */
		kNone,
		/** The state file's native signature is invalid. */
		kMagic,
		/** The current page cannot be restored, or the TLC previous-page value is invalid. */
		kPage,
		/** Bounded flags or scalar fields are invalid, or generation bookkeeping is inconsistent. */
		kScalarFields,
		/** Basecamp 1 stored-chunk headers, traits, or name terminators are invalid. */
		kBasecamp1Storage,
		/** Basecamp 2 stored-chunk headers, traits, or name terminators are invalid. */
		kBasecamp2Storage,
		/** Town stored-chunk headers, traits, or name terminators are invalid. */
		kTownStorage,
		/** The Isle pack has invalid headers, entry traits, occupancy flags, or name terminators. */
		kIslePack,
		/** The Basecamp 1 pack has invalid headers, entry traits, occupancy flags, or name terminators. */
		kBasecamp1Pack,
		/** The Basecamp 2 pack has invalid headers, entry traits, occupancy flags, or name terminators. */
		kBasecamp2Pack,
		/** The traveling pack has invalid headers, entry traits, occupancy flags, or name terminators. */
		kActivePack,
		/** The current puzzle has no valid 1..16-member pack, or Caves skips occupied entries. */
		kPuzzlePack,
		/** The current route is outside 0..18 or Town development is outside 0..6. */
		kProgressFields,
		/** A route difficulty level is outside the serialized range 0..3. */
		kRouteLevel,
		/** A level-zero route has progress values that cannot select valid RodMap shape frames. */
		kRodMapProgress,
		/** A route-perfect counter is outside the serialized range 0..2. */
		kPerfectCounter,
		/** A recorded Basecamp 1, Basecamp 2, or Town population is outside 0..625. */
		kLogicalLocationCount,
		/** The occupied traveling-pack count or the derived remaining Isle population is outside 0..625. */
		kIsleActiveCount,
		/** Recorded shelter populations do not fit stored/resident slots, or Town's count differs from its stored population. */
		kPhysicalLocationCount,
	};

	/** Exact Windows v1.0/European v1.1 state-file size. */
	static constexpr int32 kStateFileSizeWinV10 = 44549;
	/** Exact Macintosh v1.0 state-file size, including active-entry alignment. */
	static constexpr int32 kStateFileSizeMacV10 = 44678;
	/** EUv1.1 Macintosh uses aligned stored entries and header/tail padding. */
	static constexpr int32 kStateFileSizeMacV11EU = 48432;
	/** Exact USv1.1 Macintosh state-file size, including the completion tail. */
	static constexpr int32 kStateFileSizeMacV11US = 44688;
	/** Exact Windows v1.1 state-file size. */
	static constexpr int32 kStateFileSizeWinV11 = 44559;
	/** Exact Windows/Macintosh TLC v2.0 compatibility-input size without the final five-word completion tail. */
	static constexpr int32 kStateFileSizeTlcV20Compat = 48430;
	/** Exact Windows/Macintosh TLC v2.0 state-file size. */
	static constexpr int32 kStateFileSizeTlcV20 = 48440;
	/** Exact uncompressed ZOOMBINI.WHO size in every release family. */
	static constexpr uint32 kZmbRosterFileSize = 1606;
	/** Return the compact display identifier for a serialized save format. */
	static Common::U32String getSaveFormatName(ZmbSaveFormat format);

	/** Return whether an uncompressed size matches a supported native state-file layout. */
	static bool isValidStateFileSize(int64 size);
	/** Identify a state layout using its signature and length; leave the stream position unchanged. */
	static ZmbSaveFormat getSaveFormat(Common::SeekableReadStream &stream);
	/** Identify a raw state payload, including Macintosh and Windows TLC files of equal length. */
	static ZmbSaveFormat getSaveFormat(const Common::Array<byte> &bytes);

	/** Convert to a writable layout while preserving name bytes; an invalid destination preserves all source bytes. */
	static bool convertStateBytes(const Common::Array<byte> &srcBytes, ZmbSaveFormat destFormat, Common::Array<byte> &output);
	/** Convert a managed save in place, retaining its original bytes in the returned backup file. */
	static bool convertSaveForTarget(const Common::String &target, int slot, ZmbSaveFormat destFormat, Common::String &backupFilename);

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
	 * They are not serialized or reset by New Game or Load Game.
	 * Layout option changes replace the affected next selections.
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
		/** Whether the layout selectors have been initialized. */
		bool _initialVariantsSelected = false;
		/** Randomization option used for the current selectors. */
		bool _randomizeInitialLayout = false;
		/** Level-4 layout-pool option used for the current selector. */
		bool _restoreUnusedL4Layout = false;
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
	/** Serialize and write one roster file using the destination target's platform. */
	static bool saveRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster,
							   Common::Platform platform);

	/** Load the target's roster file into memory, detecting either native byte order. */
	void loadRoster();
	/** Persist the in-memory roster file using the active target's native platform. */
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
	bool moveGame(int slot, int destSlot);
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
	static bool moveSaveForTarget(const Common::String &target, int slot, int destSlot);
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
	static bool appendNewSaveEntryForTarget(Common::SaveFileManager *saveFileMan, const Common::String &target, ZmbRosterFile &roster, ZmbRosterEntry entry);
	/** Remove leading and trailing ASCII spaces from a save name. */
	static Common::U32String trimSaveName(const Common::U32String &name);
	/** Return the executable code page used for save names in a target. */
	static Common::CodePage getSaveNameCodePage(const Common::String &target);
	/** Encode and zero-fill a roster save-name field. */
	static bool encodeRosterSaveName(const Common::U32String &saveName, Common::CodePage codePage, byte *output, uint32 outputSize);
	/** Rename one entry in a decoded roster. */
	static bool renameRosterEntry(const Common::String &target, ZmbRosterFile &roster, int slot, const Common::U32String &saveName);

	/**
	 * Build the per-target save filename "{targetId}-####.ZMB" from a legacy ZOOM#### stem.
	 * @param targetName ScummVM target namespace for the managed save.
	 * @param fileStem Length-bearing eight-character ASCII stem; use ZmbRosterEntry::getSaveFileStem() for roster fields.
	 * @return The managed filename, or empty if the stem has an invalid length, prefix or counter.
	 */
	static Common::String makeSaveFilename(const Common::String &targetName, const Common::String &fileStem);

	/** Return whether serialized-state changes are not reflected in a save file. */
	bool isStateDirty() const { return _journeyState._isDirty; }
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
	 * @param puzzleLevel The player-facing difficulty level, from 1 through 4.
	 * @return True when the arguments are valid, otherwise false.
	 */
	bool setRouteDifficultyLevel(ZmbRouteId routeId, int16 puzzleLevel);

	/**
	 * Record a first-clear memorial for a completed route level.
	 *
	 * Existing records for the same route and level are not duplicated. Normal
	 * route completion calls this before route advancement, so the level is the
	 * player-facing level that was just completed.
	 * @param routeId The zero-based route that was completed.
	 * @param puzzleLevel The player-facing level that was completed, from 1 through 4.
	 * @return True when a new memorial was stored, otherwise false.
	 */
	bool recordFirstClearMemorial(ZmbRouteId routeId, int16 puzzleLevel);

	/**
	 * Return whether a first-clear memorial is active for a route and level.
	 * @param routeId The zero-based route to query.
	 * @param puzzleLevel The one-based completed level, from 1 through 4.
	 * @return True when the matching memorial slot is active.
	 */
	bool readMemorialActive(ZmbRouteId routeId, int16 puzzleLevel) const;

	/**
	 * Read the date of an active first-clear memorial.
	 * @param routeId The zero-based route to query.
	 * @param puzzleLevel The one-based completed level, from 1 through 4.
	 * @param year Receives the Gregorian year, from 1 through 9999.
	 * @param month Receives the month, from 1 through 12.
	 * @param day Receives the valid day for the selected month.
	 * @return True when the matching memorial slot is active.
	 */
	bool readMemorialDate(ZmbRouteId routeId, int16 puzzleLevel, uint16 &year, byte &month, byte &day) const;

	/**
	 * Activate or deactivate a first-clear memorial for a route and level.
	 * Activating a missing record creates it with the command-time date.
	 * Deactivating a record clears all serialized fields in its slot.
	 * @param routeId The zero-based route to update.
	 * @param puzzleLevel The one-based completed level, from 1 through 4.
	 * @param active True to create or retain the memorial, false to clear it.
	 * @return True when the arguments are valid and the requested state fits in the memorial slots.
	 */
	bool setMemorialActive(ZmbRouteId routeId, int16 puzzleLevel, bool active);

	/**
	 * Set the date of a first-clear memorial.
	 * A missing record is created and activated with the supplied date.
	 * @param routeId The zero-based route to update.
	 * @param puzzleLevel The one-based completed level, from 1 through 4.
	 * @param year The Gregorian year, from 1 through 9999.
	 * @param month The month, from 1 through 12.
	 * @param day The valid day for the selected month.
	 * @return True when the date and target memorial are valid.
	 */
	bool setMemorialDate(ZmbRouteId routeId, int16 puzzleLevel, uint16 year, byte month, byte day);

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
	void markXferContainerArrival(ZmbSrcPageKind srcPage, const ZmbXferRouteInfo &routeInfo);

	/**
	 * Runtime (non-persisted): set to the puzzle page DI when a puzzle that leads
	 * to a container page (BC1, BC2, Town) completes successfully.
	 * Read and cleared during BC1, BC2, and Town page initialization.
	 */
	uint16 _lastPageBeforeContainer = 0;
	/** Return whether RodMap practice mode has a selected difficulty. */
	bool inPracticeMode() const { return _practiceLevel != 0; }
	/** Return the state used by the active gameplay session. */
	ZmbStateFile &getCurrentState() { return _practiceModeActive ? _practiceState : _journeyState; }
	/** Return the state used by the active gameplay session. */
	const ZmbStateFile &getCurrentState() const { return _practiceModeActive ? _practiceState : _journeyState; }
	/** Seed and activate an isolated practice state. */
	void beginPracticeState();
	/** Stop using the isolated practice state without changing the saved game state. */
	void endPracticeState();

	/** Whether this release stores a separate perfect clear counter for each route. */
	bool hasRoutePerfectClearCounters() const;

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
	bool getEnableSound() { return _journeyState.getSfxEnabled(); }
	/** Return whether background music is enabled. */
	bool getEnableMusic() { return _journeyState.getBgmEnabled(); }
	/** Return whether sticky-mouse behavior is enabled. */
	bool getEnableStickyMouse() { return _journeyState.getStickyMouseEnabled(); }
	/** Return whether automatic sticky-mouse activation is enabled. */
	bool getEnableAutoStickyMouse() { return _journeyState.getAutoStickyMouseEnabled(); }
	/** Return the delay before automatic sticky-mouse activation. */
	uint16 getAutoStickyThreshold() { return _journeyState._autoStickyDelay; }
	/** Return whether transition scenes are enabled. */
	bool getEnableTransitions();
	/** Return whether touch-sense behavior is enabled. */
	bool getEnableTouchSense();
	/** Return whether help audio is enabled. */
	bool getEnableHelpAudio();
	/** Return whether reduced-action mode is enabled. */
	bool isLessActionEnabled() { return _journeyState.getLessActionEnabled(); }
	/** Return whether the persistent built-in debug option is enabled. */
	bool getDebugEnabled() const { return _journeyState.getDebugEnabled(); }
	/** Set the persistent built-in debug option. */
	void setDebugEnabled(bool enabled) { _journeyState.setDebugEnabled(enabled); }
	/** Return the generated-Zoombini count from the journey state. */
	int16 getGeneratedZoombiniCount() const { return _journeyState._zmbGeneratedCount; }
	/** Return whether the in-game cursor is visible. */
	bool isCursorVisible() { return _flagCursorVisible; }
	/** Set the global sound-enabled option and optionally show its page notification. */
	void setEnableSound(bool val, bool showNoti = true);
	/** Set the global music-enabled option and optionally show its page notification. */
	void setEnableMusic(bool val, bool showNoti = true);
	/** Set the sticky-mouse option and optionally show its page notification. */
	void setEnableStickyMouse(bool val, bool showNoti = true);
	/** Set the automatic sticky-mouse option. */
	void setEnableAutoStickyMouse(bool val);
	/** Set the transition-effects option and optionally show its page notification. */
	void setEnableTransitions(bool val, bool showNoti = true);
	/** Set the touch-sense option and optionally show its page notification. */
	void setEnableTouchSense(bool val, bool showNoti = true);
	/** Set the help-audio option and optionally show its page notification. */
	void setEnableHelpAudio(bool val, bool showNoti = true);
	/** Set the reduced-action option. */
	void setLessActionEnabled(bool val);
	/** Set visibility of the in-game cursor. */
	void setCursorVisible(bool val);
	/** Toggle sound effects and return the new value. */
	bool toggleSound(bool showNoti = true) {
		setEnableSound(!getEnableSound(), showNoti);
		return getEnableSound();
	}
	/** Toggle background music and return the new value. */
	bool toggleMusic(bool showNoti = true) {
		setEnableMusic(!getEnableMusic(), showNoti);
		return getEnableMusic();
	}
	/** Toggle sticky-mouse behavior and return the new value. */
	bool toggleStickyMouse(bool showNoti = true) {
		setEnableStickyMouse(!getEnableStickyMouse(), showNoti);
		return getEnableStickyMouse();
	}
	/** Toggle automatic sticky-mouse behavior and return the new value. */
	bool toggleAutoStickyMouse() {
		setEnableAutoStickyMouse(!getEnableAutoStickyMouse());
		return getEnableAutoStickyMouse();
	}
	/** Toggle transition effects and return the new value. */
	bool toggleTransitions(bool showNoti = true) {
		setEnableTransitions(!getEnableTransitions(), showNoti);
		return getEnableTransitions();
	}
	/** Toggle touch-sense behavior and return the new value. */
	bool toggleTouchSense(bool showNoti = true) {
		setEnableTouchSense(!getEnableTouchSense(), showNoti);
		return getEnableTouchSense();
	}
	/** Toggle help audio and return the new value. */
	bool toggleHelpAudio(bool showNoti = true) {
		setEnableHelpAudio(!getEnableHelpAudio(), showNoti);
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

	/** Current serialized save roster. */
	ZmbRosterFile _r;

	/** Selected practice difficulty, retained on rodmap between practice sessions, or zero in journey mode. */
	uint16 _practiceLevel = 0;
	/**
	 * Runtime flags indexed by canonical STRL name ID, from 0 through 624.
	 * A non-zero entry means that ID has already been selected in the current name cycle.
	 * The supported v1.11KR release is currently the only release with the STRL 30000-30006 name pool.
	 * Releases without that pool generate names procedurally and do not use this table.
	 * This table is not serialized and survives save loads and new games within the engine session.
	 */
	byte _nameIdDrawnFlags[625] = {};
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
	/** Canonical serialized state for journey gameplay and save files. */
	ZmbStateFile _journeyState;
	/** Isolated gameplay state used only while a practice puzzle or transition is active. */
	ZmbStateFile _practiceState;
	/** Whether gameplay state access is currently routed to @ref ZoombiniGameState::_practiceState. */
	bool _practiceModeActive = false;

	/** Return the save-manager error, or the operation-specific fallback. */
	static Common::Error getSaveFileWriteError(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode);
	/** Build the localized write-failure reason, including backend details when available. */
	static Common::U32String getSaveFileWriteFailureReason(Common::SaveFileManager *saveFileMan, Common::ErrorCode fallbackErrorCode);
	/** Report a state-file write failure to the player. */
	static void showStateFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode);
	/** Report a roster-file write failure to the player. */
	static void showRosterFileWriteFailure(Common::SaveFileManager *saveFileMan, const Common::String &filename, Common::ErrorCode fallbackErrorCode);
	/** Write one roster while returning the operation-specific fallback error category. */
	static bool writeRosterFile(Common::SaveFileManager *saveFileMan, const Common::String &filename, const ZmbRosterFile &roster, Common::Platform platform, Common::ErrorCode *fallbackErrorCode);
	/** Return whether any living Zoombini has a serialized name unsupported by the target release. */
	static bool hasUnsupportedZoombiniName(const ZmbStateFile &state, ZoombiniText &text);
	/** Replace every living Zoombini name through the active release's standard name generator. */
	void replaceZoombiniNamesWithStandardNames(ZmbStateFile &state);
	/** Find the active memorial slot for a route and level, or return -1. */
	int16 findFirstClearMemorialSlot(ZmbRouteId routeId, int16 puzzleLevel) const;
	/** Return whether a Gregorian memorial date is within its valid calendar range. */
	static bool isValidMemorialDate(uint16 year, byte month, byte day);
	/** Add a memorial with an already captured date if the route/level is missing. */
	bool addFirstClearMemorial(ZmbRouteId routeId, int16 puzzleLevel, uint16 year, byte month, byte day);
	/** Clear all serialized fields belonging to one memorial slot. */
	void clearFirstClearMemorial(uint16 slotIdx);
	/**
	 * Make route-path completion flags consistent with a debugger-selected route level.
	 * @param routeId Route whose puzzle and endpoint completion flags are updated.
	 * @param puzzleLevel Selected starting level (1..4); only lower levels are marked complete.
	 */
	void syncRouteProgressFlags(ZmbRouteId routeId, int16 puzzleLevel);
	/**
	 * Make memorial records consistent with a debugger-selected route level.
	 * @param routeId Route whose first-clear memorials are updated.
	 * @param puzzleLevel Selected starting level (1..4); ensure memorials exist for all lower levels.
	 * @param removeAtOrAboveTarget Whether to remove this route's memorials at or above the selected level and malformed level-zero records.
	 */
	void syncRouteFirstClearMemorials(ZmbRouteId routeId, int16 puzzleLevel, bool removeAtOrAboveTarget);
	/**
	 * Select a save format from an exact serialized file size.
	 * @param size Exact native state-file size in bytes.
	 * @param platform Native file platform used to distinguish layouts with matching file sizes.
	 */
	static ZmbSaveFormat getSaveFormatFromSize(int32 size, Common::Platform platform);

	/** Replace a managed save only after verifying both the converted file and an original-byte backup. */
	static bool replaceSaveFile(Common::SaveFileManager *manager, const Common::String &filename, const Common::Array<byte> &src, const Common::Array<byte> &converted, Common::String &backupFilename);
	/** Select the write format for the active engine release. */
	static ZmbSaveFormat getTargetSaveFormat(const MohawkEngine_Zoombini *vm);
	/** Return whether the engine can load a source save format. */
	static bool canLoadSaveFormat(ZmbSaveFormat srcFormat);
	/** Return whether saving in the target format would discard source-only fields. */
	static bool isNewerThanTargetSaveFormat(ZmbSaveFormat srcFormat, ZmbSaveFormat targetFormat);
	/** Initialize route completion counters for a newly created state. */
	static void initializeCompletionCounters(ZmbStateFile &state);
	/** Upgrade source-layout fields into the target release layout. */
	static void upgradeStateForVariant(ZmbStateFile &state, ZmbSaveFormat srcFormat, ZmbSaveFormat targetFormat);
	/** Select the serialized layout read by a target for the available file length. */
	static ZmbSaveFormat getLoadSaveFormatFromSize(int32 size, ZmbSaveFormat targetFormat);
	/** Read and validate the big-endian magic at the stream's current position. */
	static bool hasValidStateFileMagic(Common::SeekableReadStream &stream, ZmbSaveFormat format);
	/** Decode complete roster entries from a bounded file prefix, reporting serialization success separately for recovery. */
	static bool decodeAvailableRosterEntries(Common::SeekableReadStream *file, ZmbRosterFile &roster, uint16 &availableEntryCapacity, bool &hasSyncSucc);
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
	static bool copySaveFile(Common::SaveFileManager *saveFileMan, const Common::String &srcFile, const Common::String &destFile);
	/** Move one save file without overwriting an existing destination. */
	static bool moveSaveFile(Common::SaveFileManager *saveFileMan, const Common::String &src, const Common::String &dest);
	/** Rollback files staged or renamed by @ref compactRosterSaveFiles to their original filenames after failure. */
	static bool rollbackSaveFileCompaction(Common::SaveFileManager *saveFileMan, const Common::Array<Common::String> &orgFilenames,
										   const Common::Array<Common::String> &tempFilenames, const Common::Array<Common::String> &compactedFilenames,
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
	/**
	 * Serialize the current state and release-specific optional fields.
	 * @param s Serializer reading or writing the journey state.
	 * @param format Native save format selecting the layout, optional fields, alignment and platform.
	 */
	void syncGameState(Common::Serializer &s, ZmbSaveFormat format);

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
	ZmbSaveFormat _saveFormat = ZmbSaveFormat::kInvalid;
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

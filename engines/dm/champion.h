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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_CHAMPION_H
#define DM_CHAMPION_H

#include "common/str.h"

#include "dm/dm.h"
#include "dm/gfx.h"

namespace DM {

#define kDMIgnoreObjectModifiers 0x4000 // @ MASK0x4000_IGNORE_OBJECT_MODIFIERS
#define kDMIgnoreTemporaryExperience 0x8000 // @ MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE

class Scent {
	uint16 _scent;
public:
	explicit Scent(uint16 scent = 0): _scent(scent) {}

	uint16 getMapX() { return _scent & 0x1F; }
	uint16 getMapY() { return (_scent >> 5) & 0x1F; }
	uint16 getMapIndex() { return (_scent >> 10) & 0x3F; }

	void setMapX(uint16 val) { _scent = (_scent & ~0x1F) | (val & 0x1F); }
	void setMapY(uint16 val) { _scent = (_scent & ~(0x1F << 5)) | (val & 0x1F); }
	void setMapIndex(uint16 val) { _scent = (_scent & ~(0x1F << 10)) | (val & 0x3F); }
	void setVal(uint16 val) { _scent = val; }

	uint16 toUint16() { return _scent; }
}; // @ SCENT

class Party {
public:
	Party() {
		resetToZero();
	}
	int16 _magicalLightAmount;
	byte _event73Count_ThievesEye;
	byte _event79Count_Footprints;
	int16 _shieldDefense;

	int16 _fireShieldDefense;
	int16 _spellShieldDefense;
	byte _scentCount;
	byte _freezeLifeTicks;
	byte _firstScentIndex;

	byte _lastScentIndex;
	Scent _scents[24]; // if I remember correctly, user defined default constructors are always called
	byte _scentStrengths[24];
	byte _event71Count_Invisibility;
	void resetToZero() {
		_magicalLightAmount = 0;
		_event73Count_ThievesEye = 0;
		_event79Count_Footprints = 0;
		_shieldDefense = 0;

		_fireShieldDefense = 0;
		_spellShieldDefense = 0;
		_scentCount = 0;
		_freezeLifeTicks = 0;
		_firstScentIndex = 0;

		_lastScentIndex = 0;
		for (int16 i = 0; i < 24; ++i) {
			_scents[i].setVal(0);
			_scentStrengths[i] = 0;
		}
		_event71Count_Invisibility = 0;
	}
}; // @  PARTY

enum IconIndice {
	kDMIconIndiceNone = -1, // @ CM1_ICON_NONE
	kDMIconIndiceJunkCompassNorth = 0, // @ C000_ICON_JUNK_COMPASS_NORTH
	kDMIconIndiceJunkCompassWest = 3, // @ C003_ICON_JUNK_COMPASS_WEST
	kDMIconIndiceWeaponTorchUnlit = 4, // @ C004_ICON_WEAPON_TORCH_UNLIT
	kDMIconIndiceWeaponTorchLit = 7, // @ C007_ICON_WEAPON_TORCH_LIT
	kDMIconIndiceJunkWater = 8, // @ C008_ICON_JUNK_WATER
	kDMIconIndiceJunkWaterSkin = 9, // @ C009_ICON_JUNK_WATERSKIN
	kDMIconIndiceJunkJewelSymalUnequipped = 10, // @ C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED
	kDMIconIndiceJunkJewelSymalEquipped = 11, // @ C011_ICON_JUNK_JEWEL_SYMAL_EQUIPPED
	kDMIconIndiceJunkIllumuletUnequipped = 12, // @ C012_ICON_JUNK_ILLUMULET_UNEQUIPPED
	kDMIconIndiceJunkIllumuletEquipped = 13, // @ C013_ICON_JUNK_ILLUMULET_EQUIPPED
	kDMIconIndiceWeaponFlamittEmpty = 14, // @ C014_ICON_WEAPON_FLAMITT_EMPTY
	kDMIconIndiceWeaponEyeOfTimeEmpty = 16, // @ C016_ICON_WEAPON_EYE_OF_TIME_EMPTY
	kDMIconIndiceWeaponStormringEmpty = 18, // @ C018_ICON_WEAPON_STORMRING_EMPTY
	kDMIconIndiceWeaponStaffOfClawsEmpty = 20, // @ C020_ICON_WEAPON_STAFF_OF_CLAWS_EMPTY
	kDMIconIndiceWeaponStaffOfClawsFull = 22, // @ C022_ICON_WEAPON_STAFF_OF_CLAWS_FULL
	kDMIconIndiceWeaponBoltBladeStormEmpty = 23, // @ C023_ICON_WEAPON_BOLT_BLADE_STORM_EMPTY
	kDMIconIndiceWeaponFuryRaBladeEmpty = 25, // @ C025_ICON_WEAPON_FURY_RA_BLADE_EMPTY
	kDMIconIndiceWeaponTheFirestaff = 27, // @ C027_ICON_WEAPON_THE_FIRESTAFF
	kDMIconIndiceWeaponTheFirestaffComplete = 28, // @ C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE
	kDMIconIndiceScrollOpen = 30, // @ C030_ICON_SCROLL_SCROLL_OPEN
	kDMIconIndiceScrollClosed = 31, // @ C031_ICON_SCROLL_SCROLL_CLOSED
	kDMIconIndiceWeaponDagger = 32, // @ C032_ICON_WEAPON_DAGGER
	kDMIconIndiceWeaponDeltaSideSplitter = 38, // @ C038_ICON_WEAPON_DELTA_SIDE_SPLITTER
	kDMIconIndiceWeaponDiamondEdge = 39, // @ C039_ICON_WEAPON_DIAMOND_EDGE
	kDMIconIndiceWeaponVorpalBlade = 40, // @ C040_ICON_WEAPON_VORPAL_BLADE
	kDMIconIndiceWeaponTheInquisitorDragonFang = 41, // @ C041_ICON_WEAPON_THE_INQUISITOR_DRAGON_FANG
	kDMIconIndiceWeaponHardcleaveExecutioner = 43, // @ C043_ICON_WEAPON_HARDCLEAVE_EXECUTIONER
	kDMIconIndiceWeaponMaceOfOrder = 45, // @ C045_ICON_WEAPON_MACE_OF_ORDER
	kDMIconIndiceWeaponArrow = 51, // @ C051_ICON_WEAPON_ARROW
	kDMIconIndiceWeaponSlayer = 52, // @ C052_ICON_WEAPON_SLAYER
	kDMIconIndiceWeaponRock = 54, // @ C054_ICON_WEAPON_ROCK
	kDMIconIndiceWeaponPoisonDart = 55, // @ C055_ICON_WEAPON_POISON_DART
	kDMIconIndiceWeaponThrowingStar = 56, // @ C056_ICON_WEAPON_THROWING_STAR
	kDMIconIndiceWeaponStaff = 58, // @ C058_ICON_WEAPON_STAFF
	kDMIconIndiceWeaponWand = 59, // @ C059_ICON_WEAPON_WAND
	kDMIconIndiceWeaponTeowand = 60, // @ C060_ICON_WEAPON_TEOWAND
	kDMIconIndiceWeaponYewStaff = 61, // @ C061_ICON_WEAPON_YEW_STAFF
	kDMIconIndiceWeaponStaffOfManarStaffOfIrra = 62, // @ C062_ICON_WEAPON_STAFF_OF_MANAR_STAFF_OF_IRRA
	kDMIconIndiceWeaponSnakeStaffCrossOfNeta = 63, // @ C063_ICON_WEAPON_SNAKE_STAFF_CROSS_OF_NETA
	kDMIconIndiceWeaponTheConduitSerpentStaff = 64, // @ C064_ICON_WEAPON_THE_CONDUIT_SERPENT_STAFF
	kDMIconIndiceWeaponDragonSpit = 65, // @ C065_ICON_WEAPON_DRAGON_SPIT
	kDMIconIndiceWeaponSceptreOfLyf = 66, // @ C066_ICON_WEAPON_SCEPTRE_OF_LYF
	kDMIconIndiceArmourCloakOfNight = 81, // @ C081_ICON_ARMOUR_CLOAK_OF_NIGHT
	kDMIconIndiceArmourCrownOfNerra = 104, // @ C104_ICON_ARMOUR_CROWN_OF_NERRA
	kDMIconIndiceArmourElvenBoots = 119, // @ C119_ICON_ARMOUR_ELVEN_BOOTS
	kDMIconIndiceJunkGemOfAges = 120, // @ C120_ICON_JUNK_GEM_OF_AGES
	kDMIconIndiceJunkEkkhardCross = 121, // @ C121_ICON_JUNK_EKKHARD_CROSS
	kDMIconIndiceJunkMoonstone = 122, // @ C122_ICON_JUNK_MOONSTONE
	kDMIconIndiceJunkPendantFeral = 124, // @ C124_ICON_JUNK_PENDANT_FERAL
	kDMIconIndiceJunkBoulder = 128, // @ C128_ICON_JUNK_BOULDER
	kDMIconIndiceJunkRabbitsFoot = 137, // @ C137_ICON_JUNK_RABBITS_FOOT
	kDMIconIndiceArmourDexhelm = 140, // @ C140_ICON_ARMOUR_DEXHELM
	kDMIconIndiceArmourFlamebain = 141, // @ C141_ICON_ARMOUR_FLAMEBAIN
	kDMIconIndiceArmourPowertowers = 142, // @ C142_ICON_ARMOUR_POWERTOWERS
	kDMIconIndiceContainerChestClosed = 144, // @ C144_ICON_CONTAINER_CHEST_CLOSED
	kDMIconIndiceContainerChestOpen = 145, // @ C145_ICON_CONTAINER_CHEST_OPEN
	kDMIconIndiceJunkChampionBones = 147, // @ C147_ICON_JUNK_CHAMPION_BONES
	kDMIconIndicePotionMaPotionMonPotion = 148, // @ C148_ICON_POTION_MA_POTION_MON_POTION
	kDMIconIndicePotionWaterFlask = 163, // @ C163_ICON_POTION_WATER_FLASK
	kDMIconIndiceJunkApple = 168, // @ C168_ICON_JUNK_APPLE
	kDMIconIndiceJunkIronKey = 176, // @ C176_ICON_JUNK_IRON_KEY
	kDMIconIndiceJunkMasterKey = 191, // @ C191_ICON_JUNK_MASTER_KEY
	kDMIconIndiceArmourBootOfSpeed = 194, // @ C194_ICON_ARMOUR_BOOT_OF_SPEED
	kDMIconIndicePotionEmptyFlask = 195, // @ C195_ICON_POTION_EMPTY_FLASK
	kDMIconIndiceJunkZokathra = 197, // @ C197_ICON_JUNK_ZOKATHRA
	kDMIconIndiceActionEmptyHand = 201, // @ C201_ICON_ACTION_ICON_EMPTY_HAND
	kDMIconIndiceEyeNotLooking = 202, // @ C202_ICON_EYE_NOT_LOOKING /* One pixel is different in this bitmap from the eye in C017_GRAPHIC_INVENTORY. This is visible by selecting another champion after clicking the eye */
	kDMIconIndiceEyeLooking = 203, // @ C203_ICON_EYE_LOOKING
	kDMIconIndiceEmptyBox = 204, // @ C204_ICON_EMPTY_BOX
	kDMIconIndiceMouthOpen = 205, // @ C205_ICON_MOUTH_OPEN
	kDMIconIndiceNeck = 208, // @ C208_ICON_NECK
	kDMIconIndiceReadyHand = 212 // @ C212_ICON_READY_HAND
};

enum ChampionIndex {
	kDMChampionNone = -1, // @ CM1_CHAMPION_NONE
	kDMChampionFirst = 0, // @ C00_CHAMPION_FIRST
	kDMChampionSecond = 1,
	kDMChampionThird = 2,
	kDMChampionFourth = 3,
	kDMChampionCloseInventory = 4, // @ C04_CHAMPION_CLOSE_INVENTORY
	kDMChampionSpecialInventory = 5 // @ C05_CHAMPION_SPECIAL_INVENTORY
};

enum ChampionAttribute {
	kDMAttributNone = 0x0000, // @ MASK0x0000_NONE
	kDMAttributeDisableAction = 0x0008, // @ MASK0x0008_DISABLE_ACTION
	kDMAttributeMale = 0x0010, // @ MASK0x0010_MALE
	kDMAttributeNameTitle = 0x0080, // @ MASK0x0080_NAME_TITLE
	kDMAttributeStatistics = 0x0100, // @ MASK0x0100_STATISTICS
	kDMAttributeLoad = 0x0200, // @ MASK0x0200_LOAD
	kDMAttributeIcon = 0x0400, // @ MASK0x0400_ICON
	kDMAttributePanel = 0x0800, // @ MASK0x0800_PANEL
	kDMAttributeStatusBox = 0x1000, // @ MASK0x1000_STATUS_BOX
	kDMAttributeWounds = 0x2000, // @ MASK0x2000_WOUNDS
	kDMAttributeViewport = 0x4000, // @ MASK0x4000_VIEWPORT
	kDMAttributeActionHand = 0x8000 // @ MASK0x8000_ACTION_HAND
};

enum ChampionWound {
	kDMWoundNone = 0x0000, // @ MASK0x0000_NO_WOUND
	kDMWoundReadHand = 0x0001, // @ MASK0x0001_READY_HAND
	kDMWoundActionHand = 0x0002, // @ MASK0x0002_ACTION_HAND
	kDMWoundHead = 0x0004, // @ MASK0x0004_HEAD
	kDMWoundTorso = 0x0008, // @ MASK0x0008_TORSO
	kDMWoundLegs = 0x0010, // @ MASK0x0010_LEGS
	kDMWoundFeet = 0x0020 // @ MASK0x0020_FEET
};

enum ChampionStatType {
	kDMStatLuck = 0, // @ C0_STATISTIC_LUCK
	kDMStatStrength = 1, // @ C1_STATISTIC_STRENGTH
	kDMStatDexterity = 2, // @ C2_STATISTIC_DEXTERITY
	kDMStatWisdom = 3, // @ C3_STATISTIC_WISDOM
	kDMStatVitality = 4, // @ C4_STATISTIC_VITALITY
	kDMStatAntimagic = 5, // @ C5_STATISTIC_ANTIMAGIC
	kDMStatAntifire = 6, // @ C6_STATISTIC_ANTIFIRE
	kDMStatMana = 8 // @ C8_STATISTIC_MANA /* Used as a fake statistic index for objects granting a Mana bonus */
};

enum ChampionStatValue {
	kDMStatMaximum = 0, // @ C0_MAXIMUM
	kDMStatCurrent = 1, // @ C1_CURRENT
	kDMStatMinimum = 2 // @ C2_MINIMUM
};

enum ChampionSkill {
	kDMSkillFighter = 0, // @ C00_SKILL_FIGHTER
	kDMSkillNinja = 1, // @ C01_SKILL_NINJA
	kDMSkillPriest = 2, // @ C02_SKILL_PRIEST
	kDMSkillWizard = 3, // @ C03_SKILL_WIZARD
	kDMSkillSwing = 4, // @ C04_SKILL_SWING
	kDMSkillThrust = 5, // @ C05_SKILL_THRUST
	kDMSkillClub = 6, // @ C06_SKILL_CLUB
	kDMSkillParry = 7, // @ C07_SKILL_PARRY
	kDMSkillSteal = 8, // @ C08_SKILL_STEAL
	kDMSkillFight = 9, // @ C09_SKILL_FIGHT
	kDMSkillThrow = 10, // @ C10_SKILL_THROW
	kDMSkillShoot = 11, // @ C11_SKILL_SHOOT
	kDMSkillIdentify = 12, // @ C12_SKILL_IDENTIFY
	kDMSkillHeal = 13, // @ C13_SKILL_HEAL
	kDMSkillInfluence = 14, // @ C14_SKILL_INFLUENCE
	kDMSkillDefend = 15, // @ C15_SKILL_DEFEND
	kDMSkillFire = 16, // @ C16_SKILL_FIRE
	kDMSkillAir = 17, // @ C17_SKILL_AIR
	kDMSkillEarth = 18, // @ C18_SKILL_EARTH
	kDMSkillWater = 19 // @ C19_SKILL_WATER
};

enum ChampionSlot {
	kDMSlotLeaderHand = -1, // @ CM1_SLOT_LEADER_HAND
	kDMSlotReadyHand = 0, // @ C00_SLOT_READY_HAND
	kDMSlotActionHand = 1, // @ C01_SLOT_ACTION_HAND
	kDMSlotHead = 2, // @ C02_SLOT_HEAD
	kDMSlotTorso = 3, // @ C03_SLOT_TORSO
	kDMSlotLegs = 4, // @ C04_SLOT_LEGS
	kDMSlotFeet = 5, // @ C05_SLOT_FEET
	kDMSlotPouch_2 = 6, // @ C06_SLOT_POUCH_2
	kDMSlotQuiverLine2_1 = 7, // @ C07_SLOT_QUIVER_LINE2_1
	kDMSlotQuiverLine1_2 = 8, // @ C08_SLOT_QUIVER_LINE1_2
	kDMSlotQuiverLine2_2 = 9, // @ C09_SLOT_QUIVER_LINE2_2
	kDMSlotNeck = 10, // @ C10_SLOT_NECK
	kDMSlotPouch1 = 11, // @ C11_SLOT_POUCH_1
	kDMSlotQuiverLine1_1 = 12, // @ C12_SLOT_QUIVER_LINE1_1
	kDMSlotBackpackLine1_1 = 13, // @ C13_SLOT_BACKPACK_LINE1_1
	kDMSlotBackpackLine2_2 = 14, // @ C14_SLOT_BACKPACK_LINE2_2
	kDMSlotBackpackLine2_3 = 15, // @ C15_SLOT_BACKPACK_LINE2_3
	kDMSlotBackpackLine2_4 = 16, // @ C16_SLOT_BACKPACK_LINE2_4
	kDMSlotBackpackLine2_5 = 17, // @ C17_SLOT_BACKPACK_LINE2_5
	kDMSlotBackpackLine2_6 = 18, // @ C18_SLOT_BACKPACK_LINE2_6
	kDMSlotBackpackLine2_7 = 19, // @ C19_SLOT_BACKPACK_LINE2_7
	kDMSlotBackpackLine2_8 = 20, // @ C20_SLOT_BACKPACK_LINE2_8
	kDMSlotBackpackLine2_9 = 21, // @ C21_SLOT_BACKPACK_LINE2_9
	kDMSlotBackpackLine1_2 = 22, // @ C22_SLOT_BACKPACK_LINE1_2
	kDMSlotBackpackLine1_3 = 23, // @ C23_SLOT_BACKPACK_LINE1_3
	kDMSlotBackpackLine1_4 = 24, // @ C24_SLOT_BACKPACK_LINE1_4
	kDMSlotBackpackLine1_5 = 25, // @ C25_SLOT_BACKPACK_LINE1_5
	kDMSlotBackpackLine1_6 = 26, // @ C26_SLOT_BACKPACK_LINE1_6
	kDMSlotBackpackLine1_7 = 27, // @ C27_SLOT_BACKPACK_LINE1_7
	kDMSlotBackpackLine1_8 = 28, // @ C28_SLOT_BACKPACK_LINE1_8
	kDMSlotBackpackLine1_9 = 29, // @ C29_SLOT_BACKPACK_LINE1_9
	kDMSlotChest1 = 30, // @ C30_SLOT_CHEST_1
	kDMSlotChest2 = 31, // @ C31_SLOT_CHEST_2
	kDMSlotChest3 = 32, // @ C32_SLOT_CHEST_3
	kDMSlotChest4 = 33, // @ C33_SLOT_CHEST_4
	kDMSlotChest5 = 34, // @ C34_SLOT_CHEST_5
	kDMSlotChest6 = 35, // @ C35_SLOT_CHEST_6
	kDMSlotChest7 = 36, // @ C36_SLOT_CHEST_7
	kDMSlotChest8 = 37 // @ C37_SLOT_CHEST_8
};

enum ChampionAction {
	kDMActionN = 0, // @ C000_ACTION_N
	kDMActionBlock = 1, // @ C001_ACTION_BLOCK
	kDMActionChop = 2, // @ C002_ACTION_CHOP
	kDMActionX = 3, // @ C003_ACTION_X
	kDMActionBlowHorn = 4, // @ C004_ACTION_BLOW_HORN
	kDMActionFlip = 5, // @ C005_ACTION_FLIP
	kDMActionPunch = 6, // @ C006_ACTION_PUNCH
	kDMActionKick = 7, // @ C007_ACTION_KICK
	kDMActionWarCry = 8, // @ C008_ACTION_WAR_CRY
	kDMActionStab9 = 9, // @ C009_ACTION_STAB
	kDMActionClimbDown = 10, // @ C010_ACTION_CLIMB_DOWN
	kDMActionFreezeLife = 11, // @ C011_ACTION_FREEZE_LIFE
	kDMActionHit = 12, // @ C012_ACTION_HIT
	kDMActionSwing = 13, // @ C013_ACTION_SWING
	kDMActionStab14 = 14, // @ C014_ACTION_STAB
	kDMActionThrust = 15, // @ C015_ACTION_THRUST
	kDMActionJab = 16, // @ C016_ACTION_JAB
	kDMActionParry = 17, // @ C017_ACTION_PARRY
	kDMActionHack = 18, // @ C018_ACTION_HACK
	kDMActionBerzerk = 19, // @ C019_ACTION_BERZERK
	kDMActionFireball = 20, // @ C020_ACTION_FIREBALL
	kDMActionDispel = 21, // @ C021_ACTION_DISPELL
	kDMActionConfuse = 22, // @ C022_ACTION_CONFUSE
	kDMActionLightning = 23, // @ C023_ACTION_LIGHTNING
	kDMActionDisrupt = 24, // @ C024_ACTION_DISRUPT
	kDMActionMelee = 25, // @ C025_ACTION_MELEE
	kDMActionX_C026 = 26, // @ C026_ACTION_X
	kDMActionInvoke = 27, // @ C027_ACTION_INVOKE
	kDMActionSlash = 28, // @ C028_ACTION_SLASH
	kDMActionCleave = 29, // @ C029_ACTION_CLEAVE
	kDMActionBash = 30, // @ C030_ACTION_BASH
	kDMActionStun = 31, // @ C031_ACTION_STUN
	kDMActionShoot = 32, // @ C032_ACTION_SHOOT
	kDMActionSpellshield = 33, // @ C033_ACTION_SPELLSHIELD
	kDMActionFireshield = 34, // @ C034_ACTION_FIRESHIELD
	kDMActionFluxcage = 35, // @ C035_ACTION_FLUXCAGE
	kDMActionHeal = 36, // @ C036_ACTION_HEAL
	kDMActionCalm = 37, // @ C037_ACTION_CALM
	kDMActionLight = 38, // @ C038_ACTION_LIGHT
	kDMActionWindow = 39, // @ C039_ACTION_WINDOW
	kDMActionSpit = 40, // @ C040_ACTION_SPIT
	kDMActionBrandish = 41, // @ C041_ACTION_BRANDISH
	kDMActionThrow = 42, // @ C042_ACTION_THROW
	kDMActionFuse = 43, // @ C043_ACTION_FUSE
	kDMActionNone = 255 // @ C255_ACTION_NONE
};

enum AttackType {
	kDMAttackTypeNormal = 0, // @ C0_ATTACK_NORMAL
	kDMAttackTypeFire = 1, // @ C1_ATTACK_FIRE
	kDMAttackTypeSelf = 2, // @ C2_ATTACK_SELF
	kDMAttackTypeBlunt = 3, // @ C3_ATTACK_BLUNT
	kDMAttackTypeSharp = 4, // @ C4_ATTACK_SHARP
	kDMAttackTypeMagic = 5, // @ C5_ATTACK_MAGIC
	kDMAttackTypePsychic = 6, // @ C6_ATTACK_PSYCHIC
	kDMAttackTypeLightning = 7 // @ C7_ATTACK_LIGHTNING
};

enum SpellCastResult {
	kDMSpellCastFailure = 0, // @ C0_SPELL_CAST_FAILURE
	kDMSpellCastSuccess = 1, // @ C1_SPELL_CAST_SUCCESS
	kDMSpellCastFailureNeedsFlask = 3 // @ C3_SPELL_CAST_FAILURE_NEEDS_FLASK
};

enum SpellFailure {
	kDMFailureNeedsMorePractice = 0, // @ C00_FAILURE_NEEDS_MORE_PRACTICE
	kDMFailureMeaninglessSpell = 1, // @ C01_FAILURE_MEANINGLESS_SPELL
	kDMFailureNeedsFlaskInHand = 10, // @ C10_FAILURE_NEEDS_FLASK_IN_HAND
	kDMFailureNeedsMagicMapInHand = 11 // @ C11_FAILURE_NEEDS_MAGIC_MAP_IN_HAND
};

enum SpellKind {
	kDMSpellKindPotion = 1, // @ C1_SPELL_KIND_POTION
	kDMSpellKindProjectile = 2, // @ C2_SPELL_KIND_PROJECTILE
	kDMSpellKindOther = 3, // @ C3_SPELL_KIND_OTHER
	kDMSpellKindMagicMap = 4 // @ C4_SPELL_KIND_MAGIC_MAP
};

enum SpellType {
	kDMSpellTypeProjectileOpenDoor = 4, // @ C4_SPELL_TYPE_PROJECTILE_OPEN_DOOR
	kDMSpellTypeOtherLight = 0, // @ C0_SPELL_TYPE_OTHER_LIGHT
	kDMSpellTypeOtherDarkness = 1, // @ C1_SPELL_TYPE_OTHER_DARKNESS
	kDMSpellTypeOtherThievesEye = 2, // @ C2_SPELL_TYPE_OTHER_THIEVES_EYE
	kDMSpellTypeOtherInvisibility = 3, // @ C3_SPELL_TYPE_OTHER_INVISIBILITY
	kDMSpellTypeOtherPartyShield = 4, // @ C4_SPELL_TYPE_OTHER_PARTY_SHIELD
	kDMSpellTypeOtherMagicTorch = 5, // @ C5_SPELL_TYPE_OTHER_MAGIC_TORCH
	kDMSpellTypeOtherFootprints = 6, // @ C6_SPELL_TYPE_OTHER_FOOTPRINTS
	kDMSpellTypeOtherZokathra = 7, // @ C7_SPELL_TYPE_OTHER_ZOKATHRA
	kDMSpellTypeOtherFireshield = 8, // @ C8_SPELL_TYPE_OTHER_FIRESHIELD
	kDMSpellTypeMagicMap0 = 0, // @ C0_SPELL_TYPE_MAGIC_MAP
	kDMSpellTypeMagicMap1 = 1, // @ C1_SPELL_TYPE_MAGIC_MAP
	kDMSpellTypeMagicMap2 = 2, // @ C2_SPELL_TYPE_MAGIC_MAP
	kDMSpellTypeMagicMap3 = 3 // @ C3_SPELL_TYPE_MAGIC_MAP
};

#define kDMMaskNoSharpDefense 0x0000 // @ MASK0x0000_DO_NOT_USE_SHARP_DEFENSE
#define kDMMaskSharpDefense 0x8000 // @ MASK0x8000_USE_SHARP_DEFENSE

class Skill {
public:
	int16 _temporaryExperience;
	int32 _experience;

	void resetToZero() { _temporaryExperience = _experience = 0; }
}; // @ SKILL

class Champion {
private:
	DMEngine *_vm;

public:
	uint16 _attributes;
	uint16 _wounds;
	byte _statistics[7][3];
	Thing _slots[30];
	Skill _skills[20];
	char _name[8];
	char _title[20];
	Direction _dir;
	ViewCell _cell;
	ChampionAction _actionIndex;
	uint16 _symbolStep;
	char _symbols[5];
	uint16 _directionMaximumDamageReceived;
	uint16 _maximumDamageReceived;
	uint16 _poisonEventCount;
	int16 _enableActionEventIndex;
	int16 _hideDamageReceivedIndex;
	int16 _currHealth;
	int16 _maxHealth;
	int16 _currStamina;
	int16 _maxStamina;
	int16 _currMana;
	int16 _maxMana;
	int16 _actionDefense;
	int16 _food;
	int16 _water;
	uint16 _load;
	int16 _shieldDefense;
	byte _portrait[928]; // 32 x 29 pixel portrait

	Champion() {}
	void setVm(DMEngine *vm) { _vm = vm; }

	Thing &getSlot(ChampionSlot slot) { return _slots[slot]; }
	void setSlot(ChampionSlot slot, Thing val) { _slots[slot] = val; }

	Skill &getSkill(ChampionSkill skill) { return _skills[skill]; }
	void setSkillExp(ChampionSkill skill,  int32 val) { _skills[skill]._experience = val; }
	void setSkillTempExp(ChampionSkill skill,  int16 val) { _skills[skill]._temporaryExperience= val; }

	byte& getStatistic(ChampionStatType type, ChampionStatValue valType) { return _statistics[type][valType]; }
	void setStatistic(ChampionStatType type, ChampionStatValue valType, byte newVal) { _statistics[type][valType] = newVal; }

	uint16 getAttributes() { return _attributes; }
	uint16 getAttributes(ChampionAttribute flag) { return _attributes & flag; }
	void setAttributeFlag(ChampionAttribute flag, bool value);
	void clearAttributes(ChampionAttribute attribute = kDMAttributNone) { _attributes = attribute; }

	uint16 getWounds() { return _wounds; }
	void setWoundsFlag(ChampionWound flag, bool value);
	uint16 getWoundsFlag(ChampionWound wound) { return _wounds & wound; }
	void clearWounds() { _wounds = kDMWoundNone; }
	void resetSkillsToZero() {
		for (int16 i = 0; i < 20; ++i)
			_skills[i].resetToZero();
	}
	void resetToZero();

}; // @ CHAMPION_INCLUDING_PORTRAIT

class Spell {
public:
	Spell() {}
	Spell(int32 symbols, byte baseSkillReq, byte skillIndex, uint16 attributes)
	: _symbols(symbols), _baseRequiredSkillLevel(baseSkillReq), _skillIndex(skillIndex), _attributes(attributes) {}

	int32 _symbols; /* Most significant byte: 0 (spell definition does not include power symbol) / not 0 (spell definition includes power symbol) */
	byte _baseRequiredSkillLevel;
	byte _skillIndex;
	uint16 _attributes; /* Bits 15-10: Duration, Bits 9-4: Type, Bits 3-0: Kind */

	uint16 getKind() { return _attributes & 0xF; } // @ M67_SPELL_KIND
	uint16 getType() { return (_attributes >> 4) & 0x3F; } // @ M68_SPELL_TYPE
	uint16 getDuration() { return (_attributes >> 10) & 0x3F; } // @ M69_SPELL_DURATION
}; // @ SPELL

class ChampionMan {
	DMEngine *_vm;

	uint16 getChampionPortraitX(uint16 index); // @ M27_PORTRAIT_X
	uint16 getChampionPortraitY(uint16 index); // @ M28_PORTRAIT_Y

	int16 getDecodedValue(char *string, uint16 characterCount); // @ F0279_CHAMPION_GetDecodedValue
	void drawHealthOrStaminaOrManaValue(int16 posy, int16 currVal, int16 maxVal); // @ F0289_CHAMPION_DrawHealthOrStaminaOrManaValue
	uint16 getHandSlotIndex(uint16 slotBoxIndex);// @ M70_HAND_SLOT_INDEX
	int16 _championPendingWounds[4]; // @ G0410_ai_ChampionPendingWounds
	int16 _championPendingDamage[4]; // @ G0409_ai_ChampionPendingDamage

	void initConstants();

public:

	Champion *_champions; // @ K0071_as_Champions
	uint16 _partyChampionCount;	// @ G0305_ui_PartyChampionCount
	bool _partyDead; // @ G0303_B_PartyDead
	Thing _leaderHandObject; // @ G0414_T_LeaderHandObject
	ChampionIndex _leaderIndex;	// @ G0411_i_LeaderIndex
	uint16 _candidateChampionOrdinal; // @ G0299_ui_CandidateChampionOrdinal
	bool _partyIsSleeping; // @ G0300_B_PartyIsSleeping
	uint16 _actingChampionOrdinal; // @ G0506_ui_ActingChampionOrdinal
	IconIndice _leaderHandObjectIconIndex; // @ G0413_i_LeaderHandObjectIconIndex
	bool _leaderEmptyHanded; // @ G0415_B_LeaderEmptyHanded
	Party _party; // @ G0407_s_Party
	ChampionIndex _magicCasterChampionIndex; // @ G0514_i_MagicCasterChampionIndex
	bool _mousePointerHiddenToDrawChangedObjIconOnScreen; // @ G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen

	explicit ChampionMan(DMEngine *vm);
	~ChampionMan();

	ChampionIndex getIndexInCell(int16 cell); // @ F0285_CHAMPION_GetIndexInCell
	bool isLeaderHandObjectThrown(int16 side); // @ F0329_CHAMPION_IsLeaderHandObjectThrown
	bool isObjectThrown(uint16 champIndex, int16 slotIndex, int16 side); // @ F0328_CHAMPION_IsObjectThrown
	void resetDataToStartGame(); // @ F0278_CHAMPION_ResetDataToStartGame
	void addCandidateChampionToParty(uint16 championPortraitIndex); // @ F0280_CHAMPION_AddCandidateChampionToParty
	void drawChampionBarGraphs(ChampionIndex champIndex); // @ F0287_CHAMPION_DrawBarGraphs
	uint16 getStaminaAdjustedValue(Champion *champ, int16 val); // @ F0306_CHAMPION_GetStaminaAdjustedValue
	uint16 getMaximumLoad(Champion *champ); // @ F0309_CHAMPION_GetMaximumLoad
	void drawChampionState(ChampionIndex champIndex); // @ F0292_CHAMPION_DrawState
	uint16 getChampionIconIndex(int16 val, Direction dir); // @ M26_CHAMPION_ICON_INDEX
	void drawHealthStaminaManaValues(Champion *champ); // @ F0290_CHAMPION_DrawHealthStaminaManaValues
	void drawSlot(uint16 champIndex, int16 slotIndex); // @ F0291_CHAMPION_DrawSlot
	void renameChampion(Champion *champ); // @ F0281_CHAMPION_Rename
	uint16 getSkillLevel(int16 champIndex, uint16 skillIndex);// @ F0303_CHAMPION_GetSkillLevel
	Common::String getStringFromInteger(uint16 val, bool padding, uint16 paddingCharCount); // @ F0288_CHAMPION_GetStringFromInteger
	void applyModifiersToStatistics(Champion *champ, int16 slotIndex, int16 iconIndex,
									int16 modifierFactor, Thing thing); // @ F0299_CHAMPION_ApplyObjectModifiersToStatistics
	bool hasObjectIconInSlotBoxChanged(int16 slotBoxIndex, Thing thing); // @ F0295_CHAMPION_HasObjectIconInSlotBoxChanged
	void drawChangedObjectIcons(); // @ F0296_CHAMPION_DrawChangedObjectIcons
	void addObjectInSlot(ChampionIndex champIndex, Thing thing, ChampionSlot slotIndex); // @ F0301_CHAMPION_AddObjectInSlot
	int16 getScentOrdinal(int16 mapX, int16 mapY); // @ F0315_CHAMPION_GetScentOrdinal
	Thing getObjectRemovedFromLeaderHand(); // @ F0298_CHAMPION_GetObjectRemovedFromLeaderHand
	uint16 getStrength(int16 champIndex, int16 slotIndex); // @ F0312_CHAMPION_GetStrength
	Thing getObjectRemovedFromSlot(uint16 champIndex, uint16 slotIndex); // @ F0300_CHAMPION_GetObjectRemovedFromSlot
	void decrementStamina(int16 championIndex, int16 decrement); // @ F0325_CHAMPION_DecrementStamina
	int16 addPendingDamageAndWounds_getDamage(int16 champIndex, int16 attack, int16 allowedWounds,
												   uint16 attackType); // @ F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage
	int16 getWoundDefense(int16 champIndex, uint16 woundIndex); // @ F0313_CHAMPION_GetWoundDefense
	uint16 getStatisticAdjustedAttack(Champion *champ, uint16 statIndex, uint16 attack); // @ F0307_CHAMPION_GetStatisticAdjustedAttack
	void wakeUp(); // @ F0314_CHAMPION_WakeUp
	int16 getThrowingStaminaCost(Thing thing);// @ F0305_CHAMPION_GetThrowingStaminaCost
	void disableAction(uint16 champIndex, uint16 ticks); // @ F0330_CHAMPION_DisableAction
	void addSkillExperience(uint16 champIndex, uint16 skillIndex, uint16 exp);// @ F0304_CHAMPION_AddSkillExperience
	int16 getDamagedChampionCount(uint16 attack, int16 wounds,
												 int16 attackType); // @ F0324_CHAMPION_DamageAll_GetDamagedChampionCount
	int16 getTargetChampionIndex(int16 mapX, int16 mapY, uint16 cell); // @ F0286_CHAMPION_GetTargetChampionIndex
	int16 getDexterity(Champion *champ); // @ F0311_CHAMPION_GetDexterity
	bool isLucky(Champion *champ, uint16 percentage); // @ F0308_CHAMPION_IsLucky
	void championPoison(int16 championIndex, uint16 attack); // @ F0322_CHAMPION_Poison
	void setPartyDirection(int16 dir); // @ F0284_CHAMPION_SetPartyDirection
	void deleteScent(uint16 scentIndex); // @ F0316_CHAMPION_DeleteScent
	void addScentStrength(int16 mapX, int16 mapY, int32 cycleCount); // @ F0317_CHAMPION_AddScentStrength
	void putObjectInLeaderHand(Thing thing, bool setMousePointer); // @ F0297_CHAMPION_PutObjectInLeaderHand
	int16 getMovementTicks(Champion *champ); // @ F0310_CHAMPION_GetMovementTicks
	bool isAmmunitionCompatibleWithWeapon(uint16 champIndex, uint16 weaponSlotIndex,
											   uint16 ammunitionSlotIndex); // @ F0294_CHAMPION_IsAmmunitionCompatibleWithWeapon
	void drawAllChampionStates(); // @ F0293_CHAMPION_DrawAllChampionStates
	void viAltarRebirth(uint16 champIndex); // @ F0283_CHAMPION_ViAltarRebirth
	void clickOnSlotBox(uint16 slotBoxIndex); // @ F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox
	bool isProjectileSpellCast(uint16 champIndex, Thing thing, int16 kineticEnergy, uint16 requiredManaAmount); // @ F0327_CHAMPION_IsProjectileSpellCast
	void championShootProjectile(Champion *champ, Thing thing, int16 kineticEnergy,
									  int16 attack, int16 stepEnergy); // @ F0326_CHAMPION_ShootProjectile
	void applyAndDrawPendingDamageAndWounds(); // @ F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds
	void championKill(uint16 champIndex); // @ F0319_CHAMPION_Kill
	void dropAllObjects(uint16 champIndex); // @ F0318_CHAMPION_DropAllObjects
	void unpoison(int16 champIndex); // @ F0323_CHAMPION_Unpoison
	void applyTimeEffects(); // @ F0331_CHAMPION_ApplyTimeEffects_CPSF
	void savePartyPart2(Common::OutSaveFile *file);
	void loadPartyPart2(Common::InSaveFile *file);

	Box _boxChampionIcons[4];
	Color _championColor[4];
	int16 _lightPowerToLightAmount[16]; // g039_LightPowerToLightAmount
	Box _boxChampionPortrait;
	uint16 _slotMasks[38];
	const char *_baseSkillName[4];
};

}

#endif

#ifndef DM_CHAMPION_H
#define DM_CHAMPION_H

#include "dm.h"
#include "gfx.h"

namespace DM {

enum IconIndice {
	kIconIndiceNone = -1, // @ CM1_ICON_NONE
	kIconIndiceJunkCompassNorth = 0, // @ C000_ICON_JUNK_COMPASS_NORTH
	kIconIndiceJunkCompassWest = 3, // @ C003_ICON_JUNK_COMPASS_WEST
	kIconIndiceWeaponTorchUnlit = 4, // @ C004_ICON_WEAPON_TORCH_UNLIT
	kIconIndiceWeaponTorchLit = 7, // @ C007_ICON_WEAPON_TORCH_LIT
	kIconIndiceJunkWater = 8, // @ C008_ICON_JUNK_WATER
	kIconIndiceJunkWaterSkin = 9, // @ C009_ICON_JUNK_WATERSKIN
	kIconIndiceJunkJewelSymalUnequipped = 10, // @ C010_ICON_JUNK_JEWEL_SYMAL_UNEQUIPPED
	kIconIndiceJunkJewelSymalEquipped = 11, // @ C011_ICON_JUNK_JEWEL_SYMAL_EQUIPPED
	kIconIndiceJunkIllumuletUnequipped = 12, // @ C012_ICON_JUNK_ILLUMULET_UNEQUIPPED
	kIconIndiceJunkIllumuletEquipped = 13, // @ C013_ICON_JUNK_ILLUMULET_EQUIPPED
	kIconIndiceWeaponFlamittEmpty = 14, // @ C014_ICON_WEAPON_FLAMITT_EMPTY
	kIconIndiceWeaponEyeOfTimeEmpty = 16, // @ C016_ICON_WEAPON_EYE_OF_TIME_EMPTY
	kIconIndiceWeaponStormringEmpty = 18, // @ C018_ICON_WEAPON_STORMRING_EMPTY
	kIconIndiceWeaponStaffOfClawsEmpty = 20, // @ C020_ICON_WEAPON_STAFF_OF_CLAWS_EMPTY
	kIconIndiceWeaponStaffOfClawsFull = 22, // @ C022_ICON_WEAPON_STAFF_OF_CLAWS_FULL
	kIconIndiceWeaponBoltBladeStormEmpty = 23, // @ C023_ICON_WEAPON_BOLT_BLADE_STORM_EMPTY
	kIconIndiceWeaponFuryRaBladeEmpty = 25, // @ C025_ICON_WEAPON_FURY_RA_BLADE_EMPTY
	kIconIndiceWeaponTheFirestaff = 27, // @ C027_ICON_WEAPON_THE_FIRESTAFF
	kIconIndiceWeaponTheFirestaffComplete = 28, // @ C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE
	kIconIndiceScrollOpen = 30, // @ C030_ICON_SCROLL_SCROLL_OPEN
	kIconIndiceScrollClosed = 31, // @ C031_ICON_SCROLL_SCROLL_CLOSED
	kIconIndiceWeaponDagger = 32, // @ C032_ICON_WEAPON_DAGGER
	kIconIndiceWeaponDeltaSideSplitter = 38, // @ C038_ICON_WEAPON_DELTA_SIDE_SPLITTER
	kIconIndiceWeaponDiamondEdge = 39, // @ C039_ICON_WEAPON_DIAMOND_EDGE
	kIconIndiceWeaponVorpalBlade = 40, // @ C040_ICON_WEAPON_VORPAL_BLADE
	kIconIndiceWeaponTheInquisitorDragonFang = 41, // @ C041_ICON_WEAPON_THE_INQUISITOR_DRAGON_FANG
	kIconIndiceWeaponHardcleaveExecutioner = 43, // @ C043_ICON_WEAPON_HARDCLEAVE_EXECUTIONER
	kIconIndiceWeaponMaceOfOrder = 45, // @ C045_ICON_WEAPON_MACE_OF_ORDER
	kIconIndiceWeaponArrow = 51, // @ C051_ICON_WEAPON_ARROW
	kIconIndiceWeaponSlayer = 52, // @ C052_ICON_WEAPON_SLAYER
	kIconIndiceWeaponRock = 54, // @ C054_ICON_WEAPON_ROCK
	kIconIndiceWeaponPoisonDart = 55, // @ C055_ICON_WEAPON_POISON_DART
	kIconIndiceWeaponThrowingStar = 56, // @ C056_ICON_WEAPON_THROWING_STAR
	kIconIndiceWeaponStaff = 58, // @ C058_ICON_WEAPON_STAFF
	kIconIndiceWeaponWand = 59, // @ C059_ICON_WEAPON_WAND
	kIconIndiceWeaponTeowand = 60, // @ C060_ICON_WEAPON_TEOWAND
	kIconIndiceWeaponYewStaff = 61, // @ C061_ICON_WEAPON_YEW_STAFF
	kIconIndiceWeaponStaffOfManarStaffOfIrra = 62, // @ C062_ICON_WEAPON_STAFF_OF_MANAR_STAFF_OF_IRRA
	kIconIndiceWeaponSnakeStaffCrossOfNeta = 63, // @ C063_ICON_WEAPON_SNAKE_STAFF_CROSS_OF_NETA
	kIconIndiceWeaponTheConduitSerpentStaff = 64, // @ C064_ICON_WEAPON_THE_CONDUIT_SERPENT_STAFF
	kIconIndiceWeaponDragonSpit = 65, // @ C065_ICON_WEAPON_DRAGON_SPIT
	kIconIndiceWeaponSceptreOfLyf = 66, // @ C066_ICON_WEAPON_SCEPTRE_OF_LYF
	kIconIndiceArmourCloakOfNight = 81, // @ C081_ICON_ARMOUR_CLOAK_OF_NIGHT
	kIconIndiceArmourCrownOfNerra = 104, // @ C104_ICON_ARMOUR_CROWN_OF_NERRA
	kIconIndiceArmourElvenBoots = 119, // @ C119_ICON_ARMOUR_ELVEN_BOOTS
	kIconIndiceJunkGemOfAges = 120, // @ C120_ICON_JUNK_GEM_OF_AGES
	kIconIndiceJunkEkkhardCross = 121, // @ C121_ICON_JUNK_EKKHARD_CROSS
	kIconIndiceJunkMoonstone = 122, // @ C122_ICON_JUNK_MOONSTONE
	kIconIndiceJunkPendantFeral = 124, // @ C124_ICON_JUNK_PENDANT_FERAL
	kIconIndiceJunkBoulder = 128, // @ C128_ICON_JUNK_BOULDER
	kIconIndiceJunkRabbitsFoot = 137, // @ C137_ICON_JUNK_RABBITS_FOOT
	kIconIndiceArmourDexhelm = 140, // @ C140_ICON_ARMOUR_DEXHELM
	kIconIndiceArmourFlamebain = 141, // @ C141_ICON_ARMOUR_FLAMEBAIN
	kIconIndiceArmourPowertowers = 142, // @ C142_ICON_ARMOUR_POWERTOWERS
	kIconIndiceContainerChestClosed = 144, // @ C144_ICON_CONTAINER_CHEST_CLOSED
	kIconIndiceContainerChestOpen = 145, // @ C145_ICON_CONTAINER_CHEST_OPEN
	kIconIndiceJunkChampionBones = 147, // @ C147_ICON_JUNK_CHAMPION_BONES
	kIconIndicePotionMaPotionMonPotion = 148, // @ C148_ICON_POTION_MA_POTION_MON_POTION
	kIconIndicePotionWaterFlask = 163, // @ C163_ICON_POTION_WATER_FLASK
	kIconIndiceJunkApple = 168, // @ C168_ICON_JUNK_APPLE
	kIconIndiceJunkIronKey = 176, // @ C176_ICON_JUNK_IRON_KEY
	kIconIndiceJunkMasterKey = 191, // @ C191_ICON_JUNK_MASTER_KEY
	kIconIndiceArmourBootOfSpeed = 194, // @ C194_ICON_ARMOUR_BOOT_OF_SPEED
	kIconIndicePotionEmptyFlask = 195, // @ C195_ICON_POTION_EMPTY_FLASK
	kIconIndiceJunkZokathra = 197, // @ C197_ICON_JUNK_ZOKATHRA
	kIconIndiceActionEmptyHand = 201, // @ C201_ICON_ACTION_ICON_EMPTY_HAND
	kIconIndiceEyeNotLooking = 202, // @ C202_ICON_EYE_NOT_LOOKING /* One pixel is different in this bitmap from the eye in C017_GRAPHIC_INVENTORY. This is visible by selecting another champion after clicking the eye */
	kIconIndiceEyeLooking = 203, // @ C203_ICON_EYE_LOOKING
	kIconIndiceEmptyBox = 204, // @ C204_ICON_EMPTY_BOX
	kIconIndiceMouthOpen = 205, // @ C205_ICON_MOUTH_OPEN
	kIconIndiceNeck = 208, // @ C208_ICON_NECK
	kIconIndiceReadyHand = 212 // @ C212_ICON_READY_HAND
};

enum ChampionIndex {
	kChampionNone = -1, // @ CM1_CHAMPION_NONE
	kChampionFirst = 0, // @ C00_CHAMPION_FIRST
	kChampionSecond = 1,
	kChampionThird = 2,
	kChampionFourth = 3,
	kChmpionCloseInventory = 4, // @ C04_CHAMPION_CLOSE_INVENTORY
	kChampionSpecialInventory = 5 // @ C05_CHAMPION_SPECIAL_INVENTORY
};

enum ChampionAttribute {
	kChampionAttributNone = 0x0000, // @ MASK0x0000_NONE
	kChampionAttributeDisableAction = 0x0008, // @ MASK0x0008_DISABLE_ACTION
	kChampionAttributeMale = 0x0010, // @ MASK0x0010_MALE
	kChampionAttributeNameTitle = 0x0080, // @ MASK0x0080_NAME_TITLE
	kChampionAttributeStatistics = 0x0100, // @ MASK0x0100_STATISTICS
	kChampionAttributeLoad = 0x0200, // @ MASK0x0200_LOAD
	kChampionAttributeIcon = 0x0400, // @ MASK0x0400_ICON
	kChampionAttributePanel = 0x0800, // @ MASK0x0800_PANEL
	kChampionAttributeStatusBox = 0x1000, // @ MASK0x1000_STATUS_BOX
	kChampionAttributeWounds = 0x2000, // @ MASK0x2000_WOUNDS
	kChampionAttributeViewport = 0x4000, // @ MASK0x4000_VIEWPORT
	kChampionAttributeActionHand = 0x8000 // @ MASK0x8000_ACTION_HAND
};


enum ChampionWound {
	kChampionWoundNone = 0x0000, // @ MASK0x0000_NO_WOUND
	kChampionWoundReadHand = 0x0001, // @ MASK0x0001_READY_HAND
	kChampionWoundActionHand = 0x0002, // @ MASK0x0002_ACTION_HAND
	kChampionWoundHead = 0x0004, // @ MASK0x0004_HEAD
	kChampionWoundTorso = 0x0008, // @ MASK0x0008_TORSO
	kChampionWoundLegs = 0x0010, // @ MASK0x0010_LEGS
	kChampionWoundFeet = 0x0020 // @ MASK0x0020_FEET
};


enum ChampionStatisticType {
	kChampionStatLuck = 0, // @ C0_STATISTIC_LUCK
	kChampionStatStrength = 1, // @ C1_STATISTIC_STRENGTH
	kChampionStatDexterity = 2, // @ C2_STATISTIC_DEXTERITY
	kChampionStatWisdom = 3, // @ C3_STATISTIC_WISDOM
	kChampionStatVitality = 4, // @ C4_STATISTIC_VITALITY
	kChampionStatAntimagic = 5, // @ C5_STATISTIC_ANTIMAGIC
	kChampionStatAntifire = 6, // @ C6_STATISTIC_ANTIFIRE
	kChampionStatMana = 8 // @ C8_STATISTIC_MANA /* Used as a fake statistic index for objects granting a Mana bonus */
};

enum ChampionStatisticValue {
	kChampionStatMaximum = 0, // @ C0_MAXIMUM
	kChampionStatCurrent = 1, // @ C1_CURRENT
	kChampionStatMinimum = 2 // @ C2_MINIMUM
};

enum ChampionSkill {
	kChampionSkillFighter = 0, // @ C00_SKILL_FIGHTER
	kChampionSkillNinja = 1, // @ C01_SKILL_NINJA
	kChampionSkillPriest = 2, // @ C02_SKILL_PRIEST
	kChampionSkillWizard = 3, // @ C03_SKILL_WIZARD
	kChampionSkillSwing = 4, // @ C04_SKILL_SWING
	kChampionSkillThrust = 5, // @ C05_SKILL_THRUST
	kChampionSkillClub = 6, // @ C06_SKILL_CLUB
	kChampionSkillParry = 7, // @ C07_SKILL_PARRY
	kChampionSkillSteal = 8, // @ C08_SKILL_STEAL
	kChampionSkillFight = 9, // @ C09_SKILL_FIGHT
	kChampionSkillThrow = 10, // @ C10_SKILL_THROW
	kChampionSkillShoot = 11, // @ C11_SKILL_SHOOT
	kChampionSkillIdentify = 12, // @ C12_SKILL_IDENTIFY
	kChampionSkillHeal = 13, // @ C13_SKILL_HEAL
	kChampionSkillInfluence = 14, // @ C14_SKILL_INFLUENCE
	kChampionSkillDefend = 15, // @ C15_SKILL_DEFEND
	kChampionSkillFire = 16, // @ C16_SKILL_FIRE
	kChampionSkillAir = 17, // @ C17_SKILL_AIR
	kChampionSkillEarth = 18, // @ C18_SKILL_EARTH
	kChampionSkillWater = 19 // @ C19_SKILL_WATER
};

enum ChampionSlot {
	kChampionSlotLeaderHand = -1, // @ CM1_SLOT_LEADER_HAND
	kChampionSlotReadyHand = 0, // @ C00_SLOT_READY_HAND
	kChampionSlotActionHand = 1, // @ C01_SLOT_ACTION_HAND
	kChampionSlotHead = 2, // @ C02_SLOT_HEAD
	kChampionSlotTorso = 3, // @ C03_SLOT_TORSO
	kChampionSlotLegs = 4, // @ C04_SLOT_LEGS
	kChampionSlotFeet = 5, // @ C05_SLOT_FEET
	kChampionSlotPouch_2 = 6, // @ C06_SLOT_POUCH_2
	kChampionSlotQuiverLine_2_1 = 7, // @ C07_SLOT_QUIVER_LINE2_1
	kChampionSlotQuiverLine_1_2 = 8, // @ C08_SLOT_QUIVER_LINE1_2
	kChampionSlotQuiverLine_2_2 = 9, // @ C09_SLOT_QUIVER_LINE2_2
	kChampionSlotNeck = 10, // @ C10_SLOT_NECK
	kChampionSlotPouch_1 = 11, // @ C11_SLOT_POUCH_1
	kChampionSlotQuiverLine_1_1 = 12, // @ C12_SLOT_QUIVER_LINE1_1
	kChampionSlotBackpackLine_1_1 = 13, // @ C13_SLOT_BACKPACK_LINE1_1
	kChampionSlotBackpackLine_2_2 = 14, // @ C14_SLOT_BACKPACK_LINE2_2
	kChampionSlotBackpackLine_2_3 = 15, // @ C15_SLOT_BACKPACK_LINE2_3
	kChampionSlotBackpackLine_2_4 = 16, // @ C16_SLOT_BACKPACK_LINE2_4
	kChampionSlotBackpackLine_2_5 = 17, // @ C17_SLOT_BACKPACK_LINE2_5
	kChampionSlotBackpackLine_2_6 = 18, // @ C18_SLOT_BACKPACK_LINE2_6
	kChampionSlotBackpackLine_2_7 = 19, // @ C19_SLOT_BACKPACK_LINE2_7
	kChampionSlotBackpackLine_2_8 = 20, // @ C20_SLOT_BACKPACK_LINE2_8
	kChampionSlotBackpackLine_2_9 = 21, // @ C21_SLOT_BACKPACK_LINE2_9
	kChampionSlotBackpackLine_1_2 = 22, // @ C22_SLOT_BACKPACK_LINE1_2
	kChampionSlotBackpackLine_1_3 = 23, // @ C23_SLOT_BACKPACK_LINE1_3
	kChampionSlotBackpackLine_1_4 = 24, // @ C24_SLOT_BACKPACK_LINE1_4
	kChampionSlotBackpackLine_1_5 = 25, // @ C25_SLOT_BACKPACK_LINE1_5
	kChampionSlotBackpackLine_1_6 = 26, // @ C26_SLOT_BACKPACK_LINE1_6
	kChampionSlotBackpackLine_1_7 = 27, // @ C27_SLOT_BACKPACK_LINE1_7
	kChampionSlotBackpackLine_1_8 = 28, // @ C28_SLOT_BACKPACK_LINE1_8
	kChampionSlotBackpackLine_1_9 = 29, // @ C29_SLOT_BACKPACK_LINE1_9
	kChampionSlotChest_1 = 30, // @ C30_SLOT_CHEST_1
	kChampionSlotChest_2 = 31, // @ C31_SLOT_CHEST_2
	kChampionSlotChest_3 = 32, // @ C32_SLOT_CHEST_3
	kChampionSlotChest_4 = 33, // @ C33_SLOT_CHEST_4
	kChampionSlotChest_5 = 34, // @ C34_SLOT_CHEST_5
	kChampionSlotChest_6 = 35, // @ C35_SLOT_CHEST_6
	kChampionSlotChest_7 = 36, // @ C36_SLOT_CHEST_7
	kChampionSlotChest_8 = 37 // @ C37_SLOT_CHEST_8
};

enum ChampionAction {
	kChampionActionN = 0, // @ C000_ACTION_N
	kChampionActionBlock = 1, // @ C001_ACTION_BLOCK
	kChampionActionChop = 2, // @ C002_ACTION_CHOP
	kChampionActionX_C003 = 3, // @ C003_ACTION_X
	kChampionActionBlowHorn = 4, // @ C004_ACTION_BLOW_HORN
	kChampionActionFlip = 5, // @ C005_ACTION_FLIP
	kChampionActionPunch = 6, // @ C006_ACTION_PUNCH
	kChampionActionKick = 7, // @ C007_ACTION_KICK
	kChampionActionWarCry = 8, // @ C008_ACTION_WAR_CRY
	kChampionActionStab_C009 = 9, // @ C009_ACTION_STAB
	kChampionActionClimbDown = 10, // @ C010_ACTION_CLIMB_DOWN
	kChampionActionFreezeLife = 11, // @ C011_ACTION_FREEZE_LIFE
	kChampionActionHit = 12, // @ C012_ACTION_HIT
	kChampionActionSwing = 13, // @ C013_ACTION_SWING
	kChampionActionStab_C014 = 14, // @ C014_ACTION_STAB
	kChampionActionThrust = 15, // @ C015_ACTION_THRUST
	kChampionActionJab = 16, // @ C016_ACTION_JAB
	kChampionActionParry = 17, // @ C017_ACTION_PARRY
	kChampionActionHack = 18, // @ C018_ACTION_HACK
	kChampionActionBerzerk = 19, // @ C019_ACTION_BERZERK
	kChampionActionFireball = 20, // @ C020_ACTION_FIREBALL
	kChampionActionDispel = 21, // @ C021_ACTION_DISPELL
	kChampionActionConfuse = 22, // @ C022_ACTION_CONFUSE
	kChampionActionLightning = 23, // @ C023_ACTION_LIGHTNING
	kChampionActionDisrupt = 24, // @ C024_ACTION_DISRUPT
	kChampionActionMelee = 25, // @ C025_ACTION_MELEE
	kChampionActionX_C026 = 26, // @ C026_ACTION_X
	kChampionActionInvoke = 27, // @ C027_ACTION_INVOKE
	kChampionActionSlash = 28, // @ C028_ACTION_SLASH
	kChampionActionCleave = 29, // @ C029_ACTION_CLEAVE
	kChampionActionBash = 30, // @ C030_ACTION_BASH
	kChampionActionStun = 31, // @ C031_ACTION_STUN
	kChampionActionShoot = 32, // @ C032_ACTION_SHOOT
	kChampionActionSpellshield = 33, // @ C033_ACTION_SPELLSHIELD
	kChampionActionFireshield = 34, // @ C034_ACTION_FIRESHIELD
	kChampionActionFluxcage = 35, // @ C035_ACTION_FLUXCAGE
	kChampionActionHeal = 36, // @ C036_ACTION_HEAL
	kChampionActionCalm = 37, // @ C037_ACTION_CALM
	kChampionActionLight = 38, // @ C038_ACTION_LIGHT
	kChampionActionWindow = 39, // @ C039_ACTION_WINDOW
	kChampionActionSpit = 40, // @ C040_ACTION_SPIT
	kChampionActionBrandish = 41, // @ C041_ACTION_BRANDISH
	kChampionActionThrow = 42, // @ C042_ACTION_THROW
	kChampionActionFuse = 43, // @ C043_ACTION_FUSE
	kChampionActionNone = 255 // @ C255_ACTION_NONE
};


class Skill {
	int _temporaryExperience;
	long _experience;
}; // @ SKILL

class Champion {
	Thing _slots[30];
	Skill _skills[20];
	uint16 _attributes;
	byte _statistics[7][3];
	uint16 _wounds;
public:
	char _name[8];
	char _title[20];
	direction _dir;
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
	byte _portrait[464]; // 32 x 29 pixel portrait

	Thing getSlot(ChampionSlot slot) { return _slots[slot]; }
	void setSlot(ChampionSlot slot, Thing val) { _slots[slot] = val; }

	Skill getSkill(ChampionSkill skill) { return _skills[skill]; }
	void setSkill(ChampionSkill skill, Skill val) { _skills[skill] = val; }

	byte getStatistic(ChampionStatisticType type, ChampionStatisticValue valType) { return _statistics[type][valType]; }
	void setStatistic(ChampionStatisticType type, ChampionStatisticValue valType, byte newVal) { _statistics[type][valType] = newVal; }

	uint16 getAttributes() { return _attributes; }
	void setAttributeFlag(ChampionAttribute flag, bool value) {
		if (value) {
			_attributes |= flag;
		} else {
			_attributes &= ~flag;
		}
	}
	void clearAttributes() { _attributes = kChampionAttributNone; }

	uint16 getWounds() { return _wounds; }
	void setWoundsFlag(ChampionWound flag, bool value) {
		if (value) {
			_wounds |= flag;
		} else {
			_wounds &= ~flag;
		}
	}
	void clearWounds() { _wounds = kChampionWoundNone; }
}; // @ CHAMPION_INCLUDING_PORTRAIT

class ChampionMan {
	DMEngine *_vm;

	uint16 getChampionPortraitX(uint16 index); // @ M27_PORTRAIT_X
	uint16 getChampionPortraitY(uint16 index); // @ M28_PORTRAIT_Y

	ChampionIndex getIndexInCell(ViewCell cell); // @ F0285_CHAMPION_GetIndexInCell
	int16 getDecodedValue(char *string, uint16 characterCount); // @ F0279_CHAMPION_GetDecodedValue
public:
	Champion _champions[4];
	uint16 _partyChampionCount;	// @ G0305_ui_PartyChampionCount
	bool _partyDead; // @ G0303_B_PartyDead
	Thing _leaderHand; // @ G0414_T_LeaderHandObject
	ChampionIndex _leaderIndex;	// @ G0411_i_LeaderIndex
	uint16 _candidateChampionOrdinal; // @ G0299_ui_CandidateChampionOrdinal
	bool _partyIsSleeping; // @ G0300_B_PartyIsSleeping
	uint16 _actingChampionOrdinal; // @ G0506_ui_ActingChampionOrdinal
	IconIndice _leaderHandObjectIconIndex; // @ G0413_i_LeaderHandObjectIconIndex
	bool _leaderEmptyHanded; // @ G0415_B_LeaderEmptyHanded

	ChampionMan(DMEngine *vm);
	void resetDataToStartGame(); // @ F0278_CHAMPION_ResetDataToStartGame
};

}

#endif

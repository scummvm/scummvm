static const char *const kEoB2ChargenStrings1AmigaEnglish[9] = {
	"Your party is\rcomplete. Select\rthe PLAY button\ror press 'P' to\rstart the game.",
	"          ",
	"AC\rHP\rLVL",
	"%s\r%d\r%d\r%d\r%d\r%d",
	"%d\r%d",
	"%d",
	"%d/%d",
	"%d/%d/%d",
	"Select the box of\rthe character you\rwish to create or\rview."
};

static const StringListProvider kEoB2ChargenStrings1AmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenStrings1AmigaEnglish), kEoB2ChargenStrings1AmigaEnglish };

static const char *const kEoB2ChargenStrings2AmigaEnglish[12] = {
	"%s",
	"%d",
	"%s",
	"%d",
	"%d",
	"%d",
	"%s",
	"%d",
	"SELECT RACE:",
	"SELECT CLASS:",
	"SELECT ALIGNMENT:",
	"Name:"
};

static const StringListProvider kEoB2ChargenStrings2AmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenStrings2AmigaEnglish), kEoB2ChargenStrings2AmigaEnglish };

static const char *const kEoB2ChargenStatStringsAmigaEnglish[12] = {
	"STR",
	"INT",
	"WIS",
	"DEX",
	"CON",
	"CHA",
	"STRENGTH",
	"INTELLIGENCE",
	"WISDOM",
	"DEXTERITY",
	"CONSTITUTION",
	"CHARISMA"
};

static const StringListProvider kEoB2ChargenStatStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenStatStringsAmigaEnglish), kEoB2ChargenStatStringsAmigaEnglish };

static const char *const kEoB2ChargenRaceSexStringsAmigaEnglish[12] = {
	"HUMAN MALE",
	"HUMAN FEMALE",
	"ELF MALE",
	"ELF FEMALE",
	"HALF-ELF MALE",
	"HALF-ELF FEMALE",
	"DWARF MALE",
	"DWARF FEMALE",
	"GNOME MALE",
	"GNOME FEMALE",
	"HALFLING MALE",
	"HALFLING FEMALE"
};

static const StringListProvider kEoB2ChargenRaceSexStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenRaceSexStringsAmigaEnglish), kEoB2ChargenRaceSexStringsAmigaEnglish };

static const char *const kEoB2ChargenClassStringsAmigaEnglish[21] = {
	"FIGHTER",
	"RANGER",
	"PALADIN",
	"MAGE",
	"CLERIC",
	"THIEF",
	"FIGHTER/CLERIC",
	"FIGHTER/THIEF",
	"FIGHTER/MAGE",
	"FIGHTER/MAGE/THIEF",
	"THIEF/MAGE",
	"CLERIC/THIEF",
	"FIGHTER/CLERIC/MAGE",
	"RANGER/CLERIC",
	"CLERIC/MAGE",
	"FIGHTER",
	"MAGE",
	"CLERIC",
	"THIEF",
	"PALADIN",
	"RANGER"
};

static const StringListProvider kEoB2ChargenClassStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenClassStringsAmigaEnglish), kEoB2ChargenClassStringsAmigaEnglish };

static const char *const kEoB2ChargenAlignmentStringsAmigaEnglish[9] = {
	"LAWFUL GOOD",
	"NEUTRAL GOOD",
	"CHAOTIC GOOD",
	"LAWFUL NEUTRAL",
	"TRUE NEUTRAL",
	"CHAOTIC NEUTRAL",
	"LAWFUL EVIL",
	"NEUTRAL EVIL",
	"CHAOTIC EVIL"
};

static const StringListProvider kEoB2ChargenAlignmentStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenAlignmentStringsAmigaEnglish), kEoB2ChargenAlignmentStringsAmigaEnglish };

static const char *const kEoB2ChargenEnterGameStringsAmigaEnglish[1] = {
	"  Entering game.\r  Please wait."
};

static const StringListProvider kEoB2ChargenEnterGameStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ChargenEnterGameStringsAmigaEnglish), kEoB2ChargenEnterGameStringsAmigaEnglish };

static const char *const kEoB2PryDoorStringsAmigaEnglish[8] = {
	"\r",
	"You are not capable of forcing the door.\r",
	"\x06\x04""You force the door.\r",
	"\x06\x06""You try to force the door but fail.\r",
	"You can't put that item there.\r",
	"The item is too large to fit.\r",
	"No one is able to pry this door open.\r",
	"\r"
};

static const StringListProvider kEoB2PryDoorStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2PryDoorStringsAmigaEnglish), kEoB2PryDoorStringsAmigaEnglish };

static const char *const kEoB2WarningStringsAmigaEnglish[4] = {
	"You can't go that way.\r",
	"%s isn't capable of eating food!\r",
	"That food is rotten!  You don't want to eat that!\r",
	"You may only eat food!\r"
};

static const StringListProvider kEoB2WarningStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2WarningStringsAmigaEnglish), kEoB2WarningStringsAmigaEnglish };

static const char *const kEoB2ItemSuffixStringsRingsAmigaEnglish[4] = {
	"Adornment",
	"Wizardry",
	"Sustenance",
	"Feather Fall"
};

static const StringListProvider kEoB2ItemSuffixStringsRingsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsRingsAmigaEnglish), kEoB2ItemSuffixStringsRingsAmigaEnglish };

static const char *const kEoB2ItemSuffixStringsPotionsAmigaEnglish[8] = {
	"Giant Strength",
	"Healing",
	"Extra Healing",
	"Poison",
	"Vitality",
	"Speed",
	"Invisibility",
	"Cure Poison"
};

static const StringListProvider kEoB2ItemSuffixStringsPotionsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsPotionsAmigaEnglish), kEoB2ItemSuffixStringsPotionsAmigaEnglish };

static const char *const kEoB2ItemSuffixStringsWandsAmigaEnglish[8] = {
	"Stick",
	"Lightning",
	"Frost",
	"Curing",
	"Fireball",
	"Starfire",
	"Magic Missile",
	"Dispel Magic"
};

static const StringListProvider kEoB2ItemSuffixStringsWandsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ItemSuffixStringsWandsAmigaEnglish), kEoB2ItemSuffixStringsWandsAmigaEnglish };

static const char *const kEoB2RipItemStringsAmigaEnglish[3] = {
	"%s has lost her ",
	"%s has lost his ",
	".\r"
};

static const StringListProvider kEoB2RipItemStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2RipItemStringsAmigaEnglish), kEoB2RipItemStringsAmigaEnglish };

static const char *const kEoB2CursedStringAmigaEnglish[1] = {
	"%d Cursed %s"
};

static const StringListProvider kEoB2CursedStringAmigaEnglishProvider = { ARRAYSIZE(kEoB2CursedStringAmigaEnglish), kEoB2CursedStringAmigaEnglish };

static const char *const kEoB2MagicObjectStringsAmigaEnglish[5] = {
	"Mage Scroll",
	"Cleric Scroll",
	"Ring",
	"Potion",
	"Wand"
};

static const StringListProvider kEoB2MagicObjectStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicObjectStringsAmigaEnglish), kEoB2MagicObjectStringsAmigaEnglish };

static const char *const kEoB2MagicObjectString5AmigaEnglish[1] = {
	"Stick"
};

static const StringListProvider kEoB2MagicObjectString5AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicObjectString5AmigaEnglish), kEoB2MagicObjectString5AmigaEnglish };

static const char *const kEoB2PatternSuffixAmigaEnglish[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternSuffixAmigaEnglishProvider = { ARRAYSIZE(kEoB2PatternSuffixAmigaEnglish), kEoB2PatternSuffixAmigaEnglish };

static const char *const kEoB2PatternGrFix1AmigaEnglish[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternGrFix1AmigaEnglishProvider = { ARRAYSIZE(kEoB2PatternGrFix1AmigaEnglish), kEoB2PatternGrFix1AmigaEnglish };

static const char *const kEoB2PatternGrFix2AmigaEnglish[1] = {
	"%s of %s"
};

static const StringListProvider kEoB2PatternGrFix2AmigaEnglishProvider = { ARRAYSIZE(kEoB2PatternGrFix2AmigaEnglish), kEoB2PatternGrFix2AmigaEnglish };

static const char *const kEoB2ValidateArmorStringAmigaEnglish[1] = {
	"%s can't wear that type of armor.\r"
};

static const StringListProvider kEoB2ValidateArmorStringAmigaEnglishProvider = { ARRAYSIZE(kEoB2ValidateArmorStringAmigaEnglish), kEoB2ValidateArmorStringAmigaEnglish };

static const char *const kEoB2ValidateCursedStringAmigaEnglish[1] = {
	"%s cannot release the weapon!  It is cursed!\r"
};

static const StringListProvider kEoB2ValidateCursedStringAmigaEnglishProvider = { ARRAYSIZE(kEoB2ValidateCursedStringAmigaEnglish), kEoB2ValidateCursedStringAmigaEnglish };

static const char *const kEoB2ValidateNoDropStringAmigaEnglish[1] = {
	"You can't put that item there.\r"
};

static const StringListProvider kEoB2ValidateNoDropStringAmigaEnglishProvider = { ARRAYSIZE(kEoB2ValidateNoDropStringAmigaEnglish), kEoB2ValidateNoDropStringAmigaEnglish };

static const char *const kEoB2PotionStringsAmigaEnglish[2] = {
	"poisoned",
	"%s feels %s!\r"
};

static const StringListProvider kEoB2PotionStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2PotionStringsAmigaEnglish), kEoB2PotionStringsAmigaEnglish };

static const char *const kEoB2WandStringsAmigaEnglish[1] = {
	"The wand has no apparent magical effect\r"
};

static const StringListProvider kEoB2WandStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2WandStringsAmigaEnglish), kEoB2WandStringsAmigaEnglish };

static const char *const kEoB2ItemMisuseStringsAmigaEnglish[3] = {
	"%s can not use this item.\r",
	"This item automatically used when worn.\r",
	"This item is not used in this way.\r"
};

static const StringListProvider kEoB2ItemMisuseStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2ItemMisuseStringsAmigaEnglish), kEoB2ItemMisuseStringsAmigaEnglish };

static const char *const kEoB2TakenStringsAmigaEnglish[1] = {
	" taken.\r"
};

static const StringListProvider kEoB2TakenStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2TakenStringsAmigaEnglish), kEoB2TakenStringsAmigaEnglish };

static const char *const kEoB2PotionEffectStringsAmigaEnglish[8] = {
	"much stronger",
	"better",
	"much better",
	"ill for a moment",
	"satiated",
	"fast and agile",
	"transparent",
	"better"
};

static const StringListProvider kEoB2PotionEffectStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2PotionEffectStringsAmigaEnglish), kEoB2PotionEffectStringsAmigaEnglish };

static const char *const kEoB2YesNoStringsAmigaEnglish[2] = {
	"yes",
	"no"
};

static const StringListProvider kEoB2YesNoStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2YesNoStringsAmigaEnglish), kEoB2YesNoStringsAmigaEnglish };

static const char *const kEoB2MoreStringsAmigaEnglish[1] = {
	"MORE"
};

static const StringListProvider kEoB2MoreStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MoreStringsAmigaEnglish), kEoB2MoreStringsAmigaEnglish };

static const char *const kEoB2NpcMaxStringsAmigaEnglish[1] = {
	"You may only have six characters in your party.  Select the one you wish to drop."
};

static const StringListProvider kEoB2NpcMaxStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2NpcMaxStringsAmigaEnglish), kEoB2NpcMaxStringsAmigaEnglish };

static const char *const kEoB2OkStringsAmigaEnglish[1] = {
	"OK"
};

static const StringListProvider kEoB2OkStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2OkStringsAmigaEnglish), kEoB2OkStringsAmigaEnglish };

static const char *const kEoB2NpcJoinStringsAmigaEnglish[1] = {
	"%s joins the party.\r"
};

static const StringListProvider kEoB2NpcJoinStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2NpcJoinStringsAmigaEnglish), kEoB2NpcJoinStringsAmigaEnglish };

static const char *const kEoB2CancelStringsAmigaEnglish[1] = {
	"CANCEL"
};

static const StringListProvider kEoB2CancelStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2CancelStringsAmigaEnglish), kEoB2CancelStringsAmigaEnglish };

static const char *const kEoB2AbortStringsAmigaEnglish[1] = {
	"ABORT"
};

static const StringListProvider kEoB2AbortStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2AbortStringsAmigaEnglish), kEoB2AbortStringsAmigaEnglish };

static const char *const kEoB2MenuStringsMainAmigaEnglish[8] = {
	"Select Option:",
	"Rest Party",
	"Memorize Spells",
	"Pray for Spells",
	"Scribe Scrolls",
	"Preferences",
	"Game Options",
	"Exit"
};

static const StringListProvider kEoB2MenuStringsMainAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsMainAmigaEnglish), kEoB2MenuStringsMainAmigaEnglish };

static const char *const kEoB2MenuStringsSaveLoadAmigaEnglish[8] = {
	"Load Game",
	"Save Game",
	"Drop Character",
	"Quit Game",
	"Game Options:",
	"\r   Game saved.",
	"Attempts to save\ryour game have\rfailed!",
	"Cannot load your\rsave game.  The\rfile may be corrupt!"
};

static const StringListProvider kEoB2MenuStringsSaveLoadAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsSaveLoadAmigaEnglish), kEoB2MenuStringsSaveLoadAmigaEnglish };

static const char *const kEoB2MenuStringsOnOffAmigaEnglish[2] = {
	"ON",
	"OFF"
};

static const StringListProvider kEoB2MenuStringsOnOffAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsOnOffAmigaEnglish), kEoB2MenuStringsOnOffAmigaEnglish };

static const char *const kEoB2MenuStringsSpellsAmigaEnglish[17] = {
	"\r\r Select a character\r from your party who\r would like to learn\r spells.",
	"\r Your Paladin is \r too low a level\r for spells.",
	"\r\r The Mage has no\r Spell Book!",
	"\r\r\r Select a character\r from your party\r who would like to\r pray for spells.",
	"\r No party members\r are capable of\r praying for new\r spells.",
	"\r No party members\r are capable of\r memorizing new\r spells.",
	" An unconscious\r or dead Mage\r cannot memorize\r spells.",
	" An unconscious\r or dead Cleric\r cannot pray for\r spells.",
	"1",
	"2",
	"3",
	"4",
	"5",
	"Clear",
	"Spells Available:",
	"Yes",
	"No"
};

static const StringListProvider kEoB2MenuStringsSpellsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsSpellsAmigaEnglish), kEoB2MenuStringsSpellsAmigaEnglish };

static const char *const kEoB2MenuStringsRestAmigaEnglish[5] = {
	"\rWill your healers\rheal the party?",
	" Someone is still\r injured. Rest\r until healed?",
	"Resting party.",
	"\r All characters\r are fully\r rested.",
	" Your party needs\r to rest to gain\r spells."
};

static const StringListProvider kEoB2MenuStringsRestAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsRestAmigaEnglish), kEoB2MenuStringsRestAmigaEnglish };

static const char *const kEoB2MenuStringsDropAmigaEnglish[1] = {
	" You cannot have\r less than four\r characters."
};

static const StringListProvider kEoB2MenuStringsDropAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsDropAmigaEnglish), kEoB2MenuStringsDropAmigaEnglish };

static const char *const kEoB2MenuStringsExitAmigaEnglish[1] = {
	" Are you sure you\r wish to exit the\r game?"
};

static const StringListProvider kEoB2MenuStringsExitAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsExitAmigaEnglish), kEoB2MenuStringsExitAmigaEnglish };

static const char *const kEoB2MenuStringsStarveAmigaEnglish[1] = {
	" Your party is\r starving. Do you\r wish to continue\r resting?"
};

static const StringListProvider kEoB2MenuStringsStarveAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsStarveAmigaEnglish), kEoB2MenuStringsStarveAmigaEnglish };

static const char *const kEoB2MenuStringsScribeAmigaEnglish[5] = {
	"Select the scroll(s)\ryou wish to scribe.",
	"\r\r\r Select a Mage\r from your party\r who would like to\r scribe spells.",
	" You don't have\r any scolls to be\r scribed.",
	" You don't have\r any scrolls that\r this Mage needs.",
	"\r You don't have\r any Mage able to\r scribe scrolls."
};

static const StringListProvider kEoB2MenuStringsScribeAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsScribeAmigaEnglish), kEoB2MenuStringsScribeAmigaEnglish };

static const char *const kEoB2MenuStringsDrop2AmigaEnglish[3] = {
	" Select the\r character you\r wish to drop.",
	" You must specify\r a name for your\r save game!",
	" Are you sure you\r wish to replace\r your saved game?"
};

static const StringListProvider kEoB2MenuStringsDrop2AmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsDrop2AmigaEnglish), kEoB2MenuStringsDrop2AmigaEnglish };

static const char *const kEoB2MenuStringsHeadAmigaEnglish[3] = {
	"Camp:",
	"Preferences:",
	"Game Options:"
};

static const StringListProvider kEoB2MenuStringsHeadAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsHeadAmigaEnglish), kEoB2MenuStringsHeadAmigaEnglish };

static const char *const kEoB2MenuStringsPoisonAmigaEnglish[1] = {
	"Poisoned party\rmembers may die!\rRest anyway?"
};

static const StringListProvider kEoB2MenuStringsPoisonAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsPoisonAmigaEnglish), kEoB2MenuStringsPoisonAmigaEnglish };

static const char *const kEoB2MenuStringsMgcAmigaEnglish[2] = {
	"%-18s %1d",
	"%d of %d Remaining.  "
};

static const StringListProvider kEoB2MenuStringsMgcAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsMgcAmigaEnglish), kEoB2MenuStringsMgcAmigaEnglish };

static const char *const kEoB2MenuStringsPrefsAmigaEnglish[4] = {
	"Tunes are %-3s",
	"Sounds are %-3s",
	"Bar Graphs are %-3s",
	""
};

static const StringListProvider kEoB2MenuStringsPrefsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsPrefsAmigaEnglish), kEoB2MenuStringsPrefsAmigaEnglish };

static const char *const kEoB2MenuStringsRest2AmigaEnglish[5] = {
	"%s gained %s.\r",
	"%s memorized %s.\r",
	"%s casts healing on %s.\r",
	"Hours rested: %-4d",
	"\r%s\r"
};

static const StringListProvider kEoB2MenuStringsRest2AmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsRest2AmigaEnglish), kEoB2MenuStringsRest2AmigaEnglish };

static const char *const kEoB2MenuStringsRest3AmigaEnglish[1] = {
	"\x06\x06""You do not feel it is safe to rest here."
};

static const StringListProvider kEoB2MenuStringsRest3AmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsRest3AmigaEnglish), kEoB2MenuStringsRest3AmigaEnglish };

static const char *const kEoB2MenuStringsRest4AmigaEnglish[1] = {
	"\x06\x06""You can't rest here, monsters are near."
};

static const StringListProvider kEoB2MenuStringsRest4AmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsRest4AmigaEnglish), kEoB2MenuStringsRest4AmigaEnglish };

static const char *const kEoB2MenuStringsDefeatAmigaEnglish[1] = {
	"\rYour entire party\rhas been defeated!\rWould you like to\rload a previously\rsaved game?"
};

static const StringListProvider kEoB2MenuStringsDefeatAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsDefeatAmigaEnglish), kEoB2MenuStringsDefeatAmigaEnglish };

static const char *const kEoB2MenuStringsTransferAmigaEnglish[5] = {
	"You must transfer\rfour party members\rbefore you may play!",
	"You may only transfer\rfour party members to\rEOB II!",
	"Items which are not\ruseful or unbalance\rthe game will be\rdeleted.",
	" \r You do not have\r a mage in your\r party.",
	" \r You do not have\r a cleric or\r paladin in your\r party."
};

static const StringListProvider kEoB2MenuStringsTransferAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsTransferAmigaEnglish), kEoB2MenuStringsTransferAmigaEnglish };

static const char *const kEoB2MenuStringsSpecAmigaEnglish[2] = {
	"As you awaken, you\rdiscover that Insal\rthe thief is gone!",
	"A quick inventory\rreveals equipment\rhas been stolen!"
};

static const StringListProvider kEoB2MenuStringsSpecAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuStringsSpecAmigaEnglish), kEoB2MenuStringsSpecAmigaEnglish };

static const char *const kEoB2MenuYesNoStringsAmigaEnglish[2] = {
	"Yes",
	"No"
};

static const StringListProvider kEoB2MenuYesNoStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MenuYesNoStringsAmigaEnglish), kEoB2MenuYesNoStringsAmigaEnglish };

static const char *const kEoB2CharGuiStringsHpAmigaEnglish[2] = {
	"HP",
	"%3d of %-3d"
};

static const StringListProvider kEoB2CharGuiStringsHpAmigaEnglishProvider = { ARRAYSIZE(kEoB2CharGuiStringsHpAmigaEnglish), kEoB2CharGuiStringsHpAmigaEnglish };

static const char *const kEoB2CharGuiStringsWp2AmigaEnglish[3] = {
	"MISS",
	"HACK",
	"BASH"
};

static const StringListProvider kEoB2CharGuiStringsWp2AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharGuiStringsWp2AmigaEnglish), kEoB2CharGuiStringsWp2AmigaEnglish };

static const char *const kEoB2CharGuiStringsWrAmigaEnglish[4] = {
	"CAN'T",
	"REACH",
	"NO",
	"AMMO"
};

static const StringListProvider kEoB2CharGuiStringsWrAmigaEnglishProvider = { ARRAYSIZE(kEoB2CharGuiStringsWrAmigaEnglish), kEoB2CharGuiStringsWrAmigaEnglish };

static const char *const kEoB2CharGuiStringsSt2AmigaEnglish[7] = {
	"Swapping",
	"DEAD",
	"UNCONSCIOUS",
	"POISON (SLOW)",
	"POISONED",
	"PARALYZED",
	"PETRIFIED"
};

static const StringListProvider kEoB2CharGuiStringsSt2AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharGuiStringsSt2AmigaEnglish), kEoB2CharGuiStringsSt2AmigaEnglish };

static const char *const kEoB2CharGuiStringsInAmigaEnglish[4] = {
	"CHARACTER INFO",
	"ARMOR CLASS",
	"EXP",
	"LVL"
};

static const StringListProvider kEoB2CharGuiStringsInAmigaEnglishProvider = { ARRAYSIZE(kEoB2CharGuiStringsInAmigaEnglish), kEoB2CharGuiStringsInAmigaEnglish };

static const char *const kEoB2CharStatusStrings7AmigaEnglish[1] = {
	"%s no longer has giant strength.\r"
};

static const StringListProvider kEoB2CharStatusStrings7AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharStatusStrings7AmigaEnglish), kEoB2CharStatusStrings7AmigaEnglish };

static const char *const kEoB2CharStatusStrings82AmigaEnglish[1] = {
	"\x06\x06""%s feels the effects of poison!\r"
};

static const StringListProvider kEoB2CharStatusStrings82AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharStatusStrings82AmigaEnglish), kEoB2CharStatusStrings82AmigaEnglish };

static const char *const kEoB2CharStatusStrings9AmigaEnglish[1] = {
	"\x06\x04""%s is no longer paralyzed!\r"
};

static const StringListProvider kEoB2CharStatusStrings9AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharStatusStrings9AmigaEnglish), kEoB2CharStatusStrings9AmigaEnglish };

static const char *const kEoB2CharStatusStrings12AmigaEnglish[1] = {
	"%s slows down.\r"
};

static const StringListProvider kEoB2CharStatusStrings12AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharStatusStrings12AmigaEnglish), kEoB2CharStatusStrings12AmigaEnglish };

static const char *const kEoB2CharStatusStrings132AmigaEnglish[1] = {
	"\x06\x06""%s is %s!\r"
};

static const StringListProvider kEoB2CharStatusStrings132AmigaEnglishProvider = { ARRAYSIZE(kEoB2CharStatusStrings132AmigaEnglish), kEoB2CharStatusStrings132AmigaEnglish };

static const char *const kEoB2LevelGainStringsAmigaEnglish[1] = {
	"\x06\x01""%s has gained a level.""\x06\x0f""\r"
};

static const StringListProvider kEoB2LevelGainStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2LevelGainStringsAmigaEnglish), kEoB2LevelGainStringsAmigaEnglish };

static const char *const kEoB2BookNumbersAmigaEnglish[5] = {
	"First",
	"Second",
	"Third",
	"Fourth",
	"Fifth"
};

static const StringListProvider kEoB2BookNumbersAmigaEnglishProvider = { ARRAYSIZE(kEoB2BookNumbersAmigaEnglish), kEoB2BookNumbersAmigaEnglish };

static const char *const kEoB2MageSpellsListAmigaEnglish[33] = {
	"",
	"Armor",
	"Burning Hands",
	"Detect Magic",
	"Magic Missile",
	"Shield",
	"Shocking Grasp",
	"Blur",
	"Detect Invisible",
	"Imp Identify",
	"Invisibility",
	"M's Acid Arrow",
	"Dispel Magic",
	"Fireball",
	"Haste",
	"Hold Person",
	"Invisibility 10'",
	"Lightning Bolt",
	"Vampiric Touch",
	"Fear",
	"Ice Storm",
	"Imp Invisibility",
	"Remove Curse",
	"Cone of Cold",
	"Hold Monster",
	"Wall of Force",
	"Disintegrate",
	"Flesh to Stone",
	"Stone to Flesh",
	"True Seeing",
	"Finger of Death",
	"Power Word Stun",
	"Bigby's Fist"
};

static const StringListProvider kEoB2MageSpellsListAmigaEnglishProvider = { ARRAYSIZE(kEoB2MageSpellsListAmigaEnglish), kEoB2MageSpellsListAmigaEnglish };

static const char *const kEoB2ClericSpellsListAmigaEnglish[30] = {
	"",
	"Bless",
	"Cause Light Wnds",
	"Cure Light Wnds",
	"Detect Magic",
	"Protect-Evil",
	"Aid",
	"Flame Blade",
	"Hold Person",
	"Slow Poison",
	"Create Food",
	"Dispel Magic",
	"Magical Vestment",
	"Prayer",
	"Remove Paralysis",
	"Cause Serious",
	"Cure Serious",
	"Neutral-Poison",
	"Protect-Evil 10'",
	"Cause Critical",
	"Cure Critical",
	"Flame Strike",
	"Raise Dead",
	"Slay Living",
	"True Seeing",
	"Harm",
	"Heal",
	"Ressurection",
	"Lay on Hands",
	"Turn undead"
};

static const StringListProvider kEoB2ClericSpellsListAmigaEnglishProvider = { ARRAYSIZE(kEoB2ClericSpellsListAmigaEnglish), kEoB2ClericSpellsListAmigaEnglish };

static const char *const kEoB2SpellNamesAmigaEnglish[68] = {
	"armor",
	"burning hands",
	"detect magic",
	"magic missile",
	"shield",
	"shocking grasp",
	"blur",
	"detect invisibility",
	"improved identify",
	"invisibility",
	"melf's acid arrow",
	"dispel magic",
	"fireball",
	"haste",
	"Hold Person",
	"invisibility 10' radius",
	"lightning bolt",
	"vampiric touch",
	"fear",
	"ice storm",
	"improved invisibility",
	"remove curse",
	"cone of cold",
	"hold monster",
	"wall of force",
	"disintegrate",
	"flesh to stone",
	"stone to flesh",
	"true seeing",
	"finger of death",
	"power word stun",
	"bigby's clenched fist",
	"bless",
	"cause light wounds",
	"cure light wounds",
	"detect magic",
	"protection from evil",
	"aid",
	"flame blade",
	"hold person",
	"slow poison",
	"create food",
	"dispel magic",
	"magical vestment",
	"prayer",
	"remove paralysis",
	"cause serious wounds",
	"cure serious wounds",
	"neutralize poison",
	"protection from evil 10' radius",
	"cause critical wounds",
	"cure critical wounds",
	"flame strike",
	"raise dead",
	"slay living",
	"true seeing",
	"harm",
	"heal",
	"ressurection",
	"lay on hands",
	"turn undead",
	"",
	"mystic defense",
	"",
	"",
	"",
	"",
	""
};

static const StringListProvider kEoB2SpellNamesAmigaEnglishProvider = { ARRAYSIZE(kEoB2SpellNamesAmigaEnglish), kEoB2SpellNamesAmigaEnglish };

static const char *const kEoB2MagicStrings1AmigaEnglish[6] = {
	"ABORT SPELL",
	"ABORT SPELL",
	"You must have a free hand for this spell.\r",
	"You can't have two of this spell type active.\r",
	"%s casts %s.\r",
	"\rok\r"
};

static const StringListProvider kEoB2MagicStrings1AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings1AmigaEnglish), kEoB2MagicStrings1AmigaEnglish };

static const char *const kEoB2MagicStrings2AmigaEnglish[5] = {
	"The spell fails!\r",
	"%s has been disintegrated!\r",
	"\x06\x06""The party has been hit by a death spell!\r",
	"\x06\x06""%s has been hit by cause serious wounds.\r",
	"PETRIFIED"
};

static const StringListProvider kEoB2MagicStrings2AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings2AmigaEnglish), kEoB2MagicStrings2AmigaEnglish };

static const char *const kEoB2MagicStrings3AmigaEnglish[5] = {
	"Cast spell on which character? ",
	"\rSpell aborted.\r",
	"%s's %s spell expires.\r",
	"%s missed the monster.\r",
	"%s must be in the front ranks to hit!\r"
};

static const StringListProvider kEoB2MagicStrings3AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings3AmigaEnglish), kEoB2MagicStrings3AmigaEnglish };

static const char *const kEoB2MagicStrings4AmigaEnglish[1] = {
	"no effect.\r"
};

static const StringListProvider kEoB2MagicStrings4AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings4AmigaEnglish), kEoB2MagicStrings4AmigaEnglish };

static const char *const kEoB2MagicStrings6AmigaEnglish[1] = {
	"%s already has an armor class better than AC 6."
};

static const StringListProvider kEoB2MagicStrings6AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings6AmigaEnglish), kEoB2MagicStrings6AmigaEnglish };

static const char *const kEoB2MagicStrings7AmigaEnglish[2] = {
	"%s is already under the effect of a %s spell.\r",
	"The party is already under the effect of a %s spell.\r"
};

static const StringListProvider kEoB2MagicStrings7AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings7AmigaEnglish), kEoB2MagicStrings7AmigaEnglish };

static const char *const kEoB2MagicStrings8AmigaEnglish[6] = {
	"The spell fails!\r",
	"held",
	"The spell fails!\r",
	"The spell fails\r",
	"No monster within reach!\r",
	"%s is already under the effect of an aid spell.\r"
};

static const StringListProvider kEoB2MagicStrings8AmigaEnglishProvider = { ARRAYSIZE(kEoB2MagicStrings8AmigaEnglish), kEoB2MagicStrings8AmigaEnglish };

static const byte kEoB2ManDefAmigaEnglish[200] = {
	0x09, 0x03, 0x04, 0x00, 0x09, 0x0d, 0x02, 0x00,
	0x09, 0x07, 0x07, 0x00, 0x14, 0x06, 0x05, 0x00,
	0x14, 0x10, 0x06, 0x00, 0x14, 0x1c, 0x03, 0x00,
	0x1a, 0x04, 0x06, 0x00, 0x1a, 0x0a, 0x05, 0x00,
	0x1a, 0x0c, 0x05, 0x00, 0x15, 0x03, 0x04, 0x00,
	0x15, 0x05, 0x05, 0x00, 0x15, 0x0f, 0x02, 0x00,
	0x15, 0x14, 0x03, 0x00, 0x15, 0x1b, 0x02, 0x00,
	0x15, 0x1d, 0x03, 0x00, 0x23, 0x01, 0x03, 0x00,
	0x23, 0x03, 0x03, 0x00, 0x23, 0x08, 0x03, 0x00,
	0x23, 0x19, 0x02, 0x00, 0x23, 0x1f, 0x04, 0x00,
	0x23, 0x26, 0x04, 0x00, 0x26, 0x03, 0x02, 0x00,
	0x26, 0x05, 0x06, 0x00, 0x26, 0x12, 0x03, 0x00,
	0x26, 0x18, 0x01, 0x00, 0x26, 0x1e, 0x01, 0x00,
	0x26, 0x21, 0x04, 0x00, 0x17, 0x01, 0x03, 0x00,
	0x17, 0x03, 0x06, 0x00, 0x17, 0x06, 0x02, 0x00,
	0x17, 0x0f, 0x05, 0x00, 0x17, 0x1b, 0x01, 0x00,
	0x17, 0x21, 0x06, 0x00, 0x12, 0x03, 0x02, 0x00,
	0x12, 0x05, 0x04, 0x00, 0x12, 0x09, 0x02, 0x00,
	0x12, 0x0b, 0x04, 0x00, 0x12, 0x0d, 0x06, 0x00,
	0x12, 0x0f, 0x03, 0x00, 0x12, 0x11, 0x05, 0x00,
	0x12, 0x12, 0x02, 0x00, 0x1f, 0x02, 0x04, 0x00,
	0x1f, 0x02, 0x07, 0x00, 0x1f, 0x04, 0x03, 0x00,
	0x1f, 0x06, 0x03, 0x00, 0x1f, 0x09, 0x03, 0x00,
	0x1f, 0x0a, 0x01, 0x00, 0x1c, 0x03, 0x03, 0x00,
	0x1c, 0x04, 0x02, 0x00, 0x1c, 0x05, 0x06, 0x00
};

static const ByteProvider kEoB2ManDefAmigaEnglishProvider = { ARRAYSIZE(kEoB2ManDefAmigaEnglish), kEoB2ManDefAmigaEnglish };

static const char *const kEoB2ManWordAmigaEnglish[51] = {
	"cursor",
	"majority",
	"right",
	"unusable",
	"greyed",
	"thrown",
	"spellcasters",
	"button",
	"characters",
	"carefree",
	"practical",
	"inherit",
	"while",
	"combinations",
	"charm",
	"individuals",
	"gestures",
	"pummel",
	"paladins",
	"skill",
	"advancement",
	"also",
	"counters",
	"knowledge",
	"greater",
	"assume",
	"several",
	"laying",
	"reach",
	"swiftly",
	"allows",
	"limited",
	"jack",
	"thrown",
	"weapons",
	"note",
	"certain",
	"damage",
	"done",
	"bonus",
	"may",
	"over",
	"box",
	"put",
	"portrait",
	"backpack",
	"inside",
	"causes",
	"until",
	"outright",
	""
};

static const StringListProvider kEoB2ManWordAmigaEnglishProvider = { ARRAYSIZE(kEoB2ManWordAmigaEnglish), kEoB2ManWordAmigaEnglish };

static const char *const kEoB2ManPromptAmigaEnglish[1] = {
	"\r\r\r\rOn the page with this symbol...\r\rFind line %d\rEnter word %d\r"
};

static const StringListProvider kEoB2ManPromptAmigaEnglishProvider = { ARRAYSIZE(kEoB2ManPromptAmigaEnglish), kEoB2ManPromptAmigaEnglish };

static const char *const kEoB2MainMenuStringsAmigaEnglish[5] = {
	"LOAD GAME IN PROGRESS\r",
	"START A NEW PARTY\r",
	"TRANSFER EOB I PARTY\r",
	"RUN INTRODUCTION\r",
	"FORMAT SAVE GAME DISK\r"
};

static const StringListProvider kEoB2MainMenuStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MainMenuStringsAmigaEnglish), kEoB2MainMenuStringsAmigaEnglish };

static const char *const kEoB2TransferStrings1AmigaEnglish[2] = {
	"Level: %d",
	" / %d"
};

static const StringListProvider kEoB2TransferStrings1AmigaEnglishProvider = { ARRAYSIZE(kEoB2TransferStrings1AmigaEnglish), kEoB2TransferStrings1AmigaEnglish };

static const char *const kEoB2TransferStrings2AmigaEnglish[2] = {
	"You may only take four characters on your adventure.",
	"Select OK when you are finished choosing your party."
};

static const StringListProvider kEoB2TransferStrings2AmigaEnglishProvider = { ARRAYSIZE(kEoB2TransferStrings2AmigaEnglish), kEoB2TransferStrings2AmigaEnglish };

static const char *const kEoB2TransferLabelsAmigaEnglish[2] = {
	"CANCEL",
	"OK"
};

static const StringListProvider kEoB2TransferLabelsAmigaEnglishProvider = { ARRAYSIZE(kEoB2TransferLabelsAmigaEnglish), kEoB2TransferLabelsAmigaEnglish };

static const char *const kEoB2IntroStringsAmigaEnglish[20] = {
	"You were settling in for an evening \rby the fire.",
	"Then a note was slipped to you.",
	"It was from your friend Khelben \rBlackstaff, Archmage of the city.",
	"The message was urgent.",
	"What could Khelben want?",
	"Welcome, please come in.",
	"Khelben awaits you in his study.",
	"Thank you for coming so quickly.",
	"I am troubled my friend.",
	"Ancient Evil stirs in the \rTemple Darkmoon.",
	"I fear for the safety of our city.",
	"I need your help.",
	"Three nights ago \rI sent forth a scout.",
	"She has not yet returned.",
	"I fear for her safety.",
	"Take this coin.",
	"I will use it to contact you.",
	"You must act quickly.",
	"I will teleport you near Darkmoon.",
	"May luck be with you, my friend."
};

static const StringListProvider kEoB2IntroStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2IntroStringsAmigaEnglish), kEoB2IntroStringsAmigaEnglish };

static const char *const kEoB2IntroCPSFilesAmigaEnglish[16] = {
	"STREET1.CPS",
	"STREET2.CPS",
	"DOORWAY1.CPS",
	"DOORWAY2.CPS",
	"WESTWOOD1.CPS",
	"WINDING.CPS",
	"KHELBAN2.CPS",
	"KHELBAN1.CPS",
	"KHELBAN3.CPS",
	"KHELBAN4.CPS",
	"COIN.CPS",
	"KHELBAN5.CPS",
	"KHELBAN6.CPS",
	"WESTWOOD2.CPS",
	"WESTWOOD3.CPS",
	"WESTWOOD4.CPS"
};

static const StringListProvider kEoB2IntroCPSFilesAmigaEnglishProvider = { ARRAYSIZE(kEoB2IntroCPSFilesAmigaEnglish), kEoB2IntroCPSFilesAmigaEnglish };

static const char *const kEoB2FinaleStringsAmigaEnglish[20] = {
	"Finally, Dran has been defeated.",
	"Suddenly, your friend Khelben appears.",
	"Greetings, my victorious friends.",
	"You have defeated Dran!",
	"I did not know Dran was a dragon.",
	"He must have been over 300 years old!",
	"His power is gone.",
	"But Darkmoon is still a source\rof great evil.",
	"And many of his minions remain.",
	"Now we must leave this place.",
	"So my forces can destroy it\ronce and for all.",
	"Follow me.",
	"Powerful mages stand ready\rfor the final assault\ron Darkmoon.",
	"The Temple's evil is very strong.",
	"It must not be allowed to survive!",
	"The Temple ceases to exist.",
	"My friends, our work is done.",
	"Thank you.",
	"You have earned my deepest respect.",
	"We will remember you always."
};

static const StringListProvider kEoB2FinaleStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2FinaleStringsAmigaEnglish), kEoB2FinaleStringsAmigaEnglish };

static const char *const kEoB2FinaleCPSFilesAmigaEnglish[13] = {
	"DRAGON1.CPS",
	"DRAGON2.CPS",
	"HURRY1.CPS",
	"HURRY2.CPS",
	"DESTROY0.CPS",
	"DESTROY1.CPS",
	"DESTROY2.CPS",
	"MAGIC.CPS",
	"DESTROY3.CPS",
	"CREDITS2.CPS",
	"CREDITS3.CPS",
	"HEROES.CPS",
	"THANKS.CPS"
};

static const StringListProvider kEoB2FinaleCPSFilesAmigaEnglishProvider = { ARRAYSIZE(kEoB2FinaleCPSFilesAmigaEnglish), kEoB2FinaleCPSFilesAmigaEnglish };

static const char *const kEoB2MonsterDistAttStringsAmigaEnglish[5] = {
	"The party is hit with a psychic mind blast!\r",
	"paralyzed",
	"poisoned",
	"paralyzed",
	"PETRIFIED"
};

static const StringListProvider kEoB2MonsterDistAttStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MonsterDistAttStringsAmigaEnglish), kEoB2MonsterDistAttStringsAmigaEnglish };

static const char *const kEoB2Npc1StringsAmigaEnglish[2] = {
	"talk",
	"leave"
};

static const StringListProvider kEoB2Npc1StringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2Npc1StringsAmigaEnglish), kEoB2Npc1StringsAmigaEnglish };

static const char *const kEoB2Npc2StringsAmigaEnglish[2] = {
	"release him",
	"leave"
};

static const StringListProvider kEoB2Npc2StringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2Npc2StringsAmigaEnglish), kEoB2Npc2StringsAmigaEnglish };

static const char *const kEoB2MonsterDustStringsAmigaEnglish[1] = {
	"The monster crumbles to dust!\r"
};

static const StringListProvider kEoB2MonsterDustStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2MonsterDustStringsAmigaEnglish), kEoB2MonsterDustStringsAmigaEnglish };

static const char *const kEoB2KheldranStringsAmigaEnglish[1] = {
	"    Such trusting whelps!"
};

static const StringListProvider kEoB2KheldranStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2KheldranStringsAmigaEnglish), kEoB2KheldranStringsAmigaEnglish };

static const char *const kEoB2HornStringsAmigaEnglish[4] = {
	"A bellowing sound comes from the horn.\r",
	"A hollow sound comes from the horn.\r",
	"A melodious sound comes from the horn.\r",
	"An eerie sound comes from the horn.\r"
};

static const StringListProvider kEoB2HornStringsAmigaEnglishProvider = { ARRAYSIZE(kEoB2HornStringsAmigaEnglish), kEoB2HornStringsAmigaEnglish };

static const char *const kEoB2NpcPresetsNamesAmigaEnglish[6] = {
	"Insal",
	"Calandra",
	"Shorn",
	"San-raal",
	"Tanglor",
	"Amber"
};

static const StringListProvider kEoB2NpcPresetsNamesAmigaEnglishProvider = { ARRAYSIZE(kEoB2NpcPresetsNamesAmigaEnglish), kEoB2NpcPresetsNamesAmigaEnglish };

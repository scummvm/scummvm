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

#ifndef NUVIE_CORE_U6_OBJECTS_H
#define NUVIE_CORE_U6_OBJECTS_H

namespace Ultima {
namespace Nuvie {

//object numbers
#define OBJ_U6_NOTHING               0

#define OBJ_U6_LEATHER_HELM          1
#define OBJ_U6_CHAIN_COIF            2
#define OBJ_U6_IRON_HELM             3
#define OBJ_U6_SPIKED_HELM           4
#define OBJ_U6_WINGED_HELM           5
#define OBJ_U6_BRASS_HELM            6
#define OBJ_U6_GARGOYLE_HELM         7
#define OBJ_U6_MAGIC_HELM            8
#define OBJ_U6_WOODEN_SHIELD         9
#define OBJ_U6_CURVED_HEATER        10
#define OBJ_U6_WINGED_SHIELD        11
#define OBJ_U6_KITE_SHIELD          12
#define OBJ_U6_SPIKED_SHIELD        13
#define OBJ_U6_BLACK_SHIELD         14
#define OBJ_U6_DOOR_SHIELD          15
#define OBJ_U6_MAGIC_SHIELD         16
#define OBJ_U6_CLOTH_ARMOUR         17
#define OBJ_U6_LEATHER_ARMOR        18
#define OBJ_U6_RING_MAIL            19
#define OBJ_U6_SCALE_MAIL           20
#define OBJ_U6_CHAIN_MAIL           21
#define OBJ_U6_PLATE_MAIL           22
#define OBJ_U6_MAGIC_ARMOUR         23
#define OBJ_U6_SPIKED_COLLAR        24
#define OBJ_U6_GUILD_BELT           25
#define OBJ_U6_GARGOYLE_BELT        26
#define OBJ_U6_LEATHER_BOOTS        27
#define OBJ_U6_SWAMP_BOOTS          28
#define OBJ_U6_TILE_DIRT            29
#define OBJ_U6_TILE_BOARDS          30
#define OBJ_U6_TILE_TILES           31
#define OBJ_U6_TILE_BLUE_TILES      32

#define OBJ_U6_SLING                33
#define OBJ_U6_CLUB                 34
#define OBJ_U6_MAIN_GAUCHE          35
#define OBJ_U6_SPEAR                36
#define OBJ_U6_THROWING_AXE         37
#define OBJ_U6_DAGGER               38
#define OBJ_U6_MACE                 39
#define OBJ_U6_MORNING_STAR         40
#define OBJ_U6_BOW                  41
#define OBJ_U6_CROSSBOW             42
#define OBJ_U6_SWORD                43
#define OBJ_U6_TWO_HANDED_HAMMER    44
#define OBJ_U6_TWO_HANDED_AXE       45
#define OBJ_U6_TWO_HANDED_SWORD     46
#define OBJ_U6_HALBERD              47
#define OBJ_U6_GLASS_SWORD          48
#define OBJ_U6_BOOMERANG            49
#define OBJ_U6_TRIPLE_CROSSBOW      50
#define OBJ_U6_FORCE_FIELD          51
#define OBJ_U6_WIZARD_EYE           52
#define OBJ_U6_OBJECT_53            53
#define OBJ_U6_MAGIC_BOW            54
#define OBJ_U6_ARROW                55
#define OBJ_U6_BOLT                 56
#define OBJ_U6_SPELLBOOK            57
#define OBJ_U6_SPELL                58
#define OBJ_U6_CODEX                59
#define OBJ_U6_BOOK_OF_PROPHECIES   60
#define OBJ_U6_BOOK_OF_CIRCLES      61
#define OBJ_U6_VORTEX_CUBE          62
#define OBJ_U6_LOCK_PICK            63
#define OBJ_U6_KEY                  64

#define OBJ_U6_BLACK_PEARL          65
#define OBJ_U6_BLOOD_MOSS           66
#define OBJ_U6_GARLIC               67
#define OBJ_U6_GINSENG              68
#define OBJ_U6_MANDRAKE_ROOT        69
#define OBJ_U6_NIGHTSHADE           70
#define OBJ_U6_SPIDER_SILK          71
#define OBJ_U6_SULFUROUS_ASH        72
#define OBJ_U6_MOONSTONE            73
#define OBJ_U6_ANKH_AMULET          74
#define OBJ_U6_SNAKE_AMULET         75
#define OBJ_U6_AMULET_OF_SUBMISSION 76
#define OBJ_U6_GEM                  77
#define OBJ_U6_STAFF                78
#define OBJ_U6_LIGHTNING_WAND       79
#define OBJ_U6_FIRE_WAND            80
#define OBJ_U6_STORM_CLOAK          81
#define OBJ_U6_RING                 82
#define OBJ_U6_FLASK_OF_OIL         83
#define OBJ_U6_RED_GATE             84
#define OBJ_U6_MOONGATE             85
#define OBJ_U6_GAVEL                86
#define OBJ_U6_ORB_OF_THE_MOONS     87
#define OBJ_U6_GOLD                 88
#define OBJ_U6_GOLD_NUGGET          89
#define OBJ_U6_TORCH                90
#define OBJ_U6_ZU_YLEM              91
#define OBJ_U6_SNAKE_VENOM          92
#define OBJ_U6_SEXTANT              93
#define OBJ_U6_SPINNING_WHEEL       94
#define OBJ_U6_GRAPES               95
#define OBJ_U6_BUTTER               96

#define OBJ_U6_GARGISH_VOCABULARY   97
#define OBJ_U6_CHEST                98
#define OBJ_U6_BACKPACK             99
#define OBJ_U6_SCYTHE              100
#define OBJ_U6_PITCHFORK           101
#define OBJ_U6_RAKE                102
#define OBJ_U6_PICK                103
#define OBJ_U6_SHOVEL              104
#define OBJ_U6_HOE                 105
#define OBJ_U6_WOODEN_LADDER       106
#define OBJ_U6_YOKE                107
#define OBJ_U6_OVEN_SPATULA        108
#define OBJ_U6_ROLLING_PIN         109
#define OBJ_U6_SPATULA             110
#define OBJ_U6_LADLE               111
#define OBJ_U6_COOKING_SHEET       112
#define OBJ_U6_CLEAVER             113
#define OBJ_U6_KNIFE               114
#define OBJ_U6_WINE                115
#define OBJ_U6_MEAD                116
#define OBJ_U6_ALE                 117
#define OBJ_U6_WINE_GLASS          118
#define OBJ_U6_PLATE               119
#define OBJ_U6_MUG                 120
#define OBJ_U6_SILVERWARE          121
#define OBJ_U6_CANDLE              122
#define OBJ_U6_MIRROR              123
#define OBJ_U6_TUNIC               124
#define OBJ_U6_HANGER              125
#define OBJ_U6_DRESS               126
#define OBJ_U6_SKILLET             127
#define OBJ_U6_BREAD               128

#define OBJ_U6_MEAT_PORTION        129
#define OBJ_U6_ROLLS               130
#define OBJ_U6_CAKE                131
#define OBJ_U6_CHEESE              132
#define OBJ_U6_HAM                 133
#define OBJ_U6_HORSE_CARCASS       134
#define OBJ_U6_HORSE_CHOPS         135

#define OBJ_U6_PANTS               137
#define OBJ_U6_PLANT               138

#define OBJ_U6_DECORATIVE_SWORD    141

#define OBJ_U6_PICTURE             143
#define OBJ_U6_CANDELABRA          145
#define OBJ_U6_PERSON_SLEEPING     146
#define OBJ_U6_CAULDRON            147

#define OBJ_U6_SHIP_DEED           149
#define OBJ_U6_BOOK                151
#define OBJ_U6_SCROLL              152

#define OBJ_U6_PANPIPES            153
#define OBJ_U6_CRYSTAL_BALL        155
#define OBJ_U6_HARPSICHORD         156
#define OBJ_U6_HARP                157
#define OBJ_U6_LUTE                158

#define OBJ_U6_CLOCK               159
#define OBJ_U6_WATER_VASE          161
#define OBJ_U6_BED                 163
#define OBJ_U6_FIREPLACE           164

#define OBJ_U6_REMAINS             168

#define OBJ_U6_RUBBER_DUCKY        169

#define OBJ_U6_FUMAROLE            171
#define OBJ_U6_SPIKES              172
#define OBJ_U6_TRAP                173
#define OBJ_U6_SWITCH              174
#define OBJ_U6_ELECTRIC_FIELD      175
#define OBJ_U6_DRAWER              176
#define OBJ_U6_DESK                177
#define OBJ_U6_BUCKET              178
#define OBJ_U6_BUCKET_OF_WATER     179
#define OBJ_U6_BUCKET_OF_MILK      180
#define OBJ_U6_CHURN               181

#define OBJ_U6_BEEHIVE             182
#define OBJ_U6_HONEY_JAR           183
#define OBJ_U6_JAR_OF_HONEY        184

#define OBJ_U6_BARREL              186
#define OBJ_U6_BAG                 188

#define OBJ_U6_BASKET              191
#define OBJ_U6_CRATE               192

#define OBJ_U6_PLIERS              203
#define OBJ_U6_HAMMER              204

#define OBJ_U6_BRAZIER             206

#define OBJ_U6_MEAT                209
#define OBJ_U6_RIBS                210

#define OBJ_U6_DEAD_ANIMAL         211

#define OBJ_U6_FAN                 212

#define OBJ_U6_MOUSEHOLE           213
#define OBJ_U6_CANNON              221
#define OBJ_U6_POWDER_KEG          223
#define OBJ_U6_THREAD              225
#define OBJ_U6_WELL                233
#define OBJ_U6_FOUNTAIN            234
#define OBJ_U6_SUNDIAL             235
#define OBJ_U6_BELL                236

#define OBJ_U6_RUNE_HONESTY        242
#define OBJ_U6_RUNE_COMPASSION     243
#define OBJ_U6_RUNE_VALOR          244
#define OBJ_U6_RUNE_JUSTICE        245
#define OBJ_U6_RUNE_SACRIFICE      246
#define OBJ_U6_RUNE_HONOR          247
#define OBJ_U6_RUNE_SPIRITUALITY   248
#define OBJ_U6_RUNE_HUMILITY       249

#define OBJ_U6_CHAIR               252
#define OBJ_U6_CAMPFIRE            253
#define OBJ_U6_CROSS               254
#define OBJ_U6_TOMBSTONE           255

#define OBJ_U6_PROTECTION_RING     256
#define OBJ_U6_REGENERATION_RING   257
#define OBJ_U6_INVISIBILITY_RING   258

#define OBJ_U6_FISHING_POLE        264
#define OBJ_U6_FISH                265

#define OBJ_U6_GRAVE               266

#define OBJ_U6_LEVER               268
#define OBJ_U6_DRAWBRIDGE          269
#define OBJ_U6_BALLOON_PLANS       270
#define OBJ_U6_POTION              275
#define OBJ_U6_V_PASSTHROUGH       278
#define OBJ_U6_H_PASSTHROUGH       280
#define OBJ_U6_FENCE               281
#define OBJ_U6_BARS                282
#define OBJ_U6_ROPE                284
#define OBJ_U6_WATER_WHEEL         287
#define OBJ_U6_CRANK               288
#define OBJ_U6_LOG_SAW             289
#define OBJ_U6_CHAIN               293

#define OBJ_U6_XYLOPHONE           296

#define OBJ_U6_OAKEN_DOOR          297
#define OBJ_U6_WINDOWED_DOOR       298
#define OBJ_U6_CEDAR_DOOR          299
#define OBJ_U6_STEEL_DOOR          300

#define OBJ_U6_DOORWAY             301
#define OBJ_U6_LADDER              305
#define OBJ_U6_VOLCANO             307
#define OBJ_U6_HOLE                308
#define OBJ_U6_PORTCULLIS          310
#define OBJ_U6_STONE_LION          312

#define OBJ_U6_FIRE_FIELD          317
#define OBJ_U6_POISON_FIELD        318
#define OBJ_U6_PROTECTION_FIELD    319
#define OBJ_U6_SLEEP_FIELD         320

#define OBJ_U6_CAVE                326

#define OBJ_U6_THRONE              327

#define OBJ_U6_SIGN                332
#define OBJ_U6_SIGN_ARROW          333
#define OBJ_U6_SECRET_DOOR         334

#define OBJ_U6_EGG                 335
#define OBJ_U6_CHARGE              336
#define OBJ_U6_EFFECT              337
#define OBJ_U6_BLOOD               338
#define OBJ_U6_DEAD_BODY           339
#define OBJ_U6_DEAD_CYCLOPS        340
#define OBJ_U6_DEAD_GARGOYLE       341
#define OBJ_U6_GIANT_RAT           342
#define OBJ_U6_INSECTS             343
#define OBJ_U6_GIANT_BAT           344
#define OBJ_U6_GIANT_SQUID         345
#define OBJ_U6_REAPER              347
#define OBJ_U6_SEA_SERPENT         346
#define OBJ_U6_SHEEP               348
#define OBJ_U6_DOG                 349
#define OBJ_U6_DEER                350
#define OBJ_U6_WOLF                351
#define OBJ_U6_GHOST               352
#define OBJ_U6_GREMLIN             353
#define OBJ_U6_MOUSE               354
#define OBJ_U6_GAZER               355
#define OBJ_U6_BIRD                356
#define OBJ_U6_CORPSER             357
#define OBJ_U6_SNAKE               358
#define OBJ_U6_RABBIT              359
#define OBJ_U6_ROT_WORMS           360
#define OBJ_U6_GIANT_SPIDER        361
#define OBJ_U6_WINGED_GARGOYLE     362
#define OBJ_U6_GARGOYLE            363
#define OBJ_U6_ACID_SLUG           364
#define OBJ_U6_TANGLE_VINE_POD     365
#define OBJ_U6_TANGLE_VINE         366
#define OBJ_U6_DAEMON              367
#define OBJ_U6_SKELETON            368
#define OBJ_U6_DRAKE               369
#define OBJ_U6_HEADLESS            370
#define OBJ_U6_TROLL               371
#define OBJ_U6_MONGBAT             372
#define OBJ_U6_WISP                373
#define OBJ_U6_HYDRA               374
#define OBJ_U6_SLIME               375
#define OBJ_U6_FIGHTER             376
#define OBJ_U6_SWASHBUCKLER        377
#define OBJ_U6_MAGE                378
#define OBJ_U6_VILLAGER            379
#define OBJ_U6_MERCHANT            380
#define OBJ_U6_CHILD               381
#define OBJ_U6_GUARD               382

#define OBJ_U6_JESTER              383
#define OBJ_U6_PEASANT             384
#define OBJ_U6_FARMER              385
#define OBJ_U6_MUSICIAN            386
#define OBJ_U6_WOMAN               387
#define OBJ_U6_CAT                 388
#define OBJ_U6_MUSICIAN_PLAYING    392
#define OBJ_U6_SHRINE              393
#define OBJ_U6_BRITANNIAN_LENS     394
#define OBJ_U6_GARGOYLE_LENS       396
#define OBJ_U6_STATUE_OF_MONDAIN   397
#define OBJ_U6_STATUE_OF_MINAX     398
#define OBJ_U6_STATUE_OF_EXODUS    399


#define OBJ_U6_LORD_BRITISH        409
#define OBJ_U6_AVATAR              410

#define OBJ_U6_DRAGON              411

#define OBJ_U6_SHIP                412
#define OBJ_U6_SILVER_SERPENT      413
#define OBJ_U6_SKIFF               414
#define OBJ_U6_RAFT                415
#define OBJ_U6_NO_VEHICLE          416

#define OBJ_U6_QUEST_GATE          416
#define OBJ_U6_DRAGON_EGG          417
#define OBJ_U6_PULL_CHAIN          419
#define OBJ_U6_BALLOON             420
#define OBJ_U6_MAMMOTH_SILK_BAG    421
#define OBJ_U6_BALLOON_BASKET      422
#define OBJ_U6_INFLATED_BALLOON    423

#define OBJ_U6_CYCLOPS             424
#define OBJ_U6_HYDRA_BODY          425

#define OBJ_U6_GIANT_SCORPION      426
#define OBJ_U6_GIANT_ANT           427
#define OBJ_U6_COW                 428
#define OBJ_U6_ALLIGATOR           429
#define OBJ_U6_HORSE               430
#define OBJ_U6_HORSE_WITH_RIDER    431

#define OBJ_U6__LAST_              431

// Savage Empire
#define OBJ_SE_MAGNESIUM_RIBBON     10
#define OBJ_SE_SPEAR                26
#define OBJ_SE_THROWING_AXE         27
#define OBJ_SE_POISONED_DART        36
#define OBJ_SE_RIFLE_BULLET         41
#define OBJ_SE_KNIFE                44
#define OBJ_SE_ARROW                45
#define OBJ_SE_TURTLE_BAIT          47
#define OBJ_SE_FEATHER              48
#define OBJ_SE_CHOCOLATL            54
#define OBJ_SE_PINDE                55
#define OBJ_SE_YOPO                 56
#define OBJ_SE_MORTAR               59
#define OBJ_SE_GRINDING_STONE       60
#define OBJ_SE_JUG_OF_PLACHTA       63
#define OBJ_SE_GOLD                 69
#define OBJ_SE_GOLD_NUGGET          70
#define OBJ_SE_DIAMOND              72
#define OBJ_SE_EMERALD              73
#define OBJ_SE_RUBY                 74
#define OBJ_SE_CORN_MEAL            93
#define OBJ_SE_BOTTLE_OF_LIQUOR     95
#define OBJ_SE_JAR                  97
#define OBJ_SE_TORTILLA            102
#define OBJ_SE_MEAT_103            103
#define OBJ_SE_BERRY               104
#define OBJ_SE_CAKE                105
#define OBJ_SE_CORN                108
#define OBJ_SE_BEAN                109
#define OBJ_SE_MEAT_110            110
#define OBJ_SE_ORCHID              115
#define OBJ_SE_PEPPER              120
#define OBJ_SE_SULFUR              123
#define OBJ_SE_CHARCOAL            129
#define OBJ_SE_POTASSIUM_NITRATE   130
#define OBJ_SE_SOFT_CLAY_POT       132
#define OBJ_SE_FIRED_CLAY_POT      133
#define OBJ_SE_CLOTH_STRIP         134
#define OBJ_SE_GRENADE             137
#define OBJ_SE_TAR                 139
#define OBJ_SE_WATER               140
#define OBJ_SE_CLOTH               180
#define OBJ_SE_JUG                 181
#define OBJ_SE_POUCH               182
#define OBJ_SE_BASKET              183
#define OBJ_SE_POT                 184
#define OBJ_SE_TARRED_CLOTH_STRIP  191
#define OBJ_SE_CLAY                192
#define OBJ_SE_GUNPOWDER           204
#define OBJ_SE_BRANCH              206
#define OBJ_SE_TORCH               208
#define OBJ_SE_FLAX                210
#define OBJ_SE_RIB_BONE            211
#define OBJ_SE_CHOP                214
#define OBJ_SE_DEVICE              240
#define OBJ_SE_DEAD_BODY           249

// Martian Dreams
#define OBJ_MD_DOLLAR                 24
#define OBJ_MD_PISTOL_ROUND           57
#define OBJ_MD_SHOTGUN_SHELL          58
#define OBJ_MD_RIFLE_ROUND            59
#define OBJ_MD_ELEPHANT_GUN_ROUND     60
#define OBJ_MD_SLING_STONE            63
#define OBJ_MD_ARROW                  64
#define OBJ_MD_BERRY                  73
#define OBJ_MD_BERRY1                 74
#define OBJ_MD_BERRY2                 75
#define OBJ_MD_BERRY3                 76
#define OBJ_MD_BERRY4                 77
#define OBJ_MD_BACKPACK               80
#define OBJ_MD_LARGE_SACK             81
#define OBJ_MD_SMALL_POUCH            82
#define OBJ_MD_BRASS_CHEST            83
#define OBJ_MD_OBSIDIAN_BOX           85
#define OBJ_MD_WOODEN_CRATE           86
#define OBJ_MD_STEAMER_TRUNK          87
#define OBJ_MD_CARPET_BAG             89
#define OBJ_MD_POCKETWATCH            91
#define OBJ_MD_MASONIC_SYMBOL         92
#define OBJ_MD_SPECTACLES             93

#define OBJ_MD_BARREL                104
#define OBJ_MD_MATCH                 107
#define OBJ_MD_TORCH                 109
#define OBJ_MD_PAGE                  122
#define OBJ_MD_CAN_OF_LAMP_OIL       124
#define OBJ_MD_BLOB_OF_OXIUM         131
#define OBJ_MD_RUBLE                 132
#define OBJ_MD_LEAD_BOX              139
#define OBJ_MD_WORMSBANE_SEED        158
#define OBJ_MD_CRATE                 284
#define OBJ_MD_BAG                   285
#define OBJ_MD_BRASS_TRUNK           304
#define OBJ_MD_OXYGENATED_AIR_BOTTLE 324
#define OBJ_MD_DREAMSTUFF            331
#define OBJ_MD_DEAD_BODY             341
#define OBJ_MD_CHIP_OF_RADIUM        449
#define OBJ_MD_DREAM_TELEPORTER      461

} // End of namespace Nuvie
} // End of namespace Ultima

#endif

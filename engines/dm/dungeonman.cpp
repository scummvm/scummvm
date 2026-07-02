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

/*
 * Based on the Reverse Engineering work of Christophe Fontanel,
 * maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
 */

#include "common/file.h"
#include "common/memstream.h"

#include "dm/dungeonman.h"
#include "dm/timeline.h"
#include "dm/champion.h"
#include "dm/group.h"
#include "dm/movesens.h"
#include "dm/projexpl.h"

namespace DM {

void DungeonMan::mapCoordsAfterRelMovement(Direction dir, int16 stepsForward, int16 stepsRight, int16 &posX, int16 &posY) {
	posX += _vm->_dirIntoStepCountEast[dir] * stepsForward;
	posY += _vm->_dirIntoStepCountNorth[dir] * stepsForward;
	dir = _vm->turnDirRight(dir);
	posX += _vm->_dirIntoStepCountEast[dir] * stepsRight;
	posY += _vm->_dirIntoStepCountNorth[dir] * stepsRight;
}

void DungeonMan::setupConstants() {
	ObjectInfoIndex objectInfo[180] = { // @ G0237_as_Graphic559_ObjectInfo
		/* { Type, ObjectAspectIndex, ActionSetIndex, AllowedSlots } */
		ObjectInfoIndex(30,  1,  0, 0x0500),   /* COMPASS        Pouch/Chest */
		ObjectInfoIndex(144,  0,  0, 0x0200),  /* COMPASS        Hands */
		ObjectInfoIndex(148, 67,  0, 0x0500),  /* COMPASS        Pouch/Chest */
		ObjectInfoIndex(149, 67,  0, 0x0500),  /* COMPASS        Pouch/Chest */
		ObjectInfoIndex(150, 67,  0, 0x0500),  /* TORCH          Pouch/Chest */
		ObjectInfoIndex(151, 67, 42, 0x0500),  /* TORCH          Pouch/Chest */
		ObjectInfoIndex(152, 67,  0, 0x0500),  /* TORCH          Pouch/Chest */
		ObjectInfoIndex(153, 67,  0, 0x0500),  /* TORCH          Pouch/Chest */
		ObjectInfoIndex(154,  2,  0, 0x0501),  /* WATERSKIN      Mouth/Pouch/Chest */
		ObjectInfoIndex(155,  2,  0, 0x0501),  /* WATER          Mouth/Pouch/Chest */
		ObjectInfoIndex(156,  2,  0, 0x0501),  /* JEWEL SYMAL    Mouth/Pouch/Chest */
		ObjectInfoIndex(157,  2,  0, 0x0501),  /* JEWEL SYMAL    Mouth/Pouch/Chest */
		ObjectInfoIndex(158,  2,  0, 0x0501),  /* ILLUMULET      Mouth/Pouch/Chest */
		ObjectInfoIndex(159,  2,  0, 0x0501),  /* ILLUMULET      Mouth/Pouch/Chest */
		ObjectInfoIndex(160,  2,  0, 0x0501),  /* FLAMITT        Mouth/Pouch/Chest */
		ObjectInfoIndex(161,  2,  0, 0x0501),  /* FLAMITT        Mouth/Pouch/Chest */
		ObjectInfoIndex(162,  2,  0, 0x0501),  /* EYE OF TIME    Mouth/Pouch/Chest */
		ObjectInfoIndex(163,  2,  0, 0x0501),  /* EYE OF TIME    Mouth/Pouch/Chest */
		ObjectInfoIndex(164, 68,  0, 0x0500),  /* STORMRING      Pouch/Chest */
		ObjectInfoIndex(165, 68,  0, 0x0500),  /* STORMRING      Pouch/Chest */
		ObjectInfoIndex(166, 68,  0, 0x0500),  /* STAFF OF CLAWS Pouch/Chest */
		ObjectInfoIndex(167, 68, 42, 0x0500),  /* STAFF OF CLAWS Pouch/Chest */
		ObjectInfoIndex(195, 80,  0, 0x0500),  /* STAFF OF CLAWS Pouch/Chest */
		ObjectInfoIndex(16, 38, 43, 0x0500),   /* BOLT BLADE     Pouch/Chest */
		ObjectInfoIndex(18, 38,  7, 0x0500),   /* BOLT BLADE     Pouch/Chest */
		ObjectInfoIndex(4, 35,  5, 0x0400),    /* FURY           Chest */
		ObjectInfoIndex(14, 37,  6, 0x0400),   /* FURY           Chest */
		ObjectInfoIndex(20, 11,  8, 0x0040),   /* THE FIRESTAFF  Quiver 1 */
		ObjectInfoIndex(23, 12,  9, 0x0040),   /* THE FIRESTAFF  Quiver 1 */
		ObjectInfoIndex(25, 12, 10, 0x0040),   /* THE FIRESTAFF  Quiver 1 */
		ObjectInfoIndex(27, 39, 11, 0x0040),   /* OPEN SCROLL    Quiver 1 */
		ObjectInfoIndex(32, 17, 12, 0x05C0),   /* SCROLL         Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(33, 12, 13, 0x0040),   /* DAGGER         Quiver 1 */
		ObjectInfoIndex(34, 12, 13, 0x0040),   /* FALCHION       Quiver 1 */
		ObjectInfoIndex(35, 12, 14, 0x0040),   /* SWORD          Quiver 1 */
		ObjectInfoIndex(36, 12, 15, 0x0040),   /* RAPIER         Quiver 1 */
		ObjectInfoIndex(37, 12, 15, 0x0040),   /* SABRE          Quiver 1 */
		ObjectInfoIndex(38, 12, 16, 0x0040),   /* SAMURAI SWORD  Quiver 1 */
		ObjectInfoIndex(39, 12, 17, 0x0040),   /* DELTA          Quiver 1 */
		ObjectInfoIndex(40, 42, 18, 0x0040),   /* DIAMOND EDGE   Quiver 1 */
		ObjectInfoIndex(41, 12, 19, 0x0040),   /* VORPAL BLADE   Quiver 1 */
		ObjectInfoIndex(42, 13, 20, 0x0040),   /* THE INQUISITOR Quiver 1 */
		ObjectInfoIndex(43, 13, 21, 0x0040),   /* AXE            Quiver 1 */
		ObjectInfoIndex(44, 21, 22, 0x0040),   /* HARDCLEAVE     Quiver 1 */
		ObjectInfoIndex(45, 21, 22, 0x0040),   /* MACE           Quiver 1 */
		ObjectInfoIndex(46, 33, 23, 0x0440),   /* MACE OF ORDER  Quiver 1/Chest */
		ObjectInfoIndex(47, 43, 24, 0x0040),   /* MORNINGSTAR    Quiver 1 */
		ObjectInfoIndex(48, 44, 24, 0x0040),   /* CLUB           Quiver 1 */
		ObjectInfoIndex(49, 14, 27, 0x0040),   /* STONE CLUB     Quiver 1 */
		ObjectInfoIndex(50, 45, 27, 0x0040),   /* BOW            Quiver 1 */
		ObjectInfoIndex(51, 16, 26, 0x05C0),   /* CROSSBOW       Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(52, 46, 26, 0x05C0),   /* ARROW          Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(53, 11, 27, 0x0440),   /* SLAYER         Quiver 1/Chest */
		ObjectInfoIndex(54, 47, 42, 0x05C0),   /* SLING          Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(55, 48, 40, 0x05C0),   /* ROCK           Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(56, 49, 42, 0x05C0),   /* POISON DART    Quiver 1/Quiver 2/Pouch/Chest */
		ObjectInfoIndex(57, 50,  5, 0x0040),   /* THROWING STAR  Quiver 1 */
		ObjectInfoIndex(58, 11,  5, 0x0040),   /* STICK          Quiver 1 */
		ObjectInfoIndex(59, 31, 28, 0x0540),   /* STAFF          Quiver 1/Pouch/Chest */
		ObjectInfoIndex(60, 31, 29, 0x0540),   /* WAND           Quiver 1/Pouch/Chest */
		ObjectInfoIndex(61, 11, 30, 0x0040),   /* TEOWAND        Quiver 1 */
		ObjectInfoIndex(62, 11, 31, 0x0040),   /* YEW STAFF      Quiver 1 */
		ObjectInfoIndex(63, 11, 32, 0x0040),   /* STAFF OF MANAR Quiver 1 Atari ST Version 1.0 1987-12-08: ObjectAspectIndex = 35 */
		ObjectInfoIndex(64, 51, 33, 0x0040),   /* SNAKE STAFF    Quiver 1 */
		ObjectInfoIndex(65, 32,  5, 0x0440),   /* THE CONDUIT    Quiver 1/Chest */
		ObjectInfoIndex(66, 30, 35, 0x0040),   /* DRAGON SPIT    Quiver 1 */
		ObjectInfoIndex(135, 65, 36, 0x0440),  /* SCEPTRE OF LYF Quiver 1/Chest */
		ObjectInfoIndex(143, 45, 27, 0x0040),  /* ROBE           Quiver 1 */
		ObjectInfoIndex(28, 82,  1, 0x0040),   /* FINE ROBE      Quiver 1 */
		ObjectInfoIndex(80, 23,  0, 0x040C),   /* KIRTLE         Neck/Torso/Chest */
		ObjectInfoIndex(81, 23,  0, 0x040C),   /* SILK SHIRT     Neck/Torso/Chest */
		ObjectInfoIndex(82, 23,  0, 0x0410),   /* ELVEN DOUBLET  Legs/Chest */
		ObjectInfoIndex(112, 55,  0, 0x0420),  /* LEATHER JERKIN Feet/Chest */
		ObjectInfoIndex(114,  8,  0, 0x0420),  /* TUNIC          Feet/Chest */
		ObjectInfoIndex(67, 24,  0, 0x0408),   /* GHI            Torso/Chest */
		ObjectInfoIndex(83, 24,  0, 0x0410),   /* MAIL AKETON    Legs/Chest */
		ObjectInfoIndex(68, 24,  0, 0x0408),   /* MITHRAL AKETON Torso/Chest */
		ObjectInfoIndex(84, 24,  0, 0x0410),   /* TORSO PLATE    Legs/Chest */
		ObjectInfoIndex(69, 69,  0, 0x0408),   /* PLATE OF LYTE  Torso/Chest */
		ObjectInfoIndex(70, 24,  0, 0x0408),   /* PLATE OF DARC  Torso/Chest */
		ObjectInfoIndex(85, 24,  0, 0x0410),   /* CAPE           Legs/Chest */
		ObjectInfoIndex(86, 69,  0, 0x0410),   /* CLOAK OF NIGHT Legs/Chest */
		ObjectInfoIndex(71,  7,  0, 0x0408),   /* BARBARIAN HIDE Torso/Chest */
		ObjectInfoIndex(87,  7,  0, 0x0410),   /* ROBE           Legs/Chest */
		ObjectInfoIndex(119, 57,  0, 0x0420),  /* FINE ROBE      Feet/Chest */
		ObjectInfoIndex(72, 23,  0, 0x0408),   /* TABARD         Torso/Chest */
		ObjectInfoIndex(88, 23,  0, 0x0410),   /* GUNNA          Legs/Chest */
		ObjectInfoIndex(113, 29,  0, 0x0420),  /* ELVEN HUKE     Feet/Chest */
		ObjectInfoIndex(89, 69,  0, 0x0410),   /* LEATHER PANTS  Legs/Chest */
		ObjectInfoIndex(73, 69,  0, 0x0408),   /* BLUE PANTS     Torso/Chest */
		ObjectInfoIndex(74, 24,  0, 0x0408),   /* GHI TROUSERS   Torso/Chest */
		ObjectInfoIndex(90, 24,  0, 0x0410),   /* LEG MAIL       Legs/Chest */
		ObjectInfoIndex(103, 53,  0, 0x0402),  /* MITHRAL MAIL   Head/Chest */
		ObjectInfoIndex(104, 53,  0, 0x0402),  /* LEG PLATE      Head/Chest */
		ObjectInfoIndex(96,  9,  0, 0x0402),   /* POLEYN OF LYTE Head/Chest */
		ObjectInfoIndex(97,  9,  0, 0x0402),   /* POLEYN OF DARC Head/Chest */
		ObjectInfoIndex(98,  9,  0, 0x0402),   /* BEZERKER HELM  Head/Chest */
		ObjectInfoIndex(105, 54, 41, 0x0400),  /* HELMET         Chest */
		ObjectInfoIndex(106, 54, 41, 0x0200),  /* BASINET        Hands */
		ObjectInfoIndex(108, 10, 41, 0x0200),  /* CASQUE 'N COIF Hands */
		ObjectInfoIndex(107, 54, 41, 0x0200),  /* ARMET          Hands */
		ObjectInfoIndex(75, 19,  0, 0x0408),   /* HELM OF LYTE   Torso/Chest */
		ObjectInfoIndex(91, 19,  0, 0x0410),   /* HELM OF DARC   Legs/Chest */
		ObjectInfoIndex(76, 19,  0, 0x0408),   /* CALISTA        Torso/Chest */
		ObjectInfoIndex(92, 19,  0, 0x0410),   /* CROWN OF NERRA Legs/Chest */
		ObjectInfoIndex(99,  9,  0, 0x0402),   /* BUCKLER        Head/Chest */
		ObjectInfoIndex(115, 19,  0, 0x0420),  /* HIDE SHIELD    Feet/Chest */
		ObjectInfoIndex(100, 52,  0, 0x0402),  /* SMALL SHIELD   Head/Chest */
		ObjectInfoIndex(77, 20,  0, 0x0008),   /* WOODEN SHIELD  Torso */
		ObjectInfoIndex(93, 22,  0, 0x0010),   /* LARGE SHIELD   Legs */
		ObjectInfoIndex(116, 56,  0, 0x0420),  /* SHIELD OF LYTE Feet/Chest */
		ObjectInfoIndex(109, 10, 41, 0x0200),  /* SHIELD OF DARC Hands */
		ObjectInfoIndex(101, 52,  0, 0x0402),  /* SANDALS        Head/Chest */
		ObjectInfoIndex(78, 20,  0, 0x0008),   /* SUEDE BOOTS    Torso */
		ObjectInfoIndex(94, 22,  0, 0x0010),   /* LEATHER BOOTS  Legs */
		ObjectInfoIndex(117, 56,  0, 0x0420),  /* HOSEN          Feet/Chest */
		ObjectInfoIndex(110, 10, 41, 0x0200),  /* FOOT PLATE     Hands */
		ObjectInfoIndex(102, 52,  0, 0x0402),  /* GREAVE OF LYTE Head/Chest */
		ObjectInfoIndex(79, 20,  0, 0x0008),   /* GREAVE OF DARC Torso */
		ObjectInfoIndex(95, 22,  0, 0x0010),   /* ELVEN BOOTS    Legs */
		ObjectInfoIndex(118, 56,  0, 0x0420),  /* GEM OF AGES    Feet/Chest */
		ObjectInfoIndex(111, 10, 41, 0x0200),  /* EKKHARD CROSS  Hands */
		ObjectInfoIndex(140, 52,  0, 0x0402),  /* MOONSTONE      Head/Chest */
		ObjectInfoIndex(141, 19,  0, 0x0408),  /* THE HELLION    Torso/Chest */
		ObjectInfoIndex(142, 22,  0, 0x0010),  /* PENDANT FERAL  Legs */
		ObjectInfoIndex(194, 81,  0, 0x0420),  /* COPPER COIN    Feet/Chest */
		ObjectInfoIndex(196, 84,  0, 0x0408),  /* SILVER COIN    Torso/Chest */
		ObjectInfoIndex(0, 34,  0, 0x0500),    /* GOLD COIN      Pouch/Chest */
		ObjectInfoIndex(8,  6,  0, 0x0501),    /* BOULDER        Mouth/Pouch/Chest */
		ObjectInfoIndex(10, 15,  0, 0x0504),   /* BLUE GEM       Neck/Pouch/Chest */
		ObjectInfoIndex(12, 15,  0, 0x0504),   /* ORANGE GEM     Neck/Pouch/Chest */
		ObjectInfoIndex(146, 40,  0, 0x0500),  /* GREEN GEM      Pouch/Chest */
		ObjectInfoIndex(147, 41,  0, 0x0400),  /* MAGICAL BOX    Chest */
		ObjectInfoIndex(125,  4, 37, 0x0500),  /* MAGICAL BOX    Pouch/Chest */
		ObjectInfoIndex(126, 83, 37, 0x0500),  /* MIRROR OF DAWN Pouch/Chest */
		ObjectInfoIndex(127,  4, 37, 0x0500),  /* HORN OF FEAR   Pouch/Chest */
		ObjectInfoIndex(176, 18,  0, 0x0500),  /* ROPE           Pouch/Chest */
		ObjectInfoIndex(177, 18,  0, 0x0500),  /* RABBIT'S FOOT  Pouch/Chest */
		ObjectInfoIndex(178, 18,  0, 0x0500),  /* CORBAMITE      Pouch/Chest */
		ObjectInfoIndex(179, 18,  0, 0x0500),  /* CHOKER         Pouch/Chest */
		ObjectInfoIndex(180, 18,  0, 0x0500),  /* DEXHELM        Pouch/Chest */
		ObjectInfoIndex(181, 18,  0, 0x0500),  /* FLAMEBAIN      Pouch/Chest */
		ObjectInfoIndex(182, 18,  0, 0x0500),  /* POWERTOWERS    Pouch/Chest */
		ObjectInfoIndex(183, 18,  0, 0x0500),  /* SPEEDBOW       Pouch/Chest */
		ObjectInfoIndex(184, 62,  0, 0x0500),  /* CHEST          Pouch/Chest */
		ObjectInfoIndex(185, 62,  0, 0x0500),  /* OPEN CHEST     Pouch/Chest */
		ObjectInfoIndex(186, 62,  0, 0x0500),  /* ASHES          Pouch/Chest */
		ObjectInfoIndex(187, 62,  0, 0x0500),  /* BONES          Pouch/Chest */
		ObjectInfoIndex(188, 62,  0, 0x0500),  /* MON POTION     Pouch/Chest */
		ObjectInfoIndex(189, 62,  0, 0x0500),  /* UM POTION      Pouch/Chest */
		ObjectInfoIndex(190, 62,  0, 0x0500),  /* DES POTION     Pouch/Chest */
		ObjectInfoIndex(191, 62,  0, 0x0500),  /* VEN POTION     Pouch/Chest */
		ObjectInfoIndex(128, 76,  0, 0x0200),  /* SAR POTION     Hands */
		ObjectInfoIndex(129,  3,  0, 0x0500),  /* ZO POTION      Pouch/Chest */
		ObjectInfoIndex(130, 60,  0, 0x0500),  /* ROS POTION     Pouch/Chest */
		ObjectInfoIndex(131, 61,  0, 0x0500),  /* KU POTION      Pouch/Chest */
		ObjectInfoIndex(168, 27,  0, 0x0501),  /* DANE POTION    Mouth/Pouch/Chest */
		ObjectInfoIndex(169, 28,  0, 0x0501),  /* NETA POTION    Mouth/Pouch/Chest */
		ObjectInfoIndex(170, 25,  0, 0x0501),  /* BRO POTION     Mouth/Pouch/Chest */
		ObjectInfoIndex(171, 26,  0, 0x0501),  /* MA POTION      Mouth/Pouch/Chest */
		ObjectInfoIndex(172, 71,  0, 0x0401),  /* YA POTION      Mouth/Chest */
		ObjectInfoIndex(173, 70,  0, 0x0401),  /* EE POTION      Mouth/Chest */
		ObjectInfoIndex(174,  5,  0, 0x0501),  /* VI POTION      Mouth/Pouch/Chest */
		ObjectInfoIndex(175, 66,  0, 0x0501),  /* WATER FLASK    Mouth/Pouch/Chest */
		ObjectInfoIndex(120, 15,  0, 0x0504),  /* KATH BOMB      Neck/Pouch/Chest */
		ObjectInfoIndex(121, 15,  0, 0x0504),  /* PEW BOMB       Neck/Pouch/Chest */
		ObjectInfoIndex(122, 58,  0, 0x0504),  /* RA BOMB        Neck/Pouch/Chest */
		ObjectInfoIndex(123, 59,  0, 0x0504),  /* FUL BOMB       Neck/Pouch/Chest */
		ObjectInfoIndex(124, 59,  0, 0x0504),  /* APPLE          Neck/Pouch/Chest */
		ObjectInfoIndex(132, 79, 38, 0x0500),  /* CORN           Pouch/Chest */
		ObjectInfoIndex(133, 63, 38, 0x0500),  /* BREAD          Pouch/Chest */
		ObjectInfoIndex(134, 64,  0, 0x0500),  /* CHEESE         Pouch/Chest */
		ObjectInfoIndex(136, 72, 39, 0x0400),  /* SCREAMER SLICE Chest */
		ObjectInfoIndex(137, 73,  0, 0x0500),  /* WORM ROUND     Pouch/Chest */
		ObjectInfoIndex(138, 74,  0, 0x0500),  /* DRUMSTICK      Pouch/Chest */
		ObjectInfoIndex(139, 75,  0, 0x0504),  /* DRAGON STEAK   Neck/Pouch/Chest */
		ObjectInfoIndex(192, 77,  0, 0x0500),  /* IRON KEY       Pouch/Chest */
		ObjectInfoIndex(193, 78,  0, 0x0500),  /* KEY OF B       Pouch/Chest */
		ObjectInfoIndex(197, 74,  0, 0x0000),  /* SOLID KEY */
		ObjectInfoIndex(198, 41,  0, 0x0400)   /* SQUARE KEY     Chest */
	};
	ArmourInfo armourInfo[58] = { // G0239_as_Graphic559_ArmourInfo
		/* { Weight, Defense, Attributes, Unreferenced } */
		ArmourInfo(3,   5, 0x01),   /* CAPE */
		ArmourInfo(4,  10, 0x01),   /* CLOAK OF NIGHT */
		ArmourInfo(3,   4, 0x01),   /* BARBARIAN HIDE */
		ArmourInfo(6,   5, 0x02),   /* SANDALS */
		ArmourInfo(16,  25, 0x04),  /* LEATHER BOOTS */
		ArmourInfo(4,   5, 0x00),   /* ROBE */
		ArmourInfo(4,   5, 0x00),   /* ROBE */
		ArmourInfo(3,   7, 0x01),   /* FINE ROBE */
		ArmourInfo(3,   7, 0x01),   /* FINE ROBE */
		ArmourInfo(4,   6, 0x01),   /* KIRTLE */
		ArmourInfo(2,   4, 0x00),   /* SILK SHIRT */
		ArmourInfo(4,   5, 0x01),   /* TABARD */
		ArmourInfo(5,   7, 0x01),   /* GUNNA */
		ArmourInfo(3,  11, 0x02),   /* ELVEN DOUBLET */
		ArmourInfo(3,  13, 0x02),   /* ELVEN HUKE */
		ArmourInfo(4,  13, 0x02),   /* ELVEN BOOTS */
		ArmourInfo(6,  17, 0x03),   /* LEATHER JERKIN */
		ArmourInfo(8,  20, 0x03),   /* LEATHER PANTS */
		ArmourInfo(14,  20, 0x03),  /* SUEDE BOOTS */
		ArmourInfo(6,  12, 0x02),   /* BLUE PANTS */
		ArmourInfo(5,   9, 0x01),   /* TUNIC */
		ArmourInfo(5,   8, 0x01),   /* GHI */
		ArmourInfo(5,   9, 0x01),   /* GHI TROUSERS */
		ArmourInfo(4,   1, 0x04),   /* CALISTA */
		ArmourInfo(6,   5, 0x04),   /* CROWN OF NERRA */
		ArmourInfo(11,  12, 0x05),  /* BEZERKER HELM */
		ArmourInfo(14,  17, 0x05),  /* HELMET */
		ArmourInfo(15,  20, 0x05),  /* BASINET */
		ArmourInfo(11,  22, 0x85),  /* BUCKLER */
		ArmourInfo(10,  16, 0x82),  /* HIDE SHIELD */
		ArmourInfo(14,  20, 0x83),  /* WOODEN SHIELD */
		ArmourInfo(21,  35, 0x84),  /* SMALL SHIELD */
		ArmourInfo(65,  35, 0x05),  /* MAIL AKETON */
		ArmourInfo(53,  35, 0x05),  /* LEG MAIL */
		ArmourInfo(52,  70, 0x07),  /* MITHRAL AKETON */
		ArmourInfo(41,  55, 0x07),  /* MITHRAL MAIL */
		ArmourInfo(16,  25, 0x06),  /* CASQUE 'N COIF */
		ArmourInfo(16,  30, 0x06),  /* HOSEN */
		ArmourInfo(19,  40, 0x07),  /* ARMET */
		ArmourInfo(120,  65, 0x04), /* TORSO PLATE */
		ArmourInfo(80,  56, 0x04),  /* LEG PLATE */
		ArmourInfo(28,  37, 0x05),  /* FOOT PLATE */
		ArmourInfo(34,  56, 0x84),  /* LARGE SHIELD */
		ArmourInfo(17,  62, 0x05),  /* HELM OF LYTE */
		ArmourInfo(108, 125, 0x04), /* PLATE OF LYTE */
		ArmourInfo(72,  90, 0x04),  /* POLEYN OF LYTE */
		ArmourInfo(24,  50, 0x05),  /* GREAVE OF LYTE */
		ArmourInfo(30,  85, 0x84),  /* SHIELD OF LYTE */
		ArmourInfo(35,  76, 0x04),  /* HELM OF DARC */
		ArmourInfo(141, 160, 0x04), /* PLATE OF DARC */
		ArmourInfo(90, 101, 0x04),  /* POLEYN OF DARC */
		ArmourInfo(31,  60, 0x05),  /* GREAVE OF DARC */
		ArmourInfo(40, 100, 0x84),  /* SHIELD OF DARC */
		ArmourInfo(14,  54, 0x06),  /* DEXHELM */
		ArmourInfo(57,  60, 0x07),  /* FLAMEBAIN */
		ArmourInfo(81,  88, 0x04),  /* POWERTOWERS */
		ArmourInfo(3,  16, 0x02),   /* BOOTS OF SPEED */
		ArmourInfo(2,   3, 0x03)    /* HALTER */
	};

	WeaponInfo weaponInfo[46] = { // @ G0238_as_Graphic559_WeaponInfo
		/* { Weight, Class, Strength, KineticEnergy, Attributes } */
		WeaponInfo(1, 130,   2,   0, 0x2000),  /* EYE OF TIME */
		WeaponInfo(1, 131,   2,   0, 0x2000),  /* STORMRING */
		WeaponInfo(11, 0,   8,   2, 0x2000),   /* TORCH */
		WeaponInfo(12, 112,  10,  80, 0x2028), /* FLAMITT */
		WeaponInfo(9, 129,  16,   7, 0x2000),  /* STAFF OF CLAWS */
		WeaponInfo(30, 113,  49, 110, 0x0942), /* BOLT BLADE */
		WeaponInfo(47, 0,  55,  20, 0x0900),   /* FURY */
		WeaponInfo(24, 255,  25,  10, 0x20FF), /* THE FIRESTAFF */
		WeaponInfo(5, 2,  10,  19, 0x0200),    /* DAGGER */
		WeaponInfo(33, 0,  30,   8, 0x0900),   /* FALCHION */
		WeaponInfo(32, 0,  34,  10, 0x0900),   /* SWORD */
		WeaponInfo(26, 0,  38,  10, 0x0900),   /* RAPIER */
		WeaponInfo(35, 0,  42,  11, 0x0900),   /* SABRE */
		WeaponInfo(36, 0,  46,  12, 0x0900),   /* SAMURAI SWORD */
		WeaponInfo(33, 0,  50,  14, 0x0900),   /* DELTA */
		WeaponInfo(37, 0,  62,  14, 0x0900),   /* DIAMOND EDGE */
		WeaponInfo(30, 0,  48,  13, 0x0000),   /* VORPAL BLADE */
		WeaponInfo(39, 0,  58,  15, 0x0900),   /* THE INQUISITOR */
		WeaponInfo(43, 2,  49,  33, 0x0300),   /* AXE */
		WeaponInfo(65, 2,  70,  44, 0x0300),   /* HARDCLEAVE */
		WeaponInfo(31, 0,  32,  10, 0x2000),   /* MACE */
		WeaponInfo(41, 0,  42,  13, 0x2000),   /* MACE OF ORDER */
		WeaponInfo(50, 0,  60,  15, 0x2000),   /* MORNINGSTAR */
		WeaponInfo(36, 0,  19,  10, 0x2700),   /* CLUB */
		WeaponInfo(110, 0,  44,  22, 0x2600),  /* STONE CLUB */
		WeaponInfo(10, 20,   1,  50, 0x2032),  /* BOW */
		WeaponInfo(28, 30,   1, 180, 0x2078),  /* CROSSBOW */
		WeaponInfo(2, 10,   2,  10, 0x0100),   /* ARROW */
		WeaponInfo(2, 10,   2,  28, 0x0500),   /* SLAYER */
		WeaponInfo(19, 39,   5,  20, 0x2032),  /* SLING */
		WeaponInfo(10, 11,   6,  18, 0x2000),  /* ROCK */
		WeaponInfo(3, 12,   7,  23, 0x0800),   /* POISON DART */
		WeaponInfo(1, 1,   3,  19, 0x0A00),    /* THROWING STAR */
		WeaponInfo(8, 0,   4,   4, 0x2000),    /* STICK */
		WeaponInfo(26, 129,  12,   4, 0x2000), /* STAFF */
		WeaponInfo(1, 130,   0,   0, 0x2000),  /* WAND */
		WeaponInfo(2, 140,   1,  20, 0x2000),  /* TEOWAND */
		WeaponInfo(35, 128,  18,   6, 0x2000), /* YEW STAFF */
		WeaponInfo(29, 159,   0,   4, 0x2000), /* STAFF OF MANAR */
		WeaponInfo(21, 131,   0,   3, 0x2000), /* SNAKE STAFF */
		WeaponInfo(33, 136,   0,   7, 0x2000), /* THE CONDUIT */
		WeaponInfo(8, 132,   3,   1, 0x2000),  /* DRAGON SPIT */
		WeaponInfo(18, 131,   9,   4, 0x2000), /* SCEPTRE OF LYF */
		WeaponInfo(8, 192,   1,   1, 0x2000),  /* HORN OF FEAR */
		WeaponInfo(30, 26,   1, 220, 0x207D),  /* SPEEDBOW */
		WeaponInfo(36, 255, 100,  50, 0x20FF)  /* THE FIRESTAFF */
	};

	CreatureInfo creatureInfo[k27_CreatureTypeCount] = { // @ G0243_as_Graphic559_CreatureInfo
	/* { CreatureAspectIndex, AttackSoundOrdinal, Attributes, GraphicInfo,
		MovementTicks, AttackTicks, Defense, BaseHealth, Attack, PoisonAttack,
		Dexterity, Ranges, Properties, Resistances, AnimationTicks, WoundProbabilities, AttackType } */
		{0,  4, 0x0482, 0x623D,   8, 20,  55, 150, 150, 240,  55, 0x1153, 0x299B, 0x0876, 0x0254, 0xFD40, 4},
		{1,  0, 0x0480, 0xA625,  15, 32,  20, 110,  80,  15,  20, 0x3132, 0x33A9, 0x0E42, 0x0384, 0xFC41, 3},
		{2,  6, 0x0510, 0x6198,   3,  5,  50,  10,  10,   0, 110, 0x1376, 0x710A, 0x0235, 0x0222, 0xFD20, 0},
		{3,  0, 0x04B4, 0xB225,  10, 21,  30,  40,  58,   0,  80, 0x320A, 0x96AA, 0x0B3C, 0x0113, 0xF910, 5},
		{4,  1, 0x0701, 0xA3B8,   9,  8,  45, 101,  90,   0,  65, 0x1554, 0x58FF, 0x0A34, 0x0143, 0xFE93, 4},
		{5,  0, 0x0581, 0x539D,  20, 18, 100,  60,  30,   0,  30, 0x1232, 0x4338, 0x0583, 0x0265, 0xFFD6, 3},
		{6,  3, 0x070C, 0x0020, 120, 10,   5, 165,   5,   0,   5, 0x1111, 0x10F1, 0x0764, 0x02F2, 0xFC84, 6},
		{7,  7, 0x0300, 0x0220, 185, 15, 170,  50,  40,   5,  10, 0x1463, 0x25C4, 0x06E3, 0x01F4, 0xFD93, 4}, /* Atari ST: AttackSoundOrdinal = 0 */
		{8,  2, 0x1864, 0x5225,  11, 16,  15,  30,  55,   0,  80, 0x1423, 0x4664, 0x0FC8, 0x0116, 0xFB30, 6},
		{9, 10, 0x0282, 0x71B8,  21, 14, 240, 120, 219,   0,  35, 0x1023, 0x3BFF, 0x0FF7, 0x04F3, 0xF920, 3}, /* Atari ST: AttackSoundOrdinal = 7 */
		{10,  2, 0x1480, 0x11B8,  17, 12,  25,  33,  20,   0,  40, 0x1224, 0x5497, 0x0F15, 0x0483, 0xFB20, 3},
		{11,  0, 0x18C6, 0x0225, 255,  8,  45,  80, 105,   0,  60, 0x1314, 0x55A5, 0x0FF9, 0x0114, 0xFD95, 1},
		{12, 11, 0x1280, 0x6038,   7,  7,  22,  20,  22,   0,  80, 0x1013, 0x6596, 0x0F63, 0x0132, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 8 */
		{13,  9, 0x14A2, 0xB23D,   5, 10,  42,  39,  90, 100,  88, 0x1343, 0x5734, 0x0638, 0x0112, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 0 */
		{14,  0, 0x05B8, 0x1638,  10, 20,  47,  44,  75,   0,  90, 0x4335, 0xD952, 0x035B, 0x0664, 0xFD60, 5},
		{15,  5, 0x0381, 0x523D,  18, 19,  72,  70,  45,  35,  35, 0x1AA1, 0x15AB, 0x0B93, 0x0253, 0xFFC5, 4},
		{16, 10, 0x0680, 0xA038,  13,  8,  28,  20,  25,   0,  41, 0x1343, 0x2148, 0x0321, 0x0332, 0xFC30, 3}, /* Atari ST: AttackSoundOrdinal = 7 */
		{17,  0, 0x04A0, 0xF23D,   1, 16, 180,   8,  28,  20, 150, 0x1432, 0x19FD, 0x0004, 0x0112, 0xF710, 4},
		{18, 11, 0x0280, 0xA3BD,  14,  6, 140,  60, 105,   0,  70, 0x1005, 0x7AFF, 0x0FFA, 0x0143, 0xFA30, 4}, /* Atari ST: AttackSoundOrdinal = 8 */
		{19,  0, 0x0060, 0xE23D,   5, 18,  15,  33,  61,   0,  65, 0x3258, 0xAC77, 0x0F56, 0x0117, 0xFC40, 5},
		{20,  8, 0x10DE, 0x0225,  25, 25,  75, 144,  66,   0,  50, 0x1381, 0x7679, 0x0EA7, 0x0345, 0xFD93, 3}, /* Atari ST: AttackSoundOrdinal = 0 */
		{21,  3, 0x0082, 0xA3BD,   7, 15,  33,  77, 130,   0,  60, 0x1592, 0x696A, 0x0859, 0x0224, 0xFC30, 4},
		{22,  0, 0x1480, 0x53BD,  10, 14,  68, 100, 100,   0,  75, 0x4344, 0xBDF9, 0x0A5D, 0x0124, 0xF920, 3},
		{23,  0, 0x38AA, 0x0038,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5},
		{24,  1, 0x068A, 0x97BD,  13, 28, 110, 255, 255,   0,  70, 0x3645, 0xBF7C, 0x06CD, 0x0445, 0xFC30, 4}, /* Atari ST Version 1.0 1987-12-08 1987-12-11: Ranges = 0x2645 */
		{25,  0, 0x38AA, 0x0000,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5},
		{26,  0, 0x38AA, 0x0000,  12, 22, 255, 180, 210,   0, 130, 0x6369, 0xFF37, 0x0FBF, 0x0564, 0xFB52, 5}
	};
	// this is the number of uint16s the data has to be stored, not the length of the data in dungeon.dat!
	byte thingDataWordCount[16] = { // @ G0235_auc_Graphic559_ThingDataByteCount
		2,   /* Door */
		3,   /* Teleporter */
		2,   /* Text String */
		4,   /* Sensor */
		9,   /* Group */
		2,   /* Weapon */
		2,   /* Armour */
		2,   /* Scroll */
		2,   /* Potion */
		4,   /* Container */
		2,   /* Junk */
		0,   /* Unused */
		0,   /* Unused */
		0,   /* Unused */
		5,   /* Projectile */
		2    /* Explosion */
	};

	for (int i = 0; i < 180; i++)
		_objectInfos[i] = objectInfo[i];

	for (int i = 0; i < 58; i++)
		_armourInfos[i] = armourInfo[i];

	for (int i = 0; i < 46; i++)
		_weaponInfos[i] = weaponInfo[i];

	for (int i = 0; i < k27_CreatureTypeCount; i++)
		_creatureInfos[i] = creatureInfo[i];

	for (int i = 0; i < 16; i++)
		_thingDataWordCount[i] = thingDataWordCount[i];

}

DungeonMan::DungeonMan(DMEngine *dmEngine) : _vm(dmEngine) {
	_rawDunFileDataSize = 0;
	_rawDunFileData = nullptr;
	_dungeonColumCount = 0;
	_dungeonMapsFirstColumnIndex = nullptr;

	_dungeonColumnsCumulativeSquareThingCount = nullptr;
	_squareFirstThings = nullptr;
	_dungeonTextData = nullptr;

	_dungeonMapData = nullptr;
	_partyDir = (Direction)0;
	_partyMapX = 0;
	_partyMapY = 0;
	_partyMapIndex = 0;
	_currMapIndex = kDMMapIndexNone;
	_currMapData = nullptr;
	_currMap = nullptr;
	_currMapWidth = 0;
	_currMapHeight = 0;
	_currMapColCumulativeSquareFirstThingCount = nullptr;
	_dungeonMaps = nullptr;
	_dungeonRawMapData = nullptr;
	_currMapInscriptionWallOrnIndex = 0;
	for (uint16 i = 0; i < 6; ++i)
		_dungeonViewClickableBoxes[i].setToZero();
	_isFacingAlcove = false;
	_isFacingViAltar = false;
	_isFacingFountain = false;
	_squareAheadElement = (ElementType)0;

	_dungeonFileHeader._ornamentRandomSeed = 0;
	_dungeonFileHeader._rawMapDataSize = 0;
	_dungeonFileHeader._mapCount = 0;
	_dungeonFileHeader._textDataWordCount = 0;
	_dungeonFileHeader._partyStartLocation = 0;
	_dungeonFileHeader._squareFirstThingCount = 0;

	for (uint16 i = 0; i < 5; ++i)
		_pileTopObject[i] = Thing(0);
	for (uint16 i = 0; i < 2; ++i)
		_currMapDoorInfo[i].resetToZero();
	for (uint16 i = 0; i < 16; i++)
		_dungeonFileHeader._thingCounts[i] = 0;

	setupConstants();
}

DungeonMan::~DungeonMan() {
	delete[] _rawDunFileData;
	delete[] _dungeonMaps;
	delete[] _dungeonMapsFirstColumnIndex;
	delete[] _dungeonColumnsCumulativeSquareThingCount;
	delete[] _squareFirstThings;
	delete[] _dungeonTextData;
	delete[] _dungeonMapData;

	delete[] _dungeonRawMapData;
}

void DungeonMan::decompressDungeonFile() {
	Common::File f;
	if (_vm->isDemo())
		f.open("DemoDun.dat");
	else
		f.open("Dungeon.dat");

	if (!f.isOpen())
		error("Unable to open Dungeon.dat file");

	if (f.readUint16BE() == 0x8104) { // if dungeon is compressed
		_rawDunFileDataSize = f.readUint32BE();
		delete[] _rawDunFileData;
		_rawDunFileData = new byte[_rawDunFileDataSize];
		f.readUint16BE(); // discard
		byte common[4];
		for (uint16 i = 0; i < 4; ++i)
			common[i] = f.readByte();
		byte lessCommon[16];
		for (uint16 i = 0; i < 16; ++i)
			lessCommon[i] = f.readByte();

		// start unpacking
		uint32 uncompIndex = 0;
		uint8 bitsUsedInWord = 0;
		uint16 wordBuff = f.readUint16BE();
		uint8 bitsLeftInByte = 8;
		byte byteBuff = f.readByte();
		while (uncompIndex < _rawDunFileDataSize) {
			while (bitsUsedInWord != 0) {
				uint8 shiftVal;
				if (f.eos()) {
					shiftVal = bitsUsedInWord;
					wordBuff <<= shiftVal;
				} else {
					shiftVal = MIN(bitsLeftInByte, bitsUsedInWord);
					wordBuff <<= shiftVal;
					wordBuff |= (byteBuff >> (8 - shiftVal));
					byteBuff <<= shiftVal;
					bitsLeftInByte -= shiftVal;
					if (!bitsLeftInByte) {
						byteBuff = f.readByte();
						bitsLeftInByte = 8;
					}
				}
				bitsUsedInWord -= shiftVal;
			}
			if (((wordBuff >> 15) & 1) == 0) {
				_rawDunFileData[uncompIndex++] = common[(wordBuff >> 13) & 3];
				bitsUsedInWord += 3;
			} else if (((wordBuff >> 14) & 3) == 2) {
				_rawDunFileData[uncompIndex++] = lessCommon[(wordBuff >> 10) & 15];
				bitsUsedInWord += 6;
			} else if (((wordBuff >> 14) & 3) == 3) {
				_rawDunFileData[uncompIndex++] = (wordBuff >> 6) & 255;
				bitsUsedInWord += 10;
			}
		}
	} else { // read uncompressed Dungeon.dat
		f.seek(0);
		_rawDunFileDataSize = f.size();
		delete[] _rawDunFileData;
		_rawDunFileData = new byte[_rawDunFileDataSize];
		f.read(_rawDunFileData, _rawDunFileDataSize);
	}
	f.close();
}

void DungeonMan::loadDungeonFile(Common::InSaveFile *file) {
	static const byte additionalThingCounts[16] = { // @ G0236_auc_Graphic559_AdditionalThingCounts{
		0,    /* Door */
		0,    /* Teleporter */
		0,    /* Text String */
		0,    /* Sensor */
		75,   /* Group */
		100,  /* Weapon */
		120,  /* Armour */
		0,    /* Scroll */
		5,    /* Potion */
		0,    /* Container */
		140,  /* Junk */
		0,    /* Unused */
		0,    /* Unused */
		0,    /* Unused */
		60,   /* Projectile */
		50    /* Explosion */
	};

	Timeline &timeline = *_vm->_timeline;
	if (_vm->_gameMode != kDMModeLoadSavedGame)
		decompressDungeonFile();

	Common::ReadStream *dunDataStream = nullptr;
	if (file) {
		// if loading a save
		dunDataStream = file;
	} else {
		// else read dungeon.dat
		assert(_rawDunFileData && _rawDunFileDataSize);
		dunDataStream = new Common::MemoryReadStream(_rawDunFileData, _rawDunFileDataSize, DisposeAfterUse::NO);
	}

	// initialize _g278_dungeonFileHeader
	_dungeonFileHeader._ornamentRandomSeed = dunDataStream->readUint16BE();
	_dungeonFileHeader._rawMapDataSize = dunDataStream->readUint16BE();
	_dungeonFileHeader._mapCount = dunDataStream->readByte();
	dunDataStream->readByte(); // discard 1 byte
	_dungeonFileHeader._textDataWordCount = dunDataStream->readUint16BE();
	_dungeonFileHeader._partyStartLocation = dunDataStream->readUint16BE();
	_dungeonFileHeader._squareFirstThingCount = dunDataStream->readUint16BE();
	for (uint16 i = 0; i < kDMThingTypeTotal; ++i)
		_dungeonFileHeader._thingCounts[i] = dunDataStream->readUint16BE();

	// init party position and mapindex
	if (_vm->_gameMode != kDMModeLoadSavedGame) {
		uint16 startLoc = _dungeonFileHeader._partyStartLocation;
		_partyDir = (Direction)((startLoc >> 10) & 3);
		_partyMapX = startLoc & 0x1F;
		_partyMapY = (startLoc >> 5) & 0x1F;
		_partyMapIndex = 0;
	}

	// load map data
	if (!_vm->_restartGameRequest) {
		delete[] _dungeonMaps;
		_dungeonMaps = new Map[_dungeonFileHeader._mapCount];
	}

	for (uint16 i = 0; i < _dungeonFileHeader._mapCount; ++i) {
		_dungeonMaps[i]._rawDunDataOffset = dunDataStream->readUint16BE();
		dunDataStream->readUint32BE(); // discard 4 bytes
		_dungeonMaps[i]._offsetMapX = dunDataStream->readByte();
		_dungeonMaps[i]._offsetMapY = dunDataStream->readByte();

		uint16 tmp = dunDataStream->readUint16BE();
		_dungeonMaps[i]._height = tmp >> 11;
		_dungeonMaps[i]._width = (tmp >> 6) & 0x1F;
		_dungeonMaps[i]._level = tmp & 0x3F; // Only used in DMII

		tmp = dunDataStream->readUint16BE();
		_dungeonMaps[i]._randFloorOrnCount = tmp >> 12;
		_dungeonMaps[i]._floorOrnCount = (tmp >> 8) & 0xF;
		_dungeonMaps[i]._randWallOrnCount = (tmp >> 4) & 0xF;
		_dungeonMaps[i]._wallOrnCount = tmp & 0xF;

		tmp = dunDataStream->readUint16BE();
		_dungeonMaps[i]._difficulty = tmp >> 12;
		_dungeonMaps[i]._creatureTypeCount = (tmp >> 4) & 0xF;
		_dungeonMaps[i]._doorOrnCount = tmp & 0xF;

		tmp = dunDataStream->readUint16BE();
		_dungeonMaps[i]._doorSet1 = (tmp >> 12) & 0xF;
		_dungeonMaps[i]._doorSet0 = (tmp >> 8) & 0xF;
		_dungeonMaps[i]._wallSet = (WallSet)((tmp >> 4) & 0xF);
		_dungeonMaps[i]._floorSet = (FloorSet)(tmp & 0xF);

		//if (!file)
		//	delete dunDataStream;
	}

	// load column stuff thingy
	if (!_vm->_restartGameRequest) {
		delete[] _dungeonMapsFirstColumnIndex;
		_dungeonMapsFirstColumnIndex = new uint16[_dungeonFileHeader._mapCount];
	}
	uint16 columCount = 0;
	for (uint16 i = 0; i < _dungeonFileHeader._mapCount; ++i) {
		_dungeonMapsFirstColumnIndex[i] = columCount;
		columCount += _dungeonMaps[i]._width + 1;
	}
	_dungeonColumCount = columCount;

	uint32 actualSquareFirstThingCount = _dungeonFileHeader._squareFirstThingCount;
	if (_vm->_gameMode != kDMModeLoadSavedGame)
		_dungeonFileHeader._squareFirstThingCount += 300;

	if (!_vm->_restartGameRequest) {
		delete[] _dungeonColumnsCumulativeSquareThingCount;
		_dungeonColumnsCumulativeSquareThingCount = new uint16[columCount];
	}
	for (uint16 i = 0; i < columCount; ++i)
		_dungeonColumnsCumulativeSquareThingCount[i] = dunDataStream->readUint16BE();

	// load square first things
	if (!_vm->_restartGameRequest) {
		delete[] _squareFirstThings;
		_squareFirstThings = new Thing[_dungeonFileHeader._squareFirstThingCount];
	}

	for (uint16 i = 0; i < actualSquareFirstThingCount; ++i)
		_squareFirstThings[i].set(dunDataStream->readUint16BE());

	if (_vm->_gameMode != kDMModeLoadSavedGame) {
		for (uint16 i = 0; i < 300; ++i)
			_squareFirstThings[actualSquareFirstThingCount + i] = _vm->_thingNone;
	}

	// load text data
	if (!_vm->_restartGameRequest) {
		delete[] _dungeonTextData;
		_dungeonTextData = new uint16[_dungeonFileHeader._textDataWordCount];
	}

	for (uint16 i = 0; i < _dungeonFileHeader._textDataWordCount; ++i)
		_dungeonTextData[i] = dunDataStream->readUint16BE();

	if (_vm->_gameMode != kDMModeLoadSavedGame)
		timeline._eventMaxCount = 100;

	// load things
	for (uint16 thingType = kDMThingTypeDoor; thingType < kDMThingTypeTotal; ++thingType) {
		uint16 thingCount = _dungeonFileHeader._thingCounts[thingType];
		if (_vm->_gameMode != kDMModeLoadSavedGame)
			_dungeonFileHeader._thingCounts[thingType] = MIN((thingType == kDMThingTypeExplosion) ? 768 : 1024, thingCount + additionalThingCounts[thingType]);

		uint16 thingStoreWordCount = _thingDataWordCount[thingType];

		if (thingStoreWordCount == 0)
			continue;

		switch (thingType) {
		case kDMThingTypeDoor:
			_doors.clear();
			_doors.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeTeleporter:
			_teleporters.clear();
			_teleporters.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMstringTypeText:
			_textStrings.clear();
			_textStrings.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeSensor:
			_sensors.clear();
			_sensors.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeGroup:
			_groups.clear();
			_groups.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeWeapon:
			_weapons.clear();
			_weapons.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeArmour:
			_armours.clear();
			_armours.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeScroll:
			_scrolls.clear();
			_scrolls.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypePotion:
			_potions.clear();
			_potions.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeContainer:
			_containers.clear();
			_containers.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeJunk:
			_junks.clear();
			_junks.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeProjectile:
			_projectiles.clear();
			_projectiles.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		case kDMThingTypeExplosion:
			_explosions.clear();
			_explosions.resize(_dungeonFileHeader._thingCounts[thingType]);
			break;
		}

		for (uint16 i = 0; i < thingCount; ++i) {
			switch (thingType) {
			case kDMThingTypeDoor: {
				Door &door = _doors[i];
				door._nextThing = Thing(dunDataStream->readUint16BE());
				door._attributes = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeTeleporter: {
				Teleporter &tele = _teleporters[i];
				tele._nextThing = Thing(dunDataStream->readUint16BE());
				tele._attributes = dunDataStream->readUint16BE();
				tele._destMapIndex = dunDataStream->readUint16BE();
				break;
			}
			case kDMstringTypeText: {
				TextString &text = _textStrings[i];
				text._nextThing = Thing(dunDataStream->readUint16BE());
				text._textDataRef = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeSensor: {
				Sensor &sens = _sensors[i];
				sens._nextThing = Thing(dunDataStream->readUint16BE());
				sens._datAndType = dunDataStream->readUint16BE();
				sens._attributes = dunDataStream->readUint16BE();
				sens._action = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeGroup: {
				Group &grp = _groups[i];
				grp._nextThing = Thing(dunDataStream->readUint16BE());
				grp._slot = Thing(dunDataStream->readUint16BE());
				if (!file) {
					grp._type = (CreatureType)dunDataStream->readByte();
					grp._cells = dunDataStream->readByte();
				} else {
					grp._type = (CreatureType)dunDataStream->readUint16BE();
					grp._cells = dunDataStream->readUint16BE();
				}
				grp._health[0] = dunDataStream->readUint16BE();
				grp._health[1] = dunDataStream->readUint16BE();
				grp._health[2] = dunDataStream->readUint16BE();
				grp._health[3] = dunDataStream->readUint16BE();
				grp._flags = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeWeapon: {
				Weapon &weap = _weapons[i];
				weap._nextThing = Thing(dunDataStream->readUint16BE());
				weap._desc = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeArmour: {
				Armour &arm = _armours[i];
				arm._nextThing = Thing(dunDataStream->readUint16BE());
				arm._attributes = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeScroll: {
				Scroll &scr = _scrolls[i];
				scr._nextThing = Thing(dunDataStream->readUint16BE());
				scr._attributes = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypePotion: {
				Potion &pot = _potions[i];
				pot._nextThing = Thing(dunDataStream->readUint16BE());
				pot._attributes = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeContainer: {
				Container &cont = _containers[i];
				cont._nextThing = Thing(dunDataStream->readUint16BE());
				cont._slot = Thing(dunDataStream->readUint16BE());
				cont._type = dunDataStream->readUint16BE();
				dunDataStream->readUint16BE(); // unused 4th word
				break;
			}
			case kDMThingTypeJunk: {
				Junk &jnk = _junks[i];
				jnk._nextThing = Thing(dunDataStream->readUint16BE());
				jnk._attributes = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeProjectile: {
				Projectile &proj = _projectiles[i];
				proj._nextThing = Thing(dunDataStream->readUint16BE());
				proj._slot = Thing(dunDataStream->readUint16BE());
				if (!file) {
					proj._kineticEnergy = dunDataStream->readByte();
					proj._attack = dunDataStream->readByte();
				} else {
					proj._kineticEnergy = dunDataStream->readUint16BE();
					proj._attack = dunDataStream->readUint16BE();
				}
				proj._eventIndex = dunDataStream->readUint16BE();
				break;
			}
			case kDMThingTypeExplosion: {
				Explosion &expl = _explosions[i];
				expl._nextThing = Thing(dunDataStream->readUint16BE());
				expl._attributes = dunDataStream->readUint16BE();
				break;
			}
			}
		}

		if (_vm->_gameMode != kDMModeLoadSavedGame) {
			if ((thingType == kDMThingTypeGroup) || thingType >= kDMThingTypeProjectile)
				timeline._eventMaxCount += _dungeonFileHeader._thingCounts[thingType];

			for (uint16 i = 0; i < additionalThingCounts[thingType]; ++i) {
				uint16 idx = thingCount + i;
				switch (thingType) {
				case kDMThingTypeDoor:
					_doors[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeTeleporter:
					_teleporters[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMstringTypeText:
					_textStrings[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeSensor:
					_sensors[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeGroup:
					_groups[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeWeapon:
					_weapons[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeArmour:
					_armours[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeScroll:
					_scrolls[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypePotion:
					_potions[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeContainer:
					_containers[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeJunk:
					_junks[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeProjectile:
					_projectiles[idx]._nextThing = _vm->_thingNone;
					break;
				case kDMThingTypeExplosion:
					_explosions[idx]._nextThing = _vm->_thingNone;
					break;
				}
			}
		}
	}

	// load map data
	if (!_vm->_restartGameRequest) {
		delete[] _dungeonRawMapData;
		_dungeonRawMapData = new byte[_dungeonFileHeader._rawMapDataSize];
	}

	for (uint32 i = 0; i < _dungeonFileHeader._rawMapDataSize; ++i)
		_dungeonRawMapData[i] = dunDataStream->readByte();

	if (!_vm->_restartGameRequest) {
		uint8 mapCount = _dungeonFileHeader._mapCount;
		delete[] _dungeonMapData;
		_dungeonMapData = new byte**[_dungeonColumCount + mapCount];
		byte **colFirstSquares = (byte **)_dungeonMapData + mapCount;
		for (uint8 i = 0; i < mapCount; ++i) {
			_dungeonMapData[i] = colFirstSquares;
			byte *square = _dungeonRawMapData + _dungeonMaps[i]._rawDunDataOffset;
			*colFirstSquares++ = square;
			for (uint16 w = 1; w <= _dungeonMaps[i]._width; ++w) {
				square += _dungeonMaps[i]._height + 1;
				*colFirstSquares++ = square;
			}
		}
	}

	if (!file) { // this means that we created a new MemoryReadStream
		delete dunDataStream;
	} // the deletion of the function parameter 'file' happens elsewhere
}

void DungeonMan::setCurrentMap(uint16 mapIndex) {
	static const DoorInfo doorInfo[4] = { // @ G0254_as_Graphic559_DoorInfo
		/* { Attributes, Defense } */
		DoorInfo(3, 110),   /* Door type 0 Portcullis */
		DoorInfo(0,  42),   /* Door type 1 Wooden door */
		DoorInfo(0, 230),   /* Door type 2 Iron door */
		DoorInfo(5, 255)    /* Door type 3 Ra door */
	};


	_currMapIndex = mapIndex;
	_currMapData = _dungeonMapData[mapIndex];
	_currMap = _dungeonMaps + mapIndex;
	_currMapWidth = _dungeonMaps[mapIndex]._width + 1;
	_currMapHeight = _dungeonMaps[mapIndex]._height + 1;
	_currMapDoorInfo[0] = doorInfo[_currMap->_doorSet0];
	_currMapDoorInfo[1] = doorInfo[_currMap->_doorSet1];
	_currMapColCumulativeSquareFirstThingCount = &_dungeonColumnsCumulativeSquareThingCount[_dungeonMapsFirstColumnIndex[mapIndex]];
}

void DungeonMan::setCurrentMapAndPartyMap(uint16 mapIndex) {
	DisplayMan &displMan = *_vm->_displayMan;

	setCurrentMap(_partyMapIndex = mapIndex);
	byte *metaMapData = _currMapData[_currMapWidth - 1] + _currMapHeight;

	displMan._currMapAllowedCreatureTypes = metaMapData;
	metaMapData += _currMap->_creatureTypeCount;

	memcpy(displMan._currMapWallOrnIndices, metaMapData, _currMap->_wallOrnCount);
	metaMapData += _currMap->_wallOrnCount;

	memcpy(displMan._currMapFloorOrnIndices, metaMapData, _currMap->_floorOrnCount);
	metaMapData += _currMap->_floorOrnCount;

	memcpy(displMan._currMapDoorOrnIndices, metaMapData, _currMap->_doorOrnCount);

	_currMapInscriptionWallOrnIndex = _currMap->_wallOrnCount;
	displMan._currMapWallOrnIndices[_currMapInscriptionWallOrnIndex] = k0_WallOrnInscription;
}


Square DungeonMan::getSquare(int16 mapX, int16 mapY) {
	bool isMapYInBounds = (mapY >= 0) && (mapY < _currMapHeight);
	bool isMapXInBounds = (mapX >= 0) && (mapX < _currMapWidth);

	if (isMapXInBounds && isMapYInBounds)
		return Square(_currMapData[mapX][mapY]);

	if (isMapYInBounds) {
		ElementType squareType = Square(_currMapData[0][mapY]).getType();
		if (((mapX == -1) && (squareType == kDMElementTypeCorridor)) || (squareType == kDMElementTypePit))
			return Square(kDMElementTypeWall, kDMSquareMaskWallEastRandOrnament);

		squareType = Square(_currMapData[_currMapWidth - 1][mapY]).getType();
		if (((mapX == _currMapWidth) && (squareType == kDMElementTypeCorridor)) || (squareType == kDMElementTypePit))
			return Square(kDMElementTypeWall, kDMSquareMaskWallWestRandOrnament);
	} else if (isMapXInBounds) {
		ElementType squareType = Square(_currMapData[mapX][0]).getType();
		if (((mapY == -1) && (squareType == kDMElementTypeCorridor)) || (squareType == kDMElementTypePit))
			return Square(kDMElementTypeWall, kDMSquareMaslWallSouthRandOrnament);

		squareType = Square(_currMapData[mapX][_currMapHeight - 1]).getType();
		if (((mapY == _currMapHeight) && (squareType == kDMElementTypeCorridor)) || (squareType == kDMElementTypePit))
			return Square(kDMElementTypeWall, kDMSquareMaskWallNorthRandOrnament);
	}
	return Square(kDMElementTypeWall, 0);
}

Square DungeonMan::getRelSquare(Direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
	mapCoordsAfterRelMovement(dir, stepsForward, stepsForward, posX, posY);
	return getSquare(posX, posY);
}

int16 DungeonMan::getSquareFirstThingIndex(int16 mapX, int16 mapY) {
	if ((mapX < 0) || (mapX >= _currMapWidth) || (mapY < 0) || (mapY >= _currMapHeight))
		return -1;

	unsigned char *curSquare = _currMapData[mapX];
	if (!getFlag(curSquare[mapY], kDMSquareMaskThingListPresent))
		return -1;

	int16 curMapY = 0;
	uint16 thingIndex = _currMapColCumulativeSquareFirstThingCount[mapX];
	while (curMapY++ != mapY) {
		if (getFlag(*curSquare++, kDMSquareMaskThingListPresent))
			thingIndex++;
	}
	return thingIndex;
}

Thing DungeonMan::getSquareFirstThing(int16 mapX, int16 mapY) {
	int16 index = getSquareFirstThingIndex(mapX, mapY);
	if (index == -1)
		return _vm->_thingEndOfList;
	return _squareFirstThings[index];
}

void DungeonMan::setSquareAspect(uint16 *aspectArray, Direction dir, int16 mapX, int16 mapY) {
	unsigned char L0307_uc_Multiple;
#define AL0307_uc_Square            L0307_uc_Multiple
#define AL0307_uc_FootprintsAllowed L0307_uc_Multiple
#define AL0307_uc_ScentOrdinal      L0307_uc_Multiple

	DisplayMan &displMan = *_vm->_displayMan;
	ChampionMan &championMan = *_vm->_championMan;

	for (uint16 i = 0; i < 5; ++i)
		aspectArray[i] = 0;

	Thing curThing = getSquareFirstThing(mapX, mapY);
	AL0307_uc_Square = getSquare(mapX, mapY).toByte();
	bool leftRandomWallOrnamentAllowed = false;
	bool rightRandomWallOrnamentAllowed = false;
	bool frontRandomWallOrnamentAllowed = false;
	bool squareIsFakeWall;

	aspectArray[kDMSquareAspectElement] = Square(AL0307_uc_Square).getType();
	switch (aspectArray[kDMSquareAspectElement]) {
	case kDMElementTypeWall:
		switch (dir) {
		case kDMDirNorth:
			leftRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallEastRandOrnament);
			frontRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaslWallSouthRandOrnament);
			rightRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallWestRandOrnament);
			break;
		case kDMDirEast:
			leftRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaslWallSouthRandOrnament);
			frontRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallWestRandOrnament);
			rightRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallNorthRandOrnament);
			break;
		case kDMDirSouth:
			leftRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallWestRandOrnament);
			frontRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallNorthRandOrnament);
			rightRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallEastRandOrnament);
			break;
		case kDMDirWest:
			leftRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallNorthRandOrnament);
			frontRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskWallEastRandOrnament);
			rightRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaslWallSouthRandOrnament);
			break;
		default:
			assert(false);
		}
		displMan._championPortraitOrdinal = 0;
		squareIsFakeWall = false;
T0172010_ClosedFakeWall:
		setSquareAspectOrnOrdinals(aspectArray, leftRandomWallOrnamentAllowed, frontRandomWallOrnamentAllowed, rightRandomWallOrnamentAllowed, dir, mapX, mapY, squareIsFakeWall);
		while ((curThing != _vm->_thingEndOfList) && (curThing.getType() <= kDMThingTypeSensor)) {
			ThingType curThingType = curThing.getType();
			int16 AL0310_i_SideIndex = _vm->normalizeModulo4(curThing.getCell() - dir);
			if (AL0310_i_SideIndex) { /* Invisible on the back wall if 0 */
				if (curThingType == kDMstringTypeText) {
					TextString *curText = getTextString(curThing);
					if (curText->isVisible()) {
						aspectArray[AL0310_i_SideIndex + 1] = _currMapInscriptionWallOrnIndex + 1;
						displMan._inscriptionThing = curThing; /* BUG0_76 The same text is drawn on multiple sides of a wall square. The engine stores only a single text to draw on a wall in a global variable. Even if different texts are placed on different sides of the wall, the same text is drawn on each affected side */
					}
				} else {
					Sensor *curSensor = getSensor(curThing);
					aspectArray[AL0310_i_SideIndex + 1] = curSensor->getAttrOrnOrdinal();
					if (curSensor->getType() == kDMSensorWallChampionPortrait) {
						displMan._championPortraitOrdinal = _vm->indexToOrdinal(curSensor->getData());
					}
				}
			}
			curThing = getNextThing(curThing);
		}
		if (squareIsFakeWall && (_partyMapX != mapX) && (_partyMapY != mapY)) {
			aspectArray[kDMSquareAspectFirstGroupOrObject] = _vm->_thingEndOfList.toUint16();
			return;
		}
		break;
	case kDMElementTypeFakeWall:
		if (!getFlag(AL0307_uc_Square, kDMSquareMaskFakeWallOpen)) {
			aspectArray[kDMSquareAspectElement] = kDMElementTypeWall;
			leftRandomWallOrnamentAllowed = rightRandomWallOrnamentAllowed = frontRandomWallOrnamentAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskFakeWallRandOrnamentOrFootprintsAllowed);
			squareIsFakeWall = true;
			goto T0172010_ClosedFakeWall;
		}
		aspectArray[kDMSquareAspectElement] = kDMElementTypeCorridor;
		AL0307_uc_FootprintsAllowed = getFlag(AL0307_uc_Square, kDMSquareMaskFakeWallRandOrnamentOrFootprintsAllowed) ? 8 : 0;
		// fall through
	case kDMElementTypeCorridor:
	case kDMElementTypePit:
	case kDMElementTypeTeleporter:
		if (aspectArray[kDMSquareAspectElement] == kDMElementTypeCorridor) {
			aspectArray[kDMSquareAspectFloorOrn] = getRandomOrnOrdinal(getFlag(AL0307_uc_Square, kDMSquareMaskCorridorRandOrnament), _currMap->_randFloorOrnCount, mapX, mapY, 30);
			AL0307_uc_FootprintsAllowed = true;
		} else if (aspectArray[kDMSquareAspectElement] == kDMElementTypePit) {
			if (getFlag(AL0307_uc_Square, kDMSquareMaskPitOpen)) {
				aspectArray[kDMSquareAspectPitInvisible] = getFlag(AL0307_uc_Square, kDMSquareMaskPitInvisible);
				AL0307_uc_FootprintsAllowed &= 0x0001;
			} else {
				aspectArray[kDMSquareAspectElement] = kDMElementTypeCorridor;
				AL0307_uc_FootprintsAllowed = true;
			}
		} else { // k5_ElementTypeTeleporter
			aspectArray[kDMSquareAspectTeleporterVisible] = getFlag(AL0307_uc_Square, kDMSquareMaskTeleporterOpen) && getFlag(AL0307_uc_Square, kDMSquareMaskTeleporterVisible);
			AL0307_uc_FootprintsAllowed = true;
		}

		while ((curThing != _vm->_thingEndOfList) && (curThing.getType() <= kDMThingTypeSensor)) {
			if (curThing.getType() == kDMThingTypeSensor) {
				Sensor *curSensor = getSensor(curThing);
				aspectArray[kDMSquareAspectFloorOrn] = curSensor->getAttrOrnOrdinal();
			}
			curThing = getNextThing(curThing);
		}

		if (AL0307_uc_FootprintsAllowed && (AL0307_uc_ScentOrdinal = championMan.getScentOrdinal(mapX, mapY)) && (--AL0307_uc_ScentOrdinal >= championMan._party._firstScentIndex) && (AL0307_uc_ScentOrdinal < championMan._party._lastScentIndex))
			setFlag(aspectArray[kDMSquareAspectFloorOrn], kDMMaskFootprints);

		break;
	case kDMElementTypeStairs:
		aspectArray[kDMSquareAspectElement] = (bool((getFlag(AL0307_uc_Square, kDMSquareMaskStairsNorthSouth) >> 3)) == _vm->isOrientedWestEast(dir)) ? kDMElementTypeStairsSide : kDMElementTypeStairsFront;
		aspectArray[kDMSquareAspectStairsUp] = getFlag(AL0307_uc_Square, kDMSquareMaskStairsUp);
		AL0307_uc_FootprintsAllowed = false;
		while ((curThing != _vm->_thingEndOfList) && (curThing.getType() <= kDMThingTypeSensor))
			curThing = getNextThing(curThing);
		break;
	case kDMElementTypeDoor:
		if (bool((getFlag(AL0307_uc_Square, (byte) kDMSquareMaskDoorNorthSouth) >> 3)) == _vm->isOrientedWestEast(dir)) {
			aspectArray[kDMSquareAspectElement] = kDMElementTypeDoorSide;
		} else {
			aspectArray[kDMSquareAspectElement] = kDMElementTypeDoorFront;
			aspectArray[kDMSquareAspectDoorState] = Square(AL0307_uc_Square).getDoorState();
			aspectArray[kDMSquareAspectDoorThingIndex] = getSquareFirstThing(mapX, mapY).getIndex();
		}
		AL0307_uc_FootprintsAllowed = true;

		while ((curThing != _vm->_thingEndOfList) && (curThing.getType() <= kDMThingTypeSensor))
			curThing = getNextThing(curThing);

		if ((AL0307_uc_ScentOrdinal = championMan.getScentOrdinal(mapX, mapY)) && (--AL0307_uc_ScentOrdinal >= championMan._party._firstScentIndex) && (AL0307_uc_ScentOrdinal < championMan._party._lastScentIndex))
			setFlag(aspectArray[kDMSquareAspectFloorOrn], kDMMaskFootprints);
		break;
	default:
		break;
	}
	aspectArray[kDMSquareAspectFirstGroupOrObject] = curThing.toUint16();
}

void DungeonMan::setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, int16 dir,
												 int16 mapX, int16 mapY, bool isFakeWall) {

	int16 randomWallOrnamentCount = _currMap->_randWallOrnCount;
	aspectArray[kDMSquareAspectRightWallOrnOrd] = getRandomOrnOrdinal(leftAllowed, randomWallOrnamentCount, mapX, ++mapY * (_vm->normalizeModulo4(++dir) + 1), 30);
	aspectArray[kDMSquareFrontWallOrnOrd] = getRandomOrnOrdinal(frontAllowed, randomWallOrnamentCount, mapX, mapY * (_vm->normalizeModulo4(++dir) + 1), 30);
	aspectArray[kDMSquareAspectLeftWallOrnOrd] = getRandomOrnOrdinal(rightAllowed, randomWallOrnamentCount, mapX, mapY-- * (_vm->normalizeModulo4(++dir) + 1), 30);
	if (isFakeWall || (mapX < 0) || (mapX >= _currMapWidth) || (mapY < 0) || (mapY >= _currMapHeight)) { /* If square is a fake wall or is out of map bounds */
		for (int16 sideIndex = kDMSquareAspectRightWallOrnOrd; sideIndex <= kDMSquareAspectLeftWallOrnOrd; sideIndex++) { /* Loop to remove any random ornament that is an alcove */
			if (isWallOrnAnAlcove(_vm->ordinalToIndex(aspectArray[sideIndex])))
				aspectArray[sideIndex] = 0;
		}
	}
}

int16 DungeonMan::getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo) {
	int16 randomOrnamentIndex = getRandomOrnamentIndex((int16)2000 + (mapX << 5) + mapY, (int16)3000 + (_currMapIndex << (int16)6) + _currMapWidth + _currMapHeight, modulo);

	if (allowed && (randomOrnamentIndex < count))
		return _vm->indexToOrdinal(randomOrnamentIndex);

	return 0;
}


bool DungeonMan::isWallOrnAnAlcove(int16 wallOrnIndex) {
	if (wallOrnIndex >= 0) {
		DisplayMan &displMan = *_vm->_displayMan;
		for (uint16 i = 0; i < k3_AlcoveOrnCount; ++i) {
			if (displMan._currMapAlcoveOrnIndices[i] == wallOrnIndex)
				return true;
		}
	}

	return false;
}

Door *DungeonMan::getDoor(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeDoor);
	return &(_doors[thing.getIndex()]);
}

Teleporter *DungeonMan::getTeleporter(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeTeleporter);
	return &(_teleporters[thing.getIndex()]);
}

TextString *DungeonMan::getTextString(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMstringTypeText);
	return &(_textStrings[thing.getIndex()]);
}

Sensor *DungeonMan::getSensor(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeSensor);
	return &(_sensors[thing.getIndex()]);
}

Group *DungeonMan::getGroup(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeGroup);
	return &(_groups[thing.getIndex()]);
}

Weapon *DungeonMan::getWeapon(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeWeapon);
	return &(_weapons[thing.getIndex()]);
}

Armour *DungeonMan::getArmour(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeArmour);
	return &(_armours[thing.getIndex()]);
}

Scroll *DungeonMan::getScroll(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeScroll);
	return &(_scrolls[thing.getIndex()]);
}

Potion *DungeonMan::getPotion(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypePotion);
	return &(_potions[thing.getIndex()]);
}

Container *DungeonMan::getContainer(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeContainer);
	return &(_containers[thing.getIndex()]);
}

Junk *DungeonMan::getJunk(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeJunk);
	return &(_junks[thing.getIndex()]);
}

Projectile *DungeonMan::getProjectile(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeProjectile);
	return &(_projectiles[thing.getIndex()]);
}

Explosion *DungeonMan::getExplosion(Thing thing) {
	if (thing == _vm->_thingNone || thing == _vm->_thingEndOfList)
		return nullptr;
	assert(thing.getType() == kDMThingTypeExplosion);
	return &(_explosions[thing.getIndex()]);
}

Thing DungeonMan::getNextThing(Thing thing) {
	if (thing == _vm->_thingNone)
		return _vm->_thingNone;
	if (thing == _vm->_thingEndOfList)
		return _vm->_thingEndOfList;

	switch (thing.getType()) {
	case kDMThingTypeDoor:
		return _doors[thing.getIndex()]._nextThing;
	case kDMThingTypeTeleporter:
		return _teleporters[thing.getIndex()]._nextThing;
	case kDMstringTypeText:
		return _textStrings[thing.getIndex()]._nextThing;
	case kDMThingTypeSensor:
		return _sensors[thing.getIndex()]._nextThing;
	case kDMThingTypeGroup:
		return _groups[thing.getIndex()]._nextThing;
	case kDMThingTypeWeapon:
		return _weapons[thing.getIndex()]._nextThing;
	case kDMThingTypeArmour:
		return _armours[thing.getIndex()]._nextThing;
	case kDMThingTypeScroll:
		return _scrolls[thing.getIndex()]._nextThing;
	case kDMThingTypePotion:
		return _potions[thing.getIndex()]._nextThing;
	case kDMThingTypeContainer:
		return _containers[thing.getIndex()]._nextThing;
	case kDMThingTypeJunk:
		return _junks[thing.getIndex()]._nextThing;
	case kDMThingTypeProjectile:
		return _projectiles[thing.getIndex()]._nextThing;
	case kDMThingTypeExplosion:
		return _explosions[thing.getIndex()]._nextThing;
	default:
		return _vm->_thingNone;
	}
}

void DungeonMan::decodeText(char *destString, size_t maxSize, Thing thing, int16 type) {
	static char messageAndScrollEscReplacementStrings[32][8] = { // @ G0255_aac_Graphic559_MessageAndScrollEscapeReplacementStrings
		{'x',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { '?',  0,  0,  0, 0, 0, 0, 0 }, */
		{'y',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { '!',  0,  0,  0, 0, 0, 0, 0 }, */
		{'T', 'H', 'E', ' ', 0, 0, 0, 0},
		{'Y', 'O', 'U', ' ', 0, 0, 0, 0},
		{'z',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{'{',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{'|',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{'}',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{'~',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{'',   0,   0,   0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0},
		{0,     0,   0,   0, 0, 0, 0, 0}
	};

	static char escReplacementCharacters[32][2] = { // @ G0256_aac_Graphic559_EscapeReplacementCharacters
		{'a', 0}, {'b', 0}, {'c', 0}, {'d', 0},
		{'e', 0}, {'f', 0}, {'g', 0}, {'h', 0},
		{'i', 0}, {'j', 0}, {'k', 0}, {'l', 0},
		{'m', 0}, {'n', 0}, {'o', 0}, {'p', 0},
		{'q', 0}, {'r', 0}, {'s', 0}, {'t', 0},
		{'u', 0}, {'v', 0}, {'w', 0}, {'x', 0},
		{'0', 0}, {'1', 0}, {'2', 0}, {'3', 0},
		{'4', 0}, {'5', 0}, {'6', 0}, {'7', 0}
	};

	static char inscriptionEscReplacementStrings[32][8] = { // @ G0257_aac_Graphic559_InscriptionEscapeReplacementStrings
		{28,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{29,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{19,  7,  4, 26, 0, 0, 0, 0},
		{24, 14, 20, 26, 0, 0, 0, 0},
		{30,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{31,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{32,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{33,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{34,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{35,  0,  0,  0, 0, 0, 0, 0}, /* Atari ST Version 1.0 1987-12-08 1987-12-11 1.1 1.2EN 1.2GE: { 0,  0,  0,  0, 0, 0, 0, 0 }, */
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0},
		{0,   0,  0,  0, 0, 0, 0, 0}
	};

	TextString &textString = *getTextString(thing);
	if ((textString.isVisible()) || (type & kDMMaskDecodeEvenIfInvisible)) {
		type &= ~kDMMaskDecodeEvenIfInvisible;
		char sepChar;
		if (type == kDMTextTypeMessage) {
			*destString++ = '\n';
			sepChar = ' ';
		} else if (type == kDMTextTypeInscription) {
			sepChar = (char)0x80;
		} else {
			sepChar = '\n';
		}
		uint16 codeCounter = 0;
		int16 escChar = 0;
		uint16 *codeWord = _dungeonTextData + textString.getWordOffset();
		uint16 code = 0, codes = 0;
		char *escReplString = nullptr;
		char *endDestString = destString + maxSize;
		for (; destString < endDestString; ) {
			if (!codeCounter) {
				codes = *codeWord++;
				code = (codes >> 10) & 0x1F;
			} else if (codeCounter == 1) {
				code = (codes >> 5) & 0x1F;
			} else {
				code = codes & 0x1F;
			}
			++codeCounter;
			codeCounter %= 3;

			if (escChar) {
				*destString = '\0';
				if (escChar == 30) {
					if (type != kDMTextTypeInscription)
						escReplString = messageAndScrollEscReplacementStrings[code];
					else
						escReplString = inscriptionEscReplacementStrings[code];
				} else
					escReplString = escReplacementCharacters[code];

				size_t ln = Common::strlcpy(destString, escReplString, endDestString - destString);
				destString += ln;
				escChar = 0;
			} else if (code < 28) {
				if (type != kDMTextTypeInscription) {
					if (code == 26)
						code = ' ';
					else if (code == 27)
						code = '.';
					else
						code += 'A';
				}
				*destString++ = code;
			} else if (code == 28)
				*destString++ = sepChar;
			else if (code <= 30)
				escChar = code;
			else
				break;
		}
		assert(destString < endDestString);
	}
	*destString = ((type == kDMTextTypeInscription) ? 0x81 : '\0');
}

Door *DungeonMan::getDoor(uint16 index) {
	return &(_doors[index]);
}

Teleporter *DungeonMan::getTeleporter(uint16 index) {
	return &(_teleporters[index]);
}

TextString *DungeonMan::getTextString(uint16 index) {
	return &(_textStrings[index]);
}

Sensor *DungeonMan::getSensor(uint16 index) {
	return &(_sensors[index]);
}

Group *DungeonMan::getGroup(uint16 index) {
	return &(_groups[index]);
}

Weapon *DungeonMan::getWeapon(uint16 index) {
	return &(_weapons[index]);
}

Armour *DungeonMan::getArmour(uint16 index) {
	return &(_armours[index]);
}

Scroll *DungeonMan::getScroll(uint16 index) {
	return &(_scrolls[index]);
}

Potion *DungeonMan::getPotion(uint16 index) {
	return &(_potions[index]);
}

Container *DungeonMan::getContainer(uint16 index) {
	return &(_containers[index]);
}

Junk *DungeonMan::getJunk(uint16 index) {
	return &(_junks[index]);
}

Projectile *DungeonMan::getProjectile(uint16 index) {
	return &(_projectiles[index]);
}

Explosion *DungeonMan::getExplosion(uint16 index) {
	return &(_explosions[index]);
}

Thing DungeonMan::getUnusedThing(uint16 thingType) {
	int16 thingCount = _dungeonFileHeader._thingCounts[getFlag(thingType, kDMMaskThingType)];
	if (thingType == (kDMMaskChampionBones | kDMThingTypeJunk)) {
		thingType = kDMThingTypeJunk;
	} else if (thingType == kDMThingTypeJunk)
		thingCount -= 3; /* Always keep 3 unused JUNK things for the bones of dead champions */

	Thing curThing;
	bool found = false;
	for (int16 i = 0; i < thingCount; ++i) {
		curThing = Thing((thingType << 10) | i);
		Thing *nextPtr = getNextThingPtr(curThing);
		if (*nextPtr == _vm->_thingNone) {
			found = true;
			break;
		}
	}

	if (!found) {
		curThing = getDiscardThing(thingType);
		if (curThing == _vm->_thingNone)
			return _vm->_thingNone;
	}

	resetThing(curThing);
	*getNextThingPtr(curThing) = _vm->_thingEndOfList;
	return curThing;
}

uint16 DungeonMan::getObjectWeight(Thing thing) {
	static const uint16 junkInfo[] = { // @ G0241_auc_Graphic559_JunkInfo
		// COMPASS - WATERSKIN - JEWEL SYMAL - ILLUMULET - ASHES
		1, 3, 2, 2, 4,
		// BONES - COPPER COIN - SILVER COIN - GOLD COIN - IRON KEY
		15, 1, 1, 1, 2,
		// KEY OF B - SOLID KEY - SQUARE KEY - TOURQUOISE KEY - CROSS KEY
		1, 1, 1, 1, 1,
		// ONYX KEY - SKELETON KEY - GOLD KEY - WINGED KEY - TOPAZ KEY
		1, 1, 1, 1, 1,
		// SAPPHIRE KEY - EMERALD KEY - RUBY KEY - RA KEY - MASTER KEY
		1, 1, 1, 1, 1,
		// BOULDER - BLUE GEM - ORANGE GEM - GREEN GEM - APPLE
		81, 2, 3, 2, 4,
		// CORN - BREAD - CHEESE - SCREAMER SLICE - WORM ROUND
		4, 3, 8, 5, 11,
		// DRUMSTICK - DRAGON STEAK - GEM OF AGES - EKKHARD CROSS - MOONSTONE
		4, 6, 2, 3, 2,
		// THE HELLION - PENDANT FERAL - MAGICAL BOX - MAGICAL BOX - MIRROR OF DAWN
		2, 2, 6, 9, 3,
		// ROPE - RABBIT'S FOOT - CORBAMITE - CHOKER - LOCK PICKS
		10, 1, 0, 1, 1,
		// MAGNIFIER - ZOKATHRA SPELL - BONES
		2, 0, 8
	};

	if (thing == _vm->_thingNone)
		return 0;

	// Initialization is not present in original
	// Set to 0 by default as it's the default value used for _vm->_none
	uint16 weight = 0;
	switch (thing.getType()) {
	case kDMThingTypeWeapon:
		weight = _weaponInfos[getWeapon(thing)->getType()]._weight;
		break;
	case kDMThingTypeArmour:
		weight = _armourInfos[getArmour(thing)->getType()]._weight;
		break;
	case kDMThingTypeJunk: {
		Junk *jnk = getJunk(thing);
		weight = junkInfo[jnk->getType()];
		if (jnk->getType() == kDMJunkTypeWaterskin)
			weight += jnk->getChargeCount() << 1;
		break;
	}
	case kDMThingTypeContainer:
		weight = 50;
		thing = getContainer(thing)->getSlot();
		while (thing != _vm->_thingEndOfList) {
			weight += getObjectWeight(thing);
			thing = getNextThing(thing);
		}
		break;
	case kDMThingTypePotion:
		if (getPotion(thing)->getType() == kDMPotionTypeEmptyFlask)
			weight = 1;
		else
			weight = 3;
		break;
	case kDMThingTypeScroll:
		weight = 1;
		break;
	default:
		break;
	}

	return weight; // this is garbage if none of the branches were taken
}

int16 DungeonMan::getObjectInfoIndex(Thing thing) {
	switch (thing.getType()) {
	case kDMThingTypeScroll:
		return kDMObjectInfoIndexFirstScroll;
	case kDMThingTypeContainer:
		return kDMObjectInfoIndexFirstContainer + getContainer(thing)->getType();
	case kDMThingTypeJunk:
		return kDMObjectInfoIndexFirstJunk + getJunk(thing)->getType();
	case kDMThingTypeWeapon:
		return kDMObjectInfoIndexFirstWeapon + getWeapon(thing)->getType();
	case kDMThingTypeArmour:
		return kDMObjectInfoIndexFirstArmour + getArmour(thing)->getType();
	case kDMThingTypePotion:
		return kDMObjectInfoIndexFirstPotion + getPotion(thing)->getType();
	default:
		return -1;
	}
}

void DungeonMan::linkThingToList(Thing thingToLink, Thing thingInList, int16 mapX, int16 mapY) {
	if (thingToLink == _vm->_thingEndOfList)
		return;

	*getNextThingPtr(thingToLink) = _vm->_thingEndOfList;
	/* If mapX >= 0 then the thing is linked to the list of things on the specified square else it is linked at the end of the specified thing list */
	if (mapX >= 0) {
		byte *currSquare = &_currMapData[mapX][mapY];
		if (getFlag(*currSquare, kDMSquareMaskThingListPresent)) {
			thingInList = getSquareFirstThing(mapX, mapY);
		} else {
			setFlag(*currSquare, kDMSquareMaskThingListPresent);
			uint16 *tmp = _currMapColCumulativeSquareFirstThingCount + mapX + 1;
			uint16 currColumn = _dungeonColumCount - (_dungeonMapsFirstColumnIndex[_currMapIndex] + mapX) - 1;
			while (currColumn--) { /* For each column starting from and after the column containing the square where the thing is added */
				(*tmp++)++; /* Increment the cumulative first thing count */
			}
			uint16 currMapY = 0;
			currSquare -= mapY;
			uint16 currSquareFirstThingIndex = _currMapColCumulativeSquareFirstThingCount[mapX];
			while (currMapY++ != mapY) {
				if (getFlag(*currSquare++, kDMSquareMaskThingListPresent))
					currSquareFirstThingIndex++;
			}
			Thing *currThing = &_squareFirstThings[currSquareFirstThingIndex];
			// the second '- 1' is for the loop initialization, > 0 is because we are copying from one behind
			for (int16 i = _dungeonFileHeader._squareFirstThingCount - currSquareFirstThingIndex - 1 - 1; i > 0; --i)
				currThing[i] = currThing[i - 1];

			*currThing = thingToLink;
			return;
		}
	}
	Thing nextThing = getNextThing(thingInList);
	while (nextThing != _vm->_thingEndOfList)
		nextThing = getNextThing(thingInList = nextThing);

	*getNextThingPtr(thingInList) = thingToLink;
}

WeaponInfo *DungeonMan::getWeaponInfo(Thing thing) {
	Weapon *weapon = getWeapon(thing);
	return &_weaponInfos[weapon->getType()];
}

int16 DungeonMan::getProjectileAspect(Thing thing) {
	ThingType thingType = thing.getType();
	if (thingType == kDMThingTypeExplosion) {
		if (thing == _vm->_thingExplFireBall)
			return -_vm->indexToOrdinal(k10_ProjectileAspectExplosionFireBall);
		if (thing == _vm->_thingExplSlime)
			return -_vm->indexToOrdinal(k12_ProjectileAspectExplosionSlime);
		if (thing == _vm->_thingExplLightningBolt)
			return -_vm->indexToOrdinal(k3_ProjectileAspectExplosionLightningBolt);
		if ((thing == _vm->_thingExplPoisonBolt) || (thing == _vm->_thingExplPoisonCloud))
			return -_vm->indexToOrdinal(k13_ProjectileAspectExplosionPoisonBoltCloud);

		return -_vm->indexToOrdinal(k11_ProjectileAspectExplosionDefault);
	} else if (thingType == kDMThingTypeWeapon) {
		WeaponInfo *weaponInfo = getWeaponInfo(thing);
		int16 projAspOrd = weaponInfo->getProjectileAspectOrdinal();
		if (projAspOrd)
			return -projAspOrd;
	}

	int16 infoIndex = getObjectInfoIndex(thing);
	if (infoIndex < 0 || infoIndex >= 180)
		return 0;
	return _objectInfos[infoIndex]._objectAspectIndex;
}

int16 DungeonMan::getLocationAfterLevelChange(int16 mapIndex, int16 levelDelta, int16 *mapX, int16 *mapY) {
	if (_partyMapIndex == kDMMapIndexEntrance)
		return kDMMapIndexNone;

	Map *map = _dungeonMaps + mapIndex;
	int16 newMapX = map->_offsetMapX + *mapX;
	int16 newMapY = map->_offsetMapY + *mapY;
	int16 newLevel = map->_level + levelDelta;
	map = _dungeonMaps;

	for (int16 targetMapIndex = 0; targetMapIndex < _dungeonFileHeader._mapCount; targetMapIndex++) {
		if ((map->_level == newLevel)
		&& (newMapX >= map->_offsetMapX) && (newMapX <= map->_offsetMapX + map->_width)
		&& (newMapY >= map->_offsetMapY) && (newMapY <= map->_offsetMapY + map->_height)) {
			*mapY = newMapY - map->_offsetMapY;
			*mapX = newMapX - map->_offsetMapX;
			return targetMapIndex;
		}
		map++;
	}
	return kDMMapIndexNone;
}

Thing DungeonMan::getSquareFirstObject(int16 mapX, int16 mapY) {
	Thing thing = getSquareFirstThing(mapX, mapY);
	while ((thing != _vm->_thingEndOfList) && (thing.getType() < kDMThingTypeGroup))
		thing = getNextThing(thing);

	return thing;
}

uint16 DungeonMan::getArmourDefense(ArmourInfo *armourInfo, bool useSharpDefense) {
	uint16 defense = armourInfo->_defense;
	if (useSharpDefense)
		defense = _vm->getScaledProduct(defense, 3, getFlag(armourInfo->_attributes, kDMArmourAttributeSharpDefense) + 4);

	return defense;
}

Thing DungeonMan::getDiscardThing(uint16 thingType) {
	// CHECKME: Shouldn't it be saved in the savegames?
	static unsigned char lastDiscardedThingMapIndex[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (thingType == kDMThingTypeExplosion)
		return _vm->_thingNone;

	GroupMan &groupMan = *_vm->_groupMan;
	ProjExpl &projExpl = *_vm->_projexpl;

	int16 currentMapIdx = _currMapIndex;
	uint16 mapIndex = lastDiscardedThingMapIndex[thingType];
	if ((mapIndex == _partyMapIndex) && (++mapIndex >= _dungeonFileHeader._mapCount))
		mapIndex = 0;

	uint16 discardThingMapIndex = mapIndex;
	for (;;) { /*_Infinite loop_*/
		uint16 mapWidth = _dungeonMaps[mapIndex]._width;
		uint16 mapHeight = _dungeonMaps[mapIndex]._height;
		byte *currSquare = _dungeonMapData[mapIndex][0];
		Thing *squareFirstThing = &_squareFirstThings[_dungeonColumnsCumulativeSquareThingCount[_dungeonMapsFirstColumnIndex[mapIndex]]];

		for (int16 currMapX = 0; currMapX <= mapWidth; currMapX++) {
			for (int16 currMapY = 0; currMapY <= mapHeight; currMapY++) {
				if (getFlag(*currSquare++, kDMSquareMaskThingListPresent)) {
					Thing squareThing = *squareFirstThing++;
					if ((mapIndex == _partyMapIndex) && ((currMapX - _partyMapX + 5) <= 10) && ((currMapY - _partyMapY + 5) <= 10)) /* If square is too close to the party */
						continue;

					do {
						ThingType squareThingType = squareThing.getType();
						if (squareThingType == kDMThingTypeSensor) {
							if (getSensor(squareThing)->getType()) /* If sensor is not disabled */
								break;
						} else if (squareThingType == thingType) {
							switch (thingType) {
							case kDMThingTypeGroup:
								if (getGroup(squareThing)->getDoNotDiscard())
									continue;
								// fall through
							case kDMThingTypeProjectile:
								setCurrentMap(mapIndex);
								if (thingType == kDMThingTypeGroup) {
									groupMan.dropGroupPossessions(currMapX, currMapY, squareThing, kDMSoundModeDoNotPlaySound);
									groupMan.groupDelete(currMapX, currMapY);
								} else {
									projExpl.projectileDeleteEvent(squareThing);
									unlinkThingFromList(squareThing, Thing(0), currMapX, currMapY);
									projExpl.projectileDelete(squareThing, nullptr, currMapX, currMapY);
								}
								break;
							case kDMThingTypeArmour:
								if (getArmour(squareThing)->getDoNotDiscard())
									continue;

								setCurrentMap(mapIndex);
								_vm->_moveSens->getMoveResult(squareThing, currMapX, currMapY, kDMMapXNotOnASquare, 0);
								break;
							case kDMThingTypeWeapon:
								if (getWeapon(squareThing)->getDoNotDiscard())
									continue;

								setCurrentMap(mapIndex);
								_vm->_moveSens->getMoveResult(squareThing, currMapX, currMapY, kDMMapXNotOnASquare, 0);
								break;
							case kDMThingTypeJunk:
								if (getJunk(squareThing)->getDoNotDiscard())
									continue;

								setCurrentMap(mapIndex);
								_vm->_moveSens->getMoveResult(squareThing, currMapX, currMapY, kDMMapXNotOnASquare, 0);
								break;
							case kDMThingTypePotion:
								if (getPotion(squareThing)->getDoNotDiscard())
									continue;

								setCurrentMap(mapIndex);
								_vm->_moveSens->getMoveResult(squareThing, currMapX, currMapY, kDMMapXNotOnASquare, 0);
								break;
							default:
								break;
							}
							setCurrentMap(currentMapIdx);
							lastDiscardedThingMapIndex[thingType] = mapIndex;
							return Thing(squareThing.getTypeAndIndex());
						}
					} while ((squareThing = getNextThing(squareThing)) != _vm->_thingEndOfList);
				}
			}
		}
		if ((mapIndex == _partyMapIndex) || (_dungeonFileHeader._mapCount <= 1)) {
			lastDiscardedThingMapIndex[thingType] = mapIndex;
			return _vm->_thingNone;
		}

		do {
			if (++mapIndex >= _dungeonFileHeader._mapCount)
				mapIndex = 0;
		} while (mapIndex == _partyMapIndex);

		if (mapIndex == discardThingMapIndex)
			mapIndex = _partyMapIndex;
	}
}

uint16 DungeonMan::getCreatureAttributes(Thing thing) {
	Group *currGroup = getGroup(thing);
	return _creatureInfos[currGroup->_type]._attributes;
}

void DungeonMan::setGroupCells(Group *group, uint16 cells, uint16 mapIndex) {
	if (mapIndex == _partyMapIndex)
		_vm->_groupMan->_activeGroups[group->getActiveGroupIndex()]._cells = cells;
	else
		group->_cells = cells;
}

void DungeonMan::setGroupDirections(Group *group, int16 dir, uint16 mapIndex) {
	if (mapIndex == _partyMapIndex)
		_vm->_groupMan->_activeGroups[group->getActiveGroupIndex()]._directions = dir;
	else
		group->setDir(_vm->normalizeModulo4(dir));
}

bool DungeonMan::isCreatureAllowedOnMap(Thing thing, uint16 mapIndex) {
	CreatureType creatureType = getGroup(thing)->_type;
	Map *map = &_dungeonMaps[mapIndex];
	byte *allowedCreatureType = _dungeonMapData[mapIndex][map->_width] + map->_height + 1;
	for (int16 L0234_i_Counter = map->_creatureTypeCount; L0234_i_Counter > 0; L0234_i_Counter--) {
		if (*allowedCreatureType++ == creatureType)
			return true;
	}
	return false;
}

void DungeonMan::unlinkThingFromList(Thing thingToUnlink, Thing thingInList, int16 mapX, int16 mapY) {
	if (thingToUnlink == _vm->_thingEndOfList)
		return;

	uint16 tmp = thingToUnlink.toUint16();
	clearFlag(tmp, 0xC000);
	thingToUnlink = Thing(tmp);

	Thing *thingPtr = getNextThingPtr(thingToUnlink);
	if (mapX >= 0) {
		uint16 firstThingIndex = getSquareFirstThingIndex(mapX, mapY);
		Thing *currThing = &_squareFirstThings[firstThingIndex]; /* BUG0_01 Coding error without consequence. The engine does not check that there are things at the specified square coordinates. f160_getSquareFirstThingIndex would return -1 for an empty square. No consequence as the function is never called with the coordinates of an empty square (except in the case of BUG0_59) */
		if ((*thingPtr == _vm->_thingEndOfList) && (currThing->getTypeAndIndex() == thingToUnlink.toUint16())) { /* If the thing to unlink is the last thing on the square */
			clearFlag(_currMapData[mapX][mapY], kDMSquareMaskThingListPresent);
			uint16 squareFirstThingIdx = _dungeonFileHeader._squareFirstThingCount - 1;
			for (uint16 i = 0; i < squareFirstThingIdx - firstThingIndex; ++i)
				currThing[i] = currThing[i + 1];

			_squareFirstThings[squareFirstThingIdx] = _vm->_thingNone;
			uint16 *cumulativeFirstThingCount = _currMapColCumulativeSquareFirstThingCount + mapX + 1;
			uint16 currColumn = _dungeonColumCount - (_dungeonMapsFirstColumnIndex[_currMapIndex] + mapX) - 1;
			while (currColumn--) { /* For each column starting from and after the column containing the square where the thing is unlinked */
				(*cumulativeFirstThingCount++)--; /* Decrement the cumulative first thing count */
			}
			*thingPtr = _vm->_thingEndOfList;
			return;
		}
		if (currThing->getTypeAndIndex() == thingToUnlink.toUint16()) {
			*currThing = *thingPtr;
			*thingPtr = _vm->_thingEndOfList;
			return;
		}
		thingInList = *currThing;
	}

	Thing currThing = getNextThing(thingInList);
	while (currThing.getTypeAndIndex() != thingToUnlink.toUint16()) {
		if ((currThing == _vm->_thingEndOfList) || (currThing == _vm->_thingNone)) {
			if (thingPtr)
				*thingPtr = _vm->_thingEndOfList;
			return;
		}
		currThing = getNextThing(thingInList = currThing);
	}
	*getNextThingPtr(thingInList) = getNextThing(currThing);
	*thingPtr = _vm->_thingEndOfList;
}

int16 DungeonMan::getStairsExitDirection(int16 mapX, int16 mapY) {
	bool northSouthOrientedStairs = !getFlag(getSquare(mapX, mapY).toByte(), kDMSquareMaskStairsNorthSouth);

	if (northSouthOrientedStairs) {
		mapX = mapX + _vm->_dirIntoStepCountEast[kDMDirEast];
		mapY = mapY + _vm->_dirIntoStepCountNorth[kDMDirEast];
	} else {
		mapX = mapX + _vm->_dirIntoStepCountEast[kDMDirNorth];
		mapY = mapY + _vm->_dirIntoStepCountNorth[kDMDirNorth];
	}
	int16 squareType = Square(getSquare(mapX, mapY)).getType();

	int16 retval = ((squareType == kDMElementTypeWall) || (squareType == kDMElementTypeStairs)) ? 1 : 0;
	retval <<= 1;
	retval += (northSouthOrientedStairs ? 1 : 0);

	return retval;
}

Thing DungeonMan::getObjForProjectileLaucherOrObjGen(uint16 iconIndex) {
	int16 thingType = kDMThingTypeWeapon;
	if ((iconIndex >= kDMIconIndiceWeaponTorchUnlit) && (iconIndex <= kDMIconIndiceWeaponTorchLit))
		iconIndex = kDMIconIndiceWeaponTorchUnlit;

	int16 junkType;

	switch (iconIndex) {
	case kDMIconIndiceWeaponRock:
		junkType = kDMWeaponRock;
		break;
	case kDMIconIndiceJunkBoulder:
		junkType = kDMJunkTypeBoulder;
		thingType = kDMThingTypeJunk;
		break;
	case kDMIconIndiceWeaponArrow:
		junkType = kDMWeaponArrow;
		break;
	case kDMIconIndiceWeaponSlayer:
		junkType = kDMWeaponSlayer;
		break;
	case kDMIconIndiceWeaponPoisonDart:
		junkType = kDMWeaponPoisonDart;
		break;
	case kDMIconIndiceWeaponThrowingStar:
		junkType = kDMWeaponThrowingStar;
		break;
	case kDMIconIndiceWeaponDagger:
		junkType = kDMWeaponDagger;
		break;
	case kDMIconIndiceWeaponTorchUnlit:
		junkType = kDMWeaponTorch;
		break;
	default:
		return _vm->_thingNone;
	}

	Thing unusedThing = getUnusedThing(thingType);
	if (unusedThing == _vm->_thingNone)
		return _vm->_thingNone;

	if (thingType == kDMThingTypeJunk) {
		getJunk(unusedThing)->setType(junkType);
	} else {
		getWeapon(unusedThing)->setType(junkType);
		if ((iconIndex == kDMIconIndiceWeaponTorchUnlit) && getWeapon(unusedThing)->isLit()) /* BUG0_65 Torches created by object generator or projectile launcher sensors have no charges. Charges are only defined if the Torch is lit which is not possible at the time it is created */
			getWeapon(unusedThing)->setChargeCount(15);
	}

	return unusedThing;
}

int16 DungeonMan::getRandomOrnamentIndex(uint16 val1, uint16 val2, int16 modulo) {
	// TODO: Use ScummVM random number generator
	return ((((((val1 * 31417) & 0xFFFF) >> 1) + ((val2 * 11) & 0xFFFF)
			  + _dungeonFileHeader._ornamentRandomSeed) & 0xFFFF) >> 2) % modulo; /* Pseudorandom number generator */
}

void DungeonMan::duplicateThing(Thing thing) {
	uint16 thingType = thing.getType();
	uint16 thingIndex = thing.getIndex();
	switch (thingType) {
	case kDMThingTypeWeapon:
		_weapons.push_back(_weapons[thingIndex]);
		_weapons.back()._nextThing = _vm->_thingEndOfList;
		break;
	case kDMThingTypeArmour:
		_armours.push_back(_armours[thingIndex]);
		_armours.back()._nextThing = _vm->_thingEndOfList;
		break;
	case kDMThingTypeScroll:
		_scrolls.push_back(_scrolls[thingIndex]);
		_scrolls.back()._nextThing = _vm->_thingEndOfList;
		break;
	case kDMThingTypePotion:
		_potions.push_back(_potions[thingIndex]);
		_potions.back()._nextThing = _vm->_thingEndOfList;
		break;
	case kDMThingTypeContainer:
		_containers.push_back(_containers[thingIndex]);
		_containers.back()._nextThing = _vm->_thingEndOfList;
		break;
	case kDMThingTypeJunk:
		_junks.push_back(_junks[thingIndex]);
		_junks.back()._nextThing = _vm->_thingEndOfList;
		break;
	default:
		return;
	}
	_dungeonFileHeader._thingCounts[thingType]++;
}

Thing *DungeonMan::getNextThingPtr(Thing thing) {
	switch (thing.getType()) {
	case kDMThingTypeDoor:
		return &_doors[thing.getIndex()]._nextThing;
	case kDMThingTypeTeleporter:
		return &_teleporters[thing.getIndex()]._nextThing;
	case kDMstringTypeText:
		return &_textStrings[thing.getIndex()]._nextThing;
	case kDMThingTypeSensor:
		return &_sensors[thing.getIndex()]._nextThing;
	case kDMThingTypeGroup:
		return &_groups[thing.getIndex()]._nextThing;
	case kDMThingTypeWeapon:
		return &_weapons[thing.getIndex()]._nextThing;
	case kDMThingTypeArmour:
		return &_armours[thing.getIndex()]._nextThing;
	case kDMThingTypeScroll:
		return &_scrolls[thing.getIndex()]._nextThing;
	case kDMThingTypePotion:
		return &_potions[thing.getIndex()]._nextThing;
	case kDMThingTypeContainer:
		return &_containers[thing.getIndex()]._nextThing;
	case kDMThingTypeJunk:
		return &_junks[thing.getIndex()]._nextThing;
	case kDMThingTypeProjectile:
		return &_projectiles[thing.getIndex()]._nextThing;
	case kDMThingTypeExplosion:
		return &_explosions[thing.getIndex()]._nextThing;
	default:
		return nullptr;
	}
}

void DungeonMan::resetThing(Thing thing) {
	uint16 index = thing.getIndex();
	switch (thing.getType()) {
	case kDMThingTypeDoor:
		_doors[index] = Door();
		break;
	case kDMThingTypeTeleporter:
		_teleporters[index] = Teleporter();
		break;
	case kDMstringTypeText:
		_textStrings[index] = TextString();
		break;
	case kDMThingTypeSensor:
		_sensors[index] = Sensor();
		break;
	case kDMThingTypeGroup:
		_groups[index] = Group();
		break;
	case kDMThingTypeWeapon:
		_weapons[index] = Weapon();
		break;
	case kDMThingTypeArmour:
		_armours[index] = Armour();
		break;
	case kDMThingTypeScroll:
		_scrolls[index] = Scroll();
		break;
	case kDMThingTypePotion:
		_potions[index] = Potion();
		break;
	case kDMThingTypeContainer:
		_containers[index] = Container();
		break;
	case kDMThingTypeJunk:
		_junks[index] = Junk();
		break;
	case kDMThingTypeProjectile:
		_projectiles[index] = Projectile();
		break;
	case kDMThingTypeExplosion:
		_explosions[index] = Explosion();
		break;
	default:
		break;
	}
}

} // namespace DM

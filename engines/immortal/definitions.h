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

#ifndef IMMORTAL_DEFINITIONS_H
#define IMMORTAL_DEFINITIONS_H

namespace Immortal {

enum Cyc {
    kCycNone
};

enum Motive {                       // This will likely be moved to a monster ai specific file later
    kMotiveRoomCombat,
    kMotiveShadeFind,
    kMotiveShadeLoose,
    kMotiveEngage,
    kMotiveUpdateGoal,
    kMotiveFollow,
    kMotiveShadeHesitate,
    kMotiveEasyRoomCombat,
    kMotiveFind8,
    kMotiveLoose4,
    kMotiveDefensiveCombat,
    kMotiveUlinTalk,
    kMotiveGive,
    kMotiveUseUpMonster,
    kMotiveAliveRoomCombat,
    kMotiveFindAlways,
    kMotivePlayerCombat,
    kMotiveJoystick,
    kMotivePlayerDoor,
    kMotivewaittalk2,
    kMotiveGetDisturbed,
    kMotiveLoose32,
    kMotiveIfNot1Skip1,
};

enum Str {
    kStrNoDesc,
    kStrSword,
    kStrSwordDesc,
    kStrBonesText1,
    kStrBonesText2,
    kStrBonesText3,
    kStrComp,
    kStrCompDesc,
    kStrOpenBag,
    kStrThrowComp,
    kStrSmithText1,
    kStrSmithText2,
    kStrCarpet,
    kStrBomb,
    kStrBombDesc,
    kStrPickItUp,
    kStrYesNo,
    kStrOther,
    kStrChestKey,
    kStrDoorKey,
    kStrChestKeyDesc,
    kStrOpenChestDesc,
    kStrPutItOn,
    kStrDropItThen,
    kStrChestDesc,
    kStrGoodChestDesc,
    kStrBadChestDesc,
    kStrComboLock,
    kStrGold,
    kStrFindGold,
    kStrNull,
    kStrNotHere,
    kStrUnlockDoor,
    kStrWeak1,
    kStrDummyWater,
    kStrBadWizard,
    kStrDiesAnyway,
    kStrDoorKeyDesc,
    kStrNoteDesc,
    kStrNote,
    kStrLootBodyDesc,
    kStrNotEnough,
    kStrGameOver,
    kStrYouWin,
    kStrWormFoodDesc,
    kStrWormFood,
    kStrStoneDesc,
    kStrStone,
    kStrGemDesc,
    kStrGem,
    kStrFireBallDesc,
    kStrFireBall,
    kStrDeathMapDesc,
    kStrDeathMap,
    kStrBoots,
    kStrUseBoots,
    kStrWowCharmDesc,
    kStrWowCharm,
    kStrUseWowCharm,
    kStrWaterOpen,
    kStrDrinkIt,
    kStrItWorks,
    kStrSBOpen,
    kStrUsesFire,
    kStrMuscleDesc,
    kStrMuscle,
    kStrSBDesc,
    kStrSB,
    kStrFace,
    kStrFaceDesc,
    kStrTRNDesc,
    kStrTRN,
    kStrInvisDesc,
    kStrGoodLuckDesc,
    kStrAnaRing,
    kStrInvis,
    kStrGoesAway,
    kStrGiveHerRing,
    kStrGive2,
    kStrMadKingText,
    kStrMadKing3Text,
    kStrMadKing2Text,
    kStrDream1,
    kStrDream1P2,
    kStrDream1P3,
    kStrHowToGetOut,
    kStrSpore,
    kStrSporeDesc,
    kStrRequestPlayDisc,
    kStrOldGame,
    kStrEnterCertificate,
    kStrBadCertificate,
    kStrCert,
    kStrCert2,
    kStrTitle0,
    kStrTitle4,
    kStrMDesc,
    kStrM3Desc,
    kStrMapText1,
    kStrMapText2,

    // Level 0 str

    // Level 1 str

    // Level 2 str

    // Level 3 str

    // Level 4 str

    // Level 5 str

    // Level 6 str

    // Level 7 str

    kCantUnlockDoor = kStrBadChestDesc
};

enum SObjType {
    kTypeTrap,
    kTypeCoin,
    kTypeWowCharm,
    kTypeDead,
    kTypeFireBall,
    kTypeDunRing,
    kTypeChest,
    kTypeDeathMap,
    kTypeWater,
    kTypeSpores,
    kTypeWormFood,
    kTypeChestKey,
    kTypePhant,
    kTypeGold,
    kTypeHay,
    kTypeBeam
};


} // namespace immortal

#endif
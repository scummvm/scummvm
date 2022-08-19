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

#ifndef IMMORTAL_SPRITE_LIST_H
#define IMMORTAL_SPRITE_LIST_H

namespace Immortal {

struct Image {
	uint16  _deltaX;
	uint16  _deltaY;
	uint16  _rectX;
	uint16  _rectY;
	  byte *_bitmap;
};

struct DataSprite {
	uint16  _cenX;                      // These are the base center positions
	uint16  _cenY;
	uint16  _numImages;
Common::Array<Image> _images;
};

// Cycles define the animation of sprites within a level. There is a fixed total of cycles available, and they are not room dependant
struct Cycle {
DataSprite *_dSprite;
	  bool  _repeat;
	   int  _index;						// In source this is actually the position within the *instruction list*, but since cycle's are structs, it's just the index of frames now
	   int *_frames;
};

enum SpriteFrame {
	// Null
	kNoFrame,

	// Chest frames
	kChest0Frame = 0,
	kOpenChestFrame,
	kRingFrame,
	kKnifeFrame,
	kDeadGoblinFrame,

	// Normal frames
	kSwordFrame,
	kKeyFrame,
	kYesIconOff,
	kYesIconOn,
	kNoIconOff,
	kNoIconOn,
	kChoiceFrame,
	kEraseChoseFrame,
	kSwordBigFrame,
	kVaseBigFrame,
	kVaseFrame,
	kBrokenFrame,
	kKeyBigFrame,
	kBagFrame,
	kBagBigFrame,
	kBookBigFrame,
	kBookFrame,
	kScrollFrame,
	kScrollBigFrame,
	kOkayFrame,
	kAltarFrame,
	kGoldBigFrame,
	kMapBigFrame,
	kSemblanceFrame,
	kTrapDoorFrame,
	kBonesFrame,
	kSackBigFrame,
	kSporesFrame,
	kGemGlintFrame,
	kStoneFrame,
	kGreenStoneFrame,
	kGemBigFrame,
	kStoneBigFrame,
	kPileFrame,
	kNoteBigFrame,

	// 45 - 48 are Merchant frames
	kMerchantFrame,

	// Remaining frames
	kCoinBigFrame = 49,
	kPileBigFrame,
	kKingFrame,
	kDeadKingFrame,
	kBombBigFrame,
	kRingBigFrame,
	kKnifeBigFrame,
	kCarpetBigFrame,
	kAnaInHoleFrame,
	kAnaNotInHoleFrame,
	kInvisRingFrame
};

enum SpriteName {
	// Moresprites 10
	kCandle,
	kWow,
	kAnaVanish,
	kSink,
	kTrapdoor,
	kWizPhant,
	kVanish,
	kShadow,
	kSlime,
	kSlimeDeath,

	// Norlac 5
	kBridge,
	kVortex,
	kBubble,
	kNorlac,
	kNolac2,

	// Powwow 7
	kPlanners,
	kUgh,
	kIsDevoured,
	kIsBadCrawl,
	kIsGoodCrawl,
	kLeg,
	kIsWebbed,

	// Turrets 10
	kSleep,
	kShrink,
	kLocksmith,
	kAnaGlimpse,
	kMadKing,
	kTorch,
	kPipe,
	kProjectile,
	kKnife,
	kAnaHug,

	// Worm 4
	kWorm0,
	kWorm1,
	kSpike,
	kIsSpiked,

	// Iansprites 6
	kMurder,
	kWizCrawlUp,
	kWizLight,
	kWizBattle,
	kDown,
	kNorlacDown,

	// Lastsprites 3
	kWaterLadder,
	kPulledDown,
	kSpill,

	// Doorsprites 10
	kDoor,
	kTele,
	kBomb,
	kTorched,
	kLadderTop,
	kSecret,
	kLadderBottom,
	kSlipped,
	kGoblinSlipped,
	kFlame,

	// General 5
	kArrow,
	kSpark,
	kObject,
	kBigBurst,
	kBeam,

	// Mordamir 3
	kLight,
	kMord,
	kDragMask,

	// Dragon2 2
	kDFlames,
	kThroat,

	// Dragon 1
	kDragon,

	// Rope 3
	kChop,
	kHead,
	kNurse,

	// Rescue 2
	kRescue1,
	kRescue2,

	// Troll 9 (8 directions + ?)
	kTroll0,
	kTroll1,
	kTroll2,
	kTroll3,
	kTroll4,
	kTroll5,
	kTroll6,
	kTroll7,
	kTroll8,

	// Goblin 10 (8 directions + ?)
	kGoblin0,
	kGoblin1,
	kGoblin2,
	kGoblin3,
	kGoblin4,
	kGoblin5,
	kGoblin6,
	kGoblin7,
	kGoblin8,
	kGoblin9,

	// Wizard A 8 (8 directions)
	kWizard0,
	kWizard1,
	kWizard2,
	kWizard3,
	kWizard4,
	kWizard5,
	kWizard6,
	kWizard7,

	// Wizard B 3 (3 ?)
	kWizard8,
	kWizard9,
	kWizard10,

	// Ulindor 9 (8 directions + ?)
	kUlindor0,
	kUlindor1,
	kUlindor2,
	kUlindor3,
	kUlindor4,
	kUlindor5,
	kUlindor6,
	kUlindor7,
	kUlindor8,

	// Spider 10 (probably not directions)
	kSpider0,
	kSpider1,
	kSpider2,
	kSpider3,
	kSpider4,
	kSpider5,
	kSpider6,
	kSpider7,
	kSpider8,
	kSpider9,

	// Drag 9 (probably not directions)
	kDrag0,
	kDrag1,
	kDrag2,
	kDrag3,
	kDrag4,
	kDrag5,
	kDrag6,
	kDrag7,
	kDrag8,

	// Font
	kFont
};

} // namespace immortal

#endif














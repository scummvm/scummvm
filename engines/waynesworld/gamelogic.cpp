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

#include "waynesworld/gamelogic.h"
#include "waynesworld/graphics.h"
#include "waynesworld/objectids.h"
#include "common/str.h"
#include "graphics/cursorman.h"

namespace WaynesWorld {

static const int kMazeInfos[77][7] = {
	{ -5, -1, 14, -1, 23, -1, -1 },
	{ -1, -1, 37, 45, 2, 52, -1 },
	{ -1, -1, 24, 4, 22, 64, -1 },
	{ -1, -1, 12, -1, -1, -1, -1 },
	{ -1, -1, -1, 73, 62, 13, 80 },
	{ -1, -1, 15, 93, -1, 63, -1 },
	{ -1, -1, 44, 55, 25, 108, -1 },
	{ -1, -1, -1, 43, 119, 119, -1 },
	{ 46, 126, 132, -1, -1, 133, -1 },
	{ -1, -1, 142, 53, 153, -1, -1 },
	{ -1, -1, -1, 65, -1, -1, -1 },
	{ -1, -1, 75, -1, 74, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, 81 },
	{ -1, -1, 82, 85, 165, -1, -1 },
	{ -1, -1, 92, -1, 177, -1, 181 },
	{ -1, -1, 177, 94, 192, 208, 211 },
	{ -1, -1, 222, 237, 242, 134, -1 },
	{ -1, -1, 144, -1, 152, -1, -1 },
	{ -1, 146, -1, -1, -1, -1, -1 },
	{ -1, -1, 154, 195, -1, 193, -1 },
	{ -1, -1, -1, 155, -1, 253, -1 },
	{ -1, 156, 263, -3, -1, 275, -1 },
	{ -1, -1, 162, 285, 294, -1, -1 },
	{ -1, -1, 282, -1, 163, -1, -1 },
	{ -1, 306, 164, -1, -1, 319, -1 },
	{ -1, -1, -1, 208, -1, -1, -1 },
	{ -1, -1, 324, 212, 273, -1, -1 },
	{ -1, -1, 332, 264, -1, 215, -1 },
	{ -1, -1, 237, 293, 342, 223, -1 },
	{ -1, -1, 345, 283, 224, -1, -1 },
	{ -1, -1, 353, -1, 365, -1, 241 },
	{ -1, -1, 245, -1, -1, -1, -1 },
	{ -1, -1, -4, 333, 262, 382, -1 },
	{ -1, -1, 272, 323, -1, 392, -1 },
	{ -1, -1, 284, 409, -1, 292, -1 },
	{ -1, -1, -2, 302, 363, -1, -1 },
	{ -1, -1, 412, 354, -1, 304, -1 },
	{ -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, 325, 434, 433, -1, -1 },
	{ -1, -1, 335, 394, 393, -1, -1 },
	{ -1, -1, 485, 343, -1, -1, -1 },
	{ -1, -1, 362, 414, 413, 444, -1 },
	{ -1, -1, 455, 435, 432, -1, -1 },
	{ -1, -1, 424, 384, 383, 423, -1 },
	{ -1, -1, -1, -1, 415, -1, -1 },
	{ -1, -1, 463, -1, 478, 422, -1 },
	{ -1, -1, -1, 452, -1, 478, -1 },
	{ -1, -1, 454, 465, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, 409, -1 },
	{ -1, -1, 583, 563, -4, 505, -1 },
	{ -1, -1, 552, -6, 515, 495, -1 },
	{ -1, -1, 542, 535, 524, 504, -1 },
	{ -1, -1, -1, -1, 514, 534, -1 },
	{ -1, -1, -1, -1, 525, 513, -1 },
	{ -1, -1, 512, 544, 543, 555, -1 },
	{ -1, -1, 502, -7, -1, 545, -1 },
	{ -1, -1, 572, 493, -1, -1, -1 },
	{ -1, -1, 562, -1, -1, -1, -1 },
	{ -1, -1, -1, 492, -1, -1, 591 },
	{ -1, 586, 605, -1, -1, 644, -1 },
	{ -1, -1, 612, 615, 623, 592, -1 },
	{ -1, -1, 602, 625, 622, 603, -1 },
	{ -1, -1, 614, 604, 634, 613, -1 },
	{ -1, -1, -1, -1, 624, -1, -1 },
	{ -1, -1, -1, -1, 595, -8, 650 },
	{ 646, -1, 663, -1, 655, 654, -1 },
	{ -1, -1, 675, 652, 735, -1, -1 },
	{ -1, -1, 685, 693, 715, 662, -1 },
	{ -1, -1, 702, 712, 703, 672, -1 },
	{ -1, -1, 695, 673, 713, 692, -1 },
	{ -1, -1, 682, 684, 724, 714, -1 },
	{ -1, -1, 683, 694, 705, 674, -1 },
	{ -1, -1, 723, 722, 704, -10, -1 },
	{ -1, -1, 745, -1, -1, 664, -1 },
	{ -1, -1, 755, -1, -1, 732, -1 },
	{ -1, -1, 765, -1, -1, 742, -1 },
	{ -1, -1, -9, -1, -1, 752, -1 }
};

static const int kRoom24MazeHolePositionsX[] = { 58, 170, 0, 67, 177, 278, 58 };
static const int kRoom24MazeHolePositionsY[] = { 16, 16, 49, 47, 47, 49, 128 };
static const int kRoom24MazeWaynePositionsX[] = { -1, -1, 43, 91, 199, 279, 54 };
static const int kRoom24MazeWaynePositionsY[] = { -1, -1, 139, 123, 124, 140, 128 };
static const int kRoom24MazeGarthPositionsX[] = { -1, -1, 49, 110, 223, 267, 152 };
static const int kRoom24MazeGarthPositionsY[] = { -1, -1, 129, 123, 124, 128, 128 };

static const int kRoom25MazeHolePositionsX[] = { 79, 289, 63, 183, 274, 75 };
static const int kRoom25MazeHolePositionsY[] = { 48, 37, 52, 59, 50, 64 };

static const int kRoom31StarPositionsX[]  = { 45, 65, 87, 107, 129 };
static const int kRoom31StarPositionsY[]  = { 27, 39, 50, 61, 74, 27, 39, 50, 62, 75, 26, 38, 50, 63, 76, 26, 38, 50, 63, 76, 25, 38, 51, 64, 77 };
static const int kRoom31NumberPositionsX[]  = { 77, 215, 224 };
static const int kRoom31NumberPositionsY[]  = { 130, 125, 135 };

static const int kRoom37CorrectSafeCombination[] = { 3, 5, 4, 8, 1, 8 };

// GameLogic

GameLogic::GameLogic(WaynesWorldEngine *vm) : _vm(vm) {
	initVariables();
}

GameLogic::~GameLogic() {

}

void GameLogic::initVariables() {
	_word_34464 = 0;
	_word_34466 = 0;
	_r37_safeCombinationLockIndex = 0;
	_r37_word_35CEC = 0;
	_r37_safeCombinationIndex = 0;
	_r37_safeCombinationCurrentNumber = 0;
	_r12_talkObjectNumber = 0;
	_pizzathonListFlags1 = 0;
	_pizzathonListFlags2 = 0;
	_r31_flags = 0;
	_r0_flags = 0;
	_r4_flags = 0;
	_r5_flags = 0;
	_r7_flags = 0;
	_r11_flags = 0;
	_r32_flags = 0;
	_r1_flags1 = 0;
	_r1_flags2 = 0;
	_r2_flags = 0;
	_r6_flags = 0;
	_r10_flags = 0;
	_r12_flags = 0;
	_r19_flags = 0;
	_r9_flags = 0;
	_r8_flags = 0;
	_r13_flags = 0;
	_r20_flags = 0;
	_r29_flags = 0;
	_r30_flags = 0;
	_r34_flags = 0;
	_r35_flags = 0;
	_r37_flags = 0;
	_r36_flags = 0;
	_r38_flags = 0;
	_r39_flags = 0;
	_r10_selectedItemToBuy = -1;
	_r9_dialogFlag = 0;
	_r1_eventFlag = 0;
	_r1_eventCtr = 1;
	_r17_dialogCtr = 0;
	_r17_eventFlag = 0;
	_r2_backgroundValue1 = 0;
	_r2_backgroundValue2 = 0;
	_r7_cloudsPositionX = 0;
	_r19_wayneSpriteX = 0;
	_r19_garthSpriteX = 0;
	_r24_mazeRoomNumber = 0;
	_r24_mazeHoleNumber = 2;
	_r25_holeIndex = 0;
	_r31_questionsAsked = 0;
	_r31_correctAnswerChoice = 0;
	_r31_categoryIndex = 0;
	_r31_questionIndex = 0;
	_r31_categorySelected = false;
	_r31_currentPlayer = 1;
	memset(_r31_scores, 0, sizeof(_r31_scores));
	memset(_r31_askedQuestions, 0, sizeof(_r31_askedQuestions));
}

int GameLogic::handleVerbPickUp() {
    switch (_vm->_currentRoomNumber) {
    case 0:
        return r0_handleVerbPickUp();
    case 1:
        return r1_handleVerbPickUp();
    case 2:
        return r2_handleVerbPickUp();
    case 3:
    case 23:
        return r3_handleVerbPickUp();
    case 4:
        return r4_handleVerbPickUp();
    case 5:
        return r5_handleVerbPickUp();
    case 6:
        return r6_handleVerbPickUp();
    case 7:
    case 15:
    case 16:
        return r7_handleVerbPickUp();
    case 8:
    case 21:
    case 22:
        return r8_handleVerbPickUp();
    case 9:
        return r9_handleVerbPickUp();
    case 10:
        return r10_handleVerbPickUp();
    case 11:
        return r11_handleVerbPickUp();
    case 12:
        return r12_handleVerbPickUp();
    case 13:
    case 18:
        return r13_handleVerbPickUp();
    case 14:
    case 19:
    case 20:
        return r14_handleVerbPickUp();
    case 30:
        return r30_handleVerbPickUp();
    case 32:
    case 33:
        return r32_handleVerbPickUp();
    case 29:
        return r29_handleVerbPickUp();
    case 34:
        return r34_handleVerbPickUp();
    case 36:
        return r36_handleVerbPickUp();
    case 37:
        return r37_handleVerbPickUp();
    }
	return 0;
}

int GameLogic::handleVerbUse() {
    switch (_vm->_currentRoomNumber) {
    case 0:
        return r0_handleVerbUse();
    case 1:
        return r1_handleVerbUse();
    case 2:
        return r2_handleVerbUse();
    case 3:
    case 23:
        return r3_handleVerbUse();
    case 4:
        return r4_handleVerbUse();
    case 5:
        return r5_handleVerbUse();
    case 6:
        return r6_handleVerbUse();
    case 7:
    case 15:
    case 16:
        return r7_handleVerbUse();
    case 8:
    case 21:
    case 22:
        return r8_handleVerbUse();
    case 10:
        return r10_handleVerbUse();
    case 11:
        return r11_handleVerbUse();
    case 12:
        return r12_handleVerbUse();
    case 9:
        return r9_handleVerbUse();
    case 13:
    case 18:
        return r13_handleVerbUse();
    case 14:
    case 19:
    case 20:
        return r14_handleVerbUse();
    case 24:
        return r24_handleVerbUse();
    case 25:
    case 26:
    case 27:
        return r25_handleVerbUse();
    case 28:
        return r28_handleVerbUse();
    case 29:
        return r29_handleVerbUse();
    case 30:
        return r30_handleVerbUse();
    case 31:
        return r31_handleVerbUse();
    case 32:
        return r32_handleVerbUse();
    case 34:
        return r34_handleVerbUse();
    case 35:
        return r35_handleVerbUse();
    case 36:
        return r36_handleVerbUse();
    case 37:
        return r37_handleVerbUse();
    case 17:
        return r17_handleVerbUse();
    case 38:
        return r38_handleVerbUse();
    case 39:
        return r39_handleVerbUse();
    }
	return 0;
}

void GameLogic::handleVerbTalkTo() {
    switch (_vm->_currentRoomNumber) {
    case 1:
        r1_handleVerbTalkTo();
        break;
    case 2:
        r2_handleVerbTalkTo();
        break;
    case 4:
        r4_handleVerbTalkTo();
        break;
    case 13:
        r13_handleVerbTalkTo();
        break;
    case 9:
        r9_handleVerbTalkTo();
        break;
	case 10:
		r10_handleVerbTalkTo();
        break;
    case 19:
    case 20:
        r19_handleVerbTalkTo();
        break;
    case 12:
        r12_handleVerbTalkTo();
        break;
    case 7:
    case 15:
        r7_handleVerbTalkTo();
        break;
    case 3:
    case 23:
        r3_handleVerbTalkTo();
        break;
    case 32:
        r32_handleVerbTalkTo();
        break;
    case 34:
        r34_handleVerbTalkTo();
        break;
    case 35:
        r35_handleVerbTalkTo();
        break;
    case 17:
        r17_handleVerbTalkTo();
        break;
    }
}

int GameLogic::handleVerbPush() {
    switch (_vm->_currentRoomNumber) {
    case 1:
        return r1_handleVerbPush();
    case 10:
        return r10_handleVerbPush();
    case 9:
        return r9_handleVerbPush();
    case 5:
        return r5_handleVerbPush();
    case 11:
        return r11_handleVerbPush();
    case 7:
    case 15:
    case 16:
        return r7_handleVerbPush();
    case 37:
        return r37_handleVerbPush();
    }
	return 0;
}

int GameLogic::handleVerbPull() {
    switch (_vm->_currentRoomNumber) {
    case 1:
        return r1_handleVerbPull();
    case 6:
        return r6_handleVerbPull();
    case 37:
        return r37_handleVerbPull();
    case 38:
        return r38_handleVerbPull();
    }
	return 0;
}

int GameLogic::handleVerbOpen() {
    switch (_vm->_currentRoomNumber) {
    case 0:
        return r0_handleVerbOpen();
    case 1:
        return r1_handleVerbOpen();
    case 5:
        return r5_handleVerbOpen();
    case 4:
        return r4_handleVerbOpen();
    case 10:
        return r10_handleVerbOpen();
    case 6:
        return r6_handleVerbOpen();
    case 7:
    case 15:
    case 16:
        return r7_handleVerbOpen();
    case 8:
    case 22:
        return r8_handleVerbOpen();
    case 32:
        return r32_handleVerbOpen();
    case 34:
        return r34_handleVerbOpen();
    case 39:
        return r39_handleVerbOpen();
    }
	return 0;
}

int GameLogic::handleVerbClose() {
    switch (_vm->_currentRoomNumber) {
    case 1:
        return r1_handleVerbClose();
    case 8:
    case 22:
        return r8_handleVerbClose();
    case 34:
        return r34_handleVerbClose();
    }
	return 0;
}

void GameLogic::handleVerbExtremeCloseupOf() {
    switch (_vm->_objectNumber) {
    case kObjectIdInventoryPizzathonList:
        displayExtremeCloseupOfPizzathonList();
        break;
    case kObjectIdSign15_1:
        displayExtremeCloseupOfSign15();
        break;
    case kObjectIdInventorySewerMap:
    case kObjectIdMap:
        displayExtremeCloseupOfSewerMap();
        break;
    case kObjectIdPictures22:
        displayExtremeCloseupOfObjectPictures22();
        break;
    case kObjectIdInventoryMemo:
    case kObjectIdMemo:
        displayExtremeCloseupOfMemo();
        break;
    case kObjectIdBillboard7:
    case kObjectIdBillboard14:
    case kObjectIdBillboard19:
        displayExtremeCloseupOfBillboard1();
        break;
    case kObjectIdBillboard_0:
    case kObjectIdBillboard_1:
    case kObjectIdBillboard_2:
        displayExtremeCloseupOfBillboard2();
        break;
    default:
        _vm->displayText("c00", 3, 0, -1, -1, 0);
        break;
    }
}

int GameLogic::handleVerbGive() {
    switch (_vm->_currentRoomNumber) {
    case 2:
        return r2_handleVerbGive();
    case 6:
        return r6_handleVerbGive();
    case 10:
        return r10_handleVerbGive();
    case 13:
        return r13_handleVerbGive();
    case 12:
        return r12_handleVerbGive();
    case 9:
        return r9_handleVerbGive();
    }
	return 0;
}

void GameLogic::handleDialogReply(int index, int x, int y) {

    if (index < 537) {
        _vm->displayText("c04r", index, 0, x, y, 0);
    } else {
        _vm->displayText("c04r2", index, 0, x, y, 0);
    }

    switch (_vm->_currentRoomNumber) {
    case 2:
        if (index < 300) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("btalk", 0, 2, 95, 72, 0, 100);
            }
        } else {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("wait", 0, 2, 237, 69, 0, 100);
            }
        }
        break;
    case 4:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("btalk", 1, -2, 146, 45, 0, 100);
        }
        break;
    case 6:
        if (index < 154) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
            }
        }
        break;
    case 13:
        if (index > 400) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("ctalk", 0, 2, 85, 73, 0, 100);
            }
        } else {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("ptalk", 0, 2, 72, 60, 0, 100);
            }
        }
        break;
    case 9:
        if (index < 192) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("talk", 0, 2, 92, 60, 0, 100);
            }
        } else {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("mtalk", 0, 2, 215, 60, 0, 100);
            }
        }
        break;
    case 10:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("talk", 0, 8, 153, 30, 1, 100);
        }
        break;
    case 23:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("talk", 0, 2, 75, 63, 0, 100);
        }
        break;
    case 19:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("gill", 1, -2, 273, 84, 0, 100);
        }
        break;
    case 20:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("mtalk", 0, 2, 184, 68, 0, 100);
        }
        break;
    case 12:
        if (index < 236) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("lawyer", 6, -2, 55, 84, 0, 100);
            }
        } else if (index < 238) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("man1", 0, 2, 108, 83, 0, 100);
            }
        } else if (index < 240) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("man3", 0, 2, 168, 84, 0, 100);
            }
        } else if (index < 242) {
            for (int i = 0; i < 4; i++) {
                _vm->playAnimation("man4", 0, 2, 190, 84, 0, 100);
            }
        }
        break;
    case 8:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("mtalk", 0, 2, 195, 69, 0, 100);
        }
        break;
    case 32:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("jtalk", 0, 2, 262, 84, 0, 100);
        }
        break;
    case 30:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("ctalk", 0, 2, 183, 107, 0, 100);
        }
        break;
    case 28:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("reptalk", 0, 6, 26, 51, 1, 100);
        }
        break;
    case 34:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("talk", 0, 2, 203, 73, 0, 100);
        }
        break;
    case 35:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("cass", 0, 2, 179, 68, 0, 100);
        }
        break;
    case 17:
        for (int i = 0; i < 4; i++) {
            _vm->playAnimation("g1talk", 0, 2, 54, 81, 0, 100);
        }
        break;
    default:
        _vm->waitSeconds(2);
        break;
    }
    _vm->waitSeconds(2);
    _vm->_isTextVisible = false;
    _vm->refreshActors();
}

bool GameLogic::handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
    switch (_vm->_currentRoomNumber) {
    case 0:
        return r0_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 1:
        return r1_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 2:
        return r2_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 4:
        return r4_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 5:
        return r5_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 6:
        return r6_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 9:
        return r9_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 10:
        return r10_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 13:
        return r13_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 8:
        return r8_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 32:
        return r32_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 19:
    case 20:
        return r19_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 12:
        return r12_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 15:
        return r15_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 23:
        return r3_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 30:
        return r30_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 28:
        return r28_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 31:
        return r31_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 34:
        return r34_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 35:
        return r35_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    case 17:
        return r17_handleDialogSelect(replyTextX, replyTextY, replyTextIndex1, replyTextIndex2, replyTextIndex3);
    }
	return 0;
}

void GameLogic::refreshRoomBackground(int roomNum) {
    switch (roomNum) {
    case 0:
        r0_refreshRoomBackground();
        break;
    case 1:
        r1_refreshRoomBackground();
        break;
    case 2:
        r2_refreshRoomBackground();
        break;
    case 3:
        r3_refreshRoomBackground();
        break;
    case 5:
        r5_refreshRoomBackground();
        break;
    case 6:
        r6_refreshRoomBackground();
        break;
    case 4:
        r4_refreshRoomBackground();
        break;
    case 7:
        r7_refreshRoomBackground();
        break;
    case 8:
        r8_refreshRoomBackground();
        break;
    case 10:
        r10_refreshRoomBackground();
        break;
    case 11:
        r11_refreshRoomBackground();
        break;
    case 12:
        r12_refreshRoomBackground();
        break;
    case 16:
        r16_refreshRoomBackground();
        break;
    case 13:
        r13_refreshRoomBackground();
        break;
    case 14:
        r14_refreshRoomBackground();
        break;
    case 19:
        r19_refreshRoomBackground();
        break;
    case 20:
        r20_refreshRoomBackground();
        break;
    case 9:
        r9_refreshRoomBackground();
        break;
    case 18:
        r18_refreshRoomBackground();
        break;
    case 22:
        r22_refreshRoomBackground();
        break;
    case 24:
        r24_refreshRoomBackground();
        break;
    case 25:
    case 26:
    case 27:
        r25_refreshRoomBackground();
        break;
    case 28:
        r28_refreshRoomBackground();
        break;
    case 29:
        r29_refreshRoomBackground();
        break;
    case 30:
        r30_refreshRoomBackground();
        break;
    case 31:
        r31_refreshRoomBackground();
        break;
    case 32:
        r32_refreshRoomBackground();
        break;
    case 33:
        r33_refreshRoomBackground();
        break;
    case 34:
        r34_refreshRoomBackground();
        break;
    case 35:
        r35_refreshRoomBackground();
        break;
    case 36:
        r36_refreshRoomBackground();
        break;
    case 37:
        r37_refreshRoomBackground();
        break;
    case 38:
        r38_refreshRoomBackground();
        break;
    case 17:
        r17_refreshRoomBackground();
        break;
    case 39:
        r39_refreshRoomBackground();
        break;
    }
}

void GameLogic::updateRoomAnimations(bool doUpdate) {
    switch (_vm->_currentRoomNumber) {
	case 1:
		r1_updateRoomAnimations(doUpdate);
		break;
	case 4:
		r4_updateRoomAnimations(doUpdate);
		break;
	case 6:
		r6_updateRoomAnimations(doUpdate);
		break;
	case 7:
		r7_updateRoomAnimations(doUpdate);
		break;
	case 8:
		r8_updateRoomAnimations(doUpdate);
		break;
	case 9:
		r9_updateRoomAnimations(doUpdate);
		break;
	case 11:
		r11_updateRoomAnimations(doUpdate);
		break;
	case 18:
		r18_updateRoomAnimations(doUpdate);
		break;
	case 29:
		r29_updateRoomAnimations(doUpdate);
		break;
	case 32:
		r32_updateRoomAnimations(doUpdate);
		break;
	case 34:
		r34_updateRoomAnimations(doUpdate);
		break;
	case 36:
		r36_updateRoomAnimations(doUpdate);
		break;
	case 38:
		r38_updateRoomAnimations(doUpdate);
		break;
	}
}

void GameLogic::handleRoomEvent(int eventNum) {
    switch (_vm->_currentRoomNumber) {
    case 0:
        switch (eventNum) {
        case 1:
            // TODO r0_handleRoomEvent1();
            break;
        case 2:
            r0_handleRoomEvent2();
            break;
        default:
            r0_handleRoomEvent3();
            break;
        }
        break;
    case 1:
        r1_handleRoomEvent(false);
        break;
    case 5:
        r5_handleRoomEvent();
        break;
    case 6:
        switch (eventNum) {
        case 1:
            r6_handleRoomEvent1();
            break;
        case 2:
            r6_handleRoomEvent2();
            break;
        case 3:
            r6_handleRoomEvent3();
            break;
        case 4:
            r6_handleRoomEvent4();
            break;
        }
        break;
    case 4:
        r4_handleRoomEvent();
        break;
    case 19:
        r19_handleRoomEvent();
        break;
    case 20:
        r20_handleRoomEvent();
        break;
    case 13:
        r13_handleRoomEvent();
        break;
    case 8:
        switch (eventNum) {
        case 1:
            r8_handleRoomEvent1();
            break;
        case 2:
            r8_handleRoomEvent2();
            break;
        case 3:
            r8_handleRoomEvent3();
            break;
        case 4:
            r8_handleRoomEvent4();
            break;
        }
        break;
    case 32:
        r32_handleRoomEvent();
        break;
    case 28:
        switch (eventNum) {
        case 1:
            r28_handleRoomEvent1();
            break;
        default:
            r28_handleRoomEvent2();
            break;
        }
        break;
    case 29:
        r29_handleRoomEvent();
        break;
    case 24:
        switch (eventNum) {
        case 1:
            r24_handleRoomEvent(92, 72, -43, 90, 38, 0);
            _vm->drawActors(0, 1, 1, 0, 65, 136, 145, 136);
            break;
        default:
            r24_handleRoomEvent(203, 183, -43, 90, 38, 1);
            _vm->drawActors(0, 1, 1, 0, 167, 136, 250, 136);
            break;
        }
        break;
    case 30:
        switch (eventNum) {
        case 1:
            r30_handleRoomEvent1();
            break;
        case 2:
            r30_handleRoomEvent2();
            break;
        }
        break;
    case 31:
        switch (eventNum) {
        case 1:
            r31_handleRoomEvent1();
            break;
        case 2:
            r31_handleRoomEvent2();
            break;
        case 3:
            r31_handleRoomEvent3();
            break;
        case 4:
            r31_handleRoomEvent4();
            break;
        case 5:
            r31_handleRoomEvent5();
            break;
        }
        break;
    case 35:
        r35_talkToCassandra();
        break;
    case 37:
        r37_handleRoomEvent();
        break;
    case 36:
        r36_handleRoomEvent();
        break;
    case 17:
        switch (eventNum) {
        case 1:
            r17_handleRoomEvent1();
            break;
        case 2:
            r17_handleRoomEvent2();
            break;
        case 3:
            r17_handleRoomEvent3();
            break;
        }
        break;
    }
}

int GameLogic::getActorScaleFromY(int actorY) {
	int scale = 100;
	switch (_vm->_currentRoomNumber) {
	case 0:
		scale = actorY - 10;
		break;
	case 1:
		scale = actorY - 27;
		break;
	case 2:
		scale = actorY * 2 - 130;
		break;
	case 3:
		scale = actorY * 2 - 124;
		break;
	case 4:
		scale = actorY * 2 - 84;
		break;
	case 5:
		scale = actorY;
		break;
	case 6:
		scale = actorY / 3 - 91;
		break;
	case 7:
		scale = actorY - 41;
		break;
	case 8:
	case 22:
		scale = actorY - 20;
		break;
	case 11:
		scale = actorY - 60;
		break;
	case 12:
		scale = actorY * 2 - 155;
		break;
	case 13:
		scale = actorY * 2 - 116;
		break;
	case 14:
	case 19:
	case 25:
		scale = actorY - 30;
		break;
	case 16:
		scale = actorY * 2 - 100;
		break;
	case 9:
	case 15:
		scale = actorY * 2 - 115;
		break;
	case 18:
		scale = actorY - 15;
		break;
	case 20:
		scale = actorY * 3 - 242;
		break;
	case 26:
		scale = actorY * 3 / 2 - 95;
		break;
	case 27:
		scale = actorY * 3 / 2 - 100;
		break;
	case 28:
		scale = actorY - 25;
		break;
	case 29:
		scale = actorY - 20;
		break;
	case 30:
		scale = actorY / 2 + 23;
		break;
	case 32:
	case 33:
		scale = actorY * 1.5 - 97;
		break;
	case 34:
		scale = actorY * 2 - 127;
		break;
	case 35:
		scale = actorY * 1.5 - 98;
		break;
	case 36:
		scale = actorY * 1.4 - 83;
		break;
	case 37:
		scale = actorY * 2 - 41;
		break;
	case 38:
		scale = actorY - 31;
		break;
	case 17:
		scale = actorY * 1.5 - 100;
		break;
	}
    return CLIP(scale, 20, 100);
}

void GameLogic::displayExtremeCloseupOfPizzathonList() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    _vm->paletteFadeOut(0, 256, 16);
    // sysMouseDriver(2);
    _vm->drawImageToScreen("m05/list", 0, 0);
    _vm->playSound("sv14", 1);
    if (_pizzathonListFlags1 & 0x08) {
        _vm->drawImageToScreen("m05/adline", 72, 22);
    }
    if (_pizzathonListFlags1 & 0x40) {
        _vm->drawImageToScreen("m05/locline", 72, 42);
    }
    if (_pizzathonListFlags1 & 0x80) {
        _vm->drawImageToScreen("m05/volline", 74, 59);
    }
    if (_pizzathonListFlags2 & 0x01) {
        _vm->drawImageToScreen("m05/timeline", 66, 76);
    }
    if (_pizzathonListFlags2 & 0x02) {
        _vm->drawImageToScreen("m05/totline", 73, 94);
    }
    if (_pizzathonListFlags1 & 0x02) {
        _vm->drawImageToScreen("m05/vidline", 80, 111);
    }
    if (_pizzathonListFlags1 & 0x04) {
        _vm->drawImageToScreen("m05/ingline", 72, 129);
    }
    if (_pizzathonListFlags1 & 0x10) {
        _vm->drawImageToScreen("m05/musline", 78, 148);
    }
    if (_pizzathonListFlags1 & 0x01) {
        _vm->drawImageToScreen("m05/magline", 61, 164);
    }
    if (_pizzathonListFlags1 & 0x20) {
        _vm->drawImageToScreen("m05/comline", 68, 100);
    }
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfSign15() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->drawRoomImageToScreen("sign", 0, 0);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfObjectPictures22() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->drawImageToScreen("m05/oldmap", 0, 0);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfMemo() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->drawImageToScreen("m05/memo", 0, 0);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfSewerMap() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->drawImageToScreen("m05/sewermap", 0, 0);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfBillboard1() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->_screen->clear(0);
    _vm->drawImageToScreen("m05/fboard", 51, 18);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

void GameLogic::displayExtremeCloseupOfBillboard2() {
    _vm->stopRoomAnimations();
    _vm->_gameState = 5;
    // sysMouseDriver(2);
    _vm->paletteFadeOut(0, 256, 16);
    _vm->playSound("sv14", 1);
    _vm->_screen->clear(0);
    _vm->drawImageToScreen("m05/pboard", 51, 18);
    _vm->paletteFadeIn(0, 256, 16);
    // sysMouseDriver(1);
}

int GameLogic::r0_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdGuitar:
		if (_vm->_currentActorNum != 0) {
			_vm->pickupObject(kObjectIdGuitar, _r0_flags, 1, kObjectIdInventoryGuitar);
			actionTextIndex = 1;
		} else {
			actionTextIndex = 2;
		}
		break;
	case kObjectIdHockeyStick:
		_vm->pickupObject(kObjectIdHockeyStick, _r0_flags, 4, kObjectIdInventoryHockeyStick);
		actionTextIndex = 50;
		break;
	case kObjectIdLaundryBasket0:
		_vm->pickupObject(kObjectIdLaundryBasket0, _r0_flags, 8, kObjectIdInventoryLaundryBasket);
		actionTextIndex = 50;
		break;
	case kObjectIdStairs0:
		actionTextIndex = 7;
		break;
	case kObjectIdLaundryRoom:
		actionTextIndex = 8;
		break;
	case kObjectIdCueCards:
		actionTextIndex = 9;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r0_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdStairs0:
		_vm->_gameState = 1;
		break;
	case kObjectIdCouch:
	case kObjectIdChair:
		actionTextIndex = 1;
		break;
	case kObjectIdCueCards:
	case kObjectIdCamera0:
		actionTextIndex = 66;
		break;
	case kObjectIdGuitar:
	case kObjectIdSchoolClothes:
	case kObjectIdLaundryBasket:
		actionTextIndex = 3;
		break;
	case kObjectIdLaundryRoom:
		actionTextIndex = 4;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r0_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdStairs0:
	case kObjectIdCouch:
		actionTextIndex = 5;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r0_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = -1;
	replyTextY = -1;
	switch (_vm->_selectedDialogChoice) {
	case 29: case 30: case 31:
		continueDialog = true;
		replyTextIndex1 = _vm->_selectedDialogChoice + 10;
		break;
	case 33:
		continueDialog = true;
		replyTextIndex1 = 44;
		break;
	case 32:
		continueDialog = true;
		_vm->setDialogChoices(34, 35, 36, -1, -1);
		replyTextIndex1 = 42;
		replyTextIndex2 = 43;
		break;
	case 34: case 35: case 36:
		_vm->_roomEventNum = 2;
		break;
	case 37: case 38: case 39: case 40: case 41:
	case 42: case 43: case 44: case 45: case 46:
	case 47: case 48: case 49: case 50: case 51:
		_word_34466 = _word_34466 + 1;
		if (_vm->_selectedDialogChoice < 47) {
			replyTextIndex1 = 45;
			_word_34464 = _word_34464 + 1;
		} else {
			replyTextIndex1 = _vm->_selectedDialogChoice - 1;
		}
		if (_word_34464 == 10) {
			_vm->_roomEventNum = 3;
		} else {
			continueDialog = true;
			r0_buildRandomDialogChoices(_vm->_selectedDialogChoice);
		}
		break;
	default:
		break;
	}
	return continueDialog;
}

void GameLogic::r0_refreshRoomBackground() {
	if (!(_r0_flags & 0x01)) {
		_vm->drawRoomImageToBackground("guitar", 18, 84);
	}
	if (!(_r0_flags & 0x04)) {
		_vm->drawRoomImageToBackground("hockey", 235, 66);
	}
	if (!(_r0_flags & 0x08)) {
		_vm->drawRoomImageToBackground("basket", 217, 76);
	}
	if (!(_r0_flags & 0x02)) {
		_vm->_roomEventNum = 1;
	}
}

void GameLogic::r0_handleRoomEvent2() {
	_vm->displayTextLines("c11", 1, -1, -1, 2);
	_vm->_currentActorNum = 0;
	_vm->displayTextLines("c11", 3, -1, -1, 2);
	_vm->_currentActorNum = 1;
	_vm->displayTextLines("c11", 5, -1, -1, 5);
	_vm->setDialogChoices(-1, -1, -1, -1, -1);
	r0_buildRandomDialogChoices(-1);
	_vm->startDialog();
}

void GameLogic::r0_handleRoomEvent3() {
	for (int textIndex = 0; textIndex < 4; textIndex++) {
		_vm->displayTextLines("c11", textIndex + 10, -1, -1, 1);
		_vm->_currentActorNum = (_vm->_currentActorNum + 1) % 2;
		_vm->waitSeconds(1);
	}
	_vm->moveObjectToRoom(kObjectIdInventoryPizzathonList, 99);
	_vm->_gameState = 0;
	_vm->drawInterface(_vm->_verbNumber);
}

void GameLogic::r0_buildRandomDialogChoices(int selectedDialogChoice) {
	// TODO
}

int GameLogic::r1_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdRemoteControl:
		_vm->pickupObject(kObjectIdRemoteControl, _r1_flags1, 4, kObjectIdInventoryRemoteControl);
		break;
	case kObjectIdSuckCut1:
		_vm->pickupObject(kObjectIdSuckCut1, _r1_flags1, 8, kObjectIdInventorySuckCut);
		break;
	case kObjectIdExtensionCord_0:
		_vm->pickupObject(kObjectIdExtensionCord_0, _r1_flags2, 4, kObjectIdInventoryExtensionCord);
		break;
	case kObjectIdWindow1:
		actionTextIndex = 7;
		break;
	case kObjectIdDisplay1:
		actionTextIndex = 8;
		break;
	case kObjectIdUnknown:
		actionTextIndex = 10;
		break;
	case kObjectIdBoxes:
		actionTextIndex = 11;
		break;
	case kObjectIdScreen:
		actionTextIndex = 12;
		break;
	case kObjectIdMice:
		actionTextIndex = 44;
		break;
	case kObjectIdDrGadget_1:
		actionTextIndex = 45;
		break;
	case kObjectIdLock_0:
		actionTextIndex = 46;
		break;
	case kObjectIdOutlet1:
		actionTextIndex = 12;
		break;
	case kObjectIdExit1:
		actionTextIndex = 14;
		break;
	case kObjectIdLever:
		actionTextIndex = 60;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r1_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit1:
		r1_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	case kObjectIdUnknown:
		actionTextIndex = 5;
		break;
	case kObjectIdSuckCut1:
	case kObjectIdRemoteControl:
		actionTextIndex = 3;
		break;
	case kObjectIdOutlet1:
		actionTextIndex = 6;
		break;
	case kObjectIdBoxes:
		actionTextIndex = 7;
		break;
	case kObjectIdScreen:
		actionTextIndex = 78;
		break;
	case kObjectIdMachine:
		if (_vm->_firstObjectNumber == kObjectIdInventoryFreshDonut) {
			actionTextIndex = 75;
		} else if (_vm->_firstObjectNumber == kObjectIdInventoryDayOldDonut) {
			r1_useDayOldDonutWithMachine();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r1_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDrGadget_0:
		_vm->displayTextLines("c04r", 412, 200, 30, 3);
		_vm->setDialogChoices(387, 388, 389, 386, -1);
		break;
	case kObjectIdDrGadget_1:
		_vm->displayTextLines("c04r", 112, 200, 30, 2);
		_vm->setDialogChoices(190, 191, 192, -1, -1);
		break;
	default:
		break;
	}
}

int GameLogic::r1_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLever:
		if (!(_r1_flags1 & 0x20)) {
			actionTextIndex = 1;
		} else {
			_r1_flags1 &= ~0x20;
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r1_handleVerbPull() {
	int actionTextIndex = -1;
	if (!(_r1_flags1 & 0x40) || _r1_eventFlag == 0) {
		r1_checkDrGadget();
	} else {
		switch (_vm->_objectNumber) {
		case kObjectIdScreen:
			if (_r1_eventFlag != 0) {
				actionTextIndex = 6;
			} else {
				r1_pullScreen();
			}
			break;
		case kObjectIdLever:
			if (_r1_flags1 & 0x20) {
				actionTextIndex = 2;
			} else {
				r1_pullLever();
			}
			break;
		default:
			actionTextIndex = 0;
			break;
		}
	}
	return actionTextIndex;
}

int GameLogic::r1_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdSafe_0:
		r1_openSafe();
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r1_handleVerbClose() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdSafe_0:
		r1_closeSafe();
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r1_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 190: case 191:
		_vm->displayTextLines("c04r", 114, 200, 30, 4);
		_vm->_gameState = 0;
		break;
	case 192: case 386: case 394:
		_vm->_gameState = 0;
		break;
	case 387:
		_vm->displayTextLines("c04r", 415, 200, 30, 3);
		_vm->displayTextLines("c04", 390, -1, -1, 1);
		continueDialog = true;
		break;
	case 388:
		replyTextIndex1 = 418;
		replyTextIndex2 = 419;
		replyTextIndex3 = 420;
		continueDialog = true;
		_vm->setDialogChoices(391, 392, 393, 394, -1);
		break;
	case 389:
		replyTextIndex1 = 421;
		replyTextIndex2 = 422;
		replyTextIndex3 = 423;
		continueDialog = true;
		break;
	case 391:
		_vm->displayTextLines("c04r", 424, 200, 30, 1);
		_vm->displayTextLines("c04", 395, -1, -1, 4);
		_vm->displayTextLines("c04r", 425, 200, 30, 2);
		continueDialog = true;
		break;
	case 392:
		_vm->displayTextLines("c04r", 427, 200, 30, 4);
		if (_vm->getObjectRoom(kObjectIdInventoryFreshDonut) == 99 || _vm->getObjectRoom(kObjectIdInventoryDayOldDonut) == 99) {
			_vm->displayTextLines("c04r", 431, 200, 30, 1);
			_vm->displayTextLines("c04", 399, -1, -1, 1);
			if (_vm->getObjectRoom(kObjectIdInventoryFreshDonut) != 99) {
				_vm->setDialogChoices(401, -1, -1, -1, -1);
			} else if (_vm->getObjectRoom(kObjectIdInventoryDayOldDonut) != 99) {
				_vm->setDialogChoices(400, -1, -1, -1, -1);
			} else {
				_vm->setDialogChoices(400, 401, -1, -1, -1);
			}
			replyTextIndex1 = 433;
		} else {
			replyTextIndex1 = 431;
		}
		continueDialog = true;
		break;
	case 393:
		replyTextIndex1 = 432;
		continueDialog = true;
		break;
	case 400:
		replyTextIndex1 = 434;
		_vm->_gameState = 0;
		break;
	case 401:
		replyTextIndex1 = 435;
		replyTextIndex2 = 436;
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r1_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "term", 10);
	_vm->loadAnimationSpriteRange(10, "mice", 5);
	_vm->startRoomAnimations();
}

void GameLogic::r1_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
	_vm->loadPalette("m01/wstand0");
}

void GameLogic::r1_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 5000;
	if (_vm->_animationsCtr % 500 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 500, 270, 47);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
	if (!(_r1_flags1 & 0x10) && (_vm->_animationsCtr % 200 == 55 || doUpdate)) {
		_vm->drawAnimationSprite(10 + _vm->getRandom(5), 179, 67);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
	if (_r1_eventFlag != 0 && _vm->_animationsCtr % 3000 == 144) {
		_vm->_roomEventNum = _r1_eventCtr;
	}
}

void GameLogic::r1_refreshRoomBackground() {
	if (_pizzathonListFlags2 & 0x08) {
		_vm->loadRoomMask(97);
		_vm->drawRoomImageToBackground("norobot", 64, 68);
		_vm->drawRoomImageToBackground("machine", 82, 50);
		if (_r1_flags2 & 0x02) {
			_vm->drawRoomImageToBackground("donon", 86, 104);
		}
		if (!(_r1_flags2 & 0x04)) {
			_vm->drawRoomImageToBackground("cord", 143, 106);
		}
		if (!(_r1_flags2 & 0x08)) {
			_vm->moveObjectToRoom(kObjectIdDrGadget_0, 1);
			_vm->drawRoomImageToBackground("gadget2", 127, 82);
			_vm->fillRoomMaskArea(100, 0, 319, 149, 1);
		} else {
			_vm->moveObjectToNowhere(kObjectIdDrGadget_0);
			if (!(_r1_flags2 & 0x04)) {
				_vm->moveObjectToRoom(kObjectIdExtensionCord_0, 1);
			} else {
				_vm->fillRoomMaskArea(175, 0, 319, 149, 1);
			}
		}
		if (!(_r1_flags1 & 0x08)) {
			_vm->drawRoomImageToBackground("suckcut", 32, 80);
		}
		_vm->moveObjectToNowhere(kObjectIdLever);
		_vm->moveObjectToNowhere(kObjectIdMice);
		_vm->moveObjectToNowhere(kObjectIdWindow1);
		_vm->moveObjectToNowhere(kObjectIdScreen);
		_vm->moveObjectToNowhere(kObjectIdDisplay1);
		_vm->moveObjectToNowhere(kObjectIdFileCabinet);
		_vm->moveObjectToNowhere(kObjectIdBoxes);
		_vm->moveObjectToNowhere(kObjectIdOutlet1);
		_vm->moveObjectToNowhere(kObjectIdSafe_0);
		_vm->moveObjectToNowhere(kObjectIdCabinet1);
		_vm->moveObjectToNowhere(kObjectIdRobot1);
		_vm->moveObjectToRoom(kObjectIdMachine, 1);
		if (!_vm->_hasRoomAnimationCallback) {
			r1_initRoomAnimations();
		}
	} else {
		if (_r1_flags1 & 0x01) {
			_vm->drawRoomImageToBackground("screenup", 118, 53);
			if (_r1_flags1 & 0x02) {
				_vm->drawRoomImageToBackground("safeopen", 131, 69);
				if (!(_r1_flags1 & 0x04)) {
					_vm->drawRoomImageToBackground("remote", 138, 78);
				}
			}
		}
		if (!(_r1_flags1 & 0x08)) {
			_vm->drawRoomImageToBackground("suckcut", 32, 80);
		}
		if (_r1_flags1 & 0x20) {
			_vm->drawRoomImageToBackground("emptycg", 180, 66);
		} else if (_r1_flags1 & 0x10) {
			_vm->drawRoomImageToBackground("emptycls", 179, 67);
		}
		if (_r1_flags2 & ((_r1_flags1 & 0x40) + 1) & 0x01) {
			if (_r1_flags1 & 0x08) {
				_vm->drawRoomImageToBackground("gadin10", 0, 78);
			} else {
				_vm->drawRoomImageToBackground("scin10", 0, 78);
			}
		}
		if (_r1_flags1 & 0x80) {
			_vm->drawRoomImageToBackground("norobot", 64, 68);
		}
		if (_r1_eventFlag != 0) {
			switch (_r1_eventCtr) {
			case 2: case 9:
				_vm->drawRoomImageToBackground("tt011", 119, 81);
				break;
			case 3:
				_vm->drawRoomImageToBackground("tt111", 119, 81);
				break;
			case 4:
				_vm->drawRoomImageToBackground("tt211", 119, 81);
				break;
			case 5:
				_vm->drawRoomImageToBackground("tt311", 119, 81);
				break;
			case 6:
				_vm->drawRoomImageToBackground("tt46", 119, 81);
				break;
			case 7:
				_vm->drawRoomImageToBackground("tt56", 119, 81);
				break;
			case 8:
				_vm->drawRoomImageToBackground("tt66", 119, 81);
			default:
				break;
			}
		}
		if (!_vm->_hasRoomAnimationCallback) {
			r1_initRoomAnimations();
		}
	}
}

void GameLogic::r1_handleRoomEvent(bool arg6) {
	if (arg6) {
		_r1_eventCtr = -1;
	}
	switch (_r1_eventCtr) {
	case 1:
		_r1_eventFlag = 1;
		_r1_eventCtr = 0;
		break;
	case 3:
		_vm->walkTo(209, 120, 7, 151, 127);
		_vm->fillRoomMaskArea(0, 0, 180, 149, 1);
		_vm->_garthSpriteX = -1;
		_vm->playAnimation("tt0", 0, 12, 119, 81, 0, 150);
		break;
	case 4:
		_vm->playAnimation("tt1", 0, 12, 119, 81, 0, 150);
		break;
	case 5:
		_vm->playAnimation("tt2", 0, 12, 119, 81, 0, 150);
		break;
	case 6:
		_vm->playAnimation("tt3", 0, 12, 119, 81, 0, 150);
		_vm->playAnimation("tt3", 8, 4, 119, 81, 0, 150);
		break;
	case 7:
		_vm->playAnimation("tt4", 0, 7, 119, 81, 0, 150);
		_vm->playAnimation("tt4", 3, 4, 119, 81, 0, 150);
		break;
	case 8:
		_vm->playAnimation("tt5", 0, 7, 119, 81, 0, 150);
		_vm->playAnimation("tt5", 3, 4, 119, 81, 0, 150);
		break;
	case 9:
		_vm->playAnimation("tt6", 0, 19, 119, 81, 0, 150);
		_vm->playAnimation("tt6", 7, 7, 119, 81, 0, 150);
		_vm->playAnimation("tt6", 6, 1, 119, 81, 0, 150);
		break;
	case 10:
		_vm->playAnimation("tt7", 0, 9, 119, 81, 0, 150);
		_vm->playAnimation("tt7", 1, 6, 119, 81, 0, 150);
		_vm->playAnimation("tt0", 11, 1, 119, 81, 0, 150);
		break;
	case 11:
		_vm->playAnimation("tt0", 10, -11, 119, 81, 0, 150);
		break;
	default:
		break;
	}
	if (_r1_eventCtr > 0) {
		_vm->displayText("c04r", _r1_eventCtr + 124, 0, 170, 30, 0);
		_vm->waitSeconds(2);
		_vm->_isTextVisible = false;
	}
	if (_r1_eventCtr == 9) {
		_vm->refreshActors();
		_vm->_currentActorNum = 0;
		_vm->displayTextLines("c04", 199, -1, -1, 3);
		_vm->_currentActorNum = 1;
		_r1_eventFlag = 0;
		_r1_eventCtr = -1;
		_vm->_garthSpriteX = 146;
		_vm->loadRoomMask(_vm->_currentRoomNumber);
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
		if ((_r1_flags1 & 0x10) && !(_r1_flags1 & 0x40)) {
			r1_drGadgetLeaves();
		}
	}
	_r1_eventCtr = _r1_eventCtr + 1;
}

void GameLogic::r1_pullScreen() {
	if (_vm->_currentActorNum != 0) {
		_vm->walkTo(_vm->_wayneSpriteX - 1, _vm->_wayneSpriteY, _vm->_actorSpriteValue, 180, 115);
		_vm->_wayneSpriteX = -1;
		_vm->playAnimation("wscreen", 0, 7, 117, 53, 0, 100);
		_vm->_wayneSpriteX = 121;
	} else {
		_vm->walkTo(_vm->_garthSpriteX - 1, _vm->_garthSpriteY, _vm->_actorSpriteValue, 180, 115);
		_vm->_garthSpriteX = -1;
		_vm->playAnimation("gscreen", 0, 7, 117, 53, 0, 100);
		_vm->_garthSpriteX = 121;
	}
	_vm->moveObjectToNowhere(kObjectIdScreen);
	_vm->moveObjectToRoom(kObjectIdSafe_0, 1);
	_r1_flags1 |= 0x01;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r1_openSafe() {
	_vm->drawActorReachObject(kObjectIdSafe_0, 0);
	if (!(_r1_flags1 & 0x04)) {
		_vm->moveObjectToRoom(kObjectIdRemoteControl, 1);
	}
	_r1_flags1 |= 0x02;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r1_closeSafe() {
	_vm->drawActorReachObject(kObjectIdSafe_0, 0);
	_vm->moveObjectToNowhere(kObjectIdRemoteControl);
	_r1_flags1 &= ~0x02;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r1_pullLever() {
	if (!(_r1_flags1 & 0x10)) {
		_vm->_wayneSpriteX = -1;
		_r1_flags1 |= 0x10;
		_vm->playAnimation("freemc", 0, 2, 179, 67, 0, 100);
		_vm->playAnimation("fleemc", 0, 4, 174, 92, 0, 100);
		_vm->moveObjectToNowhere(kObjectIdMice);
		_vm->_wayneSpriteX = 236;
	}
	_r1_flags1 |= 0x20;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r1_checkDrGadget() {
	if (!(_r1_flags2 & 0x01)) {
		_vm->walkTo(145, 123, 6, 165, 120);
		if (_r1_flags1 & 0x08) {
			_vm->playAnimation("gadin", 0, 11, 0, 78, 0, 150);
		} else {
			_vm->playAnimation("scin", 0, 11, 0, 78, 0, 150);
		}
		_r1_flags2 |= 0x01;
		_vm->moveObjectToRoom(kObjectIdDrGadget_1, 1);
		_vm->setStaticRoomObjectPosition(1, 1, 1, 91, 78);
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
	}
	_vm->displayText("c04r", 137, 0, 170, 30, 0);
}

void GameLogic::r1_useDayOldDonutWithMachine() {
	_vm->moveObjectToNowhere(kObjectIdInventoryDayOldDonut);
	_vm->refreshInventory(true);
	_r1_flags2 |= 0x02;
	_vm->playAnimation("zap", 0, 32, 81, 47, 0, 30);
	_r1_flags2 |= 0x08;
	_vm->changeRoom(_vm->_currentRoomNumber);
}

void GameLogic::r1_drGadgetLeaves() {
	_vm->displayTextLines("c04r", 134, 170, 30, 3);
	_vm->moveObjectToNowhere(kObjectIdDrGadget_1);
	_vm->moveObjectToNowhere(kObjectIdRobot1);
	_r1_flags1 |= 0x40;
	_r1_flags2 &= ~0x01;
	_r1_flags1 |= 0x80;
	if (_r1_flags1 & 0x08) {
		_vm->playAnimation("gadout", 0, 21, 3, 68, 0, 100);
	} else {
		_vm->playAnimation("scout", 0, 21, 3, 68, 0, 100);
	}
	_vm->setStaticRoomObjectPosition(1, 1, 1, -1, 78);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

int GameLogic::r2_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdKitchen:
		actionTextIndex = 7;
		break;
	case kObjectIdKitchenDoor:
		actionTextIndex = 33;
		break;
	case kObjectIdCounter2_0:
		actionTextIndex = 35;
		break;
	case kObjectIdCounter2_1:
		actionTextIndex = 34;
		break;
	case kObjectIdDonutCase:
		actionTextIndex = 36;
		break;
	case kObjectIdExit2:
		actionTextIndex = 14;
		break;
	case kObjectIdDonut:
		_vm->pickupObject(kObjectIdDonut, _r2_flags, 32, kObjectIdInventoryFreshDonut);
		_r2_flags &= ~0x20;
		_vm->changeRoom(_vm->_currentRoomNumber);
		break;
	case kObjectIdBoxOfDonuts:
		_vm->moveObjectToRoom(kObjectIdInventoryDayOldDonut, 99);
		_vm->pickupObject(kObjectIdBoxOfDonuts, _r2_flags, 64, kObjectIdInventoryOldDonuts);
		_r2_flags &= ~0x40;
		_vm->changeRoom(_vm->_currentRoomNumber);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r2_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit2:
		_r2_flags &= ~0x80;
		_vm->_gameState = 1;
		break;
	case kObjectIdDonutCase:
		actionTextIndex = 38;
		break;
	case kObjectIdCounter2_0:
		actionTextIndex = 79;
		break;
	case kObjectIdCounter2_1:
		actionTextIndex = 79;
		break;
	case kObjectIdTables2_0:
	case kObjectIdTables2_1:
		actionTextIndex = 39;
		break;
	case kObjectIdBooths:
		actionTextIndex = 1;
		break;
	case kObjectIdKitchen:
		actionTextIndex = 40;
		break;
	case kObjectIdKitchenDoor:
		actionTextIndex = 41;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r2_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdBikerGang:
		_vm->setDialogChoices(296, 297, 298, 187, -1);
		break;
	case kObjectIdGirls:
		_vm->displayTextLines("c04", 299, -1, -1, 1);
		break;
	case kObjectIdWaitress:
		if (!(_r2_flags & 0x02) || !(_r2_flags & 0x04)) {
			_vm->displayTextLines("c04r", 391, 270, 20, 1);
			if ((_r2_flags & 0x08) && (_r2_flags & 0x10)) {
				if (_r2_flags & 0x02) {
					_vm->setDialogChoices(385, 386, -1, -1, -1);
				} else {
					if (_r2_flags & 0x04) {
						_vm->setDialogChoices(384, 386, -1, -1, -1);
					} else {
						_vm->setDialogChoices(384, 385, 386, -1, -1);
					}
				}
			} else {
				_vm->setDialogChoices(365, 366, 367, 368, -1);
			}
		} else {
			_vm->displayTextLines("c04r", 409, 270, 20, 1);
		}
	default:
		break;
	}
}

int GameLogic::r2_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdBikerGang:
		if (_vm->_firstObjectNumber == kObjectIdInventoryCandyBar) {
			r2_giveCandyBarToBikerGang();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r2_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	if (_vm->_selectedDialogChoice < 300) {
		replyTextX = 70;
		replyTextY = 30;
	} else {
		replyTextX = 260;
		replyTextY = 20;
	}
	switch (_vm->_selectedDialogChoice) {
	case 296:
		if (_r2_flags & 0x01) {
			replyTextIndex1 = 292;
		} else {
			replyTextIndex1 = 284;
			replyTextIndex2 = 296;
		}
		_vm->_gameState = 0;
		break;
	case 297:
		if (_r2_flags & 0x01) {
			replyTextIndex1 = 293;
		} else {
			replyTextIndex1 = 285;
			replyTextIndex2 = 296;
		}
		_vm->_gameState = 0;
		break;
	case 298:
		if (_r2_flags & 0x01) {
			_vm->setGameFlag(3);
			replyTextIndex1 = 294;
		} else {
			replyTextIndex1 = 286;
			replyTextIndex2 = 296;
		}
		_vm->_gameState = 0;
		break;
	case 187:
		if (_r2_flags & 0x01) {
			replyTextIndex1 = 295;
		} else {
			replyTextIndex1 = 287;
			replyTextIndex2 = 296;
		}
		_vm->_gameState = 0;
		break;
	case 369: case 384:
		if (_vm->_currentActorNum != 0 && _vm->_wayneInventory[kObjectIdInventoryDollar] < 1) {
			replyTextIndex1 = 579;
			continueDialog = true;
		} else if (_vm->_currentActorNum == 0 && _vm->_garthInventory[kObjectIdInventoryDollar] < 1) {
			replyTextIndex1 = 579;
			continueDialog = true;
		} else {
			r2_handleDialogSelect369();
		}
		_vm->_gameState = 0;
		break;
	case 385:
		if (_vm->_currentActorNum != 0 && _vm->_wayneInventory[kObjectIdInventoryDollar] < 10) {
			replyTextIndex1 = 579;
			continueDialog = true;
		} else if (_vm->_currentActorNum == 0 && _vm->_garthInventory[kObjectIdInventoryDollar] < 10) {
			replyTextIndex1 = 579;
			continueDialog = true;
		} else {
			r2_handleDialogSelect385();
		}
		_vm->_gameState = 0;
		break;
	case 366: case 386:
		_vm->_gameState = 0;
		break;
	case 365:
		replyTextIndex1 = 392;
		continueDialog = true;
		_r2_flags |= 0x08;
		_vm->setDialogChoices(369, 370, 366, -1, -1);
		break;
	case 367:
		replyTextIndex1 = 393;
		replyTextIndex2 = 394;
		continueDialog = true;
		_vm->setDialogChoices(372, 373, 374, 375, -1);
		break;
	case 368:
		replyTextIndex1 = 395;
		_vm->_gameState = 0;
		break;
	case 370:
		replyTextIndex1 = 398;
		_vm->_gameState = 0;
		break;
	case 372: case 373:
		replyTextIndex1 = _vm->_selectedDialogChoice + 27;
		continueDialog = true;
		break;
	case 374:
		replyTextIndex1 = 401;
		continueDialog = true;
		_vm->setDialogChoices(369, 370, 366, -1, -1);
		break;
	case 375:
		replyTextIndex1 = 402;
		continueDialog = true;
		_vm->setDialogChoices(376, 377, 378, -1, -1);
		break;
	case 376:
		replyTextIndex1 = 403;
		continueDialog = true;
		_vm->setDialogChoices(372, 373, 374, 375, -1);
		break;
	case 377:
		replyTextIndex1 = 404;
		replyTextIndex2 = 405;
		continueDialog = true;
		_r2_flags |= 0x10;
		if (_vm->_currentActorNum == 0) {
			if (_vm->_wayneInventory[kObjectIdInventoryDollar] >= 10) {
				_vm->setDialogChoices(379, 366, -1, -1, -1);
			} else {
				_vm->setDialogChoices(380, 366, -1, -1, -1);
			}
		} else if (_vm->_currentActorNum != 0) {
			if (_vm->_garthInventory[kObjectIdInventoryDollar] >= 10) {
				_vm->setDialogChoices(379, 366, -1, -1, -1);
			} else {
				_vm->setDialogChoices(380, 366, -1, -1, -1);
			}
		}
		break;
	case 378:
		replyTextIndex1 = 410;
		continueDialog = true;
		break;
	case 379:
		replyTextIndex1 = 406;
		continueDialog = true;
		_vm->setDialogChoices(381, 382, 383, -1, -1);
		break;
	case 380:
		replyTextIndex1 = 407;
		continueDialog = true;
		_vm->setDialogChoices(369, 370, 366, -1, -1);
		break;
	case 381:
		r2_handleDialogSelect385();
		_vm->_gameState = 0;
		break;
	case 382:
		replyTextIndex1 = 400;
		continueDialog = true;
		break;
	case 383:
		replyTextIndex1 = 408;
		continueDialog = true;
		_vm->setDialogChoices(369, 370, 366, -1, -1);
		break;
	}
	return continueDialog;
}

void GameLogic::r2_refreshRoomBackground() {
	if (!(_r2_flags & 0x80)) {
		if (!(_pizzathonListFlags2 & 0x08)) {
			_r2_backgroundValue2 = 10;
			_r2_backgroundValue1 = 10;
		} else {
			_r2_backgroundValue1 = _vm->getRandom(10);
			_r2_backgroundValue2 = _vm->getRandom(10);
		}
		_r2_flags |= 0x80;
	}
	if (_r2_backgroundValue1 < 7) {
		_vm->drawRoomImageToBackground("geeks", 152, 72);
	}
	if (!(_pizzathonListFlags2 & 0x08)) {
		_vm->drawRoomImageToBackground("gang", 0, 68);
	} else {
		_vm->moveObjectToNowhere(kObjectIdBikerGang);
	}
	if (_r2_backgroundValue2 < 5) {
		_vm->moveObjectToNowhere(kObjectIdGirls);
		_vm->setStaticRoomObjectPosition(2, 1, 1, 92, 103);
	} else {
		_vm->setStaticRoomObjectPosition(2, 2, 2, 92, 88);
		_vm->drawRoomImageToBackgroundTransparent("table2", 92, 88);
		_vm->moveObjectToRoom(kObjectIdGirls, 2);
	}
	_vm->drawRoomImageToBackground("wait1", 237, 69);
	if (!(_r2_flags & 0x04)) {
		_vm->drawRoomImageToBackground("box", 265, 83);
	} else if (_r2_flags & 0x40) {
		_vm->drawRoomImageToBackground("cbox", 226, 91);
	}
	if (_r2_flags & 0x20) {
		_vm->drawRoomImageToBackground("donut", 232, 95);
	}
}

void GameLogic::r2_giveCandyBarToBikerGang() {
	_vm->walkTo(128, 112, 6, 150, 100);
	_vm->playAnimation("getcb", 0, 1, 102, 79, 0, 150);
	_r2_flags |= 0x01;
	_vm->moveObjectToNowhere(kObjectIdInventoryCandyBar);
	_vm->refreshInventory(false);
	_vm->changeRoom(_vm->_currentRoomNumber);
	for (int textIndex = 288; textIndex < 291; textIndex++) {
		for (int i = 0; i < 4; i++) {
			_vm->playAnimation("btalk", 0, 2, 95, 72, 0, 100);
		}
		_vm->displayTextLines("c04r", textIndex, 70, 30, 1);
	}
	_vm->playAnimation("givech", 0, 2, 94, 70, 0, 150);
	_vm->moveObjectToRoom(kObjectIdInventoryChain, 99);
	_vm->refreshInventory(false);
	_vm->changeRoom(_vm->_currentRoomNumber);
	for (int i = 0; i < 4; i++) {
		_vm->playAnimation("btalk", 0, 2, 95, 72, 0, 100);
	}
	_vm->displayTextLines("c04r", 291, 70, 30, 1);
}

void GameLogic::r2_handleDialogSelect369() {
	_vm->displayTextLines("c04r", 396, 260, 20, 1);
	_vm->displayTextLines("c04", 371, -1, -1, 1);
	_vm->moveObjectToNowhere(kObjectIdInventoryDollar);
	_vm->refreshInventory(true);
	_vm->displayTextLines("c04r", 397, 260, 20, 1);
	_vm->playAnimation("getdon", 0, 3, 222, 69, 0, 100);
	_r2_flags |= 0x02;
	_r2_flags |= 0x20;
	_vm->moveObjectToRoom(kObjectIdDonut, 2);
	_vm->changeRoom(_vm->_currentRoomNumber);
}

void GameLogic::r2_handleDialogSelect385() {
	for (int i = 0; i < 10; i++) {
		_vm->moveObjectToNowhere(kObjectIdInventoryDollar);
	}
	_vm->refreshInventory(true);
	_vm->playAnimation("getbox", 0, 3, 226, 68, 0, 100);
	_r2_flags |= 0x04;
	_r2_flags |= 0x40;
	_vm->moveObjectToRoom(kObjectIdBoxOfDonuts, 2);
	_vm->changeRoom(_vm->_currentRoomNumber);
	_vm->displayTextLines("c04r", 411, 260, 20, 1);
}

void GameLogic::r3_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMan23:
		if (_pizzathonListFlags2 & 0x08) {
			_vm->displayTextLines("c04r2", 43, 100, 20, 1);
		} else {
			_vm->setDialogChoices(227, 228, 229, 187, -1);
		}
		break;
	case kObjectIdProtesters3:
		_vm->displayTextLines("c04r", 256, 100, 20, 2);
		break;
	}
}

int GameLogic::r3_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar3:
		_vm->_gameState = 1;
		break;
	case kObjectIdEntrance3:
		actionTextIndex = 42;
		break;
	case kObjectIdProtesters3:
		actionTextIndex = 70;
		break;
	case kObjectIdStairs3:
		_vm->walkTo(207, 147, 0, -1, -1);
		_vm->setWaynePosition(230, 105);
		_vm->setGarthPosition(242, 103);
		_vm->changeRoom(23);
		break;
	case kObjectIdExit23:
		_vm->setWaynePosition(205, 144);
		_vm->setGarthPosition(207, 147);
		_vm->changeRoom(3);
		break;
	case kObjectIdDesk23:
		actionTextIndex = 27;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r3_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar3:
		actionTextIndex = 35;
		break;
	case kObjectIdProtesters3:
		actionTextIndex = 55;
		break;
	case kObjectIdEntrance3:
	case kObjectIdExit23:
		actionTextIndex = 34;
		break;
	case kObjectIdStairs3:
		actionTextIndex = 14;
		break;
	case kObjectIdSatdish:
	case kObjectIdMonitors:
		actionTextIndex = 7;
		break;
	case kObjectIdSpace:
		actionTextIndex = 33;
		break;
	case kObjectIdGrill:
	case kObjectIdDesk23:
		actionTextIndex = 30;
		break;
	case kObjectIdAudioControls:
		actionTextIndex = 20;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r3_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 50;
	replyTextY = 35;
	switch (_vm->_selectedDialogChoice) {
	case 227:
		replyTextIndex1 = 167;
		continueDialog = true;
		break;
	case 228:
		if (!(_r19_flags & 0x08)) {
			_vm->displayTextLines("c04r", 168, 50, 35, 1);
			replyTextIndex1 = 169;
			replyTextIndex2 = 170;
			replyTextIndex3 = 171;
		} else {
			_vm->displayTextLines("c04r", 168, 50, 35, 2);
			_vm->displayTextLines("c04r", 173, 50, 35, 2);
			_vm->displayTextLines("c04", 230, -1, -1, 1);
			_vm->setGameFlag(4);
			replyTextIndex1 = 175;
		}
		continueDialog = true;
		break;
	case 229:
		if (!(_r19_flags & 0x08)) {
			replyTextIndex1 = 172;
		} else {
			replyTextIndex1 = 176;
		}
		continueDialog = true;
		break;
	case 187:
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r3_refreshRoomBackground() {
	if (_pizzathonListFlags2 & 0x08) {
		_vm->drawRoomImageToBackground("nosat", 232, 53);
		_vm->moveObjectToNowhere(kObjectIdProtesters3);
		_vm->moveObjectToNowhere(kObjectIdSatdish);
	} else {
		_vm->drawRoomImageToBackground("elvis", 0, 47);
		_vm->moveObjectToRoom(kObjectIdProtesters3, 3);
	}
}

int GameLogic::r4_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdOffice4:
		actionTextIndex = 8;
		break;
	case kObjectIdSign4:
		actionTextIndex = 22;
		break;
	case kObjectIdCastle:
		actionTextIndex = 32;
		break;
	case kObjectIdExit4:
		actionTextIndex = 33;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r4_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit4:
		r4_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	case kObjectIdSign4:
		actionTextIndex = 24;
		break;
	case kObjectIdOffice4:
		actionTextIndex = 35;
		break;
	case kObjectIdDesks:
	case kObjectIdDesk4:
	case kObjectIdDraftingBoard:
		actionTextIndex = 36;
		break;
	case kObjectIdCastle:
		if (_vm->_firstObjectNumber == kObjectIdInventoryCheesePizza) {
			actionTextIndex = r4_useCheesePizzaWithCastle();
		} else {
			actionTextIndex = 37;
		}
		break;
	case kObjectIdCastleDoor:
		if (_vm->_firstObjectNumber == kObjectIdInventoryDrumstick) {
			actionTextIndex = r4_useDrumstickWithCastleDoor();
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdDrumstick_1:
		if (_vm->_firstObjectNumber == kObjectIdInventoryChain) {
			r4_useChainWithObject(true, false);
		} else if (_vm->_firstObjectNumber == kObjectIdChain_1) {
			r4_useChainWithObject(true, true);
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdShade:
		if (_vm->_firstObjectNumber == kObjectIdInventoryChain) {
			r4_useChainWithObject(false, true);
		} else if (_vm->_firstObjectNumber == kObjectIdChain_1) {
			r4_useChainWithObject(true, true);
		} else if (_vm->_firstObjectNumber == kObjectIdInventoryTube) {
			actionTextIndex = r4_useTubeWithShade();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r4_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdTheBoss:
		if (_pizzathonListFlags2 & 0x08) {
			_vm->displayTextLines("c04r2", 43, 200, 20, 1);
		} else if (!(_pizzathonListFlags1 & 0x08)) {
			_vm->setDialogChoices(285, 286, 287, 288, -1);
		} else {
			_vm->displayTextLines("c04r", 251, 200, 20, 1);
			if (!(_r4_flags & 0x01)) {
				_vm->displayTextLines("c04r", 246, 200, 20, 2);
				_r4_flags |= 0x01;
				_vm->moveObjectToRoom(kObjectIdInventoryBusinessCards, 99);
				_vm->refreshInventory(true);
			}
		}
		break;
	}
}

int GameLogic::r4_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCastleDoor:
		_vm->playAnimation("opencg", 0, 4, 223, 49, 0, 100);
		actionTextIndex = 3;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r4_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 20;
	switch (_vm->_selectedDialogChoice) {
	case 288:
		if (!(_r4_flags & 0x01)) {
			_vm->displayTextLines("c04r", 246, 200, 20, 2);
			_r4_flags |= 0x01;
			_vm->moveObjectToRoom(kObjectIdInventoryBusinessCards, 99);
			_vm->refreshInventory(true);
		}
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
		_vm->_gameState = 0;
		break;
	case 285:
		replyTextIndex1 = 242;
		replyTextIndex2 = 243;
		break;
	case 286: case 287:
		replyTextIndex1 = 244;
		replyTextIndex2 = 245;
		break;
	}
	return continueDialog;
}

void GameLogic::r4_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "bktalk", 2);
	_vm->loadAnimationSpriteRange(2, "bkarm", 2);
	_vm->startRoomAnimations();
}

void GameLogic::r4_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r4_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 1000;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 1000;
	}
	if (_vm->_animationsCtr % 350 == 0 && _vm->getRandom(10) < 3) {
		_vm->drawAnimationSprite(1, 8, 48);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr % 400 == 0 && _vm->getRandom(10) < 8) {
		_vm->drawAnimationSprite(0, 8, 48);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr == 100 && _vm->getRandom(10) < 1) {
		_vm->drawAnimationSprite(3, 11, 67);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr == 150 && _vm->getRandom(10) < 2) {
		_vm->drawAnimationSprite(2, 11, 67);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r4_refreshRoomBackground() {
	char chainCheeseIn = 'n', chainUpDown1 = 'u', chainUpDown2 = 'u';
	bool chainVisible = false;
	if (!_vm->_hasRoomAnimationCallback) {
		r4_initRoomAnimations();
	}
	if (_r4_flags & 0x02) {
		_vm->drawRoomImageToBackground("opengate", 223, 49);
	}
	if (_r4_flags & 0x04) {
		_vm->drawRoomImageToBackground("cheesein", 226, 61);
		chainCheeseIn = 'y';
	}
	if (_r4_flags & 0x08) {
		chainUpDown1 = 'c';
		chainVisible = true;
	}
	if (_r4_flags & 0x10) {
		chainUpDown2 = 'c';
		chainVisible = true;
	}
	if (chainVisible) {
		Common::String filename = Common::String::format("%c%cchain%c", chainCheeseIn, chainUpDown1, chainUpDown2);
		_vm->drawRoomImageToBackground(filename.c_str(), 224, 57);
	}
	if (_r4_flags & 0x20) {
		_vm->drawRoomImageToBackground("wcrouch", 163, 53);
	} else if (_r4_flags & 0x40) {
		_vm->drawRoomImageToBackground("gcrouch", 163, 53);
	}
}

void GameLogic::r4_handleRoomEvent() {
	_vm->changeRoom(_vm->_currentRoomNumber);
	_vm->displayTextLines("c04r", 248, 200, 20, 3);
	_vm->displayTextLines("c04", 289, -1, -1, 1);
	_vm->setGameFlag(1);
}

int GameLogic::r4_useDrumstickWithCastleDoor() {
	_r4_flags |= 0x02;
	_vm->moveObjectToNowhere(kObjectIdCastleDoor);
	_vm->moveObjectToNowhere(kObjectIdInventoryDrumstick);
	_vm->moveObjectToRoom(kObjectIdDrumstick_1, 4);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	return 57;
}

int GameLogic::r4_useCheesePizzaWithCastle() {
	_r4_flags |= 0x04;
	_vm->moveObjectToNowhere(kObjectIdInventoryCheesePizza);
	_vm->moveObjectToRoom(kObjectIdCheese, 4);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	return 58;
}

void GameLogic::r4_useChainWithObject(bool arg6, bool arg8) {
	if (arg6) {
		_r4_flags |= 0x08;
	}
	if (arg8) {
		_r4_flags |= 0x10;
	}
	_vm->moveObjectToNowhere(kObjectIdInventoryChain);
	_vm->moveObjectToRoom(kObjectIdChain_1, 4);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

int GameLogic::r4_useTubeWithShade() {
	int oldX;
	if (_vm->getObjectRoom(kObjectIdInventoryJawbreakers) != 99) {
		return 59;
	}
	if (!(_r4_flags & 0x04) || !(_r4_flags & 0x08) || !(_r4_flags & 0x10) || !(_r4_flags & 0x02)) {
		return 61;
	}
	_vm->walkTo(162, 46, 1, -1, -1);
	if (_vm->_currentActorNum != 0) {
		_r4_flags |= 0x20;
		oldX = _vm->_wayneSpriteX;
		_vm->_wayneSpriteX = -1;
	} else {
		_r4_flags |= 0x40;
		oldX = _vm->_garthSpriteX;
		_vm->_garthSpriteX = -1;
	}
	_vm->displayTextLines("c03", 62, -1, -1, 1);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->waitSeconds(2);
	_r4_flags &= ~0x20;
	_r4_flags &= ~0x40;
	_vm->playAnimation("mouse", 0, 7, 212, 61, 0, 100);
	_vm->playAnimation("shoot", 0, 14, 187, 29, 0, 100);
	if (_vm->_currentActorNum != 0) {
		_vm->_wayneSpriteX = oldX;
	} else {
		_vm->_garthSpriteX = oldX;
	}
	_vm->moveObjectToNowhere(kObjectIdInventoryJawbreakers);
	_vm->moveObjectToNowhere(kObjectIdChain_1);
	_vm->moveObjectToNowhere(kObjectIdShade);
	_vm->moveObjectToNowhere(kObjectIdDrumstick_1);
	_vm->moveObjectToNowhere(kObjectIdCheese);
	_vm->moveObjectToRoom(kObjectIdCastleDoor, 4);
	_vm->moveObjectToRoom(kObjectIdDrumstick_0, 4);
	_vm->moveObjectToRoom(kObjectIdChain_0, 4);
	_r4_flags &= ~0x08;
	_r4_flags &= ~0x10;
	_r4_flags &= ~0x02;
	_r4_flags &= ~0x04;
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->_roomEventNum = 1;
	return 60;
}

int GameLogic::r5_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdFan:
		actionTextIndex = 51;
		break;
	case kObjectIdMirror5:
		actionTextIndex = 13;
		break;
	case kObjectIdExit5:
		actionTextIndex = 14;
		break;
	case kObjectIdDresser:
		actionTextIndex = 52;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r5_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit5:
		_vm->_gameState = 1;
		break;
	case kObjectIdBed:
		actionTextIndex = 8;
		break;
	case kObjectIdMirror5:
		actionTextIndex = 9;
		break;
	case kObjectIdFan:
		actionTextIndex = 10;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r5_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdBed:
		actionTextIndex = 2;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r5_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDresser:
		actionTextIndex = 6;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r5_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 170:
		replyTextIndex1 = 88;
		continueDialog = true;
		break;
	case 172: case 173:
		replyTextIndex1 = _vm->_selectedDialogChoice - 81;
		continueDialog = true;
		break;
	case 171:
		replyTextIndex1 = 89;
		replyTextIndex2 = 90;
		_vm->setDialogChoices(175, 176, 177, 178, -1);
		continueDialog = true;
		break;
	case 174:
		_vm->waitSeconds(1);
		_r5_flags &= ~0x01;
		_vm->_gameState = 1;
		break;
	case 177:
		_vm->moveObjectToRoom(kObjectIdInventoryCassContract, 99);
		_vm->refreshInventory(false);
		replyTextIndex1 = 95;
		_vm->_gameState = 1;
		break;
	case 175: case 176: case 178:
		replyTextIndex1 = _vm->_selectedDialogChoice - 82;
		_r5_flags &= ~0x01;
		_vm->_gameState = 1;
		break;
	}
	return continueDialog;
}

void GameLogic::r5_refreshRoomBackground() {
	if (!(_r5_flags & 0x01)) {
		_vm->_roomEventNum = 1;
	}
}

void GameLogic::r5_handleRoomEvent() {
	_r5_flags |= 0x01;
	_vm->walkTo(195, 102, -1, 209, 96);
	// TODO sub_185C0("wcass2", 0, 4, 0, 104, 170, 104, 26, 46, 8, 1);
	// TODO sub_185C0("scass", 2, 1, 170, 104, 171, 104, 15, 46, 2, 0);
	_vm->displayTextLines("c04r", 87, 150, 30, 1);
	_vm->setDialogChoices(170, 171, 172, 173, 174);
	_vm->startDialog();
}

int GameLogic::r6_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDog:
		actionTextIndex = 53;
		break;
	case kObjectIdWindow6:
		actionTextIndex = 56;
		break;
	case kObjectIdFrontDoor6:
		actionTextIndex = 56;
		break;
	case kObjectIdFlowerBed6_0:
		actionTextIndex = 29;
		break;
	case kObjectIdFlowerBed6_1:
		actionTextIndex = 29;
		break;
	case kObjectIdRope_0:
		_vm->pickupObject(kObjectIdRope_0, _r6_flags, 2, kObjectIdInventoryRope);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r6_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDog:
		actionTextIndex = 67;
		break;
	case kObjectIdWindow6:
		actionTextIndex = 71;
		break;
	case kObjectIdMirthmobile:
		r6_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	case kObjectIdBeam:
		if (_vm->_firstObjectNumber == kObjectIdInventoryRope) {
			r6_useRopeWithBeam();
		} else {
			actionTextIndex = 7;
		}
		break;
	case kObjectIdRope_1:
		if (_vm->_firstObjectNumber == kObjectIdInventoryHockeyStick) {
			r6_useHockeyStickWithRope();
		} else {
			actionTextIndex = 52;
		}
		break;
	case kObjectIdSeat:
		if (_vm->_currentActorNum != 0) {
			actionTextIndex = 53;
		} else {
			r6_useSeat();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r6_handleVerbPull() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdRope_1:
		if (!(_r6_flags & 0x10)) {
			actionTextIndex = 3;
		} else if (_vm->_currentActorNum == 0) {
			actionTextIndex = 4;
		} else {
			r6_pullRope();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r6_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDog:
		if (_vm->_firstObjectNumber == kObjectIdInventoryLuckySausage) {
			r6_giveLuckySausageToDog();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r6_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdFlowerBed6_0:
		actionTextIndex = 10;
		break;
	case kObjectIdFlowerBed6_1:
		actionTextIndex = 11;
		break;
	case kObjectIdGarage:
		actionTextIndex = 9;
		break;
	case kObjectIdDog:
		actionTextIndex = 7;
		break;
	case kObjectIdFrontDoor6:
		actionTextIndex = 2;
		_vm->_roomEventNum = 3;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r6_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	if (_vm->_selectedDialogChoice < 215) {
		replyTextX = 160;
		replyTextY = 10;
	} else {
		replyTextX = 100;
		replyTextY = 60;
	}
	switch (_vm->_selectedDialogChoice) {
	case 202:
		_vm->setDialogChoices(205, 206, 207, -1, -1);
		continueDialog = true;
		replyTextIndex1 = 139;
		replyTextIndex2 = 140;
		break;
	case 203:
		_vm->setDialogChoices(211, 212, 213, 214, -1);
		continueDialog = true;
		replyTextIndex1 = 141;
		break;
	case 204:
		replyTextIndex1 = 142;
		continueDialog = true;
		break;
	case 205: case 206: case 207:
		replyTextIndex1 = _vm->_selectedDialogChoice + -62;
		_vm->_roomEventNum = 1;
		break;
	case 211: case 212: case 213: case 214:
		replyTextIndex1 = 146;
		_vm->_gameState = 0;
		_vm->_roomEventNum = 2;
		break;
	case 218: case 219: case 220:
		replyTextIndex1 = 155;
		replyTextIndex2 = 156;
		replyTextIndex3 = 157;
		_vm->_roomEventNum = 4;
		_vm->_gameState = 0;
		break;
	case 221:
		replyTextIndex1 = 158;
		continueDialog = true;
		break;
	case 222:
		_vm->_roomEventNum = 4;
		_vm->_gameState = 0;
		break;
	case 224: case 226:
		_vm->displayTextLines("c04r", 160, 100, 60, 1);
		replyTextIndex1 = 161;
		replyTextIndex2 = 162;
		replyTextIndex3 = 163;
		_vm->_gameState = 0;
		_vm->_roomEventNum = 4;
		break;
	case 223:
		replyTextIndex1 = 164;
		continueDialog = true;
		break;
	case 225:
		replyTextIndex1 = 165;
		continueDialog = true;
		break;
	}
	return continueDialog;
}

void GameLogic::r6_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "sitdog", 3);
	_vm->startRoomAnimations();
}

void GameLogic::r6_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r6_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 1200;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 200) % 1200;
	}
	if (!(_r6_flags & 0x01) && (_vm->_animationsCtr % 400 == 0 || doUpdate)) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 400, 49, 111);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r6_refreshRoomBackground() {
	if (!(_r6_flags & 0x01)) {
		_vm->drawRoomImageToBackground("sitdog0", 49, 111);
		_vm->fillRoomMaskArea(65, 110, 106, 124, 0);
		_vm->fillRoomMaskArea(96, 111, 125, 123, 1);
	}
	if (_r6_flags & 0x02) {
		_vm->drawRoomImageToBackground("norope", 49, 111);
	}
	if (_r6_flags & 0x04) {
		_vm->drawRoomImageToBackground("throw5", 142, 37);
	}
	if (_r6_flags & 0x08) {
		_vm->drawRoomImageToBackground("seat", 148, 84);
	}
	if (_r6_flags & 0x10) {
		_vm->drawRoomImageToBackground("sitgarth", 151, 83);
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r6_initRoomAnimations();
	}
}

void GameLogic::r6_handleRoomEvent1() {
	_vm->displayTextLines("c04", 208, -1, -1, 3);
	for (int i = 0; i < 4; i++) {
		_vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
	}
	_vm->displayTextLines("c04r", 141, 160, 10, 1);
	_vm->setDialogChoices(211, 212, 213, 214, -1);
	_vm->startDialog();
}

void GameLogic::r6_handleRoomEvent2() {
	_vm->displayTextLines("c04", 215, -1, -1, 1);
	for (int i = 0; i < 4; i++) {
	  _vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
	}
	_vm->displayTextLines("c04r", 147, 160, 10, 1);
	_vm->displayTextLines("c04", 216, -1, -1, 1);
	for (int i = 0; i < 10; i++) {
		_vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
	}
	_vm->displayTextLines("c04r", 148, 160, 10, 4);
	_vm->displayTextLines("c04", 217, -1, -1, 1);
	for (int i = 0; i < 7; i++) {
		_vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
	}
	_vm->displayTextLines("c04r", 152, 160, 10, 2);
	_vm->playAnimation("cindi", 4, -5, 143, 55, 0, 100);
	_vm->playAnimation("raiseg", 12, -13, 133, 46, 0, 150);
	_r6_flags &= ~0x10;
	_vm->_garthSpriteX = 155;
	_vm->_wayneSpriteX = 150;
	_vm->_currentActorNum = 1;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r6_flags &= ~0x08;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r6_flags &= ~0x04;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->moveObjectToNowhere(kObjectIdRope_1);
	_vm->moveObjectToRoom(kObjectIdInventoryRope, 99);
	_vm->moveObjectToRoom(kObjectIdInventoryHockeyStick, 99);
	_vm->refreshInventory(true);
}

void GameLogic::r6_handleRoomEvent3() {
	_vm->waitSeconds(1);
	_vm->playSound("ss10", 1);
	_vm->walkTo(110, 107, 4, 135, 107);
	_vm->playAnimation("dad", 0, 3, 119, 79, 0, 100);
	if ((_r6_flags & 0x40) && !(_r6_flags & 0x20)) {
		_vm->displayTextLines("c04r", 159, 100, 60, 1);
		r6_handleRoomEvent4();
	} else {
		if (_r6_flags & 0x20) {
			_vm->displayTextLines("c04r", 166, 100, 60, 1);
			_vm->setDialogChoices(226, 223, 224, 225, 222);
		} else {
			_r6_flags |= 0x40;
			_vm->displayTextLines("c04r", 154, 100, 60, 1);
			_vm->setDialogChoices(218, 219, 220, 222, -1);
		}
		_vm->startDialog();
	}
}

void GameLogic::r6_handleRoomEvent4() {
	_vm->playAnimation("dad", 3, 2, 119, 79, 0, 100);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r6_giveLuckySausageToDog() {
	_r6_flags |= 0x01;
	_vm->drawActorReachObject(kObjectIdDog, 0);
	_vm->playAnimation("wdog", 0, 9, 64, 111, 0, 150);
	_vm->moveObjectToNowhere(kObjectIdDog);
	_vm->moveObjectToRoom(kObjectIdRope_0, 6);
	_vm->moveObjectToNowhere(kObjectIdInventoryLuckySausage);
	_vm->refreshInventory(true);
	_vm->fillRoomMaskArea(65, 110, 106, 124, 0);
	_vm->fillRoomMaskArea(96, 111, 125, 123, 1);
}

void GameLogic::r6_useRopeWithBeam() {
	if (_vm->_currentActorNum != 0) {
		_vm->walkTo(_vm->_wayneSpriteX, _vm->_wayneSpriteY, _vm->_actorSpriteValue, 165, 99);
	} else {
		_vm->walkTo(165, 99, _vm->_actorSpriteValue, _vm->_garthSpriteX, _vm->_garthSpriteY);
	}
	_r6_flags |= 0x04;
	_vm->playAnimation("throw", 0, 6, 142, 37, 0, 100);
	_vm->moveObjectToRoom(kObjectIdRope_1, 6);
	_vm->moveObjectToNowhere(kObjectIdInventoryRope);
	_vm->refreshInventory(true);
}

void GameLogic::r6_useHockeyStickWithRope() {
	_r6_flags |= 0x08;
	_vm->moveObjectToRoom(kObjectIdSeat, 6);
	_vm->moveObjectToNowhere(kObjectIdRope_1);
	_vm->moveObjectToNowhere(kObjectIdInventoryHockeyStick);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r6_useSeat() {
	_r6_flags |= 0x10;
	_vm->_garthSpriteX = -1;
	_vm->moveObjectToNowhere(kObjectIdSeat);
	_vm->moveObjectToRoom(kObjectIdRope_1, 6);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r6_pullRope() {
	_vm->playAnimation("raiseg", 0, 13, 133, 46, 0, 150);
	_vm->_wayneSpriteX = -1;
	for (int i = 0; i < 5; i++) {
		_vm->playAnimation("knock", 0, 2, 148, 61, 0, 100);
	}
	_vm->waitSeconds(1);
	_vm->_currentActorNum = 0;
	_vm->playAnimation("cindi", 0, 5, 143, 55, 0, 100);
	_vm->displayText("c04r", 138, 0, 160, 10, 0);
	for (int i = 0; i < 5; i++) {
		_vm->playAnimation("cindi", 4, 2, 143, 55, 0, 100);
	}
	_vm->waitSeconds(2);
	_vm->setDialogChoices(202, 203, 204, -1, -1);
	_vm->startDialog();
}

void GameLogic::r7_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdReceptionist:
		if (_pizzathonListFlags2 & 0x08) {
			_vm->displayTextLines("c04r2", 43, 250, 20, 1);
		} else if (_vm->getObjectRoom(kObjectIdInventoryPotatoChip) == -2) {
			_vm->setDialogChoices(54, 55, -1, -1, -1);
		} else {
			_vm->setDialogChoices(54, 56, -1, -1, -1);
		}
		break;
	case kObjectIdProtesters7:
		_vm->displayTextLines("c04r", 258, 250, 20, 2);
		break;
	}
}

int GameLogic::r7_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdFrontDoor7:
		r7_openCloseFrontDoor(true);
		r7_uninitRoomAnimations();
		_vm->setWaynePosition(16, 110);
		_vm->setGarthPosition(0, 110);
		_vm->changeRoom(15);
		break;
	case kObjectIdWindow7_0:
		r7_uninitRoomAnimations();
		_vm->setWaynePosition(206, 85);
		_vm->setGarthPosition(227, 82);
		_vm->changeRoom(16);
		break;
	case kObjectIdCar7:
		r7_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	case kObjectIdFrontDoor15:
		_vm->setWaynePosition(155, 105);
		_vm->setGarthPosition(142, 105);
		_vm->changeRoom(7);
		r7_openCloseFrontDoor(false);
		break;
	case kObjectIdWindow16:
		_vm->setWaynePosition(89, 103);
		_vm->setGarthPosition(100, 103);
		_vm->changeRoom(7);
		break;
	case kObjectIdProtesters7:
		actionTextIndex = 70;
		break;
	case kObjectIdDoor15_1:
		if ((_r30_flags & 0x01) && !(_r30_flags & 0x08) && _vm->_roomChangeCtr < 21) {
			actionTextIndex = 11;
		} else {
			_vm->setWaynePosition(230, 140);
			_vm->setGarthPosition(207, 143);
			_vm->changeRoom(30);
		}
		break;
	case kObjectIdWindow7_1:
	case kObjectIdWindow7_2:
	case kObjectIdDoor15_0:
	case kObjectIdDoor16:
		actionTextIndex = 11;
		break;
	case kObjectIdTV:
		actionTextIndex = 12;
		break;
	case kObjectIdMagazines15:
		actionTextIndex = 13;
		break;
	case kObjectIdMagiciansCape:
	case kObjectIdTopHat:
		actionTextIndex = 3;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r7_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdFrontDoor7:
		r7_openCloseFrontDoor(true);
		r7_uninitRoomAnimations();
		_vm->setWaynePosition(16, 110);
		_vm->setGarthPosition(0, 110);
		_vm->changeRoom(15);
		break;
	case kObjectIdWindow7_0:
		r7_uninitRoomAnimations();
		_vm->setWaynePosition(206, 85);
		_vm->setGarthPosition(227, 82);
		_vm->changeRoom(16);
		break;
	case kObjectIdFrontDoor15:
		_vm->setWaynePosition(155, 105);
		_vm->setGarthPosition(142, 105);
		_vm->changeRoom(7);
		r7_openCloseFrontDoor(false);
		break;
	case kObjectIdWindow16:
		_vm->setWaynePosition(89, 103);
		_vm->setGarthPosition(100, 103);
		_vm->changeRoom(7);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r7_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar7:
		actionTextIndex = 7;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r7_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMagiciansCape:
		_vm->pickupObject(kObjectIdMagiciansCape, _r7_flags, 1, kObjectIdInventoryCape);
		actionTextIndex = 3;
		break;
	case kObjectIdTopHat:
		_vm->pickupObject(kObjectIdTopHat, _r7_flags, 2, kObjectIdInventoryTopHat);
		actionTextIndex = 4;
		break;
	case kObjectIdBillboard7:
	case kObjectIdBillboard_0:
	case kObjectIdSatelliteDish7:
		actionTextIndex = 16;
		break;
	case kObjectIdWindow7_1:
	case kObjectIdWindow7_2:
	case kObjectIdFrontDoor15:
		actionTextIndex = 7;
		break;
	case kObjectIdWindow7_0:
	case kObjectIdFrontDesk15:
	case kObjectIdWindow16:
		actionTextIndex = 14;
		break;
	case kObjectIdProtesters7:
		actionTextIndex = 55;
		break;
	case kObjectIdReceptionist:
	case kObjectIdDoor15_1:
	case kObjectIdDoor16:
		actionTextIndex = 17;
		break;
	case kObjectIdFrontDoor7:
	case kObjectIdDoor15_0:
		actionTextIndex = 8;
		break;
	case kObjectIdTrophies:
		actionTextIndex = 19;
		break;
	case kObjectIdTV:
		actionTextIndex = 20;
		break;
	case kObjectIdPictures15:
		actionTextIndex = 21;
		break;
	case kObjectIdSign15_1:
		actionTextIndex = 22;
		break;
	case kObjectIdMagazines15:
		actionTextIndex = 23;
		break;
	case kObjectIdSign15_0:
		actionTextIndex = 24;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r7_initRoomAnimations() {
	_r7_cloudsPositionX = _vm->getRandom(230);
	_vm->loadAnimationSprite(0, "clouds");
	_vm->loadAnimationSprite(1, "bldg");
	_vm->loadAnimationSprite(2, "sky");
	_vm->loadAnimationSprite(3, "elvis0");
	_vm->loadAnimationSprite(4, "elvis1");
	_vm->startRoomAnimations();
}

void GameLogic::r7_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r7_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 4000;
	if (_vm->_animationsCtr % 500 == 0) {
		_r7_cloudsPositionX = (_r7_cloudsPositionX + 1) % 230;
		_vm->drawAnimationSprite(2, 0, 0);
		_vm->drawAnimationSpriteTransparent(0, _r7_cloudsPositionX - 125, 13);
		_vm->drawAnimationSpriteTransparent(1, 0, 13);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
	if (_vm->_animationsCtr % 2000 == 0) {
		_vm->drawAnimationSprite(3 + (_vm->_animationsCtr / 2000), 197, 68);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r7_refreshRoomBackground() {
	if (_pizzathonListFlags1 & 0x08) {
		_vm->drawRoomImageToBackground("bboard", 68, 39);
		_vm->moveObjectToRoom(kObjectIdBillboard_0, 7);
		_vm->moveObjectToNowhere(kObjectIdBillboard7);
	}
	if (_r7_flags & 0x04) {
		_vm->drawRoomImageToBackground("opendoor", 155, 73);
	}
	if (!(_pizzathonListFlags2 & 0x08)) {
		_vm->drawRoomImageToBackground("elvis0", 197, 68);
		_vm->moveObjectToRoom(kObjectIdProtesters7, 7);
		if (!_vm->_hasRoomAnimationCallback) {
			r7_initRoomAnimations();
		}
	} else {
		_vm->moveObjectToNowhere(kObjectIdProtesters7);
	}
}

void GameLogic::r7_openCloseFrontDoor(bool isOpen) {
	if (isOpen) {
		_vm->drawActorReachObject(kObjectIdFrontDoor7, 0);
		_r7_flags |= 0x04;
		_vm->waitMillis(500);
	} else {
		_vm->waitMillis(500);
		_r7_flags &= ~0x04;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

int GameLogic::r8_handleVerbClose() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDoor8:
		if (_r8_flags & 0x01) {
			actionTextIndex = 1;
		} else {
			r8_openCloseDoor(false);
		}
		break;
	case kObjectIdCabinet22_0:
		if (!(_r8_flags & 0x02)) {
			actionTextIndex = 1;
		} else {
			r8_openCloseCabinetDoor1(false);
		}
		break;
	case kObjectIdCabinet22_1:
		if (!(_r8_flags & 0x04)) {
			actionTextIndex = 1;
		} else {
			r8_openCloseCabinetDoor2(false);
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r8_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDoor8:
		if (!(_r8_flags & 0x01)) {
			actionTextIndex = 1;
		} else {
			r8_openCloseDoor(true);
		}
		break;
	case kObjectIdCabinet22_0:
		if (_r8_flags & 0x02) {
			actionTextIndex = 1;
		} else {
			r8_openCloseCabinetDoor1(true);
		}
		break;
	case kObjectIdCabinet22_1:
		if (_r8_flags & 0x04) {
			actionTextIndex = 1;
		} else {
			r8_openCloseCabinetDoor2(true);
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r8_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDoor8:
		if (_r8_flags & 0x01) {
			r8_openCloseDoor(true);
		}
		_vm->stopRoomAnimations();
		_r8_flags &= ~0x08;
		_vm->_gameState = 1;
		break;
	case kObjectIdHallway8:
	case kObjectIdFoyer:
	case kObjectIdOffice21:
	case kObjectIdHallway22:
		actionTextIndex = -1;
		break;
	case kObjectIdChairs22:
		actionTextIndex = 1;
		break;
	case kObjectIdMirror21_0:
	case kObjectIdMirror21_1:
		actionTextIndex = 9;
		break;
	case kObjectIdCloset:
		actionTextIndex = 14;
		break;
	case kObjectIdStaircase:
		actionTextIndex = 15;
		break;
	case kObjectIdFountain8:
		actionTextIndex = 16;
		break;
	case kObjectIdDivan:
		actionTextIndex = 17;
		break;
	case kObjectIdTorch21_0:
	case kObjectIdTorch21_1:
		actionTextIndex = 18;
		break;
	case kObjectIdDesk22:
		actionTextIndex = 19;
		break;
	case kObjectIdFireplace:
	case kObjectIdWood:
		actionTextIndex = 20;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r8_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdStaircase:
	case kObjectIdFountain8:
	case kObjectIdOffice21:
	case kObjectIdHallway22:
		actionTextIndex = 7;
		break;
	case kObjectIdDoor8:
	case kObjectIdHallway8:
	case kObjectIdBookshelf:
		actionTextIndex = 8;
		break;
	case kObjectIdFireplace:
	case kObjectIdPictures22:
		actionTextIndex = 16;
		break;
	case kObjectIdCloset:
	case kObjectIdFoyer:
		actionTextIndex = 14;
		break;
	case kObjectIdBush:
		actionTextIndex = 25;
		break;
	case kObjectIdMirror21_0:
	case kObjectIdMirror21_1:
	case kObjectIdTorch21_0:
	case kObjectIdTorch21_1:
	case kObjectIdPictures21:
		actionTextIndex = 12;
		break;
	case kObjectIdWood:
		actionTextIndex = 26;
		break;
	case kObjectIdMap:
		_vm->pickupObject(kObjectIdMap, _r29_flags, 2, kObjectIdInventorySewerMap);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r8_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 9:
		replyTextIndex1 = 12;
		replyTextIndex2 = 13;
		replyTextIndex3 = 14;
		_r8_flags |= 0x10;
		_vm->_roomEventNum = 2;
		break;
	case 10:
		replyTextIndex1 = 15;
		_r8_flags |= 0x10;
		_vm->_roomEventNum = 2;
		break;
	case 11:
		_vm->waitSeconds(1);
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 473:
		_vm->displayTextLines("c04r2", 6, 200, 30, 2);
		_vm->displayTextLines("c04r2", 14, 200, 30, 1);
		_vm->playAnimation("spit", 0, 6, 179, 66, 0, 100);
		replyTextIndex1 = 552;
		replyTextIndex2 = 553;
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 474:
		_vm->displayTextLines("c04r2", 8, 200, 30, 2);
		_vm->displayTextLines("c04r2", 14, 200, 30, 1);
		_vm->playAnimation("spit", 0, 6, 179, 66, 0, 100);
		replyTextIndex1 = 552;
		replyTextIndex2 = 553;
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 475:
		_vm->displayTextLines("c04r2", 10, 200, 30, 2);
		_vm->displayTextLines("c04r2", 14, 200, 30, 1);
		_vm->playAnimation("spit", 0, 6, 179, 66, 0, 100);
		replyTextIndex1 = 552;
		replyTextIndex2 = 553;
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 476:
		_vm->displayTextLines("c04r2", 12, 200, 30, 3);
		_vm->playAnimation("spit", 0, 6, 179, 66, 0, 100);
		replyTextIndex1 = 552;
		replyTextIndex2 = 553;
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 478:
		_r8_flags &= ~0x08;
		_vm->stopRoomAnimations();
		_vm->_gameState = 1;
		break;
	case 479:
		replyTextIndex1 = 555;
		continueDialog = true;
		break;
	case 480:
		replyTextIndex1 = 556;
		continueDialog = true;
		break;
	case 477: case 481:
		_vm->_roomEventNum = 3;
		replyTextIndex1 = 557;
		break;
	case 482:
		replyTextIndex1 = 560;
		continueDialog = true;
		break;
	case 483:
		replyTextIndex1 = 561;
		continueDialog = true;
		break;
	case 484:
		replyTextIndex1 = 562;
		continueDialog = true;
		_vm->setDialogChoices(485, 486, 487, -1, -1);
		break;
	case 485: case 486: case 487:
		_vm->displayTextLines("c04r2", 26, 200, 30, 1);
		_vm->displayTextLines("c04", 488, -1, -1, 7);
		_vm->displayTextLines("c04r2", 27, 200, 30, 1);
		_vm->displayTextLines("c04", 495, -1, -1, 1);
		_vm->displayTextLines("c04r2", 28, 200, 30, 1);
		_vm->displayTextLines("c04", 496, -1, -1, 2);
		_vm->displayTextLines("c04r2", 29, 200, 30, 5);
		for (int i = 0; i < 15; i++) {
			_vm->moveObjectToRoom(kObjectIdInventoryDollar, 99);
		}
		_vm->refreshInventory(true);
		_vm->setDialogChoices(498, 499, 500, -1, -1);
		continueDialog = true;
		break;
	case 498:
		replyTextIndex1 = 571;
		continueDialog = true;
		_vm->_roomEventNum = 4;
		_vm->_gameState = 0;
		break;
	case 499:
		replyTextIndex1 = 572;
		replyTextIndex2 = 573;
		replyTextIndex3 = 574;
		continueDialog = true;
		_vm->_roomEventNum = 4;
		_vm->_gameState = 0;
		break;
	case 500:
		replyTextIndex1 = 575;
		continueDialog = true;
		_vm->_roomEventNum = 4;
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r8_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "fount", 9);
	_vm->startRoomAnimations();
}

void GameLogic::r8_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 4500;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 4500;
	}
	if (_vm->_animationsCtr % 500 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 500, 85, 38);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r8_refreshRoomBackground() {
	if (!(_r8_flags & 0x01)) {
		_vm->drawRoomImageToBackground("opendoor", 259, 51);
	}
	if (!(_r8_flags & 0x08) && ((_r8_flags & 0x80) || (_r38_flags & 0x08))) {
		_vm->_roomEventNum = 1;
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r8_initRoomAnimations();
	}
}

void GameLogic::r8_handleRoomEvent1() {
	_r8_flags |= 0x08;
	_vm->playAnimation("mwalk0", 0, 23, 0, 94, 0, 100);
	_vm->playAnimation("mwalk1", 0, 10, 188, 66, 0, 100);
	if (_r38_flags & 0x08) {
		_vm->displayTextLines("c04r", 349, 200, 30, 9);
		_r8_flags |= 0x20;
		// TODO _quitGame = true;
	} else {
		if (_pizzathonListFlags2 & 0x08) {
			if (_r8_flags & 0x40) {
				_vm->displayTextLines("c04r2", 17, 200, 30, 1);
				if (_vm->getObjectRoom(kObjectIdInventorySinusPotion) == 99) {
					_vm->setDialogChoices(478, 479, 480, 481, -1);
				} else {
					_vm->setDialogChoices(478, 479, 480, -1, -1);
				}
				_vm->startDialog();
			} else {
				_r8_flags |= 0x40;
				_vm->displayTextLines("c04r2", 0, 200, 30, 3);
				_vm->playAnimation("spit", 0, 6, 179, 66, 0, 100);
				_vm->displayTextLines("c04r2", 3, 200, 30, 3);
				if (_vm->getObjectRoom(kObjectIdInventorySinusPotion) == 99) {
					_vm->setDialogChoices(473, 474, 475, 476, 477);
				} else {
					_vm->setDialogChoices(473, 474, 475, 476, -1);
				}
				_vm->startDialog();
			}
		} else if (!(_r8_flags & 0x10)) {
			_vm->displayTextLines("c04r", 10, 200, 30, 2);
			_vm->setDialogChoices(9, 10, 11, -1, -1);
			_vm->startDialog();
		} else {
			_vm->displayTextLines("c04r", 19, 200, 30, 2);
			_vm->displayText("c04", 17, 0, -1, -1, 0);
			_vm->waitSeconds(2);
			_r8_flags &= ~0x08;
			_vm->stopRoomAnimations();
			_vm->_gameState = 1;
		}
	}
}

void GameLogic::r8_handleRoomEvent2() {
	_vm->displayTextLines("c04", 12, -1, -1, 4);
	_vm->displayTextLines("c04r", 16, 200, 30, 3);
	_vm->displayText("c04", 16, 0, -1, -1, 0);
	_vm->waitSeconds(1);
	_r8_flags &= ~0x08;
	_vm->stopRoomAnimations();
	_vm->_gameState = 1;
}

void GameLogic::r8_handleRoomEvent3() {
	_vm->moveObjectToNowhere(kObjectIdInventorySinusPotion);
	_vm->refreshInventory(true);
	_vm->playAnimation("drink", 0, 25, 188, 64, 0, 100);
	_vm->displayTextLines("c04r2", 21, 200, 30, 2);
	_vm->setDialogChoices(482, 483, 484, -1, -1);
	_vm->startDialog();
}

void GameLogic::r8_handleRoomEvent4() {
	_vm->displayTextLines("c04r2", 39, 200, 30, 3);
	_r8_flags |= 0x80;
	_vm->playAnimation("maygo", 0, 16, 178, 40, 0, 100);
}

void GameLogic::r8_openCloseDoor(bool isOpen) {
	_vm->drawActorReachObject(kObjectIdDoor8, 0);
	if (isOpen) {
		_r8_flags &= ~0x01;
	} else {
		_r8_flags |= 0x01;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r8_openCloseCabinetDoor1(bool isOpen) {
	_vm->drawActorReachObject(kObjectIdDoor8, 0);
	if (isOpen) {
		_r8_flags |= 0x02;
	} else {
		_r8_flags &= ~0x02;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r8_openCloseCabinetDoor2(bool isOpen) {
	_vm->drawActorReachObject(kObjectIdDoor8, 0);
	if (isOpen) {
		_r8_flags |= 0x04;
	} else {
		_r8_flags &= ~0x04;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

int GameLogic::r9_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdEugene9:
		actionTextIndex = 29;
		break;
	case kObjectIdMan9:
		actionTextIndex = 29;
		break;
	case kObjectIdExit9:
		actionTextIndex = 7;
		break;
	case kObjectIdCounter9:
	case kObjectIdDisplay9:
		actionTextIndex = 8;
		break;
	case kObjectIdHorrorSection:
		actionTextIndex = 58;
		break;
	case kObjectIdComedySection:
		actionTextIndex = 59;
		break;
	case kObjectIdNewReleases:
		actionTextIndex = 57;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r9_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdEugene9:
		actionTextIndex = 72;
		break;
	case kObjectIdMan9:
		actionTextIndex = 68;
		break;
	case kObjectIdHorrorSection:
		actionTextIndex = 73;
		break;
	case kObjectIdComedySection:
		actionTextIndex = 74;
		break;
	case kObjectIdExit9:
		r9_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r9_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdEugene9:
		if (_pizzathonListFlags2 & 0x08) {
			_vm->displayTextLines("c04r2", 43, 250, 25, 1);
		} else if (_vm->getObjectRoom(kObjectIdInventoryCassContract) == 99) {
			_r9_dialogFlag = 0;
			_vm->setDialogChoices(231, 232, 233, 240, 235);
		} else if (!(_pizzathonListFlags1 & 0x02)) {
			_r9_dialogFlag = 1;
			_vm->setDialogChoices(231, 232, 233, 234, 235);
		} else {
			_r9_dialogFlag = 2;
			_vm->setDialogChoices(243, 232, 233, 244, 235);
		}
		break;
	case kObjectIdMan9:
		if (!(_r9_flags & 0x02)) {
			_vm->displayTextLines("c04", 245, -1, -1, 1);
			_vm->displayTextLines("c04r", 192, 250, 25, 1);
			for (int i = 0; i < 8; i++) {
				_vm->playAnimation("utrick", 0, 2, 206, 58, 0, 100);
			}
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
			_vm->displayTextLines("c04r", 193, 250, 25, 1);
			_vm->setDialogChoices(246, 247, 248, 249, 250);
		} else {
			_vm->setDialogChoices(246, 253, 254, 249, 255);
		}
		break;
	}
}

int GameLogic::r9_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdEugene9:
		actionTextIndex = 5;
		break;
	case kObjectIdMan9:
		actionTextIndex = 6;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r9_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMan9:
		if (_vm->_currentActorNum != 0) {
			_vm->walkTo(_vm->_wayneSpriteX - 1, _vm->_wayneSpriteY, _vm->_actorSpriteValue, 173, 86);
		} else {
			_vm->walkTo(173, 86, _vm->_actorSpriteValue, _vm->_garthSpriteX - 1, _vm->_garthSpriteY);
		}
		if (_vm->_firstObjectNumber == kObjectIdInventoryCape || _vm->_firstObjectNumber == kObjectIdInventoryTopHat) {
			r9_giveCoatOrTopHatToMan();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r9_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	if (_vm->_selectedDialogChoice < 245) {
		replyTextX = 150;
		replyTextY = 15;
	} else {
		replyTextX = 250;
		replyTextY = 25;
	}
	switch (_vm->_selectedDialogChoice) {
	case 232: case 234:
		replyTextIndex1 = 178;
		continueDialog = true;
		break;
	case 231: case 240:
		if (!(_pizzathonListFlags1 & 0x01)) {
			_vm->setDialogChoices(236, 237, 238, 239, 187);
			replyTextIndex1 = 177;
			continueDialog = true;
		} else {
			_vm->displayTextLines("c04r", 184, 150, 15, 2);
			replyTextIndex1 = 186;
			replyTextIndex2 = 187;
			replyTextIndex3 = 188;
			_vm->setDialogChoices(241, 242, -1, -1, -1);
			continueDialog = true;
			_vm->moveObjectToRoom(kObjectIdInventoryContract, 99);
		}
		break;
	case 233:
		replyTextIndex1 = 179;
		continueDialog = true;
		break;
	case 187: case 235: case 250: case 255:
		_vm->_gameState = 0;
		break;
	case 236: case 237: case 238: case 239:
		replyTextIndex1 = _vm->_selectedDialogChoice + -56;
		if (_r9_dialogFlag == 0) {
			_vm->setDialogChoices(231, 232, 233, 234, 235);
		} else if (_r9_dialogFlag == 1) {
			_vm->setDialogChoices(231, 232, 233, 240, 235);
		} else {
			continueDialog = true;
		}
		break;
	case 241:
		replyTextIndex1 = 189;
		_vm->setGameFlag(6);
	case 242:
		_vm->_gameState = 0;
		break;
	case 243:
		replyTextIndex1 = 190;
		continueDialog = true;
		break;
	case 244:
		replyTextIndex1 = 191;
		continueDialog = true;
		break;
	case 246:
		if (!(_r9_flags & 0x02)) {
			replyTextIndex1 = 194;
		} else if (_r9_flags & 0x04) {
			replyTextIndex1 = 204;
		} else {
			_vm->setGameFlag(9);
			_r9_flags |= 0x04;
			replyTextIndex1 = 203;
		}
		continueDialog = true;
		break;
	case 247:
		replyTextIndex1 = 195;
		continueDialog = true;
		break;
	case 248:
		_vm->displayTextLines("c04r", 196, 250, 25, 1);
		_vm->displayTextLines("c04", 251, -1, -1, 1);
		_vm->_currentActorNum = (_vm->_currentActorNum + 1) % 2;
		_vm->displayTextLines("c04", 252, -1, -1, 1);
		_vm->_currentActorNum = (_vm->_currentActorNum + 1) % 2;
		replyTextIndex1 = 197;
		continueDialog = true;
		break;
	case 249:
		if (!(_r9_flags & 0x02)) {
			replyTextIndex1 = 198;
		} else {
			replyTextIndex1 = 210;
		}
		continueDialog = true;
		break;
	case 253:
		replyTextIndex1 = 205;
		replyTextIndex2 = 206;
		replyTextIndex3 = 207;
		continueDialog = true;
		break;
	case 254:
		replyTextIndex1 = 208;
		replyTextIndex2 = 209;
		continueDialog = true;
		break;
	}
	return continueDialog;
}

void GameLogic::r9_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "sign", 3);
	_vm->startRoomAnimations();
}

void GameLogic::r9_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r9_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 3000;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 200) % 3000;
	}
	if (_vm->_animationsCtr % 1000 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 1000, 124, 27);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r9_refreshRoomBackground() {
	if (!(_pizzathonListFlags1 & 0x01)) {
		_vm->drawRoomImageToBackground("man", 211, 59);
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r9_initRoomAnimations();
	}
}

void GameLogic::r9_giveCoatOrTopHatToMan() {
	_vm->moveObjectToNowhere(_vm->_firstObjectNumber);
	_vm->refreshInventory(true);
	if (!(_r9_flags & 0x01)) {
		_vm->displayTextLines("c04r", 199, 250, 25, 1);
		_r9_flags |= 0x01;
	} else {
		_vm->displayTextLines("c04r", 200, 250, 25, 2);
		_vm->playAnimation("strick", 0, 17, 191, 32, 0, 100);
		for (int i = 0; i < 5; i++) {
			_vm->playAnimation("strick", 16, -2, 191, 32, 0, 100);
		}
		_vm->playAnimation("strick", 14, -15, 191, 32, 0, 100);
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
		_vm->displayTextLines("c04r", 202, 250, 25, 1);
		_vm->setDialogChoices(246, 253, 254, 249, 255);
		_vm->startDialog();
		_r9_flags |= 0x02;
	}
}

int GameLogic::r10_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdTicket:
		r10_handleVerbPickUpTicket();
		break;
	case kObjectIdCandyBar:
		r10_handleVerbPickUpCandyBar();
		break;
	case kObjectIdGum:
		if (!(_r10_flags & 0x04)) {
			_vm->displayText("c04r", 6, 0, 320, 70, 0);
		} else {
			r10_handleVerbPickUpGum();
		}
		break;
	case kObjectIdJawbreakers:
		if (!(_r10_flags & 0x08)) {
			_vm->displayText("c04r", 7, 0, 320, 70, 0);
		} else {
			r10_handleVerbPickUpJawbreakers();
		}
		break;
	case kObjectIdNewspapers:
	case kObjectIdMagazines10:
		actionTextIndex = 15;
		break;
	case kObjectIdHotDogs:
		actionTextIndex = 53;
		break;
	case kObjectIdSign10:
		actionTextIndex = 16;
		break;
	case kObjectIdCigarettes:
		actionTextIndex = 54;
		break;
	case kObjectIdExit10:
		actionTextIndex = 8;
		break;
	case kObjectIdSalesgirl:
		if (_vm->_currentActorNum != 0) {
			actionTextIndex = 17;
		} else {
			actionTextIndex = 18;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r10_handleVerbPickUpCandyBar() {
	_r10_flags &= ~0x40;
	_vm->moveObjectToNowhere(kObjectIdCandyBar);
	_vm->moveObjectToRoom(kObjectIdInventoryCandyBar, 99);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r10_handleVerbPickUpTicket() {
	_r10_flags &= ~0x02;
	_vm->moveObjectToNowhere(kObjectIdTicket);
	_vm->moveObjectToRoom(kObjectIdInventoryUnusedTicket, 99);
	_vm->refreshInventory(false);
	_vm->lookAtUnusedTicket();
}

void GameLogic::r10_handleVerbPickUpJawbreakers() {
	_r10_flags |= 0x20;
	_r10_flags &= ~0x08;
	_vm->moveObjectToRoom(kObjectIdInventoryJawbreakers, 99);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r10_handleVerbPickUpGum() {
	_r10_flags |= 0x10;
	_r10_flags &= ~0x04;
	_vm->moveObjectToRoom(kObjectIdInventoryGum, 99);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r10_handleVerbGiveWinningTicketToSalesgirl() {
	_vm->redrawInventory();
	r10_refreshObject(2);
	_vm->waitSeconds(1);
	_vm->playAnimation("puttick", 3, -4, 98, 7, 0, 100);
	_vm->playAnimation("puttick", 0, 2, 98, 7, 0, 100);
	_vm->playAnimation("getmon", 3, -2, 98, 7, 0, 100);
	_r10_flags |= 0x40;
	_vm->moveObjectToRoom(kObjectIdCandyBar, 10);
	_vm->moveObjectToNowhere(kObjectIdInventoryWinningTicket);
	_vm->refreshInventory(false);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->displayText("c04r", 9, 0, 320, 70, 0);
}

void GameLogic::r10_buyItem() {
	r10_refreshObject(1);
	_vm->waitSeconds(1);
	if (_r10_selectedItemToBuy == 0) {
		_vm->playAnimation("getmon", 0, 5, 98, 7, 0, 100);
		_r10_flags |= 0x02;
		_vm->moveObjectToRoom(kObjectIdTicket, 10);
		_vm->playAnimation("puttick", 0, 4, 98, 7, 0, 100);
	} else {
		_vm->playAnimation("getmon", 0, 3, 98, 7, 0, 100);
		_vm->displayText("c04r", 8, 0, 320, 115, 0);
		_vm->playAnimation("gest", 0, 7, 127, 7, 0, 200);
		_vm->playAnimation("getmon", 4, 2, 98, 7, 0, 100);
		if (_r10_selectedItemToBuy == 1) {
			_r10_flags |= 0x08;
		} else {
			_r10_flags |= 0x04;
		}
	}
	_vm->moveObjectToNowhere(kObjectIdInventoryDollar);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r10_selectedItemToBuy = -1;
}

int GameLogic::r10_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit10:
		_vm->_gameState = 1;
		break;
	case kObjectIdGum:
	case kObjectIdJawbreakers:
	case kObjectIdTicket:
	case kObjectIdCandyBar:
		actionTextIndex = 3;
		break;
	case kObjectIdSalesgirl:
		if (_vm->_currentActorNum != 0) {
			actionTextIndex = 21;
		} else {
			actionTextIndex = 22;
		}
		break;
	case kObjectIdNewspapers:
	case kObjectIdMagazines10:
		actionTextIndex = 23;
		break;
	case kObjectIdSign10:
		actionTextIndex = 69;
		break;
	case kObjectIdCigarettes:
		actionTextIndex = 68;
		break;
	case kObjectIdHotDogs:
		actionTextIndex = 67;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r10_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdSalesgirl:
		if (_vm->_currentActorNum != 0) {
			_vm->setDialogChoices(0, 5, 2, 3, 4);
		} else {
			_vm->setDialogChoices(0, 1, 2, 3, 4);
		}
		break;
	}
}

int GameLogic::r10_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCigarettes:
		actionTextIndex = 3;
		break;
	case kObjectIdSign10:
		actionTextIndex = 4;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r10_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdSalesgirl:
		if (_vm->_firstObjectNumber == kObjectIdInventoryWinningTicket) {
			r10_handleVerbGiveWinningTicketToSalesgirl();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r10_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdHotDogs:
		actionTextIndex = 7;
		break;
	case kObjectIdSign10:
		actionTextIndex = 8;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r10_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 320;
	replyTextY = 70;
	if (_vm->_objectNumber == kObjectIdSalesgirl) {
		if (_r10_flags & 0x02) {
			replyTextIndex1 = 5;
			_vm->_gameState = 0;
			continueDialog = false;
		} else {
			switch (_vm->_selectedDialogChoice) {
			case 3:
				replyTextIndex1 = 0;
				break;
			case 5:
				replyTextIndex1 = 1;
				replyTextIndex2 = 2;
				break;
			case 0: case 1: case 2:
				_r10_selectedItemToBuy = _vm->_selectedDialogChoice;
				if (_vm->_currentActorNum != 0 && _vm->_wayneInventory[kObjectIdInventoryDollar - 28] == 0) {
					_vm->setDialogChoices(8, -1, -1, -1, -1);
				} else if (_vm->_currentActorNum == 0 && _vm->_garthInventory[kObjectIdInventoryDollar - 28] == 0) {
					_vm->setDialogChoices(8, -1, -1, -1, -1);
				} else {
					_vm->setDialogChoices(6, 7, -1, -1, -1);
				}
				replyTextIndex1 = 3;
				continueDialog = true;
				break;
			case 6:
				r10_buyItem();
			case 4:
				_vm->_gameState = 0;
				break;
			case 8:
				replyTextIndex1 = 4;
			case 7:
				_r10_selectedItemToBuy = -1;
				handleVerbTalkTo();
				break;
			}
		}
	}
	return continueDialog;
}

void GameLogic::r10_refreshRoomBackground() {
	if (_r10_flags & 0x02) {
		_vm->drawRoomImageToBackground("cticket", 147, 100);
	}
	if (_r10_flags & 0x01) {
		_vm->drawRoomImageToBackground("money", 136, 100);
	}
	if (_r10_flags & 0x40) {
		_vm->drawRoomImageToBackground("candybar", 135, 100);
	}
	if (_r10_flags & 0x10) {
		_vm->drawRoomImageToBackground("nogum", 248, 88);
	}
	if (_r10_flags & 0x20) {
		_vm->drawRoomImageToBackground("nojawb", 271, 88);
	}
}

void GameLogic::r10_refreshObject(int arg4) {
	_r10_flags = _r10_flags + arg4;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r10_flags &= (255 - arg4);
}

int GameLogic::r11_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLampPosts11_0:
	case kObjectIdFountain11_1:
		actionTextIndex = 7;
		break;
	case kObjectIdLampPosts11_1:
		actionTextIndex = 8;
		break;
	case kObjectIdCar11:
	case kObjectIdManhole:
		actionTextIndex = 14;
		break;
	case kObjectIdLampPost:
		actionTextIndex = 30;
		break;
	case kObjectIdOutlet11:
		actionTextIndex = 16;
		break;
	case kObjectIdEntrance11:
	case kObjectIdFountain11_0:
		actionTextIndex = 33;
		break;
	case kObjectIdCityHall11:
		actionTextIndex = 34;
		break;
	case kObjectIdManholeCover:
		_vm->moveObjectToNowhere(kObjectIdManholeCover);
		_vm->moveObjectToRoom(kObjectIdManhole, 11);
		actionTextIndex = 37;
		break;
	case kObjectIdExtensionCord_1:
		if (_r11_flags & 0x08) {
			if (_vm->_currentActorNum != 0) {
				actionTextIndex = 38;
			} else {
				actionTextIndex = 39;
			}
		} else {
			r11_pickUpExtensionCord();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r11_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar11:
		if (_r11_flags & 0x01) {
			r11_useCar1();
		}
		if (_r11_flags & 0x04) {
			r11_useCar2();
		}
		r11_uninitRoomAnimations();
		_vm->_gameState = 1;
		break;
	case kObjectIdManhole:
		r11_uninitRoomAnimations();
		_vm->_musicIndex = 3;
		_vm->changeMusic();
		_vm->changeRoom(24);
		break;
	case kObjectIdLampPost:
		if (_vm->_firstObjectNumber == kObjectIdInventoryPlungers) {
			if (_vm->_currentActorNum != 0) {
				actionTextIndex = 43;
			} else if (_r11_flags & 0x10) {
				actionTextIndex = 80;
			} else if (_r11_flags & 0x01) {
				r11_useCar1();
			} else {
				r11_usePlungersWithLampPost();
			}
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdOutlet11:
		if (!(_r11_flags & 0x01)) {
			actionTextIndex = 7;
		} else {
			if (_vm->_firstObjectNumber == kObjectIdInventoryExtensionCord) {
				if (_vm->_currentActorNum != 0) {
					actionTextIndex = 46;
				} else {
					r11_useExtensionCordWithOutlet();
				}
			} else {
				actionTextIndex = 0;
			}
		}
		break;
	case kObjectIdExtensionCord_1:
		if (_vm->_firstObjectNumber == kObjectIdInventorySuckCut) {
			if (_vm->_currentActorNum == 0) {
				actionTextIndex = 47;
			} else {
				r11_useSuckCutWithExtensionCord();
			}
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdFountain11_0:
	case kObjectIdFountain11_1:
		if (_vm->_firstObjectNumber == kObjectIdSuckCut) {
			if (_vm->_currentActorNum == 0) {
				actionTextIndex = 47;
			} else {
				r11_useSuckCutWithFountain();
			}
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdEntrance11:
		if (_r11_flags & 0x01) {
			r11_useCar1();
			_vm->walkToObject();
		}
		r11_uninitRoomAnimations();
		_vm->setWaynePosition(201, 125);
		_vm->setGarthPosition(226, 122);
		_vm->changeRoom(32);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r11_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar11:
		actionTextIndex = 7;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r11_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "water", 4);
	_vm->loadAnimationSprite(4, "suckcut");
	_vm->startRoomAnimations();
}

void GameLogic::r11_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r11_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 2000;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 2000;
	}
	if (_vm->_animationsCtr % 500 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 500, 45, 92);
		if (_r11_flags & 0x04) {
			_vm->drawAnimationSprite(4, 116, 125);
		}
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r11_refreshRoomBackground() {
	if (!_vm->_hasRoomAnimationCallback && !(_r11_flags & 0x10)) {
		r11_initRoomAnimations();
	} else if (_r11_flags & 0x10) {
		_vm->drawRoomImageToBackground("fillbag", 45, 92);
	}
	if (_r11_flags & 0x08) {
		_vm->drawRoomImageToBackground("plugin5", 189, 60);
	} else if (_r11_flags & 0x01) {
		_vm->drawRoomImageToBackground("plunge15", 190, 62);
	}
	if (_r11_flags & 0x02) {
		_vm->drawRoomImageToBackground("cord", 114, 140);
	}
	if (_r11_flags & 0x04) {
		_vm->drawRoomImageToBackground("suckcut", 116, 125);
	}
}

void GameLogic::r11_useCar1() {
	if (_r11_flags & 0x08) {
		_vm->playAnimation("plunge", 17, -18, 190, 62, 0, 150);
		_r11_flags &= ~0x08;
	} else {
		_vm->playAnimation("plunge", 15, -16, 190, 62, 0, 150);
	}
	_r11_flags &= ~0x01;
	_vm->setGarthPosition(205, 148);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r11_useCar2() {
	_vm->walkTo(136, 146, 1, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->_wayneSpriteX = -1;
	_vm->_garthSpriteX = -1;
	_vm->_hasRoomAnimationCallback = false;
	_vm->playAnimation("usecord", 6, -7, 106, 102, 0, 150);
	_vm->moveObjectToNowhere(kObjectIdSuckCut);
	_vm->moveObjectToRoom(kObjectIdInventorySuckCut, 99);
	_vm->refreshInventory(true);
	_r11_flags &= ~0x04;
	_vm->setWaynePosition(136, 146);
	_vm->_hasRoomAnimationCallback = true;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r11_pickUpExtensionCord() {
	if (_vm->_currentActorNum == 0 && (_r11_flags & 0x01)) {
		r11_useCar1();
	}
	if (_r11_flags & 0x04) {
		r11_useCar2();
	}
	_r11_flags &= ~0x02;
	_vm->moveObjectToNowhere(kObjectIdExtensionCord_1);
	_vm->moveObjectToRoom(kObjectIdInventoryExtensionCord, 99);
	_vm->refreshInventory(true);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r11_useSuckCutWithFountain() {
	r11_uninitRoomAnimations();
	_vm->playAnimation("suckwt", 0, 5, 45, 92, 0, 200);
	_vm->playSound("ss40", 0);
	for (int i = 5; i < 15; i++) {
		_vm->playAnimation("elec", i - 5, 1, 176, 61, 0, 1);
		_vm->playAnimation("suckwt", i, 1, 45, 92, 0, 150);
	}
	_vm->playAnimation("elec", 10, 5, 176, 61, 0, 200);
	_vm->setWaynePosition(117, 135);
	_r11_flags |= 0x10;
	_vm->moveObjectToNowhere(kObjectIdSuckCut);
	_vm->moveObjectToNowhere(kObjectIdExtensionCord_1);
	_vm->moveObjectToRoom(kObjectIdManholeCover, 11);
	_r11_flags &= ~0x04;
	_vm->setStaticRoomObjectPosition(_vm->_currentRoomNumber, 0, 0, 97, 106);
	_vm->fillRoomMaskArea(117, 110, 140, 130, 0);
	_vm->fillRoomMaskArea(81, 131, 119, 148, 1);
	r11_useCar1();
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r11_useSuckCutWithExtensionCord() {
	_vm->walkTo(136, 146, 1, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->_wayneSpriteX = -1;
	_vm->_garthSpriteX = -1;
	_vm->_hasRoomAnimationCallback = false;
	_vm->playAnimation("usecord", 0, 7, 106, 102, 0, 150);
	_vm->drawRoomImageToBackground("suckcut", 116, 125);
	_vm->moveObjectToRoom(kObjectIdSuckCut, 11);
	_vm->moveObjectToNowhere(kObjectIdInventorySuckCut);
	_vm->refreshInventory(true);
	_r11_flags |= 0x04;
	_vm->setWaynePosition(136, 146);
	_vm->_hasRoomAnimationCallback = true;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r11_useExtensionCordWithOutlet() {
	_vm->_garthSpriteX = -1;
	_vm->playAnimation("plugin", 0, 6, 189, 60, 0, 150);
	_vm->drawRoomImageToBackground("cord", 114, 140);
	_vm->moveObjectToRoom(kObjectIdExtensionCord_1, 11);
	_vm->moveObjectToNowhere(kObjectIdInventoryExtensionCord);
	_vm->refreshInventory(true);
	_r11_flags |= 0x02;
	_r11_flags |= 0x08;
}

void GameLogic::r11_usePlungersWithLampPost() {
	_vm->displayText("c04", 166, 0, -1, -1, 0);
	_vm->waitSeconds(1);
	_vm->_isTextVisible = false;
	_vm->_currentActorNum = 1;
	_vm->walkTo(146, 146, 1, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->_currentActorNum = 0;
	_vm->_garthSpriteX = -1;
	_vm->playAnimation("plunge", 0, 16, 190, 62, 0, 150);
	_r11_flags |= 0x01;
}

int GameLogic::r12_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit12:
		actionTextIndex = 7;
		break;
	case kObjectIdOffices:
		actionTextIndex = 14;
		break;
	case kObjectIdDisplay12:
		actionTextIndex = 16;
		break;
	case kObjectIdChairs12_0:
		actionTextIndex = 21;
		break;
	case kObjectIdChairs12_1:
		actionTextIndex = 30;
		break;
	case kObjectIdGate12:
		actionTextIndex = 31;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r12_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit12:
		_vm->_gameState = 1;
		break;
	case kObjectIdGate12:
		actionTextIndex = 28;
		break;
	case kObjectIdChairs12_0:
	case kObjectIdChairs12_1:
		actionTextIndex = 33;
		break;
	case kObjectIdOffices:
		actionTextIndex = 34;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r12_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	_r12_talkObjectNumber = _vm->_objectNumber;
	switch (_vm->_objectNumber) {
	case kObjectIdMan_0:
		_vm->playAnimation("lawyer", 1, 5, 55, 84, 0, 100);
		if (_r12_flags & 0x02) {
			_vm->displayTextLines("c04r", 234, 50, 30, 1);
			r12_talkToLawyer();
		} else {
			_vm->setDialogChoices(269, 270, 187, -1, -1);
		}
		break;
	case kObjectIdMan12_0:
	case kObjectIdWoman:
	case kObjectIdFarmer:
		_vm->setDialogChoices(269, 270, 187, -1, -1);
		break;
	case kObjectIdMan12_1:
		_vm->displayTextLines("c04", 284, -1, -1, 1);
		break;
	case kObjectIdCecil:
		for (int i = 0; i < 4; i++) {
			_vm->playAnimation("ctalk", 0, 2, 203, 85, 0, 100);
		}
		_vm->displayTextLines("c04r", 358, 250, 20, 1);
		if (_vm->getObjectRoom(kObjectIdInventoryPotatoChip) == 99) {
			_vm->displayTextLines("c04", 52, -1, -1, 1);
			for (int i = 0; i < 4; i++) {
				_vm->playAnimation("ctalk", 0, 2, 203, 85, 0, 100);
			}
			_vm->displayTextLines("c04r", 359, 250, 20, 1);
		} else {
			_vm->displayTextLines("c04", 341, -1, -1, 1);
			for (int i = 0; i < 4; i++) {
				_vm->playAnimation("ctalk", 0, 2, 203, 85, 0, 100);
			}
			_vm->displayTextLines("c04r", 360, 250, 20, 1);
		}
		break;
	}
}

int GameLogic::r12_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCecil:
		if (_vm->_firstObjectNumber == kObjectIdInventoryPotatoChip) {
			r12_givePotatoChipToCecil();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r12_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 50;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 269:
		if (_r12_talkObjectNumber == 207) {
			replyTextIndex1 = 224;
			if (_vm->getObjectRoom(kObjectIdInventoryCassContract) == 99 && _vm->getObjectRoom(kObjectIdInventoryContract) == 99) {
				_vm->setDialogChoices(271, 272, 274, -1, -1);
			} else {
				_vm->setDialogChoices(271, 272, 273, -1, -1);
			}
		} else if (_r12_talkObjectNumber == 209) {
			replyTextIndex1 = 236;
		} else if (_r12_talkObjectNumber == 211) {
			replyTextIndex1 = 238;
		} else {
			replyTextIndex1 = 240;
		}
		continueDialog = true;
		break;
	case 270:
		if (_r12_talkObjectNumber == 207) {
			replyTextIndex1 = 225;
		} else if (_r12_talkObjectNumber == 209) {
			replyTextIndex1 = 237;
		} else if (_r12_talkObjectNumber == 211) {
			replyTextIndex1 = 239;
		} else {
			replyTextIndex1 = 241;
		}
		continueDialog = true;
		break;
	case 187: case 280:
		if (_r12_talkObjectNumber == 207) {
			_vm->playAnimation("lawyer", 4, -5, 55, 84, 0, 100);
		}
		_vm->_gameState = 0;
		break;
	case 271: case 272: case 273:
		replyTextIndex1 = _vm->_selectedDialogChoice - 45;
		_vm->setDialogChoices(269, 270, 187, -1, -1);
		continueDialog = true;
		break;
	case 274:
		_vm->displayTextLines("c04r", 235, 50, 30, 1);
		_vm->displayTextLines("c04", 275, -1, -1, 4);
		replyTextIndex1 = 229;
		_vm->setDialogChoices(279, 280, -1, -1, -1);
		continueDialog = true;
		break;
	case 279:
		_vm->displayTextLines("c04r", 230, 50, 30, 1);
		_vm->setGameFlag(10);
		r12_talkToLawyer();
		break;
	}
	return continueDialog;
}

void GameLogic::r12_refreshRoomBackground() {
	if ((_r30_flags & 0x08) && !(_pizzathonListFlags2 & 0x02)) {
		_vm->drawRoomImageToBackground("people0", 73, 81);
		_vm->moveObjectToRoom(kObjectIdWoman, 12);
		_vm->moveObjectToRoom(kObjectIdFarmer, 12);
		_vm->moveObjectToRoom(kObjectIdCecil, 12);
		_vm->setStaticRoomObjectPosition(12, 0, 0, -1, 69);
		_vm->setStaticRoomObjectPosition(12, 2, 2, 198, 69);
	} else {
		int peopleIndex = _vm->getRandom(10) % 2;
		_vm->setStaticRoomObjectPosition(12, 0, 0, 214, 69);
		_vm->setStaticRoomObjectPosition(12, 2, 2, -1, 69);
		_vm->moveObjectToNowhere(kObjectIdCecil);
		Common::String filename = Common::String::format("people%d", peopleIndex);
		_vm->drawRoomImageToBackground(filename.c_str(), 73, 81);
		if (peopleIndex == 0) {
			_vm->moveObjectToRoom(kObjectIdWoman, 12);
			_vm->moveObjectToRoom(kObjectIdFarmer, 12);
		} else {
			_vm->moveObjectToNowhere(kObjectIdWoman);
			_vm->moveObjectToNowhere(kObjectIdFarmer);
		}
	}
	if (!(_r12_flags & 0x01)) {
		_vm->drawRoomImageToBackground("lawyer0", 55, 84);
		_vm->moveObjectToRoom(kObjectIdMan_0, 12);
	} else {
		_vm->moveObjectToNowhere(kObjectIdMan_0);
	}
}

void GameLogic::r12_talkToLawyer() {
	if (_vm->getObjectRoom(kObjectIdInventoryBusinessCards) == 99) {
		_vm->displayTextLines("c04", 281, -1, -1, 1);
		_vm->moveObjectToNowhere(kObjectIdInventoryCassContract);
		_vm->moveObjectToNowhere(kObjectIdInventoryContract);
		_vm->moveObjectToNowhere(kObjectIdInventoryBusinessCards);
		_vm->moveObjectToRoom(kObjectIdInventoryFinalContract, 99);
		_vm->refreshInventory(true);
		_r12_flags |= 0x01;
		_vm->setGameFlag(8);
	} else {
		_vm->displayTextLines("c04", 282, -1, -1, 1);
		_vm->displayTextLines("c04r", 231, 50, 30, 2);
		_vm->displayTextLines("c04", 283, -1, -1, 1);
		_vm->displayTextLines("c04r", 233, 50, 30, 1);
		_r12_flags |= 0x02;
	}
	_vm->playAnimation("lawyer", 4, -5, 55, 84, 0, 100);
	_vm->_gameState = 0;
}

void GameLogic::r12_givePotatoChipToCecil() {
	_vm->moveObjectToNowhere(kObjectIdInventoryPotatoChip);
	_vm->refreshInventory(true);
	_vm->setGameFlag(5);
	for (int textIndex = 361; textIndex < 363; textIndex++) {
		for (int i = 0; i < 4; i++) {
			_vm->playAnimation("ctalk", 0, 2, 203, 85, 0, 100);
		}
		_vm->displayTextLines("c04r", textIndex, 250, 20, 1);
	}
	_vm->displayTextLines("c04", 342, -1, -1, 1);
	for (int textIndex = 363; textIndex < 365; textIndex++) {
		for (int i = 0; i < 4; i++) {
			_vm->playAnimation("ctalk", 0, 2, 203, 85, 0, 100);
		}
		_vm->displayTextLines("c04r", textIndex, 250, 20, 1);
	}
	_vm->playAnimation("cwalk", 0, 13, 198, 84, 0, 100);
	_vm->setStaticRoomObjectPosition(12, 0, 0, 214, 69);
	_vm->setStaticRoomObjectPosition(12, 2, 2, -1, 69);
	_vm->moveObjectToNowhere(kObjectIdCecil);
	_vm->drawRoomImageToBackground("cwalk12", 198, 84);
}

int GameLogic::r13_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit13:
		_vm->_gameState = 1;
		break;
	case kObjectIdHallway13:
		if (_r13_flags & 0x04) {
			_vm->setWaynePosition(67, 110);
			_vm->setGarthPosition(56, 109);
			_vm->changeRoom(18);
		} else {
			_vm->displayTextLines("c03", 49, 200, 30, 1);
		}
		break;
	case kObjectIdExit18:
		r18_uninitRoomAnimations();
		_vm->setWaynePosition(93, 78);
		_vm->setGarthPosition(102, 77);
		_vm->changeRoom(13);
		break;
	case kObjectIdTable13_0:
	case kObjectIdTable13_1:
	case kObjectIdTable13_2:
		actionTextIndex = 1;
		break;
	case kObjectIdLooseTube:
	case kObjectIdPieceOfPizza:
		actionTextIndex = 3;
		break;
	case kObjectIdCandle13_0:
	case kObjectIdCandle13_1:
	case kObjectIdCandle13_2:
	case kObjectIdCandle13_3:
		actionTextIndex = 24;
		break;
	case kObjectIdStage13:
		actionTextIndex = 25;
		break;
	case kObjectIdPizzaMachine:
		actionTextIndex = 26;
		break;
	case kObjectIdOven:
		actionTextIndex = 27;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r13_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdPieceOfPizza:
		_vm->pickupObject(kObjectIdPieceOfPizza, _r13_flags, 1, kObjectIdInventoryCheesePizza);
		actionTextIndex = 5;
		break;
	case kObjectIdLooseTube:
		_vm->pickupObject(kObjectIdLooseTube, _r13_flags, 2, kObjectIdInventoryTube);
		actionTextIndex = 6;
		break;
	case kObjectIdExit13:
	case kObjectIdExit18:
		actionTextIndex = 7;
		break;
	case kObjectIdHallway13:
	case kObjectIdPizzaMachine:
		actionTextIndex = 8;
		break;
	case kObjectIdStage13:
	case kObjectIdOven:
		actionTextIndex = 14;
		break;
	case kObjectIdCandle13_0:
	case kObjectIdCandle13_1:
	case kObjectIdCandle13_2:
	case kObjectIdCandle13_3:
		actionTextIndex = 27;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r13_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdPepe13:
		_vm->walkTo(98, 84, 6, 101, 80);
		_vm->setDialogChoices(183, 184, 185, 186, 187);
		break;
	}
}

int GameLogic::r13_handleVerbGive() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdPepe13:
		if (_vm->_firstObjectNumber == kObjectIdInventoryTicketToRome) {
			r13_giveTicketToRomeToPepe();
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r13_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 183:
		if (_r13_flags & 0x08) {
			replyTextIndex1 = 100;
			replyTextIndex2 = 101;
		} else {
			r13_handleDialogSelect183();
		}
		continueDialog = true;
		break;
	case 184:
		replyTextIndex1 = 102;
		replyTextIndex2 = 103;
		replyTextIndex3 = 104;
		continueDialog = true;
		break;
	case 185:
		replyTextIndex1 = 105;
		replyTextIndex2 = 106;
		continueDialog = true;
		break;
	case 186:
		replyTextIndex1 = 107;
		continueDialog = true;
		break;
	case 187:
		_vm->_gameState = 0;
		break;
	case 422:
		_vm->displayTextLines("c04r", 455, 160, 20, 2);
		_vm->displayTextLines("c04", 425, -1, -1, 1);
		_vm->displayTextLines("c04r", 457, 160, 20, 7);
		_vm->displayTextLines("c04", 426, -1, -1, 2);
		_vm->setDialogChoices(429, 430, 431, -1, -1);
		continueDialog = true;
		break;
	case 423:
		_vm->displayTextLines("c04r", 464, 160, 20, 5);
		_vm->displayTextLines("c04", 428, -1, -1, 1);
		_vm->setDialogChoices(435, 436, 437, -1, -1);
		continueDialog = true;
		break;
	case 424:
		replyTextIndex1 = 469;
		continueDialog = true;
		_vm->setDialogChoices(438, 439, 440, -1, -1);
		break;
	case 429:
		_vm->displayTextLines("c04r", 470, 160, 20, 1);
		_vm->displayTextLines("c04", 432, -1, -1, 2);
		_vm->displayTextLines("c04r", 471, 160, 20, 1);
		_vm->displayTextLines("c04", 434, -1, -1, 1);
		replyTextIndex1 = 472;
		continueDialog = true;
		_vm->setDialogChoices(441, 442, 443, -1, -1);
		break;
	case 430: case 431:
		replyTextIndex1 = _vm->_selectedDialogChoice + 43;
		continueDialog = true;
		break;
	case 435: case 437:
		replyTextIndex1 = 475;
		continueDialog = true;
		_vm->setDialogChoices(422, 423, 424, -1, -1);
		break;
	case 436:
		replyTextIndex1 = 476;
		continueDialog = true;
		break;
	case 438: case 439: case 440:
		replyTextIndex1 = 477;
		continueDialog = true;
		_vm->setDialogChoices(422, 423, 424, -1, -1);
		break;
	case 441:
		replyTextIndex1 = 478;
		replyTextIndex2 = 479;
		continueDialog = true;
		break;
	case 442:
		replyTextIndex1 = 480;
		replyTextIndex2 = 481;
		continueDialog = true;
		break;
	case 443:
		_vm->displayTextLines("c04r", 482, 160, 20, 3);
		_vm->displayTextLines("c04", 444, -1, -1, 1);
		_vm->displayTextLines("c04r", 485, 160, 20, 1);
		_vm->setDialogChoices(445, 446, 447, -1, -1);
		continueDialog = true;
		break;
	case 445: case 446: case 447:
		_vm->displayTextLines("c04r", 486, 160, 20, 1);
		_vm->displayTextLines("c04", 448, -1, -1, 1);
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r13_refreshRoomBackground() {
	if (!(_r13_flags & 0x04)) {
		_vm->drawRoomImageToBackground("ptalk0", 72, 60);
	}
	if (_pizzathonListFlags2 & 0x08) {
		_r13_flags |= 0x04;
		_vm->drawRoomImageToBackground("newbackg", 0, 0);
		_vm->setStaticRoomObjectPosition(13, 0, 3, -1, -1);
		_vm->setStaticRoomObjectPosition(13, 4, 4, 96, 53);
		_vm->setStaticRoomObjectPosition(13, 5, 5, 0, 94);
		_vm->loadRoomMask(96);
		if (!(_r13_flags & 0x10)) {
			_vm->_roomEventNum = 1;
		}
	}
}

void GameLogic::r13_handleRoomEvent() {
	_r13_flags |= 0x10;
	_vm->playAnimation("crew", 0, 17, 76, 59, 0, 100);
	_vm->displayTextLines("c04r", 453, 160, 20, 2);
	_vm->displayTextLines("c04", 420, -1, -1, 2);
	_vm->setDialogChoices(422, 423, 424, -1, -1);
	_vm->startDialog();
}

void GameLogic::r13_giveTicketToRomeToPepe() {
	_vm->moveObjectToNowhere(kObjectIdInventoryTicketToRome);
	_vm->displayTextLines("c04r", 108, 200, 30, 2);
	_vm->displayTextLines("c04", 188, -1, -1, 1);
	_vm->displayTextLines("c04r", 110, 200, 30, 2);
	_vm->displayTextLines("c04", 189, -1, -1, 1);
	_vm->moveObjectToRoom(kObjectIdInventoryLuckySausage, 99);
	_vm->refreshInventory(false);
	_r13_flags |= 0x04;
	_vm->playAnimation("pepego", 0, 10, 60, 58, 0, 100);
	_vm->moveObjectToNowhere(kObjectIdPepe13);
	_vm->setGameFlag(2);
	_vm->refreshActors();
}

void GameLogic::r13_handleDialogSelect183() {
	_vm->displayTextLines("c04r", 99, 200, 30, 1);
	_vm->displayTextLines("c04", 12, -1, -1, 3);
	_r13_flags |= 0x08;
}

int GameLogic::r14_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCar14:
	case kObjectIdCar19:
		_vm->_currentRoomNumber = 14;
		_vm->_gameState = 1;
		break;
	case kObjectIdWarehouse:
		_vm->setWaynePosition(177, 101);
		_vm->setGarthPosition(168, 98);
		_vm->changeRoom(20);
		break;
	case kObjectIdExit20:
		_vm->setWaynePosition(256, 83);
		_vm->_garthSpriteX = 250;
		_vm->_wayneSpriteY = 87;
		_vm->changeRoom(19);
		break;
	case kObjectIdBench:
		actionTextIndex = 1;
		break;
	case kObjectIdEntrance14:
		actionTextIndex = 28;
		break;
	case kObjectIdLockers20:
		actionTextIndex = 29;
		break;
	case kObjectIdMunchies:
	case kObjectIdFood:
		actionTextIndex = 30;
		break;
	case kObjectIdInventoryRemoteControl:
		if (_r19_flags & 0x08) {
			actionTextIndex = 56;
		} else if (_r19_flags & 0x04) {
			actionTextIndex = 54;
			_vm->_roomEventNum = 1;
		} else {
			actionTextIndex = 55;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r14_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLoadingDock:
	case kObjectIdWarehouse:
		actionTextIndex = 7;
		break;
	case kObjectIdBillboard14:
	case kObjectIdBillboard_1:
	case kObjectIdBillboard19:
	case kObjectIdBillboard_2:
		actionTextIndex = 8;
		break;
	case kObjectIdEntrance14:
	case kObjectIdStore:
	case kObjectIdExit20:
		actionTextIndex = 14;
		break;
	case kObjectIdClock:
		actionTextIndex = 12;
		break;
	case kObjectIdMunchies:
	case kObjectIdFood:
		actionTextIndex = 28;
		break;
	case kObjectIdRemains:
		actionTextIndex = 49;
		break;
	case kObjectIdPotatoChip:
		_vm->pickupObject(kObjectIdPotatoChip, _r19_flags, 16, kObjectIdInventoryPotatoChip);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r14_refreshRoomBackground() {
	if (_pizzathonListFlags1 & 0x08) {
		_vm->moveObjectToRoom(kObjectIdBillboard_1, 14);
		_vm->moveObjectToNowhere(kObjectIdBillboard14);
		_vm->moveObjectToRoom(kObjectIdBillboard_2, 19);
		_vm->moveObjectToNowhere(kObjectIdBillboard19);
		_vm->drawRoomImageToBackground("bboard", 144, 38);
	}
	if ((_r1_flags1 & 0x10) && !(_r19_flags & 0x08)) {
		_vm->drawRoomImageToBackground("robot", 169, 76);
		_vm->moveObjectToRoom(kObjectIdRobot, 14);
		_vm->moveObjectToRoom(kObjectIdGilligan, 19);
	} else {
		_vm->moveObjectToNowhere(kObjectIdRobot);
		_vm->moveObjectToNowhere(kObjectIdGilligan);
	}
}

bool GameLogic::r15_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 20;
	replyTextY = 10;
	switch (_vm->_selectedDialogChoice) {
	case 54:
		replyTextIndex1 = 84;
		break;
	case 55:
		replyTextIndex1 = 85;
		break;
	case 56:
		replyTextIndex1 = 86;
		break;
	}
	_vm->_gameState = 0;
	return continueDialog;
}

void GameLogic::r16_refreshRoomBackground() {
	if (_r7_flags & 0x01) {
		_vm->drawRoomImageToBackground("nocape", 179, 58);
	}
	if (_r7_flags & 0x02) {
		_vm->drawRoomImageToBackground("nohat", 201, 59);
	}
}

int GameLogic::r17_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit17:
		_vm->changeRoom(24);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r17_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdGuard17_0:
	case kObjectIdGuard17_1:
		_vm->setDialogChoices(300, 301, 302, 303, -1);
		break;
	}
}

bool GameLogic::r17_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 70;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 300:
		replyTextIndex1 = 298;
		continueDialog = true;
		break;
	case 301:
		_vm->displayTextLines("c04r", 300, 70, 30, 1);
		if (_vm->getObjectRoom(kObjectIdInventoryPassCard) == 99) {
			_vm->displayTextLines("c04", 305, -1, -1, 1);
			_vm->setDialogChoices(306, 307, 308, 309, -1);
			replyTextIndex1 = 301;
			replyTextIndex2 = 302;
			continueDialog = true;
		} else {
			_vm->displayTextLines("c04", 304, -1, -1, 1);
			_vm->_gameState = 0;
		}
		break;
	case 302:
		replyTextIndex1 = 299;
		continueDialog = true;
		break;
	case 303: case 311:
		_vm->_gameState = 0;
		break;
	case 306: case 307: case 308: case 309:
		_vm->setDialogChoices(310, 311, -1, -1, -1);
		_r17_dialogCtr = 0;
		replyTextIndex1 = 303;
		replyTextIndex2 = 304;
		continueDialog = true;
		break;
	case 310:
		replyTextIndex1 = 305;
		replyTextIndex2 = 306;
		_vm->setDialogChoices(312, 313, 314, 315, -1);
		continueDialog = true;
		break;
	case 315:
		_r17_dialogCtr = _r17_dialogCtr + 1;
	case 312: case 313: case 314:
		replyTextIndex1 = 307;
		replyTextIndex2 = 308;
		replyTextIndex3 = 309;
		_vm->setDialogChoices(316, 317, 318, 319, -1);
		continueDialog = true;
		break;
	case 317:
		_r17_dialogCtr = _r17_dialogCtr + 1;
	case 316: case 318: case 319:
		replyTextIndex1 = 310;
		replyTextIndex2 = 311;
		_vm->setDialogChoices(320, 321, 322, 323, -1);
		continueDialog = true;
		break;
	case 322:
		_r17_dialogCtr = _r17_dialogCtr + 1;
	case 320: case 321: case 323:
		replyTextIndex1 = 312;
		replyTextIndex2 = 313;
		_vm->setDialogChoices(324, 325, 326, 327, -1);
		continueDialog = true;
		break;
	case 324:
		_r17_dialogCtr = _r17_dialogCtr + 1;
	case 325: case 326: case 327:
		if (_r17_dialogCtr != 4) {
			replyTextIndex1 = 314;
			_vm->_gameState = 0;
		} else {
			replyTextIndex1 = 315;
			replyTextIndex2 = 316;
			_vm->_roomEventNum = 2;
			_vm->_gameState = 0;
		}
		break;
	}
	return continueDialog;
}

void GameLogic::r17_refreshRoomBackground() {
	_vm->drawRoomImageToBackgroundTransparent("guard1", 49, 79);
	_vm->drawRoomImageToBackgroundTransparent("guard2", 112, 79);
	if (_r17_eventFlag == 0) {
		if (_r37_flags & 0x20) {
			_vm->_roomEventNum = 3;
		} else {
			_vm->_roomEventNum = 1;
		}
	}
}

void GameLogic::r17_handleRoomEvent1() {
	_vm->displayTextLines("c04r", 297, 70, 30, 1);
	_vm->walkTo(71, 118, 4, 90, 118);
	_vm->setDialogChoices(300, 301, 302, 303, -1);
	_vm->startDialog();
}

void GameLogic::r17_handleRoomEvent2() {
	_vm->walkTo(71, 117, 0, 90, 117);
	_vm->playSound("ss13", 0);
	_vm->playAnimation("beam", 0, 13, 62, 51, 0, 150);
	_vm->changeRoom(37);
}

void GameLogic::r17_handleRoomEvent3() {
	_vm->playSound("ss13", 0);
	_vm->playAnimation("beam", 12, -13, 62, 51, 0, 150);
	_r37_flags &= ~0x20;
	_vm->setWaynePosition(71, 117);
	_vm->setGarthPosition(90, 117);
	_r17_eventFlag = 1;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r17_eventFlag = 0;
}

void GameLogic::r18_initRoomAnimations() {
	_vm->startRoomAnimations();
}

void GameLogic::r18_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r18_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 4500;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 4500;
	}
	if (_vm->_animationsCtr % 500 == 0 || doUpdate) {
		Common::String machineFilename = Common::String::format("machine%d", _vm->_animationsCtr / 500);
		_vm->drawRoomImageToBackground(machineFilename.c_str(), 115, 51);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r18_refreshRoomBackground() {
	if (!(_r13_flags & 0x01)) {
		_vm->drawRoomImageToBackground("pizza", 18, 92);
	}
	if (_r13_flags & 0x02) {
		_vm->drawRoomImageToBackground("notube", 71, 63);
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r18_initRoomAnimations();
	}
}

bool GameLogic::r19_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 275;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 256:
		_r19_flags |= 0x04;
		replyTextIndex1 = 211;
		replyTextIndex2 = 212;
		continueDialog = true;
		break;
	case 257:
		replyTextIndex1 = 213;
		continueDialog = true;
		_vm->setDialogChoices(261, 262, 263, 264, -1);
		break;
	case 258:
		replyTextIndex1 = 214;
		continueDialog = true;
		break;
	case 259:
		if (_r19_flags & 0x02) {
			replyTextIndex1 = 218;
		} else {
			replyTextIndex1 = 215;
			_vm->setDialogChoices(266, 268, -1, -1, -1);
		}
		continueDialog = true;
		break;
	case 260: case 264: case 268:
		_vm->_gameState = 0;
		break;
	case 261:
		_vm->displayTextLines("c04r", 216, 275, 30, 1);
		_vm->displayTextLines("c04", 265, -1, -1, 1);
		_vm->setDialogChoices(256, 257, 258, 259, 260);
		continueDialog = true;
		break;
	case 262:
		replyTextIndex1 = 217;
		_vm->setDialogChoices(256, 257, 258, 259, 260);
		continueDialog = true;
		break;
	case 263:
		if (_r19_flags & 0x02) {
			replyTextIndex1 = 218;
			_vm->setDialogChoices(256, 257, 258, 259, 260);
			continueDialog = true;
		} else {
			replyTextIndex1 = 215;
			_vm->setDialogChoices(266, 268, -1, -1, -1);
			continueDialog = true;
		}
		break;
	case 266:
		_vm->displayTextLines("c04r", 219, 275, 30, 1);
		_vm->displayTextLines("c04", 267, -1, -1, 1);
		replyTextIndex1 = 220;
		_vm->setDialogChoices(256, 257, 258, 259, 260);
		_r19_flags |= 0x02;
		continueDialog = true;
		break;
	case 449: case 450: case 451:
		replyTextIndex1 = _vm->_selectedDialogChoice + 40;
		replyTextIndex2 = 492;
		replyTextIndex3 = 493;
		continueDialog = true;
		_vm->setDialogChoices(453, 454, 455, 456, -1);
		break;
	case 452: case 460:
		_vm->_gameState = 0;
		break;
	case 453: case 454: case 455: case 456:
		_vm->displayTextLines("c04r", _vm->_selectedDialogChoice + 41, 300, 20, 1);
		_vm->displayTextLines("c04r", 498, 300, 20, 4);
		replyTextIndex1 = 502;
		replyTextIndex2 = 503;
		replyTextIndex3 = 504;
		continueDialog = true;
		_vm->setDialogChoices(457, 458, 459, 460, -1);
		break;
	case 457:
		_vm->displayTextLines("c04r", 505, 300, 20, 2);
		replyTextIndex1 = 507;
		replyTextIndex2 = 508;
		replyTextIndex3 = 509;
		continueDialog = true;
		_vm->setDialogChoices(461, 462, 463, -1, -1);
		break;
	case 458:
		_vm->displayTextLines("c04r", 510, 300, 20, 1);
		replyTextIndex1 = 511;
		replyTextIndex2 = 512;
		replyTextIndex3 = 513;
		continueDialog = true;
		break;
	case 459:
		replyTextIndex1 = 514;
		replyTextIndex2 = 515;
		continueDialog = true;
		_vm->setDialogChoices(469, 470, 471, -1, -1);
		break;
	case 461:
		_vm->displayTextLines("c04r", 516, 300, 20, 2);
		_vm->displayTextLines("c04", 464, -1, -1, 4);
		_vm->displayTextLines("c04r", 518, 300, 20, 1);
		_vm->displayTextLines("c04", 468, -1, -1, 1);
		continueDialog = true;
		break;
	case 462:
		_vm->displayTextLines("c04r", 519, 300, 20, 2);
		replyTextIndex1 = 521;
		replyTextIndex2 = 522;
		replyTextIndex3 = 523;
		continueDialog = true;
		break;
	case 463:
		if (_r19_flags & 0x20) {
			replyTextIndex1 = 536;
		} else {
			_vm->moveObjectToRoom(kObjectIdInventorySinusPotion, 99);
			_vm->refreshInventory(true);
			_r19_flags |= 0x20;
			replyTextIndex1 = 524;
		}
		_vm->setDialogChoices(457, 458, 459, 460, -1);
		continueDialog = true;
		break;
	case 469:
		_vm->displayTextLines("c04r", 525, 300, 20, 1);
		_vm->displayTextLines("c04", 472, -1, -1, 1);
		replyTextIndex1 = 526;
		continueDialog = true;
		break;
	case 470:
		_vm->displayTextLines("c04r", 527, 300, 20, 2);
		replyTextIndex1 = 529;
		replyTextIndex2 = 530;
		replyTextIndex3 = 531;
		continueDialog = true;
		_vm->setDialogChoices(457, 458, 459, 460, -1);
		break;
	case 471:
		replyTextIndex1 = 532;
		replyTextIndex2 = 533;
		replyTextIndex3 = 534;
		continueDialog = true;
		_vm->setDialogChoices(457, 458, 459, 460, -1);
		break;
	}
	return continueDialog;
}

void GameLogic::r19_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdGilligan:
		_vm->setDialogChoices(256, 257, 258, 259, 260);
		break;
	case kObjectIdMan_1:
		_vm->drawRoomImageToScreen("mtalk1", 184, 68);
		_vm->displayTextLines("c04r", 487, 300, 20, 2);
		_vm->setDialogChoices(449, 450, 451, 452, -1);
		break;
	}
}

void GameLogic::r19_refreshRoomBackground() {
	if (_pizzathonListFlags1 & 0x08) {
		_vm->moveObjectToRoom(kObjectIdBillboard_1, 14);
		_vm->moveObjectToNowhere(kObjectIdBillboard14);
		_vm->moveObjectToRoom(kObjectIdBillboard_2, 19);
		_vm->moveObjectToNowhere(kObjectIdBillboard19);
		_vm->drawRoomImageToBackground("bboard", 32, 38);
	}
	if ((_r1_flags1 & 0x10) && !(_r19_flags & 0x08)) {
		_vm->drawRoomImageToBackground("robot", 57, 76);
		_vm->moveObjectToRoom(kObjectIdRobot19, 19);
	} else {
		_vm->moveObjectToNowhere(kObjectIdRobot19);
	}
	if ((_r1_flags1 & 0x10) && !(_pizzathonListFlags1 & 0x04) && !(_pizzathonListFlags2 & 0x08)) {
		_vm->drawRoomImageToBackground("gill0", 273, 84);
		_vm->moveObjectToRoom(kObjectIdGilligan, 19);
	} else {
		_vm->moveObjectToNowhere(kObjectIdGilligan);
	}
}

void GameLogic::r19_handleRoomEvent() {
	_vm->waitSeconds(2);
	_vm->_isTextVisible = false;
	_vm->walkTo(259, 130, 5, 250, 120);
	_vm->playAnimation("rob1", 0, 21, 27, 76, 0, 100);
	_vm->playAnimation("rob2", 0, 18, 143, 52, 0, 100);
	_vm->playAnimation("rob3", 0, 15, 198, 53, 0, 100);
	_r19_wayneSpriteX = _vm->_wayneSpriteX;
	_r19_garthSpriteX = _vm->_garthSpriteX;
	_vm->_wayneSpriteX = -1;
	_vm->_garthSpriteX = -1;
	_r19_flags |= 0x01;
	_vm->changeRoom(20);
}

void GameLogic::r20_refreshRoomBackground() {
	if (_r19_flags & 0x01) {
		_vm->drawRoomImageToBackground("expl0", 40, 32);
		_vm->_roomEventNum = 1;
	} else {
		_vm->setStaticRoomObjectPosition(20, 0, 1, -1, -1);
		if (_pizzathonListFlags2 & 0x08) {
			_vm->drawRoomImageToBackground("newbackg", 0, 0);
			_vm->setStaticRoomObjectPosition(20, 2, 2, 176, 64);
			_vm->moveObjectToRoom(kObjectIdMan_1, 20);
			_vm->moveObjectToNowhere(kObjectIdCrate);
			_vm->moveObjectToNowhere(kObjectIdPotatoChip);
			_vm->moveObjectToNowhere(kObjectIdRemains);
			_vm->moveObjectToNowhere(kObjectIdMunchies);
			_vm->moveObjectToNowhere(kObjectIdFood);
		} else if (_r19_flags & 0x08) {
			_vm->drawRoomImageToBackground("robjunk", 38, 93);
			_vm->setStaticRoomObjectPosition(20, 1, 1, 72, 95);
			if (_r19_flags & 0x10) {
				_vm->drawRoomImageToBackground("nochip", 139, 108);
				_vm->moveObjectToNowhere(kObjectIdPotatoChip);
			} else {
				_vm->moveObjectToRoom(kObjectIdPotatoChip, 20);
			}
			_vm->moveObjectToRoom(kObjectIdRemains, 20);
			_vm->moveObjectToNowhere(kObjectIdCrate);
		} else if (_r1_flags1 & 0x10) {
			_vm->drawRoomImageToBackground("crate", 103, 84);
			_vm->setStaticRoomObjectPosition(20, 0, 0, 103, 84);
			_vm->moveObjectToRoom(kObjectIdCrate, 20);
		}
	}
}

void GameLogic::r20_handleRoomEvent() {
	_vm->waitSeconds(1);
	_vm->playAnimation("expl", 0, 36, 40, 32, 0, 150);
	_vm->playSound("ss17", 0);
	_vm->playAnimation("expl", 37, 66, 40, 32, 0, 150);
	_r19_flags &= ~0x01;
	_r19_flags |= 0x08;
	_vm->_wayneSpriteX = _r19_wayneSpriteX;
	_vm->_garthSpriteX = _r19_garthSpriteX;
	_vm->_isTextVisible = false;
	_vm->changeRoom(19);
	_vm->displayTextLines("c04r", 221, 275, 30, 3);
	_vm->playAnimation("gillgo", 0, 9, 273, 79, 0, 100);
	_vm->moveObjectToNowhere(kObjectIdGilligan);
	_vm->setGameFlag(7);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r22_refreshRoomBackground() {
	if (_r8_flags & 0x02) {
		_vm->drawRoomImageToBackground("cab1open", 258, 69);
	}
	if (_r8_flags & 0x04) {
		_vm->drawRoomImageToBackground("cab2open", 206, 52);
	}
	if (_r29_flags & 0x02) {
		_vm->drawRoomImageToBackground("nomap", 209, 87);
	}
}

int GameLogic::r24_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdOpening_0:
	case kObjectIdOpening_1:
		if (_vm->_objectNumber == kObjectIdOpening_0) {
			r24_climbLadder(92, 72, 90, -50, 0);
		} else {
			r24_climbLadder(203, 183, 90, -50, 1);
		}
		// Fall-through
	case kObjectIdOpening_2:
	case kObjectIdOpening_3:
	case kObjectIdOpening_4:
	case kObjectIdOpening_5:
	case kObjectIdOpening_6:
		_vm->setStaticRoomObjectPosition(24, 0, 1, -1, 132);
		_vm->setStaticRoomObjectPosition(24, 2, 2, -1, 144);
		r24_useMazeHole(_vm->_objectNumber - 306);
		if (_r24_mazeHoleNumber > 6) {
			r25_updateMazeRoomHole(_r24_mazeRoomNumber);
		}
		switch (_r24_mazeHoleNumber) {
		case 7:
			_vm->changeRoom(25);
			break;
		case 8:
			_vm->changeRoom(26);
			break;
		case 9:
			_vm->changeRoom(27);
			break;
		case 10:
			_vm->setWaynePosition(276, 104);
			_vm->setGarthPosition(287, 105);
			_r24_mazeHoleNumber = 5;
			_vm->changeRoom(38);
			break;
		case 11:
			_vm->setWaynePosition(273, 112);
			_vm->setGarthPosition(264, 106);
			_r24_mazeHoleNumber = 2;
			_vm->changeRoom(17);
			break;
		case 12:
			_vm->setWaynePosition(293, 118);
			_vm->setGarthPosition(281, 110);
			_r24_mazeHoleNumber = 5;
			_vm->changeRoom(36);
			break;
		case 13:
			_vm->setWaynePosition(43, 109);
			_vm->setGarthPosition(25, 112);
			_r24_mazeHoleNumber = 3;
			_vm->changeRoom(35);
			break;
		case 14:
			_vm->setWaynePosition(164, 91);
			_vm->setGarthPosition(153, 90);
			_r24_mazeHoleNumber = 3;
			_vm->changeRoom(34);
			break;
		case 15:
			_vm->setWaynePosition(124, 131);
			_vm->setGarthPosition(135, 140);
			_r24_mazeHoleNumber = 0;
			_vm->changeRoom(11);
			break;
		case 16:
			_vm->setWaynePosition(279, 140);
			_vm->setGarthPosition(267, 128);
			_r24_mazeHoleNumber = 2;
			_vm->changeRoom(39);
			break;
		case 17:
			_vm->setWaynePosition(307, 114);
			_vm->setGarthPosition(310, 105);
			_r24_mazeHoleNumber = 3;
			_vm->changeRoom(28);
			break;
		case 18:
			_vm->setWaynePosition(77, 59);
			_vm->setGarthPosition(83, 51);
			_r24_mazeHoleNumber = 2;
			_vm->changeRoom(29);
			break;
		default:
			_vm->changeRoom(24);
			break;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r24_refreshRoomBackground() {
	if (_r24_mazeRoomNumber > 48 && _r24_mazeRoomNumber < 59) {
		_vm->drawRoomImageToBackground("bbackg", 0, 0);
	} else if (_r24_mazeRoomNumber > 58) {
		_vm->drawRoomImageToBackground("rbackg", 0, 0);
	}
	for (int holeIndex = 0; holeIndex < 7; holeIndex++) {
		if (kMazeInfos[_r24_mazeRoomNumber][holeIndex] != -1) {
			Common::String holeFilename;
			_vm->moveObjectToRoom(holeIndex + 306, 24);
			if (_r24_mazeRoomNumber < 49) {
				holeFilename = Common::String::format("hole%d", holeIndex);
			} else if (_r24_mazeRoomNumber > 58) {
				holeFilename = Common::String::format("rhole%d", holeIndex);
			} else {
				holeFilename = Common::String::format("bhole%d", holeIndex);
			}
			_vm->drawRoomImageToBackground(holeFilename.c_str(), kRoom24MazeHolePositionsX[holeIndex], kRoom24MazeHolePositionsY[holeIndex]);
			if (holeIndex == 0) {
				_vm->setStaticRoomObjectPosition(24, 0, 0, 95, 18);
				_vm->fillRoomMaskArea(87, 128, 116, 136, 1);
			} else if (holeIndex == 1) {
				_vm->setStaticRoomObjectPosition(24, 1, 1, 206, 18);
				_vm->fillRoomMaskArea(199, 128, 227, 136, 1);
			} else if (holeIndex == 6) {
				_vm->setStaticRoomObjectPosition(24, 2, 2, 95, 26);
				_vm->fillRoomMaskArea(58, 128, 147, 146, 1);
			}
		} else {
			_vm->moveObjectToNowhere(holeIndex + 306);
		}
	}
	_vm->setWaynePosition(kRoom24MazeWaynePositionsX[_r24_mazeHoleNumber], kRoom24MazeWaynePositionsY[_r24_mazeHoleNumber]);
	_vm->setGarthPosition(kRoom24MazeGarthPositionsX[_r24_mazeHoleNumber], kRoom24MazeGarthPositionsY[_r24_mazeHoleNumber]);
	if (_r24_mazeHoleNumber == 0) {
		_vm->_roomEventNum = 1;
	} else if (_r24_mazeHoleNumber == 1) {
		_vm->_roomEventNum = 2;
	}
}

void GameLogic::r24_climbLadder(int wayneX, int wayneLadderX, int wayneLadderY, int ladderTopY, int ceilingIndex) {
	int garthClimbX = wayneX;
	int garthLadderX = wayneLadderX + 20;
	int ceilingX = wayneX - 10;
	Common::String tempFilename;
	WWSurface *workBackground;
	WWSurface *ceilingSprite;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *getladSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetlad%d", index);
		getladSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	workBackground = new WWSurface(320, 150);

	if (_r24_mazeRoomNumber < 49) {
		tempFilename = Common::String::format("ceil%d", ceilingIndex);
		ceilingSprite = _vm->loadRoomSurface(tempFilename.c_str());
	} else {
		tempFilename = Common::String::format("rceil%d", ceilingIndex);
		ceilingSprite = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(getladSprites[index], wayneLadderX, wayneLadderY);
		workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], garthLadderX + 20, wayneLadderY - 10);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	for (int index = 0; index < 3; index++) {
		delete getladSprites[index];
	}

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("ggetlad%d", index);
		getladSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	int climbCtr = 0, garthLadderY = wayneLadderY;
	while (wayneLadderY > ladderTopY) {
		if (climbCtr % 2 == 1) {
			wayneLadderY -= 7;
			if (climbCtr > 12) {
				garthLadderY -= 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], wayneX, wayneLadderY);
		if (climbCtr < 10) {
			workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], garthLadderX + 20, garthLadderY - 10);
		} else if (climbCtr > 12) {
			workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], garthClimbX, garthLadderY);
		} else {
			workBackground->drawSurfaceTransparent(getladSprites[climbCtr - 10], garthLadderX, garthLadderY);
		}
		workBackground->drawSurface(ceilingSprite, ceilingX, 0);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		delete getladSprites[index];
	}

	delete workBackground;
	delete ceilingSprite;
}

void GameLogic::r24_useMazeHole(int holeIndex) {
	int nextMazeRoom = kMazeInfos[_r24_mazeRoomNumber][holeIndex];
	if (nextMazeRoom < 0) {
		_r24_mazeHoleNumber = nextMazeRoom + 20;
	} else {
		_r24_mazeHoleNumber = nextMazeRoom % 10;
		_r24_mazeRoomNumber = nextMazeRoom / 10;
	}
}

void GameLogic::r24_handleRoomEvent(int wayneLadderX, int wayneX, int wayneLadderY, int ladderBottomY, int climbCtrMax, int ceilingIndex) {
	int garthX = wayneLadderX;
	int ceilingX = wayneLadderX - 10;

	Common::String tempFilename;
	WWSurface *workBackground;
	WWSurface *ceilingSprite;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *getladSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetlad%d", index);
		getladSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	workBackground = new WWSurface(320, 150);

	if (_r24_mazeRoomNumber < 49) {
		tempFilename = Common::String::format("ceil%d", ceilingIndex);
		ceilingSprite = _vm->loadRoomSurface(tempFilename.c_str());
	} else {
		tempFilename = Common::String::format("rceil%d", ceilingIndex);
		ceilingSprite = _vm->loadRoomSurface(tempFilename.c_str());
	}

	int climbCtr = 0, garthLadderY = wayneLadderY;
	while (garthLadderY < ladderBottomY) {
		if (climbCtr % 2 == 0) {
			wayneLadderY += 7;
			if (climbCtr > 12) {
				garthLadderY += 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], garthX, garthLadderY);
		if (climbCtr < climbCtrMax) {
			workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], wayneLadderX, wayneLadderY);
		} else if (climbCtrMax + 2 < climbCtr) {
			workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], wayneX - 20, ladderBottomY - 2);
		} else {
			workBackground->drawSurfaceTransparent(getladSprites[40 - climbCtr], wayneX, ladderBottomY);
		}
		workBackground->drawSurfaceTransparent(ceilingSprite, ceilingX, 0);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 0; index < 3; index++) {
		delete getladSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("ggetlad%d", index);
		getladSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 2; index >= 0; index--) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(getladSprites[index], wayneX + 20, ladderBottomY);
		workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], wayneX - 20, ladderBottomY - 2);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	delete workBackground;
	delete ceilingSprite;
	_vm->_wayneSpriteX = 0;
	_vm->_garthSpriteX = 0;
}

int GameLogic::r25_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdOpening25:
	case kObjectIdOpening_9:
	case kObjectIdOpening27:
		r24_useMazeHole(2);
		break;
	case kObjectIdOpening26:
	case kObjectIdOpening_12:
		r24_useMazeHole(3);
		break;
	case kObjectIdOpening_7:
	case kObjectIdOpening_8:
	case kObjectIdOpening_11:
		r24_useMazeHole(4);
		break;
	case kObjectIdOpening_10:
		r24_useMazeHole(5);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	if (actionTextIndex != 0) {
		_vm->changeRoom(24);
	}
	return actionTextIndex;
}

void GameLogic::r25_refreshRoomBackground() {
	if (_r25_holeIndex != -1) {
		Common::String holeFilename = Common::String::format("hole%d", _r25_holeIndex % 2);
		_vm->drawRoomImageToBackground(holeFilename.c_str(), kRoom25MazeHolePositionsX[_r25_holeIndex], kRoom25MazeHolePositionsY[_r25_holeIndex]);
	}
	_vm->setWaynePosition(150, 125);
	_vm->setGarthPosition(180, 125);
}

void GameLogic::r25_updateMazeRoomHole(int mazeRoomNumber) {
	_vm->moveObjectToNowhere(314);
	_vm->moveObjectToNowhere(315);
	_vm->moveObjectToNowhere(317);
	_vm->moveObjectToNowhere(318);
	_vm->moveObjectToNowhere(320);
	_vm->moveObjectToNowhere(321);
	switch (mazeRoomNumber) {
	case 3:
	case 10:
	case 31:
		_r25_holeIndex = -1;
		break;
	case 23:
		_vm->moveObjectToRoom(314, 25);
		_r25_holeIndex = 0;
		break;
	case 17:
		_vm->moveObjectToRoom(315, 25);
		_r25_holeIndex = 1;
		break;
	case 47:
		_vm->moveObjectToRoom(317, 26);
		_r25_holeIndex = 2;
		break;
	case 20:
		_vm->moveObjectToRoom(318, 26);
		_r25_holeIndex = 3;
		break;
	case 11:
		_vm->moveObjectToRoom(320, 27);
		_r25_holeIndex = 4;
		break;
	case 40:
		_vm->moveObjectToRoom(321, 27);
		_r25_holeIndex = 5;
		break;
	}
}

int GameLogic::r28_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit28:
		_vm->changeRoom(24);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r28_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 150;
	replyTextY = 10;
	switch (_vm->_selectedDialogChoice) {
	case 18: case 19: case 20: case 21:
		replyTextIndex1 = 35;
		_vm->setDialogChoices(22, 23, 24, 25, 26);
		continueDialog = true;
		break;
	case 22: case 23: case 24: case 25: case 26:
		replyTextIndex1 = 36;
		replyTextIndex2 = 37;
		_vm->_roomEventNum = 2;
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r28_refreshRoomBackground() {
	if (!(_r20_flags & 0x01)) {
		_vm->_roomEventNum = 1;
	}
	if (_r20_flags & 0x02) {
		_vm->drawRoomImageToBackground("repxit10", 0, 33);
	}
}

void GameLogic::r28_handleRoomEvent1() {
	_r20_flags |= 0x01;
	if (_vm->_currentActorNum != 0) {
		_vm->walkTo(172, 136, 7, 142, 136);
	} else {
		_vm->walkTo(142, 136, 7, 172, 136);
	}
	for (int textIndex = 21; textIndex < 30; textIndex++) {
		_vm->displayText("c04r", textIndex, 0, 150, 10, 1);
		_vm->playAnimation("reptalk", 0, 6, 26, 51, 0, 150);
		_vm->playAnimation("reptalk", 0, 6, 26, 51, 1, 150);
		_vm->waitSeconds(2);
		_vm->_isTextVisible = false;
		_vm->refreshActors();
	}
	_vm->playAnimation("scepter", 1, 4, 26, 42, 0, 150);
	_vm->playSound("ss13", 0);
	_vm->playAnimation("zapwg", 0, 11, 107, 53, 0, 70);
	_vm->playSound("sv08", 1);
	_vm->playAnimation("scepter", 3, -4, 26, 42, 0, 150);
	_vm->displayText("c04r", 30, 0, 150, 10, 0);
	_vm->playAnimation("reptalk", 0, 6, 26, 51, 0, 150);
	_vm->playAnimation("reptalk", 0, 6, 26, 51, 1, 150);
	_vm->waitSeconds(2);
	_vm->_isTextVisible = false;
	_vm->refreshActors();
	_vm->playAnimation("crysgun", 0, 4, 0, 0, 0, 200);
	_vm->playAnimation("crysgun", 2, -3, 0, 0, 0, 200);
	_vm->moveObjectToRoom(kObjectIdInventorySquirtGun, 99);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	for (int textIndex = 31; textIndex < 35; textIndex++) {
		_vm->displayText("c04r", textIndex, 0, 150, 10, 0);
		_vm->playAnimation("reptalk", 0, 6, 26, 51, 0, 150);
		_vm->playAnimation("reptalk", 0, 6, 26, 51, 1, 150);
		_vm->waitSeconds(2);
		_vm->_isTextVisible = false;
		_vm->refreshActors();
	}
	_vm->setDialogChoices(18, 19, 20, 21, -1);
	_vm->startDialog();
}

void GameLogic::r28_handleRoomEvent2() {
	_vm->playSound("ss12", 0);
	_vm->playAnimation("scepter", 1, 4, 26, 42, 0, 150);
	_vm->playAnimation("repxit", 0, 11, 0, 33, 0, 100);
	_vm->waitSeconds(2);
	_vm->playSound("ss12", 0);
	_vm->playAnimation("repxit", 10, -11, 0, 33, 0, 100);
	_vm->playAnimation("scepter", 3, -4, 26, 42, 0, 150);
	_vm->displayText("c04r", 38, 0, 150, 10, 0);
	_vm->playAnimation("reptalk", 0, 6, 26, 51, 1, 150);
	_vm->waitSeconds(2);
	_vm->_isTextVisible = false;
	_vm->refreshActors();
	_vm->playSound("ss12", 0);
	_vm->playAnimation("scepter", 1, 4, 26, 42, 0, 150);
	_vm->playAnimation("repxit", 0, 11, 0, 33, 0, 100);
	_vm->waitSeconds(2);
	_r20_flags |= 0x02;
	_vm->moveObjectToNowhere(kObjectIdGiantLizard);
	_vm->displayText("c04", 27, 0, -1, -1, 0);
	if (_vm->_currentActorNum != 0) {
		_vm->_currentActorNum = 0;
	} else {
		_vm->_currentActorNum = 1;
	}
	_vm->waitSeconds(2);
	_vm->_isTextVisible = false;
	_vm->refreshActors();
	_vm->displayText("c04", 28, 0, -1, -1, 0);
	if (_vm->_currentActorNum != 0) {
		_vm->_currentActorNum = 0;
	} else {
		_vm->_currentActorNum = 1;
	}
	_vm->waitSeconds(1);
	_vm->_isTextVisible = false;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

int GameLogic::r29_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdBeakerOfAcid:
		_r29_flags |= 0x04;
		_vm->pickupObject(kObjectIdBeakerOfAcid, _r29_flags, 1, kObjectIdInventoryBeakerOfAcid);
		_r29_flags &= ~0x04;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r29_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit29:
		r29_uninitRoomAnimations();
		CursorMan.showMouse(false);
		_vm->paletteFadeOut(0, 256, 8);
		_vm->_screen->clear(0);
		_vm->drawInterface(_vm->_verbNumber);
		_vm->loadPalette("m01/wstand0");
		_vm->changeRoom(24);
		CursorMan.showMouse(true);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r29_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "coil", 10);
	_vm->loadAnimationSpriteRange(10, "torch", 4);
	_vm->loadAnimationSpriteRange(14, "machine", 4);
	_vm->startRoomAnimations();
}

void GameLogic::r29_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r29_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 5000;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 5000;
	}
	if (_vm->_animationsCtr % 500 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 500, 134, 0);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr % 200 == 150 || doUpdate) {
		_vm->drawAnimationSprite(10 + _vm->getRandom(4), 0, 16);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr % 500 == 350 || doUpdate) {
		_vm->drawAnimationSprite(14 + _vm->getRandom(4), 172, 46);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r29_refreshRoomBackground() {
	if (!(_r29_flags & 0x04)) {
		_vm->paletteFadeOut(0, 256, 8);
		_vm->_screen->clear(0);
		_vm->drawInterface(_vm->_verbNumber);
		_vm->loadPalette("r29/stool");
		_vm->_roomEventNum = 1;
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r29_initRoomAnimations();
	}
	if (!(_r29_flags & 0x01)) {
		_vm->drawRoomImageToBackground("acid", 90, 81);
	}
	_vm->drawRoomImageToBackground("doc1", 138, 69);
}

void GameLogic::r29_handleRoomEvent() {
	for (int i = 0; i < 4; i++) {
		_vm->playAnimation("doc", 0, 2, 138, 69, 0, 100);
	}
	_vm->displayTextLines("c04r", 365, 200, 30, 1);
}

int GameLogic::r30_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit30:
	case kObjectIdStage30:
		actionTextIndex = 14;
		break;
	case kObjectIdCamera30_0:
	case kObjectIdCamera30_1:
		actionTextIndex = 48;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r30_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit30:
		_vm->setWaynePosition(208, 75);
		_vm->setGarthPosition(203, 72);
		_vm->changeRoom(15);
		break;
	case kObjectIdCamera30_0:
	case kObjectIdCamera30_1:
		actionTextIndex = 50;
		break;
	case kObjectIdStage30:
		actionTextIndex = 51;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r30_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 150;
	replyTextY = 40;
	switch (_vm->_selectedDialogChoice) {
	case 343:
		replyTextIndex1 = 367;
		_vm->setDialogChoices(347, 348, 349, -1, -1);
		continueDialog = true;
		break;
	case 344:
		replyTextIndex1 = 368;
		_vm->setDialogChoices(350, 351, 352, 353, -1);
		continueDialog = true;
		break;
	case 345:
		replyTextIndex1 = 369;
		replyTextIndex2 = 370;
		_vm->setDialogChoices(354, 355, 356, 357, -1);
		continueDialog = true;
		break;
	case 346:
		replyTextIndex1 = 371;
		continueDialog = true;
		break;
	case 347: case 348: case 349:
		replyTextIndex1 = _vm->_selectedDialogChoice + 25;
		_vm->setDialogChoices(343, 344, 345, 346, -1);
		continueDialog = true;
		break;
	case 350: case 351: case 352: case 353:
		replyTextIndex1 = 375;
		_vm->setDialogChoices(343, 344, 345, 346, -1);
		continueDialog = true;
		break;
	case 354:
		replyTextIndex1 = 376;
		replyTextIndex2 = 377;
		continueDialog = true;
		break;
	case 355:
		replyTextIndex1 = 378;
		_vm->setDialogChoices(358, 359, 360, -1, -1);
		continueDialog = true;
		break;
	case 356: case 357:
		replyTextIndex1 = _vm->_selectedDialogChoice + 23;
		continueDialog = true;
		break;
	case 358:
		replyTextIndex1 = 381;
		_vm->setDialogChoices(361, 362, 363, 52, -1);
		continueDialog = true;
		break;
	case 359:
		replyTextIndex1 = 382;
		continueDialog = true;
		break;
	case 360:
		replyTextIndex1 = 383;
		replyTextIndex2 = 384;
		continueDialog = true;
		break;
	case 52: case 361: case 362: case 363:
		_vm->displayTextLines("c04r", 385, 160, 40, 6);
		_vm->displayTextLines("c04", 364, -1, -1, 1);
		_vm->_roomEventNum = 2;
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r30_refreshRoomBackground() {
	if (_r30_flags & 0x10) {
		_vm->drawRoomImageToBackground("cbackg", 0, 0);
		_vm->drawRoomImageToBackgroundTransparent("cecil", 177, 102);
	} else if (!(_r30_flags & 0x02)) {
		_vm->drawRoomImageToBackground("babeshow", 105, 12);
		_vm->_roomEventNum = 1;
	} else if (!(_r30_flags & 0x01)) {
		_vm->drawRoomImageToBackground("babeshow", 105, 12);
		if (!(_r30_flags & 0x04)) {
			_vm->drawRoomImageToBackground("waynecon", 224, 30);
		} else {
			_vm->drawRoomImageToBackground("nobink", 160, 33);
		}
	} else if (!(_r30_flags & 0x08) && _vm->_roomChangeCtr > 20) {
		r30_runLudwigCutscene();
	}
}

void GameLogic::r30_handleRoomEvent1() {
	_r30_flags |= 0x02;
	_vm->displayTextLines("c04r", 51, 150, 10, 23);
	if (_vm->_currentActorNum == 0) {
		_vm->_currentActorNum = 1;
		_vm->drawInterface(_vm->_verbNumber);
	}
	_vm->walkTo(237, 51, 6, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->waitSeconds(1);
	_vm->_garthSpriteY = -1;
	_vm->_garthSpriteX = -1;
	_vm->_wayneSpriteY = -1;
	_vm->_wayneSpriteX = -1;
	_vm->changeRoom(31);
}

void GameLogic::r30_handleRoomEvent2() {
	_r30_flags |= 0x08;
	_vm->setWaynePosition(208, 75);
	_vm->setGarthPosition(203, 72);
	_vm->drawInterface(_vm->_verbNumber);
	_vm->changeRoom(15);
}

void GameLogic::r30_runLudwigCutscene() {
	_vm->_garthSpriteY = -1;
	_vm->_garthSpriteX = -1;
	_vm->_wayneSpriteY = -1;
	_vm->_wayneSpriteX = -1;
	_vm->_roomName = "m06"; // Change the active gxl file
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	// TODO gxGetDisplayPalette(palette1);
	// TODO pcxGetLibPalette(8, roomLib, "backg", palette2);
	// TODO gxSetDisplayPalette(palette2);
	_vm->paletteFadeOut(0, 256, 64);
	_vm->_screen->clear(0);
	_vm->drawRoomImageToBackground("backg", 0, 0);
	_vm->_screen->drawSurface(_vm->_backgroundSurface, 0, 0);
	_vm->_musicIndex = 1;
	_vm->changeMusic();
	_vm->paletteFadeIn(0, 256, 3);
	_vm->displayTextLines("lws", 0, 50, 10, 1);
	_vm->playAnimation("ludwig", 1, 2, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 1, 50, 10, 1);
	_vm->playAnimation("ludwig", 3, 3, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 2, 50, 10, 1);
	_vm->playAnimation("ludwig", 6, 3, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 3, 50, 10, 1);
	_vm->playAnimation("ludwig", 9, 3, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 4, 50, 10, 3);
	_vm->playAnimation("ludwig", 12, 1, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 7, 50, 10, 6);
	_vm->playAnimation("ludwig", 13, 1, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 13, 50, 10, 4);
	for (int animIndex = 14, textIndex = 17; animIndex < 42; animIndex++) {
		_vm->playAnimation("ludwig", animIndex, 1, 185, 34, 0, 150);
		if (animIndex % 3 == 0) {
			_vm->displayTextLines("lws", textIndex++, 50, 10, 1);
		}
	}
	_vm->displayTextLines("lws", 26, 50, 10, 1);
	_vm->playAnimation("ludwig", 42, 20, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 27, 50, 10, 1);
	_vm->playAnimation("ludwig", 62, 4, 185, 34, 0, 150);
	_vm->playSound("ss17", 0);
	_vm->playAnimation("ludwig", 66, 4, 186, 22, 0, 150);
	_vm->playAnimation("ludwig", 70, 3, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 28, 50, 10, 5);
	_vm->playAnimation("ludwig", 73, 6, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 33, 50, 10, 4);
	_vm->playAnimation("ludwig", 79, 2, 185, 34, 0, 150);
	_vm->displayTextLines("lws", 37, 50, 10, 1);
	_vm->playAnimation("ludwig", 81, 6, 185, 34, 0, 150);
	_vm->playAnimation("ludwig", 87, 5, 174, 35, 0, 150);
	_vm->waitSeconds(2);
	_vm->_screen->clear(0);
	_r30_flags |= 0x08;
	r30_talkToCecil();
}

void GameLogic::r30_talkToCecil() {
	_r30_flags |= 0x10;
	_vm->setWaynePosition(210, 146);
	_vm->setGarthPosition(238, 142);
	_vm->_actorSpriteValue = 6;
	_vm->changeRoom(30);
	_r30_flags &= ~0x10;
	_vm->displayTextLines("c04r", 366, 160, 40, 1);
	_vm->setDialogChoices(343, 344, 345, 346, -1);
	_vm->startDialog();
	_r30_flags &= ~0x10;
}

int GameLogic::r31_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdBuzzer:
		r31_useBuzzer();
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r31_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 150;
	replyTextY = 10;
	switch (_vm->_selectedDialogChoice) {
	case 52:
		_vm->_gameState = 0;
		_vm->_roomEventNum = 2;
		break;
	case 53:
		_vm->_roomEventNum = 3;
		break;
	case 133:
		_vm->displayTextLines("gms", (_r31_categoryIndex * 5) + _r31_questionIndex, 50, 10, -6000);
		break;
	case 160: case 161: case 162: case 163:
		_vm->_roomEventNum = 5;
		_vm->_gameState = 0;
		break;
	default:
		if (_vm->_selectedDialogChoice > 56 && _vm->_selectedDialogChoice < 133) {
			if (_vm->_selectedDialogChoice == _r31_correctAnswerChoice) {
				r31_correctAnswerSelected();
			} else {
				r31_wrongAnswerSelected();
				_vm->_gameState = 0;
			}
			_vm->_roomEventNum = 4;
		} else if (_vm->_selectedDialogChoice > 133 && _vm->_selectedDialogChoice < 159) {
			_r31_questionIndex = (_vm->_selectedDialogChoice - 134) % 5;
			_r31_categoryIndex = (_vm->_selectedDialogChoice - 134) / 5;
			_vm->_roomEventNum = 4;
			_vm->_gameState = 0;
		}
		break;
	}
	return continueDialog;
}

void GameLogic::r31_refreshRoomBackground() {
	if (!(_r31_flags & 0x02)) {
		_vm->_roomEventNum = 1;
	}
}

void GameLogic::r31_handleRoomEvent1() {
	_r31_flags |= 0x02;
	_vm->redrawInventory();
	_vm->displayText("c04r", 74, 0, 50, 10, 0);
	r31_playTalkAnim(3);
	r31_drawMBuzzer();
	r31_drawDBuzzer();
}

void GameLogic::r31_handleRoomEvent2() {
	for (int textIndex = 81; textIndex < 83; textIndex++) {
		_vm->displayText("c04r", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	r31_displayCategories();
	_vm->displayText("c04r", 83, 0, 50, 10, 0);
	r31_playTalkAnim(3);
	r31_handleRoomEvent4();
}

void GameLogic::r31_handleRoomEvent3() {
	for (int textIndex = 75; textIndex < 81; textIndex++) {
		_vm->displayText("c04r", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	_vm->setDialogChoices(52, 53, -1, -1, -1);
	_vm->startDialog();
}

void GameLogic::r31_handleRoomEvent4() {

	while (_r31_questionsAsked < 25) {
		if (_r31_currentPlayer != 0 && !_r31_categorySelected) {
			_r31_questionIndex = 5;
			while (_r31_questionIndex == 5) {
				_r31_categoryIndex = _vm->getRandom(5);
				for (int questionIndex = 0; questionIndex < 5; questionIndex++) {
					if (!_r31_askedQuestions[_r31_categoryIndex * 5 + questionIndex]) {
						_r31_questionIndex = questionIndex;
						break;
					}
				}
			}
			_vm->displayText("c04", _r31_categoryIndex * 5 + _r31_questionIndex + 134, 0, 250, 10, 0);
			r31_playTalkAnim(_r31_currentPlayer);
		} else if (!_r31_categorySelected) {
			int questionChoices[5];
			for (int categoryIndex = 0; categoryIndex < 5; categoryIndex++) {
				questionChoices[categoryIndex] = 159;
				for (int questionIndex = 0; questionIndex < 5; questionIndex++) {
					if (!_r31_askedQuestions[categoryIndex * 5 + questionIndex]) {
						questionChoices[categoryIndex] = categoryIndex * 5 + questionIndex + 134;
						break;
					}
				}
			}
			_vm->setDialogChoices(questionChoices[0], questionChoices[1], questionChoices[2], questionChoices[3], questionChoices[4]);
			_vm->startDialog();
			_vm->drawRoomImageToBackground("dnorm", 287, 30);
			_vm->drawRoomImageToBackground("mtalk0", 240, 31);
			_r31_categorySelected = true;
			return;
		}

		_r31_categorySelected = false;
		_r31_askedQuestions[_r31_categoryIndex * 5 + _r31_questionIndex] = true;
		_vm->drawRoomImageToBackground("star", kRoom31StarPositionsX[_r31_categoryIndex], kRoom31StarPositionsY[_r31_categoryIndex * 5 + _r31_questionIndex]);

        // Wait for mouse release
		//sysMouseDriver(5, 1);
		while (_vm->_mouseClickButtons != 0) {
			_vm->_mouseClickButtons = 0;
			//sysMouseDriver(5, 1);
			//sysMouseDriver(5, 2);
			//updateKeyInput();
		}

		_vm->displayText("gms", _r31_categoryIndex * 5 + _r31_questionIndex, 0, 50, 10, 0);
		_vm->playAnimation("rdcard", 0, 6, 155, 30, 0, 300);
		_vm->playAnimation("rdcard", 1, 5, 155, 30, 0, 300); // NOTE This had 6 as count in the original which is a bug
		_vm->playAnimation("rdcard", 5, -6, 155, 30, 0, 300);
		_r31_questionsAsked++;
		_vm->_isTextVisible = false;
		_vm->refreshActors();
		_vm->waitMillis(_vm->getRandom(1300) + 200);
		//sysMouseDriver(3);

		//sysMouseDriver(5, 1);
		if (_vm->_mouseClickButtons != 0 && _vm->_mouseX > 130 && _vm->_mouseX < 180 && _vm->_mouseY > 106 && _vm->_mouseY < 128) {
			_r31_currentPlayer = 0;
			r31_useBuzzer();
			_vm->drawRoomImageToBackground("dlook", 287, 30);
			_vm->drawRoomImageToBackground("mlook", 240, 31);
			r31_buildQuestionDialogChoices(_r31_categoryIndex, _r31_questionIndex);
			return;
		} else {
			_r31_currentPlayer = _vm->getRandom(2) + 1;
			if (_r31_currentPlayer == 1) {
				r31_drawMBuzzer();
			} else {
				r31_drawDBuzzer();
			}
			_vm->displayText("gms", _r31_categoryIndex * 5 + _r31_questionIndex + 25, 0, 250, 10, 0);
			r31_playTalkAnim(_r31_currentPlayer);
			r31_correctAnswerSelected();
		}
	}
	
	r31_runBabeoff();
}

void GameLogic::r31_handleRoomEvent5() {
	for (int textIndex = 72; textIndex < 74; textIndex++) {
		_vm->displayText("gms", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	_vm->_wayneSpriteX = -1;
	_vm->setGarthPosition(207, 143);
	_vm->changeRoom(30);
	_vm->displayTextLines("gms", 74, 50, 10, 9);
	_vm->playAnimation("wbink", 0, 16, 160, 21, 0, 100);
	_vm->displayTextLines("gms", 83, 50, 10, 1);
	_vm->playAnimation("lbink", 0, 6, 214, 21, 0, 120);
	_vm->moveObjectToRoom(kObjectIdInventoryTicketToRome, 99);
	_vm->refreshInventory(false);
	_vm->setWaynePosition(237, 51);
	_r30_flags |= 0x04;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->walkTo(230, 140, 0, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->displayText("c04", 164, 0, -1, -1, 0);
	_vm->waitSeconds(2);
	_vm->_currentActorNum = 0;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->displayText("c04", 165, 0, -1, -1, 0);
	_vm->_currentActorNum = 1;
	_vm->waitSeconds(2);
	_vm->_isTextVisible = false;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_r30_flags |= 0x01;
	_vm->_roomChangeCtr = 0;
	_vm->setWaynePosition(208, 75);
	_vm->setGarthPosition(203, 72);
	_vm->changeRoom(15);
}

void GameLogic::r31_drawDBuzzer() {
	_vm->drawRoomImageToScreen("dbuzzer", 247, 78);
	_vm->waitSeconds(2);
	_vm->_screen->drawSurface(_vm->_backgroundSurface, 0, 0);
}

void GameLogic::r31_drawMBuzzer() {
	_vm->drawRoomImageToScreen("mbuzzer", 210, 61);
	_vm->waitSeconds(2);
	_vm->_screen->drawSurface(_vm->_backgroundSurface, 0, 0);
}

void GameLogic::r31_playTalkAnim(int talkerIndex) {
	if (talkerIndex == 1) {
		_vm->playAnimation("mtalk", 1, 6, 240, 31, 0, 250);
		_vm->playAnimation("mtalk", 5, -6, 240, 31, 0, 250);
	} else if (talkerIndex == 2) {
		_vm->playAnimation("dtalk", 1, 3, 286, 33, 0, 300);
		_vm->playAnimation("dtalk", 2, -3, 286, 33, 0, 300);
	} else if (talkerIndex == 3) {
		_vm->playAnimation("btalk", 1, 5, 180, 30, 0, 250);
		_vm->playAnimation("btalk", 4, -5, 180, 30, 0, 250);
	}
	_vm->_isTextVisible = false;
	_vm->refreshActors();
}

void GameLogic::r31_useBuzzer() {
	_vm->drawRoomImageToScreen("buzzer", 131, 106);
	_vm->playSound("ss04", 1);
	_vm->_screen->drawSurface(_vm->_backgroundSurface, 0, 0);
	if (!(_r31_flags & 0x01)) {
		r31_handleRoomEvent3();
		_r31_flags |= 0x01;
	}
}

void GameLogic::r31_displayCategories() {
	CursorMan.showMouse(false);
	WWSurface *screenImage = _vm->loadRoomSurface("screen");
	// TODO fxSetDelay(0);
	// TODO fxSetGrain(4, 4, 1);
	// TODO fxSetEffect(8); // Spiral Effect
	// TODO fxVirtualDisplay(screenImage, 0, 0, 0, 0, 319, 199, 1);
	_vm->_screen->drawSurface(screenImage, 0, 0); // TODO Until effects drawing is done
	delete screenImage;
	for (int categoryIndex = 0; categoryIndex < 5; categoryIndex++) {
		Common::String categoryFilename = Common::String::format("cat%d", categoryIndex);
		WWSurface *categoryImage = _vm->loadRoomSurface(categoryFilename.c_str());
		// TODO fxSetGrain(1, 1, 1);
		// TODO fxSetEffect(5); // Random Effect
		// TODO fxVirtualDisplay(categoryImage, 0, 0, 17, 61, 300, 140, 0);
		_vm->_screen->drawSurface(categoryImage, 17, 61); // TODO Until effects drawing is done
		delete categoryImage;
		_vm->waitSeconds(3);
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->drawInterface(_vm->_verbNumber);
	CursorMan.showMouse(true);
}

void GameLogic::r31_runBabeoff() {
	_vm->_gameState = 0;
	_vm->drawInterface(_vm->_verbNumber);
	_vm->drawRoomImageToBackground("dnorm", 287, 30);
	_vm->drawRoomImageToBackground("mtalk0", 240, 31);
	for (int textIndex = 54; textIndex < 62; textIndex++) {
		_vm->displayText("gms", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	_vm->displayText("gms", 62, 0, 250, 10, 0);
	r31_playTalkAnim(1);
	for (int textIndex = 63; textIndex < 67; textIndex++) {
		_vm->displayText("gms", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	for (int textIndex = 67; textIndex < 69; textIndex++) {
		_vm->displayText("gms", textIndex, 0, 250, 20, 0);
		r31_playTalkAnim(2);
	}
	for (int textIndex = 69; textIndex < 72; textIndex++) {
		_vm->displayText("gms", textIndex, 0, 50, 10, 0);
		r31_playTalkAnim(3);
	}
	_vm->setDialogChoices(160, 161, 162, 163, -1);
	_vm->startDialog();
}

void GameLogic::r31_drawCurrentPlayerScore() {
	int numberX = kRoom31NumberPositionsX[_r31_currentPlayer];
	int numberY = kRoom31NumberPositionsY[_r31_currentPlayer];
	int score = _r31_scores[_r31_currentPlayer];
	if (score > 99) {
		Common::String numberFilename = Common::String::format("n%d", score / 100);
		_vm->drawRoomImageToBackground(numberFilename.c_str(), numberX, numberY);
	} else {
		_vm->_backgroundSurface->fillRect(numberX, numberY, numberX + 3, numberY + 6, 0);
	}
	if (score > 9) {
		Common::String numberFilename = Common::String::format("n%d", (score % 100) / 10);
		_vm->drawRoomImageToBackground(numberFilename.c_str(), numberX + 5, numberY);
	} else {
		_vm->_backgroundSurface->fillRect(numberX + 5, numberY, numberX + 8, numberY + 6, 0);
	}
	Common::String numberFilename = Common::String::format("n%d", score % 10);
	_vm->drawRoomImageToBackground(numberFilename.c_str(), numberX + 10, numberY);
	_vm->refreshActors();
}

void GameLogic::r31_buildQuestionDialogChoices(int categoryIndex, int questionIndex) {
	int answerShuffle[3];
	int answerChoices[3];
	answerShuffle[0] = _vm->getRandom(3);
	answerShuffle[1] = (_vm->getRandom(4) + answerShuffle[0] + 1) % 3;
	if (answerShuffle[0] + answerShuffle[1] == 1) {
		answerShuffle[2] = 2;
	} else if (answerShuffle[0] + answerShuffle[1] == 2) {
		answerShuffle[2] = 1;
	} else {
		answerShuffle[2] = 0;
	}
	for (int answerIndex = 0; answerIndex < 3; answerIndex++) {
		answerChoices[answerIndex] = answerShuffle[answerIndex] * 25 + categoryIndex * 5 + questionIndex + 57;
		if (answerShuffle[answerIndex] == 0) {
			_r31_correctAnswerChoice = answerChoices[answerIndex];
		}
	}
	_vm->setDialogChoices(133, answerChoices[0], answerChoices[1], answerChoices[2], 132);
}

void GameLogic::r31_correctAnswerSelected() {
	_vm->displayText("gms", _vm->getRandom(3) + 50, 3, 100, 10, 0);
	r31_playTalkAnim(3);
	_r31_scores[_r31_currentPlayer] += (_r31_questionIndex + 1) * 10;
	r31_drawCurrentPlayerScore();
}

void GameLogic::r31_wrongAnswerSelected() {
	_vm->drawRoomImageToBackground("dnorm", 287, 30);
	_vm->drawRoomImageToBackground("mtalk0", 240, 31);
	_vm->displayText("gms", 53, 0, 50, 10, 0);
	r31_playTalkAnim(3);
	_r31_scores[0] -= (_r31_questionIndex + 1) * 10;
	if (_r31_scores[0] < 0) {
		_r31_scores[0] = 0;
	}
	r31_drawCurrentPlayerScore();
	_r31_currentPlayer = _vm->getRandom(2) + 1;
	if (_r31_currentPlayer == 1) {
		r31_drawMBuzzer();
	} else {
		r31_drawDBuzzer();
	}
	_vm->displayText("gms", (_r31_categoryIndex * 5) + _r31_questionIndex + 25, 0, 250, 10, 0);
	r31_playTalkAnim(_r31_currentPlayer);
	r31_correctAnswerSelected();
}

int GameLogic::r32_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMemo:
		r32_pickUpMemo();
		break;
	case kObjectIdPlungers:
		_vm->pickupObject(kObjectIdPlungers, _r32_flags, 2, kObjectIdInventoryPlungers);
		break;
	case kObjectIdCharts:
		actionTextIndex = 31;
		break;
	case kObjectIdComputer:
		actionTextIndex = 36;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r32_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdElevator:
		_vm->setWaynePosition(149, 80);
		_vm->setGarthPosition(152, 87);
		if (_vm->_hasRoomAnimationCallback) {
			r32_uninitRoomAnimations();
		}
		_vm->changeRoom(11);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r32_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdJanitor:
		if (_r32_flags & 0x04) {
			_vm->displayTextLines("c04", 419, -1, -1, 1);
		} else {
			r32_uninitRoomAnimations();
			_vm->drawRoomImageToBackground("jtalk1", 262, 84);
			_vm->setDialogChoices(402, 403, 404, 405, 406);
		}
		break;
	}
}

int GameLogic::r32_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDoor32_0:
	case kObjectIdDoor32_1:
		actionTextIndex = 12;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r32_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 200;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 402: case 405:
		replyTextIndex1 = _vm->_selectedDialogChoice + 35;
		continueDialog = true;
		break;
	case 403:
		replyTextIndex1 = 438;
		continueDialog = true;
		_vm->setDialogChoices(407, 408, 409, 410, -1);
		break;
	case 404:
		replyTextIndex1 = 439;
		continueDialog = true;
		if (_vm->getObjectRoom(kObjectIdInventoryOldDonuts) == 99) {
			_vm->setDialogChoices(411, 416, 417, 418, -1);
		} else {
			_vm->setDialogChoices(415, 416, 417, 418, -1);
		}
		break;
	case 406:
		r32_initRoomAnimations();
		_vm->_gameState = 0;
		break;
	case 407: case 408: case 409: case 410: case 415:
		replyTextIndex1 = _vm->_selectedDialogChoice + 34;
		continueDialog = true;
		_vm->setDialogChoices(402, 403, 404, 405, 406);
		break;
	case 417: case 418:
		replyTextIndex1 = _vm->_selectedDialogChoice + 33;
		continueDialog = true;
		_vm->setDialogChoices(402, 403, 404, 405, 406);
		break;
	case 416:
		replyTextIndex1 = 442;
		continueDialog = true;
		_vm->setDialogChoices(402, 403, 404, 405, 406);
		break;
	case 411:
		replyTextIndex1 = 445;
		continueDialog = true;
		_vm->setDialogChoices(412, 413, 414, -1, -1);
		break;
	case 412:
		replyTextIndex1 = 446;
		continueDialog = true;
		break;
	case 413:
		replyTextIndex1 = 447;
		_vm->_roomEventNum = 1;
		_vm->_gameState = 0;
		break;
	case 414:
		replyTextIndex1 = 448;
		continueDialog = true;
		_vm->setDialogChoices(402, 403, 404, 405, 406);
		break;
	}
	return continueDialog;
}

void GameLogic::r32_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "jan", 3);
	_vm->loadAnimationSpriteRange(3, "jeat", 4);
	_vm->startRoomAnimations();
}

void GameLogic::r32_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r32_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 3600;
	if (_r32_flags & 0x04) {
		if (_vm->_animationsCtr % 900 == 0 || doUpdate) {
			_vm->drawAnimationSprite(3 + (_vm->_animationsCtr / 900), 262, 83);
			_vm->_hasRoomAnimationCallback = false;
			_vm->refreshActors();
			_vm->_hasRoomAnimationCallback = true;
		}
	} else {
		if (_vm->_animationsCtr % 400 == 0 || doUpdate) {
			_vm->drawAnimationSprite(_vm->getRandom(3), 263, 84);
			_vm->_hasRoomAnimationCallback = false;
			_vm->refreshActors();
			_vm->_hasRoomAnimationCallback = true;
		}
	}
}

void GameLogic::r32_refreshRoomBackground() {
	if (!(_pizzathonListFlags2 & 0x08)) {
		_vm->drawRoomImageToBackground("cdoor", 13, 65);
		_vm->fillRoomMaskArea(0, 0, 33, 149, 1);
	} else {
		_vm->moveObjectToNowhere(kObjectIdDoor32_0);
		_vm->moveObjectToRoom(kObjectIdOffice, 32);
		_vm->moveObjectToRoom(kObjectIdJanitor, 32);
		_vm->moveObjectToRoom(kObjectIdCart, 32);
		_vm->drawRoomImageToBackground("cart", 269, 82);
		if (_r32_flags & 0x02) {
			_vm->drawRoomImageToBackground("noplunge", 293, 94);
		}
		r32_initRoomAnimations();
	}
}

void GameLogic::r32_handleRoomEvent() {
	_vm->moveObjectToNowhere(kObjectIdInventoryOldDonuts);
	_vm->refreshInventory(true);
	_vm->moveObjectToRoom(kObjectIdPlungers, 32);
	_r32_flags |= 0x04;
	r32_initRoomAnimations();
}

void GameLogic::r32_pickUpMemo() {
	_vm->pickupObject(kObjectIdMemo, _r32_flags, 1, kObjectIdInventoryMemo);
	_vm->displayText("c04", 167, 0, -1, -1, 0);
	_vm->_isTextVisible = false;
	_vm->waitSeconds(3);
	_vm->refreshActors();
	_vm->displayText("c04", 168, 0, -1, -1, 0);
	_vm->_isTextVisible = false;
	_vm->waitSeconds(3);
	_vm->refreshActors();
	_vm->_currentActorNum = (_vm->_currentActorNum + 1) % 2;
	_vm->displayText("c04", 169, 0, -1, -1, 0);
	_vm->_isTextVisible = false;
	_vm->waitSeconds(3);
	_vm->_currentActorNum = (_vm->_currentActorNum + 1) % 2;
	_vm->refreshActors();
}

void GameLogic::r33_refreshRoomBackground() {
	if (_r32_flags & 0x01) {
		_vm->drawRoomImageToBackground("nomemo", 55, 100);
	}
}

int GameLogic::r34_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLunchBox:
		_r34_flags &= ~0x08;
		_vm->moveObjectToRoom(kObjectIdInventoryLunchBox, 99);
		_vm->refreshInventory(false);
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
		break;
	case kObjectIdPasscard:
		_r34_flags |= 0x01;
		_vm->moveObjectToRoom(kObjectIdInventoryPassCard, 99);
		_vm->refreshInventory(false);
		_vm->loadRoomBackground(_vm->_currentRoomNumber);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r34_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit34:
		if (_vm->_hasRoomAnimationCallback) {
			r34_uninitRoomAnimations();
		}
		_vm->changeRoom(24);
		break;
	case kObjectIdLocker:
		if (_vm->_firstObjectNumber == kObjectIdInventoryLunchBox) {
			_vm->moveObjectToNowhere(kObjectIdInventoryLunchBox);
			_r34_flags |= 0x08;
			_vm->refreshInventory(false);
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
		} else if (_vm->_firstObjectNumber == kObjectIdInventoryPassCard) {
			_vm->moveObjectToNowhere(kObjectIdInventoryPassCard);
			_r34_flags &= ~0x01;
			_vm->refreshInventory(false);
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r34_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMan34:
		_vm->playAnimation("mantalk", 0, 1, 198, 68, 0, 1);
		r34_uninitRoomAnimations();
		_vm->displayTextLines("c04r", 279, 275, 30, 1);
		if (_vm->getObjectRoom(kObjectIdInventoryLunchBox) == 99) {
			_vm->setDialogChoices(291, 292, 294, 295, -1);
		} else {
			_vm->setDialogChoices(291, 292, 293, 295, -1);
		}
		break;
	}
}

int GameLogic::r34_handleVerbOpen() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLocker:
		if (_r34_flags & 0x02) {
			actionTextIndex = 1;
		} else {
			_r34_flags |= 0x02;
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r34_handleVerbClose() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLocker:
		if (!(_r34_flags & 0x02)) {
			actionTextIndex = 1;
		} else {
			_r34_flags &= ~0x02;
			_vm->loadRoomBackground(_vm->_currentRoomNumber);
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

bool GameLogic::r34_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 275;
	replyTextY = 30;
	switch (_vm->_selectedDialogChoice) {
	case 291:
		replyTextIndex1 = 280;
		continueDialog = true;
		break;
	case 292:
		replyTextIndex1 = 281;
		continueDialog = true;
		break;
	case 293:
		replyTextIndex1 = 282;
		continueDialog = true;
		break;
	case 294:
		replyTextIndex1 = 283;
		_vm->moveObjectToRoom(kObjectIdLocker, 34);
		continueDialog = true;
		break;
	case 295:
		_vm->playAnimation("towel", 0, 1, 198, 68, 0, 1);
		r34_initRoomAnimations();
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r34_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "towel", 4);
	_vm->startRoomAnimations();
}

void GameLogic::r34_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r34_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 3000;
	if (_vm->_animationsCtr == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->getRandom(4), 198, 68);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r34_refreshRoomBackground() {
	if (!(_r34_flags & 0x01) && !_vm->_hasRoomAnimationCallback) {
		_vm->drawRoomImageToBackground("towel0", 198, 68);
		r34_initRoomAnimations();
	} else {
		_vm->moveObjectToNowhere(kObjectIdMan34);
		_vm->moveObjectToRoom(kObjectIdLocker, 34);
	}
	if (_r34_flags & 0x02) {
		_vm->drawRoomImageToBackground("openlckr", 95, 60);
		if (_r34_flags & 0x08) {
			_vm->drawRoomImageToBackground("lunchbox", 96, 87);
			_vm->moveObjectToRoom(kObjectIdLunchBox, 34);
		} else {
			_vm->moveObjectToNowhere(kObjectIdLunchBox);
		}
		if (!(_r34_flags & 0x01)) {
			_vm->drawRoomImageToBackground("passcard", 98, 74);
			_vm->moveObjectToRoom(kObjectIdPasscard, 34);
		} else {
			_vm->moveObjectToNowhere(kObjectIdPasscard);
		}
	}
}

int GameLogic::r35_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit35:
		_vm->changeRoom(24);
		break;
	case kObjectIdJailCells:
		if (_vm->_firstObjectNumber == kObjectIdInventorySetOfKeys) {
			if (_r35_flags & 0x04) {
				r35_useSetOfKeysWithJailCells();
			} else {
				_vm->displayTextLines("c04r", 329, 150, 20, 3);
			}
		}
		actionTextIndex = 0;
		break;
	default:
		actionTextIndex = 0;
		break;
	}	
	return actionTextIndex;
}

void GameLogic::r35_handleVerbTalkTo() {
	_vm->_dialogChoices[0] = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdCassandra35:
		r35_talkToCassandra();
		break;
	}
}

bool GameLogic::r35_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3) {
	bool continueDialog = false;
	replyTextIndex3 = -1;
	replyTextIndex2 = -1;
	replyTextIndex1 = -1;
	replyTextX = 150;
	replyTextY = 20;
	switch (_vm->_selectedDialogChoice) {
	case 328: case 329: case 330:
		for (int textIndex = 323; textIndex < 326; textIndex++) {
			for (int i = 0; i < 4; i++) {
				_vm->playAnimation("cass", 0, 2, 179, 68, 0, 100);
			}
			_vm->displayTextLines("c04r", textIndex, replyTextX, replyTextY, 1);
		}
		replyTextIndex1 = 326;
		replyTextIndex2 = 327;
		replyTextIndex3 = 328;
		if (_vm->getObjectRoom(kObjectIdInventory50000) == 99) {
			_vm->setDialogChoices(331, 333, 334, 335, 340);
		} else {
			_vm->setDialogChoices(331, 332, 334, 335, 340);
		}
		continueDialog = true;
		break;
	case 331:
		replyTextIndex1 = 329;
		replyTextIndex2 = 330;
		replyTextIndex3 = 331;
		continueDialog = true;
		break;
	case 332:
		replyTextIndex1 = 332;
		replyTextIndex2 = 333;
		replyTextIndex3 = 334;
		continueDialog = true;
		break;
	case 333:
		replyTextIndex1 = 335;
		replyTextIndex2 = 336;
		if (_vm->getObjectRoom(kObjectIdInventorySetOfKeys) == 99 && (_r38_flags & 0x01)) {
			_vm->setDialogChoices(336, 337, 338, -1, -1);
		} else {
			_vm->setDialogChoices(336, 337, -1, -1, -1);
		}
		continueDialog = true;
		break;
	case 334:
		replyTextIndex1 = 337;
		continueDialog = true;
		break;
	case 335:
		replyTextIndex1 = 338;
		continueDialog = true;
		break;
	case 336:
		replyTextIndex1 = 339;
		replyTextIndex2 = 340;
		continueDialog = true;
		break;
	case 337:
		replyTextIndex1 = 96;
		_vm->_gameState = 0;
		break;
	case 338:
		_r35_flags |= 0x04;
		replyTextIndex1 = 341;
		_vm->_gameState = 0;
		break;
	case 340:
		_vm->_gameState = 0;
		break;
	}
	return continueDialog;
}

void GameLogic::r35_refreshRoomBackground() {
	if (!(_r35_flags & 0x01)) {
		_vm->drawRoomImageToBackground("cass1", 179, 68);
	}
	_vm->_roomEventNum = 1;
}

void GameLogic::r35_talkToCassandra() {
	_vm->_gameState = 7;
	if (_r35_flags & 0x02) {
		for (int i = 0; i < 4; i++) {
			_vm->playAnimation("cass", 0, 2, 179, 68, 0, 100);
		}
		_vm->displayTextLines("c04r", 342, 150, 20, 1);
		if (_vm->getObjectRoom(kObjectIdInventory50000) == 99) {
			_vm->walkTo(217, 112, 5, 230, 115);
			_vm->displayTextLines("c04", 339, -1, -1, 1);
			for (int textIndex = 335; textIndex < 337; textIndex++) {
				for (int i = 0; i < 4; i++) {
					_vm->playAnimation("cass", 0, 2, 179, 68, 0, 100);
				}
				_vm->displayTextLines("c04r", textIndex, 150, 20, 1);
			}
			if (_vm->getObjectRoom(kObjectIdInventorySetOfKeys) == 99 && (_r38_flags & 0x01)) {
				_vm->setDialogChoices(336, 337, 338, -1, -1);
			} else {
				_vm->setDialogChoices(336, 337, -1, -1, -1);
			}
			_vm->startDialog();
		}
	} else {
		_r35_flags |= 0x02;
		for (int textIndex = 317; textIndex < 323; textIndex++) {
			for (int i = 0; i < 4; i++) {
				_vm->playAnimation("cass", 0, 2, 179, 68, 0, 100);
			}
			_vm->displayTextLines("c04r", textIndex, 150, 20, 1);
		}
		_vm->walkTo(217, 112, 5, 230, 115);
		_vm->setDialogChoices(328, 329, 330, -1, -1);
		_vm->startDialog();
	}
	_vm->_gameState = 2;
}

void GameLogic::r35_useSetOfKeysWithJailCells() {
	_vm->walkTo(179, 106, 4, _vm->_garthSpriteX, _vm->_garthSpriteY);
	_vm->waitSeconds(1);
	_vm->walkTo(217, 112, 5, 230, 115);
	_vm->playAnimation("cassgo", 0, 5, 166, 68, 0, 150);
	_r35_flags |= 0x01;
	_vm->moveObjectToNowhere(kObjectIdCassandra35);
	_vm->displayTextLines("c04r", 343, 150, 20, 1);
	_r24_mazeRoomNumber = 72;
	_r24_mazeHoleNumber = 5;
	_vm->changeRoom(38);
}

int GameLogic::r36_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLunchBox36:
		_vm->pickupObject(kObjectIdLunchBox36, _r36_flags, 4, kObjectIdInventoryLunchBox);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r36_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit36:
		r36_uninitRoomAnimations();
		_vm->changeRoom(24);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r36_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "sleep", 2);
	_vm->startRoomAnimations();
}

void GameLogic::r36_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r36_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 4000;
	if (_vm->_animationsCtr == 0 || doUpdate) {
		_vm->drawAnimationSprite(0, 263, 82);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	} else if (_vm->_animationsCtr == 2000 || doUpdate) {
		_vm->drawAnimationSprite(1, 263, 82);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r36_refreshRoomBackground() {
	if (_r36_flags & 0x04) {
		_vm->drawRoomImageToBackground("nolunch", 268, 101);
	}
	if (!_vm->_hasRoomAnimationCallback) {
		r36_initRoomAnimations();
	}
	_vm->_roomEventNum = 1;
}

void GameLogic::r36_handleRoomEvent() {
	if (!(_r36_flags & 0x01)) {
		_vm->displayTextLines("c04r", 260, 300, 30, 5);
		_r36_flags |= 0x01;
	} else if (!(_r36_flags & 0x02)) {
		_vm->displayTextLines("c04r", 265, 300, 30, 1);
		_r36_flags |= 0x02;
	} else {
		_vm->displayTextLines("c04r", 266, 300, 30, 13);
	}
}

int GameLogic::r37_handleVerbPickUp() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdMoney:
		_vm->pickupObject(kObjectIdMoney, _r37_flags, 16, kObjectIdInventory50000);
		actionTextIndex = 42;
		break;
	case kObjectIdKeys:
		_vm->pickupObject(kObjectIdKeys, _r37_flags, 8, kObjectIdInventorySetOfKeys);
		actionTextIndex = 43;
		break;
	case kObjectIdMagazines:
		if (_vm->_currentActorNum != 0) {
			actionTextIndex = 40;
		} else {
			actionTextIndex = 41;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r37_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLadder37_0:
		r37_useLadder1();
		_r37_flags |= 0x20;
		_vm->changeRoom(17);
		break;
	case kObjectIdLadder37_1:
		r37_useLadder2();
		break;
	case kObjectIdLadder:
		r37_useLadder3();
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r37_handleVerbPush() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLock_1:
		r37_pushPullLock(0);
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r37_handleVerbPull() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdLock_1:
		r37_pushPullLock(1);
		break;
	case kObjectIdHandle:
		if (!(_r37_flags & 0x02)) {
			actionTextIndex = 1;
		} else {
			r37_pullHandle();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r37_refreshRoomBackground() {
	if (!(_r37_flags & 0x01)) {
		_vm->_wayneSpriteX = -1;
		_vm->_garthSpriteX = -1;
		_vm->_roomEventNum = 1;
	}
	if (!(_r37_flags & 0x04)) {
		Common::String lockFilename = Common::String::format("lock%d", _r37_safeCombinationLockIndex);
		_vm->drawRoomImageToBackground(lockFilename.c_str(), 109, 97);
	} else {
		_vm->drawRoomImageToBackground("opensafe", 92, 84);
		if (_r37_flags & 0x08) {
			_vm->drawRoomImageToBackground("nokeys", 101, 95);
		}
		if (_r37_flags & 0x10) {
			_vm->drawRoomImageToBackground("nomoney", 102, 102);
		}
	}
}

void GameLogic::r37_useLadder1() {
	Common::String tempFilename;
	WWSurface *workBackground;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *getldrSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

    for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetldr%d", index);
		getldrSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}
	
	workBackground = new WWSurface(320, 150);

	for (int index = 0; index < 3; index++) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(getldrSprites[index], 105, 24);
		workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 125, 21);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	for (int index = 0; index < 3; index++) {
		delete getldrSprites[index];
	}
	
	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("ggetldr%d", index);
		getldrSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	int wayneLadderY = 24;
	int climbCtr = 0, garthLadderY = wayneLadderY;
	while (wayneLadderY > -46) {
		if (climbCtr % 2 == 1) {
			wayneLadderY -= 7;
			if (climbCtr > 12) {
				garthLadderY -= 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], 105, wayneLadderY);
		if (climbCtr < 10) {
			workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 145, garthLadderY - 3);
		} else if (climbCtr > 12) {
			workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], 105, garthLadderY);
		} else {
			workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr - 10], 105, garthLadderY);
		}
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		delete getldrSprites[index];
	}

	delete workBackground;
}

void GameLogic::r37_useLadder2() {
	Common::String tempFilename;

	WWSurface *workBackground;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *wgetldlSprites[3];
	WWSurface *ggetldlSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetldl%d", index);
		wgetldlSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("ggetldl%d", index);
		ggetldlSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	workBackground = new WWSurface(320, 150);

	for (int index = 0; index < 3; index++) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(wgetldlSprites[index], 168, 26);
		workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 140, 26);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	int wayneLadderY = 26;
	int climbCtr = 0, garthLadderY = wayneLadderY;
	
	while (garthLadderY < 92) {
		if (climbCtr % 2 == 0) {
			wayneLadderY += 7;
			if (climbCtr > 14) {
				garthLadderY += 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		if (climbCtr < 12) {
			workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 135, garthLadderY);
		} else if (climbCtr <= 14) {
			workBackground->drawSurfaceTransparent(ggetldlSprites[climbCtr - 12], 168, 26);
		} else {
			workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], 187, garthLadderY);
		} 
		if (climbCtr < 20) {
			workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], 187, wayneLadderY);
		} else if (climbCtr <= 22) {
			workBackground->drawSurfaceTransparent(wgetldlSprites[22 - climbCtr], 168, 92);
		} else {
			workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 135, 94);
		}
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 2; index >= 0; index--) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(ggetldlSprites[index], 168, 90);
		workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 135, 94);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		delete wgetldlSprites[index];
		delete ggetldlSprites[index];
	}

	delete workBackground;

	_vm->setWaynePosition(135, 141);
	_vm->setGarthPosition(160, 137);
	_vm->moveObjectToNowhere(kObjectIdLadder37_0);
	_vm->moveObjectToNowhere(kObjectIdLadder37_1);
	if (!(_r37_flags & 0x04)) {
		_vm->moveObjectToRoom(kObjectIdSafe_1, 37);
		_vm->moveObjectToRoom(kObjectIdLock_1, 37);
		_vm->moveObjectToRoom(kObjectIdHandle, 37);
	} else {
		_vm->moveObjectToNowhere(kObjectIdSafe_1);
		_vm->moveObjectToNowhere(kObjectIdLock_1);
		_vm->moveObjectToNowhere(kObjectIdMagazines); // TODO CHECKME Another Turbo C++ compiler bug?
		_vm->moveObjectToNowhere(kObjectIdHandle);
		if (!(_r37_flags & 0x08)) {
			_vm->moveObjectToRoom(kObjectIdKeys, 37);
		}
		if (!(_r37_flags & 0x10)) {
			_vm->moveObjectToRoom(kObjectIdMoney, 37);
		}
	}
	_vm->moveObjectToRoom(kObjectIdLadder, 37);
	_vm->loadRoomMask(98);
	_r37_flags |= 0x01;
	_vm->refreshActors();
}

void GameLogic::r37_useLadder3() {
	Common::String tempFilename;
	WWSurface *workBackground;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *wgetldlSprites[3];
	WWSurface *ggetldlSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

    for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetldl%d", index);
		wgetldlSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("ggetldl%d", index);
		ggetldlSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	workBackground = new WWSurface(320, 150);

	for (int index = 0; index < 3; index++) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(wgetldlSprites[index], 168, 90);
		workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 145, 89);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	int climbCtr = 0, wayneLadderY = 92, garthLadderY = 92;
	while (garthLadderY > 26) {
		if (climbCtr % 2 == 1) {
			wayneLadderY -=7;
			if (climbCtr > 12) {
				garthLadderY -= 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		if (climbCtr < 10) {
			workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], 187, wayneLadderY);
		} else  if (climbCtr > 12) {
			workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 155, 26);
		} else {
			workBackground->drawSurfaceTransparent(wgetldlSprites[12 - climbCtr], 168, 26);
		}
		if (climbCtr < 10) {
			workBackground->drawSurfaceTransparent(_vm->_garthSprites[0], 145, garthLadderY - 3);
		} else if (climbCtr > 12) {
			workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], 187, garthLadderY);
		} else {
			workBackground->drawSurfaceTransparent(ggetldlSprites[climbCtr - 10], 168, 90);
		}
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 2; index >= 0; index--) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(ggetldlSprites[index], 168, 28);
		workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 155, 26);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);		
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		delete wgetldlSprites[index];
		delete ggetldlSprites[index];
	}

	delete workBackground;

	_vm->moveObjectToRoom(kObjectIdLadder37_0, 37);
	_vm->moveObjectToRoom(kObjectIdLadder37_1, 37);
	_vm->moveObjectToNowhere(kObjectIdSafe_1);
	_vm->moveObjectToNowhere(kObjectIdLock_1);
	_vm->moveObjectToNowhere(kObjectIdHandle);
	_vm->moveObjectToNowhere(kObjectIdLadder);
	_vm->moveObjectToNowhere(kObjectIdKeys);
	_vm->moveObjectToNowhere(kObjectIdMoney);
	_vm->moveObjectToNowhere(kObjectIdMagazines);
	_r37_flags &= ~0x01;
	_vm->loadRoomMask(_vm->_currentRoomNumber);
	_vm->setWaynePosition(156, 75);
	_vm->setGarthPosition(178, 75);
	_vm->refreshActors();
}

void GameLogic::r37_pushPullLock(bool isPull) {
	if (isPull) {
		if (_r37_word_35CEC == 0 ||
			(_r37_safeCombinationCurrentNumber == 0 && _r37_safeCombinationIndex == 0 && _r37_safeCombinationLockIndex != 0)) {
			_r37_flags &= ~0x02;
			_r37_safeCombinationIndex = 0;
			_r37_safeCombinationCurrentNumber = 0;
			_r37_word_35CEC = 0;
		} else {
			_r37_safeCombinationCurrentNumber = _r37_safeCombinationCurrentNumber + 1;
			if (kRoom37CorrectSafeCombination[_r37_safeCombinationIndex] == _r37_safeCombinationCurrentNumber) {
				_r37_safeCombinationIndex++;
				_r37_safeCombinationCurrentNumber = 0;
				_r37_word_35CEC = (_r37_word_35CEC + 1) % 2;
			}
		}
		_r37_safeCombinationLockIndex = (_r37_safeCombinationLockIndex + 1) % 8;
	} else {
		if (_r37_word_35CEC != 0 ||
			(_r37_safeCombinationCurrentNumber == 0 && _r37_safeCombinationIndex == 0 && _r37_safeCombinationLockIndex != 0)) {
			_r37_flags &= ~0x02;
			_r37_safeCombinationIndex = 0;
			_r37_safeCombinationCurrentNumber = 0;
			_r37_word_35CEC = 0;
		} else {
			_r37_safeCombinationCurrentNumber = _r37_safeCombinationCurrentNumber + 1;
			if (kRoom37CorrectSafeCombination[_r37_safeCombinationIndex] == _r37_safeCombinationCurrentNumber) {
				_r37_safeCombinationIndex++;
				_r37_safeCombinationCurrentNumber = 0;
				_r37_word_35CEC = (_r37_word_35CEC + 1) % 2;
			}
		}
		_r37_safeCombinationLockIndex = (_r37_safeCombinationLockIndex + 7) % 8;
	}
	if (_r37_safeCombinationIndex == 6) {
		_r37_safeCombinationCurrentNumber = 0;
		_r37_safeCombinationIndex = 0;
		_r37_flags |= 0x02;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r37_pullHandle() {
	_r37_flags |= 0x04;
	_vm->moveObjectToNowhere(kObjectIdSafe_1);
	_vm->moveObjectToNowhere(kObjectIdLock_1);
	_vm->moveObjectToNowhere(kObjectIdHandle);
	_vm->moveObjectToRoom(kObjectIdMagazines, 37);
	_vm->moveObjectToRoom(kObjectIdKeys, 37);
	_vm->moveObjectToRoom(kObjectIdMoney, 37);
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r37_handleRoomEvent() {
	Common::String tempFilename;
	WWSurface *workBackground;
	WWSurface *wclimbSprites[4];
	WWSurface *gclimbSprites[4];
	WWSurface *getldrSprites[3];

	for (int index = 0; index < 4; index++) {
		tempFilename = Common::String::format("wclimb%d", index);
		wclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
		tempFilename = Common::String::format("gclimb%d", index);
		gclimbSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("wgetldr%d", index);
		getldrSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	workBackground = new WWSurface(320, 150);

	int climbCtr = 0, wayneLadderY = -46, garthLadderY = -46;
	while (climbCtr < 24) {
		if (climbCtr % 2 == 0) {
			wayneLadderY += 7;
			if (climbCtr > 12) {
				garthLadderY += 7;
			}
		}
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(gclimbSprites[climbCtr % 4], 105, garthLadderY);
		if (climbCtr < 20) {
			workBackground->drawSurfaceTransparent(wclimbSprites[climbCtr % 4], 105, wayneLadderY);
		} else if (climbCtr > 22) {
			workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 151, 22);
		} else {
			workBackground->drawSurfaceTransparent(getldrSprites[22 - climbCtr], 105, 26);
		}
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
		climbCtr++;
	}

	for (int index = 0; index < 3; index++) {
		delete getldrSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		tempFilename = Common::String::format("ggetldr%d", index);
		getldrSprites[index] = _vm->loadRoomSurface(tempFilename.c_str());
	}

	for (int index = 2; index >= 0; index--) {
		workBackground->drawSurface(_vm->_backgroundSurface, 0, 0);
		workBackground->drawSurfaceTransparent(getldrSprites[index], 105, 26);
		workBackground->drawSurfaceTransparent(_vm->_wayneSprites[0], 151, 22);
		_vm->_screen->drawSurface(workBackground, 0, 0);
		_vm->waitMillis(200);
	}

	for (int index = 0; index < 4; index++) {
		delete wclimbSprites[index];
		delete gclimbSprites[index];
	}

	for (int index = 0; index < 3; index++) {
		delete getldrSprites[index];
	}

	delete workBackground;
	
	_vm->setWaynePosition(163, 70);
	_vm->setGarthPosition(138, 75);
	_vm->refreshActors();
}

int GameLogic::r38_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit38:
		if (_vm->_hasRoomAnimationCallback) {
			r38_uninitRoomAnimations();
		}
		_vm->changeRoom(24);
		break;
	case kObjectIdCassandra:
		if (_r38_flags & 0x02) {
			r38_useCassandra();
			actionTextIndex = -1;
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdEquipment:
		actionTextIndex = 76;
		break;
	case kObjectIdHypnoBoard:
		actionTextIndex = 77;
		break;
	case kObjectIdPipes:
		actionTextIndex = 7;
		break;
	case kObjectIdRag:
		actionTextIndex = 7;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r38_handleVerbPull() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdRag:
		if (!(_r38_flags & 0x04)) {
			actionTextIndex = 5;
		} else {
			r38_pullRag();
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r38_initRoomAnimations() {
	_vm->loadAnimationSpriteRange(0, "hypno", 20);
	_vm->startRoomAnimations();
}

void GameLogic::r38_uninitRoomAnimations() {
	_vm->stopRoomAnimations();
}

void GameLogic::r38_updateRoomAnimations(bool doUpdate) {
	_vm->_animationsCtr = (_vm->_animationsCtr + 1) % 8000;
	if (doUpdate) {
		_vm->_animationsCtr = (_vm->_animationsCtr + 125) % 8000;
	}
	if (_vm->_animationsCtr % 400 == 0 || doUpdate) {
		_vm->drawAnimationSprite(_vm->_animationsCtr / 400, 8, 38);
		_vm->_hasRoomAnimationCallback = false;
		_vm->refreshActors();
		_vm->_hasRoomAnimationCallback = true;
	}
}

void GameLogic::r38_refreshRoomBackground() {
	_r38_flags |= 0x01;
	if (!_vm->_hasRoomAnimationCallback != 0 || !(_r38_flags & 0x08)) {
		r38_initRoomAnimations();
	}
	if ((_r38_flags & 0x08)) {
		_vm->drawRoomImageToBackground("backg2", 0, 0);
		_vm->setStaticRoomObjectPosition(38, 2, 2, 166, 94);
	} else {
		if (_r35_flags & 0x01) {
			_vm->setStaticRoomObjectPosition(38, 1, 1, 220, 98);
			_vm->moveObjectToRoom(kObjectIdCassandra, 38);
		} else {
			_vm->setStaticRoomObjectPosition(38, 1, 1, -1, 72);
			_vm->moveObjectToNowhere(kObjectIdCassandra);
		}
	}
}

void GameLogic::r38_atrap() {
	_vm->walkTo(153, 137, 0, 189, 137);
	_vm->_wayneSpriteX = -1;
	_vm->_garthSpriteX = -1;
	_vm->playAnimation("atrap", 0, 12, 128, 56, 0, 100);
	_r38_flags |= 0x02;
}

void GameLogic::r38_useCassandra() {
	_vm->moveObjectToNowhere(kObjectIdCassandra);
	_vm->setStaticRoomObjectPosition(38, 1, 1, -1, 72);
	_vm->drawRoomImageToScreen("nocass", 168, 76);
	_vm->drawRoomImageToBackground("nocass", 168, 76);
	_vm->playAnimation("btrap", 0, 25, 124, 15, 0, 100);
	_r38_flags |= 0x04;
}

void GameLogic::r38_pullRag() {
	_vm->moveObjectToNowhere(kObjectIdRag);
	_vm->playAnimation("apulrg", 0, 28, 106, 10, 0, 50);
	r38_uninitRoomAnimations();
	for (int i = 0; i < 21; i++) {
		_vm->playAnimation("bpulrg", i, 1, 105, 10, 0, 1);
		_vm->playAnimation("noo", i, 1, 8, 38, 0, 50);
	}
	_vm->playAnimation("aendit", 0, 10, 0, 16, 0, 50);
	_vm->playAnimation("bendit", 0, 4, 0, 33, 0, 50);
	_r38_flags |= 0x08;
	_vm->setStaticRoomObjectPosition(38, 2, 2, 166, 94);
	_vm->setWaynePosition(126, 137);
	_vm->setGarthPosition(209, 132);
	_vm->_actorSpriteValue = 5;
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->displayTextLines("c04r", 344, 180, 35, 5);
}

int GameLogic::r39_handleVerbUse() {
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdExit39:
		r39_useExit39();
		break;
	case kObjectIdExit:
		r39_useExit();
		break;
	case kObjectIdHinges:
		if (_vm->_firstObjectNumber == kObjectIdInventorySquirtGun) {
			if (_r39_flags & 0x01) {
				r39_useSquirtGunWithHinges();
			} else {
				actionTextIndex = 64;
			}
		} else if (_vm->_firstObjectNumber == kObjectIdInventoryBeakerOfAcid) {
			actionTextIndex = 65;
		} else {
			actionTextIndex = 0;
		}
		break;
	case kObjectIdLocks:
		if (_vm->_firstObjectNumber == kObjectIdInventorySquirtGun) {
			if (_r39_flags & 0x01) {
				actionTextIndex = 63;
			} else {
				actionTextIndex = 64;
			}
		} else if (_vm->_firstObjectNumber == kObjectIdInventoryBeakerOfAcid) {
			actionTextIndex = 63;
		} else {
			actionTextIndex = 0;
		}
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

int GameLogic::r39_handleVerbOpen() {	
	int actionTextIndex = -1;
	switch (_vm->_objectNumber) {
	case kObjectIdDoor39:
		actionTextIndex = 4;
		break;
	default:
		actionTextIndex = 0;
		break;
	}
	return actionTextIndex;
}

void GameLogic::r39_refreshRoomBackground() {
	if (_r39_flags & 0x02) {
		_vm->drawRoomImageToBackground("shoot13", 108, 49);
	}
	if (_r39_flags & 0x04) {
		_vm->drawRoomImageToBackground("wshoot", 87, 90);
	} else  if (_r39_flags & 0x08) {
		_vm->drawRoomImageToBackground("gshoot", 88, 90);
	}
}

void GameLogic::r39_useSquirtGunWithHinges() {
	int oldSpriteX;
	if (_vm->_currentActorNum != 0) {
		_vm->walkTo(_vm->_wayneSpriteX - 1, _vm->_wayneSpriteY, _vm->_actorSpriteValue, 60, 135);
		oldSpriteX = _vm->_wayneSpriteX;
		_vm->_wayneSpriteX = -1;
		_r39_flags |= 0x04;
	} else {
		_vm->walkTo(_vm->_garthSpriteX - 1, _vm->_garthSpriteY, _vm->_actorSpriteValue, 60, 135);
		oldSpriteX = _vm->_garthSpriteX;
		_vm->_garthSpriteX = -1;
		_r39_flags |= 0x08;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
	_vm->playAnimation("shoot", 0, 14, 108, 49, 0, 100);
	_r39_flags |= 0x02;
	_vm->moveObjectToNowhere(kObjectIdDoor39);
	_vm->moveObjectToNowhere(kObjectIdHinges);
	_vm->moveObjectToNowhere(kObjectIdLocks);
	_vm->moveObjectToRoom(kObjectIdExit, 39);
	_r39_flags &= ~0x04;
	_r39_flags &= ~0x08;
	if (_vm->_currentActorNum != 0) {
		_vm->_wayneSpriteX = oldSpriteX;
	} else {
		_vm->_garthSpriteX = oldSpriteX;
	}
	_vm->loadRoomBackground(_vm->_currentRoomNumber);
}

void GameLogic::r39_useExit39() {
	_r24_mazeHoleNumber = 2;
	_r24_mazeRoomNumber = 32;
	_vm->changeRoom(24);
}

void GameLogic::r39_useExit() {
	_r24_mazeHoleNumber = 4;
	_r24_mazeRoomNumber = 49;
	_vm->changeRoom(24);
}

} // End of namespace WaynesWorld

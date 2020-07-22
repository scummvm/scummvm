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

#ifndef WAYNESWORLD_GAMELOGIC_H
#define WAYNESWORLD_GAMELOGIC_H

#include "waynesworld/waynesworld.h"

namespace WaynesWorld {

class GameLogic {
public:
	GameLogic(WaynesWorldEngine *vm);
	~GameLogic();
	void initVariables();
	int handleVerbPickUp();
	int handleVerbUse();
	void handleVerbTalkTo();
	int handleVerbPush();
	int handleVerbPull();
	int handleVerbOpen();
	int handleVerbClose();
	void handleVerbExtremeCloseupOf();
	int handleVerbGive();
	void handleDialogReply(int index, int x, int y);
	bool handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void refreshRoomBackground(int roomNum);
	void updateRoomAnimations(bool doUpdate);
	void handleRoomEvent(int eventNum);
	int getActorScaleFromY(int actorY);
	void r38_atrap();
public:
	int _word_34464;
	int _word_34466;
	int _r37_safeCombinationLockIndex;
	int _r37_word_35CEC;
	int _r37_safeCombinationIndex;
	int _r37_safeCombinationCurrentNumber;
	int _r12_talkObjectNumber;
	byte _pizzathonListFlags1;
	byte _pizzathonListFlags2;
	byte _r31_flags;
	byte _r0_flags;
	byte _r4_flags;
	byte _r5_flags;
	byte _r7_flags;
	byte _r11_flags;
	byte _r32_flags;
	byte _r1_flags1;
	byte _r1_flags2;
	byte _r2_flags;
	byte _r6_flags;
	byte _r10_flags;
	byte _r12_flags;
	byte _r19_flags;
	byte _r9_flags;
	byte _r8_flags;
	byte _r13_flags;
	byte _r20_flags;
	byte _r29_flags;
	byte _r30_flags;
	byte _r34_flags;
	byte _r35_flags;
	byte _r37_flags;
	byte _r36_flags;
	byte _r38_flags;
	byte _r39_flags;
	int _r10_selectedItemToBuy;
	byte _r9_dialogFlag;
	byte _r1_eventFlag;
	byte _r1_eventCtr;
	byte _r17_dialogCtr;
	byte _r17_eventFlag;
	byte _r2_backgroundValue1;
	byte _r2_backgroundValue2;
	int _r7_cloudsPositionX;
	int _r19_wayneSpriteX;
	int _r19_garthSpriteX;
	int _r24_mazeRoomNumber;
	int _r24_mazeHoleNumber;
	int _r25_holeIndex;
	int _r31_questionsAsked;
	int _r31_correctAnswerChoice;
	int _r31_categoryIndex;
	int _r31_questionIndex;
	bool _r31_categorySelected;
	int _r31_currentPlayer;
	int _r31_scores[3];
	bool _r31_askedQuestions[25];
	bool _didScratchTicket;
// protected:
public://DEBUG So logic functions can be called from the engine class for testing
	WaynesWorldEngine *_vm;

	void displayExtremeCloseupOfPizzathonList();
	void displayExtremeCloseupOfSign15();
	void displayExtremeCloseupOfObjectPictures22();
	void displayExtremeCloseupOfMemo();
	void displayExtremeCloseupOfSewerMap();
	void displayExtremeCloseupOfBillboard1();
	void displayExtremeCloseupOfBillboard2();

	int r0_handleVerbPickUp();
	int r0_handleVerbUse();
	int r0_handleVerbOpen();
	bool r0_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r0_refreshRoomBackground();
	void r0_handleRoomEvent1();
	void r0_handleRoomEvent2();
	void r0_handleRoomEvent3();
	void r0_buildRandomDialogChoices(int selectedDialogChoice);

	int r1_handleVerbPickUp();
	int r1_handleVerbUse();
	void r1_handleVerbTalkTo();
	int r1_handleVerbPush();
	int r1_handleVerbPull();
	int r1_handleVerbOpen();
	int r1_handleVerbClose();
	bool r1_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r1_refreshRoomBackground();
	void r1_handleRoomEvent(bool arg6);
	void r1_initRoomAnimations();
	void r1_uninitRoomAnimations();
	void r1_updateRoomAnimations(bool doUpdate);
	void r1_useDayOldDonutWithMachine();
	void r1_checkDrGadget();
	void r1_pullScreen();
	void r1_pullLever();
	void r1_openSafe();
	void r1_closeSafe();
	void r1_drGadgetLeaves();

	int r2_handleVerbPickUp();
	int r2_handleVerbUse();
	void r2_handleVerbTalkTo();
	int r2_handleVerbGive();
	bool r2_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r2_refreshRoomBackground();
	void r2_giveCandyBarToBikerGang();
	void r2_handleDialogSelect369();
	void r2_handleDialogSelect385();

	int r3_handleVerbPickUp();
	int r3_handleVerbUse();
	void r3_handleVerbTalkTo();
	bool r3_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r3_refreshRoomBackground();

	int r4_handleVerbPickUp();
	int r4_handleVerbUse();
	void r4_handleVerbTalkTo();
	int r4_handleVerbOpen();
	bool r4_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r4_refreshRoomBackground();
	void r4_handleRoomEvent();
	void r4_initRoomAnimations();
	void r4_uninitRoomAnimations();
	void r4_updateRoomAnimations(bool doUpdate);
	int r4_useCheesePizzaWithCastle();
	int r4_useDrumstickWithCastleDoor();
	void r4_useChainWithObject(bool arg6, bool arg8);
	int r4_useTubeWithShade();

	int r5_handleVerbPickUp();
	int r5_handleVerbUse();
	int r5_handleVerbPush();
	int r5_handleVerbOpen();
	bool r5_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r5_refreshRoomBackground();
	void r5_handleRoomEvent();

	int r6_handleVerbPickUp();
	int r6_handleVerbUse();
	int r6_handleVerbPull();
	int r6_handleVerbOpen();
	int r6_handleVerbGive();
	bool r6_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r6_refreshRoomBackground();
	void r6_handleRoomEvent1();
	void r6_handleRoomEvent2();
	void r6_handleRoomEvent3();
	void r6_handleRoomEvent4();
	void r6_initRoomAnimations();
	void r6_uninitRoomAnimations();
	void r6_updateRoomAnimations(bool doUpdate);
	void r6_useRopeWithBeam();
	void r6_useHockeyStickWithRope();
	void r6_useSeat();
	void r6_pullRope();
	void r6_giveLuckySausageToDog();

	int r7_handleVerbPickUp();
	int r7_handleVerbUse();
	void r7_handleVerbTalkTo();
	int r7_handleVerbPush();
	int r7_handleVerbOpen();
	void r7_refreshRoomBackground();
	void r7_initRoomAnimations();
	void r7_uninitRoomAnimations();
	void r7_updateRoomAnimations(bool doUpdate);
	void r7_openCloseFrontDoor(bool isOpen);

	int r8_handleVerbPickUp();
	int r8_handleVerbUse();
	int r8_handleVerbOpen();
	int r8_handleVerbClose();
	bool r8_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r8_refreshRoomBackground();
	void r8_initRoomAnimations();
	void r8_updateRoomAnimations(bool doUpdate);
	void r8_handleRoomEvent1();
	void r8_handleRoomEvent2();
	void r8_handleRoomEvent3();
	void r8_handleRoomEvent4();
	void r8_openCloseDoor(bool isOpen);
	void r8_openCloseCabinetDoor1(bool isOpen);
	void r8_openCloseCabinetDoor2(bool isOpen);

	int r9_handleVerbPickUp();
	int r9_handleVerbUse();
	void r9_handleVerbTalkTo();
	int r9_handleVerbPush();
	int r9_handleVerbGive();
	bool r9_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r9_refreshRoomBackground();
	void r9_initRoomAnimations();
	void r9_uninitRoomAnimations();
	void r9_updateRoomAnimations(bool doUpdate);
	void r9_giveCoatOrTopHatToMan();

	int r10_handleVerbPickUp();
	int r10_handleVerbUse();
	void r10_handleVerbTalkTo();
	int r10_handleVerbPush();
	int r10_handleVerbOpen();
	int r10_handleVerbGive();
	bool r10_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r10_refreshRoomBackground();
	void r10_handleVerbPickUpCandyBar();
	void r10_handleVerbPickUpTicket();
	void r10_handleVerbPickUpGum();
	void r10_handleVerbGiveWinningTicketToSalesgirl();
	void r10_buyItem();
	void r10_handleVerbPickUpJawbreakers();
	void r10_refreshObject(int value);

	int r11_handleVerbPickUp();
	int r11_handleVerbUse();
	int r11_handleVerbPush();
	void r11_refreshRoomBackground();
	void r11_initRoomAnimations();
	void r11_uninitRoomAnimations();
	void r11_updateRoomAnimations(bool doUpdate);
	void r11_pickUpExtensionCord();
	void r11_usePlungersWithLampPost();
	void r11_useExtensionCordWithOutlet();
	void r11_useSuckCutWithExtensionCord();
	void r11_useSuckCutWithFountain();
	void r11_useCar1();
	void r11_useCar2();

	int r12_handleVerbPickUp();
	int r12_handleVerbUse();
	void r12_handleVerbTalkTo();
	int r12_handleVerbGive();
	bool r12_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r12_refreshRoomBackground();
	void r12_talkToLawyer();
	void r12_givePotatoChipToCecil();

	int r13_handleVerbPickUp();
	int r13_handleVerbUse();
	void r13_handleVerbTalkTo();
	int r13_handleVerbGive();
	bool r13_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r13_refreshRoomBackground();
	void r13_giveTicketToRomeToPepe();
	void r13_handleDialogSelect183();
	void r13_handleRoomEvent();

	int r14_handleVerbPickUp();
	int r14_handleVerbUse();
	void r14_refreshRoomBackground();

	bool r15_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);

	void r16_refreshRoomBackground();

	int r17_handleVerbUse();
	void r17_handleVerbTalkTo();
	bool r17_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r17_refreshRoomBackground();
	void r17_handleRoomEvent1();
	void r17_handleRoomEvent2();
	void r17_handleRoomEvent3();

	void r18_refreshRoomBackground();
	void r18_initRoomAnimations();
	void r18_uninitRoomAnimations();
	void r18_updateRoomAnimations(bool doUpdate);

	void r19_handleVerbTalkTo();
	bool r19_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r19_refreshRoomBackground();
	void r19_handleRoomEvent();

	void r20_refreshRoomBackground();
	void r20_handleRoomEvent();

	void r22_refreshRoomBackground();

	int r24_handleVerbUse();
	void r24_refreshRoomBackground();
	void r24_handleRoomEvent(int wayneLadderX, int wayneX, int wayneLadderY, int ladderBottomY, int climbCtrMax, int ceilingIndex);
	void r24_climbLadder(int wayneX, int wayneLadderX, int wayneLadderY, int ladderTopY, int ceilingIndex);
	void r24_useMazeHole(int holeIndex);

	int r25_handleVerbUse();
	void r25_refreshRoomBackground();
	void r25_updateMazeRoomHole(int mazeRoomNumber);

	int r28_handleVerbUse();
	bool r28_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r28_refreshRoomBackground();
	void r28_handleRoomEvent1();
	void r28_handleRoomEvent2();

	int r29_handleVerbPickUp();
	int r29_handleVerbUse();
	void r29_refreshRoomBackground();
	void r29_handleRoomEvent();
	void r29_initRoomAnimations();
	void r29_uninitRoomAnimations();
	void r29_updateRoomAnimations(bool doUpdate);

	int r30_handleVerbPickUp();
	int r30_handleVerbUse();
	bool r30_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r30_refreshRoomBackground();
	void r30_handleRoomEvent1();
	void r30_handleRoomEvent2();
	void r30_runLudwigCutscene();
	void r30_talkToCecil();

	int r31_handleVerbUse();
	bool r31_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r31_refreshRoomBackground();
	void r31_handleRoomEvent1();
	void r31_handleRoomEvent2();
	void r31_handleRoomEvent3();
	void r31_handleRoomEvent4();
	void r31_handleRoomEvent5();
	void r31_correctAnswerSelected();
	void r31_wrongAnswerSelected();
	void r31_useBuzzer();
	void r31_playTalkAnim(int index);
	void r31_drawMBuzzer();
	void r31_drawDBuzzer();
	void r31_displayCategories();
	void r31_runBabeoff();
	void r31_drawCurrentPlayerScore();
	void r31_buildQuestionDialogChoices(int categoryIndex, int questionIndex);

	int r32_handleVerbPickUp();
	int r32_handleVerbUse();
	void r32_handleVerbTalkTo();
	int r32_handleVerbOpen();
	bool r32_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r32_refreshRoomBackground();
	void r32_handleRoomEvent();
	void r32_initRoomAnimations();
	void r32_uninitRoomAnimations();
	void r32_updateRoomAnimations(bool doUpdate);
	void r32_pickUpMemo();

	void r33_refreshRoomBackground();

	int r34_handleVerbPickUp();
	int r34_handleVerbUse();
	void r34_handleVerbTalkTo();
	int r34_handleVerbOpen();
	int r34_handleVerbClose();
	bool r34_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r34_refreshRoomBackground();
	void r34_initRoomAnimations();
	void r34_uninitRoomAnimations();
	void r34_updateRoomAnimations(bool doUpdate);

	int r35_handleVerbUse();
	void r35_handleVerbTalkTo();
	bool r35_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void r35_refreshRoomBackground();
	void r35_useSetOfKeysWithJailCells();
	void r35_talkToCassandra();

	int r36_handleVerbPickUp();
	int r36_handleVerbUse();
	void r36_refreshRoomBackground();
	void r36_handleRoomEvent();
	void r36_initRoomAnimations();
	void r36_uninitRoomAnimations();
	void r36_updateRoomAnimations(bool doUpdate);

	int r37_handleVerbPickUp();
	int r37_handleVerbUse();
	int r37_handleVerbPush();
	int r37_handleVerbPull();
	void r37_refreshRoomBackground();
	void r37_handleRoomEvent();
	void r37_useLadder1();
	void r37_useLadder2();
	void r37_useLadder3();
	void r37_pushPullLock(bool isPull);
	void r37_pullHandle();

	int r38_handleVerbUse();
	int r38_handleVerbPull();
	void r38_refreshRoomBackground();
	void r38_initRoomAnimations();
	void r38_uninitRoomAnimations();
	void r38_updateRoomAnimations(bool doUpdate);
	void r38_useCassandra();
	void r38_pullRag();

	int r39_handleVerbUse();
	int r39_handleVerbOpen();
	void r39_refreshRoomBackground();
	void r39_useSquirtGunWithHinges();
	void r39_useExit39();
	void r39_useExit();

};

} // End of namespace WaynesWorld

#endif // WAYNESWORLD_GAMELOGIC_H

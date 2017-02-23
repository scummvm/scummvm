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

#ifndef AGI_OPCODES_H
#define AGI_OPCODES_H

namespace Agi {

void cmdIncrement(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDecrement(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAssignN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAssignV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAddN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAddV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSubN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSubV(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x08
void cmdLindirectV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRindirect(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLindirectN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSet(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReset(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdToggle(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdResetV(AgiGame *state, AgiEngine *vm, uint8 *p);    // 0x10
void cmdToggleV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdNewRoom(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdNewRoomF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadLogic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadLogicF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCall(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCallF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadPic(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x18
void cmdLoadPicV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDrawPic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDrawPicV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShowPic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDiscardPic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdOverlayPic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShowPriScreen(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadView(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadViewF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDiscardView(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x20
void cmdAnimateObj(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdUnanimateAll(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDraw(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdErase(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPosition(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPositionV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPositionF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPositionFV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetPosn(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReposition(AgiGame *state, AgiEngine *vm, uint8 *p);    // 0x28
void cmdRepositionV1(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x28
void cmdSetView(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetViewF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetLoopF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdFixLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReleaseLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetCel(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetCelF(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x30
void cmdLastCel(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCurrentCel(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCurrentLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCurrentView(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdNumberOfLoops(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetPriority(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetPriorityF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReleasePriority(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x38
void cmdGetPriority(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStopUpdate(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStartUpdate(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdForceUpdate(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdIgnoreHorizon(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdObserveHorizon(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetHorizon(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdObjectOnWater(AgiGame *state, AgiEngine *vm, uint8 *p); // 0x40
void cmdObjectOnLand(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdObjectOnAnything(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdIgnoreObjs(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdObserveObjs(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDistance(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStopCycling(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStartCycling(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdNormalCycle(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x48
void cmdEndOfLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdEndOfLoopV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReverseCycle(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReverseLoop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReverseLoopV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCycleTime(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStopMotion(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStopMotionV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStartMotion(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStartMotionV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStepSize(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStepTime(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x50
void cmdMoveObj(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdMoveObjF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdFollowEgo(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdWander(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdNormalMotion(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetDir(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetDir(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdIgnoreBlocks(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x58
void cmdObserveBlocks(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdBlock(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdUnblock(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGet(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDrop(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPut(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPutF(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x60
void cmdGetRoomF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadSound(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSound(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStopSound(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPrint(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPrintF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDisplay(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDisplayF(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x68
void cmdClearLines(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdTextScreen(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGraphics(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetCursorChar(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetTextAttribute(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShakeScreen(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdConfigureScreen(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStatusLineOn(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x70
void cmdStatusLineOff(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetString(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetString(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdWordToString(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdParse(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdGetNum(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPreventInput(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAcceptInput(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x78
void cmdSetKey(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAddToPic(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAddToPicV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAddToPicF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdStatus(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSaveGame(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLoadGame(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdInitDisk(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRestartGame(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x80
void cmdShowObj(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRandom(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRandomV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdProgramControl(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPlayerControl(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdObjStatusF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdQuit(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdQuitV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShowMem(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPause(AgiGame *state, AgiEngine *vm, uint8 *p); // 0x88
void cmdEchoLine(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCancelLine(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdInitJoy(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdToggleMonitor(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdVersion(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdScriptSize(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetGameID(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdLog(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0x90
void cmdSetScanStart(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdResetScanStart(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRepositionTo(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdRepositionToF(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdTraceOn(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdTraceInfo(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPrintAt(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPrintAtV(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0x98
//void cmdDiscardView(AgiGame *state, AgiEngine *vm, uint8 *p); // Opcode repeated from 0x20 ?
void cmdClearTextRect(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetUpperLeft(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetMenu(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetMenuItem(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSubmitMenu(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdEnableItem(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDisableItem(AgiGame *state, AgiEngine *vm, uint8 *p);   // 0xa0
void cmdMenuInput(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShowObjV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdOpenDialogue(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCloseDialogue(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdMulN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdMulV(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDivN(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDivV(AgiGame *state, AgiEngine *vm, uint8 *p);  // 0xa8
void cmdCloseWindow(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetSimple(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPushScript(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdPopScript(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdHoldKey(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetPriBase(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdDiscardSound(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdHideMouse(AgiGame *state, AgiEngine *vm, uint8 *p); // 0xb0
void cmdAllowMenu(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdShowMouse(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdFenceMouse(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdMousePosn(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdReleaseKey(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdAdjEgoMoveToXY(AgiGame *state, AgiEngine *vm, uint8 *p);

void cmdSetSpeed(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdSetItemView(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdCallV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void cmdUnknown(AgiGame *state, AgiEngine *vm, uint8 *p);

void condEqual(AgiGame *state, AgiEngine *vm, uint8 *p);
void condEqualV(AgiGame *state, AgiEngine *vm, uint8 *p);
void condLess(AgiGame *state, AgiEngine *vm, uint8 *p);
void condLessV(AgiGame *state, AgiEngine *vm, uint8 *p);
void condGreater(AgiGame *state, AgiEngine *vm, uint8 *p);
void condGreaterV(AgiGame *state, AgiEngine *vm, uint8 *p);
void condIsSet(AgiGame *state, AgiEngine *vm, uint8 *p);
void condIsSetV(AgiGame *state, AgiEngine *vm, uint8 *p);
void condHas(AgiGame *state, AgiEngine *vm, uint8 *p);
void condHasV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void condObjInRoom(AgiGame *state, AgiEngine *vm, uint8 *p);
void condPosn(AgiGame *state, AgiEngine *vm, uint8 *p);
void condController(AgiGame *state, AgiEngine *vm, uint8 *p);
void condHaveKey(AgiGame *state, AgiEngine *vm, uint8 *p);
void condSaid(AgiGame *state, AgiEngine *vm, uint8 *p);
void condCompareStrings(AgiGame *state, AgiEngine *vm, uint8 *p);
void condObjInBox(AgiGame *state, AgiEngine *vm, uint8 *p);
void condCenterPosn(AgiGame *state, AgiEngine *vm, uint8 *p);
void condRightPosn(AgiGame *state, AgiEngine *vm, uint8 *p);
void condUnknown13(AgiGame *state, AgiEngine *vm, uint8 *p);
void condUnknown(AgiGame *state, AgiEngine *vm, uint8 *p);

void condIsSetV1(AgiGame *state, AgiEngine *vm, uint8 *p);
void condSaid1(AgiGame *state, AgiEngine *vm, uint8 *p);
void condSaid2(AgiGame *state, AgiEngine *vm, uint8 *p);
void condSaid3(AgiGame *state, AgiEngine *vm, uint8 *p);
void condBit(AgiGame *state, AgiEngine *vm, uint8 *p);

} // End of namespace Agi

#endif /* AGI_OPCODES_H */

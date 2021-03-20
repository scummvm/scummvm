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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_PROTO_H
#define TRECISION_PROTO_H

#include "common/str.h"

#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"

void warning(const char *format, ...);

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

struct ATFHandle;
struct SDObj;
struct STexture;
struct SPan;

int Compare(const void *p1, const void *p2);
bool AnimFileInit(Common::String fname);
void AnimFileFinish();
Common::SeekableReadStream *AnimFileOpen(Common::String name);
Common::SeekableReadStream *FmvFileOpen(const char *name);
bool SpeechFileInit(const char *fname);
void SpeechFileFinish();
int SpeechFileLen(const char *name);
int SpeechFileRead(const char *name, uint8 *buf);
void RedrawRoom();
void decompress(const uint8 *src, unsigned int src_len, uint8 *dst, unsigned int dst_len);
uint32 DecCR(Common::String FileName, uint8 *DestArea, uint8 *DecArea);
uint16 vr(int16 x, int16 y);
void VMouseOFF();
void VMouseON();
void VMouseRestore();
void VMouseCopy();
uint16 TextLength(const char *sign, uint16 num);
void IconSnapShot();
bool DataSave();
bool DataLoad();
bool QuitGame();
void DemoOver();
void CheckFileInCD(Common::String name);
void openSys();
void OpenVideo();
uint32 ReadActor(const char *filename, uint8 *Area);
int actionInRoom(int curA);
void ReadLoc();
void TendIn();
void ReadObj();
void ReadExtraObj2C();
void ReadExtraObj41D();
void ReadSounds();
void RegenRoom();
void PaintRegenRoom();
void DrawObj(SDObj);
void ResetZB(int x1, int y1, int x2, int y2);
void MCopy(uint16 *Dest, uint16 *Src, uint32 Len);
bool CheckMask(uint16 MX, uint16 MY);
char *GetNextSent();
void actorDoAction(int whatAction);
void actorStop();
void setPosition(int num);
void goToPosition(int num);
void lookAt(float x, float z);
float sinCosAngle(float sinus, float cosinus);
void textureTriangle(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2, int32 x3, int32 y3, int32 z3, int32 c3, int32 tx3, int32 ty3, STexture *t);
void textureScanEdge(int32 x1, int32 y1, int32 z1, int32 c1, int32 tx1, int32 ty1, int32 x2, int32 y2, int32 z2, int32 c2, int32 tx2, int32 ty2);
void shadowTriangle(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 cv, int32 zv);
void shadowScanEdge(int32 x1, int32 y1, int32 x2, int32 y2);
void init3DRoom(int16 dx, uint16 *destBuffer, int16 *zBuffer);
void setClipping(int16 x1, int16 y1, int16 x2, int16 y2);
void setZBufferRegion(int16 sx, int16 sy, int16 dx);
signed char clockWise(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3);
void drawCharacter(uint8 flag);
int read3D(Common::String c);
void findPath();
void findShortPath();
float evalPath(int a, float destX, float destZ, int nearP);
void buildFramelist();
int nextStep();
void displayPath();
int findAttachedPanel(int srcP, int destP);
bool pointInside(int pan, double x, double z);
int pathCompare(const void *arg1, const void *arg2);
void sortPath();
float distF(float x1, float y1, float x2, float y2);
float dist3D(float x1, float y1, float z1, float x2, float y2, float z2);
void putPix(int x, int y, uint16 c);
void whereIs(int px, int py);
void pointOut();
void putLine(int x0, int y0, int x1, int y1, uint16 c);
void viewPanel(SPan *p);
void pointProject(float x, float y, float z);
void invPointProject(int x, int y);
int intersectLinePanel(SPan *p, float x, float y, float z);
int intersectLineFloor(float x, float y, float z);
int intersectLineLine(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd);
void initSortPan();
int panCompare(const void *arg1, const void *arg2);
void sortPanel();
void actorOrder();
void doAction();
void doMouse();
void StartCharacterAction(uint16 Act, uint16 NewRoom, uint8 NewPos, uint16 sent);
void doCharacter();
void doSystem();
void doScrollInventory(uint16 mousex);
void RollInventory(uint8 status);
void doIdle();
void DialogPrint(int x, int y, int c, const char *txt);
void MostraScelte(uint16 i);
void UpdateScelte(int16 dmx, int16 dmy);
void ScegliScelta(int16 dmx, int16 dmy);
void PlayDialog(uint16 i);
void afterChoice(int numframe);
void DialogHandler(int numframe);
void PlayScelta(uint16 i);
void doDialog();
void doRoomIn(uint16 curObj);
void doRoomOut(uint16 TheObj);
void doMouseExamine(uint16 TheObj);
void doMouseOperate(uint16 TheObj);
void doMouseTake(uint16 TheObj);
void doMouseTalk(uint16 TheObj);
void doUseWith();
void doInvInvUseWith();
void doInvScrUseWith();
void doScrScrUseWith();
void doInvExamine();
void doInvOperate();
void doDoing();
void doScript();
bool AtMouseClick(uint16 TheObj);
void AtEndChangeRoom();
void ExecuteATFDO(ATFHandle *h, int doit, int32 obj);
void ProcessATF(ATFHandle *h, int type, int atf);
void InitAtFrameHandler(uint16 an, uint16 obj);
void AtFrameNext();
void AtFrameEnd(int type);
void AtFrameHandler(int type);
void NextMessage();
void ProcessTime();
void ProcessMouse();
void PaintScreen(uint8 flag);
void AddLine(int16 x1, int16 x2, int16 y);
int BlockCompare(const void *arg1, const void *arg2);
void SortBlock();
void PaintObjAnm(uint16 CurBox);
int IntersecateRect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
void doEvent(uint8 cls, uint8 event, uint8 priority, uint16 u16Param1, uint16 u16Param2, uint8 u8Param, uint32 u32Param);
void Scheduler();
void ProcessTheMessage();
void SwapMessage(Message *m1, Message *m2);
void RepaintString();
void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool Homo);
void ShowObjName(uint16 obj, bool showhide);
void ShowInvName(uint16 obj, bool showhide);
void FormattingSuperString();
void FormattingOneString();
void CharacterSay(uint16 i);
void CharacterTalk(const char *s, bool FromCharacterSay);
void CharacterTalkInAction(uint16 ss);
void CharacterContinueTalk();
void CharacterMute();
void SomeOneTalk(uint16 s, uint16 Person, uint16 NewAnim, bool FromSomeOneSay);
void SomeOneContinueTalk();
void SomeOneMute();
void doString();
uint8 WhatIcon(uint16 mx);
uint8 IconPos(uint8 icon);
void KillIcon(uint8 icon);
void AddIcon(uint8 icon);
void ReplaceIcon(uint8 oldicon, uint8 newicon);
void EndScript();
void PlayScript(uint16 i);
void EvalScript();
void Text(uint16 x, uint16 y, const char *sign, uint16 tcol, uint16 scol);
void ClearText();
void PaintString();
void DoClearText();
void DoSys(uint16 curObj);
void SetRoom(uint16 r, bool b);
char GetKey();
char waitKey();
void FreeKey();
uint32 ReadTime();
void NlDelay(uint32 val);
void NlDissolve(int val);
void Mouse(MouseCmd opt);
void CloseSys(const char *str);
void BattutaPrint(int x, int y, int c, const char *txt);
void soundtimefunct();
void StartSoundSystem();
void StopSoundSystem();
int16 LoadAudioWav(int num, uint8 *wav, int len);
void NLPlaySound(int num);
void NLStopSound(int num);
void SoundFadOut();
void SoundFadIn(int num);
void WaitSoundFadEnd();
void SoundPasso(int midx, int midz, int act, int frame, uint16 *list);
void ContinueTalk();
int32 Talk(const char *name);
void StopTalk();

#endif
} // End of namespace Trecision

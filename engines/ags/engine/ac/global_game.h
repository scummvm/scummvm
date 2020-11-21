//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__GLOBALGAME_H
#define __AGS_EE_AC__GLOBALGAME_H

#include "util/string.h"
using namespace AGS; // FIXME later

void GiveScore(int amnt);
void restart_game();
void RestoreGameSlot(int slnum);
void DeleteSaveSlot (int slnum);
int  GetSaveSlotDescription(int slnum,char*desbuf);
int  LoadSaveSlotScreenshot(int slnum, int width, int height);
void PauseGame();
void UnPauseGame();
int  IsGamePaused();
void SetGlobalInt(int index,int valu);
int  GetGlobalInt(int index);
void SetGlobalString (int index, const char *newval);
void GetGlobalString (int index, char *strval);
int  RunAGSGame (const char *newgame, unsigned int mode, int data);
int  GetGameParameter (int parm, int data1, int data2, int data3);
void QuitGame(int dialog);
void SetRestartPoint();
void SetGameSpeed(int newspd);
int  GetGameSpeed();
int  SetGameOption (int opt, int setting);
int  GetGameOption (int opt);

void SkipUntilCharacterStops(int cc);
void EndSkippingUntilCharStops();
// skipwith decides how it can be skipped:
// 1 = ESC only
// 2 = any key
// 3 = mouse button
// 4 = mouse button or any key
// 5 = right click or ESC only
void StartCutscene (int skipwith);
int EndCutscene ();
// Tell the game to skip current cutscene
void SkipCutscene();

void sc_inputbox(const char*msg,char*bufr);

int GetLocationType(int xxx,int yyy);
void SaveCursorForLocationChange();
void GetLocationName(int xxx,int yyy,char*tempo);

int IsKeyPressed (int keycode);

int SaveScreenShot(const char*namm);
void SetMultitasking (int mode);

void RoomProcessClick(int xx,int yy,int mood);
int IsInteractionAvailable (int xx,int yy,int mood);

void GetMessageText (int msg, char *buffer);

void SetSpeechFont (int fontnum);
void SetNormalFont (int fontnum);

void _sc_AbortGame(const char* text);

int GetGraphicalVariable (const char *varName);
void SetGraphicalVariable (const char *varName, int p_value);
void scrWait(int nloops);
int WaitKey(int nloops);
int WaitMouse(int nloops);
int WaitMouseKey(int nloops);
void SkipWait();

#endif // __AGS_EE_AC__GLOBALGAME_H

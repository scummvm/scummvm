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

#ifndef AGS_ENGINE_AC_GLOBAL_GAME_H
#define AGS_ENGINE_AC_GLOBAL_GAME_H

#include "ags/shared/util/string.h"
#include "ags/lib/system/datetime.h"

namespace AGS3 {

using namespace AGS; // FIXME later

struct SaveListItem {
	int    Slot;
	Shared::String Description;
	time_t FileTime = 0;

	SaveListItem(int slot, const Shared::String &desc, time_t ft)
		: Slot(slot), Description(desc), FileTime(ft) {
	}

	inline bool operator < (const SaveListItem &other) const {
		return FileTime < other.FileTime;
	}
};

// Notify the running game that the engine requested immediate stop
void AbortGame();
void GiveScore(int amnt);
void restart_game();
void RestoreGameSlot(int slnum);
void DeleteSaveSlot(int slnum);
int  GetSaveSlotDescription(int slnum, char *desbuf);
int  LoadSaveSlotScreenshot(int slnum, int width, int height);
void FillSaveList(std::vector<SaveListItem> &saves, size_t max_count = -1);
void PauseGame();
void UnPauseGame();
int  IsGamePaused();
void SetGlobalInt(int index, int valu);
int  GetGlobalInt(int index);
void SetGlobalString(int index, const char *newval);
void GetGlobalString(int index, char *strval);
int  RunAGSGame(const Shared::String &newgame, unsigned int mode, int data);
int  GetGameParameter(int parm, int data1, int data2, int data3);
void QuitGame(int dialog);
void SetRestartPoint();
void SetGameSpeed(int newspd);
int  GetGameSpeed();
int  SetGameOption(int opt, int setting);
int  GetGameOption(int opt);

void SkipUntilCharacterStops(int cc);
void EndSkippingUntilCharStops();
// skipwith decides how it can be skipped:
// 1 = ESC only
// 2 = any key
// 3 = mouse button
// 4 = mouse button or any key
// 5 = right click or ESC only
void StartCutscene(int skipwith);
int EndCutscene();
// Tell the game to skip current cutscene
void SkipCutscene();

// ShowInputBox assumes a string buffer of MAX_MAXSTRLEN
void ShowInputBox(const char *msg, char *bufr);
void ShowInputBoxImpl(const char *msg, char *bufr, size_t buf_len);

int GetLocationType(int xxx, int yyy);
void SaveCursorForLocationChange();
// GetLocationName assumes a string buffer of MAX_MAXSTRLEN
void GetLocationName(int xxx, int yyy, char *buf);

int IsKeyPressed(int keycode);

int SaveScreenShot(const char *namm);
void SetMultitasking(int mode);

void RoomProcessClick(int xx, int yy, int mood);
int IsInteractionAvailable(int xx, int yy, int mood);

void GetMessageText(int msg, char *buffer);

void SetSpeechFont(int fontnum);
void SetNormalFont(int fontnum);

void _sc_AbortGame(const char *text);

int GetGraphicalVariable(const char *varName);
void SetGraphicalVariable(const char *varName, int p_value);
void scrWait(int nloops);
int WaitKey(int nloops);
int WaitMouse(int nloops);
int WaitMouseKey(int nloops);
int WaitInput(int input_flags, int nloops);
void SkipWait();

void scStartRecording(int keyToStop);

} // namespace AGS3

#endif

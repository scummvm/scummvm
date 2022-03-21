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

//=============================================================================
//
// AGS Platform-specific functions
//
//=============================================================================

#include "ags/shared/util/wgt2_allg.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/lib/system/datetime.h"
#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/thread.h"
#include "ags/globals.h"

#if defined (AGS_HAS_CD_AUDIO)
#include "ags/shared/libcda.h"
#endif

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

AGSPlatformDriver *AGSPlatformDriver::instance = nullptr;

// ******** DEFAULT IMPLEMENTATIONS *******

void AGSPlatformDriver::PostAllegroInit(bool windowed) {
}

void AGSPlatformDriver::AttachToParentConsole() {
}

int AGSPlatformDriver::GetLastSystemError() {
	return _G(errnum);
}

void AGSPlatformDriver::DisplaySwitchOut() {
}

void AGSPlatformDriver::DisplaySwitchIn() {
}

void AGSPlatformDriver::PauseApplication() {
}

void AGSPlatformDriver::ResumeApplication() {
}

void AGSPlatformDriver::GetSystemDisplayModes(std::vector<DisplayMode> &dms) {
}

bool AGSPlatformDriver::EnterFullscreenMode(const DisplayMode &dm) {
	return true;
}

bool AGSPlatformDriver::ExitFullscreenMode() {
	return true;
}

void AGSPlatformDriver::AdjustWindowStyleForFullscreen() {
}

void AGSPlatformDriver::AdjustWindowStyleForWindowed() {
}

void AGSPlatformDriver::RegisterGameWithGameExplorer() {
}

void AGSPlatformDriver::UnRegisterGameWithGameExplorer() {
}

void AGSPlatformDriver::PlayVideo(const char *name, int skip, int flags) {
}

const char *AGSPlatformDriver::GetAllegroFailUserHint() {
	return "Make sure you have latest version of Allegro 4 libraries installed, and your system is running in graphical mode.";
}

const char *AGSPlatformDriver::GetDiskWriteAccessTroubleshootingText() {
	return "Make sure you have write permissions, and also check the disk's free space.";
}

void AGSPlatformDriver::GetSystemTime(ScriptDateTime *sdt) {
	time_t t = getUnixTime();

	//note: subject to year 2038 problem due to shoving time_t in an integer
	sdt->rawUnixTime = static_cast<int>(t);

	tm newTime;
	localTime(&newTime);
	sdt->hour = newTime.tm_hour;
	sdt->minute = newTime.tm_min;
	sdt->second = newTime.tm_sec;
	sdt->day = newTime.tm_mday;
	sdt->month = newTime.tm_mon + 1;
	sdt->year = newTime.tm_year + 1900;
}

void AGSPlatformDriver::WriteStdOut(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Common::String str = Common::String::vformat(fmt, args);
	va_end(args);

	debug("%s", str.c_str());
}

void AGSPlatformDriver::WriteStdErr(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Common::String str = Common::String::vformat(fmt, args);
	va_end(args);

	debug("ERROR: %s", str.c_str());
}

void AGSPlatformDriver::YieldCPU() {
	// NOTE: this is called yield, but if we actually yield instead of delay,
	// we get a massive increase in CPU usage.
	this->Delay(10);
	//std::this_thread::yield();
}

void AGSPlatformDriver::InitialiseAbufAtStartup() {
	// because loading the game file accesses abuf, it must exist
	// No no no, David Blain, no magic here :P
	//abuf = BitmapHelper::CreateBitmap(10,10,8);
}

void AGSPlatformDriver::FinishedUsingGraphicsMode() {
	// don't need to do anything on any OS except DOS
}

SetupReturnValue AGSPlatformDriver::RunSetup(const ConfigTree &cfg_in, ConfigTree &cfg_out) {
	return kSetup_Cancel;
}

void AGSPlatformDriver::SetCommandArgs(const char *const argv[], size_t argc) {
	_cmdArgs = argv;
	_cmdArgCount = argc;
}

Common::String AGSPlatformDriver::GetCommandArg(size_t arg_index) {
	return arg_index < _cmdArgCount ? _cmdArgs[arg_index] : nullptr;
}

void AGSPlatformDriver::SetGameWindowIcon() {
	// do nothing
}

int AGSPlatformDriver::ConvertKeycodeToScanCode(int keycode) {
	keycode -= ('A' - Common::KEYCODE_a);
	return keycode;
}

bool AGSPlatformDriver::LockMouseToWindow() {
	return false;
}

void AGSPlatformDriver::UnlockMouse() {
}

//-----------------------------------------------
// IOutputHandler implementation
//-----------------------------------------------

void AGSPlatformDriver::PrintMessage(const Shared::DebugMessage &msg) {
	if (_logToStdErr) {
		if (msg.GroupName.IsEmpty())
			WriteStdErr("%s", msg.Text.GetCStr());
		else
			WriteStdErr("%s : %s", msg.GroupName.GetCStr(), msg.Text.GetCStr());
	} else {
		if (msg.GroupName.IsEmpty())
			WriteStdOut("%s", msg.Text.GetCStr());
		else
			WriteStdOut("%s : %s", msg.GroupName.GetCStr(), msg.Text.GetCStr());
	}
}

// ********** CD Player Functions common to Win and Linux ********

#if defined (AGS_HAS_CD_AUDIO)

int numcddrives = 0;

int cd_player_init() {
	int erro = cd_init();
	if (erro) return -1;
	numcddrives = 1;
	_G(use_cdplayer) = 1;
	return 0;
}

int cd_player_control(int cmdd, int datt) {
	// WINDOWS & LINUX VERSION
	if (cmdd == 1) {
		if (cd_current_track() > 0) return 1;
		return 0;
	} else if (cmdd == 2) {
		cd_play_from(datt);
		_G(need_to_stop_cd) = 1;
	} else if (cmdd == 3)
		cd_pause();
	else if (cmdd == 4)
		cd_resume();
	else if (cmdd == 5) {
		int first, last;
		if (cd_get_tracks(&first, &last) == 0)
			return (last - first) + 1;
		else return 0;
	} else if (cmdd == 6)
		cd_eject();
	else if (cmdd == 7)
		cd_close();
	else if (cmdd == 8)
		return numcddrives;
	else if (cmdd == 9);
	else quit("!CDAudio: Unknown command code");

	return 0;
}

#endif // AGS_HAS_CD_AUDIO

void AGSPlatformDriver::Delay(int millis) {
	auto now = AGS_Clock::now();
	auto delayUntil = now + std::chrono::milliseconds(millis);

	for (;;) {
		if (now >= delayUntil) {
			break;
		}

		auto duration = std::min<std::chrono::milliseconds>(delayUntil - now,
		                _G(MaximumDelayBetweenPolling));
		std::this_thread::sleep_for(duration);
		now = AGS_Clock::now(); // update now

		if (now >= delayUntil) {
			break;
		}

		// don't allow it to check for debug messages, since this Delay()
		// call might be from within a debugger polling loop
		now = AGS_Clock::now(); // update now
	}
}

} // namespace AGS3

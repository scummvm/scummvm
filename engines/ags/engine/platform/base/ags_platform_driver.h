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
// AGS Cross-Platform Header
//
//=============================================================================

#ifndef AGS_ENGINE_PLATFORM_BASE_AGS_PLATFORM_DRIVER_H
#define AGS_ENGINE_PLATFORM_BASE_AGS_PLATFORM_DRIVER_H

#include "common/std/vector.h"
#include "ags/engine/ac/date_time.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/shared/debugging/output_handler.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/ini_util.h"
#include "ags/lib/allegro/error.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared

namespace Engine {
struct DisplayMode;
} // namespace Engine
} // namespace AGS

using namespace AGS; // FIXME later

enum eScriptSystemOSID {
	eOS_DOS = 1,
	eOS_Win,
	eOS_Linux,
	eOS_Mac,
	eOS_Android,
	eOS_iOS,
	eOS_PSP,
	eOS_Web
};

enum SetupReturnValue {
	kSetup_Cancel,
	kSetup_Done,
	kSetup_RunGame
};

struct AGSPlatformDriver
// be used as a output target for logging system
	: public AGS::Shared::IOutputHandler {
	virtual ~AGSPlatformDriver() { instance = nullptr; }

    // Called at the creation of the platform driver
    virtual void MainInit() { };
    // Called right before the formal backend init
    virtual void PreBackendInit() { };
    // Called right after the formal backend init
    virtual void PostBackendInit() { };
    // Called right before the backend is deinitialized
    virtual void PreBackendExit() { };
    // Called right after the backend is deinitialized
    virtual void PostBackendExit() { };

	virtual void Delay(int millis);
	virtual void DisplayAlert(const char *, ...) = 0;
	virtual void AttachToParentConsole();
	virtual int  GetLastSystemError();
	// Optionally fill in config tree from the platform-specific config source
	virtual void ReadConfiguration(Shared::ConfigTree & /*cfg*/) {}
	// Get root directory for storing per-game shared data
	virtual FSLocation GetAllUsersDataDirectory() {
		return FSLocation(".");
	}
	// Get root directory for storing per-game saved games
	virtual FSLocation GetUserSavedgamesDirectory() {
		return FSLocation(".");
	}
	// Get root directory for storing per-game user configuration files
	virtual FSLocation GetUserConfigDirectory() {
		return FSLocation(".");
	}
	// Get directory for storing all-games user configuration files
	virtual FSLocation GetUserGlobalConfigDirectory() {
		return FSLocation(".");
	}
	// Get default directory for program output (logs)
	virtual FSLocation GetAppOutputDirectory() {
		return FSLocation(".");
	}
	// Tells whether it's not permitted to write to the local directory (cwd, or game dir),
	// and only specified user/app directories should be used.
	// FIXME: this is a part of a hotfix, review uses of this function later.
	virtual bool IsLocalDirRestricted() { return true; }
	// Returns array of characters illegal to use in file names
	virtual const char *GetIllegalFileChars() {
		return "\\/";
	}
	virtual const char *GetDiskWriteAccessTroubleshootingText();
	virtual const char *GetGraphicsTroubleshootingText() {
		return "";
	}
	virtual uint64_t GetDiskFreeSpaceMB(const Shared::String &path) = 0;
	virtual const char *GetNoMouseErrorString() = 0;
	// Tells whether build is capable of controlling mouse movement properly
	virtual bool IsMouseControlSupported(bool windowed) {
		return false;
	}
	// Tells whether this platform's backend library deals with mouse cursor
	// virtual->real coordinate transformation itself (otherwise AGS engine should do it)
	virtual bool IsBackendResponsibleForMouseScaling() {
		return false;
	}
	virtual const char *GetAllegroFailUserHint();
	virtual eScriptSystemOSID GetSystemOSID() = 0;
	virtual void GetSystemTime(ScriptDateTime *);
	virtual void PlayVideo(const char *name, int skip, int flags);
	virtual void InitialiseAbufAtStartup();
	virtual void PostAllegroInit(bool windowed);
	virtual void PostAllegroExit() = 0;
	virtual const char *GetBackendFailUserHint() {
		return nullptr;
	}
	virtual void FinishedUsingGraphicsMode();
	virtual SetupReturnValue RunSetup(const Shared::ConfigTree &cfg_in, Shared::ConfigTree &cfg_out);
	virtual void SetGameWindowIcon();
	// Formats message and writes to standard platform's output;
	// Always adds trailing '\n' after formatted string
	virtual void WriteStdOut(const char *fmt, ...);
	// Formats message and writes to platform's error output;
	// Always adds trailing '\n' after formatted string
	virtual void WriteStdErr(const char *fmt, ...);
	// Display a text in a message box with a "warning" icon.
	// Platforms which do not support this should do nothing.
	virtual void DisplayMessageBox(const char *text) = 0;
	virtual void YieldCPU();
	// Called when the game window is being switch out from
	virtual void DisplaySwitchOut();
	// Called when the game window is being switch back to
	virtual void DisplaySwitchIn();
	// Called when the application is being paused completely (e.g. when player alt+tabbed from it).
	// This function should suspend any platform-specific realtime processing.
	virtual void PauseApplication();
	// Called when the application is being resumed.
	virtual void ResumeApplication();
	// Returns a list of supported display modes
	virtual void GetSystemDisplayModes(std::vector<Engine::DisplayMode> &dms);
	// Switch to system fullscreen mode; store previous mode parameters
	virtual bool EnterFullscreenMode(const Engine::DisplayMode &dm);
	// Return back to the mode was before switching to fullscreen
	virtual bool ExitFullscreenMode();
	// Adjust application window's parameters to suit fullscreen mode
	virtual void AdjustWindowStyleForFullscreen();
	// Adjust application window's parameters to suit windowed mode
	virtual void AdjustWindowStyleForWindowed();
	virtual int  ConvertKeycodeToScanCode(int keyCode);
	// Adjust window's * client size * to ensure it is in the supported limits
	virtual Size ValidateWindowSize(const Size &sz, bool borderless) const;

	virtual int  InitializeCDPlayer() = 0;  // return 0 on success
	virtual int  CDPlayerCommand(int cmdd, int datt) = 0;
	virtual void ShutdownCDPlayer() = 0;

	// Returns command line argument in a UTF-8 format
	virtual Common::String GetCommandArg(size_t arg_index);

	virtual bool LockMouseToWindow();
	virtual void UnlockMouse();

	static AGSPlatformDriver *GetDriver();

	// Store command line arguments for the future use
	void SetCommandArgs(const char *const argv[], size_t argc);

	// Set whether PrintMessage should output to stdout or stderr
	void SetOutputToErr(bool on) {
		_logToStdErr = on;
	}
	// Set whether DisplayAlert is allowed to show modal GUIs on some systems;
	// it will print to either stdout or stderr otherwise, depending on above flag
	void SetGUIMode(bool on) {
		_guiMode = on;
	}

	//-----------------------------------------------
	// IOutputHandler implementation
	//-----------------------------------------------
	// Writes to the standard platform's output, prepending "AGS: " prefix to the message
	void PrintMessage(const AGS::Shared::DebugMessage &msg) override;

protected:
	// TODO: this is a quick solution for IOutputHandler implementation
	// logging either to stdout or stderr. Normally there should be
	// separate implementation, one for each kind of output, but
	// with both going through PlatformDriver need to figure a better
	// design first.
	bool _logToStdErr = false;
	// Defines whether engine is allowed to display important warnings
	// and errors by showing a message box kind of GUI.
	bool _guiMode = false;

	const char *const *_cmdArgs = nullptr;
	size_t _cmdArgCount = 0u;

private:
	static AGSPlatformDriver *instance;
};

#if defined (AGS_HAS_CD_AUDIO)
int cd_player_init();
int cd_player_control(int cmdd, int datt);
#endif

// [IKM] What is a need to have this global var if you can get AGSPlatformDriver
// instance by calling AGSPlatformDriver::GetDriver()?


} // namespace AGS3

#endif

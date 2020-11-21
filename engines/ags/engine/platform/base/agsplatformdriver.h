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

//=============================================================================
//
// AGS Cross-Platform Header
//
//=============================================================================

#ifndef __AGS_EE_PLATFORM__AGSPLATFORMDRIVER_H
#define __AGS_EE_PLATFORM__AGSPLATFORMDRIVER_H

#include <errno.h>
#include <vector>
#include "ac/datetime.h"
#include "debug/outputhandler.h"
#include "util/ini_util.h"

namespace AGS
{
    namespace Common { class Stream; }
    namespace Engine { struct DisplayMode; }
}
using namespace AGS; // FIXME later

enum eScriptSystemOSID
{
    eOS_DOS = 1,
    eOS_Win,
    eOS_Linux,
    eOS_Mac,
    eOS_Android,
    eOS_iOS,
    eOS_PSP
};

enum SetupReturnValue
{
    kSetup_Cancel,
    kSetup_Done,
    kSetup_RunGame
};

struct AGSPlatformDriver
    // be used as a output target for logging system
    : public AGS::Common::IOutputHandler
{
    virtual void AboutToQuitGame();
    virtual void Delay(int millis);
    virtual void DisplayAlert(const char*, ...) = 0;
    virtual void AttachToParentConsole();
    virtual int  GetLastSystemError() { return errno; }
    // Get root directory for storing per-game shared data
    virtual const char *GetAllUsersDataDirectory() { return "."; }
    // Get root directory for storing per-game saved games
    virtual const char *GetUserSavedgamesDirectory() { return "."; }
    // Get root directory for storing per-game user configuration files
    virtual const char *GetUserConfigDirectory() { return "."; }
    // Get directory for storing all-games user configuration files
    virtual const char *GetUserGlobalConfigDirectory()  { return "."; }
    // Get default directory for program output (logs)
    virtual const char *GetAppOutputDirectory() { return "."; }
    // Returns array of characters illegal to use in file names
    virtual const char *GetIllegalFileChars() { return "\\/"; }
    virtual const char *GetDiskWriteAccessTroubleshootingText();
    virtual const char *GetGraphicsTroubleshootingText() { return ""; }
    virtual unsigned long GetDiskFreeSpaceMB() = 0;
    virtual const char* GetNoMouseErrorString() = 0;
    // Tells whether build is capable of controlling mouse movement properly
    virtual bool IsMouseControlSupported(bool windowed) { return false; }
    // Tells whether this platform's backend library deals with mouse cursor
    // virtual->real coordinate transformation itself (otherwise AGS engine should do it)
    virtual bool IsBackendResponsibleForMouseScaling() { return false; }
    virtual const char* GetAllegroFailUserHint();
    virtual eScriptSystemOSID GetSystemOSID() = 0;
    virtual void GetSystemTime(ScriptDateTime*);
    virtual void PlayVideo(const char* name, int skip, int flags);
    virtual void InitialiseAbufAtStartup();
    virtual void PostAllegroInit(bool windowed);
    virtual void PostAllegroExit() = 0;
    virtual void FinishedUsingGraphicsMode();
    virtual SetupReturnValue RunSetup(const Common::ConfigTree &cfg_in, Common::ConfigTree &cfg_out);
    virtual void SetGameWindowIcon();
    // Formats message and writes to standard platform's output;
    // Always adds trailing '\n' after formatted string
    virtual void WriteStdOut(const char *fmt, ...);
    // Formats message and writes to platform's error output;
    // Always adds trailing '\n' after formatted string
    virtual void WriteStdErr(const char *fmt, ...);
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
    virtual void RegisterGameWithGameExplorer();
    virtual void UnRegisterGameWithGameExplorer();
    virtual int  ConvertKeycodeToScanCode(int keyCode);
    // Adjust window size to ensure it is in the supported limits
    virtual void ValidateWindowSize(int &x, int &y, bool borderless) const {}

    virtual int  InitializeCDPlayer() = 0;  // return 0 on success
    virtual int  CDPlayerCommand(int cmdd, int datt) = 0;
    virtual void ShutdownCDPlayer() = 0;

    virtual bool LockMouseToWindow();
    virtual void UnlockMouse();

    static AGSPlatformDriver *GetDriver();

    // Set whether PrintMessage should output to stdout or stderr
    void SetOutputToErr(bool on) { _logToStdErr = on; }
    // Set whether DisplayAlert is allowed to show modal GUIs on some systems;
    // it will print to either stdout or stderr otherwise, depending on above flag
    void SetGUIMode(bool on) { _guiMode = on; }

    //-----------------------------------------------
    // IOutputHandler implementation
    //-----------------------------------------------
    // Writes to the standard platform's output, prepending "AGS: " prefix to the message
    void PrintMessage(const AGS::Common::DebugMessage &msg) override;

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

private:
    static AGSPlatformDriver *instance;
};

#if defined (AGS_HAS_CD_AUDIO)
int cd_player_init();
int cd_player_control(int cmdd, int datt);
#endif

// [IKM] What is a need to have this global var if you can get AGSPlatformDriver
// instance by calling AGSPlatformDriver::GetDriver()?
extern AGSPlatformDriver *platform;

#endif // __AGS_EE_PLATFORM__AGSPLATFORMDRIVER_H

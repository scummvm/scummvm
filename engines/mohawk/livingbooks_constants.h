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

#ifndef MOHAWK_LIVINGBOOKS_CONSTANTS_H
#define MOHAWK_LIVINGBOOKS_CONSTANTS_H

namespace Mohawk {

/* Some disambiguations:
 *  _soundMode is a priority (see LBItem::getSoundPriority), not a kLBControlMode.
 *  _controlMode (item flags) is unrelated to kLBControlMode (the menu screens).
 *  LBProxyItem::_page shadows LBItem::_page (preexisting, intentional-looking, untouched).
 */

// Engine Debug Flags
enum {
	kDebugCode = 1,
};

#define LBKEY_MOD_CTRL 1
#define LBKEY_MOD_ALT 2
#define LBKEY_MOD_SHIFT 4
enum NodeState {
	kLBNodeDone = 0,
	kLBNodeRunning = 1,
	kLBNodeWaiting = 2
};

enum LBMode {
	kLBIntroMode = 1,
	kLBControlMode = 2,
	kLBCreditsMode = 3,
	kLBPreviewMode = 4,
	kLBReadMode = 5,
	kLBPlayMode = 6
};

enum {
	kLBPhaseInit = 0x0,
	kLBPhaseIntro = 0x1,
	kLBPhaseMain = 0x2,
	kLBPhaseNone = 0x7fff,
	kLBPhaseLoad = 0xfffe,
	kLBPhaseCreate = 0xffff
};

// automatic modes used in _timingMode
enum {
	kLBAutoNone = 0,
	kLBAutoIntro = 1,
	kLBAutoUserIdle = 2,
	kLBAutoMain = 3,
	kLBAutoSync = 4,
	kLBAutoInit = 5,
	kLBAutoCreate = 6,
	kLBAutoLoad = 7
};

// control modes used in _controlMode
enum {
	kLBControlNone = 0,
	kLBControlHideMouse = 1,
	kLBControlPauseItems = 2
};

enum {
	kLBStaticTextItem = 0x1,
	kLBPictureItem = 0x2,
	kLBEditTextItem = 0x14,
	kLBLiveTextItem = 0x15,
	kLBAnimationItem = 0x40,
	kLBSoundItem = 0x41,
	kLBGroupItem = 0x42,
	kLBMovieItem = 0x43,
	kLBPaletteAItem = 0x44, // unused?
	kLBPaletteItem = 0x45,
	kLBProxyItem = 0x46,
	kLBMiniGameItem = 666, // EVIL!!!!
	kLBXDataFileItem = 0x3e9,
	kLBDiscDectectorItem = 0xfa1
};

enum {
	// no 0x1?
	kLBAnimOpNotify = 0x2,
	kLBAnimOpSetTempo = 0x3,
	// no 0x4?
	kLBAnimOpMoveTo = 0x5,
	kLBAnimOpWait = 0x6,
	kLBAnimOpSetCel = 0x7,
	kLBAnimOpSleepUntil = 0x8,
	kLBAnimOpDrawMode = 0x9,
	kLBAnimOpPlaySound = 0xa,
	kLBAnimOpWaitForSound = 0xb,
	kLBAnimOpReleaseSound = 0xc,
	kLBAnimOpResetSound = 0xd,
	kLBAnimOpSetTempoDiv = 0xe,
	kLBAnimOpDelay = 0xf
};

enum {
	kLBEventPhaseInit = 0,
	kLBEventPhaseIntro = 1,
	kLBEventMouseDown = 2,
	kLBEventStarted = 3,
	kLBEventDone = 4,
	kLBEventMouseUp = 5,
	kLBEventPhaseMain = 6,
	kLBEventNotified = 7,
	kLBEventDragStart = 8,
	kLBEventDragMove = 9,
	kLBEventDragEnd = 0xa,
	kLBEventRolloverBegin = 0xb,
	kLBEventRolloverMove = 0xc,
	kLBEventRolloverEnd = 0xd,
	kLBEventMouseUpIn = 0xe,
	kLBEventMouseUpOut = 0xf,
	kLBEventMouseTrackIn = 0x10,
	kLBEventMouseTrackMove = 0x11,
	kLBEventMouseTrackMoveIn = 0x12,
	kLBEventMouseTrackMoveOut = 0x13,
	kLBEventMouseTrackOut = 0x14,
	kLBEventFocusBegin = 0x15,
	kLBEventFocusEnd = 0x16,
	kLBEventInit = 0x17,
	kLBEventLoad = 0x1a,
	kLBEventListLoad = 0x1b,
	kLBEventPhaseCreate = 0xff
};

enum {
	kLBGroupData = 0x64,
	kLBLiveTextData = 0x65,
	kLBMsgListScript = 0x66,
	kLBNotifyScript = 0x67,
	kLBSetPlayInfo = 0x68,
	kLBSetRandomLoc = 0x69,  // unused?
	kLBSetDrag = 0x6a,       // unused?
	kLBSetDrawMode = 0x6b,
	kLBSetFont = 0x6c,       // unused?
	kLBSetOneShot = 0x6d,    // unused?
	kLBSetPlayPhase = 0x6e,
	// from here, 2.x+
	kLBSetKeyNotify = 0x6f,
	kLBCommand = 0x70,
	kLBPaletteAData = 0x71,  // unused?
	kLBPaletteXData = 0x72,
	kLBDisable = 0x73,       // unused?
	kLBEnable = 0x74,        // unused?
	kLBSetNotVisible = 0x75,
	kLBSetVisible = 0x76,    // unused?
	kLBGlobalDisable = 0x77,
	kLBGlobalEnable = 0x78,  // unused?
	kLBGlobalSetNotVisible = 0x79,
	kLBGlobalSetVisible = 0x7a, // unused?
	kLBSetAmbient = 0x7b,
	kLBSetDragParams = 0x7c,
	kLBSetKeyEvent = 0x7d,
	kLBSetRolloverData = 0x7e,
	kLBSetParent = 0x7f,
	kLBSetHitTest = 0x80,
	// from here, rugrats
	kLBUnknown194 = 0x194
};

enum {
	kLBOpNone = 0x0,
	kLBOpXShow = 0x1,
	kLBOpTogglePlay = 0x2,
	kLBOpSetNotVisible = 0x3,
	kLBOpSetVisible = 0x4,
	kLBOpDestroy = 0x5,
	kLBOpRewind = 0x6,
	kLBOpStop = 0x7,
	kLBOpDisable = 0x8,
	kLBOpEnable = 0x9,
	// (no 0xa)
	kLBOpGlobalSetNotVisible = 0xb,
	kLBOpGlobalSetVisible = 0xc,
	kLBOpGlobalDisable = 0xd,
	kLBOpGlobalEnable = 0xe,
	kLBOpSeekToEnd = 0xf,
	// (no 0x10)
	kLBOpMute = 0x11,
	kLBOpUnmute = 0x12,
	kLBOpLoad = 0x13,
	kLBOpPreload = 0x14,
	kLBOpUnload = 0x15,
	kLBOpSeekToNext = 0x16,
	kLBOpSeekToPrev = 0x17,
	kLBOpDragBegin = 0x18,
	kLBOpDragEnd = 0x19,
	kLBOpScriptDisable = 0x1a,
	kLBOpScriptEnable = 0x1b,
	kLBOpUnknown1C = 0x1c,
	kLBOpSendExpression = 0x1d,
	kLBOpJumpUnlessExpression = 0xfffb,
	kLBOpBreakExpression = 0xfffc,
	kLBOpJumpToExpression = 0xfffd,
	kLBOpRunSubentries = 0xfffe,
	kLBOpRunData = 0xffff
};

enum {
	kLBNotifyGUIAction = 1,
	kLBNotifyGoToControls = 2,
	kLBNotifyChangePage = 3,
	kLBNotifyGotoQuit = 4,
	kLBNotifyIntroDone = 5,
	kLBNotifyChangeMode = 6,
	kLBNotifyCursorChange = 7,
	kLBNotifyPrintPage = 0xc,
	kLBNotifyQuit = 0xd
};

enum {
	kTargetTypeExpression = 0x3f3f,
	kTargetTypeCode = 0xfffe,
	kTargetTypeName = 0xffff
};

} // End of namespace Mohawk

#endif

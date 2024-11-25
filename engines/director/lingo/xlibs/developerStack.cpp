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

#include "audio/mixer.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/developerStack.h"

/*************************************
 *
 * USED IN:
 * overringunder: Over-Ring-Under
 *
 *************************************/

/*************************************
 *
 * Developer Stack 1.3r
 * Issue date 11/05/88
 *
 * This stack is © 1987,88 Steve Drazga & AnalytX.  The individual XCMDs
 * and scripts are copyrighted by the original authors as stated their description.
 *
 * This issue of Developer Stack may be freely distributed by individuals in its
 * original form with all original messages.  DO NOT DISTRIBUTE MODIFIED COPIES!
 * User groups may distribute it for regular disk fees.  Other organizations
 * desiring to charge for the distribution of Developer Stack should contact AnalytX.
 *
 * Developer Stack is intended for educational & training purposes only and
 * no warranty is made as to the suitablity of anything included in this stack
 * for any specific purpose.  For support on any individual contributions, please
 * contact the creator directly.
 *
 */

 /*
  * XMCDs:
  *
  * Alert
  * BarButton
  * ChangeMenu
  * CheckMenu
  * ClipToPICTRes
  * Color
  * CombineFile
  * CommInit
  * CommWrite
  * DeleteFile2
  * DeProtect
  * DispPict
  * DoList
  * doRestart
  * DragOn
  * DrawPict
  * Ejector
  * EnableMenu
  * GetDANames
  * HyperSND
  * Import
  * ImportPict
  * InitMidi
  * MungeMCTB
  * OSErr
  * PrintClip
  * Progress
  * ResCopy
  * ResetMIDI
  * ResetPrinter
  * sendSerial
  * SetVolume  -- Done
  * ShowMenu
  * ShutDown
  * SoundCapToRes
  * Speak
  * StdFile
  * Stripper
  * Tabs2Spaces
  * Talk
  * TitleBar
  * TxMIDI
  */


/*
 * This XCMD sets the current volume level of the Macintosh.
 *
 * This XCMD is ©1988 by Steve Drazga and all rights are reserved.
 * It may be used in any non-commercial, shareware, or commercial stacks
 * as long as the following conditions are met:
 *
 * 1) Within the credits of the stack the phrase 'Portions © 1988 Steve Drazga" should be included.
 *
 * 2) Send a copy of your finished stack to me at:
 * Steve Drazga
 * AnalytX
 * Box 388
 * Southampton, PA  18966
 *
 * If you need custom XCMDs/XFCNs, stacks, or HyperCard training for your organization contact:
 * AnalytX at (215) 750-0792.
 *
 *
 * Syntax:
 * SetVolume "©1988SDrazga",<newVolume>
 * The first parameter must be the copyright notice (the © symbol is option-g).
 * The second parameter is the new volume level (0-7)
 *
 * Example usage: SetVolume "©1988SDrazga",
 *
 *************************************/

namespace Director {

const char *const DeveloperStack::xlibName = "SetVolume";
const XlibFileDesc DeveloperStack::fileNames[] = {
	{ "SetVolume",	nullptr },
	{ nullptr,		nullptr },
};

static const BuiltinProto builtins[] = {
	{ "SetVolume", DeveloperStack::b_setvolume, 2, 2, 300, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void DeveloperStack::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void DeveloperStack::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void DeveloperStack::b_setvolume(int nargs) {
	int volume = g_lingo->pop().asInt();
	Common::String copyright = g_lingo->pop().asString();

	debug(5, "LB::b_setvolume: copyright: %s vol: %d", copyright.c_str(), volume);

	volume = CLIP<int>(volume, 0, 7);

	g_director->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, Audio::Mixer::kMaxMixerVolume / 7 * volume);
}

} // End of namespace Director

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

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"
#include "sci/engine/features.h"
#include "sci/engine/script_patches.h"

#include "common/util.h"

namespace Sci {

// IMPORTANT:
// every patch entry needs the following:
//  - script number (pretty obvious)
//
//  - apply count
//     specifies the number of times a patch is supposed to get applied.
//     Most of the time, it should be 1.
//
//  - magicDWORD + magicOffset
//     please ALWAYS put 0 for those two. Both will get filled out at runtime by the patcher.
//
//  - signature data (is used to identify certain script code, that needs patching)
//     every signature needs to contain SIG_MAGICDWORD once.
//      The following 4 bytes after SIG_MAGICDWORD - which don't have to be fixed, you may for example
//      use SIG_SELECTOR16, will get used to quickly search for a partly match before verifying that
//      the whole signature actually matches. If it's not included, the script patcher will error() out
//      right when loading up the game.
//     If selector-IDs are included, please use SIG_SELECTOR16 + SIG_SELECTOR8 [1]. Simply
//      specify the selector that way, so that the patcher will search for the specific
//      selector instead of looking for a hardcoded value. Selectors may not be the same
//      between game versions.
//     For UINT16s either use SIG_UINT16 or SIG_SELECTOR16.
//      Macintosh versions of SCI games are using BE ordering instead of LE since SCI1.1 for UINT16s in scripts
//      By using those 2 commands, it's possible to make patches work for PC and Mac versions of the same game.
//     You may also skip bytes by using the SIG_ADDTOOFFSET command
//     Every signature data needs to get terminated using SIGNATURE_END
//
//  - patch data (is used for actually patching scripts)
//     When a match is found, the patch data will get applied.
//     Patch data is similar to signature data. Just use PATCH_SELECTOR16 + PATCH_SELECTOR8 [1]
//      for patching in selectors.
//     There are also patch specific commands.
//     Those are PATCH_GETORIGINALBYTE, which fetches a byte from the original script
//      and PATCH_GETORIGINALBYTEADJUST, which does the same but gets a second value
//      from the uint16 array and uses that value to adjust the original byte.
//     Every patch data needs to get terminated using PATCH_END
//
//  - and please always add a comment about why the patch was done and what's causing issues.
//     If possible make sure, that the patch works on localized (or just different) game versions
//      as well in case those need patching too.
//
// [1] - selectors need to get specified in selectorTable[] and ScriptPatcherSelectors-enum
//        before they can get used using the SIG_SELECTORx and PATCH_SELECTORx commands.
//        You have to use the exact same order in both the table and the enum, otherwise
//        it won't work.
//        ATTENTION: selectors will only work here, when they are also in SelectorCache (selector.h)

static const char *const selectorNameTable[] = {
	"cycles",       // system selector
	"seconds",      // system selector
	"init",         // system selector
	"dispose",      // system selector
	"new",          // system selector
	"curEvent",     // system selector
	"disable",      // system selector
	"doit",         // system selector
	"show",         // system selector
	"x",            // system selector
	"cel",          // system selector
	"setMotion",    // system selector
	"overlay",      // system selector
	"setPri",       // system selector - for setting priority
	"deskSarg",     // Gabriel Knight
	"localize",     // Freddy Pharkas
	"put",          // Police Quest 1 VGA
	"say",          // Quest For Glory 1 VGA
	"contains",     // Quest For Glory 2
	"solvePuzzle",  // Quest For Glory 3
	"timesShownID", // Space Quest 1 VGA
	"startText",    // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"startAudio",   // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"modNum",       // King's Quest 6 CD / Laura Bow 2 CD for audio+text support
	"cycler",       // Space Quest 4 / system selector
	"setLoop",      // Laura Bow 1 Colonel's Bequest
	"ignoreActors", // Laura Bow 1 Colonel's Bequest
#ifdef ENABLE_SCI32
	"newWith",      // SCI2 array script
	"scrollSelections", // GK2
	"posn",         // SCI2 benchmarking script
	"detailLevel",  // GK2 benchmarking
	"view",         // RAMA benchmarking
#endif
	NULL
};

enum ScriptPatcherSelectors {
	SELECTOR_cycles = 0,
	SELECTOR_seconds,
	SELECTOR_init,
	SELECTOR_dispose,
	SELECTOR_new,
	SELECTOR_curEvent,
	SELECTOR_disable,
	SELECTOR_doit,
	SELECTOR_show,
	SELECTOR_x,
	SELECTOR_cel,
	SELECTOR_setMotion,
	SELECTOR_overlay,
	SELECTOR_setPri,
	SELECTOR_deskSarg,
	SELECTOR_localize,
	SELECTOR_put,
	SELECTOR_say,
	SELECTOR_contains,
	SELECTOR_solvePuzzle,
	SELECTOR_timesShownID,
	SELECTOR_startText,
	SELECTOR_startAudio,
	SELECTOR_modNum,
	SELECTOR_cycler,
	SELECTOR_setLoop,
	SELECTOR_ignoreActors
#ifdef ENABLE_SCI32
	,
	SELECTOR_newWith,
	SELECTOR_scrollSelections,
	SELECTOR_posn,
	SELECTOR_detailLevel,
	SELECTOR_view
#endif
};

#ifdef ENABLE_SCI32
// It is not possible to change the directory for ScummVM save games, so disable
// the "change directory" button in the standard save dialogue
static const uint16 sci2ChangeDirSignature[] = {
	0x72, SIG_ADDTOOFFSET(+2), // lofsa changeDirI
	0x4a, SIG_UINT16(0x04),    // send 4
	SIG_MAGICDWORD,
	0x36,                      // push
	0x35, 0xF7,                // ldi $f7
	0x12,                      // and
	0x36,                      // push
	SIG_END
};

static const uint16 sci2ChangeDirPatch[] = {
	PATCH_ADDTOOFFSET(+3),    // lofsa changeDirI
	PATCH_ADDTOOFFSET(+3),    // send 4
	PATCH_ADDTOOFFSET(+1),    // push
	0x35, 0x00,               // ldi 0
	PATCH_END
};

// Save game script hardcodes the maximum number of save games to 20, but
// this is an artificial constraint that does not apply to ScummVM
static const uint16 sci2NumSavesSignature1[] = {
	SIG_MAGICDWORD,
	0x8b, 0x02,                    // lsl local[2]
	0x35, 0x14,                    // ldi 20
	0x22,                          // lt?
	SIG_END
};

static const uint16 sci2NumSavesPatch1[] = {
	PATCH_ADDTOOFFSET(+2),         // lsl local[2]
	0x35, 0x63,                    // ldi 99
	PATCH_END
};

static const uint16 sci2NumSavesSignature2[] = {
	SIG_MAGICDWORD,
	0x8b, 0x02,                    // lsl local[2]
	0x35, 0x14,                    // ldi 20
	0x1a,                          // eq?
	SIG_END
};

static const uint16 sci2NumSavesPatch2[] = {
	PATCH_ADDTOOFFSET(+2),         // lsl local[2]
	0x35, 0x63,                    // ldi 99
	PATCH_END
};

// Phantasmagoria & SQ6 try to initialize the first entry of an int16 array
// using an empty string, which is not valid (it should be a number)
static const uint16 sci21IntArraySignature[] = {
	0x38, SIG_SELECTOR16(newWith), // pushi newWith
	0x7a,                          // push2
	0x39, 0x04,                    // pushi $4
	0x72, SIG_ADDTOOFFSET(+2),     // lofsa string ""
	SIG_MAGICDWORD,
	0x36,                          // push
	0x51, 0x0b,                    // class IntArray
	0x4a, 0x8,                     // send $8
	SIG_END
};

static const uint16 sci21IntArrayPatch[] = {
	PATCH_ADDTOOFFSET(+6),      // push $b9; push2; pushi $4
	0x76,                       // push0
	0x34, PATCH_UINT16(0x0001), // ldi 0001 (waste bytes)
	PATCH_END
};

// Most SCI32 games have a video performance benchmarking loop at the
// beginning of the game. Running this benchmark with calls to
// `OSystem::updateScreen` will often cause the benchmark to return a low value,
// which causes games to disable some visual effects. Running without calls to
// `OSystem::updateScreen` on any reasonably modern CPU will cause the benchmark
// to overflow, leading to randomly disabled effects. This patch changes the
// benchmarking code to always return the game's maximum speed value.
//
// Applies to at least: GK1 floppy, PQ4 floppy, PQ4CD, LSL6hires, Phant1,
// Shivers, SQ6
static const uint16 sci2BenchmarkSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(init),  // pushi init
	0x76,                        // push0
	0x38, SIG_SELECTOR16(posn),  // pushi posn
	SIG_END
};

static const uint16 sci2BenchmarkPatch[] = {
	0x7a,                         // push2
	0x38, SIG_UINT16(64908),      // pushi 64908
	0x76,                         // push0
	0x43, 0x03, SIG_UINT16(0x04), // callk DisposeScript[3], 4
	0x48,                         // ret
	PATCH_END
};

#endif

// ===========================================================================
// Conquests of Camelot
// At the bazaar in Jerusalem, it's possible to see a girl taking a shower.
//  If you get too close, you get warned by the father - if you don't get away,
//  he will kill you.
// Instead of walking there manually, it's also possible to enter "look window"
//  and ego will automatically walk to the window. It seems that this is something
//  that wasn't properly implemented, because instead of getting killed, you will
//  get an "Oops" message in Sierra SCI.
//
// This is caused by peepingTom in script 169 not getting properly initialized.
// peepingTom calls the object behind global b9h. This global variable is
//  properly initialized, when walking there manually (method fawaz::doit).
// When you instead walk there automatically (method fawaz::handleEvent), that
//  global isn't initialized, which then results in the Oops-message in Sierra SCI
//  and an error message in ScummVM/SCI.
//
// We fix the script by patching in a jump to the proper code inside fawaz::doit.
// Responsible method: fawaz::handleEvent
// Fixes bug: #6402
static const uint16 camelotSignaturePeepingTom[] = {
	0x72, SIG_MAGICDWORD, SIG_UINT16(0x077e), // lofsa fawaz <-- start of proper initializion code
	0xa1, 0xb9,                      // sag b9h
	SIG_ADDTOOFFSET(+571),           // skip 571 bytes
	0x39, 0x7a,                      // pushi 7a <-- initialization code when walking automatically
	0x78,                            // push1
	0x7a,                            // push2
	0x38, SIG_UINT16(0x00a9), // + 0xa9, 0x00,   // pushi 00a9 - script 169
	0x78,                            // push1
	0x43, 0x02, 0x04,                // call kScriptID
	0x36,                            // push
	0x81, 0x00,                      // lag 00
	0x4a, 0x06,                      // send 06
	0x32, SIG_UINT16(0x0520),        // jmp [end of fawaz::handleEvent]
	SIG_END
};

static const uint16 camelotPatchPeepingTom[] = {
	PATCH_ADDTOOFFSET(+576),
	0x32, PATCH_UINT16(0xfdbd),      // jmp to fawaz::doit / properly init peepingTom code
	PATCH_END
};

//         script, description,                                       signature                   patch
static const SciScriptPatcherEntry camelotSignatures[] = {
	{ true,    62, "fix peepingTom Sierra bug",                    1, camelotSignaturePeepingTom, camelotPatchPeepingTom },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// stayAndHelp::changeState (0) is called when ego swims to the left or right
//  boundaries of room 660. Normally a textbox is supposed to get on screen
//  but the call is wrong, so not only do we get an error message the script
//  is also hanging because the cue won't get sent out
//  This also happens in sierra sci
// Applies to at least: PC-CD
// Responsible method: stayAndHelp::changeState
// Fixes bug: #5107
static const uint16 ecoquest1SignatureStayAndHelp[] = {
	0x3f, 0x01,                      // link 01
	0x87, 0x01,                      // lap param[1]
	0x65, 0x14,                      // aTop state
	0x36,                            // push
	0x3c,                            // dup
	0x35, 0x00,                      // ldi 00
	0x1a,                            // eq?
	0x31, 0x1c,                      // bnt [next state]
	0x76,                            // push0
	0x45, 0x01, 0x00,                // callb export1 from script 0 (switching control off)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0122),        // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, SIG_SELECTOR8(init),       // pushi "init"
	0x39, 0x04,                      // pushi 04
	0x76,                            // push0
	0x76,                            // push0
	0x39, 0x17,                      // pushi 17
	0x7c,                            // pushSelf
	0x51, 0x82,                      // class EcoNarrator
	0x4a, 0x0c,                      // send 0c - call EcoNarrator::init(0, 0, 23, self) (BADLY BROKEN!)
	0x33,                            // jmp [end]
	SIG_END
};

static const uint16 ecoquest1PatchStayAndHelp[] = {
	0x87, 0x01,                      // lap param[1]
	0x65, 0x14,                      // aTop state
	0x36,                            // push
	0x2f, 0x22,                      // bt [next state] (this optimization saves 6 bytes)
	0x39, 0x00,                      // pushi 0 (wasting 1 byte here)
	0x45, 0x01, 0x00,                // callb export1 from script 0 (switching control off)
	0x38, PATCH_UINT16(0x0122),      // pushi 0122
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x06,                      // send 06 - call ego::setMotion(0)
	0x39, PATCH_SELECTOR8(init),     // pushi "init"
	0x39, 0x06,                      // pushi 06
	0x39, 0x02,                      // pushi 02 (additional 2 bytes)
	0x76,                            // push0
	0x76,                            // push0
	0x39, 0x17,                      // pushi 17
	0x7c,                            // pushSelf
	0x38, PATCH_UINT16(0x0280),      // pushi 280 (additional 3 bytes)
	0x51, 0x82,                      // class EcoNarrator
	0x4a, 0x10,                      // send 10 - call EcoNarrator::init(2, 0, 0, 23, self, 640)
	PATCH_END
};

//          script, description,                                      signature                      patch
static const SciScriptPatcherEntry ecoquest1Signatures[] = {
	{  true,   660, "CD: bad messagebox and freeze",               1, ecoquest1SignatureStayAndHelp, ecoquest1PatchStayAndHelp },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// doMyThing::changeState (2) is supposed to remove the initial text on the
//  ecorder. This is done by reusing temp-space, that was filled on state 1.
//  this worked in sierra sci just by accident. In our sci, the temp space
//  is resetted every time, which means the previous text isn't available
//  anymore. We have to patch the code because of that.
// Fixes bug: #4993
static const uint16 ecoquest2SignatureEcorder[] = {
	0x31, 0x22,                      // bnt [next state]
	0x39, 0x0a,                      // pushi 0a
	0x5b, 0x04, 0x1e,                // lea temp[1e]
	0x36,                            // push
	SIG_MAGICDWORD,
	0x39, 0x64,                      // pushi 64
	0x39, 0x7d,                      // pushi 7d
	0x39, 0x32,                      // pushi 32
	0x39, 0x66,                      // pushi 66
	0x39, 0x17,                      // pushi 17
	0x39, 0x69,                      // pushi 69
	0x38, PATCH_UINT16(0x2631),      // pushi 2631
	0x39, 0x6a,                      // pushi 6a
	0x39, 0x64,                      // pushi 64
	0x43, 0x1b, 0x14,                // call kDisplay
	0x35, 0x0a,                      // ldi 0a
	0x65, 0x20,                      // aTop ticks
	0x33,                            // jmp [end]
	SIG_ADDTOOFFSET(+1),             // [skip 1 byte]
	0x3c,                            // dup
	0x35, 0x03,                      // ldi 03
	0x1a,                            // eq?
	0x31,                            // bnt [end]
	SIG_END
};

static const uint16 ecoquest2PatchEcorder[] = {
	0x2f, 0x02,                      // bt [to pushi 07]
	0x3a,                            // toss
	0x48,                            // ret
	0x38, PATCH_UINT16(0x0007),      // pushi 07 (parameter count) (waste 1 byte)
	0x39, 0x0b,                      // push (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x38, PATCH_UINT16(0x0017),      // pushi 17 (color) (waste 1 byte)
	0x43, 0x6c, 0x0e,                // call kGraph
	0x38, PATCH_UINT16(0x0005),      // pushi 05 (parameter count) (waste 1 byte)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // call kGraph
	PATCH_END
};

// ===========================================================================
// Same patch as above for the ecorder introduction.
// Two workarounds are needed for this patch in workarounds.cpp (when calling
// kGraphFillBoxAny and kGraphUpdateBox), as there isn't enough space to patch
// the function otherwise.
// Fixes bug: #6467
static const uint16 ecoquest2SignatureEcorderTutorial[] = {
	0x30, SIG_UINT16(0x0023),        // bnt [next state]
	0x39, 0x0a,                      // pushi 0a
	0x5b, 0x04, 0x1f,                // lea temp[1f]
	0x36,                            // push
	SIG_MAGICDWORD,
	0x39, 0x64,                      // pushi 64
	0x39, 0x7d,                      // pushi 7d
	0x39, 0x32,                      // pushi 32
	0x39, 0x66,                      // pushi 66
	0x39, 0x17,                      // pushi 17
	0x39, 0x69,                      // pushi 69
	0x38, SIG_UINT16(0x2631),        // pushi 2631
	0x39, 0x6a,                      // pushi 6a
	0x39, 0x64,                      // pushi 64
	0x43, 0x1b, 0x14,                // call kDisplay
	0x35, 0x1e,                      // ldi 1e
	0x65, 0x20,                      // aTop ticks
	0x32,                            // jmp [end]
	// 2 extra bytes, jmp offset
	SIG_END
};

static const uint16 ecoquest2PatchEcorderTutorial[] = {
	0x31, 0x23,                      // bnt [next state] (save 1 byte)
	// The parameter count below should be 7, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,                            // push1 (parameter count)
	//0x39, 0x07,                    // pushi 07 (parameter count)
	0x39, 0x0b,                      // push (FillBoxAny)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x78,                            // push1 (visual screen)
	0x39, 0x17,                      // pushi 17 (color)
	0x43, 0x6c, 0x0e,                // call kGraph
	// The parameter count below should be 5, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,                            // push1 (parameter count)
	//0x39, 0x05,                    // pushi 05 (parameter count)
	0x39, 0x0c,                      // pushi 12d (UpdateBox)
	0x39, 0x1d,                      // pushi 29d
	0x39, 0x73,                      // pushi 115d
	0x39, 0x5e,                      // pushi 94d
	0x38, PATCH_UINT16(0x00d7),      // pushi 215d
	0x43, 0x6c, 0x0a,                // call kGraph
	// We are out of bytes to patch at this point,
	// so we skip 494 (0x1EE) bytes to reuse this code:
	// ldi 1e
	// aTop 20
	// jmp 030e (jump to end)
	0x32, PATCH_UINT16(0x01ee),      // skip 494 (0x1EE) bytes
	PATCH_END
};

//          script, description,                                       signature                          patch
static const SciScriptPatcherEntry ecoquest2Signatures[] = {
	{  true,    50, "initial text not removed on ecorder",          1, ecoquest2SignatureEcorder,         ecoquest2PatchEcorder },
	{  true,   333, "initial text not removed on ecorder tutorial", 1, ecoquest2SignatureEcorderTutorial, ecoquest2PatchEcorderTutorial },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Fan-made games
// Attention: Try to make script patches as specific as possible

// CascadeQuest::autosave in script 994 is called various times to auto-save the game.
// The script use a fixed slot "999" for this purpose. This doesn't work in ScummVM, because we do not let
//  scripts save directly into specific slots, but instead use virtual slots / detect scripts wanting to
//  create a new slot.
//
// For this game we patch the code to use slot 99 instead. kSaveGame also checks for Cascade Quest,
//  will then check, if slot 99 is asked for and will then use the actual slot 0, which is the official
//  ScummVM auto-save slot.
//
// Responsible method: CascadeQuest::autosave
// Fixes bug: #7007
static const uint16 fanmadeSignatureCascadeQuestFixAutoSaving[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x03e7),        // pushi 3E7 (999d) -> save game slot 999
	0x74, SIG_UINT16(0x06f8),        // lofss "AutoSave"
	0x89, 0x1e,                      // lsg global[1E]
	0x43, 0x2d, 0x08,                // callk SaveGame
	SIG_END
};

static const uint16 fanmadePatchCascadeQuestFixAutoSaving[] = {
	0x38, PATCH_UINT16((SAVEGAMEID_OFFICIALRANGE_START - 1)), // fix slot
	PATCH_END
};

// EventHandler::handleEvent in Demo Quest has a bug, and it jumps to the
// wrong address when an incorrect word is typed, therefore leading to an
// infinite loop. This script bug was not apparent in SSCI, probably because
// event handling was slightly different there, so it was never discovered.
// Fixes bug: #5120
static const uint16 fanmadeSignatureDemoQuestInfiniteLoop[] = {
	0x38, SIG_UINT16(0x004c),        // pushi 004c
	0x39, 0x00,                      // pushi 00
	0x87, 0x01,                      // lap 01
	0x4b, 0x04,                      // send 04
	SIG_MAGICDWORD,
	0x18,                            // not
	0x30, SIG_UINT16(0x002f),        // bnt 002f  [06a5]    --> jmp ffbc  [0664] --> BUG! infinite loop
	SIG_END
};

static const uint16 fanmadePatchDemoQuestInfiniteLoop[] = {
	PATCH_ADDTOOFFSET(+10),
	0x30, PATCH_UINT16(0x0032),      // bnt 0032  [06a8] --> pushi 004c
	PATCH_END
};

//          script, description,                                      signature                                  patch
static const SciScriptPatcherEntry fanmadeSignatures[] = {
	{  true,   994, "Cascade Quest: fix auto-saving",              1, fanmadeSignatureCascadeQuestFixAutoSaving, fanmadePatchCascadeQuestFixAutoSaving },
	{  true,   999, "Demo Quest: infinite loop on typo",           1, fanmadeSignatureDemoQuestInfiniteLoop,     fanmadePatchDemoQuestInfiniteLoop },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// WORKAROUND
// Freddy Pharkas intro screen
// Sierra used inner loops for the scaling of the 2 title views.
// Those inner loops don't call kGameIsRestarting, which is why
// we do not update the screen and we also do not throttle.
//
// This patch fixes this and makes it work.
// Applies to at least: English PC-CD
// Responsible method: sTownScript::changeState(1), sTownScript::changeState(3) (script 110)
static const uint16 freddypharkasSignatureIntroScaling[] = {
	0x38, SIG_ADDTOOFFSET(+2),       // pushi (setLoop) (009b for PC CD)
	0x78,                            // push1
	PATCH_ADDTOOFFSET(1),            // push0 for first code, push1 for second code
	0x38, SIG_ADDTOOFFSET(+2),       // pushi (setStep) (0143 for PC CD)
	0x7a,                            // push2
	0x39, 0x05,                      // pushi 05
	0x3c,                            // dup
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa (view)
	SIG_MAGICDWORD,
	0x4a, 0x1e,                      // send 1e
	0x35, 0x0a,                      // ldi 0a
	0xa3, 0x02,                      // sal local[2]
	// start of inner loop
	0x8b, 0x02,                      // lsl local[2]
	SIG_ADDTOOFFSET(+43),            // skip almost all of inner loop
	0xa3, 0x02,                      // sal local[2]
	0x33, 0xcf,                      // jmp [inner loop start]
	SIG_END
};

static const uint16 freddypharkasPatchIntroScaling[] = {
	// remove setLoop(), objects in heap are already prepared, saves 5 bytes
	0x38,
	PATCH_GETORIGINALUINT16(+6),     // pushi (setStep)
	0x7a,                            // push2
	0x39, 0x05,                      // pushi 05
	0x3c,                            // dup
	0x72,
	PATCH_GETORIGINALUINT16(+13),    // lofsa (view)
	0x4a, 0x18,                      // send 18 - adjusted
	0x35, 0x0a,                      // ldi 0a
	0xa3, 0x02,                      // sal local[2]
	// start of new inner loop
	0x39, 0x00,                      // pushi 00
	0x43, 0x2c, 0x00,                // callk GameIsRestarting <-- add this so that our speed throttler is triggered
	SIG_ADDTOOFFSET(+47),            // skip almost all of inner loop
	0x33, 0xca,                      // jmp [inner loop start]
	PATCH_END
};

//  script 0 of freddy pharkas/CD PointsSound::check waits for a signal and if
//   no signal received will call kDoSound(0xD) which is a dummy in sierra sci
//   and ScummVM and will use acc (which is not set by the dummy) to trigger
//   sound disposal. This somewhat worked in sierra sci, because the sample
//   was already playing in the sound driver. In our case we would also stop
//   the sample from playing, so we patch it out
//   The "score" code is already buggy and sets volume to 0 when playing
// Applies to at least: English PC-CD
// Responsible method: unknown
static const uint16 freddypharkasSignatureScoreDisposal[] = {
	0x67, 0x32,                      // pTos 32 (selector theAudCount)
	0x78,                            // push1
	SIG_MAGICDWORD,
	0x39, 0x0d,                      // pushi 0d
	0x43, 0x75, 0x02,                // call kDoAudio
	0x1c,                            // ne?
	0x31,                            // bnt (-> to skip disposal)
	SIG_END
};

static const uint16 freddypharkasPatchScoreDisposal[] = {
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	PATCH_END
};

//  script 235 of freddy pharkas rm235::init and sEnterFrom500::changeState
//   disable icon 7+8 of iconbar (CD only). When picking up the canister after
//   placing it down, the scripts will disable all the other icons. This results
//   in IconBar::disable doing endless loops even in sierra sci, because there
//   is no enabled icon left. We remove disabling of icon 8 (which is help),
//   this fixes the issue.
// Applies to at least: English PC-CD
// Responsible method: rm235::init and sEnterFrom500::changeState
static const uint16 freddypharkasSignatureCanisterHang[] = {
	0x38, SIG_SELECTOR16(disable),   // pushi disable
	0x7a,                            // push2
	SIG_MAGICDWORD,
	0x39, 0x07,                      // pushi 07
	0x39, 0x08,                      // pushi 08
	0x81, 0x45,                      // lag 45
	0x4a, 0x08,                      // send 08 - call IconBar::disable(7, 8)
	SIG_END
};

static const uint16 freddypharkasPatchCanisterHang[] = {
	PATCH_ADDTOOFFSET(+3),
	0x78,                            // push1
	PATCH_ADDTOOFFSET(+2),
	0x33, 0x00,                      // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET(+3),
	0x06,                            // send 06 - call IconBar::disable(7)
	PATCH_END
};

//  script 215 of freddy pharkas lowerLadder::doit and highLadder::doit actually
//   process keyboard-presses when the ladder is on the screen in that room.
//   They strangely also call kGetEvent. Because the main User::doit also calls
//   kGetEvent, it's pure luck, where the event will hit. It's the same issue
//   as in QfG1VGA and if you turn dos-box to max cycles, and click around for
//   ego, sometimes clicks also won't get registered. Strangely it's not nearly
//   as bad as in our sci, but these differences may be caused by timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
// Applies to at least: English PC-CD, German Floppy, English Mac
// Responsible method: lowerLadder::doit and highLadder::doit
static const uint16 freddypharkasSignatureLadderEvent[] = {
	0x39, SIG_MAGICDWORD,
	SIG_SELECTOR8(new),              // pushi new
	0x76,                            // push0
	0x38, SIG_SELECTOR16(curEvent),  // pushi curEvent
	0x76,                            // push0
	0x81, 0x50,                      // lag global[50]
	0x4a, 0x04,                      // send 04 - read User::curEvent
	0x4a, 0x04,                      // send 04 - call curEvent::new
	0xa5, 0x00,                      // sat temp[0]
	0x38, SIG_SELECTOR16(localize),
	0x76,                            // push0
	0x4a, 0x04,                      // send 04 - call curEvent::localize
	SIG_END
};

static const uint16 freddypharkasPatchLadderEvent[] = {
	0x34, 0x00, 0x00,                // ldi 0000 (waste 3 bytes, overwrites first 2 pushes)
	PATCH_ADDTOOFFSET(+8),
	0xa5, 0x00,                      // sat temp[0] (waste 2 bytes, overwrites 2nd send)
	PATCH_ADDTOOFFSET(+2),
	0x34, 0x00, 0x00,                // ldi 0000
	0x34, 0x00, 0x00,                // ldi 0000 (waste 6 bytes, overwrites last 3 opcodes)
	PATCH_END
};

// In the Macintosh version of Freddy Pharkas, kRespondsTo is broken for
// property selectors. They hacked the script to work around the issue,
// so we revert the script back to using the values of the DOS script.
// Applies to at least: English Mac
// Responsible method: unknown
static const uint16 freddypharkasSignatureMacInventory[] = {
	SIG_MAGICDWORD,
	0x39, 0x23,                      // pushi 23
	0x39, 0x74,                      // pushi 74
	0x78,                            // push1
	0x38, SIG_UINT16(0x0174),        // pushi 0174 (on mac it's actually 0x01, 0x74)
	0x85, 0x15,                      // lat 15
	SIG_END
};

static const uint16 freddypharkasPatchMacInventory[] = {
	0x39, 0x02,                      // pushi 02 (now matches the DOS version)
	PATCH_ADDTOOFFSET(+23),
	0x39, 0x04,                      // pushi 04 (now matches the DOS version)
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry freddypharkasSignatures[] = {
	{  true,     0, "CD: score early disposal",                    1, freddypharkasSignatureScoreDisposal, freddypharkasPatchScoreDisposal },
	{  true,    15, "Mac: broken inventory",                       1, freddypharkasSignatureMacInventory,  freddypharkasPatchMacInventory },
	{  true,   110, "intro scaling workaround",                    2, freddypharkasSignatureIntroScaling,  freddypharkasPatchIntroScaling },
	{  true,   235, "CD: canister pickup hang",                    3, freddypharkasSignatureCanisterHang,  freddypharkasPatchCanisterHang },
	{  true,   320, "ladder event issue",                          2, freddypharkasSignatureLadderEvent,   freddypharkasPatchLadderEvent },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Hoyle 5

// Several scripts in Hoyle5 contain a subroutine which spins on kGetTime until
// a certain number of ticks elapse. Since this wastes CPU and makes ScummVM
// unresponsive, the kWait kernel function (which was removed in SCI2) is
// reintroduced for Hoyle5, and the spin subroutines are patched here to call
// that function instead.
// Applies to at least: English Demo
static const uint16 hoyle5SignatureSpinLoop[] = {
	SIG_MAGICDWORD,
	0x76,                         // push0
	0x43, 0x79, SIG_UINT16(0x00), // callk GetTime, $0
	0x36,                         // push
	0x87, 0x01,                   // lap param[1]
	0x02,                         // add
	0xa5, 0x00,                   // sat temp[0]
	SIG_END
};

static const uint16 hoyle5PatchSpinLoop[] = {
	0x78,                                     // push1
	0x8f, 0x01,                               // lsp param[1]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, $2
	0x48,                                     // ret
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry hoyle5Signatures[] = {
	{  true,     3, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,    23, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true,   500, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	{  true, 64937, "remove kGetTime spin",                        1, hoyle5SignatureSpinLoop,          hoyle5PatchSpinLoop },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 1

// ===========================================================================
// daySixBeignet::changeState (4) is called when the cop goes out and sets cycles to 220.
//  this is not enough time to get to the door, so we patch that to 23 seconds
// Applies to at least: English PC-CD, German PC-CD, English Mac
// Responsible method: daySixBeignet::changeState(4), script 230
static const uint16 gk1SignatureDay6PoliceBeignet1[] = {
	0x35, 0x04,                         // ldi 04
	0x1a,                               // eq?
	0x30, SIG_ADDTOOFFSET(+2),          // bnt [next state check]
	0x38, SIG_SELECTOR16(dispose),      // pushi dispose
	0x76,                               // push0
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa deskSarg
	0x4a, SIG_UINT16(0x0004),           // send 04
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x00dc),           // ldi 220
	0x65, SIG_ADDTOOFFSET(+1),          // aTop cycles (1a for PC, 1c for Mac)
	0x32,                               // jmp [end]
	SIG_END
};

static const uint16 gk1PatchDay6PoliceBeignet1[] = {
	PATCH_ADDTOOFFSET(+16),
	0x34, PATCH_UINT16(0x0017),         // ldi 23
	0x65, PATCH_GETORIGINALBYTEADJUST(+20, +2), // aTop seconds (1c for PC, 1e for Mac)
	PATCH_END
};

// sInGateWithPermission::changeState (0) is called, when the player walks through the swinging door.
//  When it's day 6 and the desk sergeant is outside for the beignet, this state will also set
//  daySixBeignet::cycles to basically reset the overall timer, which is 200.
//  This is not enough time to get to the door, so we patch that to 20 seconds
// Applies to at least: English PC-CD, German PC-CD, English Mac
// Responsible method: sInGateWithPermission::changeState(0), script 230
// Fixes bug: #9805
static const uint16 gk1SignatureDay6PoliceBeignet2[] = {
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa daySixBeignet
	0x1a,                               // eq?
	0x31, 0x0d,                         // bnt [skip set cycles]
	0x38, SIG_SELECTOR16(cycles),       // pushi (cycles)
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(200),              // pushi 200d
	0x72,                               // lofsa
	SIG_END
};

static const uint16 gk1PatchDay6PoliceBeignet2[] = {
	PATCH_ADDTOOFFSET(+6),
	0x38, PATCH_SELECTOR16(seconds),    // pushi (seconds)
	0x78,                               // push1
	0x38, PATCH_UINT16(20),             // pushi 20
	PATCH_END
};

// sargSleeping::changeState (8) is called when the cop falls asleep and sets cycles to 220.
//  this is not enough time to get to the door, so we patch it to 42 seconds
// Applies to at least: English PC-CD, German PC-CD, English Mac
// Responsible method: sargSleeping::changeState
static const uint16 gk1SignatureDay6PoliceSleep[] = {
	0x35, 0x08,                         // ldi 08
	0x1a,                               // eq?
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [next state check]
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x00dc),           // ldi 220
	0x65, SIG_ADDTOOFFSET(+1),          // aTop cycles (1a for PC, 1c for Mac)
	0x32,                               // jmp [end]
	SIG_END
};

static const uint16 gk1PatchDay6PoliceSleep[] = {
	PATCH_ADDTOOFFSET(+5),
	0x34, PATCH_UINT16(0x002a),         // ldi 42
	0x65, PATCH_GETORIGINALBYTEADJUST(+9, +2), // aTop seconds (1c for PC, 1e for Mac)
	PATCH_END
};

// At the start of day 5, there is like always some dialogue with Grace.
//
// The dialogue script code about the drum book + veve newspaper clip is a bit broken.
//
// In case the player already has the veve, but is supposed to get the drum book, then the drum book
// dialogue is repeated twice and the veve newspaper dialogue is also repeated (although it was played on day 4
// in such case already).
//
// Drum book dialogue is called twice.
// Once via GetTheVeve::changeState(0) and a second time via GetTheVeve::changeState(11).
//
// GetTheVeve::changeState(0) would also play the first line of the veve pattern newspaper and that's skipped,
// when the player is supposed to get the drum book.
// GetTheVeve::changeState(1) up to state 10 will do the dialogue about the veve newspaper.
// At the start of state 1 though, the player will get the drum book in case he ask for research.
// Right after that the scripts check, if the player has the drum book and then go the veve newspaper route.
//
// We fix this by skipping the drum book check in case the player just got the drum book.
// The scripts will then skip to state 12, skipping over the second drum book dialogue call.
//
// More notes: The veve newspaper item is inventory 9. The drum book is inventory 14.
//             The flag for veve research is 36, the flag for drum research is 73.
//
// This bug of course also occurs, when using the original interpreter.
//
// Special thanks, credits and kudos to sluicebox on IRC, who did a ton of research on this and even found this game bug originally.
//
// Applies to at least: English PC-CD, German PC-CD
// Responsible method: getTheVeve::changeState(1) - script 212
static const uint16 gk1SignatureDay5DrumBookDialogue[] = {
	0x31, 0x0b,                         // bnt [skip giving player drum book code]
	0x38, SIG_UINT16(0x0200),           // pushi 0200h
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x0e,                         // pushi 0Eh
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x06, 0x00,                   // send 06 - GKEgo::get(0Eh)
	// end of giving player drum book code
	0x38, SIG_UINT16(0x0202),           // pushi 0202h
	0x78,                               // push1
	0x39, 0x0e,                         // pushi 0Eh
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x06, 0x00,                   // send 06 - GKEgo::has(0Eh)
	0x18,                               // not
	0x30, SIG_UINT16(0x0025),           // bnt [veve newspaper code]
	SIG_END
};

static const uint16 gk1PatchDay5DrumBookDialogue[] = {
	0x31, 0x0d,                         // bnt [skip giving player drum book code] adjusted
	PATCH_ADDTOOFFSET(+11),             // skip give player drum book original code
	0x33, 0x0D,                         // jmp [over the check inventory for drum book code]
	// check inventory for drum book
	0x38, SIG_UINT16(0x0202),           // pushi 0202h
	0x78,                               // push1
	0x39, 0x0e,                         // pushi 0Eh
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x06, 0x00,                   // send 06 - GKEgo::has(0Eh)
	0x2f, 0x23,                         // bt [veve newspaper code] (adjusted, saves 2 bytes)
	PATCH_END
};

// startOfDay5::changeState (20h) - when gabriel goes to the phone the script will hang
// Applies to at least: English PC-CD, German PC-CD, English Mac
// Responsible method: startOfDay5::changeState
static const uint16 gk1SignatureDay5PhoneFreeze[] = {
	0x4a,
	SIG_MAGICDWORD, SIG_UINT16(0x000c), // send 0c
	0x35, 0x03,                         // ldi 03
	0x65, SIG_ADDTOOFFSET(+1),          // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),          // jmp [end]
	0x3c,                               // dup
	0x35, 0x21,                         // ldi 21
	SIG_END
};

static const uint16 gk1PatchDay5PhoneFreeze[] = {
	PATCH_ADDTOOFFSET(+3),
	0x35, 0x06,                         // ldi 01
	0x65, PATCH_GETORIGINALBYTEADJUST(+6, +6), // aTop ticks
	PATCH_END
};

// Floppy version: Interrogation::dispose() compares an object reference
// (stored in the view selector) with a number, leading to a crash (this kind
// of comparison was not used in SCI32). The view selector is used to store
// both a view number (in some cases), and a view reference (in other cases).
// In the floppy version, the checks are in the wrong order, so there is a
// comparison between a number an an object. In the CD version, the checks are
// in the correct order, thus the comparison is correct, thus we use the code
// from the CD version in the floppy one.
// Applies to at least: English Floppy
// Responsible method: Interrogation::dispose
// TODO: Check, if English Mac is affected too and if this patch applies
static const uint16 gk1SignatureInterrogationBug[] = {
	SIG_MAGICDWORD,
	0x65, 0x4c,                      // aTop 4c
	0x67, 0x50,                      // pTos 50
	0x34, SIG_UINT16(0x2710),        // ldi 2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 08  [05a0]
	0x67, 0x50,                      // pTos 50
	0x34, SIG_UINT16(0x2710),        // ldi 2710
	0x04,                            // sub
	0x65, 0x50,                      // aTop 50
	0x63, 0x50,                      // pToa 50
	0x31, 0x15,                      // bnt 15  [05b9]
	0x39, 0x0e,                      // pushi 0e
	0x76,                            // push0
	0x4a, SIG_UINT16(0x0004),        // send 0004
	0xa5, 0x00,                      // sat 00
	0x38, SIG_SELECTOR16(dispose),   // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa 50
	0x4a, SIG_UINT16(0x0004),        // send 0004
	0x85, 0x00,                      // lat 00
	0x65, 0x50,                      // aTop 50
	SIG_END
};

static const uint16 gk1PatchInterrogationBug[] = {
	0x65, 0x4c,                      // aTop 4c
	0x63, 0x50,                      // pToa 50
	0x31, 0x15,                      // bnt 15  [05b9]
	0x39, 0x0e,                      // pushi 0e
	0x76,                            // push0
	0x4a, 0x04, 0x00,                // send 0004
	0xa5, 0x00,                      // sat 00
	0x38, PATCH_SELECTOR16(dispose), // pushi dispose
	0x76,                            // push0
	0x63, 0x50,                      // pToa 50
	0x4a, 0x04, 0x00,                // send 0004
	0x85, 0x00,                      // lat 00
	0x65, 0x50,                      // aTop 50
	0x67, 0x50,                      // pTos 50
	0x34, PATCH_UINT16(0x2710),      // ldi 2710
	0x1e,                            // gt?
	0x31, 0x08,                      // bnt 08  [05b9]
	0x67, 0x50,                      // pTos 50
	0x34, PATCH_UINT16(0x2710),      // ldi 2710
	0x04,                            // sub
	0x65, 0x50,                      // aTop 50
	PATCH_END
};

// On day 10 nearly at the end of the game, Gabriel Knight dresses up and right after that
// someone will be at the door. Gabriel turns around to see what's going on.
//
// In ScummVM Gabriel turning around plays endlessly. This is caused by the loop of Gabriel
// being kept at 1, but view + cel were changed accordingly. The view used - which is view 859 -
// does not have a loop 1. kNumCels is called on that, BUT kNumCels in SSCI is broken in that
// regard. It checks for loop > count and not loop >= count and will return basically random data
// in case loop == count.
//
// In SSCI this simply worked by accident. kNumCels returned 0x53 in this case, but later script code
// fixed that up somehow, so it worked out in the end.
//
// The setup for this is done in SDJEnters::changeState(0). The cycler will never reach the goal
// because the goal will be cel -1, so it loops endlessly.
//
// We fix this by adding a setLoop(0).
//
// Applies to at least: English PC-CD, German PC-CD
// Responsible method: sDJEnters::changeState
static const uint16 gk1SignatureDay10GabrielDressUp[] = {
	0x87, 0x01,                         // lap param[1]
	0x65, 0x14,                         // aTop state
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x006f),           // bnt [next state 1]
	SIG_ADDTOOFFSET(+84),
	0x39, 0x0e,                         // pushi 0Eh (view)
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x035B),           // pushi 035Bh (859d)
	0x38, SIG_UINT16(0x0141),           // pushi 0141h (setCel)
	0x78,                               // push1
	0x76,                               // push0
	0x38, SIG_UINT16(0x00E9),           // pushi 00E9h (setCycle)
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x14, 0x00,                   // send 14h
										// GKEgo::view(859)
										// GKEgo::setCel(0)
										// GKEgo::setCycle(End, sDJEnters)
	0x32, SIG_UINT16(0x0233),           // jmp [ret]
	// next state
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt [next state 2]
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x0226),           // jmp [ret]
	// next state
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x2a,                         // bnt [next state 3]
	0x78,                               // push1
	SIG_ADDTOOFFSET(+34),
	// part of state 2 code, delays for 1 cycle
	0x35, 0x01,                         // ldi 1
	0x65, 0x1a,                         // aTop cycles
	SIG_END
};

static const uint16 gk1PatchDay10GabrielDressUp[] = {
	PATCH_ADDTOOFFSET(+9),
	0x30, SIG_UINT16(0x0073),           // bnt [next state 1] - offset adjusted
	SIG_ADDTOOFFSET(+84 + 11),
	// added by us: setting loop to 0 (5 bytes needed)
	0x38, SIG_UINT16(0x00FB),           // pushi 00FBh (setLoop)
	0x78,                               // push1
	0x76,                               // push0
	// original code, but offset changed
	0x38, SIG_UINT16(0x00E9),           // pushi 00E9h (setCycle)
	0x7a,                               // push2
	0x51, 0x18,                         // class End
	0x36,                               // push
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x1a, 0x00,                   // send 1Ah - adjusted
										// GKEgo::view(859)
										// GKEgo::setCel(0)
										// GKEgo::setLoop(0) <-- new, by us
										// GKEgo::setCycle(End, sDJEnters)
	// end of original code
	0x3a,                               // toss
	0x48,                               // ret (saves 1 byte)
	// state 1 code
	0x3c,                               // dup
	0x34, SIG_UINT16(0x0001),           // ldi 0001 (waste 1 byte)
	0x1a,                               // eq?
	0x31, 2,                            // bnt [next state 2]
	0x33, 41,                           // jmp to state 2 delay code
	SIG_ADDTOOFFSET(+41),
	// wait 2 cycles instead of only 1
	0x35, 0x02,                         // ldi 2
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry gk1Signatures[] = {
	{  true,    51, "interrogation bug",                           1, gk1SignatureInterrogationBug,     gk1PatchInterrogationBug },
	{  true,   212, "day 5 drum book dialogue error",              1, gk1SignatureDay5DrumBookDialogue, gk1PatchDay5DrumBookDialogue },
	{  true,   212, "day 5 phone freeze",                          1, gk1SignatureDay5PhoneFreeze,      gk1PatchDay5PhoneFreeze },
	{  true,   230, "day 6 police beignet timer issue 1/2",        1, gk1SignatureDay6PoliceBeignet1,   gk1PatchDay6PoliceBeignet1 },
	{  true,   230, "day 6 police beignet timer issue 2/2",        1, gk1SignatureDay6PoliceBeignet2,   gk1PatchDay6PoliceBeignet2 },
	{  true,   230, "day 6 police sleep timer issue",              1, gk1SignatureDay6PoliceSleep,      gk1PatchDay6PoliceSleep },
	{  true,   808, "day 10 gabriel dress up infinite turning",    1, gk1SignatureDay10GabrielDressUp,  gk1PatchDay10GabrielDressUp },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Gabriel Knight 2

// The down scroll button in GK2 jumps up a pixel on mousedown because there is
// a send to scrollSelections using an immediate value 1, which means to scroll
// up by 1 pixel. This patch fixes the send to scrollSelections by passing the
// button's delta instead of 1.
//
// Applies to at least: English CD 1.00, English Steam 1.01
// Responsible method: ScrollButton::track
static const uint16 gk2InvScrollSignature[] = {
	0x7e, SIG_ADDTOOFFSET(2),               // line whatever
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(scrollSelections), // pushi $2c3
	0x78,                                   // push1
	0x78,                                   // push1
	0x63, 0x98,                             // pToa $98
	0x4a, SIG_UINT16(0x06),                 // send $6
	SIG_END
};

static const uint16 gk2InvScrollPatch[] = {
	0x38, PATCH_SELECTOR16(scrollSelections), // pushi $2c3
	0x78,                                     // push1
	0x67, 0x9a,                               // pTos $9a (delta)
	0x63, 0x98,                               // pToa $98
	0x4a, PATCH_UINT16(0x06),                 // send $6
	0x18, 0x18,                               // waste bytes
	PATCH_END
};

// The init code that runs when GK2 starts up unconditionally resets the
// music volume to 63, but the game should always use the volume stored in
// ScummVM.
// Applies to at least: English 1.00 CD
static const uint16 gk2VolumeResetSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x3f, // ldi $3f
	0xa1, 0x4c, // sag $4c (music volume)
	SIG_END
};

static const uint16 gk2VolumeResetPatch[] = {
	0x33, 0x02,  // jmp 2 [past volume changes]
	PATCH_END
};

// GK2 has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature
static const uint16 gk2BenchmarkSignature[] = {
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class Actor
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x7e, SIG_ADDTOOFFSET(+2), // line
	0x39, 0x0e,                // pushi $e
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0xfdd4),  // pushi 64980
	SIG_END
};

static const uint16 gk2BenchmarkPatch[] = {
	0x38, PATCH_SELECTOR16(detailLevel), // pushi detailLevel
	0x78,                                // push1
	0x38, PATCH_UINT16(399),             // pushi 10000 / 25 - 1
	0x81, 0x01,                          // lag 1
	0x4a, PATCH_UINT16(0x06),            // send 6
	0x34, PATCH_UINT16(10000),           // ldi 10000
	0x48,                                // ret
	PATCH_END
};

//          script, description,                                              signature                         patch
static const SciScriptPatcherEntry gk2Signatures[] = {
	{  true,     0, "disable volume reset on startup",                     1, gk2VolumeResetSignature,          gk2VolumeResetPatch },
	{  true,     0, "disable video benchmarking",                          1, gk2BenchmarkSignature,            gk2BenchmarkPatch },
	{  true,    23, "inventory starts scroll down in the wrong direction", 1, gk2InvScrollSignature,            gk2InvScrollPatch },
	{  true, 64990, "increase number of save games",                       1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",                       1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",                     1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// at least during harpy scene export 29 of script 0 is called in kq5cd and
//  has an issue for those calls, where temp 3 won't get inititialized, but
//  is later used to set master volume. This issue makes sierra sci set
//  the volume to max. We fix the export, so volume won't get modified in
//  those cases.
static const uint16 kq5SignatureCdHarpyVolume[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0191),        // lag global[191h]
	0x18,                            // not
	0x30, SIG_UINT16(0x002c),        // bnt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, SIG_UINT16(0x0191),        // sag global[191h] (setting global 191h to 1)
	0x38, SIG_UINT16(0x017b),        // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume in temp 3)
	0x38, SIG_UINT16(0x017b),        // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0x36,                            // push
	0x35, 0x04,                      // ldi 04
	0x20,                            // ge? (followed by bnt)
	SIG_END
};

static const uint16 kq5PatchCdHarpyVolume[] = {
	0x38, PATCH_UINT16(0x022f),      // pushi 022f (selector theVol) (3 new bytes)
	0x76,                            // push0 (1 new byte)
	0x51, 0x88,                      // class SpeakTimer (2 new bytes)
	0x4a, 0x04,                      // send 04 (2 new bytes) -> read SpeakTimer::theVol
	0xa5, 0x03,                      // sat temp[3] (2 new bytes) -> write to temp 3
	0x80, PATCH_UINT16(0x0191),      // lag global[191h]
	// saving 1 byte due optimization
	0x2e, PATCH_UINT16(0x0023),      // bt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,                      // ldi 01
	0xa0, PATCH_UINT16(0x0191),      // sag global[191h] (setting global 191h to 1)
	0x38, PATCH_UINT16(0x017b),      // pushi 017b
	0x76,                            // push0
	0x81, 0x01,                      // lag global[1]
	0x4a, 0x04,                      // send 04 - read KQ5::masterVolume
	0xa5, 0x03,                      // sat temp[3] (store volume in temp 3)
	// saving 8 bytes due removing of duplicate code
	0x39, 0x04,                      // pushi 04 (saving 1 byte due swapping)
	0x22,                            // lt? (because we switched values)
	PATCH_END
};

// This is a heap patch, and it modifies the properties of an object, instead
// of patching script code.
//
// The witchCage object in script 200 is broken and claims to have 12
// variables instead of the 8 it should have because it is a Cage.
// Additionally its top,left,bottom,right properties are set to 0 rather
// than the right values. We fix the object by setting the right values.
// If they are all zero, this causes an impossible position check in
// witch::cantBeHere and an infinite loop when entering room 22.
//
// This bug is accidentally not triggered in SSCI because the invalid number
// of variables effectively hides witchCage::doit, causing this position check
// to be bypassed entirely.
// See also the warning+comment in Object::initBaseObject
//
// Fixes bug: #4964
static const uint16 kq5SignatureWitchCageInit[] = {
	SIG_UINT16(0x0000),         // top
	SIG_UINT16(0x0000),         // left
	SIG_UINT16(0x0000),         // bottom
	SIG_UINT16(0x0000),         // right
	SIG_UINT16(0x0000),         // extra property #1
	SIG_MAGICDWORD,
	SIG_UINT16(0x007a),         // extra property #2
	SIG_UINT16(0x00c8),         // extra property #3
	SIG_UINT16(0x00a3),         // extra property #4
	SIG_END
};

static const uint16 kq5PatchWitchCageInit[] = {
	PATCH_UINT16(0x0000),       // top
	PATCH_UINT16(0x007a),       // left
	PATCH_UINT16(0x00c8),       // bottom
	PATCH_UINT16(0x00a3),       // right
	PATCH_END
};

// The multilingual releases of KQ5 hang right at the end during the magic battle with Mordack.
// It seems additional code was added to wait for signals, but the signals are never set and thus
// the game hangs. We disable that code, so that the battle works again.
// This also happened in the original interpreter.
// We must not change similar code, that happens before.

// Applies to at least: French PC floppy, German PC floppy, Spanish PC floppy
// Responsible method: stingScript::changeState, dragonScript::changeState, snakeScript::changeState
static const uint16 kq5SignatureMultilingualEndingGlitch[] = {
	SIG_MAGICDWORD,
	0x89, 0x57,                      // lsg global[57h]
	0x35, 0x00,                      // ldi 0
	0x1a,                            // eq?
	0x18,                            // not
	0x30, SIG_UINT16(0x0011),        // bnt [skip signal check]
	SIG_ADDTOOFFSET(+8),             // skip globalSound::prevSignal get code
	0x36,                            // push
	0x35, 0x0a,                      // ldi 0Ah
	SIG_END
};

static const uint16 kq5PatchMultilingualEndingGlitch[] = {
	PATCH_ADDTOOFFSET(+6),
	0x32,                            // change BNT into JMP
	PATCH_END
};

// In the final battle, the DOS version uses signals in the music to handle
// timing, while in the Windows version another method is used and the GM
// tracks do not contain these signals.
// The original kq5 interpreter used global 400 to distinguish between
// Windows (1) and DOS (0) versions.
// We replace the 4 relevant checks for global 400 by a fixed true when
// we use these GM tracks.
//
// Instead, we could have set global 400, but this has the possibly unwanted
// side effects of switching to black&white cursors (which also needs complex
// changes to GameFeatures::detectsetCursorType() ) and breaking savegame
// compatibilty between the DOS and Windows CD versions of KQ5.
// TODO: Investigate these side effects more closely.
static const uint16 kq5SignatureWinGMSignals[] = {
	SIG_MAGICDWORD,
	0x80, SIG_UINT16(0x0190),        // lag 0x190
	0x18,                            // not
	0x30, SIG_UINT16(0x001b),        // bnt +0x001B
	0x89, 0x57,                      // lsg 0x57
	SIG_END
};

static const uint16 kq5PatchWinGMSignals[] = {
	0x34, PATCH_UINT16(0x0001),      // ldi 0x0001
	PATCH_END
};

//          script, description,                                      signature                  patch
static const SciScriptPatcherEntry kq5Signatures[] = {
	{  true,     0, "CD: harpy volume change",                     1, kq5SignatureCdHarpyVolume,            kq5PatchCdHarpyVolume },
	{  true,   200, "CD: witch cage init",                         1, kq5SignatureWitchCageInit,            kq5PatchWitchCageInit },
	{  true,   124, "Multilingual: Ending glitching out",          3, kq5SignatureMultilingualEndingGlitch, kq5PatchMultilingualEndingGlitch },
	{ false,   124, "Win: GM Music signal checks",                 4, kq5SignatureWinGMSignals,             kq5PatchWinGMSignals },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// When giving the milk bottle to one of the babies in the garden in KQ6 (room
// 480), script 481 starts a looping baby cry sound. However, that particular
// script also has an overriden check method (cryMusic::check). This method
// explicitly restarts the sound, even if it's set to be looped, thus the same
// sound is played twice, squelching all other sounds. We just rip the
// unnecessary cryMusic::check method out, thereby stopping the sound from
// constantly restarting (since it's being looped anyway), thus the normal
// game speech can work while the baby cry sound is heard.
// Fixes bug: #4955
static const uint16 kq6SignatureDuplicateBabyCry[] = {
	SIG_MAGICDWORD,
	0x83, 0x00,                      // lal 00
	0x31, 0x1e,                      // bnt 1e  [07f4]
	0x78,                            // push1
	0x39, 0x04,                      // pushi 04
	0x43, 0x75, 0x02,                // callk DoAudio[75] 02
	SIG_END
};

static const uint16 kq6PatchDuplicateBabyCry[] = {
	0x48,                            // ret
	PATCH_END
};

// The inventory of King's Quest 6 is buggy. When it grows too large,
//  it will get split into 2 pages. Switching between those pages will
//  grow the stack, because it's calling itself per switch.
// Which means after a while ScummVM will bomb out because the stack frame
//  will be too large. This patch fixes the buggy script.
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: KqInv::showSelf
// Fixes bug: #5681
static const uint16 kq6SignatureInventoryStackFix[] = {
	0x67, 0x30,                         // pTos state
	0x34, SIG_UINT16(0x2000),           // ldi 2000
	0x12,                               // and
	0x18,                               // not
	0x31, 0x04,                         // bnt [not first refresh]
	0x35, 0x00,                         // ldi 00
	SIG_MAGICDWORD,
	0x65, 0x1e,                         // aTop curIcon
	0x67, 0x30,                         // pTos state
	0x34, SIG_UINT16(0xdfff),           // ldi dfff
	0x12,                               // and
	0x65, 0x30,                         // aTop state
	0x38, SIG_SELECTOR16(show),         // pushi "show" ("show" is e1h for KQ6CD)
	0x78,                               // push1
	0x87, 0x00,                         // lap param[0]
	0x31, 0x04,                         // bnt [use global for show]
	0x87, 0x01,                         // lap param[1]
	0x33, 0x02,                         // jmp [use param for show]
	0x81, 0x00,                         // lag global[0]
	0x36,                               // push
	0x54, 0x06,                         // self 06 (KqInv::show)
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [exit menu code] (0x08 for PC, 0x07 for mac)
	0x39, 0x39,                         // pushi 39
	0x76,                               // push0
	0x54, 0x04,                         // self 04 (KqInv::doit)
	SIG_END                             // followed by jmp (0x32 for PC, 0x33 for mac)
};

static const uint16 kq6PatchInventoryStackFix[] = {
	0x67, 0x30,                         // pTos state
	0x3c,                               // dup (1 more byte, needed for patch)
	0x3c,                               // dup (1 more byte, saves 1 byte later)
	0x34, PATCH_UINT16(0x2000),         // ldi 2000
	0x12,                               // and
	0x2f, 0x02,                         // bt [not first refresh] - saves 3 bytes in total
	0x65, 0x1e,                         // aTop curIcon
	0x00,                               // neg (either 2000 or 0000 in acc, this will create dfff or ffff) - saves 2 bytes
	0x12,                               // and
	0x65, 0x30,                         // aTop state
	0x38,                               // pushi "show"
	PATCH_GETORIGINALUINT16(+22),
	0x78,                               // push1
	0x87, 0x00,                         // lap param[0]
	0x31, 0x04,                         // bnt [call show using global 0]
	0x8f, 0x01,                         // lsp param[1], save 1 byte total with lsg global[0] combined
	0x33, 0x02,                         // jmp [call show using param 1]
	0x89, 0x00,                         // lsg global[0], save 1 byte total, see above
	0x54, 0x06,                         // self 06 (call x::show)
	0x31,                               // bnt [menu exit code]
	PATCH_GETORIGINALBYTEADJUST(+39, +6),// dynamic offset must be 0x0E for PC and 0x0D for mac
	0x34, PATCH_UINT16(0x2000),         // ldi 2000
	0x12,                               // and
	0x2f, 0x05,                         // bt [to return]
	0x39, 0x39,                         // pushi 39
	0x76,                               // push0
	0x54, 0x04,                         // self 04 (self::doit)
	0x48,                               // ret (saves 2 bytes for PC, 1 byte for mac)
	PATCH_END
};

// The "Drink Me" bottle code doesn't repaint the AddToPics elements to the screen,
//  when Alexander returns back from the effect of the bottle.
//  It's pretty strange that Sierra didn't find this bug, because it occurs when
//  drinking the bottle right on the screen, where the bottle is found.
// This bug also occurs in Sierra SCI.
// Applies to at least: PC-CD, English PC floppy, German PC floppy, English Mac
// Responsible method: drinkMeScript::changeState
// Fixes bug: #5252
static const uint16 kq6SignatureDrinkMeFix[] = {
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x0f,                         // ldi 0f
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x00a4),           // bnt [skip to next check]
	SIG_ADDTOOFFSET(+161),
	0x32, SIG_UINT16(0x007f),           // jmp [return]
	0x3c,                               // dup
	0x35, 0x10,                         // ldi 10
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt [skip to next check]
	0x35, 0x03,                         // ldi 03
	0x65, 0x1a,                         // aTop (cycles)
	0x32, SIG_UINT16(0x0072),           // jmp [return]
	0x3c,                               // dup
	0x35, 0x11,                         // ldi 11
	0x1a,                               // eq?
	0x31, 0x13,                         // bnt [skip to next check]
	SIG_ADDTOOFFSET(+20),
	0x35, 0x12,                         // ldi 12
	SIG_ADDTOOFFSET(+23),
	0x35, 0x13,                         // ldi 13
	SIG_END
};

static const uint16 kq6PatchDrinkMeFix[] = {
	PATCH_ADDTOOFFSET(+5),              // skip to bnt offset
	PATCH_GETORIGINALBYTEADJUST(+5, +13), // adjust jump to [check for 11h code]
	PATCH_ADDTOOFFSET(+162),
	0x39, PATCH_SELECTOR8(doit),        // pushi (doit)
	0x76,                               // push0
	0x81, 0x0a,                         // lag 0a
	0x4a, 0x04,                         // send 04 (call addToPics::doit)
	0x3a,                               // toss
	0x48,                               // ret
	PATCH_ADDTOOFFSET(+8),              // skip to check 11h code
	0x35, 0x10,                         // ldi 10 instead of 11
	PATCH_ADDTOOFFSET(+23),             // skip to check 12h code
	0x35, 0x11,                         // ldi 11 instead of 12
	PATCH_ADDTOOFFSET(+23),             // skip to check 13h code
	0x35, 0x12,                         // ldi 12 instead of 13
	PATCH_END
};

// During the common Game Over cutscene, one of the guys says "Tickets, only",
// but the subtitle says "Tickets, please".
// Normally people wouldn't have noticed, but ScummVM supports audio + subtitles
// in this game at the same time.
// This is caused by a buggy message, which really has this text + audio attached.
// We assume that "Tickets, only" (the audio) is the correct one and there is a
// message with "Tickets, only" in both text and audio.
// We change message 1, 0, 1, 1 to message 5, 0, 0, 2 to fix this issue.
//
// This mismatch also occurs in Sierra SCI.
// Applies to at least: PC-CD
// Responsible method: modeLessScript::changeState(0)
static const uint16 kq6SignatureTicketsOnly[] = {
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	SIG_MAGICDWORD,
	0x31, 0x2b,                         // bnt [skip over state 0]
	0x39, 0x1e,                         // pushi (font) (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, SIG_UINT16(0x009a),           // pushi (posn)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c8),           // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, SIG_UINT16(0x00ab),           // pushi (say)
	0x39, 0x05,                         // pushi 05 (parameter count for say)
	0x76,                               // push0
	0x78,                               // push1
	0x76,                               // push0
	0x78,                               // push1
	0x78,                               // push1
	SIG_END
};

static const uint16 kq6PatchTicketsOnly[] = {
	0x32, PATCH_UINT16(0x0000),         // jmp (waste 3 bytes)
	0x2f, 0x2c,                         // bt [skip over state 0] (saves 1 byte)
	0x39, 0x1e,                         // pushi (font) (we keep the hardcoded selectors in here simply because this is only for KQ6-CD)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h]
	0x38, PATCH_UINT16(0x009a),         // pushi (posn)
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x00c8),         // pushi 00c8h (200d)
	0x39, 0x64,                         // pushi 64h (100d)
	0x38, PATCH_UINT16(0x00ab),         // pushi (say)
	0x39, 0x05,                         // pushi 05 (parameter count for say)
	0x76,                               // push0
	0x39, 0x05,                         // pushi 05
	0x76,                               // push0
	0x76,                               // push0
	0x7a,                               // push2
	PATCH_END
};

// Audio + subtitles support - SHARED! - used for King's Quest 6 and Laura Bow 2
//  this patch gets enabled, when the user selects "both" in the ScummVM "Speech + Subtitles" menu
//  We currently use global 98d to hold a kMemory pointer.
// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (always use text branch)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport1[] = {
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	SIG_MAGICDWORD,
	0x31, 0x13,                         // bnt [audio call]
	0x38, SIG_SELECTOR16(modNum),       // pushi modNum
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport1[] = {
	PATCH_ADDTOOFFSET(+5),
	0x33, 0x13,                         // jmp [audio call]
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (allocate audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport2[] = {
	0x7a,                               // push2
	0x78,                               // push1
	0x39, 0x0c,                         // pushi 0c
	0x43, SIG_MAGICDWORD, 0x72, 0x04,   // kMemory
	0xa5, 0xc9,                         // sat global[c9]
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport2[] = {
	PATCH_ADDTOOFFSET(+7),
	0xa1, 98,                           // sag global[98d]
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Messager::sayNext / lb2Messager::sayNext (release audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport3[] = {
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	SIG_MAGICDWORD,
	0x8d, 0xc9,                         // lst temp[c9]
	0x43, 0x72, 0x04,                   // kMemory
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport3[] = {
	PATCH_ADDTOOFFSET(+3),
	0x89, 98,                           // lsg global[98d]
	PATCH_END
};

// startText call gets acc = 0 for text-only and acc = 2 for audio+text
// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Narrator::say (use audio memory)
static const uint16 kq6laurabow2CDSignatureAudioTextSupport4[] = {
	// set caller property code
	0x31, 0x08,                         // bnt [set acc to 0 for caller]
	0x87, 0x02,                         // lap param[2]
	0x31, 0x04,                         // bnt [set acc to 0 for caller]
	0x87, 0x02,                         // lap param[2]
	0x33, 0x02,                         // jmp [set caller]
	0x35, 0x00,                         // ldi 00
	0x65, 0x68,                         // aTop caller
	// call startText + startAudio code
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x12,                               // and
	0x31, 0x08,                         // bnt [skip code]
	0x38, SIG_SELECTOR16(startText),    // pushi startText
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x54, 0x06,                         // self 06
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31, 0x08,                         // bnt [skip code]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(startAudio),   // pushi startAudio
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[1]
	0x54, 0x06,                         // self 06
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport4[] = {
	0x31, 0x02,                         // bnt [set caller]
	0x87, 0x02,                         // lap param[2]
	0x65, 0x68,                         // aTop caller
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x12,                               // and
	0x31, 0x11,                         // bnt [skip startText code]
	0x81, 0x5a,                         // lag global[5a]
	0x7a,                               // push2
	0x12,                               // and
	0x33, 0x03,                         // skip over 3 unused bytes
	PATCH_ADDTOOFFSET(+22),
	0x89, 98,                           // lsp global[98d]
	PATCH_END
};

// Applies to at least: KQ6 PC-CD, LB2 PC-CD
// Patched method: Talker::display/Narrator::say (remove reset saved mouse cursor code)
//  code would screw over mouse cursor
static const uint16 kq6laurabow2CDSignatureAudioTextSupport5[] = {
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 00
	0x65, 0x82,                         // aTop saveCursor
	SIG_END
};

static const uint16 kq6laurabow2CDPatchAudioTextSupport5[] = {
	0x18, 0x18, 0x18, 0x18,             // waste bytes, do nothing
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes text window placement, when in "dual" mode
// Applies to at least: PC-CD
// Patched method: Kq6Talker::init
static const uint16 kq6CDSignatureAudioTextSupport1[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, SIG_ADDTOOFFSET(+1),          // bnt [jump-to-text-code]
	0x78,                               // push1
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport1[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes low-res portrait staying on screen for hi-res mode
// Applies to at least: PC-CD
// Patched method: Talker::startText
//  this method is called by Narrator::say and acc is 0 for text-only and 2 for dual mode (audio+text)
static const uint16 kq6CDSignatureAudioTextSupport2[] = {
	SIG_MAGICDWORD,
	0x3f, 0x01,                         // link 01
	0x63, 0x8a,                         // pToa viewInPrint
	0x18,                               // not
	0x31, 0x06,                         // bnt [skip following code]
	0x38, SIG_UINT16(0x00e1),           // pushi 00e1
	0x76,                               // push0
	0x54, 0x04,                         // self 04
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport2[] = {
	PATCH_ADDTOOFFSET(+2),
	0x67, 0x8a,                         // pTos viewInPrint
	0x14,                               // or
	0x2f,                               // bt [skip following code]
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes special windows, used for example in the Pawn shop (room 280),
//   when the man in a robe complains about no more mints.
//   Or also in room 300 at the cliffs (aka copy protection), when Alexander falls down the cliffs (closes automatically, but too late).
//   Or in room 210, when Alexander gives the ring to the nightingale (these ones will need a mouse click).
//
//  We have to change even more code, because the game uses PODialog class for
//   text windows and myDialog class for audio. Both are saved to KQ6Print::dialog
//
//  Changing KQ6Print::dialog is disabled for now, because it has side-effects (breaking game over screens)
//
//  Original comment:
//  Sadly PODialog is created during KQ6Print::addText, myDialog is set during
//   KQ6Print::showSelf, which is called much later and KQ6Print::addText requires
//   KQ6Print::dialog to be set, which means we have to set it before calling addText
//   for audio mode, otherwise the user would have to click to get those windows disposed.
//
// Applies to at least: PC-CD
// Patched method: KQ6Print::say
static const uint16 kq6CDSignatureAudioTextSupport3[] = {
	0x31, 0x6e,                         // bnt [to text code]
	SIG_ADDTOOFFSET(+85),
	SIG_MAGICDWORD,
	0x8f, 0x01,                         // lsp param[1]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt [code to set property repressText to 1]
	0x38,                               // pushi (selector addText)
	SIG_ADDTOOFFSET(+9),                // skip addText-calling code
	0x33, 0x10,                         // jmp [to ret]
	0x35, 0x01,                         // ldi 01
	0x65, 0x2e,                         // aTop repressText
	0x33, 0x0a,                         // jmp [to ret]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport3[] = {
	0x31, 0x68,                         // adjust jump to reuse audio mode addText-calling code
	PATCH_ADDTOOFFSET(+85),             // right at the MAGIC_DWORD
	// check, if text is supposed to be shown. If yes, skip the follow-up check (param[1])
	0x89, 0x5a,                         // lsg global[5Ah]
	0x35, 0x01,                         // ldi 01
	0x12,                               // and
	0x2f, 0x07,                         // bt [skip over param check]
	// original code, checks param[1]
	0x8f, 0x01,                         // lsp param[1]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x10,                         // bnt [code to set property repressText to 1], adjusted
	// waste 5 bytes instead of using myDialog class for now
	// setting myDialog class all the time causes game over screens to misbehave (bug #9771)
	0x34, 0x00, 0x00,
	0x35, 0x00,
	// use myDialog class, so that text box automatically disappears (this is not done for text only mode, like in the original)
	//0x72, 0x0e, 0x00,                   // lofsa myDialog
	//0x65, 0x12,                         // aTop dialog
	// followed by original addText-calling code
	0x38,
	PATCH_GETORIGINALUINT16(+95),       // pushi (addText)
	0x78,                               // push1
	0x8f, 0x02,                         // lsp param[2]
	0x59, 0x03,                         // &rest 03
	0x54, 0x06,                         // self 06
	0x48,                               // ret

	0x35, 0x01,                         // ldi 01
	0x65, 0x2e,                         // aTop repressText
	0x48,                               // ret
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Fixes text-window size for hires portraits mode
//   Otherwise at least at the end some text-windows will be way too small
// Applies to at least: PC-CD
// Patched method: Talker::init
static const uint16 kq6CDSignatureAudioTextSupport4[] = {
	SIG_MAGICDWORD,
	0x63, 0x94,                         // pToa raving
	0x31, 0x0a,                         // bnt [no rave code]
	0x35, 0x00,                         // ldi 00
	SIG_ADDTOOFFSET(6),                 // skip reset of bust, eyes and mouth
	0x33, 0x24,                         // jmp [to super class code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupport4[] = {
	PATCH_ADDTOOFFSET(+12),
	0x33, PATCH_GETORIGINALBYTEADJUST(+13, -6), // adjust jump to also include setSize call
	PATCH_END
};

//  Fixes text window placement, when dual mode is active (Guards in room 220)
// Applies to at least: PC-CD
// Patched method: tlkGateGuard1::init & tlkGateGuard2::init
static const uint16 kq6CDSignatureAudioTextSupportGuards[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	SIG_END                             // followed by bnt for Guard1 and bt for Guard2
};

static const uint16 kq6CDPatchAudioTextSupportGuards[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x02,                         // ldi 02
	0x1c,                               // ne?
	PATCH_END
};

//  Fixes text window placement, when portrait+text is shown (Stepmother in room 250)
// Applies to at least: PC-CD
// Patched method: tlkStepmother::init
static const uint16 kq6CDSignatureAudioTextSupportStepmother[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportJumpAlways[] = {
	PATCH_ADDTOOFFSET(+4),
	0x1a,                               // eq?
	PATCH_END
};

//  Fixes "Girl In The Tower" to get played in dual mode as well
//  Also changes credits to use CD audio for dual mode.
//
// Applies to at least: PC-CD
// Patched method: rm740::cue (script 740), sCredits::init (script 52)
static const uint16 kq6CDSignatureAudioTextSupportGirlInTheTower[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportGirlInTheTower[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

//  Fixes dual mode for scenes with Azure and Ariel (room 370)
//   Effectively same patch as the one for fixing "Girl In The Tower"
// Applies to at least: PC-CD
// Patched methods: rm370::init, caughtAtGateCD::changeState, caughtAtGateTXT::changeState, toLabyrinth::changeState
// Fixes bug: #6750
static const uint16 kq6CDSignatureAudioTextSupportAzureAriel[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31,                               // bnt [jump-for-text-code]
	SIG_END
};

static const uint16 kq6CDPatchAudioTextSupportAzureAriel[] = {
	PATCH_ADDTOOFFSET(+4),
	0x12,                               // and
	PATCH_END
};

// Additional patch specifically for King's Quest 6
//  Adds another button state for the text/audio button. We currently use the "speech" view for "dual" mode.
// View 947, loop 9, cel 0+1 -> "text"
// View 947, loop 8, cel 0+1 -> "speech"
// View 947, loop 12, cel 0+1 -> "dual" (this view is injected by us into the game)
// Applies to at least: PC-CD
// Patched method: iconTextSwitch::show, iconTextSwitch::doit
static const uint16 kq6CDSignatureAudioTextMenuSupport[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt [set text view]
	0x35, 0x08,                         // ldi 08
	0x65, 0x14,                         // aTop loop
	0x33, 0x04,                         // jmp [skip over text view]
	0x35, 0x09,                         // ldi 09
	0x65, 0x14,                         // aTop loop
	SIG_ADDTOOFFSET(+102),              // skip to iconTextSwitch::doit code
	0x89, 0x5a,                         // lsg global[5a]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x06,                         // bnt [set text mode]
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	0x33, 0x0a,                         // jmp [skip over text mode code]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [skip over text ode code]
	0x35, 0x01,                         // ldi 01
	0xa1, 0x5a,                         // sag global[5a]
	0x3a,                               // toss
	0x67, 0x14,                         // pTos loop
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [set text view]
	0x35, 0x08,                         // ldi 08
	0x33, 0x02,                         // jmp [skip text view]
	0x35, 0x09,                         // ldi 09
	0x65, 0x14,                         // aTop loop
	SIG_END
};

static const uint16 kq6CDPatchAudioTextMenuSupport[] = {
	PATCH_ADDTOOFFSET(+13),
	0x33, 0x79,                         // jmp to new text+dual code
	PATCH_ADDTOOFFSET(+104),            // seek to iconTextSwitch::doit
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x02,                               // add
	0xa1, 0x5a,                         // sag global[5a]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x1e,                               // gt?
	0x31, 0x03,                         // bnt [skip over]
	0x78,                               // push1
	0xa9, 0x5a,                         // ssg global[5a]
	0x33, 0x17,                         // jmp [iconTextSwitch::show call]
	// additional code for iconTextSwitch::show
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x04,                         // bnt [dual mode]
	0x35, 0x09,                         // ldi 09
	0x33, 0x02,                         // jmp [skip over dual mode]
	0x35, 0x0c,                         // ldi 0c (view 947, loop 12, cel 0+1 is our "dual" view, injected by view.cpp)
	0x65, 0x14,                         // aTop loop
	0x32, PATCH_UINT16(0xff75),         // jmp [back to iconTextSwitch::show]
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq6Signatures[] = {
	{  true,   481, "duplicate baby cry",                          1, kq6SignatureDuplicateBabyCry,             kq6PatchDuplicateBabyCry },
	{  true,   907, "inventory stack fix",                         1, kq6SignatureInventoryStackFix,            kq6PatchInventoryStackFix },
	{  true,    87, "Drink Me bottle fix",                         1, kq6SignatureDrinkMeFix,                   kq6PatchDrinkMeFix },
	{  true,   640, "Tickets, only fix",                           1, kq6SignatureTicketsOnly,                  kq6PatchTicketsOnly },
	// King's Quest 6 and Laura Bow 2 share basic patches for audio + text support
	// *** King's Quest 6 audio + text support ***
	{ false,   924, "CD: audio + text support KQ6&LB2 1",             1, kq6laurabow2CDSignatureAudioTextSupport1,     kq6laurabow2CDPatchAudioTextSupport1 },
	{ false,   924, "CD: audio + text support KQ6&LB2 2",             1, kq6laurabow2CDSignatureAudioTextSupport2,     kq6laurabow2CDPatchAudioTextSupport2 },
	{ false,   924, "CD: audio + text support KQ6&LB2 3",             1, kq6laurabow2CDSignatureAudioTextSupport3,     kq6laurabow2CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support KQ6&LB2 4",             1, kq6laurabow2CDSignatureAudioTextSupport4,     kq6laurabow2CDPatchAudioTextSupport4 },
	{ false,   928, "CD: audio + text support KQ6&LB2 5",             2, kq6laurabow2CDSignatureAudioTextSupport5,     kq6laurabow2CDPatchAudioTextSupport5 },
	{ false,   909, "CD: audio + text support KQ6 1",                 2, kq6CDSignatureAudioTextSupport1,              kq6CDPatchAudioTextSupport1 },
	{ false,   928, "CD: audio + text support KQ6 2",                 1, kq6CDSignatureAudioTextSupport2,              kq6CDPatchAudioTextSupport2 },
	{ false,   104, "CD: audio + text support KQ6 3",                 1, kq6CDSignatureAudioTextSupport3,              kq6CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support KQ6 4",                 1, kq6CDSignatureAudioTextSupport4,              kq6CDPatchAudioTextSupport4 },
	{ false,  1009, "CD: audio + text support KQ6 Guards",            2, kq6CDSignatureAudioTextSupportGuards,         kq6CDPatchAudioTextSupportGuards },
	{ false,  1027, "CD: audio + text support KQ6 Stepmother",        1, kq6CDSignatureAudioTextSupportStepmother,     kq6CDPatchAudioTextSupportJumpAlways },
	{ false,    52, "CD: audio + text support KQ6 Girl In The Tower", 1, kq6CDSignatureAudioTextSupportGirlInTheTower, kq6CDPatchAudioTextSupportGirlInTheTower },
	{ false,   740, "CD: audio + text support KQ6 Girl In The Tower", 1, kq6CDSignatureAudioTextSupportGirlInTheTower, kq6CDPatchAudioTextSupportGirlInTheTower },
	{ false,   370, "CD: audio + text support KQ6 Azure & Ariel",     6, kq6CDSignatureAudioTextSupportAzureAriel,     kq6CDPatchAudioTextSupportAzureAriel },
	{ false,   903, "CD: audio + text support KQ6 menu",              1, kq6CDSignatureAudioTextMenuSupport,           kq6CDPatchAudioTextMenuSupport },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Kings Quest 7

// ===========================================================================

// King's Quest 7 has really weird subtitles. It seems as if the subtitles were
// not fully finished.
//
// Method kqMessager::findTalker in script 0 tries to figure out, which class to use for
// displaying subtitles. It uses the "talker" data of the given message to do that.
// Strangely this "talker" data seems to be quite broken.
// For example chapter 2 starts with a cutscene.
// Troll king: "Welcome, most beautiful of princesses!" - talker 6
// Which is followed by the princess going
// "Hmm?" - which is set to talker 99, normally the princess is talker 7.
//
// Talker 99 is seen as unknown and thus treated as "narrator", which makes
// the scripts put the text at the top of the game screen and even use a
// different font.
//
// In other cases, when the player character thinks to himself talker 99
// is also used. In such situations it may make somewhat sense to do so,
// but putting the text at the top of the screen is also irritating to the player.
// It's really weird.
//
// The scripts also put the regular text in the middle of the screen, blocking
// animations.
//
// And for certain rooms, the subtitle box may use another color
// like for example pink/purple at the start of chapter 5.
//
// We fix all of that (hopefully - lots of testing is required).
// We put the text at the bottom of the play screen.
// We also make the scripts use the regular KQTalker instead of KQNarrator.
// And we also make the subtitle box use color 255, which is fixed white.
//
// Applies to at least: PC CD 1.4 English, 1.51 English, 1.51 German, 2.00 English
// Patched method: KQNarrator::init (script 31)
static const uint16 kq7SignatureSubtitleFix1[] = {
	SIG_MAGICDWORD,
	0x39, 0x25,                         // pushi 25h (fore)
	0x78,                               // push1
	0x39, 0x06,                         // pushi 06 - sets back to 6
	0x39, 0x26,                         // pushi 26 (back)
	0x78,                               // push1
	0x78,                               // push1 - sets back to 1
	0x39, 0x2a,                         // pushi 2Ah (font)
	0x78,                               // push1
	0x89, 0x16,                         // lsg global[16h] - sets font to global[16h]
	0x7a,                               // push2 (y)
	0x78,                               // push1
	0x76,                               // push0 - sets y to 0
	0x54, SIG_UINT16(0x0018),           // self 18h
	SIG_END
};

static const uint16 kq7PatchSubtitleFix1[] = {
	0x33, 0x12,                         // jmp [skip special init code]
	PATCH_END
};

// Applies to at least: PC CD 1.51 English, 1.51 German, 2.00 English
// Patched method: Narrator::init (script 64928)
static const uint16 kq7SignatureSubtitleFix2[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x12,                               // and
	0x31, 0x1e,                         // bnt [skip audio volume code]
	0x38, SIG_ADDTOOFFSET(+2),          // pushi masterVolume (0212h for 2.00, 0219h for 1.51)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x04, 0x00,                   // send 04
	0x65, 0x32,                         // aTop curVolume
	0x38, SIG_ADDTOOFFSET(+2),          // pushi masterVolume (0212h for 2.00, 0219h for 1.51)
	0x78,                               // push1
	0x67, 0x32,                         // pTos curVolume
	0x35, 0x02,                         // ldi 02
	0x06,                               // mul
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x08,                               // div
	0x36,                               // push
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x06, 0x00,                   // send 06
	// end of volume code
	0x35, 0x01,                         // ldi 01
	0x65, 0x28,                         // aTop initialized
	SIG_END
};

static const uint16 kq7PatchSubtitleFix2[] = {
	PATCH_ADDTOOFFSET(+5),              // skip to bnt
	0x31, 0x1b,                         // bnt [skip audio volume code]
	PATCH_ADDTOOFFSET(+15),             // right after "aTop curVolume / pushi masterVolume / push1"
	0x7a,                               // push2
	0x06,                               // mul (saves 3 bytes in total)
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x08,                               // div
	0x36,                               // push
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x06, 0x00,                   // send 06
	// end of volume code
	0x35, 118,                          // ldi 118d
	0x65, 0x16,                         // aTop y
	0x78,                               // push1 (saves 1 byte)
	0x69, 0x28,                         // sTop initialized
	PATCH_END
};

// Applies to at least: PC CD 1.51 English, 1.51 German, 2.00 English
// Patched method: Narrator::say (script 64928)
static const uint16 kq7SignatureSubtitleFix3[] = {
	SIG_MAGICDWORD,
	0x63, 0x28,                         // pToa initialized
	0x18,                               // not
	0x31, 0x07,                         // bnt [skip init code]
	0x38, SIG_ADDTOOFFSET(+2),          // pushi init (008Eh for 2.00, 0093h for 1.51)
	0x76,                               // push0
	0x54, SIG_UINT16(0x0004),           // self 04
	// end of init code
	0x8f, 0x00,                         // lsp param[0]
	0x35, 0x01,                         // ldi 01
	0x1e,                               // gt?
	0x31, 0x08,                         // bnt [set acc to 0]
	0x87, 0x02,                         // lap param[2]
	0x31, 0x04,                         // bnt [set acc to 0]
	0x87, 0x02,                         // lap param[2]
	0x33, 0x02,                         // jmp [over set acc to 0 code]
	0x35, 0x00,                         // ldi 00
	0x65, 0x18,                         // aTop caller
	SIG_END
};

static const uint16 kq7PatchSubtitleFix3[] = {
	PATCH_ADDTOOFFSET(+2),              // skip over "pToa initialized code"
	0x2f, 0x0c,                         // bt [skip init code] - saved 1 byte
	0x38,
	PATCH_GETORIGINALUINT16(+6),        // pushi (init)
	0x76,                               // push0
	0x54, PATCH_UINT16(0x0004),           // self 04
	// additionally set background color here (5 bytes)
	0x34, PATCH_UINT16(255),            // pushi 255d
	0x65, 0x2e,                         // aTop back
	// end of init code
	0x8f, 0x00,                         // lsp param[0]
	0x35, 0x01,                         // ldi 01 - this may get optimized to get another byte
	0x1e,                               // gt?
	0x31, 0x04,                         // bnt [set acc to 0]
	0x87, 0x02,                         // lap param[2]
	0x2f, 0x02,                         // bt [over set acc to 0 code]
	PATCH_END
};

// KQ7 has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature
static const uint16 kq7BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class Actor
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x39, 0x0e,                // pushi $e
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0xfdd4),  // pushi 64980
	SIG_END
};

static const uint16 kq7BenchmarkPatch[] = {
	0x34, PATCH_UINT16(10000), // ldi 10000
	0x48,                      // ret
	PATCH_END
};

// When attempting to use an inventory item on an object that does not interact
// with that item, the game temporarily displays an X cursor, but does this by
// spinning for 90000 cycles, which make the duration dependent on CPU speed,
// maxes out the CPU for no reason, and keeps the engine from polling for events
// (which may make the window appear nonresponsive to the OS)
// Applies to at least: KQ7 English 2.00b
static const uint16 kq7PragmaFailSpinSignature[] = {
	0x35, 0x00,               // ldi 0
	0xa5, 0x02,               // sat 2
	SIG_MAGICDWORD,
	0x8d, 0x02,               // lst 2
	0x35, 0x03,               // ldi 3
	0x22,                     // lt?
	SIG_END
};

static const uint16 kq7PragmaFailSpinPatch[] = {
	0x78,                                     // push1
	0x39, 0x12,                               // pushi 18 (~300ms)
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x33, 0x16,                               // jmp to setCursor
	PATCH_END
};

//          script, description,                                      signature                                 patch
static const SciScriptPatcherEntry kq7Signatures[] = {
	{  true,     0, "disable video benchmarking",                  1, kq7BenchmarkSignature,                    kq7BenchmarkPatch },
	{  true,     0, "remove hardcoded spinloop",                   1, kq7PragmaFailSpinSignature,               kq7PragmaFailSpinPatch },
	{  true,    31, "subtitle fix 1/3",                            1, kq7SignatureSubtitleFix1,                 kq7PatchSubtitleFix1 },
	{  true, 64928, "subtitle fix 2/3",                            1, kq7SignatureSubtitleFix2,                 kq7PatchSubtitleFix2 },
	{  true, 64928, "subtitle fix 3/3",                            1, kq7SignatureSubtitleFix3,                 kq7PatchSubtitleFix3 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Script 210 in the German version of Longbow handles the case where Robin
// hands out the scroll to Marion and then types his name using the hand code.
// The German version script contains a typo (probably a copy/paste error),
// and the function that is used to show each letter is called twice. The
// second time that the function is called, the second parameter passed to
// the function is undefined, thus kStrCat() that is called inside the function
// reads a random pointer and crashes. We patch all of the 5 function calls
// (one for each letter typed from "R", "O", "B", "I", "N") so that they are
// the same as the English version.
// Applies to at least: German floppy
// Responsible method: unknown
// Fixes bug: #5264
static const uint16 longbowSignatureShowHandCode[] = {
	0x78,                            // push1
	0x78,                            // push1
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa (letter, that was typed)
	0x36,                            // push
	0x40, SIG_ADDTOOFFSET(+2),       // call
	0x02,                            // perform the call above with 2 parameters
	0x36,                            // push
	0x40, SIG_ADDTOOFFSET(+2),       // call
	SIG_MAGICDWORD,
	0x02,                            // perform the call above with 2 parameters
	0x38, SIG_SELECTOR16(setMotion), // pushi "setMotion" (0x11c in Longbow German)
	0x39, SIG_SELECTOR8(x),          // pushi "x" (0x04 in Longbow German)
	0x51, 0x1e,                      // class MoveTo
	SIG_END
};

static const uint16 longbowPatchShowHandCode[] = {
	0x39, 0x01,                      // pushi 1 (combine the two push1's in one, like in the English version)
	PATCH_ADDTOOFFSET(+3),           // leave the lofsa call untouched
	// The following will remove the duplicate call
	0x32, PATCH_UINT16(0x0002),      // jmp 02 - skip 2 bytes (the remainder of the first call)
	0x48,                            // ret (dummy, should never be reached)
	0x48,                            // ret (dummy, should never be reached)
	PATCH_END
};

// When walking through the forest, arithmetic errors may occur at "random".
// The scripts try to add a value and a pointer to the object "berryBush".
//
// This is caused by a local variable overflow.
//
// The scripts create berry bush objects dynamically. The array storage for
// those bushes may hold a total of 8 bushes. But sometimes 10 bushes
// are created. This overwrites 2 additional locals in script 225 and
// those locals are used normally for value lookups.
//
// Changing the total of bushes could cause all sorts of other issues,
// that's why I rather patched the code, that uses the locals for a lookup.
// Which means it doesn't matter anymore when those locals are overwritten.
//
// Applies to at least: English PC floppy, German PC floppy, English Amiga floppy
// Responsible method: export 2 of script 225
// Fixes bug: #6751
static const uint16 longbowSignatureBerryBushFix[] = {
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x03,                      // ldi 03h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x002d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x04,                      // ldi 04h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0025),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x05,                      // ldi 05h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x001d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x06,                      // ldi 06h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0015),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x18,                      // ldi 18h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x000d),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x19,                      // ldi 19h
	0x1a,                            // eq?
	0x2e, SIG_UINT16(0x0005),        // bt [process code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x1a,                      // ldi 1Ah
	0x1a,                            // eq?
	// jump location for the "bt" instructions
	0x30, SIG_UINT16(0x0011),        // bnt [skip over follow up code, to offset 0c35]
	// 55 bytes until here
	0x85, 00,                        // lat temp[0]
	SIG_MAGICDWORD,
	0x9a, SIG_UINT16(0x0110),        // lsli local[110h] -> 110h points normally to 110h / 2Bh
	// 5 bytes
	0x7a,                            // push2
	SIG_END
};

static const uint16 longbowPatchBerryBushFix[] = {
	PATCH_ADDTOOFFSET(+4),           // keep: lsg global[70h], ldi 03h
	0x22,                            // lt? (global < 03h)
	0x2f, 0x42,                      // bt [skip over all the code directly]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x06,                      // ldi 06h
	0x24,                            // le? (global <= 06h)
	0x2f, 0x0e,                      // bt [to kRandom code]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x18,                      // ldi 18h
	0x22,                            // lt? (global < 18h)
	0x2f, 0x34,                      // bt [skip over all the code directly]
	0x89, 0x70,                      // lsg global[70h]
	0x35, 0x1a,                      // ldi 1Ah
	0x24,                            // le? (global <= 1Ah)
	0x31, 0x2d,                      // bnt [skip over all the code directly]
	// 28 bytes, 27 bytes saved
	// kRandom code
	0x85, 0x00,                      // lat temp[0]
	0x2f, 0x05,                      // bt [skip over case 0]
	// temp[0] == 0
	0x38, SIG_UINT16(0x0110),        // pushi 0110h - that's what's normally at local[110h]
	0x33, 0x18,                      // jmp [kRandom call]
	// check temp[0] further
	0x78,                            // push1
	0x1a,                            // eq?
	0x31, 0x05,                      // bt [skip over case 1]
	// temp[0] == 1
	0x38, SIG_UINT16(0x002b),        // pushi 002Bh - that's what's normally at local[111h]
	0x33, 0x0F,                      // jmp [kRandom call]
	// temp[0] >= 2
	0x8d, 00,                        // lst temp[0]
	0x35, 0x02,                      // ldi 02
	0x04,                            // sub
	0x9a, SIG_UINT16(0x0112),        // lsli local[112h] -> look up value in 2nd table
	                                 // this may not be needed at all and was just added for safety reasons
	// waste 9 spare bytes
	0x35, 0x00,                      // ldi 00
	0x35, 0x00,                      // ldi 00
	0x34, PATCH_UINT16(0x0000),      // ldi 0000
	PATCH_END
};

//          script, description,                                      signature                     patch
static const SciScriptPatcherEntry longbowSignatures[] = {
	{  true,   210, "hand code crash",                             5, longbowSignatureShowHandCode, longbowPatchShowHandCode },
	{  true,   225, "arithmetic berry bush fix",                   1, longbowSignatureBerryBushFix, longbowPatchBerryBushFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 2
// On the plane, Larry is able to wear the parachute. This grants 4 points.
// In early versions of LSL2, it was possible to get "unlimited" points by
//  simply wearing it multiple times.
// They fixed it in later versions by remembering, if the parachute was already
//  used before.
// But instead of adding it properly, it seems they hacked the script / forgot
//  to replace script 0 as well, which holds information about how many global
//  variables are allocated at the start of the game.
// The script tries to read an out-of-bounds global variable, which somewhat
//  "worked" in SSCI, but ScummVM/SCI doesn't allow that.
// That's why those points weren't granted here at all.
// We patch the script to use global 90, which seems to be unused in the whole game.
// Applies to at least: English floppy
// Responsible method: rm63Script::handleEvent
// Fixes bug: #6346
static const uint16 larry2SignatureWearParachutePoints[] = {
	0x35, 0x01,                      // ldi 01
	0xa1, SIG_MAGICDWORD, 0x8e,      // sag 8e
	0x80, SIG_UINT16(0x01e0),        // lag 1e0
	0x18,                            // not
	0x30, SIG_UINT16(0x000f),        // bnt [don't give points]
	0x35, 0x01,                      // ldi 01
	0xa0, 0xe0, 0x01,                // sag 1e0
	SIG_END
};

static const uint16 larry2PatchWearParachutePoints[] = {
	PATCH_ADDTOOFFSET(+4),
	0x80, PATCH_UINT16(0x005a),      // lag 5a (global 90)
	PATCH_ADDTOOFFSET(+6),
	0xa0, PATCH_UINT16(0x005a),      // sag 5a (global 90)
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry larry2Signatures[] = {
	{  true,    63, "plane: no points for wearing plane",          1, larry2SignatureWearParachutePoints, larry2PatchWearParachutePoints },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Leisure Suit Larry 5
// In Miami the player can call the green card telephone number and get
//  green card including limo at the same time in the English 1.000 PC release.
// This results later in a broken game in case the player doesn't read
//  the second telephone number for the actual limousine service, because
//  in that case it's impossible for the player to get back to the airport.
//
// We disable the code, that is responsible to make the limo arrive.
//
// This bug was fixed in the European (dual language) versions of the game.
//
// Applies to at least: English PC floppy (1.000)
// Responsible method: sPhone::changeState(40)
static const uint16 larry5SignatureGreenCardLimoBug[] = {
	0x7a,                               // push2
	SIG_MAGICDWORD,
	0x39, 0x07,                         // pushi 07
	0x39, 0x0c,                         // pushi 0Ch
	0x45, 0x0a, 0x04,                   // call export 10 of script 0
	0x78,                               // push1
	0x39, 0x26,                         // pushi 26h (limo arrived flag)
	0x45, 0x07, 0x02,                   // call export 7 of script 0 (sets flag)
	SIG_END
};

static const uint16 larry5PatchGreenCardLimoBug[] = {
	PATCH_ADDTOOFFSET(+8),
	0x34, PATCH_UINT16(0),              // ldi 0000 (dummy)
	0x34, PATCH_UINT16(0),              // ldi 0000 (dummy)
	PATCH_END
};

// In one of the conversations near the end (to be exact - room 380 and the text
//  about using champagne on Reverse Biaz - only used when you actually did that
//  in the game), the German text is too large, causing the textbox to get too large.
// Because of that the talking head of Patti is drawn over the textbox. A translation oversight.
// Applies to at least: German floppy
// Responsible method: none, position of talker object on screen needs to get modified
static const uint16 larry5SignatureGermanEndingPattiTalker[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x006e),                 // object pattiTalker::x (110)
	SIG_UINT16(0x00b4),                 // object pattiTalker::y (180)
	SIG_ADDTOOFFSET(+469),              // verify that it's really the German version
	0x59, 0x6f, 0x75,                   // (object name) "You"
	0x23, 0x47, 0x44, 0x75,             // "#GDu"
	SIG_END
};

static const uint16 larry5PatchGermanEndingPattiTalker[] = {
	PATCH_UINT16(0x005a),               // change pattiTalker::x to 90
	PATCH_END
};

//          script, description,                                      signature                               patch
static const SciScriptPatcherEntry larry5Signatures[] = {
	{  true,   280, "English-only: fix green card limo bug",       1, larry5SignatureGreenCardLimoBug,        larry5PatchGreenCardLimoBug },
	{  true,   380, "German-only: Enlarge Patti Textbox",          1, larry5SignatureGermanEndingPattiTalker, larry5PatchGermanEndingPattiTalker },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
// Applies to at least: German PC-CD
// Responsible method: unknown
static const uint16 larry6SignatureDeathDialog[] = {
	SIG_MAGICDWORD,
	0x3e, SIG_UINT16(0x0133),        // link 0133 (offset 0x20)
	0x35, 0xff,                      // ldi ff
	0xa3, 0x00,                      // sal 00
	SIG_ADDTOOFFSET(+680),           // [skip 680 bytes]
	0x8f, 0x01,                      // lsp 01 (offset 0x2cf)
	0x7a,                            // push2
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,                            // push
	0x43, 0x7c, 0x0e,                // kMessage[7c] 0e
	SIG_ADDTOOFFSET(+90),            // [skip 90 bytes]
	0x38, SIG_UINT16(0x00d6),        // pushi 00d6 (offset 0x335)
	0x78,                            // push1
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,                            // push
	SIG_ADDTOOFFSET(+76),            // [skip 76 bytes]
	0x38, SIG_UINT16(0x00cd),        // pushi 00cd (offset 0x38b)
	0x39, 0x03,                      // pushi 03
	0x5a, SIG_UINT16(0x0004), SIG_UINT16(0x010e), // lea 0004 010e
	0x36,
	SIG_END
};

static const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,                // link 0200
	PATCH_ADDTOOFFSET(+687),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_ADDTOOFFSET(+98),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_ADDTOOFFSET(+82),
	0x5a, PATCH_UINT16(0x0004), PATCH_UINT16(0x0140), // lea 0004 0140
	PATCH_END
};

//          script, description,                                      signature                   patch
static const SciScriptPatcherEntry larry6Signatures[] = {
	{  true,    82, "death dialog memory corruption",              1, larry6SignatureDeathDialog, larry6PatchDeathDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Leisure Suit Larry 6 Hires

// When entering room 270 (diving board) from room 230, a typo in the game
// script means that `setScale` is called accidentally instead of `setScaler`.
// In SSCI this did not do much because the first argument happened to be
// smaller than the y-position of `ego`, but in ScummVM the first argument is
// larger and so a debug message "y value less than vanishingY" is displayed.
static const uint16 larry6HiresSignatureSetScale[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x14b), // pushi 014b (setScale)
	0x38, SIG_UINT16(0x05),  // pushi 0005
	0x51, 0x2c,              // class 2c (Scaler)
	SIG_END
};

static const uint16 larry6HiresPatchSetScale[] = {
	0x38, SIG_UINT16(0x14f), // pushi 014f (setScaler)
	PATCH_END
};

// The init code that runs when LSL6hires starts up unconditionally resets the
// master music volume to 12 (and the volume dial to 11), but the game should
// always use the volume stored in ScummVM.
// Applies to at least: English CD
static const uint16 larry6HiresSignatureVolumeReset[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x221), // pushi $221 (masterVolume)
	0x78,                    // push1
	0x39, 0x0c,              // push $0c
	0x81, 0x01,              // lag $01
	0x4a, SIG_UINT16(0x06),  // send $6
	0x35, 0x0b,              // ldi $0b
	0xa1, 0xc2,              // sag $c2
	SIG_END
};

static const uint16 larry6HiresPatchVolumeReset[] = {
	0x32, PATCH_UINT16(12),  // jmp 12 [past volume changes]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry larry6HiresSignatures[] = {
	{  true,    71, "disable volume reset on startup",             1, larry6HiresSignatureVolumeReset,  larry6HiresPatchVolumeReset },
	{  true,   270, "fix incorrect setScale call",                 1, larry6HiresSignatureSetScale,     larry6HiresPatchSetScale },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Leisure Suit Larry 7

// ===========================================================================
// In room 540 of Leisure Suit Larry 7, Larry will use 4 items on a so called cheese maker.
//  A short cutscene will then play.
//  During that cutscene on state 6, an animation will get triggered via a special
//  cycler ("End", but from script 64041), that is capable of doing ::cues on specific cels.
//  The code of the state is broken and pushes the object itself as the 2nd cel to cue on.
//  This parameter gets later changed to last cel by CycleCueList::init.
//  Right now, we do not handle comparisons between references to objects and regular values like
//  SSCI, so this will need to get fixed too. But this script bug should also get fixed, because
//  otherwise it works just by accident.
//
// Applies to at least: English PC-CD, German PC-CD
// Responsible method: soMakeCheese::changeState(6) in script 540
static const uint16 larry7SignatureMakeCheese[] = {
	0x38, SIG_UINT16(4),             // pushi 04
	0x51, 0xc4,                      // class End
	0x36,                            // push
	SIG_MAGICDWORD,
	0x7c,                            // pushSelf
	0x39, 0x04,                      // pushi 04
	0x7c,                            // pushSelf
	SIG_END
};

static const uint16 larry7PatchMakeCheese[] = {
	0x39, 0x04,                      // pushi 04 - save 1 byte
	0x51, 0xc4,                      // class End
	0x36,
	0x7c,                            // pushSelf
	0x39, 0x04,                      // pushi 04
	0x39, 0x10,                      // pushi 10h (last cel of view 54007, loop 0)
	PATCH_END
};

// ===========================================================================
// During the same cheese maker cutscene as mentioned before, there is also
//  a little priority issue, which also happens in the original interpreter.
//  While Larry is pouring liquid into the cheese maker, he appears shortly right
//  in front of the guillotine instead of behind it.
//  This is caused by soMakeCheese::changeState(2) setting priority of ego to 500.
//  It is needed to change priority a bit, otherwise Larry would also appear behind the cheese
//  maker and that wouldn't make sense, but the cheese maker has a priority of only 373.
//
// This of course also happens, when using the original interpreter.
//
// We change this to set priority to 374, which works fine.
//
// Applies to at least: English PC-CD, German PC-CD
// Responsible method: soMakeCheese::changeState(2) in script 540
static const uint16 larry7SignatureMakeCheesePriority[] = {
	0x38, SIG_SELECTOR16(setPri),    // pushi (setPri)
	SIG_MAGICDWORD,
	0x78,                            // push1
	0x38, SIG_UINT16(500),           // pushi 1F4h (500d)
	SIG_END
};

static const uint16 larry7PatchMakeCheesePriority[] = {
	PATCH_ADDTOOFFSET(+4),
	0x38, PATCH_UINT16(374),         // pushi 176h (374d)
	PATCH_END
};

//          script, description,                                signature                           patch
static const SciScriptPatcherEntry larry7Signatures[] = {
	{  true,   540, "fix make cheese cutscene (cycler)",     1, larry7SignatureMakeCheese,          larry7PatchMakeCheese },
	{  true,   540, "fix make cheese cutscene (priority)",   1, larry7SignatureMakeCheesePriority,  larry7PatchMakeCheesePriority },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Laura Bow 1 - Colonel's Bequest
//
// This is basically just a broken easter egg in Colonel's Bequest.
// A plane can show up in room 4, but that only happens really rarely.
// Anyway the Sierra developer seems to have just entered the wrong loop,
// which is why the statue view is used instead (loop 0).
// We fix it to use the correct loop.
//
// This is only broken in the PC version. It was fixed for Amiga + Atari ST.
//
// Credits to OmerMor, for finding it.

// Applies to at least: English PC Floppy
// Responsible method: room4::init
static const uint16 laurabow1SignatureEasterEggViewFix[] = {
	0x78,                               // push1
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(setLoop),      // pushi "setLoop"
	0x78,                               // push1
	0x39, 0x03,                         // pushi 3 (loop 3, view only has 3 loops)
	SIG_END
};

static const uint16 laurabow1PatchEasterEggViewFix[] = {
	PATCH_ADDTOOFFSET(+7),
	0x02,                            // change loop to 2
	PATCH_END
};

// When oiling the armor or opening the visor of the armor, the scripts
//  first check if Laura/ego is near the armor and if she is not, they will move her
//  to the armor. After that further code is executed.
//
// The current location is checked by a ego::inRect() call.
//
// The given rect for the inRect call inside openVisor::changeState was made larger for Atari ST/Amiga versions.
//  We change the PC version to use the same rect.
//
// Additionally the coordinate, that Laura is moved to, is 152, 107 and may not be reachable depending on where
//  Laura/ego was, when "use oil on helmet of armor" / "open visor of armor" got entered.
//  Bad coordinates are for example 82, 110, which then cause collisions and effectively an endless loop.
//  Game will effectively "freeze" and the user is only able to restore a previous game.
//  This also happened, when using the original interpreter.
//  We change the destination coordinate to 152, 110, which seems to be reachable all the time.
//
// The following patch fixes the rect for the PC version of the game.
//
// Applies to at least: English PC Floppy
// Responsible method: openVisor::changeState (script 37)
// Fixes bug: #7119
static const uint16 laurabow1SignatureArmorOpenVisorFix[] = {
	0x39, 0x04,                         // pushi 04
	SIG_MAGICDWORD,
	0x39, 0x6a,                         // pushi 6a (106d)
	0x38, SIG_UINT16(0x96),             // pushi 0096 (150d)
	0x39, 0x6c,                         // pushi 6c (108d)
	0x38, SIG_UINT16(0x98),             // pushi 0098 (152d)
	SIG_END
};

static const uint16 laurabow1PatchArmorOpenVisorFix[] = {
	PATCH_ADDTOOFFSET(+2),
	0x39, 0x68,                         // pushi 68 (104d)   (-2)
	0x38, SIG_UINT16(0x94),             // pushi 0094 (148d) (-2)
	0x39, 0x6f,                         // pushi 6f (111d)   (+3)
	0x38, SIG_UINT16(0x9a),             // pushi 009a (154d) (+2)
	PATCH_END
};

// This here fixes the destination coordinate (exact details are above).
//
// Applies to at least: English PC Floppy, English Atari ST Floppy, English Amiga Floppy
// Responsible method: openVisor::changeState, oiling::changeState (script 37)
// Fixes bug: #7119
static const uint16 laurabow1SignatureArmorMoveToFix[] = {
	SIG_MAGICDWORD,
	0x36,                               // push
	0x39, 0x6b,                         // pushi 6B (107d)
	0x38, SIG_UINT16(0x0098),           // pushi 98 (152d)
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0]
	SIG_END
};

static const uint16 laurabow1PatchArmorMoveToFix[] = {
	PATCH_ADDTOOFFSET(+1),
	0x39, 0x6e,                         // pushi 6E (110d) - adjust x, so that no collision can occur anymore
	PATCH_END
};

// In some cases like for example when the player oils the arm of the armor, command input stays
// disabled, even when the player exits fast enough, so that Laura doesn't die.
//
// This is caused by the scripts only enabling control (directional movement), but do not enable command input as well.
//
// This bug also happens, when using the original interpreter.
// And it was fixed for the Atari ST + Amiga versions of the game.
//
// Applies to at least: English PC Floppy
// Responsible method: 2nd subroutine in script 37, called by oiling::changeState(7)
// Fixes bug: #7154
static const uint16 laurabow1SignatureArmorOilingArmFix[] = {
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x72, SIG_UINT16(0x1a5c),           // lofsa "Can" - offsets are not skipped to make sure only the PC version gets patched
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x19a1),           // lofsa "Visor"
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x194a),           // lofsa "note"
	0x4a, 0x04,                         // send 04
	0x38, SIG_UINT16(0x0089),           // pushi 89h
	0x76,                               // push0
	0x72, SIG_UINT16(0x18f3),           // lofsa "valve"
	0x4a, 0x04,                         // send 04
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x02,                         // ldi 02
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x0014),           // bnt [to ret]
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x05,                         // ldi 05
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x000c),           // bnt [to ret]
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x06,                         // ldi 06
	0x1c,                               // ne?
	0x30, SIG_UINT16(0x0004),           // bnt [to ret]
	// followed by code to call script 0 export to re-enable controls and call setMotion
	SIG_END
};

static const uint16 laurabow1PatchArmorOilingArmFix[] = {
	PATCH_ADDTOOFFSET(+3),              // skip over pushi 89h
	0x3c,                               // dup
	0x3c,                               // dup
	0x3c,                               // dup
	// saves a total of 6 bytes
	0x76,                               // push0
	0x72, SIG_UINT16(0x1a59),           // lofsa "Can"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x19a1),           // lofsa "Visor"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x194d),           // lofsa "note"
	0x4a, 0x04,                         // send 04
	0x76,                               // push0
	0x72, SIG_UINT16(0x18f9),           // lofsa "valve" 18f3
	0x4a, 0x04,                         // send 04
	// new code to enable input as well, needs 9 spare bytes
	0x38, SIG_UINT16(0x00e2),           // canInput
	0x78,                               // push1
	0x78,                               // push1
	0x51, 0x2b,                         // class User
	0x4a, 0x06,                         // send 06 -> call User::canInput(1)
	// original code, but changed a bit to save some more bytes
	0x8b, 0x34,                         // lsl local[34h]
	0x35, 0x02,                         // ldi 02
	0x04,                               // sub
	0x31, 0x12,                         // bnt [to ret]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x04,                               // sub
	0x31, 0x0c,                         // bnt [to ret]
	0x78,                               // push1
	0x1a,                               // eq?
	0x2f, 0x08,                         // bt [to ret]
	// saves 7 bytes, we only need 3, so waste 4 bytes
	0x35, 0x00,                         // ldi 0
	0x35, 0x00,                         // ldi 0
	PATCH_END
};

// When you tell Lilly about Gertie in room 35, Lilly will then walk to the left and off the screen.
// In case Laura (ego) is in the way, the whole game will basically block and you won't be able
// to do anything except saving + restoring the game.
//
// If this happened already, the player can enter
// "send Lillian ignoreActors 1" inside the debugger to fix this situation.
//
// This issue is very difficult to solve, because Lilly also walks diagonally after walking to the left right
// under the kitchen table. This means that even if we added a few more rectangle checks, there could still be
// spots, where the game would block.
//
// Also the mover "PathOut" is used for Lillian instead of the regular "MoveTo", which would avoid other
// actors by itself.
//
// So instead we set Lilly to ignore other actors during that cutscene, which is the least invasive solution.
//
// Applies to at least: English PC Floppy, English Amiga Floppy, English Atari ST Floppy
// Responsible method: goSee::changeState(1) in script 236
// Fixes bug: (happened during GOG Let's Play)
static const uint16 laurabow1SignatureTellLillyAboutGerieBlockingFix1[] = {
	0x7a,                               // puah2
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00c1),           // pushi 00C1h
	0x38, SIG_UINT16(0x008f),           // pushi 008Fh
	0x38, SIG_SELECTOR16(ignoreActors), // pushi (ignoreActors)
	0x78,                               // push1
	0x76,                               // push0
	SIG_END
};

static const uint16 laurabow1PatchTellLillyAboutGertieBlockingFix1[] = {
	PATCH_ADDTOOFFSET(+11),             // skip over until push0
	0x78,                               // push1 (change push0 to push1)
	PATCH_END
};

// a second patch to call Lillian::ignoreActors(1) on goSee::changeState(9) in script 236
static const uint16 laurabow1SignatureTellLillyAboutGerieBlockingFix2[] = {
	0x3c,                               // dup
	0x35, 0x09,                         // ldi 09
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x003f),           // bnt [ret]
	0x39, SIG_ADDTOOFFSET(+1),          // pushi (view)
	0x78,                               // push1
	0x38, SIG_UINT16(0x0203),           // pushi 203h (515d)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi (posn)
	0x7a,                               // push2
	0x38, SIG_UINT16(0x00c9),           // pushi C9h (201d)
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x0084),           // pushi 84h (132d)
	0x72, SIG_ADDTOOFFSET(+2),          // lofsa Lillian (different offsets for different platforms)
	0x4a, 0x0e,                         // send 0Eh
	SIG_END
};

static const uint16 laurabow1PatchTellLillyAboutGertieBlockingFix2[] = {
	0x38, PATCH_SELECTOR16(ignoreActors), // pushi (ignoreActors)
	0x78,                                 // push1
	0x76,                                 // push0
	0x33, 0x00,                           // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET(+19),               // skip over until send
	0x4a, 0x14,                           // send 14h
	PATCH_END
};

//          script, description,                                      signature                           patch
static const SciScriptPatcherEntry laurabow1Signatures[] = {
	{  true,     4, "easter egg view fix",                      1, laurabow1SignatureEasterEggViewFix,                laurabow1PatchEasterEggViewFix },
	{  true,    37, "armor open visor fix",                     1, laurabow1SignatureArmorOpenVisorFix,               laurabow1PatchArmorOpenVisorFix },
	{  true,    37, "armor move to fix",                        2, laurabow1SignatureArmorMoveToFix,                  laurabow1PatchArmorMoveToFix },
	{  true,    37, "allowing input, after oiling arm",         1, laurabow1SignatureArmorOilingArmFix,               laurabow1PatchArmorOilingArmFix },
	{  true,   236, "tell Lilly about Gertie blocking fix 1/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix1, laurabow1PatchTellLillyAboutGertieBlockingFix1 },
	{  true,   236, "tell Lilly about Gertie blocking fix 2/2", 1, laurabow1SignatureTellLillyAboutGerieBlockingFix2, laurabow1PatchTellLillyAboutGertieBlockingFix2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Laura Bow 2
//
// Moving away the painting in the room with the hidden safe is problematic
//  for the CD version of the game. safePic::doVerb gets triggered by the mouse-click.
// This method sets local 0 as signal, which is only meant to get handled, when
//  the player clicks again to move the painting back. This signal is processed by
//  the room doit-script.
// That doit-script checks safePic::cel to be not equal 0 and would then skip over
//  the "close painting" trigger code. On very fast computers this script may
//  get called too early (which is the case when running under ScummVM and when
//  running the game using Sierra SCI in DOS-Box with cycles 15000) and thinks
//  that it's supposed to move the painting back. Which then results in the painting
//  getting moved to its original position immediately (which means it won't be possible
//  to access the safe behind it).
//
// We patch the script, so that we check for cel to be not equal 4 (the final cel) and
//  we also reset the safePic-signal immediately as well.
//
// In the floppy version Laura's coordinates are checked directly in rm560::doit
//  and as soon as she moves, the painting will automatically move to its original position.
//  This is not the case for the CD version of the game. The painting will only "move" back,
//  when the player actually exits the room and re-enters.
//
// Applies to at least: English PC-CD
// Responsible method: rm560::doit
// Fixes bug: #6460
static const uint16 laurabow2CDSignaturePaintingClosing[] = {
	0x39, 0x04,                         // pushi 04 (cel)
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x7a,                               // push2
	0x38, SIG_UINT16(0x0231),           // pushi 0231h (561)
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x18,                               // not
	0x31, 0x21,                         // bnt [exit]
	0x38, SIG_UINT16(0x0283),           // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 32)
	0x4a, 0x04,                         // send 04 (get sHeimlich::room)
	0x36,                               // push
	0x81, 0x0b,                         // lag global[b] (current room)
	0x1c,                               // ne?
	0x31, 0x0e,                         // bnt [exit]
	0x35, 0x00,                         // ldi 00
	0xa3, 0x00,                         // sal local[0] -> reset safePic signal
	SIG_END
};

static const uint16 laurabow2CDPatchPaintingClosing[] = {
	PATCH_ADDTOOFFSET(+2),
	0x3c,                               // dup (1 additional byte)
	0x76,                               // push0
	0x3c,                               // dup (1 additional byte)
	0xab, 0x00,                         // ssl local[0] -> reset safePic signal
	0x7a,                               // push2
	0x38, PATCH_UINT16(0x0231),         // pushi 0231h (561)
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 561)
	0x4a, 0x04,                         // send 04 (gets safePicture::cel)
	0x1a,                               // eq?
	0x31, 0x1d,                         // bnt [exit]
	0x38, PATCH_UINT16(0x0283),         // pushi 0283h
	0x76,                               // push0
	0x7a,                               // push2
	0x39, 0x20,                         // pushi 20
	0x76,                               // push0
	0x43, 0x02, 0x04,                   // kScriptID (get export 0 of script 32)
	0x4a, 0x04,                         // send 04 (get sHeimlich::room)
	0x36,                               // push
	0x81, 0x0b,                         // lag global[b] (current room)
	0x1a,                               // eq? (2 opcodes changed, to save 2 bytes)
	0x2f, 0x0a,                         // bt [exit]
	PATCH_END
};

// In the CD version the system menu is disabled for certain rooms. LB2::handsOff is called,
//  when leaving the room (and in other cases as well). This method remembers the disabled
//  icons of the icon bar. In the new room LB2::handsOn will get called, which then enables
//  all icons, but also disabled the ones, that were disabled before.
//
// Because of this behaviour certain rooms, that should have the system menu enabled, have
//  it disabled, when entering those rooms from rooms, where the menu is supposed to be
//  disabled.
//
// We patch this by injecting code into LB2::newRoom (which is called right after a room change)
//  and reset the global variable there, that normally holds the disabled buttons.
//
// This patch may cause side-effects and it's difficult to test, because it affects every room
//  in the game. At least for the intro, the speakeasy and plenty of rooms in the beginning it
//  seems to work correctly.
//
// Applies to at least: English PC-CD
// Responsible method: LB2::newRoom, LB2::handsOff, LB2::handsOn
// Fixes bug: #6440
static const uint16 laurabow2CDSignatureFixProblematicIconBar[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00f1),           // pushi 00f1 (disable) - hardcoded, we only want to patch the CD version
	0x76,                               // push0
	0x81, 0x45,                         // lag global[45]
	0x4a, 0x04,                         // send 04
	SIG_END
};

static const uint16 laurabow2CDPatchFixProblematicIconBar[] = {
	0x35, 0x00,                      // ldi 00
	0xa1, 0x74,                      // sag 74h
	0x35, 0x00,                      // ldi 00 (waste bytes)
	0x35, 0x00,                      // ldi 00
	PATCH_END
};

// When entering the main musem party room (w/ the golden Egyptian head),
// Laura is waslking a bit into the room automatically.
// In case you press a mouse button while this is happening, you will get
// stuck inside that room and won't be able to exit it anymore.
//
// Users, who played the game w/ a previous version of ScummVM can simply
// enter the debugger and then enter "send rm350 script 0:0", which will
// fix the script state.
//
// This is caused by the user controls not being locked at that point.
// Pressing a button will cause the cue from the PolyPath walker to never
// happen, which then causes sEnterSouth to never dispose itself.
//
// User controls are locked in the previous room 335, but controls
// are unlocked by frontDoor::cue.
// We do not want to change this, because it could have side-effects.
// We instead add another LB2::handsOff call inside the script responsible
// for making Laura walk into the room (sEnterSouth::changeState(0).
//
// Applies to at least: English PC-CD, English PC-Floppy, German PC-Floppy
// Responsible method: sEnterSouth::changeState
// Fixes bug: (no bug report, from GOG forum post)
static const uint16 laurabow2SignatureMuseumPartyFixEnteringSouth1[] = {
	0x3c,                              // dup
	0x35, 0x00,                        // ldi 00
	0x1a,                              // eq?
	0x30, SIG_UINT16(0x0097),          // bnt [state 1 code]
	SIG_ADDTOOFFSET(+141),             // skip to end of follow-up code
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x008d for CD, 0x007d for floppy)
	0x35, 0x01,                        // ldi 01
	0x65, 0x1a,                        // aTop cycles
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x0086 for CD, 0x0076 for floppy)
	// state 1 code
	0x3c,                              // dup
	0x35, 0x01,                        // ldi 01
	0x1a,                              // eq?
	SIG_MAGICDWORD,
	0x31, 0x05,                        // bnt [state 2 code]
	0x35, 0x00,                        // ldi 00
	0x32, SIG_ADDTOOFFSET(+2),         // jmp [ret] (0x007b for CD, 0x006b for floppy)
	// state 2 code
	0x3c,                              // dup
	SIG_END
};

static const uint16 laurabow2PatchMuseumPartyFixEnteringSouth1[] = {
	0x2e, PATCH_UINT16(0x00a6),        // bt [state 2 code] (we skip state 1, because it's a NOP anyways)
	// state 0 processing
	0x32, PATCH_UINT16(+151),
	SIG_ADDTOOFFSET(+149),             // skip to end of follow-up code
	// save 1 byte by replacing jump to [ret] into straight toss/ret
	0x3a,                              // toss
	0x48,                              // ret

	// additional code, that gets called right at the start of step 0 processing
	0x18,                              // not -- this here is where pushi handsOff will be inserted by the second patch
	0x18,                              // not    offset and handsOff is different for floppy + CD, that's why we do this
	0x18,                              // not    floppy also does not have a selector table, so we can't go by "handsOff" name
	0x18,                              // not
	0x76,                              // push0
	0x81, 0x01,                        // lag global[1]
	0x4a, 0x04,                        // send 04
	0x32, PATCH_UINT16(0xFF5e),        // jmp [back to start of step 0 processing]
	PATCH_END
};

// second patch, which only inserts pushi handsOff inside our new code
// There is no other way to do this except making 2 full patches for floppy + CD, because handsOff/handsOn
// is not the same value between floppy + CD *and* floppy doesn't even have a vocab, so we can't figure out the id
// by ourselves.
static const uint16 laurabow2SignatureMuseumPartyFixEnteringSouth2[] = {
	0x18,                              // our injected code
	0x18,
	0x18,
	SIG_ADDTOOFFSET(+92),              // skip to the handsOn code, that we are interested in
	0x38, SIG_ADDTOOFFSET(+2),         // pushi handsOn (0x0189 for CD, 0x024b for floppy)
	0x76,                              // push0
	0x81, 0x01,                        // lag global[1]
	0x4a, 0x04,                        // send 04
	0x38, SIG_ADDTOOFFSET(+2),         // pushi 0274h
	SIG_MAGICDWORD,
	0x78,                              // push1
	0x38, SIG_UINT16(0x033f),          // pushi 033f
	SIG_END
};

static const uint16 laurabow2PatchMuseumPartyFixEnteringSouth2[] = {
	0x38,                              // pushi
	PATCH_GETORIGINALUINT16ADJUST(+96, -1), // get handsOff code and ubstract 1 from it to get handsOn
	PATCH_END
};

// Opening/Closing the east door in the pterodactyl room doesn't
//  check, if it's locked and will open/close the door internally
//  even when it is.
//
// It will get wired shut later in the game by Laura Bow and will be
//  "locked" because of this. We patch in a check for the locked
//  state. We also add code, that will set the "locked" state
//  in case our eastDoor-wired-global is set. This makes the locked
//  state effectively persistent.
//
// Applies to at least: English PC-CD, English PC-Floppy
// Responsible method (CD): eastDoor::doVerb
// Responsible method (Floppy): eastDoor::<noname300>
// Fixes bug: #6458 (partly, see additional patch below)
static const uint16 laurabow2SignatureFixWiredEastDoor[] = {
	0x30, SIG_UINT16(0x0022),           // bnt [skip hand action]
	0x67, SIG_ADDTOOFFSET(+1),          // pTos CD: doorState, Floppy: state
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x08,                         // bnt [close door code]
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39, 0x63,                         // pushi 63h
	0x45, 0x04, 0x02,                   // callb export000_4, 02 (sets door-bitflag)
	0x33, 0x06,                         // jmp [super-code]
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x45, 0x03, 0x02,                   // callb export000_3, 02 (resets door-bitflag)
	0x38, SIG_ADDTOOFFSET(+2),          // pushi CD: 011dh, Floppy: 012ch
	0x78,                               // push1
	0x8f, 0x01,                         // lsp param[01]
	0x59, 0x02,                         // rest 02
	0x57, SIG_ADDTOOFFSET(+1), 0x06,    // super CD: LbDoor, Floppy: Door, 06
	0x33, 0x0b,                         // jmp [ret]
	SIG_END
};

static const uint16 laurabow2PatchFixWiredEastDoor[] = {
	0x31, 0x23,                         // bnt [skip hand action] (saves 1 byte)
	0x81,   97,                         // lag 97d (get our eastDoor-wired-global)
	0x31, 0x04,                         // bnt [skip setting locked property]
	0x35, 0x01,                         // ldi 01
	0x65, 0x6a,                         // aTop locked (set eastDoor::locked to 1)
	0x63, 0x6a,                         // pToa locked (get eastDoor::locked)
	0x2f, 0x17,                         // bt [skip hand action]
	0x63, PATCH_GETORIGINALBYTE(+4),    // pToa CD: doorState, Floppy: state
	0x78,                               // push1
	0x39, 0x63,                         // pushi 63h
	0x2f, 0x05,                         // bt [close door code]
	0x45, 0x04, 0x02,                   // callb export000_4, 02 (sets door-bitflag)
	0x33, 0x0b,                         // jmp [super-code]
	0x45, 0x03, 0x02,                   // callb export000_3, 02 (resets door-bitflag)
	0x33, 0x06,                         // jmp [super-code]
	PATCH_END
};

// We patch in code, so that our eastDoor-wired-global will get set to 1.
//  This way the wired-state won't get lost when exiting room 430.
//
// Applies to at least: English PC-CD, English PC-Floppy
// Responsible method (CD): sWireItShut::changeState
// Responsible method (Floppy): sWireItShut::<noname144>
// Fixes bug: #6458 (partly, see additional patch above)
static const uint16 laurabow2SignatureRememberWiredEastDoor[] = {
	SIG_MAGICDWORD,
	0x33, 0x27,                         // jmp [ret]
	0x3c,                               // dup
	0x35, 0x06,                         // ldi 06
	0x1a,                               // eq?
	0x31, 0x21,                         // bnt [skip step]
	SIG_END
};

static const uint16 laurabow2PatchRememberWiredEastDoor[] = {
	PATCH_ADDTOOFFSET(+2),              // skip jmp [ret]
	0x34, PATCH_UINT16(0x0001),         // ldi 0001
	0xa1, PATCH_UINT16(97),             // sag 97d (set our eastDoor-wired-global)
	PATCH_END
};

// Laura Bow 2 CD resets the audio mode to speech on init/restart
//  We already sync the settings from ScummVM (see SciEngine::syncIngameAudioOptions())
//  and this script code would make it impossible to see the intro using "dual" mode w/o using debugger command
//  That's why we remove the corresponding code
// Patched method: LB2::init, rm100::init
static const uint16 laurabow2CDSignatureAudioTextSupportModeReset[] = {
	SIG_MAGICDWORD,
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextSupportModeReset[] = {
	0x34, PATCH_UINT16(0x0001),         // ldi 0001 (waste bytes)
	0x18,                               // not (waste bytes)
	PATCH_END
};

// Directly use global 5a for view-cel id
//  That way it's possible to use a new "dual" mode view in the game menu
// View 995, loop 13, cel 0 -> "text"
// View 995, loop 13, cel 1 -> "speech"
// View 995, loop 13, cel 2 -> "dual"  (this view is injected by us into the game)
// Patched method: gcWin::open
static const uint16 laurabow2CDSignatureAudioTextMenuSupport1[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x36,                               // push
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextMenuSupport1[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x01,                         // ldi 01
	0x04,                               // sub
	PATCH_END
};

//  Adds another button state for the text/audio button. We currently use the "speech" view for "dual" mode.
// Patched method: iconMode::doit
static const uint16 laurabow2CDSignatureAudioTextMenuSupport2[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg global[5a]
	0x3c,                               // dup
	0x1a,                               // eq?
	0x31, 0x0a,                         // bnt [set text mode]
	0x35, 0x02,                         // ldi 02
	0xa1, 0x5a,                         // sag global[5a]
	0x35, 0x01,                         // ldi 01
	0xa5, 0x00,                         // sat temp[0]
	0x33, 0x0e,                         // jmp [draw cel code]
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x08,                         // bnt [draw cel code]
	0x35, 0x01,                         // ldi 01
	0xa1, 0x5a,                         // sag global[5a]
	0x35, 0x00,                         // ldi 00
	0xa5, 0x00,                         // sat temp[0]
	0x3a,                               // toss
	SIG_END
};

static const uint16 laurabow2CDPatchAudioTextMenuSupport2[] = {
	0x81, 0x5a,                         // lag global[5a]
	0x78,                               // push1
	0x02,                               // add
	0xa1, 0x5a,                         // sag global[5a]
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x1e,                               // gt?
	0x31, 0x03,                         // bnt [skip over]
	0x78,                               // push1
	0xa9, 0x5a,                         // ssg global[5a]
	0x89, 0x5a,                         // lsg global[5a]
	0x35, 0x01,                         // ldi 01
	0x04,                               // sub
	0xa5, 0x00,                         // sat temp[0] - calculate global[5a] - 1 to use as view cel id
	0x33, 0x07,                         // jmp [draw cel code, don't do toss]
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry laurabow2Signatures[] = {
	{  true,   560, "CD: painting closing immediately",               1, laurabow2CDSignaturePaintingClosing,            laurabow2CDPatchPaintingClosing },
	{  true,     0, "CD: fix problematic icon bar",                   1, laurabow2CDSignatureFixProblematicIconBar,      laurabow2CDPatchFixProblematicIconBar },
	{  true,   350, "CD/Floppy: museum party fix entering south 1/2", 1, laurabow2SignatureMuseumPartyFixEnteringSouth1, laurabow2PatchMuseumPartyFixEnteringSouth1 },
	{  true,   350, "CD/Floppy: museum party fix entering south 2/2", 1, laurabow2SignatureMuseumPartyFixEnteringSouth2, laurabow2PatchMuseumPartyFixEnteringSouth2 },
	{  true,   430, "CD/Floppy: make wired east door persistent",     1, laurabow2SignatureRememberWiredEastDoor,        laurabow2PatchRememberWiredEastDoor },
	{  true,   430, "CD/Floppy: fix wired east door",                 1, laurabow2SignatureFixWiredEastDoor,             laurabow2PatchFixWiredEastDoor },
	// King's Quest 6 and Laura Bow 2 share basic patches for audio + text support
	{ false,   924, "CD: audio + text support 1",                     1, kq6laurabow2CDSignatureAudioTextSupport1,       kq6laurabow2CDPatchAudioTextSupport1 },
	{ false,   924, "CD: audio + text support 2",                     1, kq6laurabow2CDSignatureAudioTextSupport2,       kq6laurabow2CDPatchAudioTextSupport2 },
	{ false,   924, "CD: audio + text support 3",                     1, kq6laurabow2CDSignatureAudioTextSupport3,       kq6laurabow2CDPatchAudioTextSupport3 },
	{ false,   928, "CD: audio + text support 4",                     1, kq6laurabow2CDSignatureAudioTextSupport4,       kq6laurabow2CDPatchAudioTextSupport4 },
	{ false,   928, "CD: audio + text support 5",                     2, kq6laurabow2CDSignatureAudioTextSupport5,       kq6laurabow2CDPatchAudioTextSupport5 },
	{ false,     0, "CD: audio + text support disable mode reset",    1, laurabow2CDSignatureAudioTextSupportModeReset,  laurabow2CDPatchAudioTextSupportModeReset },
	{ false,   100, "CD: audio + text support disable mode reset",    1, laurabow2CDSignatureAudioTextSupportModeReset,  laurabow2CDPatchAudioTextSupportModeReset },
	{ false,    24, "CD: audio + text support LB2 menu 1",            1, laurabow2CDSignatureAudioTextMenuSupport1,      laurabow2CDPatchAudioTextMenuSupport1 },
	{ false,    24, "CD: audio + text support LB2 menu 2",            1, laurabow2CDSignatureAudioTextMenuSupport2,      laurabow2CDPatchAudioTextMenuSupport2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Mother Goose SCI1/SCI1.1
// MG::replay somewhat calculates the savedgame-id used when saving again
//  this doesn't work right and we remove the code completely.
//  We set the savedgame-id directly right after restoring in kRestoreGame.
//  We also draw the background picture in here instead.
//  This Mixed Up Mother Goose draws the background picture before restoring,
//  instead of doing it properly in MG::replay. This fixes graphic issues,
//  when restoring from GMM.
//
// Applies to at least: English SCI1 CD, English SCI1.1 floppy, Japanese FM-Towns
// Responsible method: MG::replay (script 0)
static const uint16 mothergoose256SignatureReplay[] = {
	0x7a,                            // push2
	0x78,                            // push1
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x43, 0x70, 0x04,                // callk MemorySegment
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x76,                            // push0
	0x43, 0x62, 0x04,                // callk StrAt
	0xa1, 0xaa,                      // sag global[AAh]
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x78,                            // push1
	0x43, 0x62, 0x04,                // callk StrAt
	0x36,                            // push
	0x35, 0x20,                      // ldi 20
	0x04,                            // sub
	0xa1, SIG_ADDTOOFFSET(+1),       // sag global[57h] -> FM-Towns [9Dh]
	// 35 bytes
	0x39, 0x03,                      // pushi 03
	0x89, SIG_ADDTOOFFSET(+1),       // lsg global[1Dh] -> FM-Towns [1Eh]
	0x76,                            // push0
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BEh]
	0x36,                            // push
	0x7a,                            // push2
	0x43, 0x62, 0x04,                // callk StrAt
	0x36,                            // push
	0x35, 0x01,                      // ldi 01
	0x04,                            // sub
	0x36,                            // push
	0x43, 0x62, 0x06,                // callk StrAt
	// 22 bytes
	0x7a,                            // push2
	0x5b, 0x00, 0xbe,                // lea global[BE]
	0x36,                            // push
	0x39, 0x03,                      // pushi 03
	0x43, 0x62, 0x04,                // callk StrAt
	// 10 bytes
	0x36,                            // push
	0x35, SIG_MAGICDWORD, 0x20,      // ldi 20
	0x04,                            // sub
	0xa1, 0xb3,                      // sag global[b3]
	// 6 bytes
	SIG_END
};

static const uint16 mothergoose256PatchReplay[] = {
	0x39, 0x06,                      // pushi 06
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(200),         // pushi 200d
	0x38, PATCH_UINT16(320),         // pushi 320d
	0x76,                            // push0
	0x76,                            // push0
	0x43, 0x15, 0x0c,                // callk SetPort -> set picture port to full screen
	// 15 bytes
	0x39, 0x04,                      // pushi 04
	0x3c,                            // dup
	0x76,                            // push0
	0x38, PATCH_UINT16(255),         // pushi 255d
	0x76,                            // push0
	0x43, 0x6f, 0x08,                // callk Palette -> set intensity to 0 for all colors
	// 11 bytes
	0x7a,                            // push2
	0x38, PATCH_UINT16(800),         // pushi 800
	0x76,                            // push0
	0x43, 0x08, 0x04,                // callk DrawPic -> draw picture 800
	// 8 bytes
	0x39, 0x06,                      // pushi 06
	0x39, 0x0c,                      // pushi 0Ch
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(200),         // push 200
	0x38, PATCH_UINT16(320),         // push 320
	0x78,                            // push1
	0x43, 0x6c, 0x0c,                // callk Graph -> send everything to screen
	// 16 bytes
	0x39, 0x06,                      // pushi 06
	0x76,                            // push0
	0x76,                            // push0
	0x38, PATCH_UINT16(156),         // pushi 156d
	0x38, PATCH_UINT16(258),         // pushi 258d
	0x39, 0x03,                      // pushi 03
	0x39, 0x04,                      // pushi 04
	0x43, 0x15, 0x0c,                // callk SetPort -> set picture port back
	// 17 bytes
	0x34, PATCH_UINT16(0x0000),      // ldi 0000 (dummy)
	0x34, PATCH_UINT16(0x0000),      // ldi 0000 (dummy)
	PATCH_END
};

// when saving, it also checks if the savegame ID is below 13.
//  we change this to check if below 113 instead
//
// Applies to at least: English SCI1 CD, English SCI1.1 floppy, Japanese FM-Towns
// Responsible method: Game::save (script 994 for SCI1), MG::save (script 0 for SCI1.1)
static const uint16 mothergoose256SignatureSaveLimit[] = {
	0x89, SIG_MAGICDWORD, 0xb3,      // lsg global[b3]
	0x35, 0x0d,                      // ldi 0d
	0x20,                            // ge?
	SIG_END
};

static const uint16 mothergoose256PatchSaveLimit[] = {
	PATCH_ADDTOOFFSET(+2),
	0x35, 0x0d + SAVEGAMEID_OFFICIALRANGE_START, // ldi 113d
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry mothergoose256Signatures[] = {
	{  true,     0, "replay save issue",                           1, mothergoose256SignatureReplay,    mothergoose256PatchReplay },
	{  true,     0, "save limit dialog (SCI1.1)",                  1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	{  true,   994, "save limit dialog (SCI1)",                    1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Mixed-up Mother Goose Deluxe

// The game uses pic 10005 to render the Sierra logo, but then it also
// initialises a logo object with view 502 on the same priority as the pic. In
// the original interpreter, it is dumb luck which is drawn first (based on the
// order of the memory IDs), though usually the pic is drawn first because not
// many objects have been created at the start of the game. In ScummVM, the
// renderer guarantees a sort order based on the creation order of screen items,
// and since the view is created after the pic, it wins and is drawn on top.
// This patch stops the view object from being created at all.
//
// Applies to at least: English CD from King's Quest Collection
// Responsible method: sShowLogo::changeState
static const uint16 mothergooseHiresSignatureLogo[] = {
	0x38, SIG_UINT16(0x8e),          // pushi $8e
	SIG_MAGICDWORD,
	0x76,                            // push0
	0x72, SIG_UINT16(0x82),          // lofsa logo[82]
	0x4a, SIG_UINT16(0x04),          // send $4
	SIG_END
};

static const uint16 mothergooseHiresPatchLogo[] = {
	0x18, 0x18, 0x18,                // waste bytes
	0x18,                            // waste bytes
	0x18, 0x18, 0x18,                // waste bytes
	0x18, 0x18, 0x18,                // waste bytes
	PATCH_END
};

// After finishing the rhyme at the fountain, a horse will appear and walk
// across the screen. The priority of the horse is set too high, so it is
// rendered in front of the fountain instead of behind the fountain. This patch
// corrects the priority so the horse draws behind the fountain.
//
// Applies to at least: English CD from King's Quest Collection
// Responsible method: rhymeScript::changeState
static const uint16 mothergooseHiresSignatureHorse[] = {
	SIG_MAGICDWORD,
	0x39, SIG_SELECTOR8(setPri), // pushi $4a (setPri)
	0x78,                        // push1
	0x38, SIG_UINT16(0xb7),      // pushi $b7
	SIG_END
};

static const uint16 mothergooseHiresPatchHorse[] = {
	PATCH_ADDTOOFFSET(3),
	0x38, PATCH_UINT16(0x59),
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry mothergooseHiresSignatures[] = {
	{  true,   108, "bad logo rendering",                          1, mothergooseHiresSignatureLogo,    mothergooseHiresPatchLogo },
	{  true,   318, "bad horse z-index",                           1, mothergooseHiresSignatureHorse,   mothergooseHiresPatchHorse },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Phantasmagoria

// Phantasmagoria persists audio volumes in the save games, but ScummVM manages
// game volumes through the launcher, so stop the game from overwriting the
// ScummVM volumes with volumes from save games.
// Applies to at least: English CD
static const uint16 phant1SignatureSavedVolume[] = {
	0x7a,                         // push2
	0x39, 0x08,                   // pushi 8
	0x38, SIG_UINT16(0x20b),      // push $20b (readWord)
	0x76,                         // push0
	0x72, SIG_UINT16(0x13c),      // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),       // send 4
	SIG_MAGICDWORD,
	0xa1, 0xbc,                   // sag $bc
	0x36,                         // push
	0x43, 0x76, SIG_UINT16(0x04), // callk DoAudio[76], 4
	0x7a,                         // push2
	0x76,                         // push0
	0x38, SIG_UINT16(0x20b),      // push $20b (readWord)
	0x76,                         // push0
	0x72, SIG_UINT16(0x13c),      // lofsa $13c (PREF.DAT)
	0x4a, SIG_UINT16(0x04),       // send 4
	0xa1, 0xbb,                   // sag $bb
	0x36,                         // push
	0x43, 0x75, SIG_UINT16(0x04), // callk DoSound[75], 4
	SIG_END
};

static const uint16 phant1PatchSavedVolume[] = {
	0x32, PATCH_UINT16(36),         // jmp [to prefFile::close]
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry phantasmagoriaSignatures[] = {
	{  true,   901, "invalid array construction",                  1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,  1111, "ignore audio settings from save game",        1, phant1SignatureSavedVolume,      phant1PatchSavedVolume },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,          sci2BenchmarkPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Phantasmagoria 2

// The interface bars at the top and bottom of the screen fade in and out when
// hovered over. This fade is performed by a script loop that calls kFrameOut
// directly and uses global 227 as the fade delta for each frame. Global 227
// normally contains the value 1, which means that these fades are quite slow.
// This patch replaces the use of global 227 with an immediate value that gives
// a reasonable fade speed.
// Applies to at least: US English
static const uint16 phant2SlowIFadeSignature[] = {
	0x43, 0x21, SIG_UINT16(0), // callk FrameOut, 0
	SIG_MAGICDWORD,
	0x67, 0x03,                // pTos 03 (scratch)
	0x81, 0xe3,                // lag $e3 (227)
	SIG_END
};

static const uint16 phant2SlowIFadePatch[] = {
	PATCH_ADDTOOFFSET(6),      // skip to lag
	0x35, 0x05,                // ldi 5
	PATCH_END
};

// The game uses a spin loop during music transitions which causes the mouse to
// appear unresponsive during scene changes. Replace the spin loop with a call
// to ScummVM kWait.
// Applies to at least: US English
// Responsible method: P2SongPlyr::wait4Fade
static const uint16 phant2Wait4FadeSignature[] = {
	SIG_MAGICDWORD,
	0x76,                      // push0
	0x43, 0x79, SIG_UINT16(0), // callk GetTime, 0
	0xa5, 0x01,                // sat 1
	0x78,                      // push1
	SIG_END
};

static const uint16 phant2Wait4FadePatch[] = {
	0x78,                                     // push1
	0x8d, 0x00,                               // lst temp[0]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x48,                                     // ret
	PATCH_END
};

// The game uses a spin loop when navigating to and from Curtis's computer in
// the office, which causes the mouse to appear unresponsive. Replace the spin
// loop with a call to ScummVM kWait.
// Applies to at least: US English
// Responsible method: localproc 4f04
static const uint16 phant2CompSlideDoorsSignature[] = {
	SIG_MAGICDWORD,
	0x35, 0x00, // ldi 0
	0xa5, 0x00, // sat 0
	0x8d, 0x00, // lst 0
	0x87, 0x01, // lap 1
	SIG_END
};

static const uint16 phant2CompSlideDoorsPatch[] = {
	0x78,                                     // push1
	0x8f, 0x01,                               // lsp param[1]
	0x43, kScummVMWaitId, PATCH_UINT16(0x02), // callk Wait, 2
	0x48,                                     // ret
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry phantasmagoria2Signatures[] = {
	{  true,     0, "slow interface fades",                        3, phant2SlowIFadeSignature,      phant2SlowIFadePatch },
	{  true, 63016, "non-responsive mouse during music fades",     1, phant2Wait4FadeSignature,      phant2Wait4FadePatch },
	{  true, 63019, "non-responsive mouse during computer load",   1, phant2CompSlideDoorsSignature, phant2CompSlideDoorsPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
// Police Quest 1 VGA

// When briefing is about to start in room 15, other officers will get into the room too.
// When one of those officers gets into the way of ego, they will tell the player to sit down.
// But control will be disabled right at that point. Ego may then go to his seat by himself,
// or more often than not will just stand there. The player is unable to do anything.
//
// Sergeant Dooley will then enter the room. Tell the player to sit down 3 times and after
// that it's game over.
//
// Because the Sergeant is telling the player to sit down, one has to assume that the player
// is meant to still be in control. Which is why this script patch removes disabling of player control.
//
// The script also tries to make ego walk to the chair, but it fails because it gets stuck with other
// actors. So I guess the safest way is to remove all of that and let the player do it manually.
//
// The responsible method seems to use a few hardcoded texts, which is why I have to assume that it's
// not used anywhere else. I also checked all scripts and couldn't find any other calls to it.
//
// This of course also happens when using the original interpreter.
//
// Scripts work like this: manX::doit (script 134) triggers gab::changeState, which then triggers rm015::notify
//
// Applies to at least: English floppy
// Responsible method: gab::changeState (script 152)
// Fixes bug: #5865
static const uint16 pq1vgaSignatureBriefingGettingStuck[] = {
	0x76,                                // push0
	0x45, 0x02, 0x00,                    // call export 2 of script 0 (disable control)
	0x38, SIG_ADDTOOFFSET(+2),           // pushi notify
	0x76,                                // push0
	0x81, 0x02,                          // lag global[2] (get current room)
	0x4a, 0x04,                          // send 04
	SIG_MAGICDWORD,
	0x8b, 0x02,                          // lsl local[2]
	0x35, 0x01,                          // ldi 01
	0x02,                                // add
	SIG_END
};

static const uint16 pq1vgaPatchBriefingGettingStuck[] = {
	0x33, 0x0a,                      // jmp to lsl local[2], skip over export 2 and ::notify
	PATCH_END                        // rm015::notify would try to make ego walk to the chair
};

// When at the police station, you can put or get your gun from your locker.
// The script, that handles this, is buggy. It disposes the gun as soon as
//  you click, but then waits 2 seconds before it also closes the locker.
// Problem is that it's possible to click again, which then results in a
//  disposed object getting accessed. This happened to work by pure luck in
//  SSCI.
// This patch changes the code, so that the gun is actually given away
//  when the 2 seconds have passed and the locker got closed.
// Applies to at least: English floppy
// Responsible method: putGun::changeState (script 341)
// Fixes bug: #5705 / #6400
static const uint16 pq1vgaSignaturePutGunInLockerBug[] = {
	0x35, 0x00,                      // ldi 00
	0x1a,                            // eq?
	0x31, 0x25,                      // bnt [next state check]
	SIG_ADDTOOFFSET(+22),            // [skip 22 bytes]
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(put),       // pushi "put"
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag 00
	0x4a, 0x06,                      // send 06 - ego::put(0)
	0x35, 0x02,                      // ldi 02
	0x65, 0x1c,                      // aTop 1c (set timer to 2 seconds)
	0x33, 0x0e,                      // jmp [end of method]
	0x3c,                            // dup --- next state check target
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x08,                      // bnt [end of method]
	0x39, SIG_SELECTOR8(dispose),    // pushi "dispose"
	0x76,                            // push0
	0x72, SIG_UINT16(0x0088),        // lofsa 0088
	0x4a, 0x04,                      // send 04 - locker::dispose
	SIG_END
};

static const uint16 pq1vgaPatchPutGunInLockerBug[] = {
	PATCH_ADDTOOFFSET(+3),
	0x31, 0x1c,                      // bnt [next state check]
	PATCH_ADDTOOFFSET(+22),
	0x35, 0x02,                      // ldi 02
	0x65, 0x1c,                      // aTop 1c (set timer to 2 seconds)
	0x33, 0x17,                      // jmp [end of method]
	0x3c,                            // dup --- next state check target
	0x35, 0x01,                      // ldi 01
	0x1a,                            // eq?
	0x31, 0x11,                      // bnt [end of method]
	0x38, PATCH_SELECTOR16(put),     // pushi "put"
	0x78,                            // push1
	0x76,                            // push0
	0x81, 0x00,                      // lag 00
	0x4a, 0x06,                      // send 06 - ego::put(0)
	PATCH_END
};

// When restoring a saved game, which was made while driving around,
//  the game didn't redraw the map. This also happened in Sierra SCI.
//
// The map is a picture resource and drawn over the main picture.
//  This is called an "overlay" in SCI. This wasn't implemented properly.
//  We fix it by actually implementing it properly.
//
// Applies to at least: English floppy
// Responsible method: rm500::init, changeOverlay::changeState (script 500)
// Fixes bug: #5016
static const uint16 pq1vgaSignatureMapSaveRestoreBug[] = {
	0x39, 0x04,                          // pushi 04
	SIG_ADDTOOFFSET(+2),                 // skip either lsg global[f9] or pTos register
	SIG_MAGICDWORD,
	0x38, 0x64, 0x80,                    // pushi 8064
	0x76,                                // push0
	0x89, 0x28,                          // lsg global[28]
	0x43, 0x08, 0x08,                    // kDrawPic (8)
	SIG_END
};

static const uint16 pq1vgaPatchMapSaveRestoreBug[] = {
	0x38, PATCH_SELECTOR16(overlay), // pushi "overlay"
	0x7a,                            // push2
	0x89, 0xf9,                      // lsg global[f9]
	0x39, 0x64,                      // pushi 64 (no transition)
	0x81, 0x02,                      // lag global[02] (current room object)
	0x4a, 0x08,                      // send 08
	0x18,                            // not (waste byte)
	PATCH_END
};

//          script, description,                                         signature                            patch
static const SciScriptPatcherEntry pq1vgaSignatures[] = {
	{  true,   152, "getting stuck while briefing is about to start", 1, pq1vgaSignatureBriefingGettingStuck, pq1vgaPatchBriefingGettingStuck },
	{  true,   341, "put gun in locker bug",                          1, pq1vgaSignaturePutGunInLockerBug,    pq1vgaPatchPutGunInLockerBug },
	{  true,   500, "map save/restore bug",                           2, pq1vgaSignatureMapSaveRestoreBug,    pq1vgaPatchMapSaveRestoreBug },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Police Quest 4

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry pq4Signatures[] = {
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Police Quest: SWAT

// The init code that runs when PQ:SWAT starts up unconditionally resets the
// master sound volume to 127, but the game should always use the volume stored
// in ScummVM.
// Applies to at least: English CD
static const uint16 pqSwatSignatureVolumeReset1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x21a), // pushi $21a (masterVolume)
	0x78,                    // push1
	0x39, 0x7f,              // push $7f
	0x54, SIG_UINT16(0x06),  // self 6
	SIG_END
};

static const uint16 pqSwatPatchVolumeReset1[] = {
	0x32, PATCH_UINT16(6), // jmp 6 [past volume reset]
	PATCH_END
};

// pqInitCode::doit
static const uint16 pqSwatSignatureVolumeReset2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x21a), // pushi $21a (masterVolume)
	0x78,                    // push1
	0x39, 0x0f,              // pushi $f
	0x81, 0x01,              // lag 1
	0x4a, SIG_UINT16(0x06),  // send 6
	SIG_END
};

static const uint16 pqSwatPatchVolumeReset2[] = {
	0x32, PATCH_UINT16(8), // jmp 8 [past volume reset]
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry pqSwatSignatures[] = {
	{  true,     0, "disable volume reset on startup",             1, pqSwatSignatureVolumeReset1,       pqSwatPatchVolumeReset1 },
	{  true,     1, "disable volume reset on startup",             1, pqSwatSignatureVolumeReset2,       pqSwatPatchVolumeReset2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
//  At the healer's house there is a bird's nest up on the tree.
//   The player can throw rocks at it until it falls to the ground.
//   The hero will then grab the item, that is in the nest.
//
//  When running is active, the hero will not reach the actual destination
//   and because of that, the game will get stuck.
//
//  We just change the coordinate of the destination slightly, so that walking,
//   sneaking and running work.
//
//  This bug was fixed by Sierra at least in the Japanese PC-9801 version.
// Applies to at least: English floppy (1.000, 1.012)
// Responsible method: pickItUp::changeState (script 54)
// Fixes bug: #6407
static const uint16 qfg1egaSignatureThrowRockAtNest[] = {
	0x4a, 0x04,                         // send 04 (nest::x)
	0x36,                               // push
	SIG_MAGICDWORD,
	0x35, 0x0f,                         // ldi 0f (15d)
	0x02,                               // add
	0x36,                               // push
	SIG_END
};

static const uint16 qfg1egaPatchThrowRockAtNest[] = {
	PATCH_ADDTOOFFSET(+3),
	0x35, 0x12,                         // ldi 12 (18d)
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry qfg1egaSignatures[] = {
	{  true,    54, "throw rock at nest while running",            1, qfg1egaSignatureThrowRockAtNest,     qfg1egaPatchThrowRockAtNest },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 215 of qfg1vga pointBox::doit actually processes button-presses
//   during fighting with monsters. It strangely also calls kGetEvent. Because
//   the main User::doit also calls kGetEvent it's pure luck, where the event
//   will hit. It's the same issue as in freddy pharkas and if you turn dos-box
//   to max cycles, sometimes clicks also won't get registered. Strangely it's
//   not nearly as bad as in our sci, but these differences may be caused by
//   timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
// Applies to at least: English floppy
// Responsible method: pointBox::doit
static const uint16 qfg1vgaSignatureFightEvents[] = {
	0x39, SIG_MAGICDWORD,
	SIG_SELECTOR8(new),                 // pushi "new"
	0x76,                               // push0
	0x51, 0x07,                         // class Event
	0x4a, 0x04,                         // send 04 - call Event::new
	0xa5, 0x00,                         // sat temp[0]
	0x78,                               // push1
	0x76,                               // push0
	0x4a, 0x04,                         // send 04 - read Event::x
	0xa5, 0x03,                         // sat temp[3]
	0x76,                               // push0 (selector y)
	0x76,                               // push0
	0x85, 0x00,                         // lat temp[0]
	0x4a, 0x04,                         // send 04 - read Event::y
	0x36,                               // push
	0x35, 0x0a,                         // ldi 0a
	0x04,                               // sub (poor mans localization) ;-)
	SIG_END
};

static const uint16 qfg1vgaPatchFightEvents[] = {
	0x38, PATCH_SELECTOR16(curEvent), // pushi 15a (selector curEvent)
	0x76,                            // push0
	0x81, 0x50,                      // lag global[50]
	0x4a, 0x04,                      // send 04 - read User::curEvent -> needs one byte more than previous code
	0xa5, 0x00,                      // sat temp[0]
	0x78,                            // push1
	0x76,                            // push0
	0x4a, 0x04,                      // send 04 - read Event::x
	0xa5, 0x03,                      // sat temp[3]
	0x76,                            // push0 (selector y)
	0x76,                            // push0
	0x85, 0x00,                      // lat temp[0]
	0x4a, 0x04,                      // send 04 - read Event::y
	0x39, 0x00,                      // pushi 00
	0x02,                            // add (waste 3 bytes) - we don't need localization, User::doit has already done it
	PATCH_END
};

// Script 814 of QFG1VGA is responsible for showing dialogs. However, the death
// screen message shown when the hero dies in room 64 (ghost room) is too large
// (254 chars long). Since the window header and main text are both stored in
// temp space, this is an issue, as the scripts read the window header, then the
// window text, which erases the window header text because of its length. To
// fix that, we allocate more temp space and move the pointer used for the
// window header a little bit, wherever it's used in script 814.
// Fixes bug: #6139.

// Patch 1: Increase temp space
static const uint16 qfg1vgaSignatureTempSpace[] = {
	SIG_MAGICDWORD,
	0x3f, 0xba,                         // link 0xba
	0x87, 0x00,                         // lap 0
	SIG_END
};

static const uint16 qfg1vgaPatchTempSpace[] = {
	0x3f, 0xca,                         // link 0xca
	PATCH_END
};

// Patch 2: Move the pointer used for the window header a little bit
static const uint16 qfg1vgaSignatureDialogHeader[] = {
	SIG_MAGICDWORD,
	0x5b, 0x04, 0x80,                   // lea temp[0x80]
	0x36,                               // push
	SIG_END
};

static const uint16 qfg1vgaPatchDialogHeader[] = {
	0x5b, 0x04, 0x90,                   // lea temp[0x90]
	PATCH_END
};

// When clicking on the crusher in room 331, Ego approaches him to talk to him,
// an action that is handled by moveToCrusher::changeState in script 331. The
// scripts set Ego to move close to the crusher, but when Ego is sneaking instead
// of walking, the target coordinates specified by script 331 are never reached,
// as Ego is making larger steps, and never reaches the required spot. This is an
// edge case that can occur when Ego is set to sneak. Normally, when clicking on
// the crusher, ego is supposed to move close to position 79, 165. We change it
// to 85, 165, which is not an edge case thus the freeze is avoided.
// Fixes bug: #6180
static const uint16 qfg1vgaSignatureMoveToCrusher[] = {
	SIG_MAGICDWORD,
	0x51, 0x1f,                         // class Motion
	0x36,                               // push
	0x39, 0x4f,                         // pushi 4f (79 - x)
	0x38, SIG_UINT16(0x00a5),           // pushi 00a5 (165 - y)
	0x7c,                               // pushSelf
	SIG_END
};

static const uint16 qfg1vgaPatchMoveToCrusher[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x55,                         // pushi 55 (85 - x)
	PATCH_END
};

// Same pathfinding bug as above, where Ego is set to move to an impossible
// spot when sneaking. In GuardsTrumpet::changeState, we change the final
// location where Ego is moved from 111, 111 to 116, 116.
// target coordinate is really problematic here.
//
// 114, 114 works when the speed slider is all the way up, but doesn't work
// when the speed slider is not.
//
// It seems that this bug was fixed by Sierra for the Macintosh version.
//
// Applies to at least: English PC floppy
// Responsible method: GuardsTrumpet::changeState(8)
// Fixes bug: #6248
static const uint16 qfg1vgaSignatureMoveToCastleGate[] = {
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	SIG_MAGICDWORD,
	0x36,                               // push
	0x39, 0x6f,                         // pushi 6f (111d)
	0x3c,                               // dup (111d) - coordinates 111, 111
	0x7c,                               // pushSelf
	SIG_END
};

static const uint16 qfg1vgaPatchMoveToCastleGate[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x74,                         // pushi 74 (116d), changes coordinates to 116, 116
	PATCH_END
};

// Typo in the original Sierra scripts
// Looking at a cheetaur resulted in a text about a Saurus Rex
// The code treats both monster types the same.
// Applies to at least: English floppy
// Responsible method: smallMonster::doVerb
// Fixes bug #6249
static const uint16 qfg1vgaSignatureCheetaurDescription[] = {
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x01b8),           // ldi 01b8
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt 16
	0x38, SIG_SELECTOR16(say),          // pushi 0127h (selector "say")
	0x39, 0x06,                         // pushi 06
	0x39, 0x03,                         // pushi 03
	0x78,                               // push1
	0x39, 0x12,                         // pushi 12 -> monster type Saurus Rex
	SIG_END
};

static const uint16 qfg1vgaPatchCheetaurDescription[] = {
	PATCH_ADDTOOFFSET(+14),
	0x39, 0x11,                         // pushi 11 -> monster type cheetaur
	PATCH_END
};

// In the "funny" room (Yorick's room) in QfG1 VGA, pulling the chain and
//  then pressing the button on the right side of the room results in
//  a broken game. This also happens in SSCI.
// Problem is that the Sierra programmers forgot to disable the door, that
//  gets opened by pulling the chain. So when ego falls down and then
//  rolls through the door, one method thinks that the player walks through
//  it and acts that way and the other method is still doing the roll animation.
// Local 5 of that room is a timer, that closes the door (object door11).
// Setting it to 1 during happyFace::changeState(0) stops door11::doit from
//  calling goTo6::init, so the whole issue is stopped from happening.
//
// Applies to at least: English floppy
// Responsible method: happyFace::changeState, door11::doit
// Fixes bug #6181
static const uint16 qfg1vgaSignatureFunnyRoomFix[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0025),           // bnt 0025 [-> next state]
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal 4e
	SIG_END
};

static const uint16 qfg1vgaPatchFunnyRoomFix[] = {
	PATCH_ADDTOOFFSET(+3),
	0x2e, PATCH_UINT16(0x0029),         // bt 0029 [-> next state] - saves 4 bytes
	0x35, 0x01,                         // ldi 01
	0xa3, 0x4e,                         // sal 4e
	0xa3, 0x05,                         // sal 05 (sets local 5 to 1)
	0xa3, 0x05,                         // and again to make absolutely sure (actually to waste 2 bytes)
	PATCH_END
};

// The player is able to buy (and also steal) potions in the healer's hut
//  Strangely Sierra delays the actual buy/get potion code for 60 ticks
//  Why they did that is unknown. The code is triggered anyway only after
//  the relevant dialog boxes are closed.
//
// This delay causes problems in case the user quickly enters the inventory.
// That's why we change the amount of ticks to 1, so that the remaining states
//  are executed right after the dialog boxes are closed.
//
// Applies to at least: English floppy
// Responsible method: cueItScript::changeState
// Fixes bug #6706
static const uint16 qfg1vgaSignatureHealerHutNoDelay[] = {
	0x65, 0x14,                         // aTop 14 (state)
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 00
	0x1a,                               // eq?
	0x31, 0x07,                         // bnt 07 [-> next state]
	SIG_MAGICDWORD,
	0x35, 0x3c,                         // ldi 3c (60 ticks)
	0x65, 0x20,                         // aTop ticks
	0x32,                               // jmp [-> end of method]
	SIG_END
};

static const uint16 qfg1vgaPatchHealerHutNoDelay[] = {
	PATCH_ADDTOOFFSET(+9),
	0x35, 0x01,                         // ldi 01 (1 tick only, so that execution will resume as soon as dialog box is closed)
	PATCH_END
};

// When following the white stag, you can actually enter the 2nd room from the mushroom/fairy location,
//  which results in ego entering from the top. When you then throw a dagger at the stag, one animation
//  frame will stay on screen, because of a script bug.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: stagHurt::changeState
// Fixes bug #6135
static const uint16 qfg1vgaSignatureWhiteStagDagger[] = {
	0x87, 0x01,                         // lap param[1]
	0x65, 0x14,                         // aTop state
	0x36,                               // push
	0x3c,                               // dup
	0x35, 0x00,                         // ldi 0
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt [next parameter check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb export 2 from script 0, 0
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(say),          // pushi 0127h (selector "say")
	0x39, 0x05,                         // pushi 05
	0x39, 0x03,                         // pushi 03
	0x39, 0x51,                         // pushi 51h
	0x76,                               // push0
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[5Bh] -> qg1Messager
	0x4a, 0x0e,                         // send 0Eh -> qg1Messager::say(3, 51h, 0, 0, stagHurt)
	0x33, 0x12,                         // jmp -> [ret]
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt [ret]
	0x38,                               // pushi...
	SIG_ADDTOOFFSET(+11),
	0x3a,                               // toss
	0x48,                               // ret
	SIG_END
};

static const uint16 qfg1vgaPatchWhiteStagDagger[] = {
	PATCH_ADDTOOFFSET(+4),
	0x2f, 0x05,                         // bt [next check] (state != 0)
	// state = 0 code
	0x35, 0x01,                         // ldi 1
	0x65, 0x1a,                         // aTop cycles
	0x48,                               // ret
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x1a,                               // eq?
	0x31, 0x16,                         // bnt [state = 2 code]
	// state = 1 code
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // callb export 2 from script 0, 0
	0x38, PATCH_SELECTOR16(say),        // pushi 0127h (selector "say")
	0x39, 0x05,                         // pushi 05
	0x39, 0x03,                         // pushi 03
	0x39, 0x51,                         // pushi 51h
	0x76,                               // push0
	0x76,                               // push0
	0x7c,                               // pushSelf
	0x81, 0x5b,                         // lag global[5Bh] -> qg1Messager
	0x4a, 0x0e,                         // send 0Eh -> qg1Messager::say(3, 51h, 0, 0, stagHurt)
	0x48,                               // ret
	// state = 2 code
	PATCH_ADDTOOFFSET(+13),
	0x48,                               // ret (remove toss)
	PATCH_END
};

// The dagger range has a script bug that can freeze the game or cause Brutus to kill you even after you've killed him.
// This is a bug in the original game.
//
// When Bruno leaves, a 300 tick countdown starts. If you kill Brutus or leave room 73 within those 300 ticks then
// the game is left in a broken state. For the rest of the game, if you ever return to the dagger range from the
// east or west during the first half of the day then the game will freeze or Brutus will come back to life
// and kill you, even if you already killed him.
//
// Special thanks, credits and kudos to sluicebox, who did a ton of research on this and even found this game bug originally.
//
// Applies to at least: English floppy, Mac floppy
// Responsible method: brutusWaits::changeState
// Fixes bug #9558
static const uint16 qfg1vgaSignatureBrutusScriptFreeze[] = {
	0x78,                               // push1
	0x38, SIG_UINT16(0x144),            // pushi 144h (324d)
	0x45, 0x05, 0x02,                   // call export 5 of script 0
	SIG_MAGICDWORD,
	0x34, SIG_UINT16(0x12c),            // ldi 12Ch (300d)
	0x65, 0x20,                         // aTop ticks
	SIG_END
};

static const uint16 qfg1vgaPatchBrutusScriptFreeze[] = {
	0x34, PATCH_UINT16(0),              // ldi 0 (waste 7 bytes)
	0x35, 0x00,                         // ldi 0
	0x35, 0x00,                         // ldi 0
	PATCH_END
};

//          script, description,                                      signature                            patch
static const SciScriptPatcherEntry qfg1vgaSignatures[] = {
	{  true,    41, "moving to castle gate",                       1, qfg1vgaSignatureMoveToCastleGate,    qfg1vgaPatchMoveToCastleGate },
	{  true,    55, "healer's hut, no delay for buy/steal",        1, qfg1vgaSignatureHealerHutNoDelay,    qfg1vgaPatchHealerHutNoDelay },
	{  true,    73, "brutus script freeze glitch",                 1, qfg1vgaSignatureBrutusScriptFreeze,  qfg1vgaPatchBrutusScriptFreeze },
	{  true,    77, "white stag dagger throw animation glitch",    1, qfg1vgaSignatureWhiteStagDagger,     qfg1vgaPatchWhiteStagDagger },
	{  true,    96, "funny room script bug fixed",                 1, qfg1vgaSignatureFunnyRoomFix,        qfg1vgaPatchFunnyRoomFix },
	{  true,   210, "cheetaur description fixed",                  1, qfg1vgaSignatureCheetaurDescription, qfg1vgaPatchCheetaurDescription },
	{  true,   215, "fight event issue",                           1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   216, "weapon master event issue",                   1, qfg1vgaSignatureFightEvents,         qfg1vgaPatchFightEvents },
	{  true,   331, "moving to crusher",                           1, qfg1vgaSignatureMoveToCrusher,       qfg1vgaPatchMoveToCrusher },
	{  true,   814, "window text temp space",                      1, qfg1vgaSignatureTempSpace,           qfg1vgaPatchTempSpace },
	{  true,   814, "dialog header offset",                        3, qfg1vgaSignatureDialogHeader,        qfg1vgaPatchDialogHeader },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================

// This is a very complicated bug.
// When the player encounters an enemy in the desert while riding a saurus and later
//  tries to get back on it by entering "ride", the game will not give control back
//  to the player.
//
// This is caused by script mountSaurus getting triggered twice.
//  Once by entering the command "ride" and then a second time by a proximity check.
//
// Both are calling mountSaurus::init() in script 20, this one disables controls
//  then mountSaurus::changeState() from script 660 is triggered
//  mountSaurus::changeState(5) finally calls mountSaurus::dispose(), which is also in script 20
//  which finally re-enables controls
//
// A fix is difficult to implement. The code in script 20 is generic and used by multiple objects
//
// Originally I decided to change the responsible globals (66h and A1h) during mountSaurus::changeState(5).
//  This worked as far as for controls, but mountSaurus::init changes a few selectors of ego as well, which
//  won't get restored in that situation, which then messes up room changes and other things.
//
// I have now decided to change sheepScript::changeState(2) in script 665 instead.
//
// This fix could cause issues in case there is a cutscene, where ego is supposed to get onto the saurus using
//  sheepScript.
//
// Applies to at least: English PC Floppy, English Amiga Floppy
// Responsible method: mountSaurus::changeState(), mountSaurus::init(), mountSaurus::dispose()
// Fixes bug: #5156
static const uint16 qfg2SignatureSaurusFreeze[] = {
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 5
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0043),           // bnt [ret]
	0x76,                               // push0
	SIG_ADDTOOFFSET(+61),               // skip to dispose code
	0x39, SIG_SELECTOR8(dispose),       // pushi "dispose"
	0x76,                               // push0
	0x54, 0x04,                         // self 04
	SIG_END
};

static const uint16 qfg2PatchSaurusFreeze[] = {
	0x81, 0x66,                         // lag 66h
	0x2e, SIG_UINT16(0x0040),           // bt [to dispose code]
	0x35, 0x00,                         // ldi 0 (waste 2 bytes)
	PATCH_END
};

// Script 944 in QFG2 contains the FileSelector system class, used in the
// character import screen. This gets incorrectly called constantly, whenever
// the user clicks on a button in order to refresh the file list. This was
// probably done because it would be easier to refresh the list whenever the
// user inserted a new floppy disk, or changed directory. The problem is that
// the script has a bug, and it invalidates the text of the entries in the
// list. This has a high probability of breaking, as the user could change the
// list very quickly, or the garbage collector could kick in and remove the
// deleted entries. We don't allow the user to change the directory, thus the
// contents of the file list are constant, so we can avoid the constant file
// and text entry refreshes whenever a button is pressed, and prevent possible
// crashes because of these constant quick object reallocations.
// Fixes bug: #5096
static const uint16 qfg2SignatureImportDialog[] = {
	0x63, SIG_MAGICDWORD, 0x20,         // pToa text
	0x30, SIG_UINT16(0x000b),           // bnt [next state]
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	0x36,                               // push
	0x43, 0x72, 0x04,                   // callk Memory 4
	0x35, 0x00,                         // ldi 00
	0x65, 0x20,                         // aTop text
	SIG_END
};

static const uint16 qfg2PatchImportDialog[] = {
	PATCH_ADDTOOFFSET(+5),
	0x48,                               // ret
	PATCH_END
};

// Quest For Glory 2 character import doesn't properly set the character type
//  in versions 1.102 and below, which makes all importerted characters a fighter.
//
// Sierra released an official patch. However the fix is really easy to
//  implement on our side, so we also patch the flaw in here in case we find it.
//
// The version released on GOG is 1.102 without this patch applied, so us
//  patching it is quite useful.
//
// Applies to at least: English Floppy
// Responsible method: importHero::changeState
// Fixes bug: inside versions 1.102 and below
static const uint16 qfg2SignatureImportCharType[] = {
	0x35, 0x04,                         // ldi 04
	0x90, SIG_UINT16(0x023b),           // lagi global[23Bh]
	0x02,                               // add
	0x36,                               // push
	0x35, 0x04,                         // ldi 04
	0x08,                               // div
	0x36,                               // push
	0x35, 0x0d,                         // ldi 0D
	0xb0, SIG_UINT16(0x023b),           // sagi global[023Bh]
	0x8b, 0x1f,                         // lsl local[1Fh]
	0x35, 0x05,                         // ldi 05
	SIG_MAGICDWORD,
	0xb0, SIG_UINT16(0x0150),           // sagi global[0150h]
	0x8b, 0x02,                         // lsl local[02h]
	SIG_END
};

static const uint16 qfg2PatchImportCharType[] = {
	0x80, PATCH_UINT16(0x023f),         // lag global[23Fh] <-- patched to save 2 bytes
	0x02,                               // add
	0x36,                               // push
	0x35, 0x04,                         // ldi 04
	0x08,                               // div
	0x36,                               // push
	0xa8, SIG_UINT16(0x0248),           // ssg global[0248h] <-- patched to save 2 bytes
	0x8b, 0x1f,                         // lsl local[1Fh]
	0xa8, SIG_UINT16(0x0155),           // ssg global[0155h] <-- patched to save 2 bytes
	// new code, directly from the official sierra patch file
	0x83, 0x01,                         // lal local[01h]
	0xa1, 0xbb,                         // sag global[BBh]
	0xa1, 0x73,                         // sag global[73h]
	PATCH_END
};

//          script, description,                                      signature                    patch
static const SciScriptPatcherEntry qfg2Signatures[] = {
	{  true,   665, "getting back on saurus freeze fix",           1, qfg2SignatureSaurusFreeze,   qfg2PatchSaurusFreeze },
	{  true,   805, "import character type fix",                   1, qfg2SignatureImportCharType, qfg2PatchImportCharType },
	{  true,   944, "import dialog continuous calls",              1, qfg2SignatureImportDialog,   qfg2PatchImportDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Patch for the import screen in QFG3, same as the one for QFG2 above
static const uint16 qfg3SignatureImportDialog[] = {
	0x63, SIG_MAGICDWORD, 0x2a,         // pToa text
	0x31, 0x0b,                         // bnt [next state]
	0x7a,                               // push2
	0x39, 0x03,                         // pushi 03
	0x36,                               // push
	0x43, 0x72, 0x04,                   // callk Memory 4
	0x35, 0x00,                         // ldi 00
	0x65, 0x2a,                         // aTop text
	SIG_END
};

static const uint16 qfg3PatchImportDialog[] = {
	PATCH_ADDTOOFFSET(+4),
	0x48,                               // ret
	PATCH_END
};



// ===========================================================================
// Patch for the Woo dialog option in Uhura's conversation.
// Problem: The Woo dialog option (0xffb5) is negative, and therefore
// treated as an option opening a submenu. This leads to uhuraTell::doChild
// being called, which calls hero::solvePuzzle and then proceeds with
// Teller::doChild to open the submenu. However, there is no actual submenu
// defined for option -75 since -75 does not show up in uhuraTell::keys.
// This will cause Teller::doChild to run out of bounds while scanning through
// uhuraTell::keys.
// Strategy: there is another conversation option in uhuraTell::doChild calling
// hero::solvePuzzle (0xfffc) which does a ret afterwards without going to
// Teller::doChild. We jump to this call of hero::solvePuzzle to get that same
// behaviour.
// Applies to at least: English, German, Italian, French, Spanish Floppy
// Responsible method: uhuraTell::doChild
// Fixes bug: #5172
static const uint16 qfg3SignatureWooDialog[] = {
	SIG_MAGICDWORD,
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb6,                         // ldi b6
	0x1a,                               // eq?
	0x2f, 0x05,                         // bt 05
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0x9b,                         // ldi 9b
	0x1a,                               // eq?
	0x31, 0x0c,                         // bnt 0c
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi 0297
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag 00
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialog[] = {
	PATCH_ADDTOOFFSET(+0x29),
	0x33, 0x11,                         // jmp to 0x6a2, the call to hero::solvePuzzle for 0xFFFC
	PATCH_END
};

// Alternative version, with uint16 offsets, for GOG release of QfG3.
static const uint16 qfg3SignatureWooDialogAlt[] = {
	SIG_MAGICDWORD,
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb6,                         // ldi b6
	0x1a,                               // eq?
	0x2e, SIG_UINT16(0x0005),           // bt 05
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0x9b,                         // ldi 9b
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x000c),           // bnt 0c
	0x38, SIG_SELECTOR16(solvePuzzle),  // pushi 0297
	0x7a,                               // push2
	0x38, SIG_UINT16(0x010c),           // pushi 010c
	0x7a,                               // push2
	0x81, 0x00,                         // lag 00
	0x4a, 0x08,                         // send 08
	0x67, 0x12,                         // pTos 12 (query)
	0x35, 0xb5,                         // ldi b5
	SIG_END
};

static const uint16 qfg3PatchWooDialogAlt[] = {
	PATCH_ADDTOOFFSET(+0x2C),
	0x33, 0x12,                         // jmp to 0x708, the call to hero::solvePuzzle for 0xFFFC
	PATCH_END
};

// When exporting characters at the end of Quest for Glory 3, the underlying
//  code has issues with values, that are above 9999.
//  For further study: https://github.com/Blazingstix/QFGImporter/blob/master/QFGImporter/QFGImporter/QFG3.txt
//
// If a value is above 9999, parts or even the whole character file will get corrupted.
//
// We are fixing the code because of that. We are patching code, that is calculating the checksum
//  and add extra code to lower such values to 9999.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: saveHero::changeState
// Fixes bug #6807
static const uint16 qfg3SignatureExportChar[] = {
	0x35, SIG_ADDTOOFFSET(+1),          // ldi  00 / ldi 01 (2 loops, we patch both)
	0xa5, 0x00,                         // sat  temp[0] [contains index to data]
	0x8d, 0x00,                         // lst  temp[0]
	SIG_MAGICDWORD,
	0x35, 0x2c,                         // ldi  2c
	0x22,                               // lt?  [index above or equal 2Ch (44d)?
	0x31, 0x23,                         // bnt  [exit loop]
	// from this point it's actually useless code, maybe a sci compiler bug
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0x9b, 0x00,                         // lsli local[0] ---------- load local[0 + ACC] onto stack
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0xb3, 0x00,                         // sali local[0] ---------- save stack to local[0 + ACC]
	// end of useless code
	0x8b, SIG_ADDTOOFFSET(+1),          // lsl  local[36h/37h] ---- load local[36h/37h] onto stack
	0x8d, 0x00,                         // lst  temp[0]
	0x35, 0x01,                         // ldi  01
	0x02,                               // add
	0x93, 0x00,                         // lali local[0] ---------- load local[0 + ACC] into ACC
	0x02,                               // add -------------------- add ACC + stack and put into ACC
	0xa3, SIG_ADDTOOFFSET(+1),          // sal  local[36h/37h] ---- save ACC to local[36h/37h]
	0x8d, 0x00,                         // lst temp[0] ------------ temp[0] to stack
	0x35, 0x02,                         // ldi 02
	0x02,                               // add -------------------- add 2 to stack
	0xa5, 0x00,                         // sat temp[0] ------------ save ACC to temp[0]
	0x33, 0xd6,                         // jmp [loop]
	SIG_END
};

static const uint16 qfg3PatchExportChar[] = {
	PATCH_ADDTOOFFSET(+11),
	0x85, 0x00,                         // lat  temp[0]
	0x9b, 0x01,                         // lsli local[0] + 1 ------ load local[ ACC + 1] onto stack
	0x3c,                               // dup
	0x34, PATCH_UINT16(0x2710),         // ldi  2710h (10000d)
	0x2c,                               // ult? ------------------- is value smaller than 10000?
	0x2f, 0x0a,                         // bt   [jump over]
	0x3a,                               // toss
	0x38, PATCH_UINT16(0x270f),         // pushi 270fh (9999d)
	0x3c,                               // dup
	0x85, 0x00,                         // lat  temp[0]
	0xba, PATCH_UINT16(0x0001),         // ssli local[0] + 1 ------ save stack to local[ ACC + 1] (UINT16 to waste 1 byte)
	// jump offset
	0x83, PATCH_GETORIGINALBYTE(+26),   // lal  local[37h/36h] ---- load local[37h/36h] into ACC
	0x02,                               // add -------------------- add local[37h/36h] + data value
	PATCH_END
};

// Quest for Glory 3 doesn't properly import the character type of Quest for Glory 1 character files.
//  This issue was never addressed. It's caused by Sierra reading data directly from the local
//  area, which is only set by Quest For Glory 2 import data, instead of reading the properly set global variable.
//
// We fix it, by also directly setting the local variable.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: importHero::changeState(4)
static const uint16 qfg3SignatureImportQfG1Char[] = {
	SIG_MAGICDWORD,
	0x82, SIG_UINT16(0x0238),           // lal local[0x0238]
	0xa0, SIG_UINT16(0x016a),           // sag global[0x016a]
	0xa1, 0x7d,                         // sag global[0x7d]
	0x35, 0x01,                         // ldi 01
	0x99, 0xfb,                         // lsgi global[0xfb]
	SIG_END
};

static const uint16 qfg3PatchImportQfG1Char[] = {
	PATCH_ADDTOOFFSET(+8),
	0xa3, 0x01,                         // sal 01           -> also set local[01]
	0x89, 0xfc,                         // lsg global[0xFD] -> save 2 bytes
	PATCH_END
};

// The chief in his hut (room 640) is not drawn using the correct priority,
//  which results in a graphical glitch. This is a game bug and also happens
//  in Sierra's SCI. We adjust priority accordingly to fix it.
//
// Applies to at least: English, French, German, Italian, Spanish floppy
// Responsible method: heap in script 640
// Fixes bug #5173
static const uint16 qfg3SignatureChiefPriority[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(0x0002),                 // yStep     0x0002
	SIG_UINT16(0x0281),                 // view      0x0281
	SIG_UINT16(0x0000),                 // loop      0x0000
	SIG_UINT16(0x0000),                 // cel       0x0000
	SIG_UINT16(0x0000),                 // priority  0x0000
	SIG_UINT16(0x0000),                 // underbits 0x0000
	SIG_UINT16(0x1000),                 // signal    0x1000
	SIG_END
};

static const uint16 qfg3PatchChiefPriority[] = {
	PATCH_ADDTOOFFSET(+8),
	PATCH_UINT16(0x000A),               // new priority 0x000A (10d)
	PATCH_ADDTOOFFSET(+2),
	PATCH_UINT16(0x1010),               // signal       0x1010 (set fixed priority flag)
	PATCH_END
};

// There are 3 points that can't be achieved in the game. They should've been
// awarded for telling Rakeesh and Kreesha (room 285) about the Simabni
// initiation.
// However the array of posibble messages the hero can tell in that room
// (local 156) is missing the "Tell about Initiation" message (#31) which
// awards these points.
// This patch adds the message to that array, thus allowing the hero to tell
// that message (after completing the initiation) and gain the 3 points.
// A side effect of increasing the local156 array is that the next local
// array is shifted and shrinks in size from 4 words to 3. The patch changes
// the 2 locations in the script that reference that array, to point to the new
// location ($aa --> $ab). It is safe to shrink the 2nd array to 3 words
// because only the first element in it is ever used.
//
// Note: You have to re-enter the room in case a saved game was loaded from a
// previous version of ScummVM and that saved game was made inside that room.
//
// Applies to: English, French, German, Italian, Spanish and the GOG release.
// Responsible method: heap in script 285
// Fixes bug #7086
static const uint16 qfg3SignatureMissingPoints1[] = {
	// local[$9c] = [0 -41 -76 1 -30 -77 -33 -34 -35 -36 -37 -42 -80 999]
	// local[$aa] = [0 0 0 0]
	SIG_UINT16(0x0000),                 //   0 START MARKER
	SIG_MAGICDWORD,
	SIG_UINT16(0xFFD7),                 // -41 "Greet"
	SIG_UINT16(0xFFB4),                 // -76 "Say Good-bye"
	SIG_UINT16(0x0001),                 //   1 "Tell about Tarna"
	SIG_UINT16(0xFFE2),                 // -30 "Tell about Simani"
	SIG_UINT16(0xFFB3),                 // -77 "Tell about Prisoner"
	SIG_UINT16(0xFFDF),                 // -33 "Dispelled Leopard Lady"
	SIG_UINT16(0xFFDE),                 // -34 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDD),                 // -35 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDC),                 // -36 "Tell about Leopard Lady"
	SIG_UINT16(0xFFDB),                 // -37 "Tell about Village"
	SIG_UINT16(0xFFD6),                 // -42 "Greet"
	SIG_UINT16(0xFFB0),                 // -80 "Say Good-bye"
	SIG_UINT16(0x03E7),                 // 999 END MARKER
	SIG_ADDTOOFFSET(+2),                // local[$aa][0]
	SIG_END
};

static const uint16 qfg3PatchMissingPoints1[] = {
	PATCH_ADDTOOFFSET(+14),
	PATCH_UINT16(0xFFE1),               // -31 "Tell about Initiation"
	PATCH_UINT16(0xFFDE),               // -34 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDD),               // -35 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDC),               // -36 "Tell about Leopard Lady"
	PATCH_UINT16(0xFFDB),               // -37 "Tell about Village"
	PATCH_UINT16(0xFFD6),               // -42 "Greet"
	PATCH_UINT16(0xFFB0),               // -80 "Say Good-bye"
	PATCH_UINT16(0x03E7),               // 999 END MARKER
	PATCH_GETORIGINALUINT16(+28),       // local[$aa][0]
	PATCH_END
};

static const uint16 qfg3SignatureMissingPoints2a[] = {
	SIG_MAGICDWORD,
	0x35, 0x00,                         // ldi 0
	0xb3, 0xaa,                         // sali local[$aa]
	SIG_END
};

static const uint16 qfg3SignatureMissingPoints2b[] = {
	SIG_MAGICDWORD,
	0x36,                               // push
	0x5b, 0x02, 0xaa,                   // lea local[$aa]
	SIG_END
};

static const uint16 qfg3PatchMissingPoints2[] = {
	PATCH_ADDTOOFFSET(+3),
	0xab,                               // local[$aa] ==> local[$ab]
	PATCH_END
};


// Partly WORKAROUND:
// During combat, the game is not properly throttled. That's because the game uses
// an inner loop for combat and does not iterate through the main loop.
// It also doesn't call kGameIsRestarting. This may get fixed properly at some point
// by rewriting the speed throttler.
//
// Additionally Sierra set the cycle speed of the hero to 0. Which explains
// why the actions of the hero are so incredibly fast. This issue also happened
// in the original interpreter, when the computer was too powerful.
//
// Applies to at least: English, French, German, Italian, Spanish PC floppy
// Responsible method: combatControls::dispatchEvent (script 550) + WarriorObj in heap
// Fixes bug #6247
static const uint16 qfg3SignatureCombatSpeedThrottling1[] = {
	0x31, 0x0d,                         // bnt [skip code]
	SIG_MAGICDWORD,
	0x89, 0xd2,                         // lsg global[D2h]
	0x35, 0x00,                         // ldi 0
	0x1e,                               // gt?
	0x31, 0x06,                         // bnt [skip code]
	0xe1, 0xd2,                         // -ag global[D2h] (jump skips over this)
	0x81, 0x58,                         // lag global[58h]
	0xa3, 0x01,                         // sal local[01]
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling1[] = {
	0x80, 0xd2,                         // lsg global[D2h]
	0x14,                               // or
	0x31, 0x06,                         // bnt [skip code] - saves 4 bytes
	0xe1, 0xd2,                         // -ag global[D2h]
	0x81, 0x58,                         // lag global[58h]
	0xa3, 0x01,                         // sal local[01] (jump skips over this)
	// our code
	0x76,                               // push0
	0x43, 0x2c, 0x00,                   // callk GameIsRestarting <-- add this so that our speed throttler is triggered
	PATCH_END
};

static const uint16 qfg3SignatureCombatSpeedThrottling2[] = {
	SIG_MAGICDWORD,
	SIG_UINT16(12),                     // priority 12
	SIG_UINT16(0),                      // underbits 0
	SIG_UINT16(0x4010),                 // signal 4010h
	SIG_ADDTOOFFSET(+18),
	SIG_UINT16(0),                      // scaleSignal 0
	SIG_UINT16(128),                    // scaleX
	SIG_UINT16(128),                    // scaleY
	SIG_UINT16(128),                    // maxScale
	SIG_UINT16(0),                      // cycleSpeed
	SIG_END
};

static const uint16 qfg3PatchCombatSpeedThrottling2[] = {
	PATCH_ADDTOOFFSET(+32),
	PATCH_UINT16(5),                    // set cycleSpeed to 5
	PATCH_END
};

// In room #750, when the hero enters from the top east path (room #755), it
// could go out of the contained-access polygon bounds, and be able to travel
// freely in the room.
// The reason is that the cutoff y value (42) that determines whether the hero
// enters from the top or bottom path is inaccurate: it's possible to enter the
// top path from as low as y=45.
// This patch changes the cutoff to be 50 which should be low enough.
// It also changes the position in which the hero enters from the top east path
// as the current location is hidden behind the tree.
//
// Applies to: English, French, German, Italian, Spanish and the GOG release.
// Responsible method: enterEast::changeState (script 750)
// Fixes bug #6693
static const uint16 qfg3SignatureRoom750Bounds1[] = {
	// (if (< (ego y?) 42)
	0x76,                               // push0 ("y")
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x04,                         // send 4
	SIG_MAGICDWORD,
	0x36,                               // push
	0x35,   42,                         // ldi 42 <-- comparing ego.y with 42
	0x22,                               // lt?
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds1[] = {
	// (if (< (ego y?) 50)
	PATCH_ADDTOOFFSET(+8),
	50,                                 // 42 --> 50
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds2[] = {
	// (ego x: 294 y: 39)
	0x78,                               // push1 ("x")
	0x78,                               // push1
	0x38, SIG_UINT16(294),              // pushi 294
	0x76,                               // push0 ("y")
	0x78,                               // push1
	SIG_MAGICDWORD,
	0x39,   29,                         // pushi 29
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x0c,                         // send 12
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds2[] = {
	// (ego x: 320 y: 39)
	PATCH_ADDTOOFFSET(+3),
	PATCH_UINT16(320),                  // 294 --> 320
	PATCH_ADDTOOFFSET(+3),
	39,                                 //  29 -->  39
	PATCH_END
};

static const uint16 qfg3SignatureRoom750Bounds3[] = {
	// (ego setMotion: MoveTo 282 29 self)
	0x38, SIG_SELECTOR16(setMotion),    // pushi "setMotion" 0x133 in QfG3
	0x39, 0x04,                         // pushi 4
	0x51, SIG_ADDTOOFFSET(+1),          // class MoveTo
	0x36,                               // push
	0x38, SIG_UINT16(282),              // pushi 282
	SIG_MAGICDWORD,
	0x39,   29,                         // pushi 29
	0x7c,                               // pushSelf
	0x81, 0x00,                         // lag global[0] (ego)
	0x4a, 0x0c,                         // send 12
	SIG_END
};

static const uint16 qfg3PatchRoom750Bounds3[] = {
	// (ego setMotion: MoveTo 309 35 self)
	PATCH_ADDTOOFFSET(+9),
	PATCH_UINT16(309),                  // 282 --> 309
	PATCH_ADDTOOFFSET(+1),
	35,                                 //  29 -->  35
	PATCH_END
};

//          script, description,                                      signature                    patch
static const SciScriptPatcherEntry qfg3Signatures[] = {
	{  true,   944, "import dialog continuous calls",                     1, qfg3SignatureImportDialog,           qfg3PatchImportDialog },
	{  true,   440, "dialog crash when asking about Woo",                 1, qfg3SignatureWooDialog,              qfg3PatchWooDialog },
	{  true,   440, "dialog crash when asking about Woo",                 1, qfg3SignatureWooDialogAlt,           qfg3PatchWooDialogAlt },
	{  true,    52, "export character save bug",                          2, qfg3SignatureExportChar,             qfg3PatchExportChar },
	{  true,    54, "import character from QfG1 bug",                     1, qfg3SignatureImportQfG1Char,         qfg3PatchImportQfG1Char },
	{  true,   640, "chief in hut priority fix",                          1, qfg3SignatureChiefPriority,          qfg3PatchChiefPriority },
	{  true,   285, "missing points for telling about initiation heap",   1, qfg3SignatureMissingPoints1,         qfg3PatchMissingPoints1 },
	{  true,   285, "missing points for telling about initiation script", 1, qfg3SignatureMissingPoints2a,	      qfg3PatchMissingPoints2 },
	{  true,   285, "missing points for telling about initiation script", 1, qfg3SignatureMissingPoints2b,        qfg3PatchMissingPoints2 },
	{  true,   550, "combat speed throttling script",                     1, qfg3SignatureCombatSpeedThrottling1, qfg3PatchCombatSpeedThrottling1 },
	{  true,   550, "combat speed throttling heap",                       1, qfg3SignatureCombatSpeedThrottling2, qfg3PatchCombatSpeedThrottling2 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds1,         qfg3PatchRoom750Bounds1 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds2,         qfg3PatchRoom750Bounds2 },
	{  true,   750, "hero goes out of bounds in room 750",                2, qfg3SignatureRoom750Bounds3,         qfg3PatchRoom750Bounds3 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Quest for Glory 4

// The init code that runs when QFG4 starts up unconditionally resets the
// master music volume to 15, but the game should always use the volume stored
// in ScummVM.
// Applies to at least: English floppy
static const uint16 qfg4SignatureVolumeReset[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x215), // pushi $215 (masterVolume)
	0x78,                    // push1
	0x39, 0x0f,              // pushi $f
	0x81, 0x01,              // lag 1 (Glory object)
	0x4a, SIG_UINT16(0x06),  // send 6
	SIG_END
};

// Same as above, but with a different masterVolume selector.
// Applies to at least: English CD
static const uint16 qfg4CDSignatureVolumeReset[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x217), // pushi $217 (masterVolume)
	0x78,                    // push1
	0x39, 0x0f,              // pushi $f
	0x81, 0x01,              // lag 1 (Glory object)
	0x4a, SIG_UINT16(0x06),  // send 6
	SIG_END
};

static const uint16 qfg4PatchVolumeReset[] = {
	0x32, PATCH_UINT16(8),  // jmp 8 [past volume changes]
	PATCH_END
};

// The trap init code incorrectly creates an int array for string data.
// Applies to at least: English CD
static const uint16 qfg4SignatureTrapArrayType[] = {
	0x38, SIG_UINT16(0x92), // pushi $92 (new)
	0x78,                   // push1
	0x38, SIG_UINT16(0x80), // pushi $80 (128)
	SIG_MAGICDWORD,
	0x51, 0x0b,             // class $b (IntArray)
	0x4a, SIG_UINT16(0x06), // send 6
	SIG_END
};

static const uint16 qfg4PatchTrapArrayType[] = {
	PATCH_ADDTOOFFSET(+4),     // pushi $92 (new), push1
	0x38, PATCH_UINT16(0x100), // pushi $100 (256)
	0x51, 0x0d,                // class $d (ByteArray)
	PATCH_END
};

// QFG4 has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature
static const uint16 qfg4BenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(new), // pushi new
	0x76,                      // push0
	0x51, SIG_ADDTOOFFSET(+1), // class View
	0x4a, SIG_UINT16(0x04),    // send 4
	0xa5, 0x00,                // sat 0
	0x39, 0x0e,                // pushi $e
	SIG_MAGICDWORD,
	0x78,                      // push1
	0x38, SIG_UINT16(0x270f),  // push $270f (9999)
	SIG_END
};

static const uint16 qfg4BenchmarkPatch[] = {
	0x35, 0x01, // ldi 0
	0xa1, 0xbf, // sag $bf (191)
	0x48,       // ret
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry qfg4Signatures[] = {
	{  true,     1, "disable volume reset on startup (floppy)",    1, qfg4SignatureVolumeReset,         qfg4PatchVolumeReset },
	{  true,     1, "disable volume reset on startup (CD)",        1, qfg4CDSignatureVolumeReset,       qfg4PatchVolumeReset },
	{  true,     1, "disable video benchmarking",                  1, qfg4BenchmarkSignature,           qfg4BenchmarkPatch },
	{  true,    83, "fix incorrect array type",                    1, qfg4SignatureTrapArrayType,       qfg4PatchTrapArrayType },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,           sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// ===========================================================================
//  script 298 of sq4/floppy has an issue. object "nest" uses another property
//   which isn't included in property count. We return 0 in that case, the game
//   adds it to nest::x. The problem is that the script also checks if x exceeds
//   we never reach that of course, so the pterodactyl-flight will go endlessly
//   we could either calculate property count differently somehow fixing this
//   but I think just patching it out is cleaner.
// Fixes bug: #5093
static const uint16 sq4FloppySignatureEndlessFlight[] = {
	0x39, 0x04,                         // pushi 04 (selector x)
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x67, 0x08,                         // pTos 08 (property x)
	0x63, SIG_ADDTOOFFSET(+1),          // pToa (invalid property) - 44h for English floppy, 4ch for German floppy
	0x02,                               // add
	SIG_END
};

static const uint16 sq4FloppyPatchEndlessFlight[] = {
	PATCH_ADDTOOFFSET(+5),
	0x35, 0x03,                         // ldi 03 (which would be the content of the property)
	PATCH_END
};

// Floppy-only: When the player tries to throw something at the sequel police in Space Quest X (zero g zone),
//   the game will first show a textbox and then cause a signature mismatch in ScummVM/
//   crash the whole game in Sierra SCI/display garbage (the latter when the Sierra "patch" got applied).
//
// All of this is caused by a typo in the script. Right after the code for showing the textbox,
//  there is more similar code for showing another textbox, but without a pointer to the text.
//  This has to be a typo, because there is no unused text to be found within that script.
//
// Sierra's "patch" didn't include a proper fix (as in a modified script). Instead they shipped a dummy
//  text resource, which somewhat "solved" the issue in Sierra SCI, but it still showed another textbox
//  with garbage in it. Funnily Sierra must have known that, because that new text resource contains:
//  "Hi! This is a kludge!"
//
// We properly fix it by removing the faulty code.
// Applies to at least: English Floppy
// Responsible method: sp1::doVerb
// Fixes bug: found by SCI developer
static const uint16 sq4FloppySignatureThrowStuffAtSequelPoliceBug[] = {
	0x47, 0xff, 0x00, 0x02,             // call export 255_0, 2
	0x3a,                               // toss
	SIG_MAGICDWORD,
	0x36,                               // push
	0x47, 0xff, 0x00, 0x02,             // call export 255_0, 2
	SIG_END
};

static const uint16 sq4FloppyPatchThrowStuffAtSequelPoliceBug[] = {
	PATCH_ADDTOOFFSET(+5),
	0x48,                            // ret
	PATCH_END
};

// Right at the start of Space Quest 4 CD, when walking up in the first room, ego will
//  immediately walk down just after entering the upper room.
//
// This is caused by the scripts setting ego's vertical coordinate to 189 (BDh), which is the
//  trigger in rooms to walk to the room below it. Sometimes this isn't triggered, because
//  the scripts also initiate a motion to vertical coordinate 188 (BCh). When you lower the game's speed,
//  this bug normally always triggers. And it triggers of course also in the original interpreter.
//
// It doesn't happen in PC floppy, because nsRect is not the same as in CD.
//
// We fix it by setting ego's vertical coordinate to 188 and we also initiate a motion to 187.
//
// Applies to at least: English PC CD
// Responsible method: rm045::doit
// Fixes bug: #5468
static const uint16 sq4CdSignatureWalkInFromBelowRoom45[] = {
	0x76,                               // push0
	SIG_MAGICDWORD,
	0x78,                               // push1
	0x38, SIG_UINT16(0x00bd),           // pushi 00BDh
	0x38, SIG_ADDTOOFFSET(+2),          // pushi [setMotion selector]
	0x39, 0x03,                         // pushi 3
	0x51, SIG_ADDTOOFFSET(+1),          // class [MoveTo]
	0x36,                               // push
	0x78,                               // push1
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 04 -> get ego::x
	0x36,                               // push
	0x38, SIG_UINT16(0x00bc),           // pushi 00BCh
	SIG_END
};

static const uint16 sq4CdPatchWalkInFromBelowRoom45[] = {
	PATCH_ADDTOOFFSET(+2),
	0x38, PATCH_UINT16(0x00bc),         // pushi 00BCh
	PATCH_ADDTOOFFSET(+15),
	0x38, PATCH_UINT16(0x00bb),         // pushi 00BBh
	PATCH_END
};

// It seems that Sierra forgot to set a script flag, when cleaning out the bank account
// in Space Quest 4 CD. This was probably caused by the whole bank account interaction
// getting a rewrite and polish in the CD version.
//
// Because of this bug, points for changing back clothes will not get awarded, which
// makes it impossible to get a perfect point score in the CD version of the game.
// The points are awarded by rm371::doit in script 371.
//
// We fix this. Bug also happened, when using the original interpreter.
// Bug does not happen for PC floppy.
//
// Attention: Some Let's Plays on youtube show that points are in fact awarded. Which is true.
//            But those Let's Plays were actually created by playing a hacked Space Quest 4 version
//            (which is part Floppy, part CD version - we consider it to be effectively pirated)
//            and not the actual CD version of Space Quest 4.
//            It's easy to identify - talkie + store called "Radio Shack" -> is hacked version.
//
// Applies to at least: English PC CD
// Responsible method: but2Script::changeState(2)
// Fixes bug: #6866
static const uint16 sq4CdSignatureGetPointsForChangingBackClothes[] = {
	0x35, 0x02,                         // ldi 02
	SIG_MAGICDWORD,
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x006a),           // bnt [state 3]
	0x76,
	SIG_ADDTOOFFSET(+46),               // jump over "withdraw funds" code
	0x33, 0x33,                         // jmp [end of state 2, set cycles code]
	SIG_ADDTOOFFSET(+51),               // jump over "clean bank account" code
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x33, 0x0b,                         // jmp [toss/ret]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x05,                         // bnt [toss/ret]
	SIG_END
};

static const uint16 sq4CdPatchGetPointsForChangingBackClothes[] = {
	PATCH_ADDTOOFFSET(+3),
	0x30, PATCH_UINT16(0x0070),         // bnt [state 3]
	PATCH_ADDTOOFFSET(+47),             // "withdraw funds" code
	0x33, 0x39,                         // jmp [end of state 2, set cycles code]
	PATCH_ADDTOOFFSET(+51),
	0x78,                               // push1
	0x39, 0x1d,                         // ldi 1Dh
	0x45, 0x07, 0x02,                   // call export 7 of script 0 (set flag) -> effectively sets global 73h, bit 2
	0x35, 0x02,                         // ldi 02
	0x65, 0x1c,                         // aTop cycles
	0x33, 0x05,                         // jmp [toss/ret]
	// check for state 3 code removed to save 6 bytes
	PATCH_END
};


// For Space Quest 4 CD, Sierra added a pick up animation for Roger, when he picks up the rope.
//
// When the player is detected by the zombie right at the start of the game, while picking up the rope,
// scripts bomb out. This also happens, when using the original interpreter.
//
// This is caused by code, that's supposed to make Roger face the arriving drone.
// We fix it, by checking if ego::cycler is actually set before calling that code.
//
// Applies to at least: English PC CD
// Responsible method: droidShoots::changeState(3)
// Fixes bug: #6076
static const uint16 sq4CdSignatureGettingShotWhileGettingRope[] = {
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x02fa),           // jmp [end]
	SIG_MAGICDWORD,
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x0b,                         // bnt [state 3 check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // call export 2 of script 0 -> disable controls
	0x35, 0x02,                         // ldi 02
	0x65, 0x1a,                         // aTop cycles
	0x32, SIG_UINT16(0x02e9),           // jmp [end]
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x1e,                         // bnt [state 4 check]
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // call export 2 of script 0 -> disable controls again??
	0x7a,                               // push2
	0x89, 0x00,                         // lsg global[0]
	0x72, SIG_UINT16(0x0242),           // lofsa deathDroid
	0x36,                               // push
	0x45, 0x0d, 0x04,                   // call export 13 of script 0 -> set heading of ego to face droid
	SIG_END
};

static const uint16 sq4CdPatchGettingShotWhileGettingRope[] = {
	PATCH_ADDTOOFFSET(+11),
	// this makes state 2 only do the 2 cycles wait, controls should always be disabled already at this point
	0x2f, 0xf3,                         // bt [previous state aTop cycles code]
	// Now we check for state 3, this change saves us 11 bytes
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 03
	0x1a,                               // eq?
	0x31, 0x29,                         // bnt [state 4 check]
	// new state 3 code
	0x76,                               // push0
	0x45, 0x02, 0x00,                   // call export 2 of script 0 (disable controls, actually not needed)
	0x38, PATCH_SELECTOR16(cycler),     // pushi cycler
	0x76,                               // push0
	0x81, 0x00,                         // lag global[0]
	0x4a, 0x04,                         // send 04 (get ego::cycler)
	0x30, PATCH_UINT16(10),             // bnt [jump over heading call]
	PATCH_END
};

// The scripts in SQ4CD support simultaneous playing of speech and subtitles,
// but this was not available as an option. The following two patches enable
// this functionality in the game's GUI options dialog.
// Patch 1: iconTextSwitch::show, called when the text options button is shown.
// This is patched to add the "Both" text resource (i.e. we end up with
// "Speech", "Text" and "Both")
static const uint16 sq4CdSignatureTextOptionsButton[] = {
	SIG_MAGICDWORD,
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x53,                         // sag 0x53
	0x39, 0x03,                         // pushi 0x03
	0x78,                               // push1
	0x39, 0x09,                         // pushi 0x09
	0x54, 0x06,                         // self 0x06
	SIG_END
};

static const uint16 sq4CdPatchTextOptionsButton[] = {
	PATCH_ADDTOOFFSET(+7),
	0x39, 0x0b,                         // pushi 0x0b
	PATCH_END
};

// Patch 2: Adjust a check in babbleIcon::init, which handles the babble icon
// (e.g. the two guys from Andromeda) shown when dying/quitting.
// Fixes bug: #6068
static const uint16 sq4CdSignatureBabbleIcon[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x02,                         // ldi 02
	0x1a,                               // eq?
	0x31, 0x26,                         // bnt 26  [02a7]
	SIG_END
};

static const uint16 sq4CdPatchBabbleIcon[] = {
	0x89, 0x5a,                         // lsg 5a
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x2f, 0x26,                         // bt 26  [02a7]
	PATCH_END
};

// Patch 3: Add the ability to toggle among the three available options,
// when the text options button is clicked: "Speech", "Text" and "Both".
// Refer to the patch above for additional details.
// iconTextSwitch::doit (called when the text options button is clicked)
static const uint16 sq4CdSignatureTextOptions[] = {
	SIG_MAGICDWORD,
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack)
	0x3c,                               // dup
	0x35, 0x01,                         // ldi 0x01
	0x1a,                               // eq? (global 90 == 1)
	0x31, 0x06,                         // bnt 0x06 (0x0691)
	0x35, 0x02,                         // ldi 0x02
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x33, 0x0a,                         // jmp 0x0a (0x69b)
	0x3c,                               // dup
	0x35, 0x02,                         // ldi 0x02
	0x1a,                               // eq? (global 90 == 2)
	0x31, 0x04,                         // bnt 0x04 (0x069b)
	0x35, 0x01,                         // ldi 0x01
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x3a,                               // toss
	0x38, SIG_SELECTOR16(show),         // pushi 0x00d9
	0x76,                               // push0
	0x54, 0x04,                         // self 0x04
	0x48,                               // ret
	SIG_END
};

static const uint16 sq4CdPatchTextOptions[] = {
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack)
	0x3c,                               // dup
	0x35, 0x03,                         // ldi 0x03 (acc = 3)
	0x1a,                               // eq? (global 90 == 3)
	0x2f, 0x07,                         // bt 0x07
	0x89, 0x5a,                         // lsg 0x5a (load global 90 to stack again)
	0x35, 0x01,                         // ldi 0x01 (acc = 1)
	0x02,                               // add: acc = global 90 (on stack) + 1 (previous acc value)
	0x33, 0x02,                         // jmp 0x02
	0x35, 0x01,                         // ldi 0x01 (reset acc to 1)
	0xa1, 0x5a,                         // sag 0x5a (save acc to global 90)
	0x33, 0x03,                         // jmp 0x03 (jump over the wasted bytes below)
	0x34, PATCH_UINT16(0x0000),         // ldi 0x0000 (waste 3 bytes)
	0x3a,                               // toss
	// (the rest of the code is the same)
	PATCH_END
};

//          script, description,                                      signature                                      patch
static const SciScriptPatcherEntry sq4Signatures[] = {
	{  true,   298, "Floppy: endless flight",                      1, sq4FloppySignatureEndlessFlight,               sq4FloppyPatchEndlessFlight },
	{  true,   700, "Floppy: throw stuff at sequel police bug",    1, sq4FloppySignatureThrowStuffAtSequelPoliceBug, sq4FloppyPatchThrowStuffAtSequelPoliceBug },
	{  true,    45, "CD: walk in from below for room 45 fix",      1, sq4CdSignatureWalkInFromBelowRoom45,           sq4CdPatchWalkInFromBelowRoom45 },
	{  true,   396, "CD: get points for changing back clothes fix",1, sq4CdSignatureGetPointsForChangingBackClothes, sq4CdPatchGetPointsForChangingBackClothes },
	{  true,   701, "CD: getting shot, while getting rope",        1, sq4CdSignatureGettingShotWhileGettingRope,     sq4CdPatchGettingShotWhileGettingRope },
	{  true,     0, "CD: Babble icon speech and subtitles fix",    1, sq4CdSignatureBabbleIcon,                      sq4CdPatchBabbleIcon },
	{  true,   818, "CD: Speech and subtitles option",             1, sq4CdSignatureTextOptions,                     sq4CdPatchTextOptions },
	{  true,   818, "CD: Speech and subtitles option button",      1, sq4CdSignatureTextOptionsButton,               sq4CdPatchTextOptionsButton },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// When you leave Ulence Flats, another timepod is supposed to appear.
// On fast machines, that timepod appears fully immediately and then
//  starts to appear like it should be. That first appearance is caused
//  by the scripts setting an invalid cel number and the machine being
//  so fast that there is no time for another script to actually fix
//  the cel number. On slower machines, the cel number gets fixed
//  by the cycler and that's why only fast machines are affected.
//  The same issue happens in Sierra SCI.
// We simply set the correct starting cel number to fix the bug.
// Responsible method: robotIntoShip::changeState(9)
static const uint16 sq1vgaSignatureUlenceFlatsTimepodGfxGlitch[] = {
	0x39,
	SIG_MAGICDWORD, SIG_SELECTOR8(cel), // pushi "cel"
	0x78,                               // push1
	0x39, 0x0a,                         // pushi 0x0a (set ship::cel to 10)
	0x38, SIG_UINT16(0x00a0),           // pushi 0x00a0 (ship::setLoop)
	SIG_END
};

static const uint16 sq1vgaPatchUlenceFlatsTimepodGfxGlitch[] = {
	PATCH_ADDTOOFFSET(+3),
	0x39, 0x09,                         // pushi 0x09 (set ship::cel to 9)
	PATCH_END
};

// In Ulence Flats, there is a space ship, that you will use at some point.
//  Near that space ship are 2 force field generators.
//  When you look at the top of those generators, the game will crash.
//  This happens also in Sierra SCI. It's caused by a jump, that goes out of bounds.
//  We currently do not know if this was caused by a compiler glitch or if it was a developer error.
//  Anyway we patch this glitchy code, so that the game won't crash anymore.
//
// Applies to at least: English Floppy
// Responsible method: radar1::doVerb
// Fixes bug: #6816
static const uint16 sq1vgaSignatureUlenceFlatsGeneratorGlitch[] = {
	SIG_MAGICDWORD, 0x1a,               // eq?
	0x30, SIG_UINT16(0xcdf4),           // bnt absolute 0xf000
	SIG_END
};

static const uint16 sq1vgaPatchUlenceFlatsGeneratorGlitch[] = {
	PATCH_ADDTOOFFSET(+1),
	0x32, PATCH_UINT16(0x0000),         // jmp 0x0000 (waste bytes)
	PATCH_END
};

// No documentation for this patch (TODO)
static const uint16 sq1vgaSignatureEgoShowsCard[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(timesShownID), // push "timesShownID"
	0x78,                               // push1
	0x38, SIG_SELECTOR16(timesShownID), // push "timesShownID"
	0x76,                               // push0
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x04,                         // send 0x04 (get timesShownID)
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x02,                               // add
	0x36,                               // push
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x06,                         // send 0x06 (set timesShownID)
	0x36,                               // push      (wrong, acc clobbered by class, above)
	0x35, 0x03,                         // ldi 0x03
	0x22,                               // lt?
	SIG_END
};

// Note that this script patch is merely a reordering of the
// instructions in the original script.
static const uint16 sq1vgaPatchEgoShowsCard[] = {
	0x38, PATCH_SELECTOR16(timesShownID), // push "timesShownID"
	0x76,                               // push0
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x04,                         // send 0x04 (get timesShownID)
	0x36,                               // push
	0x35, 0x01,                         // ldi 1
	0x02,                               // add
	0x36,                               // push (this push corresponds to the wrong one above)
	0x38, PATCH_SELECTOR16(timesShownID), // push "timesShownID"
	0x78,                               // push1
	0x36,                               // push
	0x51, 0x7c,                         // class DeltaurRegion
	0x4a, 0x06,                         // send 0x06 (set timesShownID)
	0x35, 0x03,                         // ldi 0x03
	0x22,                               // lt?
	PATCH_END
};

// The spider droid on planet Korona has a fixed movement speed,
//  which is way faster than the default movement speed of ego.
// This means that the player would have to turn up movement speed,
//  otherwise it will be impossible to escape it.
// We fix this issue by making the droid move a bit slower than ego
//  does (relative to movement speed setting).
//
// Applies to at least: English PC floppy
// Responsible method: spider::doit
static const uint16 sq1vgaSignatureSpiderDroidTiming[] = {
	SIG_MAGICDWORD,
	0x63, 0x4e,                         // pToa script
	0x30, SIG_UINT16(0x0005),           // bnt [further method code]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_UINT16(0x0062),           // jmp [super-call]
	0x38, SIG_UINT16(0x0088),           // pushi 0088h (script)
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] (current room)
	0x4a, 0x04,                         // send 04 (get [current room].script)
	0x30, SIG_UINT16(0x0005),           // bnt [further method code]
	0x35, 0x00,                         // ldi 00
	0x32, SIG_UINT16(0x0052),           // jmp [super-call]
	0x89, 0xa6,                         // lsg global[a6] <-- flag gets set to 1 when ego went up the skeleton tail, when going down it's set to 2
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x30, SIG_UINT16(0x0012),           // bnt [PChase set code], in case global A6 <> 1
	0x81, 0xb5,                         // lag global[b5]
	0x30, SIG_UINT16(0x000d),           // bnt [PChase set code], in case global B5 == 0
	0x38, SIG_UINT16(0x008c),           // pushi 008c
	0x78,                               // push1
	0x72, SIG_UINT16(0x1cb6),           // lofsa 1CB6 (moveToPath)
	0x36,                               // push
	0x54, 0x06,                         // self 06
	0x32, SIG_UINT16(0x0038),           // jmp [super-call]
	// PChase set call
	0x81, 0xb5,                         // lag global[B5]
	0x18,                               // not
	0x30, SIG_UINT16(0x0032),           // bnt [super-call], in case global B5 <> 0
	// followed by:
	// is spider in current room
	// is global A6h == 2? -> set PChase
	SIG_END
}; // 58 bytes)

// Global A6h <> 1 (did NOT went up the skeleton)
//  Global B5h = 0 -> set PChase
//  Global B5h <> 0 -> do not do anything
// Global A6h = 1 (did went up the skeleton)
//  Global B5h = 0 -> set PChase
//  Global B5h <> 0 -> set moveToPath

static const uint16 sq1vgaPatchSpiderDroidTiming[] = {
	0x63, 0x4e,                         // pToa script
	0x2f, 0x68,                         // bt [super-call]
	0x38, PATCH_UINT16(0x0088),         // pushi 0088 (script)
	0x76,                               // push0
	0x81, 0x02,                         // lag global[2] (current room)
	0x4a, 0x04,                         // send 04
	0x2f, 0x5e,                         // bt [super-call]
	// --> 12 bytes saved
	// new code
	0x38, PATCH_UINT16(0x0176),         // pushi 0176 (egoMoveSpeed)
	0x76,                               // push0
	0x81, 0x01,                         // lag global[1]
	0x4a, 0x04,                         // send 04 - sq1::egoMoveSpeed
	0x36,                               // push
	0x36,                               // push
	0x35, 0x03,                         // ldi 03
	0x0c,                               // shr
	0x02,                               // add --> egoMoveSpeed + (egoMoveSpeed >> 3)
	0x39, 0x01,                         // push 01 (waste 1 byte)
	0x02,                               // add --> egoMoveSpeed++
	0x65, 0x4c,                         // aTop cycleSpeed
	0x65, 0x5e,                         // aTop moveSpeed
	// new code end
	0x81, 0xb5,                         // lag global[B5]
	0x31, 0x13,                         // bnt [PChase code chunk]
	0x89, 0xa6,                         // lsg global[A6]
	0x35, 0x01,                         // ldi 01
	0x1a,                               // eq?
	0x31, 0x3e,                         // bnt [super-call]
	0x38, PATCH_UINT16(0x008c),         // pushi 008c
	0x78,                               // push1
	0x72, PATCH_UINT16(0x1cb6),         // lofsa moveToPath
	0x36,                               // push
	0x54, 0x06,                         // self 06 - spider::setScript(movePath)
	0x33, 0x32,                         // jmp [super-call]
	// --> 9 bytes saved
	PATCH_END
};

//          script, description,                                      signature                                   patch
static const SciScriptPatcherEntry sq1vgaSignatures[] = {
	{  true,    45, "Ulence Flats: timepod graphic glitch",        1, sq1vgaSignatureUlenceFlatsTimepodGfxGlitch, sq1vgaPatchUlenceFlatsTimepodGfxGlitch },
	{  true,    45, "Ulence Flats: force field generator glitch",  1, sq1vgaSignatureUlenceFlatsGeneratorGlitch,  sq1vgaPatchUlenceFlatsGeneratorGlitch },
	{  true,    58, "Sarien armory droid zapping ego first time",  1, sq1vgaSignatureEgoShowsCard,                sq1vgaPatchEgoShowsCard },
	{  true,   704, "spider droid timing issue",                   1, sq1vgaSignatureSpiderDroidTiming,           sq1vgaPatchSpiderDroidTiming },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// The toolbox in sq5 is buggy. When you click on the upper part of the "put
//  in inventory"-button (some items only - for example the hole puncher - at the
//  upper left), points will get awarded correctly and the item will get put into
//  the player's inventory, but you will then get a "not here" message and the
//  item will also remain to be the current mouse cursor.
// The bug report also says that items may get lost. I wasn't able to reproduce
//  that part.
// This is caused by the mouse-click event getting reprocessed (which wouldn't
//  be a problem by itself) and during this reprocessing coordinates are not
//  processed the same as during the first click (script 226 includes a local
//  subroutine, which checks coordinates in a hardcoded way w/o port-adjustment).
// Because of this, the hotspot for the button is lower than it should be, which
//  then results in the game thinking that the user didn't click on the button
//  and also results in the previously mentioned message.
// This happened in Sierra SCI as well (of course).
// We fix it by combining state 0 + 1 of takeTool::changeState and so stopping
//  the event to get reprocessed. This was the only way possible, because everything
//  else is done in SCI system scripts and I don't want to touch those.
// Applies to at least: English/German/French PC floppy
// Responsible method: takeTool::changeState
// Fixes bug: #6457
static const uint16 sq5SignatureToolboxFix[] = {
	0x31, 0x13,                    // bnt [check for state 1]
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x00aa),      // pushi 00aa
	0x39, 0x05,                    // pushi 05
	0x39, 0x16,                    // pushi 16
	0x76,                          // push0
	0x39, 0x03,                    // pushi 03
	0x76,                          // push0
	0x7c,                          // pushSelf
	0x81, 0x5b,                    // lag 5b
	0x4a, 0x0e,                    // send 0e
	0x32, SIG_UINT16(0x0088),      // jmp [end-of-method]
	0x3c,                          // dup
	0x35, 0x01,                    // ldi 01
	0x1a,                          // eq?
	0x31, 0x28,                    // bnt [check for state 2]
	SIG_END
};

static const uint16 sq5PatchToolboxFix[] = {
	0x31, 0x41,                    // bnt [check for state 2]
	PATCH_ADDTOOFFSET(+16),        // skip to jmp offset
	0x35, 0x01,                    // ldi 01
	0x65, 0x14,                    // aTop [state]
	0x36, 0x00, 0x00,              // ldi 0000 (waste 3 bytes)
	0x35, 0x00,                    // ldi 00 (waste 2 bytes)
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry sq5Signatures[] = {
	{  true,   226, "toolbox fix",                                 1, sq5SignatureToolboxFix,          sq5PatchToolboxFix },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark RAMA

// RAMA has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature
static const uint16 ramaBenchmarkSignature[] = {
	0x38, SIG_SELECTOR16(view), // pushi view
	SIG_MAGICDWORD,
	0x78,                       // push1
	0x38, SIG_UINT16(0xfdd4),   // pushi 64980
	SIG_END
};

static const uint16 ramaBenchmarkPatch[] = {
	0x34, PATCH_UINT16(10000), // ldi 10000
	0x48,                      // ret
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry ramaSignatures[] = {
	{  true, 64908, "disable video benchmarking",                  1, ramaBenchmarkSignature,          ramaBenchmarkPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Shivers

// In room 35170, there is a CCTV control station with a joystick that must be
// clicked and dragged to pan the camera. In order to enable dragging, on
// mousedown, the vJoystick::handleEvent method calls vJoystick::doVerb(1),
// which enables the drag functionality of the joystick. However,
// vJoystick::handleEvent then makes a super call to ShiversProp::handleEvent,
// which calls vJoystick::doVerb(). This second call, which fails to pass an
// argument, causes an uninitialized read off the stack for the first parameter.
// In SSCI, this happens to work because the uninitialized value on the stack
// happens to be 1. Disabling the super call avoids the bad doVerb call without
// any apparent ill effect.
// The same problem exists when trying to drag the volume & brightness sliders
// in the main menu. These controls are also fixed by this patch.
static const uint16 shiversSignatureSuperCall[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0xa5),    // pushi handleEvent
	0x78,                      // push1
	0x8f, 0x01,                // lsp 1
	0x59, 0x02,                // &rest 2
	0x57, 0x7f, SIG_UINT16(6), // super ShiversProp[7f], 6
	SIG_END
};

static const uint16 shiversPatchSuperCall[] = {
	0x48,                      // ret
	PATCH_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry shiversSignatures[] = {
	{  true, 35170, "fix CCTV joystick interaction",               1, shiversSignatureSuperCall,     shiversPatchSuperCall },
	{  true,   990, "fix volume & brightness sliders",             2, shiversSignatureSuperCall,     shiversPatchSuperCall },
	{  true, 64908, "disable video benchmarking",                  1, sci2BenchmarkSignature,        sci2BenchmarkPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Space Quest 6

// After the explosion in the Quarters of Deepship 86, the game tries to perform
// a dramatic long fade, but does this with an unreasonably large number of
// divisions which takes tens of seconds to finish (because transitions are not
// CPU-dependent in ScummVM).
static const uint16 sq6SlowTransitionSignature1[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x578), // pushi $0578
	0x51, 0x33,              // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch1[] = {
	0x38, SIG_UINT16(500), // pushi 500
	PATCH_END
};

// For whatever reason, SQ6 sets the default number of transition divisions to
// be a much larger value at startup (200 vs 30) if it thinks it is running in
// Windows. Room 410 (eulogy room) also unconditionally resets divisions to the
// larger value.
static const uint16 sq6SlowTransitionSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0xc8), // pushi $c8
	0x51, 0x33,             // class Styler
	SIG_END
};

static const uint16 sq6SlowTransitionPatch2[] = {
	0x38, SIG_UINT16(30), // pushi 30
	PATCH_END
};

// SQ6 has custom video benchmarking code that needs to be disabled; see
// sci2BenchmarkSignature. (The sci2BenchmarkPatch is suitable for use with
// SQ6 as well.)
static const uint16 sq6BenchmarkSignature[] = {
	SIG_MAGICDWORD,
	0x38, SIG_SELECTOR16(init),  // pushi init
	0x76,                        // push0
	0x7e, SIG_ADDTOOFFSET(+2),   // line
	0x38, SIG_SELECTOR16(posn),  // pushi $140 (posn)
	SIG_END
};

//          script, description,                                      signature                        patch
static const SciScriptPatcherEntry sq6Signatures[] = {
	{  true,     0, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,    15, "invalid array construction",                  1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,    22, "invalid array construction",                  1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   410, "fix slow transitions",                        1, sq6SlowTransitionSignature2,     sq6SlowTransitionPatch2 },
	{  true,   460, "invalid array construction",                  1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true,   500, "fix slow transitions",                        1, sq6SlowTransitionSignature1,     sq6SlowTransitionPatch1 },
	{  true,   510, "invalid array construction",                  1, sci21IntArraySignature,          sci21IntArrayPatch },
	{  true, 64908, "disable video benchmarking",                  1, sq6BenchmarkSignature,           sci2BenchmarkPatch },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,          sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,          sci2NumSavesPatch2 },
	{  true, 64990, "disable change directory button",             1, sci2ChangeDirSignature,          sci2ChangeDirPatch },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#pragma mark -
#pragma mark Torins Passage

// The init code that runs when Torin starts up unconditionally resets the
// master music volume to defaults, but the game should always use the volume
// stored in ScummVM.
// Applies to at least: English CD
static const uint16 torinVolumeResetSignature1[] = {
	SIG_MAGICDWORD,
	0x35, 0x28, // ldi $28
	0xa1, 0xe3, // sag $e3 (music volume)
	0x35, 0x3c, // ldi $3c
	0xa1, 0xe4, // sag $e4 (sfx volume)
	0x35, 0x64, // ldi $64
	0xa1, 0xe5, // sag $e5 (speech volume)
	SIG_END
};

static const uint16 torinVolumeResetPatch1[] = {
	0x33, 0x0a, // jmp [past volume resets]
	PATCH_END
};

// The init code that runs when Torin starts up unconditionally resets the
// master music volume to values stored in torin.prf, but the game should always
// use the volume stored in ScummVM.
// Applies to at least: English CD
static const uint16 torinVolumeResetSignature2[] = {
	SIG_MAGICDWORD,
	0x38, SIG_UINT16(0x20b), // pushi $020b
	0x76,                    // push0
	SIG_ADDTOOFFSET(6),      // advance file stream
	0xa1, 0xe3,              // sag $e3 (music volume)
	SIG_ADDTOOFFSET(10),     // advance file stream
	0xa1, 0xe4,              // sag $e4 (sfx volume)
	SIG_ADDTOOFFSET(10),     // advance file stream
	0xa1, 0xe5,              // sag $e5 (speech volume)
	SIG_END
};

static const uint16 torinVolumeResetPatch2[] = {
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_ADDTOOFFSET(10), // advance file stream
	0x18, 0x18,            // waste bytes
	PATCH_END
};

//          script, description,                                      signature                         patch
static const SciScriptPatcherEntry torinSignatures[] = {
	{  true, 64000, "disable volume reset on startup 1/2",         1, torinVolumeResetSignature1,        torinVolumeResetPatch1 },
	{  true, 64000, "disable volume reset on startup 2/2",         1, torinVolumeResetSignature2,        torinVolumeResetPatch2 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature1,           sci2NumSavesPatch1 },
	{  true, 64990, "increase number of save games",               1, sci2NumSavesSignature2,           sci2NumSavesPatch2 },
	SCI_SIGNATUREENTRY_TERMINATOR
};

#endif

// =================================================================================

ScriptPatcher::ScriptPatcher() {
	int selectorCount = ARRAYSIZE(selectorNameTable);
	int selectorNr;

	// Allocate table for selector-IDs and initialize that table as well
	_selectorIdTable = new Selector[ selectorCount ];
	for (selectorNr = 0; selectorNr < selectorCount; selectorNr++)
		_selectorIdTable[selectorNr] = -1;

	_runtimeTable = NULL;
	_isMacSci11 = false;
}

ScriptPatcher::~ScriptPatcher() {
	delete[] _runtimeTable;
	delete[] _selectorIdTable;
}

// will actually patch previously found signature area
void ScriptPatcher::applyPatch(const SciScriptPatcherEntry *patchEntry, SciSpan<byte> scriptData, int32 signatureOffset) {
	const uint16 *patchData = patchEntry->patchData;
	byte orgData[PATCH_VALUELIMIT];
	int32 offset = signatureOffset;
	uint16 patchWord = *patchEntry->patchData;
	uint16 patchSelector = 0;

	// Copy over original bytes from script
	uint32 orgDataSize = scriptData.size() - offset;
	if (orgDataSize > PATCH_VALUELIMIT)
		orgDataSize = PATCH_VALUELIMIT;
	scriptData.subspan(offset, orgDataSize).unsafeCopyDataTo(orgData);

	while (patchWord != PATCH_END) {
		uint16 patchCommand = patchWord & PATCH_COMMANDMASK;
		uint16 patchValue = patchWord & PATCH_VALUEMASK;
		switch (patchCommand) {
		case PATCH_CODE_ADDTOOFFSET: {
			// add value to offset
			offset += patchValue;
			break;
		}
		case PATCH_CODE_GETORIGINALBYTE: {
			// get original byte from script and adjust it
			if (patchValue >= orgDataSize)
				error("Script-Patcher: can not get requested original byte from script");
			byte orgByte = orgData[patchValue];
			int16 adjustValue;
			patchData++; adjustValue = (int16)(*patchData);
			scriptData[offset] = orgByte + adjustValue;
			offset++;
			break;
		}
		case PATCH_CODE_GETORIGINALUINT16: {
			// get original byte from script and adjust it
			if ((patchValue >= orgDataSize) || (((uint32)patchValue + 1) >= orgDataSize))
				error("Script-Patcher: can not get requested original uint16 from script");
			uint16 orgUINT16;
			int16 adjustValue;

			if (!_isMacSci11) {
				orgUINT16 = orgData[patchValue] | (orgData[patchValue + 1] << 8);
			} else {
				orgUINT16 = orgData[patchValue + 1] | (orgData[patchValue] << 8);
			}
			patchData++; adjustValue = (int16)(*patchData);
			orgUINT16 += adjustValue;
			if (!_isMacSci11) {
				scriptData[offset] = orgUINT16 & 0xFF;
				scriptData[offset + 1] = orgUINT16 >> 8;
			} else {
				scriptData[offset] = orgUINT16 >> 8;
				scriptData[offset + 1] = orgUINT16 & 0xFF;
			}
			offset += 2;
			break;
		}
		case PATCH_CODE_UINT16:
		case PATCH_CODE_SELECTOR16: {
			byte byte1;
			byte byte2;

			switch (patchCommand) {
			case PATCH_CODE_UINT16: {
				byte1 = patchValue & PATCH_BYTEMASK;
				patchData++; patchWord = *patchData;
				if (patchWord & PATCH_COMMANDMASK)
					error("Script-Patcher: Patch inconsistent");
				byte2 = patchWord & PATCH_BYTEMASK;
				break;
			}
			case PATCH_CODE_SELECTOR16: {
				patchSelector = _selectorIdTable[patchValue];
				byte1 = patchSelector & 0xFF;
				byte2 = patchSelector >> 8;
				break;
			}
			default:
				byte1 = 0; byte2 = 0;
			}
			if (!_isMacSci11) {
				scriptData[offset++] = byte1;
				scriptData[offset++] = byte2;
			} else {
				// SCI1.1+ on macintosh had uint16s in script in BE-order
				scriptData[offset++] = byte2;
				scriptData[offset++] = byte1;
			}
			break;
		}
		case PATCH_CODE_SELECTOR8: {
			patchSelector = _selectorIdTable[patchValue];
			if (patchSelector & 0xFF00)
				error("Script-Patcher: 8 bit selector required, game uses 16 bit selector");
			scriptData[offset] = patchSelector & 0xFF;
			offset++;
			break;
		}
		case PATCH_CODE_BYTE:
			scriptData[offset] = patchValue & PATCH_BYTEMASK;
			offset++;
		}
		patchData++;
		patchWord = *patchData;
	}
}

bool ScriptPatcher::verifySignature(uint32 byteOffset, const uint16 *signatureData, const char *signatureDescription, const SciSpan<const byte> &scriptData) {
	uint16 sigSelector = 0;

	uint16 sigWord = *signatureData;
	while (sigWord != SIG_END) {
		uint16 sigCommand = sigWord & SIG_COMMANDMASK;
		uint16 sigValue = sigWord & SIG_VALUEMASK;
		switch (sigCommand) {
		case SIG_CODE_ADDTOOFFSET: {
			// add value to offset
			byteOffset += sigValue;
			break;
		}
		case SIG_CODE_UINT16:
		case SIG_CODE_SELECTOR16: {
			if (byteOffset + 1 < scriptData.size()) {
				byte byte1;
				byte byte2;

				switch (sigCommand) {
				case SIG_CODE_UINT16: {
					byte1 = sigValue & SIG_BYTEMASK;
					signatureData++; sigWord = *signatureData;
					if (sigWord & SIG_COMMANDMASK)
						error("Script-Patcher: signature inconsistent\nFaulty signature: '%s'", signatureDescription);
					byte2 = sigWord & SIG_BYTEMASK;
					break;
				}
				case SIG_CODE_SELECTOR16: {
					sigSelector = _selectorIdTable[sigValue];
					byte1 = sigSelector & 0xFF;
					byte2 = sigSelector >> 8;
					break;
				}
				default:
					byte1 = 0; byte2 = 0;
				}
				if (!_isMacSci11) {
					if ((scriptData[byteOffset] != byte1) || (scriptData[byteOffset + 1] != byte2))
						sigWord = SIG_MISMATCH;
				} else {
					// SCI1.1+ on macintosh had uint16s in script in BE-order
					if ((scriptData[byteOffset] != byte2) || (scriptData[byteOffset + 1] != byte1))
						sigWord = SIG_MISMATCH;
				}
				byteOffset += 2;
			} else {
				sigWord = SIG_MISMATCH;
			}
			break;
		}
		case SIG_CODE_SELECTOR8: {
			if (byteOffset < scriptData.size()) {
				sigSelector = _selectorIdTable[sigValue];
				if (sigSelector & 0xFF00)
					error("Script-Patcher: 8 bit selector required, game uses 16 bit selector\nFaulty signature: '%s'", signatureDescription);
				if (scriptData[byteOffset] != (sigSelector & 0xFF))
					sigWord = SIG_MISMATCH;
				byteOffset++;
			} else {
				sigWord = SIG_MISMATCH; // out of bounds
			}
			break;
		}
		case SIG_CODE_BYTE:
			if (byteOffset < scriptData.size()) {
				if (scriptData[byteOffset] != sigWord)
					sigWord = SIG_MISMATCH;
				byteOffset++;
			} else {
				sigWord = SIG_MISMATCH; // out of bounds
			}
		}

		if (sigWord == SIG_MISMATCH)
			break;

		signatureData++;
		sigWord = *signatureData;
	}

	if (sigWord == SIG_END) // signature fully matched?
		return true;
	return false;
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 ScriptPatcher::findSignature(uint32 magicDWord, int magicOffset, const uint16 *signatureData, const char *patchDescription, const SciSpan<const byte> &scriptData) {
	if (scriptData.size() < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	// magicDWord is in platform-specific BE/LE form, so that the later match will work, this was done for performance
	const uint32 searchLimit = scriptData.size() - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == scriptData.getUint32At(DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + magicOffset;

			if (verifySignature(offset, signatureData, patchDescription, scriptData))
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

int32 ScriptPatcher::findSignature(const SciScriptPatcherEntry *patchEntry, const SciScriptPatcherRuntimeEntry *runtimeEntry, const SciSpan<const byte> &scriptData) {
	return findSignature(runtimeEntry->magicDWord, runtimeEntry->magicOffset, patchEntry->signatureData, patchEntry->description, scriptData);
}

// Attention: Magic DWord is returned using platform specific byte order. This is done on purpose for performance.
void ScriptPatcher::calculateMagicDWordAndVerify(const char *signatureDescription, const uint16 *signatureData, bool magicDWordIncluded, uint32 &calculatedMagicDWord, int &calculatedMagicDWordOffset) {
	Selector curSelector = -1;
	int magicOffset;
	byte magicDWord[4];
	int magicDWordLeft = 0;
	uint16 curWord;
	uint16 curCommand;
	uint32 curValue;
	byte byte1 = 0;
	byte byte2 = 0;

	memset(magicDWord, 0, sizeof(magicDWord));

	curWord = *signatureData;
	magicOffset = 0;
	while (curWord != SIG_END) {
		curCommand = curWord & SIG_COMMANDMASK;
		curValue   = curWord & SIG_VALUEMASK;
		switch (curCommand) {
		case SIG_MAGICDWORD: {
			if (magicDWordIncluded) {
				if ((calculatedMagicDWord) || (magicDWordLeft))
					error("Script-Patcher: Magic-DWORD specified multiple times in signature\nFaulty patch: '%s'", signatureDescription);
				magicDWordLeft = 4;
				calculatedMagicDWordOffset = magicOffset;
			} else {
				error("Script-Patcher: Magic-DWORD sequence found in patch data\nFaulty patch: '%s'", signatureDescription);
			}
			break;
		}
		case SIG_CODE_ADDTOOFFSET: {
			magicOffset -= curValue;
			if (magicDWordLeft)
				error("Script-Patcher: Magic-DWORD contains AddToOffset command\nFaulty patch: '%s'", signatureDescription);
			break;
		}
		case SIG_CODE_UINT16:
		case SIG_CODE_SELECTOR16: {
			// UINT16 or 1
			switch (curCommand) {
			case SIG_CODE_UINT16: {
				signatureData++; curWord = *signatureData;
				if (curWord & SIG_COMMANDMASK)
					error("Script-Patcher: signature entry inconsistent\nFaulty patch: '%s'", signatureDescription);
				if (!_isMacSci11) {
					byte1 = curValue;
					byte2 = curWord & SIG_BYTEMASK;
				} else {
					byte1 = curWord & SIG_BYTEMASK;
					byte2 = curValue;
				}
				break;
			}
			case SIG_CODE_SELECTOR16: {
				curSelector = _selectorIdTable[curValue];
				if (curSelector == -1) {
					curSelector = g_sci->getKernel()->findSelector(selectorNameTable[curValue]);
					_selectorIdTable[curValue] = curSelector;
				}
				if (!_isMacSci11) {
					byte1 = curSelector & 0x00FF;
					byte2 = curSelector >> 8;
				} else {
					byte1 = curSelector >> 8;
					byte2 = curSelector & 0x00FF;
				}
				break;
			}
			}
			magicOffset -= 2;
			if (magicDWordLeft) {
				// Remember current word for Magic DWORD
				magicDWord[4 - magicDWordLeft] = byte1;
				magicDWordLeft--;
				if (magicDWordLeft) {
					magicDWord[4 - magicDWordLeft] = byte2;
					magicDWordLeft--;
				}
				if (!magicDWordLeft) {
					// Magic DWORD is now known, convert to platform specific byte order
					calculatedMagicDWord = READ_UINT32(magicDWord);
				}
			}
			break;
		}
		case SIG_CODE_BYTE:
		case SIG_CODE_SELECTOR8: {
			if (curCommand == SIG_CODE_SELECTOR8) {
				curSelector = _selectorIdTable[curValue];
				if (curSelector == -1) {
					curSelector = g_sci->getKernel()->findSelector(selectorNameTable[curValue]);
					_selectorIdTable[curValue] = curSelector;
					if (curSelector != -1) {
						if (curSelector & 0xFF00)
							error("Script-Patcher: 8 bit selector required, game uses 16 bit selector\nFaulty patch: '%s'", signatureDescription);
					}
				}
				curValue = curSelector;
			}
			magicOffset--;
			if (magicDWordLeft) {
				// Remember current byte for Magic DWORD
				magicDWord[4 - magicDWordLeft] = (byte)curValue;
				magicDWordLeft--;
				if (!magicDWordLeft) {
					// Magic DWORD is now known, convert to platform specific byte order
					calculatedMagicDWord = READ_UINT32(magicDWord);
				}
			}
			break;
		}
		case PATCH_CODE_GETORIGINALBYTE:
		case PATCH_CODE_GETORIGINALUINT16: {
			signatureData++; // skip over extra uint16
			break;
		}
		default:
			break;
		}
		signatureData++;
		curWord = *signatureData;
	}

	if (magicDWordLeft)
		error("Script-Patcher: Magic-DWORD beyond End-Of-Signature\nFaulty patch: '%s'", signatureDescription);
	if (magicDWordIncluded) {
		if (!calculatedMagicDWord) {
			error("Script-Patcher: Magic-DWORD not specified in signature\nFaulty patch: '%s'", signatureDescription);
		}
	}
}

// This method calculates the magic DWORD for each entry in the signature table
//  and it also initializes the selector table for selectors used in the signatures/patches of the current game
void ScriptPatcher::initSignature(const SciScriptPatcherEntry *patchTable) {
	const SciScriptPatcherEntry *curEntry = patchTable;
	SciScriptPatcherRuntimeEntry *curRuntimeEntry;
	int patchEntryCount = 0;

	// Count entries and allocate runtime data
	while (curEntry->signatureData) {
		patchEntryCount++; curEntry++;
	}
	_runtimeTable = new SciScriptPatcherRuntimeEntry[patchEntryCount];
	memset(_runtimeTable, 0, sizeof(SciScriptPatcherRuntimeEntry) * patchEntryCount);

	curEntry = patchTable;
	curRuntimeEntry = _runtimeTable;
	while (curEntry->signatureData) {
		// process signature
		curRuntimeEntry->active = curEntry->defaultActive;
		curRuntimeEntry->magicDWord = 0;
		curRuntimeEntry->magicOffset = 0;

		// We verify the signature data and remember the calculated magic DWord from the signature data
		calculateMagicDWordAndVerify(curEntry->description, curEntry->signatureData, true, curRuntimeEntry->magicDWord, curRuntimeEntry->magicOffset);
		// We verify the patch data
		calculateMagicDWordAndVerify(curEntry->description, curEntry->patchData, false, curRuntimeEntry->magicDWord, curRuntimeEntry->magicOffset);

		curEntry++; curRuntimeEntry++;
	}
}

// This method enables certain patches
//  It's used for patches, which are not meant to get applied all the time
void ScriptPatcher::enablePatch(const SciScriptPatcherEntry *patchTable, const char *searchDescription) {
	const SciScriptPatcherEntry *curEntry = patchTable;
	SciScriptPatcherRuntimeEntry *runtimeEntry = _runtimeTable;
	int searchDescriptionLen = strlen(searchDescription);
	int matchCount = 0;

	while (curEntry->signatureData) {
		if (strncmp(curEntry->description, searchDescription, searchDescriptionLen) == 0) {
			// match found, enable patch
			runtimeEntry->active = true;
			matchCount++;
		}
		curEntry++; runtimeEntry++;
	}

	if (!matchCount)
		error("Script-Patcher: no patch found to enable");
}

void ScriptPatcher::processScript(uint16 scriptNr, SciSpan<byte> scriptData) {
	const SciScriptPatcherEntry *signatureTable = NULL;
	const SciScriptPatcherEntry *curEntry = NULL;
	SciScriptPatcherRuntimeEntry *curRuntimeEntry = NULL;
	const Sci::SciGameId gameId = g_sci->getGameId();

	switch (gameId) {
	case GID_CAMELOT:
		signatureTable = camelotSignatures;
		break;
	case GID_ECOQUEST:
		signatureTable = ecoquest1Signatures;
		break;
	case GID_ECOQUEST2:
		signatureTable = ecoquest2Signatures;
		break;
	case GID_FANMADE:
		signatureTable = fanmadeSignatures;
		break;
	case GID_FREDDYPHARKAS:
		signatureTable = freddypharkasSignatures;
		break;
#ifdef ENABLE_SCI32
	case GID_HOYLE5:
		signatureTable = hoyle5Signatures;
		break;
	case GID_GK1:
		signatureTable = gk1Signatures;
		break;
	case GID_GK2:
		signatureTable = gk2Signatures;
		break;
#endif
	case GID_KQ5:
		signatureTable = kq5Signatures;
		break;
	case GID_KQ6:
		signatureTable = kq6Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_KQ7:
		signatureTable = kq7Signatures;
		break;
#endif
	case GID_LAURABOW:
		signatureTable = laurabow1Signatures;
		break;
	case GID_LAURABOW2:
		signatureTable = laurabow2Signatures;
		break;
	case GID_LONGBOW:
		signatureTable = longbowSignatures;
		break;
	case GID_LSL2:
		signatureTable = larry2Signatures;
		break;
	case GID_LSL5:
		signatureTable = larry5Signatures;
		break;
	case GID_LSL6:
		signatureTable = larry6Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_LSL6HIRES:
		signatureTable = larry6HiresSignatures;
		break;
	case GID_LSL7:
		signatureTable = larry7Signatures;
		break;
#endif
	case GID_MOTHERGOOSE256:
		signatureTable = mothergoose256Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_MOTHERGOOSEHIRES:
		signatureTable = mothergooseHiresSignatures;
		break;

	case GID_PHANTASMAGORIA:
		signatureTable = phantasmagoriaSignatures;
		break;

	case GID_PHANTASMAGORIA2:
		signatureTable = phantasmagoria2Signatures;
		break;
#endif
	case GID_PQ1:
		signatureTable = pq1vgaSignatures;
		break;
#ifdef ENABLE_SCI32
	case GID_PQ4:
		signatureTable = pq4Signatures;
		break;
	case GID_PQSWAT:
		signatureTable = pqSwatSignatures;
		break;
#endif
	case GID_QFG1:
		signatureTable = qfg1egaSignatures;
		break;
	case GID_QFG1VGA:
		signatureTable = qfg1vgaSignatures;
		break;
	case GID_QFG2:
		signatureTable = qfg2Signatures;
		break;
	case GID_QFG3:
		signatureTable = qfg3Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_QFG4:
		signatureTable = qfg4Signatures;
		break;
	case GID_RAMA:
		signatureTable = ramaSignatures;
		break;
	case GID_SHIVERS:
		signatureTable = shiversSignatures;
		break;
#endif
	case GID_SQ1:
		signatureTable = sq1vgaSignatures;
		break;
	case GID_SQ4:
		signatureTable = sq4Signatures;
		break;
	case GID_SQ5:
		signatureTable = sq5Signatures;
		break;
#ifdef ENABLE_SCI32
	case GID_SQ6:
		signatureTable = sq6Signatures;
		break;
	case GID_TORIN:
		signatureTable = torinSignatures;
		break;
#endif
	default:
		break;
	}

	if (signatureTable) {
		_isMacSci11 = (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_1_1);

		if (!_runtimeTable) {
			// Abort, in case selectors are not yet initialized (happens for games w/o selector-dictionary)
			if (!g_sci->getKernel()->selectorNamesAvailable())
				return;

			// signature table needs to get initialized (Magic DWORD set, selector table set)
			initSignature(signatureTable);

			// Do additional game-specific initialization
			switch (gameId) {
			case GID_KQ5:
				if (g_sci->_features->useAltWinGMSound()) {
					// See the explanation in the kq5SignatureWinGMSignals comment
					enablePatch(signatureTable, "Win: GM Music signal checks");
				}
				break;
			case GID_KQ6:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for King's Quest 6
					enablePatch(signatureTable, "CD: audio + text support");
				}
				break;
			case GID_LAURABOW2:
				if (g_sci->isCD()) {
					// Enables Dual mode patches (audio + subtitles at the same time) for Laura Bow 2
					enablePatch(signatureTable, "CD: audio + text support");
				}
				break;
			default:
				break;
			}
		}

		curEntry = signatureTable;
		curRuntimeEntry = _runtimeTable;

		while (curEntry->signatureData) {
			if ((scriptNr == curEntry->scriptNr) && (curRuntimeEntry->active)) {
				int32 foundOffset = 0;
				int16 applyCount = curEntry->applyCount;
				do {
					foundOffset = findSignature(curEntry, curRuntimeEntry, scriptData);
					if (foundOffset != -1) {
						// found, so apply the patch
						debugC(kDebugLevelScriptPatcher, "Script-Patcher: '%s' on script %d offset %d", curEntry->description, scriptNr, foundOffset);
						applyPatch(curEntry, scriptData, foundOffset);
					}
					applyCount--;
				} while ((foundOffset != -1) && (applyCount));
			}
			curEntry++; curRuntimeEntry++;
		}
	}
}

} // End of namespace Sci

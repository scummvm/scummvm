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

#ifndef ULTIMA8_CONVERT_U8_CONVERTUSECODECRUSADER_H
#define ULTIMA8_CONVERT_U8_CONVERTUSECODECRUSADER_H

#ifndef INCLUDE_CONVERTUSECODEU8_WITHOUT_BRINGING_IN_FOLD
#include "ultima/ultima8/convert/convert.h"
#else
#include "ultima/ultima8/convert/u8/convert_usecode_u8.h"
#endif

namespace Ultima {
namespace Ultima8 {

class ConvertUsecodeCrusader : public ConvertUsecode {
public:
	const char* const *intrinsics() override  { return _intrinsics;  };
	const char* const *event_names() override { return _event_names; };
	void readheader(Common::SeekableReadStream *ucfile, UsecodeHeader &uch, uint32 &curOffset) override;
	void readevents(Common::SeekableReadStream *ucfile, const UsecodeHeader &/*uch*/) override
	{
#ifndef INCLUDE_CONVERTUSECODEU8_WITHOUT_BRINGING_IN_FOLD
		EventMap.clear();
		uint32 num_crusader_routines = uch.offset / 6;
		for (uint32 i=0; i<32; ++i)
		{
			/*uint32 size =*/ ucfile->readUint16LE();
			uint32 offset = ucfile->readUint32LE();
			EventMap[offset] = i;
#ifdef DISASM_DEBUG
			pout << "Crusader Routine: " << i << ": " << Std::hex << Std::setw(4) << offset << Std::dec << endl;
#endif
		}
#endif
	}

	void readOp(TempOp &op, Common::SeekableReadStream *ucfile, uint32 &dbg_symbol_offset, Std::vector<DebugSymbol> &debugSymbols, bool &done) override
	{ readOpGeneric(op, ucfile, dbg_symbol_offset, debugSymbols, done, true); };
	Node *readOp(Common::SeekableReadStream *ucfile, uint32 &dbg_symbol_offset, Std::vector<DebugSymbol> &debugSymbols, bool &done) override
	{ return readOpGeneric(ucfile, dbg_symbol_offset, debugSymbols, done, true); };

private:
	static const char* const _intrinsics[512];
	static const char* const _event_names[];
};

// By convention, last pushed argument goes first on the list.
const char* const ConvertUsecodeCrusader::_intrinsics[] = {
	// 0000
	"byte Ultima8Engine::I_getAlertActive(void)",
	"int16 Item::I_getFrame(Item *)",
	"void Item::I_setFrame(Item *, frame)",
	"int16 Item::I_getMapArray(Item *)", // See TRIGGER::ordinal21 - stored in a variable 'mapNum'
	"int16 Item::I_getStatus(Item *)",
	"void Item::I_orStatus(Item *, uint16 flags)",
	"int16 Intrinsic006(6 bytes)", // same coff as 0B5
	"byte Item::I_getSOMETHING_07(Item *)", // called for gattling guns and camera
	"byte Actor::I_isNPC(Item *)", // proably - actually checks is itemno < 256?
	"byte Item::I_getZ(Item *)",
	"void Item::I_destroy(Item *)", // probably? often called after creating a replacement object and setting it to the same position (eg, LUGGAGE::gotHit)
	"int16 I_GetNPCDataField0x63_00B(Actor *)", // could be getNPCNum? Called from ANDROID::calledFromAnim, goes to NPCDEATH
	"void I_NPCsetSomething_00C(int)",
	"byte Item::I_getDirToItem(Item *, itemno)", // based on disasm
	"int16 I_NPCSomething00E(Actor *, int, int)",
	"void I_playFlic(void), int16 I_playFlic(Item *, char *name, int16 sizex, int16 sizey)",
	// 0010
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff as 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Actor::I_getMap(4 bytes)", // based on disasm.
	"void Intrinsic012(2 bytes)",
	"int16 Item::I_getX(Item *)",
	"int16 Item::I_getY(Item *)",
	"void AudioProcess::I_playSFXCru(Item *, uint16 sfxnum)",
	"int16 Item::I_getShape(Item *)", // in STEAMBOX::func0A, is compared to 0x511 (the STEAM2 shape number) to determine direction
	"void Intrinsic017(8 bytes)",
	"int16 UCMachine::I_rndRange(uint16 x, uint16 y)", // // probably.. always called with 2 constants, then result often compared to some number between
	"byte Item::I_legalCreateAtCoords(Item *, int16 shapeno, int16 frame, int16 x, int16 y, int16 z)", // probably, see usage in DOOR2::ordinal37
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132. Always associated with a bitwise-not or bitmask
	"int16 Intrinsic01B(void)",
	"byte Actor::I_getDir(4 bytes)", // based on disasm. same coff as 112, 121
	"int16 Actor::I_getLastAnimSet(4 bytes)", // based on disasm. part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"int16 Intrinsic01E(16 bytes)",
	"byte Item::I_create(Item *, uint16 shapenum, uint16 framenum)", // probably - used in MISS1EGG referencing keycards and NPCDEATH in creating blood spills
	// 0020
	"void Item::I_popToCoords(Item *, uint16 x, uint16 y, uint16 z)", // set coords, used after creating blood spills in NPCDEATH
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void I_push(Item *)", // same code as U8
	"int16 Intrinsic023(void)", // returns some ItemCache global in disassembly
	"void Item::I_setShape(Item *, int16 shapeno)", // probably. See PEPSIEW::gotHit.
	"void Item::I_touch(Item *)", // same code as U8
	"int16 Item::I_getQHi(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"int16 I_getDirectionSomething(x1, y1, x2, y2, numdirs, aa, bb)",  // TODO: understand the decompile of this better.. what is it doing with aa and bb?
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 I_GetGlobal7e2d_029(void)",
	"void AudioProcess::I_playAmbientSFXCru(Item *, sndno)",
	"int16 Item::I_getQLo(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"byte Item::I_inFastArea(Item *)",
	"void Item::I_setQHi(Item *, uint16 qhi)", // probably setQHi, see usage in FREE::ordinal2E where object position is copied
	"byte I_legalMoveToPoint(Item *, Point *, int16 force)", // based on disasm
	"byte Intrinsic02F(10 bytes)",
	// 0030
	"void Item::I_pop(Item *)", // same code as U8
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Intrinsic032(12 bytes)",
	"byte Actor::I_isBusy(4 bytes)", // same code as U8
	"int16 Actor::I_getDir16(x1, y1, x2, y2)",
	"byte Actor::I_getSomeFlagMaybeCrouch(Item *)",
	"int16 Actor::I_doAnim(12 bytes)", // v. similar code to U8
	"byte Intrinsic037(4 bytes)", // same coff as 0B8
	"void AudioProcess::Intrinsic038(Item *, int16 sndno)",
	"byte Actor::I_isDead(Item *)", // same coff as 122, 12E
	"byte AudioProcess::Intrinsic03A(Item *, int16 unk)",
	"void Item::I_setQLo(Item *, int16 qlo)", // probably setQLo, see usage in FREE::ordinal2E where object position is copied.  Disassembly confirms.
	"int16 Item::I_getItemFamily(Item *)", // based on disasm
	"void Container::I_destroyContents(Item *)",
	"void Item::I_fallProbably(Item *)", // similar disasm to U8, but not totally the same.
	"int16 Egg::I_getEggId(Item *)", // from disasm
	// 0040
	"void CameraProcess::I_move_to(x, y, z)",
	"void CameraProcess::I_setCenterOn(objid)",
	"byte Intrinsic042(6 bytes)",
	"void AudioProcess::Intrinsic043(Item *, soundno)", // something with sound, probably Item *, soundno based on disassembly, maybe stopSFX?
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno))", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_IsOn(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 0050
	"int16 I_GetNPCDataField0x2_050(Actor *)",
	"void I_NPCSomething_051(Actor *)",
	"void Intrinsic052(6 bytes)",
	"void I_SetNPCDataField0x8_053(Actor *, int)",
	"void I_SetNPCDataField0xA_054(Actor *, int)",
	"void I_NPCSetActivityProbably_055(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void Intrinsic056(2 bytes)",
	"int16 Item::I_getSOMETHING_57(Item *)",
	"byte Item::I_doSOMETHING_58(Item *, uint16 unk)",
	"void Item::I_setFrame(Item *, frame)", // based on same coff as 002
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"byte Item::I_legalCreateAtPoint(Item *, int16 shape, int16 frame, Point *)", // see PEPSIEW::use
	"void Item::I_getPoint(Item *, Point *)",
	"void Intrinsic05D(void)",
	"int16 I_playFlicsomething(uint32, char *, int16 a, int16 b)", // Play video (as texture? parameters like (150, 250, "MVA11A") and other mvas)
	"void Intrinsic05F(void)",
	// 0060
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Actor::I_create(8 bytes)",
	"void Intrinsic062(void)",
	"void Actor::I_teleport(12 bytes)", // based on disasm same as U8
	"void Item::I_getFootpad(Item *, uint *, uint *, uint *)", // based on disasm. same coff as 12D
	"byte Item::I_isInNPC(Item *)", // based on disasm - following parent containers, is one of them an NPC
	"int16 	Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_setNpcNum(Item *, uint16 npcnum)", // maybe, see EVENT::func0A or VALUEBOX::ordinal20.. right next to getNPCNum in coff (confirmed by disassembly)
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Item::I_move(Item *, int16 x, int16 y, uint16 z)",
	"int16 Intrinsic06B(void)",
	"void Intrinsic06C(sometimes Item *)", // TODO: when param not item, what is it?
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Item::I_isCompletelyOn(Item *, uint16 other)",
	// 0070
	"byte Intrinsic070(void)",
	"void Intrinsic071(void)",
	"void Intrinsic072(void)",
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Intrinsic074(void)",
	"void Intrinsic075(void)",
	"void AudioProcess::I_stopSFX(Item *)",
	"int16 Intrinsic077_Fade(void)", // something about fades
	"void Intrinsic078(void)",
	"int16 MainActor::I_teleportToEgg(int, int, int)",
	"int16 Intrinsic07A_Fade(void)", // something about fades
	"void Actor::I_clrImmortal(Actor *)", // same coff as 130
	"int16 I_GetNPCDataField0_07C(Actor *)",
	"void I_NPCSetActivityProbably_07D(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Item::I_getQuality(Item *)", // based on disassembly
	"void Item::I_setQuality(Item *, int)", // based on disassembly. same coff as 0BA, 125
	// 0080
	"int16 Item::I_use(Item *)", // same coff as 0D0, 0D5
	"int16 Intrinsic081(4 bytes)",
	"int16 I_GetNPCDataField0x57_082(Actor *)",
	"void I_SetNPCDataField0x57_083(Actor *, int)",
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void Actor::I_setImmortal(Actor *)",
	"int16 Intrinsic086(void)",
	"int16 Intrinsic087(void)",
	"void Item::I_setMapArray(Item *, uint16 maparray)", // based on decompile - sets same value as read by getmaparray .. see VALUEBOX:ordinal20
	"int16 Item::I_getNpcNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Intrinsic08A(12 bytes)",
	"int16 Item::I_enterFastArea(Item *)", // based on disasm, v similar to U8
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 119, 12A
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Intrinsic08F_Fade(void)", // something about fades
	// 0090
	"void Intrinsic090(void)",
	"void Intrinsic091(void)", // sets some global (cleared by 93)
	"void I_playFlic092(char *)", // same coff as 0A9
	"void Intrinsic093(void)", // clears som global (set by 91)
	"UNUSEDInt0094()",
	"byte Intrinsic095(void)", // get global - something about keyboard (by disasm)
	"int16 MainActor::I_teleportToEgg(int, int)", // a bit different to the U8 one - uses main actor map by default.
	"void Intrinsic097(void)",
	"void I_ProcessDeathSilence_098(void)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Intrinsic09A_Fade(void)", // something about fades
	"int16 Intrinsic09B_Fade(2 bytes)", // something about fades
	"int16 Intrinsic09C_Fade(4 bytes)", // something about fades
	"int16 Intrinsic09D_Fade(2 bytes)", // something about fades
	"int16 Intrinsic09E_Fade(4 bytes)", // something about fades
	"int16 Intrinsic09F_Fade(10 bytes)", // something about fades
	// 00A0
	"void I_SetItemFlag0x8000AndNPCField0x13Flag0_0A0(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getUnkEggType(Item *)", // based on disassembly, same as U8
	"void Egg::I_setEggXRange(Egg *, int)", // based on disasm
	"byte Item::I_overlaps(Item *, uint16 unk)", // same disasm as U8
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Intrinsic0A6(void)",
	"int16 Egg::I_getEggXRange(Egg *)", // based on disasm
	"void Actor::I_setDead(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void I_playFlic0A9(char *)", // same coff as 092
	"void I_playSFX(2 bytes)", // same coff as 0D4
	"byte I_NPCGetField0x59Flag1_0AB(Actor *)",
	"int16 I_maybeGetFamilyOfType(2 bytes)", // per pentagram notes
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 00B0
	"int16 Item::I_unequip(6 bytes)",
	"int16 Item::I_spawnUsecodeEvent0x13(Item *, 2 bytes)", // based on disasm - what is event 0x13? "avatar stole something" in U8..
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int32 I_getCurrentTimerTick(void)",
	"void Intrinsic0B4(void)",  // set or clear alert? (based on coff)
	"int16 Item::I_equip(6 bytes)",
	"void Intrinsic0B6(void)",  // set or clear alert? (based on coff)
	"int16 I_GetNPCGlobal0x7e24_0B7(void)",
	"byte Intrinsic0B8(4 bytes)", // same coff as 037
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Item::I_setQuality(Item *, int)", // same coff as 07F, 125
	"byte Intrinsic0BB(8 bytes)",
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	// 00C0
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Intrinsic0C4(2 bytes)",
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void SpriteProcess::I_createSprite(word, word, word, word, uword, uword, ubyte)",
	"byte Item::I_getDirFromItem(Item *, itemno)", // same disasm as U8
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte I_NPCSomething_0CA(Actor *, int)",
	"void Intrinsic0CB(2 bytes)",
	"byte I_GetNPCDataField0x59Flag3_0CC(Actor *)",
	"void I_NPCSetActivityProbably_0CD(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Intrinsic0CE(void)",
	"void Item::setQAndCallSomething(Item *, int16 q)", // based on disassembly
	// 00D0
	"int16 Item::I_use(Item *)", // same coff as 080, 0D5
	"UNUSEDInt00D1()", // I_StopAllSFX (unused so not implmeneted)
	"void Intrinsic0D2(int *item,char *flicname,word sizex,word sizey)", // play flic
	"UNUSEDInt00D3()",
	"void I_playSFX(2 bytes)", // same coff as 0AA.  Based on disasm.
	"int16 Item::I_use(Item *)", // same coff as 080, 0D0
	"byte Intrinsic0D6(void)",
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void Intrinsic0DA_Fade(void)", // something about fades
	"void I_NPCSetActivityProbably_0DB(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 I_GetNPCDataField0x4_0DD(Actor *)",
	"void I_SetNPCDataField0x5c_0DE(Actor *, int)",
	"int16 Actor::I_getEquip(6 bytes)", // based on disasm
	// 00E0
	"void Actor::I_setEquip(8 bytes)",
	"int16 I_GetNPCDataField0x6_0E1(Actor *)",
	"int16 I_GetNPCDataField0x8_0E2(Actor *)",
	"int16 I_GetNPCDataField0xa_0E3(Actor *)",
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void I_NPCSomething0E5(Actor *, uint16)",
	"void I_SetNPCDataField0x63_0E6(Actor *, int)",
	"void Actor::I_setDead(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_cast(6 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 I_GetQOfAvatarInventoryItem0x4ed_0EB(void)",
	"void Item::I_popToEnd(Item*, int)", // similar code to U8
	"void Item::I_popToContainer(Item*, int)",  // same code as U8
	"void Intrinsic0EE(void)",
	"int16 Intrinsic0EF(4 bytes not Item *)",
	// 00F0
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void I_NPCSetActivityProbably_0F2(Actor *, int)(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"int16 Item::I_getQ(Item *)", // based on disassembly
	"void Item::I_setQ(Item *, uint16 q)", // based on disassembly
	"void Intrinsic0F6(void)",
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte I_SomethingAboutGlobal7e2f_0FD(int)",
	"void Intrinsic0FE(4 bytes)",
	"int16 UCMachine::I_numToStr(int16 num)", // same as 113 based on same coff set 0FF, 113, 126
	// 0100
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // Based on variable name in TRIGGER::ordinal21. Part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Intrinsic103(uint16 shapenum)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	// 0110
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Actor::I_getDir(4 bytes)", // same coff as 01C, 121
	"int16 UCMachine::I_numToStr(int16 num)", // based on VMAIL::func0A example usage
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Intrinsic116(14 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 08C, 12A
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Item::I_getSomeFlag(Item *, uint16 unk)",
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_hurl(Item *,8 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Intrinsic11E(Item *)", // same coff as 12B
	"byte Intrinsic11F(Item *)",
	// 0120
	"int16 Intrinsic120(Item *)",
	"byte Actor::I_getDir(4 bytes)", // same coff as 01C, 112
	"byte Actor::I_isDead(Item *)", // same coff as 12E, 039
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Actor::I_getLastAnimSet(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Item::I_setQuality(Item *, int)", // same coff as 07F, 0BA
	"int16 UCMachine::I_numToStr(int16 num)", // same as 113 based on same coff set 0FF, 113, 126
	"byte Item::I_getDirToCoords(Item *, uin16 x, uint16 y)", // based on disassembly - FIXME: returns 2* values from U8
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 08C, 119
	"int16 Intrinsic12B(Item *)", // same coff as 11E
	"byte Item::I_isOn(Item *, itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void Item::I_getFootpadData(Item *, uint *, uint *, uint *)", // same coff as 064
	"byte Actor::I_isDead(Item *)", // same coff as 122, 039
	"int16 Intrinsic12F(Item *, uint16 unk)",
	// 0130
	"void Actor::I_clrImmortal(Actor *)", // same coff as 07B
	"void I_NPCSetActivityProbably_131(Actor *, int)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void Item::I_andStatus(Item *, int16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Intrinsic134(2 bytes)",
	"void Actor::I_setDead(Actor *)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void UNUSEDInt136()",
	"void UNUSEDInt137()"
};

const char * const ConvertUsecodeCrusader::_event_names[] = {
	"look()",						// 0x00
	"use()",						// 0x01
	"anim()",						// 0x02
	"setActivity()",				// 0x03
	"cachein()",					// 0x04
	"hit(uword, word)",				// 0x05
	"gotHit(uword, word)",			// 0x06
	"hatch()",						// 0x07
	"schedule()",					// 0x08
	"release()",					// 0x09
	"equip()",						// 0x0A
	"unequip()",					// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"cast(uword)",					// 0x11
	"justMoved()",					// 0x12
	"avatarStoleSomething(uword)",	// 0x13
	"animGetHit()",					// 0x14
	"guardianBark(word)",			// 0x15
	"func16",						// 0x16
	"func17",						// 0x17
	"func18",						// 0x18
	"func19",						// 0x19
	"func1A",						// 0x1A
	"func1B",						// 0x1B
	"func1C",						// 0x1C
	"func1D",						// 0x1D
	"func1E",						// 0x1E
	"func1F",						// 0x1F
	0
};

void ConvertUsecodeCrusader::readheader(Common::SeekableReadStream *ucfile, UsecodeHeader &uch, uint32 &curOffset) {
	#ifdef DISASM_DEBUG
	perr << Std::setfill('0') << Std::hex;
	perr << "unknown1: " << Std::setw(4) << ucfile->readUint32LE() << endl; // unknown
	uch.maxOffset = ucfile->readUint32LE() - 0x0C; // file size
	perr << "maxoffset: " << Std::setw(4) << maxOffset << endl;
	perr << "unknown2: " << Std::setw(4) << ucfile->readUint32LE() << endl; // unknown
	curOffset = 0;
	#else
	ucfile->readUint32LE(); // unknown
	uch._maxOffset = ucfile->readUint32LE() - 0x0C; // file size
	ucfile->readUint32LE(); // unknown
	curOffset = 0;
	#endif
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

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
	"int16 I_getSOMETHING_0B(MainActor *?)", // could be getNPCNum? Called from ANDROID::calledFromAnim, goes to NPCDEATH
	"void Intrinsic00C(2 bytes)",
	"byte Item::Intrinsic00D(Item *, itemno)",
	"int16 Intrinsic00E(8 bytes)",
	"void I_playFlic(void), int16 I_playFlic(Item *, char *name, int16 sizex, int16 sizey)",
	// 0010
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff as 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Intrinsic011(4 bytes)",
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
	"byte Intrinsic01C(4 bytes)", // same coff as 112, 121
	"int16 Intrinsic01D(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"int16 Intrinsic01E(16 bytes)",
	"byte Item::I_create(Item *, uint16 shapenum, uint16 framenum)", // probably - used in MISS1EGG referencing keycards and NPCDEATH in creating blood spills
	// 0020
	"void Item::I_popToCoords(Item *, uint16 x, uint16 y, uint16 z)", // set coords, used after creating blood spills in NPCDEATH
	"void Intrinsic021(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Intrinsic022(Item *)", // I_push??
	"int16 Intrinsic023(void)", // returns some ItemCache global in disassembly
	"void Item::I_setShape(Item *, int16 shapeno)", // probably. See PEPSIEW::gotHit.
	"void I_maybeTouch025(4 bytes)", // I_touch??
	"int16 Item::I_getQHi(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"int16 Intrinsic027(14 bytes)",
	"int16 Intrinsic028(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Intrinsic029([pid, [byte]])",
	"void AudioProcess::I_playAmbientSFXCru(Item *, sndno)",
	"int16 Item::I_getQLo(Item *)", // guess, based on variable name in BOUNCBOX::gotHit
	"byte Item::I_inFastArea(Item *)",
	"void Item::I_setQHi(Item *, uint16 qhi)", // probably setQHi, see usage in FREE::ordinal2E where object position is copied
	"byte Intrinsic02E(Item *, 8 bytes)",
	"byte Intrinsic02F(10 bytes)",
	// 0030
	"void I_maybePop030(4 bytes)", // I_pop??
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Intrinsic032(12 bytes)",
	"byte Intrinsic033(4 bytes)",
	"int16 Intrinsic034(8 bytes)",
	"byte Intrinsic035(4 bytes)",
	"int16 Intrinsic036(12 bytes)",
	"byte Intrinsic037(4 bytes)", // same coff as 0B8
	"void AudioProcess::Intrinsic038(Item *, int16 sndno)",
	"byte Intrinsic039(4 bytes)", // same coff as 122, 12E
	"byte AudioProcess::Intrinsic03A(Item *, int16 unk)",
	"void Item::I_setQLo(Item *, int16 qlo)", // probably setQLo, see usage in FREE::ordinal2E where object position is copied.  Disassembly confirms.
	"int16 Item::I_getSomethingShapey(Item *)", // something shape related according to disasm
	"void Intrinsic03D(4 bytes)",
	"void Intrinsic03E(4 bytes)",
	"int16 Intrinsic03F(4 bytes)",
	// 0040
	"void Intrinsic040(8 bytes)",
	"void Intrinsic041(2 bytes)",
	"byte Intrinsic042(6 bytes)",
	"void AudioProcess::Intrinsic043(Item *, soundno)", // something with sound, probably Item *, soundno based on disassembly, maybe stopSFX?
	"byte Item::Intrinsic044(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::Intrinsic046(Item *, uint16 itemno))", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::Intrinsic048(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::Intrinsic04A(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::Intrinsic04C(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Item::Intrinsic04E(Item *, uint16 itemno)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 0050
	"int16 Intrinsic050(4 bytes)",
	"void Intrinsic051(4 bytes)",
	"void Intrinsic052(6 bytes)",
	"void Intrinsic053(6 bytes)",
	"void Intrinsic054(6 bytes)",
	"void Intrinsic055(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void Intrinsic056(2 bytes)",
	"int16 Item::I_getSOMETHING_57(Item *)",
	"byte Item::I_doSOMETHING_58(Item *, uint16 unk)",
	"void Item::I_setFrame(Item *, frame)", // based on same coff as 002
	"int16 Intrinsic05A(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"byte Item::I_legalCreateAtPoint(Item *, int16 shape, int16 frame, Point *)", // see PEPSIEW::use
	"void Item::I_getPoint(Item *, Point *)",
	"void Intrinsic05D(void)",
	"int16 I_playFlicsomething(uint32, char *, int16 a, int16 b)", // Play video (as texture? parameters like (150, 250, "MVA11A") and other mvas)
	"void Intrinsic05F(void)",
	// 0060
	"void Intrinsic060(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Intrinsic061(8 bytes)",
	"void Intrinsic062(void)",
	"void Intrinsic063(12 bytes)",
	"void Item::Intrinsic064(Item *, uint *, uint *, uint *)", // same coff as 12D
	"byte Intrinsic065(4 bytes)",
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_setNpcNum(Item *, uint16 npcnum)", // maybe, see EVENT::func0A or VALUEBOX::ordinal20.. right next to getNPCNum in coff (confirmed by disassembly)
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Item::Intrinsic06A(Item *, int16 x, int16 y, uint16 z)",
	"int16 Intrinsic06B(void)",
	"void Intrinsic06C(sometimes Item *)", // TODO: when param not item, what is it?
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Intrinsic06F(6 bytes)",
	// 0070
	"byte Intrinsic070(void)",
	"void Intrinsic071(void)",
	"void Intrinsic072(void)",
	"void Intrinsic073(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void Intrinsic074(void)",
	"void Intrinsic075(void)",
	"void AudioProcess::Intrinsic076(Item *)",
	"int16 Intrinsic077_Fade(void)", // something about fades
	"void Intrinsic078(void)",
	"int16 Intrinsic079(6 bytes)",
	"int16 Intrinsic07A_Fade(void)", // something about fades
	"void Intrinsic07B(4 bytes)", // same coff as 130
	"int16 Intrinsic07C(4 bytes)",
	"void Intrinsic07D(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 I_getQIfSomething(Item *)", // based on disassembly
	"void Intrinsic07F(6 bytes)", // same coff as 0BA, 125
	// 0080
	"int16 Intrinsic080(4 bytes)", // same coff as 0D0, 0D5
	"int16 Intrinsic081(4 bytes)",
	"int16 Intrinsic082(4 bytes)",
	"void Intrinsic083(6 bytes)",
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void Intrinsic085(4 bytes)",
	"int16 Intrinsic086(void)",
	"int16 Intrinsic087(void)",
	"void Item::I_setMapArray(Item *, uint16 maparray)", // based on decompile - sets same value as read by getmaparray .. see VALUEBOX:ordinal20
	"int16 Item::I_getNpcNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Intrinsic08A(12 bytes)",
	"int16 Item::I_enterFastArea(Item *)", // maybe.. based on disasm
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 119, 12A
	"int16 Intrinsic08D(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Intrinsic08F_Fade(void)", // something about fades
	// 0090
	"void Intrinsic090(void)",
	"void Intrinsic091(void)", // sets some global (cleared by 93)
	"void I_playFlic092(char *)", // same coff as 0A9
	"void Intrinsic093(void)", // clears som global (set by 91)
	"UNUSEDInt0094()",
	"byte Intrinsic095(void)", // get global - something about keyboard (by disasm)
	"int16 Intrinsic096(4 bytes)",
	"void Intrinsic097(void)",
	"void Intrinsic098(void)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"void Intrinsic09A_Fade(void)", // something about fades
	"int16 Intrinsic09B_Fade(2 bytes)", // something about fades
	"int16 Intrinsic09C_Fade(4 bytes)", // something about fades
	"int16 Intrinsic09D_Fade(2 bytes)", // something about fades
	"int16 Intrinsic09E_Fade(4 bytes)", // something about fades
	"int16 Intrinsic09F_Fade(10 bytes)", // something about fades
	// 00A0
	"void Intrinsic0A0(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::getQLoIfSomething(Item *)", // based on disassembly
	"void Intrinsic0A3(6 bytes)",
	"byte Item::Intrinsic0A4(Item *, uint16 unk)",
	"byte Intrinsic0A5(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Intrinsic0A6(void)",
	"int16 Intrinsic0A7(4 bytes)",
	"void Intrinsic0A8(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"void I_playFlic0A9(char *)", // same coff as 092
	"void I_playSFX(2 bytes)", // same coff as 0D4
	"byte Intrinsic0AB(4 bytes)",
	"int16 I_maybeGetFamilyOfType(2 bytes)", // per pentagram notes
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	// 00B0
	"int16 Intrinsic0B0(6 bytes)",
	"int16 Intrinsic0B1(6 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int32 I_getCurrentTimerTick(void)",
	"void Intrinsic0B4(void)",  // set or clear alert? (based on coff)
	"int16 Intrinsic0B5(6 bytes)",
	"void Intrinsic0B6(void)",  // set or clear alert? (based on coff)
	"int16 Intrinsic0B7(void)",
	"byte Intrinsic0B8(4 bytes)", // same coff as 037
	"int16 Intrinsic0B9(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Intrinsic0BA(6 bytes)", // same coff as 07F, 125
	"byte Intrinsic0BB(8 bytes)",
	"byte Intrinsic0BC(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Intrinsic0BD(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	// 00C0
	"int16 Intrinsic0C0(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Intrinsic0C2(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Intrinsic0C4(2 bytes)",
	"byte Intrinsic0C5(6 bytes)",
	"void Intrinsic0C6(14 bytes)",
	"byte Item::Intrinsic0C7(Item *, itemno)", // does something with the x/y from itemno
	"int16 Intrinsic0C8(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Intrinsic0CA(6 bytes)",
	"void Intrinsic0CB(2 bytes)",
	"byte Intrinsic0CC(4 bytes)",
	"void Intrinsic0CD(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Intrinsic0CE(void)",
	"void Item::setQAndCallSomething(Item *, int16 q)", // based on disassembly
	// 00D0
	"int16 Intrinsic0D0(4 bytes)", // same coff as 080, 0D5
	"UNUSEDInt00D1()", // I_StopAllSFX (unused so not implmeneted)
	"void Intrinsic0D2(int *item,char *flicname,word sizex,word sizey)", // play flic
	"UNUSEDInt00D3()",
	"void I_playSFX(2 bytes)", // same coff as 0AA.  Based on disasm.
	"int16 Intrinsic0D5(4 bytes)", // same coff as 080, 0D0
	"byte Intrinsic0D6(void)",
	"int16 Intrinsic0D7(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Intrinsic0D8(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"void Intrinsic0DA_Fade(void)", // something about fades
	"void Intrinsic0DB(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"byte Intrinsic0DC(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Intrinsic0DD(4 bytes)",
	"void Intrinsic0DE(6 bytes)",
	"int16 Intrinsic0DF(6 bytes)",
	// 00E0
	"void Intrinsic0E0(8 bytes)",
	"int16 Intrinsic0E1(4 bytes)",
	"int16 Intrinsic0E2(4 bytes)",
	"int16 Intrinsic0E3(4 bytes)",
	"int16 Intrinsic0E4(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Intrinsic0E5(6 bytes)",
	"void Intrinsic0E6(6 bytes)",
	"void Intrinsic0E7(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
	"int16 Intrinsic0E8(6 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQLo(Item *)", // same as 02B based on same coff set 010, 02B, 066, 084, 0A1, 0AE, 0D9, 0EA
	"int16 Intrinsic0EB(void)",
	"void Intrinsic0EC(6 bytes)",
	"void Intrinsic0ED(6 bytes)", // poptocontainer??
	"void Intrinsic0EE(void)",
	"int16 Intrinsic0EF(4 bytes not Item *)",
	// 00F0
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"byte Intrinsic0F1(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void Intrinsic0F2(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"int16 Item::I_getQ(Item *)", // based on disassembly
	"void Item::I_setQ(Item *, uint16 q)", // based on disassembly
	"void Intrinsic0F6(void)",
	"int16 Intrinsic0F7(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Intrinsic0F9(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"byte Intrinsic0FA(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Intrinsic0FD(2 bytes)",
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
	"byte Intrinsic112(4 bytes)", // same coff as 01C, 121
	"int16 UCMachine::I_numToStr(int16 num)", // based on VMAIL::func0A example usage
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"byte Intrinsic116(14 bytes)",
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Intrinsic118(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 08C, 12A
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"byte Item::I_getSomeFlag(Item *, uint16 unk)",
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Intrinsic11D(12 bytes)", // part of same coff set 028, 08D, 0BD, 0C0, 0C2, 0C8, 0F7, 0F9, 118, 11D
	"int16 Intrinsic11E(Item *)", // same coff as 12B
	"byte Intrinsic11F(Item *)",
	// 0120
	"int16 Intrinsic120(Item *)",
	"byte Intrinsic121(4 bytes)", // same coff as 01C, 112
	"byte Intrinsic122(4 bytes)", // same coff as 12E, 039
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"int16 Intrinsic124(4 bytes)", // part of same coff set 01D, 05A, 0B9, 0D7, 0E4, 124
	"void Intrinsic125(6 bytes)", // same coff as 07F, 0BA
	"int16 UCMachine::I_numToStr(int16 num)", // same as 113 based on same coff set 0FF, 113, 126
	"byte Item::Intrinsic127(Item *, uin16 x, uint16 y)", // based on disassembly
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getNPCNum(Item *)", // part of same coff set 067, 06D, 089, 08E, 0AD, 0F8, 100, 102, 105, 107, 109, 10B, 10D, 10F, 111, 115, 11C, 123, 129
	"void Item::I_DoSomethingAndSetStatusFlag0x8000(Item *)", // same coff as 08C, 119
	"int16 Intrinsic12B(Item *)", // same coff as 11E
	"byte Intrinsic12C(6 bytes)", // part of same coff set 044, 046, 048, 04A, 04C, 04E, 0A5, 0BC, 0C5, 0DC, 0F1, 0FA, 12C
	"void Item::Intrinsic12D(Item *, uint *, uint *, uint *)", // same coff as 064
	"byte Intrinsic12E(4 bytes)", // same coff as 122, 039
	"int16 Intrinsic12F(Item *, uint16 unk)",
	// 0130
	"void Intrinsic130(4 bytes)", // same coff as 07B
	"void Intrinsic131(6 bytes)", // part of same coff set 055, 07D, 0CD, 0DB, 0F2, 131
	"void Item::I_andStatus(Item *, uint16 status)", // part of same coff set 01A, 031, 069, 06E, 099, 0B2, 0BF, 0C1, 0C3, 0E9, 0FC, 101, 104, 106, 108, 10A, 10C, 10E, 110, 114, 117, 11A, 128, 132
	"int16 Item::I_getQHi(Item *)", // same as 026 based on same coff set 026, 045, 047, 049, 04B, 04D, 04F, 0AF, 0BE, 0C9, 0F0, 0F3, 0FB, 133
	"void Intrinsic134(2 bytes)",
	"void UNUSEDInt135(4 bytes)", // part of same coff set 021, 060, 073, 0A0, 0A8, 0D8, 0E7, 135
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
	"AvatarStoleSomething(uword)",	// 0x13
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

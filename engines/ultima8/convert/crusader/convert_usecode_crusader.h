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

#ifndef ULTIMA8_CONVERT_CRuSADER_CONVERTUSECODECRUSADER_H
#define ULTIMA8_CONVERT_CRuSADER_CONVERTUSECODECRUSADER_H

#include "ultima8/convert/convert.h"

/* Needs to be split into CrusaderRemorse and CrusaderRegret classes */

namespace Ultima8 {

class ConvertUsecodeCrusader : public ConvertUsecode {
	public:
		const char* const *intrinsics()  { return _intrinsics;  };
		const char* const *event_names() { return _event_names; };
		void readheader(IDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset);
		void readevents(IDataSource *ucfile, const UsecodeHeader &uch)
		{
			EventMap.clear();
			uint32 num_crusader_routines = uch.offset / 6;
			for (uint32 i=0; i < num_crusader_routines; i++) {
				/*uint32 size =*/ read2(ucfile);
				uint32 offset = read4(ucfile);
				EventMap[offset] = i;
				#ifdef DISASM_DEBUG
				pout << "Crusader Routine: " << i << ": " << std::hex << std::setw(4) << offset << std::dec << " size " << size << endl;
				#endif
			}
		};
		
		// as weird as this may seem, we'll start this with Crusader's opcodes first.
		// They're both simpler and more complex. *grin*
		void readOp(TempOp &op, IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)
		{ readOpGeneric(op, ucfile, dbg_symbol_offset, debugSymbols, done, true); };
		Node *readOp(IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)
		{ return readOpGeneric(ucfile, dbg_symbol_offset, debugSymbols, done, true); };

	private:
		static const char* const _intrinsics[];
		static const char* const _event_names[];
};

// current discovered intrinsics are for regret1.21 only
const char* const ConvertUsecodeCrusader::_intrinsics[] = {
	// 0000
	"Intrinsic0000()",
	"Item::getFrame(void)",
	"Item::setFrame(uint16)",
	"Item::getMapNum(void)",
	"Item::getStatus(void)",
	"Item::orStatus(sint16)",
	"Item::callEvent0A(sint16)",
	"Intrinsic0007()",
	"Item::isNpc(void)",
	"Item::getZ(void)",
	"Intrinsic000A()",
	"Item::getQLo(void)",
	"Item::destroy(void)",
	"Intrinsic000D()",
	"Item::getX(void)",
	"Item::getY(void)",
	// 0010
	"Intrinsic0010()",
	"Item::getType(void)",
	"Intrinsic0012()",
	"Intrinsic0013()",
	"Item::legal_create(uint16,uint16,uint16,uint16,uint16)",
	"Item::andStatus(void)",
	"Intrinsic0016()",
	"Intrinsic00C3()",
	"Intrinsic00DA()",
	"Intrinsic0019()",
	"Item::create(uint16,uint16)",
	"Item::pop(uint16,uint16,uint8)",
	"Intrinsic00FA()",
	"Item::push(void)",
	"Intrinsic001E()",
	"Item::getQLo(void)",
	// 0020
	"Item::setQLo(sint16)",
	"Item::getQHi(void)",
	"Item::setQHi(sint16)",
	"Intrinsic0023()",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getCY(void)",
	"Item::getCX(void)",
	"Intrinsic0027()",
	"Item::setNpcNum(sint16)",
	"Intrinsic0029()",
	"Intrinsic002A()",
	"Item::pop(void)",
	"Intrinsic002C()",
	"Item::isCompletelyOn(uint16)",
	"Item::pop(uint16)",
	"Intrinsic002F()",
	// 0030
	"Intrinsic0030()",
	"Item::getFamily(void)",
	"Item::destroyContents(void)",
	"Intrinsic0033()",
	"Item::getDirToItem(uint16)",
	"Intrinsic0035()",
	"Intrinsic0036()",
	"Intrinsic0037()",
	"Item::andStatus(void)",
	"Kernel::resetRef(uint16,ProcessType)",
	"Item::touch(void)",
	"Egg::getEggId(void)",
	"Intrinsic003C()",
	"Intrinsic003D()",
	"Item::callEvent11(sint16)",
	"Intrinsic003F()",
	// 0040
	"Intrinsic0040()",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Intrinsic004D()",
	"Npc::isDead(void)",
	"Intrinsic009C()",
	// 0050
	"Intrinsic0050()",
	"Intrinsic0051()",
	"Intrinsic0052()",
	"Intrinsic00BD()",
	"Intrinsic0054()",
	"Intrinsic0055()",
	"Intrinsic0056()",
	"Intrinsic0057()",
	"Item::use(void)",
	"Item::setQuantity(sint16)",
	"Intrinsic005A()",
	"Item::getSurfaceWeight(void)",
	"Intrinsic005C()",
	"Item::setFrame(uint16)",
	"Intrinsic00DA()",
	"Intrinsic005F()",
	// 0060
	"Intrinsic0060()",
	"Intrinsic0061()",
	"Intrinsic0062()",
	"Item::legal_create(uint16,uint16,WorldPoint&)",
	"Item::getPoint(WorldPoint&)",
	"Item::legal_move(WorldPoint&,uint16,uint16)",
	"Item::fall(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Kernel::getNumProcesses(uint16,ProcessType)",
	"Item::getCY(void)",
	"Intrinsic006A()",
	"Intrinsic006B()",
	"Intrinsic006C()",
	"Intrinsic006D()",
	"Intrinsic006E()",
	"Item::isInNpc(void)",
	// 0070
	"Intrinsic0070()",
	"Intrinsic0071()",
	"Intrinsic0072()",
	"Intrinsic0073()",
	"Npc::isDead(void)",
	"Item::getNpcNum(void)",
	"IntrinsicReturn0",
	"Intrinsic0077()",
	"Item::callEvent0B(sint16)",
	"Item::andStatus(void)",
	"Item::move(uint16,uint16,uint8)",
	"Intrinsic007B()",
	"Intrinsic007C()",
	"Intrinsic007D()",
	"Intrinsic007E()",
	"Intrinsic007F()",
	// 0080
	"Intrinsic0080()",
	"Intrinsic0081()",
	"Intrinsic0082()",
	"Intrinsic0083()",
	"Intrinsic0084()",
	"Intrinsic0085()",
	"teleportToEgg(sint16,int,uint8)",
	"Intrinsic0087()",
	"Intrinsic0088()",
	"Intrinsic00BD()",
	"Item::getQuality(void)",
	"Item::setQuality(sint16)",
	"Intrinsic008C()",
	"Intrinsic008D()",
	"Intrinsic008E()",
	"Camera::getX(void)",
	// 0090
	"Camera::getY(void)",
	"Item::setMapNum(sint16)",
	"Item::getNpcNum(void)",
	"Item::shoot(WorldPoint&,sint16,sint16)",
	"Intrinsic0094()",
	"Item::enterFastArea(void)",
	"Intrinsic00CA()",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Intrinsic0099()",
	"teleportToEgg(sint16,uint8)",
	"Intrinsic009B()",
	"Intrinsic009C()",
	"Intrinsic009D()",
	"Intrinsic009E()",
	"Intrinsic009F()",
	// 00A0
	"Item::andStatus(void)",
	"Item::getUnkEggType(void)",
	"Egg::setEggXRange(uint16)",
	"Item::setFrame(uint16)",
	"Item::overlaps(uint16)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Intrinsic00DA()",
	"Item::getCY(void)",
	"Intrinsic00A9()",
	"Item::isOn(uint16)",
	"Npc::isDead(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Intrinsic00AD()",
	"Item::getQHi(void)",
	"Item::andStatus(void)",
	// 00B0
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::andStatus(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::andStatus(void)",
	"Item::getDirToCoords(uint16,uint16)",
	"Intrinsic00B5()",
	"Intrinsic00B6()",
	"Item::getNpcNum(void)",
	"Item::getCY(void)",
	"Item::isOn(uint16)",
	"Item::getFootpad(sint16&,sint16&,sint16&)",
	"Npc::isDead(void)",
	"Intrinsic00BC()",
	"Intrinsic00BD()",
	"Intrinsic00BE()",
	"Item::andStatus(void)",
	// 00C0
	"Intrinsic00C0()",
	"Intrinsic00C1()",
	"IntrinsicReturn0",
	"Intrinsic00C3()",
	"Item::getQHi(void)",
	"Item::setQuality(sint16)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Intrinsic00C7()",
	"Intrinsic00C8()",
	"Item::callEvent0A(sint16)",
	"Intrinsic00CA()",
	"Item::isOn(uint16)",
	"Intrinsic00CC()",
	"Intrinsic00CD()",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	// 00D0
	"Intrinsic00D0()",
	"Intrinsic00D1()",
	"Intrinsic00D2()",
	"Intrinsic00FA()",
	"Camera::getY(void)",
	"Intrinsic00D5()",
	"Intrinsic00D6()",
	"Intrinsic00D7()",
	"Intrinsic00D8()",
	"Intrinsic00D9()",
	"Intrinsic00DA()",
	"Intrinsic00DB()",
	"Item::getQLo(void)",
	"Item::getQHi(void)",
	"Item::getNpcNum(void)",
	"Intrinsic00DF()",
	// 00E0
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Intrinsic00FA()",
	"Item::getQLo(void)",
	"Item::getCY(void)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	// 00F0
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Intrinsic00FA()",
	"Item::getQLo(void)",
	"Intrinsic00FA()",
	"Intrinsic00FB()",
	"Intrinsic00FC()",
	"Item::getQLo(void)",
	"Intrinsic00FE()",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	// 0100
	"Item::andStatus(void)",
	"Item::isOn(uint16)",
	"Npc::isDead(void)",
	"Intrinsic00BD()",
	"Item::getQHi(void)",
	"Intrinsic00DA()",
	"Intrinsic00FA()",
	"Item::getQLo(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::isOn(uint16)",
	"Item::getQHi(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getNpcNum(void)",
	"Item::getCY(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	// 0110
	"Item::isOn(uint16)",
	"Intrinsic0111()",
	"IntrinsicReturn0",
	"Npc::isDead(void)",
	"Intrinsic0088()",
	"Intrinsic00C1()",
	"Item::getQHi(void)",
	"Intrinsic00BD()",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Intrinsic011C()",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::AvatarStoleSomehting(uint16)",
	// 0120
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::getQ(void)",
	"Item::setQ(uint)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Item::andStatus(void)",
	"Item::getNpcNum(void)",
	"Intrinsic00C3()",
	"Item::andStatus(void)",
	// 0130
	"Item::getNpcNum(void)",
	"Intrinsic0131()",
	"Item::andStatus(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::andStatus(void)",
	"Camera::getY(void)",
	"Camera::getZ(void)",
	"Intrinsic0137()",
	"Intrinsic009C()",
	"Item::getTypeFlagCrusader(sint16)",
	"Item::getNpcNum(void)",
	"Item::hurl(sint16,sint16,sint16,sint16)",
	"Item::getCY(void)",
	"Item::getCZ(void)",
	"Item::setFrame(uint16)",
	"Intrinsic013F()",
	// 0140
	"Intrinsic0140()",
	"Intrinsic0141()",
	"Intrinsic0142()",
	"Npc::isDead(void)",
	"Intrinsic00FA()",
	"Intrinsic0145()",
	"Intrinsic0146()",
	"Intrinsic0147()",
	"Item::getNpcNum(void)",
	"Item::getQLo(void)",
	"Item::andStatus(void)",
	"Intrinsic014B()",
	"Intrinsic014C()",
	"Intrinsic014D()",
	"Intrinsic003C()",
	"Egg::getEggXRange(void)",
	// 0150
	"Intrinsic009C()",
	"Intrinsic0072()",
	"Item::setFrame(uint16)",
	"Intrinsic00C1()",
	"Intrinsic00C3()",
	"Intrinsic00C1()",
	"Item::isOn(uint16)",
	"Intrinsic00C3()",
	"Intrinsic00FA()",
	"Item::getQHi(void)",
	"Item::getQLo(void)",
	"Intrinsic00C1()",
	"Intrinsic00C3()",
	"Intrinsic015D()",
	0
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
	"func0A(sint16)",				// 0x0A
	"func0B(sint16)",				// 0x0B
	"combine()",					// 0x0C
	"func0D",						// 0x0D
	"calledFromAnim()",				// 0x0E
	"enterFastArea()",				// 0x0F

	"leaveFastArea()",				// 0x10
	"func11(sint16)",				// 0x11
	"justMoved()",					// 0x12
	"AvatarStoleSomething(uword)",	// 0x13
	"animGetHit()",					// 0x14
	"func15",						// 0x15
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

void ConvertUsecodeCrusader::readheader(IDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)
{
	uch.routines = read4(ucfile);     // routines
	uch.maxOffset = read4(ucfile);           // total code size,
	uch.offset = read4(ucfile)-20;    // code offset,
	uch.externTable = read4(ucfile); // extern symbol table offset
	uch.fixupTable = read4(ucfile);  // fixup table offset
	#ifdef DISASM_DEBUG
	con.Printf("Routines:\t%04X\n", uch.routines);
	con.Printf("MaxOffset:\t%04X\nOffset:\t\t%04X\n", uch.maxOffset, uch.offset);
	con.Printf("ExternTable:\t%04X\nFixupTable:\t%04X\n", uch.externTable, uch.fixupTable);
	#endif
	uch.maxOffset += 1;
	#ifdef DISASM_DEBUG
	con.Printf("Adjusted MaxOffset:\t%04X\n", uch.maxOffset);
	#endif
	curOffset = 1-uch.offset;
};

/*void ConvertUsecodeCrusader::readOp(TempOp &op, IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)
{

};*/

} // End of namespace Ultima8

#endif

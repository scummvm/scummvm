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

#include "scumm/scumm_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/insane/insane_rebel1.h"

namespace Scumm {

InsaneRebel1::InsaneRebel1(ScummEngine_v7 *scumm) : Insane(), _vm(scumm) {
	// Null out Insane base class pointers that the default constructor doesn't initialize
	_smush_roadrashRip = nullptr;
	_smush_roadrsh2Rip = nullptr;
	_smush_roadrsh3Rip = nullptr;
	_smush_goglpaltRip = nullptr;
	_smush_tovista1Flu = nullptr;
	_smush_tovista2Flu = nullptr;
	_smush_toranchFlu = nullptr;
	_smush_minedrivFlu = nullptr;
	_smush_minefiteFlu = nullptr;
	_smush_bensgoggNut = nullptr;
	_smush_bencutNut = nullptr;
	_smush_iconsNut = nullptr;
	_smush_icons2Nut = nullptr;
}

InsaneRebel1::~InsaneRebel1() {
}

void InsaneRebel1::procPreRendering(byte *renderBitmap) {
	// Stub: no pre-rendering for RA1 yet
}

void InsaneRebel1::procPostRendering(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, int32 curFrame, int32 maxFrame) {
	// Stub: no post-rendering for RA1 yet
}

void InsaneRebel1::procIACT(byte *renderBitmap, int32 codecparam, int32 setupsan12,
	int32 setupsan13, Common::SeekableReadStream &b, int32 size, int32 flags,
	int16 par1, int16 par2, int16 par3, int16 par4) {
	// RA1 does not use IACT chunks
}

void InsaneRebel1::procSKIP(int32 subSize, Common::SeekableReadStream &b) {
	// Stub
}

void InsaneRebel1::handleGameChunk(int32 subSize, Common::SeekableReadStream &b) {
	// GAME chunks contain gameplay data (opcodes for corridor, turret, etc.)
	// For now just log and skip
	if (subSize >= 4) {
		uint32 opcode = b.readUint32BE();
		debug(5, "InsaneRebel1::handleGameChunk: opcode=0x%02x size=%d", opcode >> 24, subSize);
	}
}

void InsaneRebel1::playLevel(int level) {
	Common::String filename = Common::String::format("LVL%d/L%dPLAY1L.ANM", level, level);
	debug(1, "InsaneRebel1::playLevel(%d): playing %s", level, filename.c_str());

	SmushPlayer *splayer = _vm->_splayer;
	splayer->play(filename.c_str(), 12);
}

} // End of namespace Scumm

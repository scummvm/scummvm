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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

namespace MacVenture {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK)

#define BASEGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT, GUIO0()}

static const ADGameDescription gameDescriptions[] = {
	BASEGAME("shadowgate", "Zojoi Rerelease", "Shadowgate.bin", "ebbfbcbf93938bd2900cb0c0213b19ad", 68974), // Zojoi Rerelease
	BASEGAME("deja_vu", "Zojoi Rerelease", "Déjà Vu.bin", "5e9f5a8e3c8eb29ed02b34ae5937354f", 69034), // Zojoi Rerelease
	AD_TABLE_END_MARKER
};
} // End of namespace MacVenture

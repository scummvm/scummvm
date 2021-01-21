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

namespace AGS {

static const PlainGameDescriptor GAME_NAMES[] = {
	{ "ags", "Adventure Game Studio Game" },

	{ "atotk", "A Tale Of Two Kingdoms" },
	{ "bcremake", "Black Cauldron Remake" },
	{ "blackwelllegacy", "The Blackwell Legacy" },
	{ "kq2agdi", "Kings Quest II - Romancing The Stones" },

	{ 0, 0 }
};

#define ENGLISH_ENTRY(ID, FILENAME, MD5, SIZE) { \
		{ \
			ID, \
			nullptr, \
			AD_ENTRY1s(FILENAME, MD5, SIZE), \
			Common::EN_ANY, \
			Common::kPlatformUnknown, \
			ADGF_NO_FLAGS, \
			GUIO1(GUIO_NOSPEECH) \
		} \
	}

static const AGSGameDescription GAME_DESCRIPTIONS[] = {
	ENGLISH_ENTRY("bcremake", "bc.exe", "0710e2ec71042617f565c01824f0cf3c", 7683255),
	ENGLISH_ENTRY("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42872046),
	ENGLISH_ENTRY("blackwelllegacy", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 18822697),
	ENGLISH_ENTRY("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12563246),	// 3.1

	{ AD_TABLE_END_MARKER }
};

} // namespace AGS

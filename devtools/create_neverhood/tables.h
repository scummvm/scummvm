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

static const uint32 hitRectListOffsets[] = {
	// Scene1001
	1, 0x004B4860,
	// Scene1002
	1, 0x004B4138,
	// Scene1201
	4, 0x004AEBD0,
	// Scene1705
	1, 0x004B69D8,
	0, 0
};

static const uint32 rectListOffsets[] = {
	// Scene1001
	1, 0x004B49F0,
	1, 0x004B4A00,
	// Scene1002
	3, 0x004B43A0,
	1, 0x004B4418,
	3, 0x004B43A0,
	// Scene1004
	1, 0x004B7C70,
	// Scene1201
	1, 0x004AEE58,
	1, 0x004AEDC8,
	1, 0x004AEE18,
	1, 0x004AED88,
	// Scene1401
	1, 0x004B6758,
	// Scene1402
	1, 0x004B0C48,
	1, 0x004B0C98,
	// Scene1403
	1, 0x004B1FF8,
	1, 0x004B2008,
	// Scene1404
	1, 0x004B8D80,
	// Scene1705
	1, 0x004B6B40,
	1, 0x004B6B30,
	0, 0
};

static const uint32 messageListOffsets[] = {
	// Scene1001
	1, 0x004B4888,
	2, 0x004B4898,
	1, 0x004B4970,
	1, 0x004B4890,
	3, 0x004B4910,
	5, 0x004B4938,
	1, 0x004B4960,
	4, 0x004B48A8,
	3, 0x004B48C8,
	// Scene1002
	1, 0x004B4270,
	1, 0x004B4478,
	3, 0x004B4298,
	1, 0x004B4470,
	4, 0x004B4428,
	5, 0x004B4448,
	1, 0x004B44B8,
	2, 0x004B44A8,
	1, 0x004B44A0,
	2, 0x004B43D0,
	4, 0x004B4480,
	2, 0x004B41E0,
	5, 0x004B4148,
	// Scene1004
	3, 0x004B7BF0,
	2, 0x004B7C08,
	1, 0x004B7C18,
	2, 0x004B7C20,
	// Scene1201
	1, 0x004AEC08,
	2, 0x004AEC10,
	2, 0x004AEC20,
	2, 0x004AEC30,
	4, 0x004AEC90,
	2, 0x004AECB0,
	2, 0x004AECC0,
	5, 0x004AECF0,
	2, 0x004AECD0,
	2, 0x004AECE0,
	2, 0x004AED38,
	// Scene1401
	1, 0x004B65C8,
	1, 0x004B65D0,
	1, 0x004B65D8,
	1, 0x004B65E8,
	3, 0x004B6670,
	4, 0x004B6690,
	1, 0x004B66B0,
	3, 0x004B6658,
	2, 0x004B65F0,
	// Scene1402
	1, 0x004B0B48,
	1, 0x004B0B50,
	1, 0x004B0B58,
	1, 0x004B0B60,
	2, 0x004B0B68,
	3, 0x004B0BB8,
	3, 0x004B0BD0,
	// Scene1403
	1, 0x004B1F18,
	1, 0x004B1F20,
	3, 0x004B1F70,
	2, 0x004B1FA8,
	4, 0x004B1F88,
	3, 0x004B1F58,
	2, 0x004B1F28,
	2, 0x004B1FB8,
	// Scene1404
	1, 0x004B8C28,
	1, 0x004B8C30,
	1, 0x004B8C38,
	1, 0x004B8D28,
	3, 0x004B8CB8,
	2, 0x004B8C40,
	6, 0x004B8CE8,
	3, 0x004B8CA0,
	2, 0x004B8CD0,
	2, 0x004B8D18,
	// Scene1705
	1, 0x004B69E8,
	2, 0x004B6A08,
	4, 0x004B6AA0,
	2, 0x004B6A18,
	1, 0x004B69F0,
	2, 0x004B6AC0,
	0, 0
};

static const uint32 navigationListOffsets[] = {
	// Module1700
	2, 0x004AE8B8,
	3, 0x004AE8E8,
	// Module1800
	4, 0x004AFD38,
	1, 0x004AFD98,
	2, 0x004AFDB0,
	4, 0x004AFDE0,
	2, 0x004AFE40,
	// Module2300
	2, 0x004B67B8,
	6, 0x004B67E8,
	2, 0x004B6878,
	3, 0x004B68F0,
	3, 0x004B68A8,
	0, 0
};

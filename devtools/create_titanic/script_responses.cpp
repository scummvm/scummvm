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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "file.h"
#include "script_responses.h"

static const int DESKBOT_RESPONSES[128][5] = {
	{ MKTAG('P', 'K', 'U', 'P'), 240573, 0, 0, 0 },
	{ MKTAG('S', 'E', 'X', '1'), 240573, 0, 0, 0 },
	{ MKTAG('R', 'E', 'S', '1'), 240354, 0, 0, 0 },
	{ MKTAG('R', 'E', 'S', '2'), 240547, 0, 0, 0 },
	{ MKTAG('S', 'W', 'E', 'R'), 240578, 0, 0, 0 },
	{ MKTAG('B', 'Y', 'Z', 'A'), 241173, 0, 0, 0 },
	{ MKTAG('S', 'A', 'S', 'S'), 240986, 0, 0, 0 },
	{ MKTAG('S', 'H', 'M', 'S'), 240453, 0, 0, 0 },
	{ MKTAG('F', 'O', 'O', 'D'), 240849, 0, 0, 0 },
	{ MKTAG('J', 'F', 'O', 'D'), 240849, 0, 0, 0 },

	{ MKTAG('C', 'H', 'S', 'E'), 240849, 0, 0, 0 },
	{ MKTAG('A', 'C', 'T', 'R'), 240654, 0, 0, 0 },
	{ MKTAG('A', 'C', 'T', 'S'), 240655, 0, 0, 0 },
	{ MKTAG('M', 'U', 'S', 'I'), 240681, 240849, 0, 0 },
	{ MKTAG('S', 'A', 'N', 'G'), 240681, 240657, 0, 0 },
	{ MKTAG('S', 'O', 'A', 'P'), 240681, 0, 0, 0 },
	{ MKTAG('T', 'V', 'S', 'H'), 240681, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'I'), 240657, 0, 0, 0 },
	{ MKTAG('A', 'U', 'T', 'H'), 240657, 0, 0, 0 },
	{ MKTAG('C', 'O', 'M', 'D'), 240657, 240785, 0, 0 },

	{ MKTAG('C', 'O', 'O', 'K'), 240657, 0, 0, 0 },
	{ MKTAG('C', 'O', 'P', 'S'), 240657, 0, 0, 0 },
	{ MKTAG('H', 'E', 'R', 'O'), 240657, 0, 0, 0 },
	{ MKTAG('H', 'O', 'S', 'T'), 240657, 0, 0, 0 },
	{ MKTAG('P', 'T', 'I', 'C'), 240657, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'T'), 240657, 0, 0, 0 },
	{ MKTAG('E', 'A', 'R', 'T'), 240728, 0, 0, 0 },
	{ MKTAG('P', 'L', 'A', 'N'), 240728, 0, 0, 0 },
	{ MKTAG('F', 'I', 'L', 'M'), 240939, 0, 0, 0 },
	{ MKTAG('F', 'I', 'S', 'H'), 240437, 0, 0, 0 },

	{ MKTAG('H', 'H', 'G', 'Q'), 241065, 240453, 0, 0 },
	{ MKTAG('L', 'I', 'Q', 'D'), 241167, 0, 0, 0 },
	{ MKTAG('P', 'H', 'I', 'L'), 240607, 0, 0, 0 },
	{ MKTAG('S', 'I', 'C', 'K'), 241170, 0, 0, 0 },
	{ MKTAG('T', 'W', 'A', 'T'), 240975, 0, 0, 0 },
	{ MKTAG('H', 'A', 'H', 'A'), 240785, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'T'), 240968, 241617, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'E'), 240967, 241616, 0, 0 },
	{ MKTAG('S', 'L', 'O', 'W'), 241614, 0, 0, 0 },
	{ MKTAG('T', 'H', 'A', 'T'), 240760,  241615, 0, 0 },

	{ MKTAG('T', 'D', 'U', 'P'), 241161, 241618, 0, 0 },
	{ MKTAG('T', 'I', 'T', 'A'), 241619, 0, 0, 0 },
	{ MKTAG('C', 'S', 'P', 'Y'), 241620, 0, 0, 0 },
	{ MKTAG('M', 'I', 'N', 'S'), 241621, 0, 0, 0 },
	{ MKTAG('M', 'C', 'P', 'Y'), 241622, 0, 0, 0 },
	{ MKTAG('D', 'N', 'C', 'E'), 241623, 0, 0, 0 },
	{ MKTAG('N', 'A', 'U', 'T'), 241624, 0, 0, 0 },
	{ MKTAG('A', 'D', 'V', 'T'), 240939, 241622, 0, 0 },
	{ MKTAG('A', 'N', 'S', 'W'), 240453, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'Y'), 240658, 0, 0, 0 },

	{ MKTAG('B', 'A', 'R', 'T'), 240491, 0, 0, 0 },
	{ MKTAG('B', 'A', 'R', '3'), 240610, 0, 0, 0 },
	{ MKTAG('B', 'A', 'R', 'K'), 240768, 0, 0, 0 },
	{ MKTAG('B', 'A', 'R', 'U'), 240768, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '1'), 240940, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '2'), 240591, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '3'), 240775, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '4'), 240558, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '5'), 240336, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '6'), 240759, 240760, 0, 0 },

	{ MKTAG('B', 'E', 'L', '7'), 240726, 0, 0, 0 },
	{ MKTAG('B', 'L', 'F', '1'), 241652, 0, 0, 0 },
	{ MKTAG('B', 'L', 'F', '2'), 240939, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '1'), 240654, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '2'), 240654, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '3'), 240654, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '4'), 240655, 0, 0, 0 },
	{ MKTAG('B', 'L', 'R', '1'), 240654, 0, 0, 0 },
	{ MKTAG('B', 'L', 'R', '2'), 240655, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '1'), 240718, 0, 0, 0 },

	{ MKTAG('B', 'L', 'T', '2'), 240681, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '3'), 240655, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '4'), 240664, 0, 0, 0 },
	{ MKTAG('B', 'T', 'T', '5'), 240681, 0, 0, 0 },
	{ MKTAG('B', 'O', 'D', 'Y'), 240596, 0, 0, 0 },
	{ MKTAG('B', 'O', 'Y', 'S'), 240654, 0, 0, 0 },
	{ MKTAG('B', 'R', 'N', 'D'), 240939, 241622, 0, 0 },
	{ MKTAG('C', 'L', 'U', 'B'), 241675, 240681, 241623, 0 },
	{ MKTAG('C', 'M', 'N', 'T'), 240849, 0, 0, 0 },
	{ MKTAG('C', 'R', 'I', 'M'), 241096, 240725, 240729, 0 },

	{ MKTAG('D', 'C', 'T', 'R'), 240725, 0, 0, 0 },
	{ MKTAG('D', 'O', 'R', '2'), 241405, 241404, 241403, 241402 },
	{ MKTAG('D', 'B', 'U', 'G'), 240922, 240931, 0, 0 },
	{ MKTAG('F', 'A', 'M', 'E'), 240726, 0, 0, 0 },
	{ MKTAG('F', 'A', 'S', 'H'), 241172, 0, 0, 0 },
	{ MKTAG('F', 'A', 'U', 'N'), 240939, 0, 0, 0 },
	{ MKTAG('F', 'L', 'O', 'R'), 240825, 0, 0, 0 },
	{ MKTAG('F', 'U', 'L', 'N'), 240864, 241072, 0, 0 },
	{ MKTAG('G', 'I', 'R', 'L'), 241144, 0, 0, 0 },
	{ MKTAG('H', 'B', 'B', 'Y'), 241144, 0, 0, 0 },

	{ MKTAG('H', 'H', 'L', 'D'), 241144, 0, 0, 0 },
	{ MKTAG('H', 'O', 'M', 'E'), 240844, 240626, 0, 0 },
	{ MKTAG('I', 'S', 'H', 'E'), 240731, 0, 0, 0 },
	{ MKTAG('J', 'N', 'A', 'M'), 240785, 240657, 0, 0 },
	{ MKTAG('J', 'O', 'K', 'E'), 240785, 0, 0, 0 },
	{ MKTAG('K', 'N', 'O', 'B'), 240657, 0, 0, 0 },
	{ MKTAG('K', 'P', 'L', 'C'), 240844, 240626, 0, 0 },
	{ MKTAG('L', 'I', 'F', '3'), 240722, 0, 0, 0 },
	{ MKTAG('L', 'I', 'T', 'E'), 240785, 0, 0, 0 },
	{ MKTAG('L', 'I', 'T', 'R'), 241404, 241405, 241403, 241406 },

	{ MKTAG('M', 'A', 'D', '1'), 241124, 240971, 241615, 0 },
	{ MKTAG('M', 'A', 'D', '4'), 241341, 0, 0, 0 },
	{ MKTAG('M', 'A', 'D', '6'), 240860, 241114, 0, 0 },
	{ MKTAG('M', 'A', 'G', 'S'), 241404, 241405, 241403, 241407 },
	{ MKTAG('M', 'L', 'T', 'Y'), 240718, 240719, 0, 0 },
	{ MKTAG('N', 'I', 'K', 'E'), 241622, 0, 0, 0 },
	{ MKTAG('N', 'I', 'K', 'N'), 240785, 0, 0, 0 },
	{ MKTAG('N', 'P', 'L', 'C'), 240844, 240626, 0, 0 },
	{ MKTAG('O', 'R', 'D', '1'), 240695, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '2'), 240744, 240650, 0, 0 },

	{ MKTAG('O', 'R', 'D', '3'), 240647, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '4'), 240647, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '5'), 241191, 0, 0, 0 },
	{ MKTAG('P', 'G', 'R', 'P'), 240725, 0, 0, 0 },
	{ MKTAG('P', 'L', 'A', 'C'), 240728, 0, 0, 0 },
	{ MKTAG('R', 'C', 'K', 'T'), 241070, 241161, 0, 0 },
	{ MKTAG('S', 'F', 'S', 'F'), 241172, 0, 0, 0 },
	{ MKTAG('S', 'P', 'R', 'T'), 241172, 0, 0, 0 },
	{ MKTAG('S', 'U', 'C', '1'), 240467, 0, 0, 0 },
	{ MKTAG('T', 'E', 'A', 'M'), 241172, 0, 0, 0 },

	{ MKTAG('T', 'L', 'A', ' '), 240727, 240658, 0, 0 },
	{ MKTAG('T', 'O', 'Y', 'S'), 240607, 240606, 0, 0 },
	{ MKTAG('T', 'R', 'A', '2'), 240611, 0, 0, 0 },
	{ MKTAG('T', 'R', 'A', '3'), 240611, 0, 0, 0 },
	{ MKTAG('W', 'E', 'A', 'P'), 240939, 0, 0, 0 },
	{ MKTAG('W', 'E', 'A', 'T'), 241093, 241094, 241095, 0 },
	{ MKTAG('W', 'T', 'H', 'R'), 241093, 241094, 241095, 0 },
	{ MKTAG('W', 'W', 'E', 'B'), 241172, 0, 0, 0 }
};

static const int BELLBOT_RESPONSES[130][5] = {
	{ MKTAG('A', 'C', 'T', 'R'), 200505, 0, 0, 0 },
	{ MKTAG('A', 'C', 'T', 'S'), 200505, 0, 0, 0 },
	{ MKTAG('F', 'A', 'M', 'E'), 200532, 200585, 0, 0 },
	{ MKTAG('A', 'D', 'V', 'T'), 200506, 0, 0, 0 },
	{ MKTAG('L', 'I', 'Q', 'D'), 200507, 200527, 0, 0 },
	{ MKTAG('F', 'A', 'U', 'N'), 200511, 0, 0, 0 },
	{ MKTAG('H', 'B', 'B', 'Y'), 200514, 0, 0, 0 },
	{ MKTAG('N', 'I', 'K', 'E'), 200514, 0, 0, 0 },
	{ MKTAG('B', 'R', 'N', 'D'), 200514, 0, 0, 0 },
	{ MKTAG('P', 'G', 'R', 'P'), 200514, 0, 0, 0 },
	{ MKTAG('R', 'C', 'K', 'T'), 200514, 0, 0, 0 },
	{ MKTAG('D', 'R', 'U', 'G'), 200528, 0, 0, 0 },
	{ MKTAG('F', 'A', 'S', 'H'), 200533, 0, 0, 0 },
	{ MKTAG('T', 'O', 'Y', 'S'), 200533, 0, 0, 0 },
	{ MKTAG('F', 'O', 'O', 'D'), 202270, 0, 0, 0 },
	{ MKTAG('J', 'F', 'O', 'D'), 202270, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'I'), 200538, 0, 0, 0 },
	{ MKTAG('A', 'R', 'T', 'Y'), 200538, 0, 0, 0 },
	{ MKTAG('L', 'I', 'T', 'R'), 200538, 0, 0, 0 },
	{ MKTAG('C', 'R', 'I', 'M'), 200538, 0, 0, 0 },
	{ MKTAG('C', 'S', 'P', 'Y'), 200538, 0, 0, 0 },
	{ MKTAG('W', 'E', 'A', 'T'), 200538, 0, 0, 0 },
	{ MKTAG('M', 'U', 'S', 'I'), 200539, 0, 0, 0 },
	{ MKTAG('S', 'O', 'N', 'G'), 200539, 0, 0, 0 },
	{ MKTAG('F', 'I', 'L', 'M'), 200534, 0, 0, 0 },
	{ MKTAG('B', 'L', 'F', '1'), 200535, 0, 0, 0 },
	{ MKTAG('B', 'L', 'F', '2'), 200536, 0, 0, 0 },
	{ MKTAG('M', 'A', 'G', 'S'), 200542, 0, 0, 0 },
	{ MKTAG('P', 'H', 'I', 'L'), 200557, 0, 0, 0 },
	{ MKTAG('P', 'L', 'A', 'N'), 200562, 0, 0, 0 },
	{ MKTAG('E', 'A', 'R', 'T'), 200562, 202252, 0, 0 },
	{ MKTAG('P', 'L', 'A', 'C'), 200562, 202252, 0, 0 },
	{ MKTAG('F', 'L', 'O', 'R'), 200570, 0, 0, 0 },
	{ MKTAG('P', 'T', 'I', 'C'), 200571, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '1'), 200577, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '2'), 200575, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '3'), 200576, 0, 0, 0 },
	{ MKTAG('B', 'L', 'P', '4'), 200578, 0, 0, 0 },
	{ MKTAG('K', 'N', 'O', 'B'), 200579, 0, 0, 0 },
	{ MKTAG('B', 'L', 'R', '1'), 200580, 0, 0, 0 },
	{ MKTAG('B', 'L', 'R', '2'), 200581, 0, 0, 0 },
	{ MKTAG('S', 'E', 'X', '1'), 200582, 0, 0, 0 },
	{ MKTAG('S', 'P', 'R', 'T'), 200584, 0, 0, 0 },
	{ MKTAG('T', 'E', 'A', 'M'), 200584, 0, 0, 0 },
	{ MKTAG('H', 'E', 'R', 'O'), 200585, 0, 0, 0 },
	{ MKTAG('T', 'W', 'A', 'T'), 200588, 0, 0, 0 },
	{ MKTAG('S', 'W', 'E', 'R'), 200590, 200336, 0, 0 },
	{ MKTAG('T', 'R', 'A', '2'), 200594, 0, 0, 0 },
	{ MKTAG('T', 'R', 'A', '3'), 200594, 0, 0, 0 },
	{ MKTAG('T', 'V', 'S', 'H'), 200595, 0, 0, 0 },
	{ MKTAG('S', 'O', 'A', 'P'), 200595, 0, 0, 0 },
	{ MKTAG('C', 'O', 'M', 'D'), 200595, 0, 0, 0 },
	{ MKTAG('C', 'O', 'O', 'K'), 200595, 0, 0, 0 },
	{ MKTAG('C', 'O', 'P', 'S'), 200595, 0, 0, 0 },
	{ MKTAG('D', 'C', 'T', 'R'), 200595, 0, 0, 0 },
	{ MKTAG('S', 'F', 'S', 'F'), 200595, 0, 0, 0 },
	{ MKTAG('H', 'O', 'S', 'T'), 200595, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '1'), 200596, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '2'), 200597, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '3'), 200598, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '4'), 200599, 0, 0, 0 },
	{ MKTAG('B', 'L', 'T', '5'), 200600, 0, 0, 0 },
	{ MKTAG('W', 'E', 'A', 'P'), 200601, 0, 0, 0 },
	{ MKTAG('A', 'U', 'T', 'H'), 200605, 0, 0, 0 },
	{ MKTAG('H', 'H', 'L', 'D'), 200536, 0, 0, 0 },
	{ MKTAG('W', 'W', 'E', 'B'), 200608, 0, 0, 0 },
	{ MKTAG('M', 'L', 'T', 'Y'), 200608, 0, 0, 0 },
	{ MKTAG('P', 'K', 'U', 'P'), 200067, 0, 0, 0 },
	{ MKTAG('S', 'U', 'C', '1'), 200067, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '1'), 200684, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '2'), 200887, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '3'), 200610, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '4'), 200015, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '5'), 200043, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '6'), 200333, 0, 0, 0 },
	{ MKTAG('B', 'E', 'L', '7'), 200884, 0, 0, 0 },
	{ MKTAG('H', 'H', 'G', 'Q'), 200516, 0, 0, 0 },
	{ MKTAG('C', 'H', 'A', 'N'), 200961, 0, 0, 0 },
	{ MKTAG('B', 'Y', 'Z', 'A'), 201271, 0, 0, 0 },
	{ MKTAG('F', 'I', 'S', 'H'), 201242, 0, 0, 0 },
	{ MKTAG('S', 'A', 'S', 'S'), 201256, 0, 0, 0 },
	{ MKTAG('S', 'I', 'C', 'K'), 201704, 0, 0, 0 },
	{ MKTAG('N', 'H', 'R', 'O'), 201704, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'E'), 201675, 0, 0, 0 },
	{ MKTAG('S', 'C', 'I', 'T'), 201676, 0, 0, 0 },
	{ MKTAG('T', 'D', 'V', 'P'), 200490, 0, 0, 0 },
	{ MKTAG('H', 'A', 'H', 'A'), 200950, 0, 0, 0 },
	{ MKTAG('S', 'U', 'C', '1'), 200582, 0, 0, 0 },
	{ MKTAG('T', 'L', 'A', ' '), 201243, 0, 0, 0 },
	{ MKTAG('J', 'O', 'K', 'E'), 201244, 0, 0, 0 },
	{ MKTAG('C', 'H', 'S', 'E'), 202270, 0, 0, 0 },
	{ MKTAG('C', 'L', 'U', 'B'), 201654, 0, 0, 0 },
	{ MKTAG('S', 'L', 'O', 'W'), 201877, 0, 0, 0 },
	{ MKTAG('T', 'H', 'R', 'T'), 201238, 201269, 201982, 200336 },
	{ MKTAG('A', 'N', 'S', 'W'), 200139, 0, 0, 0 },
	{ MKTAG('B', 'A', 'R', '1'), 202051, 200473, 200229, 0 },
	{ MKTAG('B', 'A', 'R', 'K'), 200510, 201326, 0, 0 },
	{ MKTAG('B', 'A', 'R', 'U'), 200510, 201326, 0, 0 },
	{ MKTAG('B', 'O', 'D', 'Y'), 201704, 0, 0, 0 },
	{ MKTAG('C', 'M', 'N', 'T'), 202270, 0, 0, 0 },
	{ MKTAG('D', 'E', 'S', '2'), 201529, 0, 0, 0 },
	{ MKTAG('D', 'N', 'C', 'E'), 200952, 200953, 200960, 0 },
	{ MKTAG('D', 'O', 'R', '2'), 200372, 0, 0, 0 },
	{ MKTAG('F', 'U', 'L', 'N'), 202035, 202044, 0, 0 },
	{ MKTAG('H', 'O', 'M', 'E'), 202252, 0, 0, 0 },
	{ MKTAG('I', 'S', 'H', 'E'), 201609, 0, 0, 0 },
	{ MKTAG('J', 'N', 'A', 'M'), 202035, 0, 0, 0 },
	{ MKTAG('L', 'I', 'F', '1'), 201704, 0, 0, 0 },
	{ MKTAG('L', 'I', 'F', '2'), 201704, 0, 0, 0 },
	{ MKTAG('L', 'I', 'F', '3'), 201704, 0, 0, 0 },
	{ MKTAG('M', 'A', 'D', '1'), 201238, 0, 0, 0 },
	{ MKTAG('M', 'A', 'D', '4'), 200292, 0, 0, 0 },
	{ MKTAG('M', 'A', 'D', '5'), 200140, 0, 0, 0 },
	{ MKTAG('M', 'A', 'D', '6'), 200968, 0, 0, 0 },
	{ MKTAG('M', 'C', 'P', 'Y'), 200514, 0, 0, 0 },
	{ MKTAG('M', 'I', 'N', 'S'), 200541, 0, 0, 0 },
	{ MKTAG('N', 'A', 'U', 'T'), 200529, 0, 0, 0 },
	{ MKTAG('N', 'O', 'N', 'O'), 200127, 0, 0, 0 },
	{ MKTAG('N', 'P', 'L', 'C'), 200234, 201625, 0, 0 },
	{ MKTAG('O', 'R', 'D', '1'), 200473, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '2'), 200473, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '3'), 200473, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '4'), 200473, 0, 0, 0 },
	{ MKTAG('O', 'R', 'D', '5'), 200473, 0, 0, 0 },
	{ MKTAG('S', 'U', 'C', '2'), 200024, 0, 0, 0 },
	{ MKTAG('S', 'U', 'M', 'S'), 200139, 0, 0, 0 },
	{ MKTAG('T', 'I', 'T', 'A'), 200606, 0, 0, 0 },
	{ MKTAG('W', 'T', 'H', 'R'), 201094, 201097, 201092, 0 },
	{ MKTAG('Y', 'E', 'S', 'S'), 201525, 201529, 0, 0 },
};

void writeScriptTags(const char *name, const int *tags, uint count) {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < count * 5; ++idx, ++tags)
		outputFile.writeLong(*tags);
	
	writeEntryHeader(name, dataOffset, count * 5 * 4);
	dataOffset += count * 5 * 4;
}

void writeAllScriptTags() {
	writeScriptTags("Responses/Deskbot", &DESKBOT_RESPONSES[0][0], 128);
	writeScriptTags("Responses/Bellbot", &BELLBOT_RESPONSES[0][0], 130);
}
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
#include "script_ranges.h"

static uint BARBOT_RANGE1[] = { 250062, 250064, 250065, 250066, 250067, 250068, 250069, 250070, 250071, 250063, 0 };
static uint BARBOT_RANGE2[] = { 250200, 250201, 250202, 250203, 250204, 250205, 250206, 250207, 0 };
static uint BARBOT_RANGE3[] = { 250160, 250161, 250162, 250163, 0 };
static uint BARBOT_RANGE4[] = { 250175, 250176, 250177, 250178, 250179, 250180, 250181, 250182, 250183, 0 };
static uint BARBOT_RANGE5[] = { 250114, 250004, 0 };
static uint BARBOT_RANGE6[] = { 250042, 250009, 250010, 250011, 251692, 251693, 251694, 0 };
static uint BARBOT_RANGE7[] = { 250012, 250111, 0 };
static uint BARBOT_RANGE8[] = { 250013, 250119, 0 };
static uint BARBOT_RANGE9[] = { 250027, 250126, 250055, 0 };
static uint BARBOT_RANGE10[] = { 250032, 250130, 0 };
static uint BARBOT_RANGE11[] = { 250038, 250134, 0 };
static uint BARBOT_RANGE12[] = { 250039, 250135, 250050, 0 };
static uint BARBOT_RANGE13[] = { 250016, 250120, 250136, 250145, 250143, 0 };
static uint BARBOT_RANGE14[] = { 250040, 250138, 250139, 0 };
static uint BARBOT_RANGE15[] = { 250037, 250133, 250617, 0 };
static uint BARBOT_RANGE16[] = { 250031, 250129, 0 };
static uint BARBOT_RANGE17[] = { 250036, 250132, 0 };
static uint BARBOT_RANGE18[] = { 250035, 250137, 0 };
static uint BARBOT_RANGE19[] = { 250021, 250123, 0 };
static uint BARBOT_RANGE20[] = { 250006, 250117, 0 };
static uint BARBOT_RANGE21[] = { 250024, 250124, 0 };
static uint BARBOT_RANGE22[] = { 250002, 250113, 0 };
static uint BARBOT_RANGE23[] = { 250152, 250153, 250079, 250194, 250149, 250151, 250150,
250208, 250209, 250157, 250195, 250076, 250156, 250074,
250075, 250080, 250192, 250109, 250159, 250110, 250148,
250053, 250155, 250197, 250072, 250077, 250001, 250154,
250196, 250198, 250199, 250158, 250073, 0 };
static uint BARBOT_RANGE24[] = { 251831, 251832, 251833, 251834, 251835, 250617, 0 };
static uint BARBOT_RANGE25[] = { 250033, 251838, 0 };
static uint BARBOT_RANGE26[] = { 250014, 251827, 251781, 250617, 0 };
static uint BARBOT_RANGE27[] = { 251855, 251856, 0 };
static uint BARBOT_RANGE28[] = { 251852, 251853, 0 };
static uint BARBOT_RANGE29[] = { 251250, 251261, 251272, 251277, 251278, 251279, 251280,
251281, 251282, 251251, 251252, 251253, 251254, 251255,
251256, 251257, 251258, 251259, 251260, 251262, 251263,
251264, 251265, 251266, 251267, 251268, 251269, 251270,
251271, 251273, 251274, 251275, 251276, 0 };
static uint BARBOT_RANGE30[] = { 251283, 251284, 251285, 251286, 0 };
static uint BARBOT_RANGE31[] = { 250527, 250530, 250531, 250532, 250533, 250534, 250535,
250536, 250537, 250528, 250529, 0 };
static uint BARBOT_RANGE32[] = { 250593, 250594, 0 };
static uint BARBOT_RANGE33[] = { 250263, 250264, 250265, 250266, 0 };
static uint BARBOT_RANGE34[] = { 250227, 250228, 250229, 250230, 0 };
static uint BARBOT_RANGE35[] = { 250239, 250240, 250241, 250242, 250243, 0 };
static uint BARBOT_RANGE36[] = { 250507, 250222, 250678, 250588, 0 };
static uint BARBOT_RANGE37[] = { 250365, 250366, 250367, 250368, 0 };
static uint BARBOT_RANGE38[] = { 250936, 250937, 250938, 0 };
static uint BARBOT_RANGE39[] = { 250610, 250611, 0 };
static uint BARBOT_RANGE40[] = { 250082, 250093, 250102, 250104, 250105, 250106, 250107,
250108, 250091, 250092, 250094, 250095, 250096, 250097,
250098, 250099, 250100, 250101, 251700, 251701, 251702,
251703, 251704, 251862, 250617, 250268, 250272, 0 };
static uint BARBOT_RANGE41[] = { 250584, 250585, 0 };
static uint BARBOT_RANGE42[] = { 250579, 251142, 0 };
static uint BARBOT_RANGE43[] = { 250577, 250578, 0 };
static uint BARBOT_RANGE44[] = { 250244, 250245, 250246, 250247, 250248, 250249, 250250,
250251, 250252, 250253, 250254, 250255, 250256, 250257,
250258, 250259, 250260, 250261, 250262, 0 };
static uint BARBOT_RANGE45[] = { 250286, 250288, 250289, 250290, 250291, 250292, 250293,
250294, 250295, 250287, 0 };
static uint BARBOT_RANGE46[] = { 250296, 250299, 250300, 250301, 250302, 250303, 250304,
250305, 250306, 250297, 250298, 0 };
static uint BARBOT_RANGE47[] = { 250307, 250309, 250310, 250311, 250312, 250313, 250314,
250315, 250316, 0 };
static uint BARBOT_RANGE48[] = { 251682, 251683, 251684, 251685, 251686, 251687, 251688,
251689, 250756, 250757, 250758, 250759, 0 };
static uint BARBOT_RANGE49[] = { 250738, 250742, 250743, 250744, 250745, 250746, 250747,
250748, 250749, 250739, 250740, 250741, 0 };
static uint BARBOT_RANGE50[] = { 250659, 250660, 250661, 250379, 0 };
static uint BARBOT_RANGE51[] = { 251761, 251762, 251763, 251764, 0 };
static uint BARBOT_RANGE52[] = { 251754, 251755, 251756, 251757, 251758, 251759, 0 };
static uint BARBOT_RANGE53[] = { 250482, 250504, 250424, 250463, 250466, 250467, 250468,
250478, 250501, 250502, 250503, 250506, 250413, 251014,
250614, 250756, 250758, 250759, 250223, 250737, 250658,
251027, 250633, 250935, 250237, 251618, 0 };
static uint BARBOT_RANGE54[] = { 250504, 250434, 250436, 250466, 250467, 250468, 250469,
250470, 250472, 250501, 250502, 250503, 250505, 250413,
251681, 250756, 250758, 250759, 250223, 251027, 250633,
250935, 250237, 251618, 250371, 0 };
static uint BARBOT_RANGE55[] = { 250952, 250953, 0 };
static uint BARBOT_RANGE56[] = { 251777, 250951, 0 };
static uint BARBOT_RANGE57[] = { 251871, 251877, 251878, 251879, 251880, 251883, 251884,
251872, 251873, 0 };
static uint BARBOT_RANGE58[] = { 250228, 250236, 250258, 250259, 250378, 250465, 250536,
251016, 251048, 251068, 0 };
static uint BARBOT_RANGE59[] = { 250141, 250378, 251048, 0 };
static uint BARBOT_RANGE60[] = { 251621, 251622, 251623, 251624, 251625, 251626, 0 };
static uint BARBOT_RANGE61[] = { 251650, 251651, 0 };
static uint BARBOT_RANGE62[] = { 251305, 251306, 251307, 251308, 0 };
static uint BARBOT_RANGE63[] = { 251836, 251890, 251891, 251892, 0 };
static uint BARBOT_RANGE64[] = { 250760, 251246, 251156, 251335, 251510, 251059, 251097,
251136, 250374, 250375, 250376, 250377, 251015, 251016,
251017, 251018, 0 };
static uint BARBOT_RANGE65[] = { 250899, 250906, 250948, 250713, 250690, 0 };
static uint BARBOT_RANGE66[] = { 250906, 250948, 250713, 250899, 250690, 0 };
static uint BARBOT_RANGE67[] = { 250949, 250713, 250711, 250152, 250153, 250690, 250906, 0 };
static uint BARBOT_RANGE68[] = { 251815, 250711, 0 };
static uint BARBOT_RANGE69[] = { 251829, 250711, 0 };
static uint BARBOT_RANGE70[] = { 251779, 250712, 0 };

#define BARBOT_RANGE_COUNT 70
static ScriptRange BARBOT_RANGES[70] = {
	{ 250062, BARBOT_RANGE1, false, false },
	{ 250200, BARBOT_RANGE2, false, false },
	{ 250160, BARBOT_RANGE3, false, false },
	{ 250175, BARBOT_RANGE4, false, false },
	{ 250004, BARBOT_RANGE5, false, false },
	{ 250042, BARBOT_RANGE6, true, false },
	{ 250012, BARBOT_RANGE7, false, false },
	{ 250013, BARBOT_RANGE8, false, false },
	{ 250027, BARBOT_RANGE9, false, false },
	{ 250032, BARBOT_RANGE10, false, false },
	{ 250038, BARBOT_RANGE11, false, false },
	{ 250039, BARBOT_RANGE12, false, false },
	{ 250016, BARBOT_RANGE13, false, false },
	{ 250040, BARBOT_RANGE14, false, false },
	{ 250037, BARBOT_RANGE15, false, false },
	{ 250031, BARBOT_RANGE16, false, false },
	{ 250036, BARBOT_RANGE17, false, false },
	{ 250035, BARBOT_RANGE18, false, false },
	{ 250021, BARBOT_RANGE19, false, false },
	{ 250006, BARBOT_RANGE20, false, false },

	{ 250024, BARBOT_RANGE21, false, false },
	{ 250002, BARBOT_RANGE22, false, false },
	{ 250210, BARBOT_RANGE23, false, false },
	{ 251831, BARBOT_RANGE24, false, false },
	{ 250033, BARBOT_RANGE25, false, false },
	{ 250014, BARBOT_RANGE26, false, false },
	{ 251855, BARBOT_RANGE27, false, false },
	{ 251852, BARBOT_RANGE28, false, false },
	{ 251250, BARBOT_RANGE29, false, false },
	{ 251283, BARBOT_RANGE30, false, false },
	{ 250527, BARBOT_RANGE31, false, false },
	{ 250593, BARBOT_RANGE32, false, false },
	{ 250263, BARBOT_RANGE33, false, false },
	{ 250227, BARBOT_RANGE34, false, false },
	{ 250239, BARBOT_RANGE35, false, false },
	{ 250507, BARBOT_RANGE36, false, false },
	{ 250365, BARBOT_RANGE37, false, false },
	{ 250936, BARBOT_RANGE38, false, false },
	{ 250610, BARBOT_RANGE39, false, false },
	{ 250082, BARBOT_RANGE40, true, false },

	{ 250584, BARBOT_RANGE41, false, false },
	{ 250579, BARBOT_RANGE42, false, false },
	{ 250577, BARBOT_RANGE43, false, false },
	{ 250244, BARBOT_RANGE44, true, false },
	{ 250286, BARBOT_RANGE45, true, false },
	{ 250296, BARBOT_RANGE46, true, false },
	{ 250307, BARBOT_RANGE47, true, false },
	{ 251682, BARBOT_RANGE48, true, false },
	{ 250738, BARBOT_RANGE49, true, false },
	{ 250659, BARBOT_RANGE50, true, false },
	{ 251761, BARBOT_RANGE51, false, false },
	{ 251754, BARBOT_RANGE52, false, false },
	{ 251896, BARBOT_RANGE53, true, false },
	{ 251897, BARBOT_RANGE54, true, false },
	{ 250952, BARBOT_RANGE55, false, false },
	{ 251777, BARBOT_RANGE56, false, false },
	{ 251871, BARBOT_RANGE57, true, false },
	{ 250140, BARBOT_RANGE58, true, false },
	{ 250141, BARBOT_RANGE59, false, false },
	{ 251621, BARBOT_RANGE60, false, false },

	{ 251650, BARBOT_RANGE61, false, false },
	{ 251305, BARBOT_RANGE62, false, false },
	{ 251836, BARBOT_RANGE63, false, false },
	{ 251018, BARBOT_RANGE64, true, false },
	{ 250899, BARBOT_RANGE65, false, false },
	{ 250899, BARBOT_RANGE66, false, false },
	{ 251899, BARBOT_RANGE67, false, false },
	{ 251815, BARBOT_RANGE68, false, false },
	{ 251829, BARBOT_RANGE69, false, false },
	{ 251779, BARBOT_RANGE70 }
};


void writeScriptRange(const char *name, const ScriptRange *ranges, int count) {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < count; ++idx) {
		outputFile.writeLong(ranges[idx]._id);
		outputFile.writeByte(ranges[idx]._isRandom);
		outputFile.writeByte(ranges[idx]._isSequential);

		const uint *v = ranges[idx]._array;
		do {
			outputFile.writeLong(*v);
		} while (*v++ != 0);
	}
	outputFile.writeLong(0);

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeAllScriptRanges() {
	writeScriptRange("Ranges/Barbot", BARBOT_RANGES, 70);
}
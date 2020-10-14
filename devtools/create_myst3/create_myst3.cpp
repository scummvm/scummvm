/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <vector>
#include <string>
#include "file.h"

enum ScriptType {
	kNodeScripts,
	kAmbientSoundScripts,
	kBackgroundSoundScripts,
	kInitScript,
	kAmbientCues
};

struct RoomScripts {
	const char *room;
	ScriptType type;
	uint sourceOffset;
	uint size;
	uint targetOffset;
};

void copyData(Common::File &temp, Common::File &target);

void writeSoundNames(const char *string, Common::File &target, uint offset);

static const uint kVersion = 3;

RoomScripts roomScripts[] = {
		{ "ATIX", kNodeScripts, 571168, 324, 0 },
		{ "DEMO", kAmbientSoundScripts, 571716, 76, 0 },
		{ "DEMO", kNodeScripts, 571528, 186, 0 },
		{ "ENCH", kAmbientSoundScripts, 576692, 76, 0 },
		{ "ENCH", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENCH", kNodeScripts, 573488, 3204, 0 },
		{ "ENDD", kAmbientSoundScripts, 582640, 1260, 0 },
		{ "ENDD", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENDD", kNodeScripts, 576800, 5838, 0 },
		{ "ENEM", kAmbientSoundScripts, 586320, 216, 0 },
		{ "ENEM", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENEM", kNodeScripts, 583936, 2382, 0 },
		{ "ENLC", kAmbientSoundScripts, 591988, 416, 0 },
		{ "ENLC", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENLC", kNodeScripts, 586568, 5420, 0 },
		{ "ENLI", kAmbientSoundScripts, 594860, 288, 0 },
		{ "ENLI", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENLI", kNodeScripts, 592440, 2418, 0 },
		{ "ENPP", kAmbientSoundScripts, 597692, 752, 0 },
		{ "ENPP", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENPP", kNodeScripts, 595184, 2506, 0 },
		{ "ENSI", kAmbientSoundScripts, 602056, 760, 0 },
		{ "ENSI", kBackgroundSoundScripts, 571820, 1664, 0 },
		{ "ENSI", kNodeScripts, 598480, 3576, 0 },
		{ "INIT", kAmbientCues, 570996, 126, 0 },
		{ "INIT", kInitScript, 548928, 72, 0 },
		{ "INTR", kAmbientSoundScripts, 754184, 2, 0 },
		{ "INTR", kNodeScripts, 602848, 16, 0 },
		{ "JRNL", kAmbientSoundScripts, 605420, 26, 0 },
		{ "JRNL", kBackgroundSoundScripts, 605448, 30, 0 },
		{ "JRNL", kNodeScripts, 604976, 444, 0 },
		{ "LEET", kAmbientSoundScripts, 606540, 40, 0 },
		{ "LEET", kBackgroundSoundScripts, 606580, 88, 0 },
		{ "LEET", kNodeScripts, 605512, 1026, 0 },
		{ "LEIS", kAmbientSoundScripts, 623360, 854, 0 },
		{ "LEIS", kBackgroundSoundScripts, 624216, 956, 0 },
		{ "LEIS", kNodeScripts, 606704, 16656, 0 },
		{ "LELT", kAmbientSoundScripts, 626236, 64, 0 },
		{ "LELT", kBackgroundSoundScripts, 626300, 88, 0 },
		{ "LELT", kNodeScripts, 625208, 1026, 0 },
		{ "LEMT", kAmbientSoundScripts, 628840, 178, 0 },
		{ "LEMT", kBackgroundSoundScripts, 629020, 38, 0 },
		{ "LEMT", kNodeScripts, 626424, 2414, 0 },
		{ "LEOF", kAmbientSoundScripts, 633276, 154, 0 },
		{ "LEOF", kBackgroundSoundScripts, 633432, 404, 0 },
		{ "LEOF", kNodeScripts, 629096, 4180, 0 },
		{ "LEOS", kAmbientSoundScripts, 641024, 152, 0 },
		{ "LEOS", kBackgroundSoundScripts, 633432, 404, 0 },
		{ "LEOS", kNodeScripts, 633872, 7150, 0 },
		{ "LIDR", kAmbientSoundScripts, 646800, 446, 0 },
		{ "LIDR", kBackgroundSoundScripts, 641204, 1462, 0 },
		{ "LIDR", kNodeScripts, 642672, 4128, 0 },
		{ "LIFO", kAmbientSoundScripts, 655932, 922, 0 },
		{ "LIFO", kBackgroundSoundScripts, 641204, 1462, 0 },
		{ "LIFO", kNodeScripts, 647280, 8650, 0 },
		{ "LINE", kAmbientSoundScripts, 657460, 106, 0 },
		{ "LINE", kBackgroundSoundScripts, 641204, 1462, 0 },
		{ "LINE", kNodeScripts, 656888, 572, 0 },
		{ "LISP", kAmbientSoundScripts, 662004, 1632, 0 },
		{ "LISP", kBackgroundSoundScripts, 641204, 1462, 0 },
		{ "LISP", kNodeScripts, 657600, 4404, 0 },
		{ "LISW", kAmbientSoundScripts, 669412, 838, 0 },
		{ "LISW", kBackgroundSoundScripts, 641204, 1462, 0 },
		{ "LISW", kNodeScripts, 663672, 5740, 0 },
		{ "LOGO", kNodeScripts, 670288, 48, 0 },
		{ "MACA", kAmbientSoundScripts, 674456, 606, 0 },
		{ "MACA", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MACA", kNodeScripts, 671896, 2560, 0 },
		{ "MAIS", kAmbientSoundScripts, 679248, 22, 0 },
		{ "MAIS", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MAIS", kNodeScripts, 675096, 4150, 0 },
		{ "MALL", kAmbientSoundScripts, 683240, 82, 0 },
		{ "MALL", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MALL", kNodeScripts, 679304, 3936, 0 },
		{ "MASS", kAmbientSoundScripts, 689316, 558, 0 },
		{ "MASS", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MASS", kNodeScripts, 683360, 5956, 0 },
		{ "MATO", kAmbientSoundScripts, 692304, 96, 0 },
		{ "MATO", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MATO", kNodeScripts, 689912, 2390, 0 },
		{ "MAWW", kAmbientSoundScripts, 693700, 44, 0 },
		{ "MAWW", kBackgroundSoundScripts, 670364, 1530, 0 },
		{ "MAWW", kNodeScripts, 692432, 1266, 0 },
		{ "MENU", kAmbientSoundScripts, 698144, 106, 0 },
		{ "MENU", kBackgroundSoundScripts, 754188, 2, 0 },
		{ "NACH", kAmbientSoundScripts, 718260, 732, 0 },
		{ "NACH", kBackgroundSoundScripts, 718992, 512, 0 },
		{ "NACH", kNodeScripts, 698288, 19972, 0 },
		{ "TOHB", kAmbientSoundScripts, 720008, 16, 0 },
		{ "TOHB", kNodeScripts, 719536, 470, 0 },
		{ "TOHO", kAmbientSoundScripts, 722316, 138, 0 },
		{ "TOHO", kNodeScripts, 720056, 2258, 0 },
		{ "XXXX", kNodeScripts, 602896, 2044, 0 }
};

RoomScripts menuScriptsDvd[] = {
		{ "MENU", kNodeScripts, 693776, 4368, 0 }
};

RoomScripts menuScriptsCdIntl[] = {
		{ "MENU", kNodeScripts, 693504, 3994, 0 }
};

RoomScripts menuScriptsCdEnglish[] = {
		{ "MENU", kNodeScripts, 693504, 3522, 0 }
};

RoomScripts roomScriptsXBox[] = {
		{ "ATIX", kNodeScripts, 935888, 324, 0 },
		{ "DEMO", kAmbientSoundScripts, 936432, 76, 0 },
		{ "DEMO", kNodeScripts, 936240, 186, 0 },
		{ "ENCH", kAmbientSoundScripts, 941408, 76, 0 },
		{ "ENCH", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENCH", kNodeScripts, 938200, 3204, 0 },
		{ "ENDD", kAmbientSoundScripts, 947352, 1260, 0 },
		{ "ENDD", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENDD", kNodeScripts, 941512, 5838, 0 },
		{ "ENEM", kAmbientSoundScripts, 951040, 216, 0 },
		{ "ENEM", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENEM", kNodeScripts, 948640, 2394, 0 },
		{ "ENLC", kAmbientSoundScripts, 956736, 416, 0 },
		{ "ENLC", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENLC", kNodeScripts, 951288, 5444, 0 },
		{ "ENLI", kAmbientSoundScripts, 959616, 288, 0 },
		{ "ENLI", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENLI", kNodeScripts, 957184, 2432, 0 },
		{ "ENPP", kAmbientSoundScripts, 962472, 752, 0 },
		{ "ENPP", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENPP", kNodeScripts, 959936, 2530, 0 },
		{ "ENSI", kAmbientSoundScripts, 966840, 760, 0 },
		{ "ENSI", kBackgroundSoundScripts, 936536, 1664, 0 },
		{ "ENSI", kNodeScripts, 963256, 3582, 0 },
		{ "INIT", kAmbientCues, 935744, 126, 0 },
		{ "INIT", kInitScript, 913520, 72, 0 },
		{ "INTR", kAmbientSoundScripts, 1104896, 2, 0 },
		{ "INTR", kNodeScripts, 967628, 16, 0 },
		{ "JRNL", kAmbientSoundScripts, 970140, 26, 0 },
		{ "JRNL", kBackgroundSoundScripts, 970168, 30, 0 },
		{ "JRNL", kNodeScripts, 969744, 394, 0 },
		{ "LEET", kAmbientSoundScripts, 971316, 40, 0 },
		{ "LEET", kBackgroundSoundScripts, 971360, 88, 0 },
		{ "LEET", kNodeScripts, 970232, 1082, 0 },
		{ "LEIS", kAmbientSoundScripts, 988432, 854, 0 },
		{ "LEIS", kBackgroundSoundScripts, 989288, 956, 0 },
		{ "LEIS", kNodeScripts, 971480, 16948, 0 },
		{ "LELT", kAmbientSoundScripts, 991360, 64, 0 },
		{ "LELT", kBackgroundSoundScripts, 991424, 88, 0 },
		{ "LELT", kNodeScripts, 990272, 1082, 0 },
		{ "LEMT", kAmbientSoundScripts, 994016, 178, 0 },
		{ "LEMT", kBackgroundSoundScripts, 994196, 38, 0 },
		{ "LEMT", kNodeScripts, 991544, 2470, 0 },
		{ "LEOF", kAmbientSoundScripts, 998544, 154, 0 },
		{ "LEOF", kBackgroundSoundScripts, 998704, 404, 0 },
		{ "LEOF", kNodeScripts, 994264, 4276, 0 },
		{ "LEOS", kAmbientSoundScripts, 1006352, 152, 0 },
		{ "LEOS", kBackgroundSoundScripts, 998704, 404, 0 },
		{ "LEOS", kNodeScripts, 999136, 7214, 0 },
		{ "LIDR", kAmbientSoundScripts, 1012192, 446, 0 },
		{ "LIDR", kBackgroundSoundScripts, 1006536, 1462, 0 },
		{ "LIDR", kNodeScripts, 1008000, 4192, 0 },
		{ "LIFO", kAmbientSoundScripts, 1021440, 922, 0 },
		{ "LIFO", kBackgroundSoundScripts, 1006536, 1462, 0 },
		{ "LIFO", kNodeScripts, 1012672, 8766, 0 },
		{ "LINE", kAmbientSoundScripts, 1022976, 106, 0 },
		{ "LINE", kBackgroundSoundScripts, 1006536, 1462, 0 },
		{ "LINE", kNodeScripts, 1022392, 578, 0 },
		{ "LISP", kAmbientSoundScripts, 1027544, 1632, 0 },
		{ "LISP", kBackgroundSoundScripts, 1006536, 1462, 0 },
		{ "LISP", kNodeScripts, 1023112, 4430, 0 },
		{ "LISW", kAmbientSoundScripts, 1034952, 838, 0 },
		{ "LISW", kBackgroundSoundScripts, 1006536, 1462, 0 },
		{ "LISW", kNodeScripts, 1029208, 5740, 0 },
		{ "LOGO", kNodeScripts, 1035824, 112, 0 },
		{ "MACA", kAmbientSoundScripts, 1040080, 606, 0 },
		{ "MACA", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MACA", kNodeScripts, 1037504, 2572, 0 },
		{ "MAIS", kAmbientSoundScripts, 1044956, 22, 0 },
		{ "MAIS", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MAIS", kNodeScripts, 1040720, 4234, 0 },
		{ "MAKG", kAmbientSoundScripts, 1104900, 2, 0 },
		{ "MAKG", kNodeScripts, 1045008, 116, 0 },
		{ "MALL", kAmbientSoundScripts, 1049128, 82, 0 },
		{ "MALL", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MALL", kNodeScripts, 1045152, 3972, 0 },
		{ "MASS", kAmbientSoundScripts, 1055256, 558, 0 },
		{ "MASS", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MASS", kNodeScripts, 1049240, 6016, 0 },
		{ "MATO", kAmbientSoundScripts, 1058272, 96, 0 },
		{ "MATO", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MATO", kNodeScripts, 1055848, 2420, 0 },
		{ "MAWW", kAmbientSoundScripts, 1059668, 44, 0 },
		{ "MAWW", kBackgroundSoundScripts, 1035968, 1530, 0 },
		{ "MAWW", kNodeScripts, 1058400, 1266, 0 },
		{ "MENU", kAmbientSoundScripts, 1062944, 108, 0 },
		{ "MENU", kBackgroundSoundScripts, 1104904, 2, 0 },
		{ "NACH", kAmbientSoundScripts, 1083128, 732, 0 },
		{ "NACH", kBackgroundSoundScripts, 1083864, 512, 0 },
		{ "NACH", kNodeScripts, 1063080, 20042, 0 },
		{ "TOHB", kAmbientSoundScripts, 1084828, 16, 0 },
		{ "TOHB", kNodeScripts, 1084408, 420, 0 },
		{ "TOHO", kAmbientSoundScripts, 1087280, 138, 0 },
		{ "TOHO", kNodeScripts, 1084872, 2404, 0 },
		{ "XXXX", kNodeScripts, 967672, 2044, 0 }
};

RoomScripts menuScriptsXboxIntl[] = {
		{ "MENU", kNodeScripts, 1059744, 3198, 0 }
};

RoomScripts menuScriptsXboxEnglish[] = {
		{ "MENU", kNodeScripts, 1059744, 3222, 0 }
};

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeScriptData(const char *sourceFilename, Common::File &target, RoomScripts *scripts, uint scriptCount) {
	Common::File source;
	if (!source.open(sourceFilename)) {
		error("Unable to open '%s'", sourceFilename);
	}

	for (uint i = 0; i < scriptCount; i++) {
		// Search if a copy of the same script was already written to the .dat file
		int alreadyCopiedIndex = -1;
		for (uint j = 0; j < i; j++) {
			if (scripts[i].sourceOffset == scripts[j].sourceOffset) {
				alreadyCopiedIndex = j;
			}
		}

		if (alreadyCopiedIndex != -1) {
			// Already copied, just point to the previously copied data
			scripts[i].targetOffset = scripts[alreadyCopiedIndex].targetOffset;
		} else {
			// Make a new copy
			source.seek(scripts[i].sourceOffset);
			scripts[i].targetOffset = target.pos();

			byte *buffer = new byte[scripts[i].size];
			source.read(buffer, scripts[i].size);
			target.write(buffer, scripts[i].size);
			delete[] buffer;
		}
	}

	source.close();
}

void writeScriptIndex(Common::File &target, RoomScripts *scripts, uint scriptCount) {
	target.writeLong(scriptCount);
	for (uint i = 0; i < scriptCount; i++) {
		target.writeString(scripts[i].room);
		target.writeLong(scripts[i].type);
		target.writeLong(scripts[i].targetOffset);
		target.writeLong(scripts[i].size);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s [exe folder]\n", argv[0]);
		printf("Where [exe folder] contains the following files:\n");
		printf("- M3_EN_127.exe: English v1.27 Window binary from the DVD release\n");
		printf("- M3_FR_122.exe: French v1.22 Window binary from the update patch\n");
		printf("- M3_EN_122.exe: English v1.22 Window binary from the update patch\n");
		printf("- M3_XBOX_PAL.xbe: PAL XBox binary\n");
		printf("- M3_XBOX_NTSC.xbe: NTSC XBox binary\n");
		return -1;
	}

	std::string path = argv[1];
	std::string dvdExe = path + "M3_EN_127.exe";
	std::string cdIntlExe = path + "M3_FR_122.exe";
	std::string cdEnglishExe = path + "M3_EN_122.exe";
	std::string xboxPalExe = path + "M3_XBOX_PAL.xbe";
	std::string xboxNtscExe = path + "M3_XBOX_NTSC.xbe";

	Common::File temp;
	if (!temp.open("data.tmp", Common::kFileWriteMode)) {
		error("Unable to open '%s'", "data.tmp");
	}

	writeScriptData(dvdExe.c_str(), temp, roomScripts, ARRAYSIZE(roomScripts));
	writeScriptData(dvdExe.c_str(), temp, menuScriptsDvd, ARRAYSIZE(menuScriptsDvd));
	writeScriptData(cdIntlExe.c_str(), temp, menuScriptsCdIntl, ARRAYSIZE(menuScriptsCdIntl));
	writeScriptData(cdEnglishExe.c_str(), temp, menuScriptsCdEnglish, ARRAYSIZE(menuScriptsCdEnglish));
	writeScriptData(xboxPalExe.c_str(), temp, roomScriptsXBox, ARRAYSIZE(roomScriptsXBox));
	writeScriptData(xboxPalExe.c_str(), temp, menuScriptsXboxIntl, ARRAYSIZE(menuScriptsXboxIntl));
	writeScriptData(xboxNtscExe.c_str(), temp, menuScriptsXboxEnglish, ARRAYSIZE(menuScriptsXboxEnglish));

	Common::File target;
	if (!target.open("myst3.dat", Common::kFileWriteMode)) {
		error("Unable to open '%s'", "myst3.dat");
	}
	target.writeLong(MKTAG('M', 'Y', 'S', 'T'));
	target.writeLong(kVersion);

	writeScriptIndex(target, roomScripts, ARRAYSIZE(roomScripts));
	writeScriptIndex(target, menuScriptsDvd, ARRAYSIZE(menuScriptsDvd));
	writeScriptIndex(target, menuScriptsCdIntl, ARRAYSIZE(menuScriptsCdIntl));
	writeScriptIndex(target, menuScriptsCdEnglish, ARRAYSIZE(menuScriptsCdEnglish));
	writeScriptIndex(target, roomScriptsXBox, ARRAYSIZE(roomScriptsXBox));
	writeScriptIndex(target, menuScriptsXboxIntl, ARRAYSIZE(menuScriptsXboxIntl));
	writeScriptIndex(target, menuScriptsXboxEnglish, ARRAYSIZE(menuScriptsXboxEnglish));

	writeSoundNames(dvdExe.c_str(), target, 549360);
	writeSoundNames(xboxPalExe.c_str(), target, 913960);

	copyData(temp, target);

	temp.close();
	target.close();

	return 0;
}

void writeSoundNames(const char *sourceFilename, Common::File &target, uint offset) {
	Common::File source;
	if (!source.open(sourceFilename)) {
		error("Unable to open '%s'", sourceFilename);
	}

	source.seek(offset);

	// Count the sounds
	uint count = 0;
	while (1) {
		uint32 id = source.readUint32LE();

		if (!id)
			break;

		source.skip(32);
		count++;
	}

	target.writeLong(count);

	source.seek(offset);
	for (uint i = 0; i < count; i++) {
		uint32 id = source.readUint32LE();
		char name[32];
		source.read(name, sizeof(name));

		target.writeLong(id);
		target.write(name, sizeof(name));
	}

	source.close();
}

void copyData(Common::File &temp, Common::File &target) {
	uint dataSize = temp.size();
	byte *data = new byte[dataSize];

	temp.seek(0);
	temp.read(data, dataSize);
	target.write(data, dataSize);

	delete[] data;
}
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
 * $URL$
 * $Id$
 *
 */

#include "drascula/drascula.h"

namespace Drascula {

bool DrasculaEngine::saveLoadScreen() {
	char names[10][23];
	char file[50];
	char fileEpa[50];
	int n, n2, num_sav = 0, y = 27;
	Common::InSaveFile *sav;

	clearRoom();

	snprintf(fileEpa, 50, "%s.epa", _targetName.c_str());
	if (!(sav = _saveFileMan->openForLoading(fileEpa))) {
		Common::OutSaveFile *epa;
		if (!(epa = _saveFileMan->openForSaving(fileEpa)))
			error("Can't open %s file", fileEpa);
		for (n = 0; n < NUM_SAVES; n++)
			epa->writeString("*\n");
		epa->finalize();
		delete epa;
		if (!(sav = _saveFileMan->openForLoading(fileEpa))) {
			error("Can't open %s file", fileEpa);
		}
	}
	for (n = 0; n < NUM_SAVES; n++)
		sav->readLine_OLD(names[n], 23);
	delete sav;

	loadPic("savescr.alg", bgSurface, HALF_PAL);

	color_abc(kColorLightGreen);

	select[0] = 0;

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	setCursor(kCursorCrosshair);

	for (;;) {
		y = 27;
		copyBackground();
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(names[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		updateScreen();
		y = 27;

		updateEvents();

		if (leftMouseButton == 1) {
			delay(50);
			for (n = 0; n < NUM_SAVES; n++) {
				if (mouseX > 115 && mouseY > y + (9 * n) && mouseX < 115 + 175 && mouseY < y + 10 + (9 * n)) {
					strcpy(select, names[n]);

					if (strcmp(select, "*"))
						selectionMade = 1;
					else {
						enterName();
						strcpy(names[n], select);
						if (selectionMade == 1) {
							snprintf(file, 50, "%s%02d", _targetName.c_str(), n + 1);
							saveGame(file);
							Common::OutSaveFile *tsav;
							if (!(tsav = _saveFileMan->openForSaving(fileEpa))) {
								error("Can't open %s file", fileEpa);
							}
							for (n = 0; n < NUM_SAVES; n++) {
								tsav->writeString(names[n]);
								tsav->writeString("\n");
							}
							tsav->finalize();
							delete tsav;
						}
					}

					print_abc(select, 117, 15);
					y = 27;
					for (n2 = 0; n2 < NUM_SAVES; n2++) {
						print_abc(names[n2], 116, y);
						y = y + 9;
					}
					if (selectionMade == 1) {
						snprintf(file, 50, "%s%02d", _targetName.c_str(), n + 1);
					}
					num_sav = n;
				}
			}

			if (mouseX > 117 && mouseY > 15 && mouseX < 295 && mouseY < 24 && selectionMade == 1) {
				enterName();
				strcpy(names[num_sav], select);
				print_abc(select, 117, 15);
				y = 27;
				for (n2 = 0; n2 < NUM_SAVES; n2++) {
					print_abc(names[n2], 116, y);
					y = y + 9;
				}

				if (selectionMade == 1) {
					snprintf(file, 50, "%s%02d", _targetName.c_str(), n + 1);
					saveGame(file);
					Common::OutSaveFile *tsav;
					if (!(tsav = _saveFileMan->openForSaving(fileEpa))) {
						error("Can't open %s file", fileEpa);
					}
					for (n = 0; n < NUM_SAVES; n++) {
						tsav->writeString(names[n]);
						tsav->writeString("\n");
					}
					tsav->finalize();
					delete tsav;
				}
			}

			if (mouseX > 125 && mouseY > 123 && mouseX < 199 && mouseY < 149 && selectionMade == 1) {
				if (!loadGame(file)) {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
					return false;
				}
				break;
			} else if (mouseX > 208 && mouseY > 123 && mouseX < 282 && mouseY < 149 && selectionMade == 1) {
				saveGame(file);
				Common::OutSaveFile *tsav;
				if (!(tsav = _saveFileMan->openForSaving(fileEpa))) {
					error("Can't open %s file", fileEpa);
				}
				for (n = 0; n < NUM_SAVES; n++) {
					tsav->writeString(names[n]);
					tsav->writeString("\n");
				}
				tsav->finalize();
				delete tsav;
			} else if (mouseX > 168 && mouseY > 154 && mouseX < 242 && mouseY < 180)
				break;
			else if (selectionMade == 0) {
				print_abc("Please select a slot", 117, 15);
			}
			updateScreen();
			delay(200);
		}
		y = 26;

		delay(5);
	}

	selectVerb(0);

	clearRoom();
	loadPic(roomNumber, bgSurface, HALF_PAL);
	selectionMade = 0;

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	return true;
}

bool DrasculaEngine::loadGame(const char *gameName) {
	int l, savedChapter, roomNum = 0;
	Common::InSaveFile *sav;

	previousMusic = roomMusic;
	menuScreen = 0;
	if (currentChapter != 1)
		clearRoom();

	if (!(sav = _saveFileMan->openForLoading(gameName))) {
		error("missing savegame file");
	}

	savedChapter = sav->readSint32LE();
	if (savedChapter != currentChapter) {
		strcpy(saveName, gameName);
		currentChapter = savedChapter - 1;
		loadedDifferentChapter = 1;
		return false;
	}
	sav->read(currentData, 20);
	curX = sav->readSint32LE();
	curY = sav->readSint32LE();
	trackProtagonist = sav->readSint32LE();

	for (l = 1; l < 43; l++) {
		inventoryObjects[l] = sav->readSint32LE();
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		flags[l] = sav->readSint32LE();
	}

	takeObject = sav->readSint32LE();
	pickedObject = sav->readSint32LE();
	loadedDifferentChapter = 0;
	sscanf(currentData, "%d.ald", &roomNum);
	enterRoom(roomNum);
	selectVerb(0);

	return true;
}

void DrasculaEngine::saveGame(char gameName[]) {
	Common::OutSaveFile *out;
	int l;

	if (!(out = _saveFileMan->openForSaving(gameName))) {
		error("Unable to open the file");
	}
	out->writeSint32LE(currentChapter);
	out->write(currentData, 20);
	out->writeSint32LE(curX);
	out->writeSint32LE(curY);
	out->writeSint32LE(trackProtagonist);

	for (l = 1; l < 43; l++) {
		out->writeSint32LE(inventoryObjects[l]);
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		out->writeSint32LE(flags[l]);
	}

	out->writeSint32LE(takeObject);
	out->writeSint32LE(pickedObject);

	out->finalize();
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", gameName);

	delete out;

	playSound(99);
	finishSound();
}

} // End of namespace Drascula

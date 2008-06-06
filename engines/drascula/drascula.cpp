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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "drascula/drascula.h"

namespace Drascula {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings drasculaSettings[] = {
	{"drascula", "Drascula game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

DrasculaEngine::DrasculaEngine(OSystem *syst, const DrasculaGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource();
	syst->getEventManager()->registerRandomSource(*_rnd, "drascula");

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);

	_lang = 0;
}

DrasculaEngine::~DrasculaEngine() {
	delete _rnd;
}

int DrasculaEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	switch (getLanguage()) {
	case Common::EN_ANY:
		_lang = 0;
		break;
	case Common::ES_ESP:
		_lang = 1;
		break;
	case Common::DE_DEU:
		_lang = 2;
		break;
	case Common::FR_FRA:
		_lang = 3;
		break;
	case Common::IT_ITA:
		_lang = 4;
		break;
	default:
		warning("Unknown game language. Falling back to English");
		_lang = 0;
	}

	setupRoomsTable();
	loadArchives();

	return 0;
}

int DrasculaEngine::go() {
	currentChapter = 1; // values from 1 to 6 will start each part of game
	hay_que_load = 0;

	for (;;) {
		int i;

		VGA = (byte *)malloc(320 * 200);
		memset(VGA, 0, 64000);

		takeObject = 0;
		menuBar = 0; menuScreen = 0; hasName = 0;
		frame_y = 0;
		curX = -1; characterMoved = 0; trackProtagonist = 3; num_frame = 0; hare_se_ve = 1;
		checkFlags = 1;
		doBreak = 0;
		walkToObject = 0;
		stepX = STEP_X; stepY = STEP_Y;
		curHeight = CHARACTER_HEIGHT; curWidth = CHARACTER_WIDTH; feetHeight = FEET_HEIGHT;
		talkHeight = TALK_HEIGHT; talkWidth = TALK_WIDTH;
		hasAnswer = 0;
		savedTime = 0;
		changeColor = 0;
		breakOut = 0;
		vonBraunX = 120; trackVonBraun = 1; vonBraunHasMoved = 0;
		framesWithoutAction = 0;
		term_int = 0;
		musicStopped = 0;
		selectionMade = 0;
		UsingMem = 0;
		globalSpeed = 0;
		curExcuseLook = 0;
		curExcuseAction = 0;
		roomNumber = 0;

		for (i = 0; i < 8; i++)
			actorFrames[i] = 0;
		actorFrames[kFrameVonBraun] = 1;

		allocMemory();

		withVoices = 0;
		selectionMade = 0;

		if (currentChapter != 6)
			loadPic(95, tableSurface);

		if (currentChapter == 1) {
			loadPic(96, frontSurface, COMPLETE_PAL);
			loadPic(99, backSurface);
			loadPic(97, extraSurface);
		} else if (currentChapter == 2) {
			loadPic(96, frontSurface, COMPLETE_PAL);
			loadPic("pts.alg", drawSurface2);
		} else if (currentChapter == 3) {
			loadPic("aux13.alg", drawSurface1, COMPLETE_PAL);
			loadPic(96, frontSurface);
			loadPic(97, extraSurface);
			loadPic(99, backSurface);
		} else if (currentChapter == 4) {
			loadPic(96, frontSurface, COMPLETE_PAL);
			if (hay_que_load == 0)
				animation_ray();
			loadPic(96, frontSurface);
			clearRoom();
			loadPic(99, backSurface);
			loadPic(97, extraSurface);
		} else if (currentChapter == 5) {
			loadPic(96, frontSurface, COMPLETE_PAL);
			loadPic(97, extraSurface);
			loadPic(99, backSurface);
		} else if (currentChapter == 6) {
			igorX = 105, igorY = 85, trackIgor = 1;
			drasculaX = 62, drasculaY = 99, trackDrascula = 1;
			actorFrames[kFramePendulum] = 0;
			flag_tv = 0;

			pendulumSurface = drawSurface3;

			loadPic(96, frontSurface, COMPLETE_PAL);
			loadPic(99, backSurface);
			loadPic(97, extraSurface);
			loadPic(95, tableSurface);
		}
		memset(iconName, 0, sizeof(iconName));

		for (i = 0; i < 6; i++)
			strcpy(iconName[i + 1], _textverbs[_lang][i]);

		assignDefaultPalette();
		if (!runCurrentChapter()) {
			quitGame();
			break;
		}
		quitGame();
		if (currentChapter == 6)
			break;

		currentChapter++;
	}

	return 0;
}

void DrasculaEngine::quitGame() {
	stopSound();
	clearRoom();
	black();
	MusicFadeout();
	stopMusic();
	freeMemory();
	free(VGA);
}

void DrasculaEngine::allocMemory() {
	screenSurface = (byte *)malloc(64000);
	assert(screenSurface);
	frontSurface = (byte *)malloc(64000);
	assert(frontSurface);
	backSurface = (byte *)malloc(64000);
	assert(backSurface);
	drawSurface1 = (byte *)malloc(64000);
	assert(drawSurface1);
	drawSurface2 = (byte *)malloc(64000);
	assert(drawSurface2);
	drawSurface3 = (byte *)malloc(64000);
	assert(drawSurface3);
	tableSurface = (byte *)malloc(64000);
	assert(tableSurface);
	extraSurface = (byte *)malloc(64000);
	assert(extraSurface);
}

void DrasculaEngine::freeMemory() {
	free(screenSurface);
	free(drawSurface1);
	free(backSurface);
	free(drawSurface2);
	free(tableSurface);
	free(drawSurface3);
	free(extraSurface);
	free(frontSurface);
}

void DrasculaEngine::loadPic(const char *NamePcc, byte *targetSurface, int colorCount) {
	unsigned int con, x = 0;
	unsigned int fExit = 0;
	byte ch, rep;
	byte *auxPun;

	_arj.open(NamePcc);
	if (!_arj.isOpen())
		error("missing game data %s %c", NamePcc, 7);

	pcxBuffer = (byte *)malloc(65000);
	auxPun = pcxBuffer;
	_arj.seek(128);
	while (!fExit) {
		ch = _arj.readByte();
		rep = 1;
		if ((ch & 192) == 192) {
			rep = (ch & 63);
			ch = _arj.readByte();
		}
		for (con = 0; con < rep; con++) {
			*auxPun++ = ch;
			x++;
			if (x > 64000)
				fExit = 1;
		}
	}

	_arj.read(cPal, 768);
	_arj.close();

	memcpy(targetSurface, pcxBuffer, 64000);
	free(pcxBuffer);
	setRGB((byte *)cPal, colorCount);
}

void DrasculaEngine::setRGB(byte *pal, int colorCount) {
	int x, cnt = 0;

	for (x = 0; x < colorCount; x++) {
		gamePalette[x][0] = pal[cnt++] / 4;
		gamePalette[x][1] = pal[cnt++] / 4;
		gamePalette[x][2] = pal[cnt++] / 4;
	}
	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::black() {
	int color, component;
	DacPalette256 blackPalette;

	for (color = 0; color < 256; color++)
		for (component = 0; component < 3; component++)
			blackPalette[color][component] = 0;

	blackPalette[254][0] = 0x3F;
	blackPalette[254][1] = 0x3F;
	blackPalette[254][2] = 0x15;

	setPalette((byte *)&blackPalette);
}

void DrasculaEngine::setPalette(byte *PalBuf) {
	byte pal[256 * 4];
	int i;

	for (i = 0; i < 256; i++) {
		pal[i * 4 + 0] = PalBuf[i * 3 + 0] * 4;
		pal[i * 4 + 1] = PalBuf[i * 3 + 1] * 4;
		pal[i * 4 + 2] = PalBuf[i * 3 + 2] * 4;
		pal[i * 4 + 3] = 0;
	}
	_system->setPalette(pal, 0, 256);
	_system->updateScreen();
}

void DrasculaEngine::copyBackground(int xorg, int yorg, int xdes, int ydes, int width,
								  int height, byte *src, byte *dest) {
	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;
	for (int x = 0; x < height; x++) {
		memcpy(dest, src, width);
		dest += 320;
		src += 320;
	}
}

void DrasculaEngine::copyRect(int xorg, int yorg, int xdes, int ydes, int width,
								   int height, byte *src, byte *dest) {
	int y, x;

	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			if (src[x + y * 320] != 255)
				dest[x + y * 320] = src[x + y * 320];
}

void DrasculaEngine::copyRectClip(int *Array, byte *src, byte *dest) {
	int y, x;
	int xorg = Array[0];
	int yorg = Array[1];
	int xdes = Array[2];
	int ydes = Array[3];
	int width = Array[4];
	int height = Array[5];

	if (ydes < 0) {
		yorg += -ydes;
		height += ydes;
		ydes = 0;
	}
	if (xdes < 0) {
		xorg += -xdes;
		width += xdes;
		xdes = 0;
	}
	if ((xdes + width) > 319)
		width -= (xdes + width) - 320;
	if ((ydes + height) > 199)
		height -= (ydes + height) - 200;

	dest += xdes + ydes * 320;
	src += xorg + yorg * 320;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			if (src[x + y * 320] != 255)
				dest[x + y * 320] = src[x + y * 320];
}

void DrasculaEngine::updateScreen(int xorg, int yorg, int xdes, int ydes, int width, int height, byte *buffer) {
	byte *ptr = VGA;

	ptr += xdes + ydes * 320;
	buffer += xorg + yorg * 320;
	for (int x = 0; x < height; x++) {
		memcpy(ptr, buffer, width);
		ptr += 320;
		buffer += 320;
	}

	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

bool DrasculaEngine::runCurrentChapter() {
	int n;

	previousMusic = -1;

	if (currentChapter != 2) {
		int soc = 0;
		for (n = 0; n < 6; n++) {
			soc = soc + CHARACTER_WIDTH;
			frameX[n] = soc;
		}
	}

	for (n = 1; n < 43; n++)
		inventoryObjects[n] = 0;

	for (n = 0; n < NUM_FLAGS; n++)
		flags[n] = 0;

	if (currentChapter == 2) {
		flags[16] = 1;
		flags[17] = 1;
		flags[27] = 1;
	}

	for (n = 1; n < 7; n++)
		inventoryObjects[n] = n;

	if (currentChapter == 1) {
		pickObject(28);

		if (hay_que_load == 0)
			animation_1_1();

		withoutVerb();
		loadPic("2aux62.alg", drawSurface2);
		trackProtagonist = 1;
		objExit = 104;
		if (hay_que_load != 0) {
			if (!loadGame(saveName)) {
				return true;
			}
		} else {
			enterRoom(62);
			curX = -20;
			curY = 56;
			gotoObject(65, 145);
		}
	} else if (currentChapter == 2) {
		addObject(kItemPhone);
		trackProtagonist = 3;
		objExit = 162;
		if (hay_que_load == 0)
			enterRoom(14);
		else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 3) {
		addObject(kItemPhone);
		addObject(kItemEarplugs);
		addObject(kItemSickle);
		addObject(kItemHandbag);
		addObject(kItemCross);
		addObject(kItemReefer);
		addObject(kItemOneCoin);
		flags[1] = 1;
		trackProtagonist = 1;
		objExit = 99;
		if (hay_que_load == 0)
			enterRoom(20);
		else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	// From here onwards the items have different IDs
	} else if (currentChapter == 4) {
		addObject(kItemPhone2);
		addObject(kItemCross2);
		addObject(kItemReefer2);
		addObject(kItemOneCoin2);
		objExit = 100;
		if (hay_que_load == 0) {
			enterRoom(21);
			trackProtagonist = 0;
			curX = 235;
			curY = 164;
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 5) {
		addObject(28);
		addObject(7);
		addObject(9);
		addObject(11);
		addObject(13);
		addObject(14);
		addObject(15);
		addObject(17);
		addObject(20);
		trackProtagonist = 1;
		objExit = 100;
		if (hay_que_load == 0) {
			enterRoom(45);
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 6) {
		addObject(28);
		addObject(9);

		trackProtagonist = 1;
		objExit = 104;
		if (hay_que_load == 0) {
			enterRoom(58);
			animation_1_6();
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
			loadPic("auxdr.alg", drawSurface2);
		}
	}

	while (1) {
		if (characterMoved == 0) {
			stepX = STEP_X;
			stepY = STEP_Y;
		}
		if (characterMoved == 0 && walkToObject == 1) {
			trackProtagonist = trackFinal;
			walkToObject = 0;
		}

		if (currentChapter == 2) {
			if (roomNumber == 3 && (curX == 279) && (curY + curHeight == 101))
				animation_1_2();
			else if (roomNumber == 14 && (curX == 214) && (curY + curHeight == 121))
				gotoObject(190, 130);
			else if (roomNumber == 14 && (curX == 246) && (curY + curHeight == 112))
				gotoObject(190, 130);
		}

		moveCursor();
		updateScreen();

		if (currentChapter == 2) {
			if (musicStatus() == 0 && roomMusic != 0)
				playMusic(roomMusic);
		} else {
			if (musicStatus() == 0)
				playMusic(roomMusic);
		}

		updateEvents();

		if (menuScreen == 0 && takeObject == 1)
			checkObjects();
		
		if (rightMouseButton == 1 && menuScreen == 1) {
			delay(100);
			if (currentChapter == 2)
				loadPic(menuBackground, backSurface);
			else
				loadPic(99, backSurface);
			setPalette((byte *)&gamePalette);
			menuScreen = 0;
			updateEvents();
		}
		if (rightMouseButton == 1 && menuScreen == 0) {
			delay(100);
			characterMoved = 0;
			if (trackProtagonist == 2)
				trackProtagonist = 1;
			if (currentChapter == 4)
				loadPic("icons2.alg", backSurface);
			else if (currentChapter == 5)
				loadPic("icons3.alg", backSurface);
			else if (currentChapter == 6)
				loadPic("iconsp.alg", backSurface);
			else
				loadPic("icons.alg", backSurface);
			menuScreen = 1;
			updateEvents();
			withoutVerb();
		}

		if (leftMouseButton == 1 && menuBar == 1) {
			delay(100);
			selectVerbFromBar();
		} else if (leftMouseButton == 1 && takeObject == 0) {
			delay(100);
			if (verify1())
				return true;
		} else if (leftMouseButton == 1 && takeObject == 1) {
			if (verify2())
				return true;
		}

		menuBar = (mouseY < 24 && menuScreen == 0) ? 1 : 0;

		Common::KeyCode key = getScan();
		if (key == Common::KEYCODE_F1 && menuScreen == 0) {
			selectVerb(1);
		} else if (key == Common::KEYCODE_F2 && menuScreen == 0) {
			selectVerb(2);
		} else if (key == Common::KEYCODE_F3 && menuScreen == 0) {
			selectVerb(3);
		} else if (key == Common::KEYCODE_F4 && menuScreen == 0) {
			selectVerb(4);
		} else if (key == Common::KEYCODE_F5 && menuScreen == 0) {
			selectVerb(5);
		} else if (key == Common::KEYCODE_F6 && menuScreen == 0) {
			selectVerb(6);
		} else if (key == Common::KEYCODE_F9) {
			volumeControls();
		} else if (key == Common::KEYCODE_F10) {
			if (!saveLoadScreen())
				return true;
		} else if (key == Common::KEYCODE_F8) {
			withoutVerb();
		} else if (key == Common::KEYCODE_v) {
			withVoices = 1;
			print_abc(_textsys[_lang][2], 96, 86);
			updateScreen();
			delay(1410);
		} else if (key == Common::KEYCODE_t) {
			withVoices = 0;
			print_abc(_textsys[_lang][3], 94, 86);
			updateScreen();
			delay(1460);
		} else if (key == Common::KEYCODE_ESCAPE) {
			if (!confirmExit())
				return false;
		} else if (currentChapter == 6 && key == Common::KEYCODE_0 && roomNumber == 61) {
			loadPic("alcbar.alg", drawSurface1, 255);
		}
		
		if (leftMouseButton != 0 || rightMouseButton != 0 || key != 0)
			if (currentChapter != 3)
				framesWithoutAction = 0;

		if (framesWithoutAction == 15000) {
			screenSaver();
			if (currentChapter != 3)
				framesWithoutAction = 0;
		}

		if (currentChapter != 3)
			framesWithoutAction++;

	}
}

void DrasculaEngine::pickObject(int objeto) {
	if (currentChapter == 6)
		loadPic("iconsp.alg", backSurface);
	else if (currentChapter == 4)
		loadPic("icons2.alg", backSurface);
	else if (currentChapter == 5)
		loadPic("icons3.alg", backSurface);
	else
		loadPic("icons.alg", backSurface);
	chooseObject(objeto);
	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
}

void DrasculaEngine::chooseObject(int objeto) {
	if (currentChapter == 5) {
		if (takeObject == 1 && menuScreen == 0 && pickedObject != 16)
			addObject(pickedObject);
	} else {
		if (takeObject == 1 && menuScreen == 0)
			addObject(pickedObject);
	}
	copyBackground(x1d_menu[objeto], y1d_menu[objeto], 0, 0, OBJWIDTH,OBJHEIGHT, backSurface, drawSurface3);
	takeObject = 1;
	pickedObject = objeto;
}

int DrasculaEngine::removeObject(int obj) {
	int result = 1;

	for (int h = 1; h < 43; h++) {
		if (inventoryObjects[h] == obj) {
			inventoryObjects[h] = 0;
			result = 0;
			break;
		}
	}

	return result;
}

void DrasculaEngine::withoutVerb() {
	int c = (menuScreen == 1) ? 0 : 171;

	if (currentChapter == 5) {
		if (takeObject == 1 && pickedObject != 16)
			addObject(pickedObject);
	} else {
		if (takeObject == 1)
			addObject(pickedObject);
	}
	copyBackground(0, c, 0, 0, OBJWIDTH,OBJHEIGHT, backSurface, drawSurface3);

	takeObject = 0;
	hasName = 0;
}

char *DrasculaEngine::getLine(char *buf, int len) {
	byte c;
	char *b;

	for (;;) {
		b = buf;
		while (!_arj.eos()) {
			c = ~_arj.readByte();
			if (c == '\r')
				continue;
			if (c == '\n' || b - buf >= (len - 1))
				break;
			*b++ = c;
		}
		*b = '\0';
		if (_arj.eos() && b == buf)
			return NULL;
		if (b != buf)
			break;
	}
	return buf;
}

void DrasculaEngine::getIntFromLine(char *buf, int len, int* result) {
	getLine(buf, len);
	sscanf(buf, "%d", result);
}

void DrasculaEngine::getStringFromLine(char *buf, int len, char* result) {
	getLine(buf, len);
	sscanf(buf, "%s", result);
}

void DrasculaEngine::enterRoom(int roomIndex) {
	debug(2, "Entering room %d", roomIndex);

	char fileName[20];
	sprintf(fileName, "%d.ald", roomIndex);
	int soc, l, martin = 0, objIsExit = 0;
	float chiquez = 0, pequegnez = 0;
	char pant1[20], pant2[20], pant3[20], pant4[20];
	char buffer[256];
	int palLevel = 0;

	hasName = 0;

	strcpy(currentData, fileName);

	_arj.open(fileName);
	if (!_arj.isOpen()) {
		error("missing data file %s", fileName);
	}
	int size = _arj.size();

	getIntFromLine(buffer, size, &roomNumber);
	getIntFromLine(buffer, size, &roomMusic);
	getStringFromLine(buffer, size, roomDisk);
	getIntFromLine(buffer, size, &palLevel);

	if (currentChapter == 2)
		getIntFromLine(buffer, size, &martin);

	if (currentChapter == 2 && martin != 0) {
		curWidth = martin;
		getIntFromLine(buffer, size, &curHeight);
		getIntFromLine(buffer, size, &feetHeight);
		getIntFromLine(buffer, size, &stepX);
		getIntFromLine(buffer, size, &stepY);

		getStringFromLine(buffer, size, pant1);
		getStringFromLine(buffer, size, pant2);
		getStringFromLine(buffer, size, pant3);
		getStringFromLine(buffer, size, pant4);

		strcpy(menuBackground, pant4);
	}

	getIntFromLine(buffer, size, &numRoomObjs);

	for (l = 0; l < numRoomObjs; l++) {
		getIntFromLine(buffer, size, &objectNum[l]);
		getStringFromLine(buffer, size, objName[l]);
		getIntFromLine(buffer, size, &x1[l]);
		getIntFromLine(buffer, size, &y1[l]);
		getIntFromLine(buffer, size, &x2[l]);
		getIntFromLine(buffer, size, &y2[l]);
		getIntFromLine(buffer, size, &roomObjX[l]);
		getIntFromLine(buffer, size, &roomObjY[l]);
		getIntFromLine(buffer, size, &trackObj[l]);
		getIntFromLine(buffer, size, &visible[l]);
		getIntFromLine(buffer, size, &isDoor[l]);
		if (isDoor[l] != 0) {
			getStringFromLine(buffer, size, _targetSurface[l]);
			getIntFromLine(buffer, size, &_destX[l]);
			getIntFromLine(buffer, size, &_destY[l]);
			getIntFromLine(buffer, size, &trackCharacter_alkeva[l]);
			getIntFromLine(buffer, size, &alapuertakeva[l]);
			updateDoor(l);
		}
	}

	getIntFromLine(buffer, size, &floorX1);
	getIntFromLine(buffer, size, &floorY1);
	getIntFromLine(buffer, size, &floorX2);
	getIntFromLine(buffer, size, &floorY2);

	if (currentChapter != 2) {
		getIntFromLine(buffer, size, &far);
		getIntFromLine(buffer, size, &near);
	}
	_arj.close();

	if (currentChapter == 2 && martin != 0) {
		loadPic(pant2, extraSurface);
		loadPic(pant1, frontSurface);
		loadPic(pant4, backSurface);
	}

	if (currentChapter == 2) {
		if (martin == 0) {
			stepX = STEP_X;
			stepY = STEP_Y;
			curHeight = CHARACTER_HEIGHT;
			curWidth = CHARACTER_WIDTH;
			feetHeight = FEET_HEIGHT;
			loadPic(97, extraSurface);
			loadPic(96, frontSurface);
			loadPic(99, backSurface);

			strcpy(menuBackground, "99.alg");
		}
	}

	for (l = 0; l < numRoomObjs; l++) {
		if (objectNum[l] == objExit)
			objIsExit = l;
	}

	if (currentChapter == 2) {
		if (curX == -1) {
			curX = _destX[objIsExit];
			curY = _destY[objIsExit] - curHeight;
		}
		characterMoved = 0;
	}

	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, drawSurface1, HALF_PAL);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	setDefaultPalette();
	if (palLevel != 0)
		setPaletteBase(palLevel);
	assignBrightPalette();
	setDefaultPalette();
	setPaletteBase(palLevel + 2);
	assignDarkPalette();

	setBrightPalette();
	changeColor = -1;

	if (currentChapter == 2)
		color_abc(kColorLightGreen);

	if (currentChapter != 2) {
		for (l = 0; l <= floorY1; l++)
			factor_red[l] = far;
		for (l = floorY1; l <= 201; l++)
			factor_red[l] = near;

		chiquez = (float)(near - far) / (float)(floorY2 - floorY1);
		for (l = floorY1; l <= floorY2; l++) {
			factor_red[l] = (int)(far + pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (roomNumber == 24) {
		for (l = floorY1 - 1; l > 74; l--) {
			factor_red[l] = (int)(far - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (currentChapter == 5 && roomNumber == 54) {
		for (l = floorY1 - 1; l > 84; l--) {
			factor_red[l] = (int)(far - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (currentChapter != 2) {
		if (curX == -1) {
			curX = _destX[objIsExit];
			curY = _destY[objIsExit];
			curHeight = (CHARACTER_HEIGHT * factor_red[curY]) / 100;
			curWidth = (CHARACTER_WIDTH * factor_red[curY]) / 100;
			curY = curY - curHeight;
		} else {
			curHeight = (CHARACTER_HEIGHT * factor_red[curY]) / 100;
			curWidth = (CHARACTER_WIDTH * factor_red[curY]) / 100;
		}
		characterMoved = 0;
	}

	if (currentChapter == 2) {
		soc = 0;
		for (l = 0; l < 6; l++) {
			soc += curWidth;
			frameX[l] = soc;
		}
	}

	if (currentChapter == 5)
		hare_se_ve = 1;

	updateVisible();

	if (currentChapter == 1)
		isDoor[7] = 0;

	if (currentChapter == 2) {
		if (roomNumber == 14 && flags[39] == 1)
			roomMusic = 16;
		else if (roomNumber == 15 && flags[39] == 1)
			roomMusic = 16;
		if (roomNumber == 14 && flags[5] == 1)
			roomMusic = 0;
		else if (roomNumber == 15 && flags[5] == 1)
			roomMusic = 0;

		if (previousMusic != roomMusic && roomMusic != 0)
			playMusic(roomMusic);
		if (roomMusic == 0)
			stopMusic();
	} else {
		if (previousMusic != roomMusic && roomMusic != 0)
			playMusic(roomMusic);
	}

	if (currentChapter == 2) {
		if (roomNumber == 9 || roomNumber == 2 || roomNumber == 14 || roomNumber == 18)
			savedTime = getTime();
	}
	if (currentChapter == 4) {
		if (roomNumber == 26)
			savedTime = getTime();
	}

	if (currentChapter == 4 && roomNumber == 24 && flags[29] == 1)
		animation_7_4();

	if (currentChapter == 5) {
		if (roomNumber == 45)
			hare_se_ve = 0;
		if (roomNumber == 49 && flags[7] == 0)
			animation_4_5();
	}

	updateRoom();
}

void DrasculaEngine::clearRoom() {
	memset(VGA, 0, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void DrasculaEngine::gotoObject(int pointX, int pointY) {
	if (currentChapter == 5 || currentChapter == 6) {
		if (hare_se_ve == 0) {
			curX = roomX;
			curY = roomY;
			updateRoom();
			updateScreen();
			return;
		}
	}
	roomX = pointX;
	roomY = pointY;
	startWalking();

	for (;;) {
		updateRoom();
		updateScreen();
		if (characterMoved == 0)
			break;
	}

	if (walkToObject == 1) {
		walkToObject = 0;
		trackProtagonist = trackFinal;
	}
	updateRoom();
	updateScreen();
}

void DrasculaEngine::moveCursor() {
	int cursorPos[8];

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	updateRefresh_pre();
	moveCharacters();
	updateRefresh();

	if (!strcmp(textName, "hacker") && hasName == 1) {
		if (_color != kColorRed && menuScreen == 0)
			color_abc(kColorRed);
	} else if (menuScreen == 0 && _color != kColorLightGreen)
		color_abc(kColorLightGreen);
	if (hasName == 1 && menuScreen == 0)
		centerText(textName, mouseX, mouseY);
	if (menuScreen == 1)
		showMenu();
	else if (menuBar == 1)
		clearMenu();

	cursorPos[0] = 0;
	cursorPos[1] = 0;
	cursorPos[2] = mouseX - 20;
	cursorPos[3] = mouseY - 17;
	cursorPos[4] = OBJWIDTH;
	cursorPos[5] = OBJHEIGHT;
	copyRectClip(cursorPos, drawSurface3, screenSurface);
}

void DrasculaEngine::checkObjects() {
	int l, veo = 0;

	for (l = 0; l < numRoomObjs; l++) {
		if (mouseX > x1[l] && mouseY > y1[l]
				&& mouseX < x2[l] && mouseY < y2[l]
				&& visible[l] == 1 && isDoor[l] == 0) {
			strcpy(textName, objName[l]);
			hasName = 1;
			veo = 1;
		}
	}

	if (mouseX > curX + 2 && mouseY > curY + 2
			&& mouseX < curX + curWidth - 2 && mouseY < curY + curHeight - 2) {
		if (currentChapter == 2 || veo == 0) {
			strcpy(textName, "hacker");
			hasName = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hasName = 0;
}

void DrasculaEngine::selectVerbFromBar() {
	for (int n = 0; n < 7; n++) {
		if (mouseX > verbBarX[n] && mouseX < verbBarX[n + 1] && n > 0) {
			selectVerb(n);
			return;
		}
	}

	// no verb selected
	withoutVerb();
}

bool DrasculaEngine::verify1() {
	int l;

	if (menuScreen == 1)
		removeObject();
	else {
		for (l = 0; l < numRoomObjs; l++) {
			if (mouseX >= x1[l] && mouseY >= y1[l]
					&& mouseX <= x2[l] && mouseY <= y2[l] && doBreak == 0) {
				if (exitRoom(l))
					return true;
				if (doBreak == 1)
					break;
			}
		}

		if (mouseX > curX && mouseY > curY
				&& mouseX < curX + curWidth && mouseY < curY + curHeight)
			doBreak = 1;

		for (l = 0; l < numRoomObjs; l++) {
			if (mouseX > x1[l] && mouseY > y1[l]
					&& mouseX < x2[l] && mouseY < y2[l] && doBreak == 0) {
				roomX = roomObjX[l];
				roomY = roomObjY[l];
				trackFinal = trackObj[l];
				doBreak = 1;
				walkToObject = 1;
				startWalking();
			}
		}

		if (doBreak == 0) {
			roomX = CLIP(mouseX, floorX1, floorX2);
			roomY = CLIP(mouseY, floorY1 + feetHeight, floorY2);
			startWalking();
		}
		doBreak = 0;
	}

	return false;
}

bool DrasculaEngine::verify2() {
	int l;

	if (menuScreen == 1) {
		if (pickupObject())
			return true;
	} else {
		if (!strcmp(textName, "hacker") && hasName == 1) {
			if (checkAction(50))
				return true;
		} else {
			for (l = 0; l < numRoomObjs; l++) {
				if (mouseX > x1[l] && mouseY > y1[l]
						&& mouseX < x2[l] && mouseY < y2[l] && visible[l] == 1) {
					trackFinal = trackObj[l];
					walkToObject = 1;
					gotoObject(roomObjX[l], roomObjY[l]);
					if (checkAction(objectNum[l]))
						return true;
					if (currentChapter == 4)
						break;
				}
			}
		}
	}

	return false;
}

Common::KeyCode DrasculaEngine::getScan() {
	updateEvents();

	return _keyPressed.keycode;
}

void DrasculaEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	AudioCD.updateCD();

	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyPressed = event.kbd;
			break;
		case Common::EVENT_KEYUP:
			_keyPressed.keycode = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			mouseX = event.mouse.x;
			mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			leftMouseButton = 1;
			break;
		case Common::EVENT_LBUTTONUP:
			leftMouseButton = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			rightMouseButton = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			rightMouseButton = 0;
			break;
		case Common::EVENT_QUIT:
			// TODO
			quitGame();
			exit(0);
			break;
		default:
			break;
		}
	}
}

void DrasculaEngine::selectVerb(int verbo) {
	int c = (menuScreen == 1) ? 0 : 171;

	if (currentChapter == 5) {
		if (takeObject == 1 && pickedObject != 16)
			addObject(pickedObject);
	} else {
		if (takeObject == 1)
			addObject(pickedObject);
	}

	copyBackground(OBJWIDTH * verbo, c, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	takeObject = 1;
	pickedObject = verbo;
}

void DrasculaEngine::updateVolume(Audio::Mixer::SoundType soundType, int prevVolume) {
	int vol = _mixer->getVolumeForSoundType(soundType) / 16;
	if (mouseY < prevVolume && vol < 15)
		vol++;
	if (mouseY > prevVolume && vol > 0)
		vol--;
	_mixer->setVolumeForSoundType(soundType, vol * 16);
}

void DrasculaEngine::volumeControls() {
	int masterVolume, voiceVolume, musicVolume;

	copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);
	updateScreen(73, 63, 73, 63, 177, 97, screenSurface);

	masterVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4);
	voiceVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4);
	musicVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4);

	for (;;) {
		updateRoom();

		copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);

		copyBackground(183, 56, 82, masterVolume, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4), tableSurface, screenSurface);
		copyBackground(183, 56, 138, voiceVolume, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4), tableSurface, screenSurface);
		copyBackground(183, 56, 194, musicVolume, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4), tableSurface, screenSurface);

		setCursorTable();

		updateScreen();

		updateEvents();

		if (rightMouseButton == 1) {
			delay(100);
			break;
		}
		if (leftMouseButton == 1) {
			delay(100);
			if (mouseX > 80 && mouseX < 121) {
				updateVolume(Audio::Mixer::kPlainSoundType, mouseY);
				masterVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4);
			}

			if (mouseX > 136 && mouseX < 178) {
				updateVolume(Audio::Mixer::kSFXSoundType, mouseY);
				voiceVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4);
			}

			if (mouseX > 192 && mouseX < 233) {
				updateVolume(Audio::Mixer::kMusicSoundType, mouseY);
				musicVolume = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4);
			}
		}

	}

	updateEvents();
}

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
		sav->readLine(names[n], 23);
	delete sav;

	loadPic("savescr.alg", drawSurface1, HALF_PAL);

	color_abc(kColorLightGreen);

	select[0] = 0;

	for (;;) {
		y = 27;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(names[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		setCursorTable();
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
			}

			if (mouseX > 125 && mouseY > 123 && mouseX < 199 && mouseY < 149 && selectionMade == 1) {
				if (!loadGame(file))
					return false;
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
				print_abc("elige una partida", 117, 15);
			}
			updateScreen();
			delay(200);
		}
		y = 26;

		delay(5);
	}

	clearRoom();
	loadPic(roomNumber, drawSurface1, HALF_PAL);
	selectionMade = 0;

	return true;
}

void DrasculaEngine::print_abc(const char *said, int screenX, int screenY) {
	int textPos[8];
	int letterY = 0, letterX = 0, h, length;
	length = strlen(said);

	for (h = 0; h < length; h++) {
		int c = toupper(said[h]);

		for (int i = 0; i < CHARMAP_SIZE; i++) {
			if (c == charMap[i].inChar) {
				letterX = charMap[i].mappedChar;

				switch (charMap[i].charType) {
					case 0:		// letters
						letterY = (_lang == kSpanish) ? 149 : 158;
						break;
					case 1:		// signs
						letterY = (_lang == kSpanish) ? 160 : 169;
						break;
					case 2:		// accented
						letterY = 180;
						break;
				}	// switch
				break;
			}	// if
		}	// for

		textPos[0] = letterX;
		textPos[1] = letterY;
		textPos[2] = screenX;
		textPos[3] = screenY;
		textPos[4] = CHAR_WIDTH;
		textPos[5] = CHAR_HEIGHT;

		copyRectClip(textPos, tableSurface, screenSurface);

		screenX = screenX + CHAR_WIDTH;
		if (screenX > 317) {
			screenX = 0;
			screenY = screenY + CHAR_HEIGHT + 2;
		}
	}	// for
}

void DrasculaEngine::print_abc_opc(const char *said, int screenX, int screenY, int game) {
	int textPos[6];
	int signY, letterY, letterX = 0, h, length;
	length = strlen(said);

	for (h = 0; h < length; h++) {
		if (game == 1) {
			letterY = 6;
			signY = 15;
		} else if (game == 3) {
			letterY = 56;
			signY = 65;
		} else {
			letterY = 31;
			signY = 40;
		}

		int c = toupper(said[h]);

		for (int i = 0; i < CHARMAP_SIZE; i++) {
			if (c == charMap[i].inChar) {
				// Convert the mapped char of the normal font to the
				// mapped char of the dialogue font

				int multiplier = (charMap[i].mappedChar - 6) / 9;

				letterX = multiplier * 7 + 10;

				if (charMap[i].charType > 0)
					letterY = signY;
				break;
			}	// if
		}	// for

		textPos[0] = letterX;
		textPos[1] = letterY;
		textPos[2] = screenX;
		textPos[3] = screenY;
		textPos[4] = CHAR_WIDTH_OPC;
		textPos[5] = CHAR_HEIGHT_OPC;

		copyRectClip(textPos, backSurface, screenSurface);

		screenX = screenX + CHAR_WIDTH_OPC;
	}
}

void DrasculaEngine::delay(int ms) {
	_system->delayMillis(ms * 2); // originaly was 1
}

bool DrasculaEngine::confirmExit() {
	byte key;

	color_abc(kColorRed);
	updateRoom();
	centerText(_textsys[_lang][1], 160, 87);
	updateScreen();

	delay(100);
	for (;;) {
		key = getScan();
		if (key != 0)
			break;
	}

	if (key == Common::KEYCODE_ESCAPE) {
		stopMusic();
		return false;
	}

	return true;
}

void DrasculaEngine::screenSaver() {
	int xr, yr;
	byte *copia, *ghost;
	float coeff = 0, coeff2 = 0;
	int count = 0;
	int count2 = 0;
	int tempLine[320];
	int tempRow[200];

	clearRoom();

	loadPic("sv.alg", drawSurface1, HALF_PAL);

	// inicio_ghost();
	copia = (byte *)malloc(64000);
	ghost = (byte *)malloc(65536);

	// carga_ghost();
	_arj.open("ghost.drv");
	if (!_arj.isOpen())
		error("Cannot open file ghost.drv");

	_arj.read(ghost, 65536);
	_arj.close();

	updateEvents();
	xr = mouseX;
	yr = mouseY;

	for (;;) {
		// efecto(drawSurface1);

		memcpy(copia, drawSurface1, 64000);
		coeff += 0.1f;
		coeff2 = coeff;

		if (++count > 319)
			count = 0;

		for (int i = 0; i < 320; i++) {
			tempLine[i] = (int)(sin(coeff2) * 16);
			coeff2 += 0.02f;
			tempLine[i] = checkWrapY(tempLine[i]);
		}

		coeff2 = coeff;
		for (int i = 0; i < 200; i++) {
			tempRow[i] = (int)(sin(coeff2) * 16);
			coeff2 += 0.02f;
			tempRow[i] = checkWrapX(tempRow[i]);
		}

		if (++count2 > 199)
			count2 = 0;

		int x1_, y1_, off1, off2;

		for (int i = 0; i < 200; i++) {
			for (int j = 0; j < 320; j++) {
				x1_ = j + tempRow[i];
				x1_ = checkWrapX(x1_);

				y1_ = i + count2;
				y1_ = checkWrapY(y1_);

				off1 = 320 * y1_ + x1_;

				x1_ = j + count;
				x1_ = checkWrapX(x1_);

				y1_ = i + tempLine[j];
				y1_ = checkWrapY(y1_);
				off2 = 320 * y1_ + x1_;

				VGA[320 * i + j] = ghost[drawSurface1[off2] + (copia[off1] << 8)];
			}
		}
		_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		_system->updateScreen();

		_system->delayMillis(20);

		// end of efecto()

		updateEvents();
		if (rightMouseButton == 1 || leftMouseButton == 1)
			break;
		if (mouseX != xr)
			break;
		if (mouseY != yr)
			break;
	}
	// fin_ghost();
	free(copia);
	free(ghost);

	loadPic(roomNumber, drawSurface1, HALF_PAL);
}

void DrasculaEngine::playFLI(const char *filefli, int vel) {
	// Open file
	MiVideoSSN = (byte *)malloc(64256);
	globalSpeed = 1000 / vel;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	_arj.open(filefli);
	mSession = TryInMem();
	LastFrame = _system->getMillis();

	while (playFrameSSN() && (!term_int)) {
		if (getScan() == Common::KEYCODE_ESCAPE)
			term_int = 1;
	}

	free(MiVideoSSN);
	if (UsingMem)
		free(pointer);
	else
		_arj.close();
}

void DrasculaEngine::fadeFromBlack(int fadeSpeed) {
	char fade;
	unsigned int color, component;

	DacPalette256 palFade;

	for (fade = 0; fade < 64; fade++) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFade[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fade);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFade);
	}
}

void DrasculaEngine::color_abc(int cl) {
	_color = cl;

	char colorTable[][3] = {
		{    0,    0,    0 }, { 0x10, 0x3E, 0x28 },
		{    0,    0,    0 },	// unused
		{ 0x16, 0x3F, 0x16 }, { 0x09, 0x3F, 0x12 },
		{ 0x3F, 0x3F, 0x15 },
		{    0,    0,    0 },	// unused
		{ 0x38,    0,    0 }, { 0x3F, 0x27, 0x0B },
		{ 0x2A,    0, 0x2A }, { 0x30, 0x30, 0x30 },
		{   98,   91,  100 }
	};

	for (int i = 0; i <= 2; i++)
		gamePalette[254][i] = colorTable[cl][i];

	setPalette((byte *)&gamePalette);
}

char DrasculaEngine::adjustToVGA(char value) {
	return (value & 0x3F) * (value > 0);
}

void DrasculaEngine::centerText(const char *message, int textX, int textY) {
	char bb[200], m2[200], m1[200], mb[10][50];
	char m3[200];
	int h, fil, textX3, textX2, textX1, conta_f = 0, ya = 0;

	strcpy(m1, " ");
	strcpy(m2, " ");
	strcpy(m3, " ");
	strcpy(bb, " ");

	for (h = 0; h < 10; h++)
		strcpy(mb[h], " ");

	if (textX > 160)
		ya = 1;

	strcpy(m1, message);
	textX = CLIP<int>(textX, 60, 255);

	textX1 = textX;

	if (ya == 1)
		textX1 = 315 - textX;

	textX2 = (strlen(m1) / 2) * CHAR_WIDTH;

	while (true) {
		strcpy(bb, m1);
		scumm_strrev(bb);

		if (textX1 < textX2) {
			strcpy(m3, strrchr(m1, ' '));
			strcpy(m1, strstr(bb, " "));
			scumm_strrev(m1);
			m1[strlen(m1) - 1] = '\0';
			strcat(m3, m2);
			strcpy(m2, m3);
		};

		textX2 = (strlen(m1) / 2) * CHAR_WIDTH;

		if (textX1 < textX2)
			continue;

		strcpy(mb[conta_f], m1);

		if (!strcmp(m2, ""))
			break;

		scumm_strrev(m2);
		m2[strlen(m2) - 1] = '\0';
		scumm_strrev(m2);
		strcpy(m1, m2);
		strcpy(m2, "");
		conta_f++;
	}

	fil = textY - (((conta_f + 3) * CHAR_HEIGHT));

	for (h = 0; h < conta_f + 1; h++) {
		textX3 = strlen(mb[h]) / 2;
		print_abc(mb[h], ((textX) - textX3 * CHAR_WIDTH) - 1, fil);
		fil = fil + CHAR_HEIGHT + 2;
	}
}

void DrasculaEngine::playSound(int soundNum) {
	char file[20];
	sprintf(file, "s%i.als", soundNum);

	playFile(file);
}

bool DrasculaEngine::animate(const char *animationFile, int FPS) {
	unsigned j;
	int NFrames = 1;
	int cnt = 2;
	int dataSize = 0;

	AuxBuffLast = (byte *)malloc(65000);
	AuxBuffDes = (byte *)malloc(65000);

	_arj.open(animationFile);

	if (!_arj.isOpen()) {
		error("Animation file %s not found", animationFile);
	}

	NFrames = _arj.readSint32LE();
	dataSize = _arj.readSint32LE();
	AuxBuffOrg = (byte *)malloc(dataSize);
	_arj.read(AuxBuffOrg, dataSize);
	_arj.read(cPal, 768);
	loadPCX(AuxBuffOrg);
	free(AuxBuffOrg);
	memcpy(VGA, AuxBuffDes, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
	setPalette(cPal);
	memcpy(AuxBuffLast, AuxBuffDes, 64000);
	WaitForNext(FPS);
	while (cnt < NFrames) {
		dataSize = _arj.readSint32LE();
		AuxBuffOrg = (byte *)malloc(dataSize);
		_arj.read(AuxBuffOrg, dataSize);
		_arj.read(cPal, 768);
		loadPCX(AuxBuffOrg);
		free(AuxBuffOrg);
		for (j = 0;j < 64000; j++) {
			VGA[j] = AuxBuffLast[j] = AuxBuffDes[j] ^ AuxBuffLast[j];
		}
		_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		_system->updateScreen();
		WaitForNext(FPS);
		cnt++;
		byte key = getScan();
		if (key == Common::KEYCODE_ESCAPE)
			term_int = 1;
		if (key != 0)
			break;
	}
	free(AuxBuffLast);
	free(AuxBuffDes);
	_arj.close();

	return ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE));
}

void DrasculaEngine::fadeToBlack(int fadeSpeed) {
	char fade;
	unsigned int color, component;

	DacPalette256 palFade;

	for (fade = 63; fade >= 0; fade--) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFade[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fade);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFade);
	}
}

void DrasculaEngine::pause(int duration) {
	_system->delayMillis(duration * 30); // was originaly 2
}

void DrasculaEngine::placeIgor() {
	int pos_igor[6];

	pos_igor[0] = 1;
	if (currentChapter == 4) {
		pos_igor[1] = 138;
	} else {
		if (trackIgor == 3)
			pos_igor[1] = 138;
		else if (trackIgor == 1)
			pos_igor[1] = 76;
	}
	pos_igor[2] = igorX;
	pos_igor[3] = igorY;
	pos_igor[4] = 54;
	pos_igor[5] = 61;

	copyRectClip(pos_igor, frontSurface, screenSurface);
}

void DrasculaEngine::placeDrascula() {
	int pos_dr[6];

	if (trackDrascula == 1)
		pos_dr[0] = 47;
	else if (trackDrascula == 0)
		pos_dr[0] = 1;
	else if (trackDrascula == 3 && currentChapter == 1)
		pos_dr[0] = 93;
	pos_dr[1] = 122;
	pos_dr[2] = drasculaX;
	pos_dr[3] = drasculaY;
	pos_dr[4] = 45;
	pos_dr[5] = 77;

	if (currentChapter == 6)
		copyRectClip(pos_dr, drawSurface2, screenSurface);
	else
		copyRectClip(pos_dr, backSurface, screenSurface);
}

void DrasculaEngine::placeBJ() {
	int pos_bj[6];

	if (trackBJ == 3)
		pos_bj[0] = 10;
	else if (trackBJ == 0)
		pos_bj[0] = 37;
	pos_bj[1] = 99;
	pos_bj[2] = bjX;
	pos_bj[3] = bjY;
	pos_bj[4] = 26;
	pos_bj[5] = 76;

	copyRectClip(pos_bj, drawSurface3, screenSurface);
}

void DrasculaEngine::hiccup(int counter) {
	int y = 0, trackCharacter = 0;
	if (currentChapter == 3)
		y = -1;

	do {
		counter--;

		updateRoom();
		if (currentChapter == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (trackCharacter == 0)
			y++;
		else
			y--;

		if (currentChapter == 3) {
			if (y == 1)
				trackCharacter = 1;
			if (y == -1)
				trackCharacter = 0;
		} else {
			if (y == 2)
				trackCharacter = 1;
			if (y == 0)
				trackCharacter = 0;
		}
	} while (counter > 0);

	updateRoom();
	updateScreen();
}

void DrasculaEngine::finishSound() {
	delay(1);

	while (soundIsActive())
		_system->delayMillis(10);
}

void DrasculaEngine::playMusic(int p) {
	AudioCD.stop();
	AudioCD.play(p - 1, 1, 0, 0);
}

void DrasculaEngine::stopMusic() {
	AudioCD.stop();
}

int DrasculaEngine::musicStatus() {
	return AudioCD.isPlaying();
}

void DrasculaEngine::updateRoom() {
	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateRefresh_pre();
	if (currentChapter == 3) {
		if (flags[0] == 0)
			moveCharacters();
		else
			copyRect(113, 54, curX - 20, curY - 1, 77, 89, drawSurface3, screenSurface);
	} else {
		moveCharacters();
	}
	updateRefresh();
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
		hay_que_load = 1;
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
	hay_que_load = 0;
	sscanf(currentData, "%d.ald", &roomNum);
	enterRoom(roomNum);
	withoutVerb();

	return true;
}

void DrasculaEngine::updateDoor(int doorNum) {
	if (currentChapter == 1 || currentChapter == 3 || currentChapter == 5 || currentChapter == 6)
		return;
	else if (currentChapter == 2) {
		if (objectNum[doorNum] == 138)
			isDoor[doorNum] = flags[0];
		else if (objectNum[doorNum] == 136)
			isDoor[doorNum] = flags[8];
		else if (objectNum[doorNum] == 156)
			isDoor[doorNum] = flags[16];
		else if (objectNum[doorNum] == 163)
			isDoor[doorNum] = flags[17];
		else if (objectNum[doorNum] == 177)
			isDoor[doorNum] = flags[15];
		else if (objectNum[doorNum] == 175)
			isDoor[doorNum] = flags[40];
		else if (objectNum[doorNum] == 173)
			isDoor[doorNum] = flags[36];
	} else if (currentChapter == 4) {
		if (objectNum[doorNum] == 101 && flags[0] == 0)
			isDoor[doorNum] = 0;
		else if (objectNum[doorNum] == 101 && flags[0] == 1 && flags[28] == 1)
			isDoor[doorNum] = 1;
		else if (objectNum[doorNum] == 103)
			isDoor[doorNum] = flags[0];
		else if (objectNum[doorNum] == 104)
			isDoor[doorNum] = flags[1];
		else if (objectNum[doorNum] == 105)
			isDoor[doorNum] = flags[1];
		else if (objectNum[doorNum] == 106)
			isDoor[doorNum] = flags[2];
		else if (objectNum[doorNum] == 107)
			isDoor[doorNum] = flags[2];
		else if (objectNum[doorNum] == 110)
			isDoor[doorNum] = flags[6];
		else if (objectNum[doorNum] == 114)
			isDoor[doorNum] = flags[4];
		else if (objectNum[doorNum] == 115)
			isDoor[doorNum] = flags[4];
		else if (objectNum[doorNum] == 116 && flags[5] == 0)
			isDoor[doorNum] = 0;
		else if (objectNum[doorNum] == 116 && flags[5] == 1 && flags[23] == 1)
			isDoor[doorNum] = 1;
		else if (objectNum[doorNum] == 117)
			isDoor[doorNum] = flags[5];
		else if (objectNum[doorNum] == 120)
			isDoor[doorNum] = flags[8];
		else if (objectNum[doorNum] == 122)
			isDoor[doorNum] = flags[7];
	}
}

void DrasculaEngine::assignDefaultPalette() {
	int color, component;

	for (color = 235; color < 253; color++)
		for (component = 0; component < 3; component++)
			defaultPalette[color][component] = gamePalette[color][component];
}

void DrasculaEngine::assignBrightPalette() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			brightPalette[color][component] = gamePalette[color][component];
	}
}

void DrasculaEngine::assignDarkPalette() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			darkPalette[color][component] = gamePalette[color][component];
	}
}

void DrasculaEngine::setDefaultPalette() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++) {
			gamePalette[color][component] = defaultPalette[color][component];
		}
	}
	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::setBrightPalette() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			gamePalette[color][component] = brightPalette[color][component];
	}

	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::setDarkPalette() {
	int color, component;

	for (color = 235; color < 253; color++ )
		for (component = 0; component < 3; component++)
			gamePalette[color][component] = darkPalette[color][component];

	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::setPaletteBase(int darkness) {
	char fade;
	unsigned int color, component;

	for (fade = darkness; fade >= 0; fade--) {
		for (color = 235; color < 253; color++) {
			for (component = 0; component < 3; component++)
				gamePalette[color][component] = adjustToVGA(gamePalette[color][component] - 8 + fade);
		}
	}

	setPalette((byte *)&gamePalette);
}

void DrasculaEngine::startWalking() {
	characterMoved = 1;

	stepX = STEP_X;
	stepY = STEP_Y;

	if (currentChapter == 2) {
		if ((roomX < curX) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth) && (roomY > (curY + curHeight)))
			quadrant_4();
		else if (roomY < curY + curHeight)
			walkUp();
		else if (roomY > curY + curHeight)
			walkDown();
	} else {
		if ((roomX < curX + curWidth / 2 ) && (roomY <= (curY + curHeight)))
			quadrant_1();
		else if ((roomX < curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_3();
		else if ((roomX > curX + curWidth / 2) && (roomY <= (curY + curHeight)))
			quadrant_2();
		else if ((roomX > curX + curWidth / 2) && (roomY > (curY + curHeight)))
			quadrant_4();
		else
			characterMoved = 0;
	}
	startTime = getTime();
}

void DrasculaEngine::moveCharacters() {
	int curPos[6];
	int r;

	if (characterMoved == 1 && stepX == STEP_X) {
		for (r = 0; r < stepX; r++) {
			if (currentChapter != 2) {
				if (trackProtagonist == 0 && roomX - r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth / 2) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth / 2;
					curY = roomY - curHeight;
				}
			} else if (currentChapter == 2) {
				if (trackProtagonist == 0 && roomX - r == curX) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
				}
				if (trackProtagonist == 1 && roomX + r == curX + curWidth) {
					characterMoved = 0;
					stepX = STEP_X;
					stepY = STEP_Y;
					curX = roomX - curWidth + 4;
					curY = roomY - curHeight;
				}
			}
		}
	}
	if (characterMoved == 1 && stepY == STEP_Y) {
		for (r = 0; r < stepY; r++) {
			if (trackProtagonist == 2 && roomY - r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
			if (trackProtagonist == 3 && roomY + r == curY + curHeight) {
				characterMoved = 0;
				stepX = STEP_X;
				stepY = STEP_Y;
			}
		}
	}

	if (currentChapter == 1 || currentChapter == 4 || currentChapter == 5 || currentChapter == 6) {
		if (hare_se_ve == 0) {
			increaseFrameNum();
			return;
		}
	}

	if (characterMoved == 0) {
		curPos[0] = 0;
		curPos[1] = DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}

		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRectClip(curPos, backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRectClip(curPos, frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
	} else if (characterMoved == 1) {
		curPos[0] = frameX[num_frame];
		curPos[1] = frame_y + DIF_MASK_HARE;
		curPos[2] = curX;
		curPos[3] = curY;
		if (currentChapter == 2) {
			curPos[4] = curWidth;
			curPos[5] = curHeight;
		} else {
			curPos[4] = CHARACTER_WIDTH;
			curPos[5] = CHARACTER_HEIGHT;
		}
		if (trackProtagonist == 0) {
			curPos[1] = 0;
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 1) {
			if (currentChapter == 2)
				copyRectClip(curPos, extraSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], extraSurface, screenSurface);
		} else if (trackProtagonist == 2) {
			if (currentChapter == 2)
				copyRectClip(curPos, backSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], backSurface, screenSurface);
		} else {
			if (currentChapter == 2)
				copyRectClip(curPos, frontSurface, screenSurface);
			else
				reduce_hare_chico(curPos[0], curPos[1], curPos[2], curPos[3], curPos[4], curPos[5],
									factor_red[curY + curHeight], frontSurface, screenSurface);
		}
		increaseFrameNum();
	}
}

void DrasculaEngine::showMenu() {
	int h, n, x;
	char textIcon[13];

	x = whichObject();
	strcpy(textIcon, iconName[x]);

	for (n = 1; n < 43; n++) {
		h = inventoryObjects[n];

		if (h != 0) {
			if (currentChapter == 6)
				copyBackground(x_pol[n], y_pol[n], itemLocations[n].x, itemLocations[n].y,
						OBJWIDTH, OBJHEIGHT, tableSurface, screenSurface);
			else
				copyBackground(x_pol[n], y_pol[n], itemLocations[n].x, itemLocations[n].y,
						OBJWIDTH, OBJHEIGHT, frontSurface, screenSurface);
		}
		copyRect(x1d_menu[h], y1d_menu[h], itemLocations[n].x, itemLocations[n].y,
				OBJWIDTH, OBJHEIGHT, backSurface, screenSurface);
	}

	if (x < 7)
		print_abc(textIcon, itemLocations[x].x - 2, itemLocations[x].y - 7);
}

void DrasculaEngine::clearMenu() {
	int n, verbActivated = 1;

	for (n = 0; n < 7; n++) {
		if (mouseX > verbBarX[n] && mouseX < verbBarX[n + 1])
			verbActivated = 0;
		copyRect(OBJWIDTH * n, OBJHEIGHT * verbActivated, verbBarX[n], 2,
						OBJWIDTH, OBJHEIGHT, backSurface, screenSurface);
		verbActivated = 1;
	}
}

void DrasculaEngine::removeObject() {
	int h = 0, n;

	updateRoom();

	for (n = 1; n < 43; n++){
		if (whichObject() == n) {
			h = inventoryObjects[n];
			inventoryObjects[n] = 0;
			if (h != 0)
				takeObject = 1;
		}
	}

	updateEvents();

	if (takeObject == 1)
		chooseObject(h);
}

bool DrasculaEngine::exitRoom(int l) {
	debug(2, "Exiting room from door %d", l);

	int roomNum = 0;

	if (currentChapter == 1) {
		if (objectNum[l] == 105 && flags[0] == 0)
			talk(442);
		else {
			updateDoor(l);
			if (isDoor[l] != 0) {
				gotoObject(roomObjX[l], roomObjY[l]);
				trackProtagonist = trackObj[l];
				updateRoom();
				updateScreen();
				characterMoved = 0;
				trackProtagonist = trackCharacter_alkeva[l];
				objExit = alapuertakeva[l];
				doBreak = 1;
				previousMusic = roomMusic;

				if (objectNum[l] == 105) {
					animation_2_1();
					return true;
				}
				clearRoom();
				sscanf(_targetSurface[l], "%d", &roomNum);
				curX = -1;
				enterRoom(roomNum);
			}
		}
	} else if (currentChapter == 2) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			gotoObject(roomObjX[l], roomObjY[l]);
			characterMoved = 0;
			trackProtagonist = trackCharacter_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			if (objectNum[l] == 136)
				animation_2_2();
			if (objectNum[l] == 124)
				animation_3_2();
			if (objectNum[l] == 173) {
				animation_35_2();
				return true;
			} if (objectNum[l] == 146 && flags[39] == 1) {
				flags[5] = 1;
				flags[11] = 1;
			}
			if (objectNum[l] == 176 && flags[29] == 1) {
				flags[29] = 0;
				removeObject(kItemEarWithEarPlug);
				addObject(kItemEarplugs);
			}
			clearRoom();
			sscanf(_targetSurface[l], "%d", &roomNum);
			curX =- 1;
			enterRoom(roomNum);
		}
	} else if (currentChapter == 3) {
		updateDoor(l);
		if (isDoor[l] != 0 && visible[l] == 1) {
			gotoObject(roomObjX[l], roomObjY[l]);
			trackProtagonist = trackObj[l];
			updateRoom();
			updateScreen();
			characterMoved = 0;
			trackProtagonist = trackCharacter_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			clearRoom();
			sscanf(_targetSurface[l], "%d", &roomNum);
			curX =- 1;
			enterRoom(roomNum);
		}
	} else if (currentChapter == 4) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			gotoObject(roomObjX[l], roomObjY[l]);
			trackProtagonist = trackObj[l];
			updateRoom();
			updateScreen();
			characterMoved = 0;
			trackProtagonist = trackCharacter_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;

			if (objectNum[l] == 108)
				gotoObject(171, 78);
			clearRoom();
			sscanf(_targetSurface[l], "%d", &roomNum);
			curX = -1;
			enterRoom(roomNum);
		}
	} else if (currentChapter == 5) {
		updateDoor(l);
		if (isDoor[l] != 0 && visible[l] == 1) {
			gotoObject(roomObjX[l], roomObjY[l]);
			trackProtagonist = trackObj[l];
			updateRoom();
			updateScreen();
			characterMoved = 0;
			trackProtagonist = trackCharacter_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			hare_se_ve = 1;
			clearRoom();
			sscanf(_targetSurface[l], "%d", &roomNum);
			curX = -1;
			enterRoom(roomNum);
		}
	} else if (currentChapter == 6) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			gotoObject(roomObjX[l], roomObjY[l]);
			trackProtagonist = trackObj[l];
			updateRoom();
			updateScreen();
			characterMoved = 0;
			trackProtagonist = trackCharacter_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			clearRoom();
			sscanf(_targetSurface[l], "%d", &roomNum);
			curX = -1;
			enterRoom(roomNum);

			if (objExit == 105)
				animation_19_6();
		}
	}

	return false;
}

bool DrasculaEngine::pickupObject() {
	int h = pickedObject;
	checkFlags = 1;

	updateRoom();
	showMenu();
	updateScreen();

	// Objects with an ID smaller than 7 are the inventory verbs
	if (pickedObject >= 7) {
		for (int n = 1; n < 43; n++) {
			if (whichObject() == n && inventoryObjects[n] == 0) {
				inventoryObjects[n] = h;
				takeObject = 0;
				checkFlags = 0;
			}
		}
	}

	if (checkFlags == 1) {
		if (checkMenuFlags())
			return true;
	}
	updateEvents();
	if (takeObject == 0)
		withoutVerb();

	return false;
}

void DrasculaEngine::setCursorTable() {
	int cursorPos[8];

	cursorPos[0] = 225;
	cursorPos[1] = 56;
	cursorPos[2] = mouseX - 20;
	cursorPos[3] = mouseY - 12;
	cursorPos[4] = 40;
	cursorPos[5] = 25;

	copyRectClip(cursorPos, tableSurface, screenSurface);
}

void DrasculaEngine::enterName() {
	Common::KeyCode key;
	int v = 0, h = 0;
	char select2[23];
	strcpy(select2, "                      ");
	for (;;) {
		select2[v] = '-';
		copyBackground(115, 14, 115, 14, 176, 9, drawSurface1, screenSurface);
		print_abc(select2, 117, 15);
		updateScreen();
		key = getScan();
		delay(70);
		if (key != 0) {
			if (key >= 0 && key <= 0xFF && isalpha(key))
				select2[v] = tolower(key);
			else if ((key == Common::KEYCODE_LCTRL) || (key == Common::KEYCODE_RCTRL))
				select2[v] = '\164';
			else if (key >= Common::KEYCODE_0 && key <= Common::KEYCODE_9)
				select2[v] = key;
			else if (key == Common::KEYCODE_SPACE)
				select2[v] = '\167';
			else if (key == Common::KEYCODE_ESCAPE)
				break;
			else if (key == Common::KEYCODE_RETURN) {
				select2[v] = '\0';
				h = 1;
				break;
			} else if (key == Common::KEYCODE_BACKSPACE)
				select2[v] = '\0';
			else
				v--;

			if (key == Common::KEYCODE_BACKSPACE)
				v--;
			else
				v++;
		}
		if (v == 22)
			v = 21;
		else if (v == -1)
			v = 0;
	}
	if (h == 1) {
		strcpy(select, select2);
		selectionMade = 1;
	}
}

int DrasculaEngine::playFrameSSN() {
	int Exit = 0;
	uint32 Lengt;
	byte *BufferSSN;

	if (!UsingMem)
		_arj.read(&CHUNK, 1);
	else {
		memcpy(&CHUNK, mSession, 1);
		mSession += 1;
	}

	switch (CHUNK) {
	case kFrameSetPal:
		if (!UsingMem)
			_arj.read(dacSSN, 768);
		else {
			memcpy(dacSSN, mSession, 768);
			mSession += 768;
		}
		setPalette(dacSSN);
		break;
	case kFrameEmptyFrame:
		WaitFrameSSN();
		break;
	case kFrameInit:
		if (!UsingMem) {
			CMP = _arj.readByte();
			Lengt = _arj.readUint32LE();
		} else {
			memcpy(&CMP, mSession, 1);
			mSession += 1;
			Lengt = READ_LE_UINT32(mSession);
			mSession += 4;
		}
		if (CMP == kFrameCmpRle) {
			if (!UsingMem) {
				BufferSSN = (byte *)malloc(Lengt);
				_arj.read(BufferSSN, Lengt);
			} else {
				BufferSSN = (byte *)malloc(Lengt);
				memcpy(BufferSSN, mSession, Lengt);
				mSession += Lengt;
			}
			Des_RLE(BufferSSN, MiVideoSSN);
			free(BufferSSN);
			if (FrameSSN) {
				WaitFrameSSN();
				MixVideo(VGA, MiVideoSSN);
				_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
			} else {
				WaitFrameSSN();
				memcpy(VGA, MiVideoSSN, 64000);
				_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
			}
			_system->updateScreen();
			FrameSSN++;
		} else {
			if (CMP == kFrameCmpOff) {
				if (!UsingMem) {
					BufferSSN = (byte *)malloc(Lengt);
					_arj.read(BufferSSN, Lengt);
				} else {
					BufferSSN = (byte *)malloc(Lengt);
					memcpy(BufferSSN, mSession, Lengt);
					mSession += Lengt;
				}
				Des_OFF(BufferSSN, MiVideoSSN, Lengt);
				free(BufferSSN);
				if (FrameSSN) {
					WaitFrameSSN();
					MixVideo(VGA, MiVideoSSN);
					_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
				} else {
					WaitFrameSSN();
					memcpy(VGA, MiVideoSSN, 64000);
					_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
				}
				_system->updateScreen();
				FrameSSN++;
			}
		}
		break;
	case kFrameEndAnim:
		Exit = 1;
		break;
	default:
		Exit = 1;
		break;
	}

	return (!Exit);
}

byte *DrasculaEngine::TryInMem() {
	int Lengt;

	_arj.seek(0, SEEK_END);
	Lengt = _arj.pos();
	_arj.seek(0, SEEK_SET);
	pointer = (byte *)malloc(Lengt);
	if (pointer == NULL)
		return NULL;
	_arj.read(pointer, Lengt);
	UsingMem = 1;
	_arj.close();

	return pointer;
}

void DrasculaEngine::Des_OFF(byte *BufferOFF, byte *MiVideoOFF, int Lenght) {
	int x = 0;
	unsigned char Reps;
	int Offset;

	memset(MiVideoSSN, 0, 64000);
	while (x < Lenght) {
		Offset = BufferOFF[x] + BufferOFF[x + 1] * 256;
		Reps = BufferOFF[x + 2];
		memcpy(MiVideoOFF + Offset, &BufferOFF[x + 3], Reps);
		x += 3 + Reps;
	}
}

void DrasculaEngine::Des_RLE(byte *BufferRLE, byte *MiVideoRLE) {
	signed int con = 0;
	unsigned int X = 0;
	unsigned int fExit = 0;
	char ch, rep;
	while (!fExit) {
		ch = *BufferRLE++;
		rep = 1;
		if ((ch & 192) == 192) {
			rep = (ch & 63);
			ch =* BufferRLE++;
		}
		for (con = 0; con < rep; con++) {
			*MiVideoRLE++ = ch;
			X++;
			if (X > 64000)
				fExit = 1;
		}
	}
}

void DrasculaEngine::MixVideo(byte *OldScreen, byte *NewScreen) {
	int x;
	for (x = 0; x < 64000; x++)
		OldScreen[x] ^= NewScreen[x];
}

void DrasculaEngine::WaitFrameSSN() {
	uint32 now;
	while ((now = _system->getMillis()) - LastFrame < ((uint32) globalSpeed))
		_system->delayMillis(globalSpeed - (now - LastFrame));
	LastFrame = LastFrame + globalSpeed;
}

byte *DrasculaEngine::loadPCX(byte *NamePcc) {
	signed int con = 0;
	unsigned int X = 0;
	unsigned int fExit = 0;
	char ch, rep;
	byte *AuxPun;

	AuxPun = AuxBuffDes;

	while (!fExit) {
		ch = *NamePcc++;
		rep = 1;
		if ((ch & 192) == 192) {
			rep = (ch & 63);
			ch = *NamePcc++;
		}
		for (con = 0; con< rep; con++) {
			*AuxPun++ = ch;
			X++;
			if (X > 64000)
				fExit = 1;
		}
	}
	return AuxBuffDes;
}

void DrasculaEngine::WaitForNext(int FPS) {
	_system->delayMillis(1000 / FPS);
}

int DrasculaEngine::getTime() {
	return _system->getMillis() / 20; // originaly was 1
}

void DrasculaEngine::reduce_hare_chico(int xx1, int yy1, int xx2, int yy2, int width, int height, int factor, byte *dir_inicio, byte *dir_fin) {
	float totalX, totalY;
	int n, m;
	float pixelX, pixelY;
	int pixelPos[6];

	newWidth = (width * factor) / 100;
	newHeight = (height * factor) / 100;

	totalX = width / newWidth;
	totalY = height / newHeight;

	pixelX = xx1;
	pixelY = yy1;

	for (n = 0; n < newHeight; n++) {
		for (m = 0; m < newWidth; m++) {
			pixelPos[0] = (int)pixelX;
			pixelPos[1] = (int)pixelY;
			pixelPos[2] = xx2 + m;
			pixelPos[3] = yy2 + n;
			pixelPos[4] = 1;
			pixelPos[5] = 1;

			copyRectClip(pixelPos, dir_inicio, dir_fin);

			pixelX = pixelX + totalX;
		}
		pixelX = xx1;
		pixelY = pixelY + totalY;
	}
}

void DrasculaEngine::quadrant_1() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = 0;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 7;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_2() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = abs(curX + curWidth - roomX);
	else
		distanceX = abs(curX + curWidth / 2 - roomX);

	distanceY = (curY + curHeight) - roomY;

	if (distanceX < distanceY) {
		curDirection = 1;
		trackProtagonist = 2;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 2;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_3() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = curX - roomX;
	else
		distanceX = curX + curWidth / 2 - roomX;

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = 5;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 6;
		trackProtagonist = 0;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::quadrant_4() {
	float distanceX, distanceY;

	if (currentChapter == 2)
		distanceX = abs(curX + curWidth - roomX);
	else
		distanceX = abs(curX + curWidth / 2 - roomX);

	distanceY = roomY - (curY + curHeight);

	if (distanceX < distanceY) {
		curDirection = 4;
		trackProtagonist = 3;
		stepX = (int)(distanceX / (distanceY / STEP_Y));
	} else {
		curDirection = 3;
		trackProtagonist = 1;
		stepY = (int)(distanceY / (distanceX / STEP_X));
	}
}

void DrasculaEngine::saveGame(char gameName[]) {
	Common::OutSaveFile *out;
	int l;

	if (!(out = _saveFileMan->openForSaving(gameName))) {
		error("no puedo abrir el archivo");
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

void DrasculaEngine::increaseFrameNum() {
	timeDiff = getTime() - startTime;

	if (timeDiff >= 6) {
		startTime = getTime();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (curDirection == 0 || curDirection == 7) {
			curX -= stepX;
			curY -= stepY;
		} else if (curDirection == 1 || curDirection == 2) {
			curX += stepX;
			curY -= stepY;
		} else if (curDirection == 3 || curDirection == 4) {
			curX += stepX;
			curY += stepY;
		} else if (curDirection == 5 || curDirection == 6) {
			curX -= stepX;
			curY += stepY;
		}
	}

	if (currentChapter != 2) {
		curY += (int)(curHeight - newHeight);
		curX += (int)(curWidth - newWidth);
		curHeight = (int)newHeight;
		curWidth = (int)newWidth;
	}
}

int DrasculaEngine::whichObject() {
	int n = 0;

	for (n = 1; n < 43; n++) {
		if (mouseX > itemLocations[n].x && mouseY > itemLocations[n].y
				&& mouseX < itemLocations[n].x + OBJWIDTH && mouseY < itemLocations[n].y + OBJHEIGHT)
			break;
	}

	return n;
}

bool DrasculaEngine::checkMenuFlags() {
	for (int n = 0; n < 43; n++) {
		if (whichObject() == n) {
			if (inventoryObjects[n] != 0 && checkAction(inventoryObjects[n]))
				return true;
		}
	}

	return false;
}

void DrasculaEngine::converse(int index) {
	char fileName[20];
	sprintf(fileName, "op_%d.cal", index);
	int h;
	int game1 = 1, game2 = 1, game3 = 1, game4 = 1;
	char phrase1[78];
	char phrase2[78];
	char phrase3[87];
	char phrase4[78];
	char sound1[13];
	char sound2[13];
	char sound3[13];
	char sound4[13];
	int length;
	int answer1;
	int answer2;
	int answer3;
	int used1 = 0;
	int used2 = 0;
	int used3 = 0;
	char buffer[256];

	breakOut = 0;

	if (currentChapter == 5)
		withoutVerb();

	_arj.open(fileName);
	if (!_arj.isOpen()) {
		error("missing data file %s", fileName);
	}
	int size = _arj.size();

	getStringFromLine(buffer, size, phrase1);
	getStringFromLine(buffer, size, phrase2);
	getStringFromLine(buffer, size, phrase3);
	getStringFromLine(buffer, size, phrase4);
	getStringFromLine(buffer, size, sound1);
	getStringFromLine(buffer, size, sound2);
	getStringFromLine(buffer, size, sound3);
	getStringFromLine(buffer, size, sound4);
	getIntFromLine(buffer, size, &answer1);
	getIntFromLine(buffer, size, &answer2);
	getIntFromLine(buffer, size, &answer3);

	_arj.close();

	if (currentChapter == 2 && !strcmp(fileName, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(phrase3, _text[_lang][405]);
		strcpy(sound3, "405.als");
		answer3 = 31;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[7] == 1) {
		strcpy(phrase3, _text[_lang][273]);
		strcpy(sound3, "273.als");
		answer3 = 14;
	}

	if (currentChapter == 6 && !strcmp(fileName, "op_12.cal") && flags[10] == 1) {
		strcpy(phrase3, " cuanto queda para que acabe el partido?");
		strcpy(sound3, "274.als");
		answer3 = 15;
	}

	length = strlen(phrase1);
	for (h = 0; h < length; h++)
		if (phrase1[h] == (char)0xa7)
			phrase1[h] = ' ';

	length = strlen(phrase2);
	for (h = 0; h < length; h++)
		if (phrase2[h] == (char)0xa7)
			phrase2[h] = ' ';

	length = strlen(phrase3);
	for (h = 0; h < length; h++)
		if (phrase3[h] == (char)0xa7)
			phrase3[h] = ' ';

	length = strlen(phrase4);
	for (h = 0; h < length; h++)
		if (phrase4[h] == (char)0xa7)
			phrase4[h] = ' ';

	loadPic("car.alg", backSurface);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this due lack backend functionality
	// from 1(top) to 31
	color_abc(kColorLightGreen);

	while (breakOut == 0) {
		updateRoom();

		if (currentChapter == 1 || currentChapter == 4 || currentChapter == 6) {
			if (musicStatus() == 0 && flags[11] == 0)
				playMusic(roomMusic);
		} else if (currentChapter == 2) {
			if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
				playMusic(roomMusic);
		} else if (currentChapter == 3 || currentChapter == 5) {
			if (musicStatus() == 0)
				playMusic(roomMusic);
		}

		updateEvents();

		if (mouseY > 0 && mouseY < 9) {
			if (used1 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used1 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (mouseY > 8 && mouseY < 17) {
			if (used2 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used2 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (mouseY > 16 && mouseY < 25) {
			if (used3 == 1 && _color != kColorWhite)
				color_abc(kColorWhite);
			else if (used3 == 0 && _color != kColorLightGreen)
				color_abc(kColorLightGreen);
		} else if (_color != kColorLightGreen)
			color_abc(kColorLightGreen);

		if (mouseY > 0 && mouseY < 9)
			game1 = 2;
		else if (mouseY > 8 && mouseY < 17)
			game2 = 2;
		else if (mouseY > 16 && mouseY < 25)
			game3 = 2;
		else if (mouseY > 24 && mouseY < 33)
			game4 = 2;

		print_abc_opc(phrase1, 1, 2, game1);
		print_abc_opc(phrase2, 1, 10, game2);
		print_abc_opc(phrase3, 1, 18, game3);
		print_abc_opc(phrase4, 1, 26, game4);

		updateScreen();

		if ((leftMouseButton == 1) && (game1 == 2)) {
			delay(100);
			used1 = 1;
			talk(phrase1, sound1);
			if (currentChapter == 3)
				grr();
			else
				response(answer1);
		} else if ((leftMouseButton == 1) && (game2 == 2)) {
			delay(100);
			used2 = 1;
			talk(phrase2, sound2);
			if (currentChapter == 3)
				grr();
			else
				response(answer2);
		} else if ((leftMouseButton == 1) && (game3 == 2)) {
			delay(100);
			used3 = 1;
			talk(phrase3, sound3);
			if (currentChapter == 3)
				grr();
			else
				response(answer3);
		} else if ((leftMouseButton == 1) && (game4 == 2)) {
			delay(100);
			talk(phrase4, sound4);
			breakOut = 1;
		}

		if (leftMouseButton == 1) {
			delay(100);
			color_abc(kColorLightGreen);
		}

		game1 = (used1 == 0) ? 1 : 3;
		game2 = (used2 == 0) ? 1 : 3;
		game3 = (used3 == 0) ? 1 : 3;
		game4 = 1;
	} // while (breakOut == 0)

	if (currentChapter == 2)
		loadPic(menuBackground, backSurface);
	else
		loadPic(99, backSurface);
	if (currentChapter != 5)
		withoutVerb();
}

void DrasculaEngine::response(int function) {
	if (currentChapter == 1) {
		if (function >= 10 && function <= 12)
			talk_drunk(function - 9);
	} else if (currentChapter == 2) {
		if (function == 8)
			animation_8_2();
		else if (function == 9)
			animation_9_2();
		else if (function == 10)
			animation_10_2();
		else if (function == 15)
			animation_15_2();
		else if (function == 16)
			animation_16_2();
		else if (function == 17)
			animation_17_2();
		else if (function == 19)
			animation_19_2();
		else if (function == 20)
			animation_20_2();
		else if (function == 21)
			animation_21_2();
		else if (function == 23)
			animation_23_2();
		else if (function == 28)
			animation_28_2();
		else if (function == 29)
			animation_29_2();
		else if (function == 30)
			animation_30_2();
		else if (function == 31)
			animation_31_2();
	} else if (currentChapter == 4) {
		if (function == 2)
			animation_2_4();
		else if (function == 3)
			animation_3_4();
		else if (function == 4)
			animation_4_4();
	} else if (currentChapter == 5) {
		if (function == 2)
			animation_2_5();
		else if (function == 3)
			animation_3_5();
		else if (function == 6)
			animation_6_5();
		else if (function == 7)
			animation_7_5();
		else if (function == 8)
			animation_8_5();
		else if (function == 15)
			animation_15_5();
		else if (function == 16)
			animation_16_5();
		else if (function == 17)
			animation_17_5();
	} else if (currentChapter == 6) {
		if (function == 2)
			animation_2_6();
		else if (function == 3)
			animation_3_6();
		else if (function == 4)
			animation_4_6();
		else if (function == 11)
			animation_11_6();
		else if (function == 12)
			animation_12_6();
		else if (function == 13)
			animation_13_6();
		else if (function == 14)
			animation_14_6();
		else if (function == 15)
			animation_15_6();
	}
}

void DrasculaEngine::addObject(int obj) {
	int h, position = 0;

	for (h = 1; h < 43; h++) {
		if (inventoryObjects[h] == obj)
			position = 1;
	}

	if (position == 0) {
		for (h = 1; h < 43; h++) {
			if (inventoryObjects[h] == 0) {
				inventoryObjects[h] = obj;
				position = 1;
				break;
			}
		}
	}
}

void DrasculaEngine::stopSound() {
	_mixer->stopHandle(_soundHandle);
}

void DrasculaEngine::MusicFadeout() {
	int org_vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	for (;;) {
		int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		vol -= 10;
			if (vol < 0)
				vol = 0;
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
		if (vol == 0)
			break;
		updateEvents();
		_system->updateScreen();
		_system->delayMillis(50);
	}
	AudioCD.stop();
	_system->delayMillis(100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, org_vol);
}

void DrasculaEngine::playFile(const char *fname) {
	if (_arj.open(fname)) {
		int soundSize = _arj.size();
		byte *soundData = (byte *)malloc(soundSize);
		_arj.seek(32);
		_arj.read(soundData, soundSize);
		_arj.close();

		_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_soundHandle, soundData, soundSize - 64,
						11025, Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);
	} else
		warning("playFile: Could not open %s", fname);
}

bool DrasculaEngine::soundIsActive() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void DrasculaEngine::updateVisible() {
	if (currentChapter == 1) {
		// nothing
	} else if (currentChapter == 2) {
		if (roomNumber == 2 && flags[40] == 0)
			visible[3] = 0;
		else if (roomNumber == 3 && flags[3] == 1)
			visible[8] = 0;
		else if (roomNumber == 6 && flags[1] == 1 && flags[10] == 0) {
			visible[2] = 0;
			visible[4] = 1;
		} else if (roomNumber == 7 && flags[35] == 1)
			visible[3] = 0;
		else if (roomNumber == 14 && flags[5] == 1)
			visible[4] = 0;
		else if (roomNumber == 18 && flags[28] == 1)
			visible[2] = 0;
	} else if (currentChapter == 3) {
		// nothing
	} else if (currentChapter == 4) {
		if (roomNumber == 23 && flags[0] == 0 && flags[11] == 0)
			visible[2] = 1;
		if (roomNumber == 23 && flags[0] == 1 && flags[11] == 0)
			visible[2] = 0;
		if (roomNumber == 21 && flags[10] == 1)
			visible[2] = 0;
		if (roomNumber == 22 && flags[26] == 1) {
			visible[2] = 0;
			visible[1] = 1;
		}
		if (roomNumber == 22 && flags[27] == 1)
			visible[3] = 0;
		if (roomNumber == 26 && flags[21] == 0)
			strcpy(objName[2], _textmisc[_lang][0]);
		if (roomNumber == 26 && flags[18] == 1)
			visible[2] = 0;
		if (roomNumber == 26 && flags[12] == 1)
			visible[1] = 0;
		if (roomNumber == 35 && flags[14] == 1)
			visible[2] = 0;
		if (roomNumber == 35 && flags[17] == 1)
			visible[3] = 1;
		if (roomNumber == 35 && flags[15] == 1)
			visible[1] = 0;
	} else if (currentChapter == 5) {
		if (roomNumber == 49 && flags[6] == 1)
			visible[2] = 0;
		if (roomNumber == 49 && flags[6] == 0)
			visible[1] = 0;
		if (roomNumber == 49 && flags[6] == 1)
			visible[1] = 1;
		if (roomNumber == 45 && flags[6] == 1)
			visible[3] = 1;
		if (roomNumber == 53 && flags[2] == 1)
			visible[3] = 0;
		if (roomNumber == 54 && flags[13] == 1)
			visible[3] = 0;
		if (roomNumber == 55 && flags[8] == 1)
			visible[1] = 0;
	} else if (currentChapter == 6) {
		if (roomNumber == 58 && flags[8] == 0)
			isDoor[1] = 0;
		if (roomNumber == 58 && flags[8] == 1)
			isDoor[1] = 1;
		if (roomNumber == 59)
			isDoor[1] = 0;
		if (roomNumber == 60) {
			trackDrascula = 0;
			drasculaX = 155;
			drasculaY = 69;
		}
	}
}

void DrasculaEngine::walkDown() {
	curDirection = 4;
	trackProtagonist = 3;
	stepX = 0;
}

void DrasculaEngine::walkUp() {
	curDirection = 0;
	trackProtagonist = 2;
	stepX = 0;
}

void DrasculaEngine::moveVonBraun() {
	int pos_vb[6];

	if (vonBraunHasMoved == 0) {
		pos_vb[0] = 256;
		pos_vb[1] = 129;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 33;
		pos_vb[5] = 69;
		if (trackVonBraun == 0)
			pos_vb[0] = 222;
		else if (trackVonBraun == 1)
			pos_vb[0] = 188;
	} else {
		pos_vb[0] = actorFrames[kFrameVonBraun];
		pos_vb[1] = (trackVonBraun == 0) ? 62 : 131;
		pos_vb[2] = vonBraunX;
		pos_vb[3] = 66;
		pos_vb[4] = 28;
		pos_vb[5] = 68;

		actorFrames[kFrameVonBraun] += 29;
		if (actorFrames[kFrameVonBraun] > 146)
			actorFrames[kFrameVonBraun] = 1;
	}

	copyRectClip(pos_vb, frontSurface, screenSurface);
}

void DrasculaEngine::placeVonBraun(int pointX) {
	trackVonBraun = (pointX < vonBraunX) ? 0 : 1;
	vonBraunHasMoved = 1;

	for (;;) {
		updateRoom();
		updateScreen();
		if (trackVonBraun == 0) {
			vonBraunX = vonBraunX - 5;
			if (vonBraunX <= pointX)
				break;
		} else {
			vonBraunX = vonBraunX + 5;
			if (vonBraunX >= pointX)
				break;
		}
		pause(5);
	}

	vonBraunHasMoved = 0;
}

void DrasculaEngine::hipo_sin_nadie(int counter){
	int y = 0, trackCharacter = 0;
	if (currentChapter == 3)
		y = -1;

	do {
		counter--;

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		if (currentChapter == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (trackCharacter == 0)
			y++;
		else
			y--;

		if (currentChapter == 3) {
			if (y == 1)
				trackCharacter = 1;
			if (y == -1)
				trackCharacter = 0;
		} else {
			if (y == 2)
				trackCharacter = 1;
			if (y == 0)
				trackCharacter = 0;
		}
	} while (counter > 0);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen();
}

void DrasculaEngine::openDoor(int nflag, int doorNum) {
	if (flags[nflag] == 0) {
		if (currentChapter == 1 /*|| currentChapter == 4*/) {
			if (nflag != 7) {
				playSound(3);
				flags[nflag] = 1;
			}
		} else {
			playSound(3);
			flags[nflag] = 1;
		}

		if (doorNum != NO_DOOR)
			updateDoor(doorNum);
		updateRoom();
		updateScreen();
		finishSound();
		withoutVerb();
	}
}

void DrasculaEngine::showMap() {
	hasName = 0;

	for (int l = 0; l < numRoomObjs; l++) {
		if (mouseX > x1[l] && mouseY > y1[l]
				&& mouseX < x2[l] && mouseY < y2[l]
				&& visible[l] == 1) {
			strcpy(textName, objName[l]);
			hasName = 1;
		}
	}
}

void DrasculaEngine::grr() {
	int length = 30;

	color_abc(kColorDarkGreen);

	playFile("s10.als");

	updateRoom();
	copyBackground(253, 110, 150, 65, 20, 30, drawSurface3, screenSurface);

	if (withVoices == 0)
		centerText(".groaaarrrrgghhh!", 153, 65);

	updateScreen();

	while (!isTalkFinished(&length));

	updateRoom();
	updateScreen();
}

void DrasculaEngine::activatePendulum() {
	flags[1] = 2;
	hare_se_ve = 0;
	roomNumber = 102;
	loadPic(102, drawSurface1, HALF_PAL);
	loadPic("an_p1.alg", drawSurface3);
	loadPic("an_p2.alg", extraSurface);
	loadPic("an_p3.alg", frontSurface);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	savedTime = getTime();
}

void DrasculaEngine::closeDoor(int nflag, int doorNum) {
	if (flags[nflag] == 1) {
		playSound(4);
		flags[nflag] = 0;
		if (doorNum != NO_DOOR)
			updateDoor(doorNum);
		updateRoom();
		updateScreen();
		finishSound();
		withoutVerb();
	}
}

} // End of namespace Drascula

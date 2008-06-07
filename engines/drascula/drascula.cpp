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

bool DrasculaEngine::runCurrentChapter() {
	int n;

	if (_lang == kSpanish)
		textSurface = extraSurface;
	else
		textSurface = tableSurface;

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

void DrasculaEngine::delay(int ms) {
	_system->delayMillis(ms * 2); // originaly was 1
}

void DrasculaEngine::pause(int duration) {
	_system->delayMillis(duration * 30); // was originaly 2
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

			pixelX += totalX;
		}
		pixelX = xx1;
		pixelY += totalY;
	}
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

} // End of namespace Drascula

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

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = drasculaSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

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

	return 0;
}

int DrasculaEngine::go() {
	num_ejec = 1; // values from 1 to 6 will start each part of game
	hay_que_load = 0;

	for (;;) {
		int i;

		VGA = (byte *)malloc(320 * 200);
		memset(VGA, 0, 64000);

		takeObject = 0;
		menuBar = 0; menuScreen = 0; hasName = 0;
		frame_y = 0;
		hare_x = -1; characterMoved = 0; sentido_hare = 3; num_frame = 0; hare_se_ve = 1;
		checkFlags = 1;
		doBreak = 0;
		walkToObject = 0;
		stepX = PASO_HARE_X; stepY = PASO_HARE_Y;
		alto_hare = CHARACTER_HEIGHT; ancho_hare = CHARACTER_WIDTH; feetHeight = PIES_HARE;
		alto_talk = ALTO_TALK_HARE; ancho_talk = ANCHO_TALK_HARE;
		hasAnswer = 0;
		conta_blind_vez = 0;
		changeColor = 0;
		rompo_y_salgo = 0;
		vb_x = 120; sentido_vb = 1; vb_se_mueve = 0; frame_vb = 1;
		frame_piano = 0;
		frame_drunk = 0;
		frame_candles = 0;
		cont_sv = 0;
		term_int = 0;
		musicStopped = 0;
		hay_seleccion = 0;
		UsingMem = 0;
		globalSpeed = 0;
		frame_blind = 0;
		frame_snore = 0;
		frame_bat = 0;
		c_mirar = 0;
		c_poder = 0;
		ald = NULL;
		sku = NULL;

		allocMemory();

		hay_sb = 1;
		withVoices = 0;
		hay_seleccion = 0;

		if (num_ejec != 6) {
			loadAndDecompressPic("95.alg", tableSurface, 1);
		}

		if (num_ejec == 1) {
			loadAndDecompressPic("96.alg", frontSurface, COMPLETE_PAL);
			loadAndDecompressPic("99.alg", backSurface, 1);
			loadAndDecompressPic("97.alg", extraSurface, 1);
		} else if (num_ejec == 2) {
			loadAndDecompressPic("96.alg", frontSurface, COMPLETE_PAL);
			loadAndDecompressPic("pts.alg", drawSurface2, 1);
		} else if (num_ejec == 3) {
			loadAndDecompressPic("aux13.alg", drawSurface1, COMPLETE_PAL);
			loadAndDecompressPic("96.alg", frontSurface, 1);
			loadAndDecompressPic("97.alg", extraSurface, 1);
			loadAndDecompressPic("99.alg", backSurface, 1);
		} else if (num_ejec == 4) {
			loadAndDecompressPic("96.alg", frontSurface, COMPLETE_PAL);
			if (hay_que_load == 0)
				animation_rayo();
			loadAndDecompressPic("96.alg", frontSurface, 1);
			clearRoom();
			loadAndDecompressPic("99.alg", backSurface, 1);
			loadAndDecompressPic("97.alg", extraSurface, 1);
		} else if (num_ejec == 5) {
			loadAndDecompressPic("96.alg", frontSurface, COMPLETE_PAL);
			loadAndDecompressPic("97.alg", extraSurface, 1);
			loadAndDecompressPic("99.alg", backSurface, 1);
		} else if (num_ejec == 6) {
			x_igor = 105, y_igor = 85, sentido_igor = 1;
			x_dr = 62, y_dr = 99, sentido_dr = 1;
			frame_pen = 0;
			flag_tv = 0;

			pendulumSurface = drawSurface3;

			loadAndDecompressPic("96.alg", frontSurface, COMPLETE_PAL);
			loadAndDecompressPic("99.alg", backSurface, 1);
			loadAndDecompressPic("97.alg", extraSurface, 1);
			loadAndDecompressPic("95.alg", tableSurface, 1);
		}
		memset(iconName, 0, sizeof(iconName));

		for (i = 0; i < 6; i++)
			strcpy(iconName[i + 1], _textverbs[_lang][i]);

		paleta_hare();
		if (!escoba()) {
			releaseGame();
			break;
		}
		releaseGame();
		if (num_ejec == 6)
			break;

		num_ejec++;
	}

	return 0;
}

void DrasculaEngine::releaseGame() {
	if (hay_sb == 1)
		ctvd_end();
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

void DrasculaEngine::loadPic(const char *NamePcc) {
	unsigned int con, x = 0;
	unsigned int fExit = 0;
	byte ch, rep;
	Common::File file;
	byte *auxPun;

	file.open(NamePcc);
	if (!file.isOpen())
		error("missing game data %s %c", NamePcc, 7);

	pcxBuffer = (byte *)malloc(65000);
	auxPun = pcxBuffer;
	file.seek(128);
	while (!fExit) {
		ch = file.readByte();
		rep = 1;
		if ((ch & 192) == 192) {
			rep = (ch & 63);
			ch = file.readByte();
		}
		for (con = 0; con < rep; con++) {
			*auxPun++ = ch;
			x++;
			if (x > 64000)
				fExit = 1;
		}
	}

	file.read(cPal, 768);
	file.close();
}

void DrasculaEngine::decompressPic(byte *targetSurface, int colorCount) {
	memcpy(targetSurface, pcxBuffer, 64000);
	free(pcxBuffer);
	setRGB((byte *)cPal, colorCount);
}

void DrasculaEngine::paleta_hare() {
	int color, component;

	for (color = 235; color < 253; color++)
		for (component = 0; component < 3; component++)
			palHare[color][component] = gamePalette[color][component];
}

void DrasculaEngine::hare_oscuro() {
	int color, component;

	for (color = 235; color < 253; color++ )
		for (component = 0; component < 3; component++)
			gamePalette[color][component] = palHareOscuro[color][component];

	updatePalette();
}

void DrasculaEngine::setRGB(byte *dir_lectura, int plt) {
	int x, cnt = 0;

	for (x = 0; x < plt; x++) {
		gamePalette[x][0] = dir_lectura[cnt++] / 4;
		gamePalette[x][1] = dir_lectura[cnt++] / 4;
		gamePalette[x][2] = dir_lectura[cnt++] / 4;
	}
	updatePalette();
}

void DrasculaEngine::black() {
	int color, component;
	DacPalette256 palNegra;

	for (color = 0; color < 256; color++)
		for (component = 0; component < 3; component++)
			palNegra[color][component] = 0;

	palNegra[254][0] = 0x3F;
	palNegra[254][1] = 0x3F;
	palNegra[254][2] = 0x15;

	setPalette((byte *)&palNegra);
}

void DrasculaEngine::updatePalette() {
	setPalette((byte *)&gamePalette);
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

bool DrasculaEngine::escoba() {
	int n;

	if (_lang == kSpanish)
		textSurface = extraSurface;
	else
		textSurface = tableSurface;

	previousMusic = -1;

	if (num_ejec != 2) {
		int soc = 0;
		for (n = 0; n < 6; n++) {
			soc = soc + CHARACTER_WIDTH;
			frame_x[n] = soc;
		}
	}

	for (n = 1; n < 43; n++)
		inventoryObjects[n] = 0;

	for (n = 0; n < NUM_FLAGS; n++)
		flags[n] = 0;

	if (num_ejec == 2) {
		flags[16] = 1;
		flags[17] = 1;
		flags[27] = 1;
	}

	for (n = 1; n < 7; n++)
		inventoryObjects[n] = n;

	if (num_ejec == 1) {
		pickObject(28);

		if (hay_que_load == 0)
			animation_1_1();

		withoutVerb();
		loadAndDecompressPic("2aux62.alg", drawSurface2, 1);
		sentido_hare = 1;
		objExit = 104;
		if (hay_que_load != 0) {
			if (!para_cargar(saveName)) {
				return true;
			}
		} else {
			carga_escoba("62.ald");
			hare_x = -20;
			hare_y = 56;
			lleva_al_hare(65, 145);
		}
	} else if (num_ejec == 2) {
		addObject(28);
		sentido_hare = 3;
		objExit = 162;
		if (hay_que_load == 0)
			carga_escoba("14.ald");
		else {
			if (!para_cargar(saveName)) {
				return true;
			}
		}
	} else if (num_ejec == 3) {
		addObject(28);
		addObject(11);
		addObject(14);
		addObject(22);
		addObject(9);
		addObject(20);
		addObject(19);
		flags[1] = 1;
		sentido_hare = 1;
		objExit = 99;
		if (hay_que_load == 0)
			carga_escoba("20.ald");
		else {
			if (!para_cargar(saveName)) {
				return true;
			}
		}
	} else if (num_ejec == 4) {
		addObject(28);
		addObject(9);
		addObject(20);
		addObject(22);
		objExit = 100;
		if (hay_que_load == 0) {
			carga_escoba("21.ald");
			sentido_hare = 0;
			hare_x = 235;
			hare_y = 164;
		} else {
			if (!para_cargar(saveName)) {
				return true;
			}
		}
	} else if (num_ejec == 5) {
		addObject(28);
		addObject(7);
		addObject(9);
		addObject(11);
		addObject(13);
		addObject(14);
		addObject(15);
		addObject(17);
		addObject(20);
		sentido_hare = 1;
		objExit = 100;
		if (hay_que_load == 0) {
			carga_escoba("45.ald");
		} else {
			if (!para_cargar(saveName)) {
				return true;
			}
		}
	} else if (num_ejec == 6) {
		addObject(28);
		addObject(9);

		sentido_hare = 1;
		objExit = 104;
		if (hay_que_load == 0) {
			carga_escoba("58.ald");
			animation_1_6();
		} else {
			if (!para_cargar(saveName)) {
				return true;
			}
			loadAndDecompressPic("auxdr.alg", drawSurface2, 1);
		}
	}

bucles:
	if (characterMoved == 0) {
		stepX = PASO_HARE_X;
		stepY = PASO_HARE_Y;
	}
	if (characterMoved == 0 && walkToObject == 1) {
		sentido_hare = sentido_final;
		walkToObject = 0;
	}

	if (num_ejec == 2) {
		if (roomNumber == 3 && (hare_x == 279) && (hare_y + alto_hare == 101))
			animation_1_2();
		else if (roomNumber == 14 && (hare_x == 214) && (hare_y + alto_hare == 121))
			lleva_al_hare(190, 130);
		else if (roomNumber == 14 && (hare_x == 246) && (hare_y + alto_hare == 112))
			lleva_al_hare(190, 130);
	}

	moveCursor();
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

	if (num_ejec == 2) {
		if (musicStatus() == 0 && roomMusic != 0)
			playMusic(roomMusic);
	} else {
		if (musicStatus() == 0)
			playMusic(roomMusic);
	}

	updateEvents();

	if (menuScreen == 0 && takeObject == 1)
		checkObjects();

	if (button_dch == 1 && menuScreen == 1) {
		delay(100);
		if (num_ejec == 2)
			loadAndDecompressPic(menuBackground, backSurface, 1);
		else
			loadAndDecompressPic("99.alg", backSurface, 1);
		setPalette((byte *)&gamePalette);
		menuScreen = 0;
		updateEvents();
		if (num_ejec != 3)
			cont_sv = 0;
	}
	if (button_dch == 1 && menuScreen == 0) {
		delay(100);
		characterMoved = 0;
		if (sentido_hare == 2)
			sentido_hare = 1;
		if (num_ejec == 4)
			loadAndDecompressPic("icons2.alg", backSurface, 1);
		else if (num_ejec == 5)
			loadAndDecompressPic("icons3.alg", backSurface, 1);
		else if (num_ejec == 6)
			loadAndDecompressPic("iconsp.alg", backSurface, 1);
		else
			loadAndDecompressPic("icons.alg", backSurface, 1);
		menuScreen = 1;
		updateEvents();
		withoutVerb();
		if (num_ejec != 3)
			cont_sv = 0;
	}

	if (button_izq == 1 && menuBar == 1) {
		delay(100);
		elige_en_barra();
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (button_izq == 1 && takeObject == 0) {
		delay(100);
		if (comprueba1())
			return true;
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (button_izq == 1 && takeObject == 1) {
		if (comprueba2())
			return true;
		if (num_ejec != 3)
			cont_sv = 0;
	}

	menuBar = (mouseY < 24 && menuScreen == 0) ? 1 : 0;

	Common::KeyCode key = getScan();
	if (key == Common::KEYCODE_F1 && menuScreen == 0) {
		selectVerb(1);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F2 && menuScreen == 0) {
		selectVerb(2);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F3 && menuScreen == 0) {
		selectVerb(3);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F4 && menuScreen == 0) {
		selectVerb(4);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F5 && menuScreen == 0) {
		selectVerb(5);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F6 && menuScreen == 0) {
		selectVerb(6);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F9) {
		mesa();
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F10) {
		if (!saves())
			return true;
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F8) {
		withoutVerb();
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_v) {
		withVoices = 1;
		print_abc(_textsys[_lang][2], 96, 86);
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		delay(1410);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_t) {
		withVoices = 0;
		print_abc(_textsys[_lang][3], 94, 86);
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		delay(1460);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_ESCAPE) {
		if (!confirmExit())
			return false;
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (num_ejec == 6 && key == Common::KEYCODE_0 && roomNumber == 61) {
		 loadAndDecompressPic("alcbar.alg", drawSurface1, 255);
	} else if (cont_sv == 15000) {
		screenSaver();
		if (num_ejec != 3)
			cont_sv = 0;
	} else {
		if (num_ejec != 3)
			cont_sv++;
	}
	goto bucles;
}

void DrasculaEngine::pickObject(int objeto) {
	if (num_ejec == 6)
		loadAndDecompressPic("iconsp.alg", backSurface, 1);
	else if (num_ejec == 4)
		loadAndDecompressPic("icons2.alg", backSurface, 1);
	else if (num_ejec == 5)
		loadAndDecompressPic("icons3.alg", backSurface, 1);
	else
		loadAndDecompressPic("icons.alg", backSurface, 1);
	chooseObject(objeto);
	if (num_ejec == 2)
		loadAndDecompressPic(menuBackground, backSurface, 1);
	else
		loadAndDecompressPic("99.alg", backSurface, 1);
}

void DrasculaEngine::chooseObject(int objeto) {
	if (num_ejec == 5) {
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

int DrasculaEngine::removeObject(int osj) {
	int result = 1;

	for (int h = 1; h < 43; h++) {
		if (inventoryObjects[h] == osj) {
			inventoryObjects[h] = 0;
			result = 0;
			break;
		}
	}

	return result;
}

void DrasculaEngine::withoutVerb() {
	int c = 171;
	if (menuScreen == 1)
		c = 0;
	if (num_ejec == 5) {
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

bool DrasculaEngine::para_cargar(char gameName[]) {
	previousMusic = roomMusic;
	menuScreen = 0;
	if (num_ejec != 1)
		clearRoom();
	if (!loadGame(gameName))
		return false;
	if (num_ejec == 2 || num_ejec == 3 || num_ejec == 5) {
		delete ald;
		ald = NULL;
	}
	carga_escoba(currentData);
	withoutVerb();

	return true;
}

static char *getLine(Common::File *fp, char *buf, int len) {
	byte c;
	char *b;

	for (;;) {
		b = buf;
		while (!fp->eos()) {
			c = ~fp->readByte();
			if (c == '\r')
				continue;
			if (c == '\n' || b - buf >= (len - 1))
				break;
			*b++ = c;
		}
		*b = '\0';
		if (fp->eos() && b == buf)
			return NULL;
		if (b != buf)
			break;
	}
	return buf;
}

void DrasculaEngine::carga_escoba(const char *nom_fich) {
	int soc, l, martin = 0, obj_salir = 0;
	float chiquez = 0, pequegnez = 0;
	char pant1[20], pant2[20], pant3[20], pant4[20];
	char para_codificar[20];
	char buffer[256];

	hasName = 0;

	strcpy(para_codificar, nom_fich);
	strcpy(currentData, nom_fich);

	ald = new Common::File;
	ald->open(nom_fich);
	if (!ald->isOpen()) {
		error("missing data file");
	}
	int size = ald->size();
	getLine(ald, buffer, size);
	roomNumber = atoi(buffer);

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &roomMusic);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", roomDisk);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &nivel_osc);

	if (num_ejec == 2) {
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &martin);
	}

	if (num_ejec == 2 && martin != 0) {
		ancho_hare = martin;
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&alto_hare);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&feetHeight);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&stepX);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&stepY);

		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant1);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant2);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant3);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant4);

		loadAndDecompressPic(pant2, extraSurface, 1);
		loadAndDecompressPic(pant1, frontSurface, 1);
		loadAndDecompressPic(pant4, backSurface, 1);

		strcpy(menuBackground, pant4);
	}

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &numRoomObjs);

	for (l = 0; l < numRoomObjs; l++) {
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &objectNum[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s", objName[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &x1[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &y1[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &x2[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &y2[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &sitiobj_x[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &sitiobj_y[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &sentidobj[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &visible[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &isDoor[l]);
		if (isDoor[l] != 0) {
			getLine(ald, buffer, size);
			sscanf(buffer, "%s", _targetSurface[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &_destX[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &_destY[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &sentido_alkeva[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &alapuertakeva[l]);
			updateDoor(l);
		}
	}

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &suelo_x1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &suelo_y1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &suelo_x2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &suelo_y2);

	if (num_ejec != 2) {
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &far);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &near);
	}
	delete ald;
	ald = NULL;

	if (num_ejec == 2) {
		if (martin == 0) {
			stepX = PASO_HARE_X;
			stepY = PASO_HARE_Y;
			alto_hare = CHARACTER_HEIGHT;
			ancho_hare = CHARACTER_WIDTH;
			feetHeight = PIES_HARE;
			loadAndDecompressPic("97.alg", extraSurface, 1);
			loadAndDecompressPic("96.alg", frontSurface, 1);
			loadAndDecompressPic("99.alg", backSurface, 1);

			strcpy(menuBackground, "99.alg");
		}
	}

	for (l = 0; l < numRoomObjs; l++) {
		if (objectNum[l] == objExit)
			obj_salir = l;
	}

	if (num_ejec == 2) {
		if (hare_x == -1) {
			hare_x = _destX[obj_salir];
			hare_y = _destY[obj_salir] - alto_hare;
		}
		characterMoved = 0;
	}
	loadAndDecompressPic(roomDisk, drawSurface3, 1);

	char rm[20];
	sprintf(rm, "%i.alg", roomNumber);
	loadAndDecompressPic(rm, drawSurface1, HALF_PAL);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	color_hare();
	if (nivel_osc != 0)
		funde_hare(nivel_osc);
	paleta_hare_claro();
	color_hare();
	funde_hare(nivel_osc + 2);
	paleta_hare_oscuro();

	hare_claro();
	changeColor = -1;

	if (num_ejec == 2)
		color_abc(kColorLightGreen);

	if (num_ejec != 2) {
		for (l = 0; l <= suelo_y1; l++)
			factor_red[l] = far;
		for (l = suelo_y1; l <= 201; l++)
			factor_red[l] = near;

		chiquez = (float)(near - far) / (float)(suelo_y2 - suelo_y1);
		for (l = suelo_y1; l <= suelo_y2; l++) {
			factor_red[l] = (int)(far + pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (roomNumber == 24) {
		for (l = suelo_y1 - 1; l > 74; l--) {
			factor_red[l] = (int)(far - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (num_ejec == 5 && roomNumber == 54) {
		for (l = suelo_y1 - 1; l > 84; l--) {
			factor_red[l] = (int)(far - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (num_ejec != 2) {
		if (hare_x == -1) {
			hare_x = _destX[obj_salir];
			hare_y = _destY[obj_salir];
			alto_hare = (CHARACTER_HEIGHT * factor_red[hare_y]) / 100;
			ancho_hare = (CHARACTER_WIDTH * factor_red[hare_y]) / 100;
			hare_y = hare_y - alto_hare;
		} else {
			alto_hare = (CHARACTER_HEIGHT * factor_red[hare_y]) / 100;
			ancho_hare = (CHARACTER_WIDTH * factor_red[hare_y]) / 100;
		}
		characterMoved = 0;
	}

	if (num_ejec == 2) {
		soc = 0;
		for (l = 0; l < 6; l++) {
			soc += ancho_hare;
			frame_x[l] = soc;
		}
	}

	if (num_ejec == 5)
		hare_se_ve = 1;

	updateData();

	if (num_ejec == 1)
		isDoor[7] = 0;

	if (num_ejec == 2) {
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

	if (num_ejec == 2) {
		if (roomNumber == 9 || roomNumber == 2 || roomNumber == 14 || roomNumber == 18)
			conta_blind_vez = getTime();
	}
	if (num_ejec == 4) {
		if (roomNumber == 26)
			conta_blind_vez = getTime();
	}

	if (num_ejec == 4 && roomNumber == 24 && flags[29] == 1)
		animation_7_4();

	if (num_ejec == 5) {
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

void DrasculaEngine::lleva_al_hare(int pointX, int pointY) {
	if (num_ejec == 5 || num_ejec == 6) {
		if (hare_se_ve == 0) {
			hare_x = sitio_x;
			hare_y = sitio_y;
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			return;
		}
	}
	sitio_x = pointX;
	sitio_y = pointY;
	startWalking();

	for (;;) {
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		if (characterMoved == 0)
			break;
	}

	if (walkToObject == 1) {
		walkToObject = 0;
		sentido_hare = sentido_final;
	}
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
}

void DrasculaEngine::moveCursor() {
	int cursorPos[8];

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);

	updateRefresh_pre();
	pon_hare();
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

	if (num_ejec == 2) {
		if (mouseX > hare_x + 2 && mouseY > hare_y + 2
				&& mouseX < hare_x + ancho_hare - 2 && mouseY < hare_y + alto_hare - 2) {
			strcpy(textName, "hacker");
			hasName = 1;
			veo = 1;
		}
	} else {
		if (mouseX > hare_x + 2 && mouseY > hare_y + 2
				&& mouseX < hare_x + ancho_hare - 2 && mouseY < hare_y + alto_hare - 2 && veo == 0) {
			strcpy(textName, "hacker");
			hasName = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hasName = 0;
}

void DrasculaEngine::elige_en_barra() {
	int n, num_verbo = -1;

	for (n = 0; n < 7; n++)
		if (mouseX > x_barra[n] && mouseX < x_barra[n + 1])
			num_verbo = n;

	if (num_verbo < 1)
		withoutVerb();
	else
		selectVerb(num_verbo);
}

bool DrasculaEngine::comprueba1() {
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

		if (mouseX > hare_x && mouseY > hare_y
				&& mouseX < hare_x + ancho_hare && mouseY < hare_y + alto_hare)
			doBreak = 1;

		for (l = 0; l < numRoomObjs; l++) {
			if (mouseX > x1[l] && mouseY > y1[l]
					&& mouseX < x2[l] && mouseY < y2[l] && doBreak == 0) {
				sitio_x = sitiobj_x[l];
				sitio_y = sitiobj_y[l];
				sentido_final = sentidobj[l];
				doBreak = 1;
				walkToObject = 1;
				startWalking();
			}
		}

		if (doBreak == 0) {
			sitio_x = CLIP(mouseX, suelo_x1, suelo_x2);
			sitio_y = CLIP(mouseY, suelo_y1 + feetHeight, suelo_y2);
			startWalking();
		}
		doBreak = 0;
	}

	return false;
}

bool DrasculaEngine::comprueba2() {
	int l;

	if (menuScreen == 1) {
		if (pickupObject())
			return true;
	} else {
		if (!strcmp(textName, "hacker") && hasName == 1) {
			if (checkFlag(50))
				return true;
		} else {
			for (l = 0; l < numRoomObjs; l++) {
				if (mouseX > x1[l] && mouseY > y1[l]
						&& mouseX < x2[l] && mouseY < y2[l] && visible[l] == 1) {
					sentido_final = sentidobj[l];
					walkToObject = 1;
					lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
					if (checkFlag(objectNum[l]))
						return true;
					if (num_ejec == 4)
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
			button_izq = 1;
			break;
		case Common::EVENT_LBUTTONUP:
			button_izq = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			button_dch = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			button_dch = 0;
			break;
		case Common::EVENT_QUIT:
			// TODO
			releaseGame();
			exit(0);
			break;
		default:
			break;
		}
	}
}

void DrasculaEngine::selectVerb(int verbo) {
	int c = 171;

	if (menuScreen == 1)
		c = 0;
	if (num_ejec == 5) {
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

void DrasculaEngine::mesa() {
	int nivel_master, nivel_voc, nivel_cd;

	copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);
	updateScreen(73, 63, 73, 63, 177, 97, screenSurface);

	for (;;) {
		nivel_master = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4);
		nivel_voc = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4);
		nivel_cd = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4);

		updateRoom();

		copyRect(1, 56, 73, 63, 177, 97, tableSurface, screenSurface);

		copyBackground(183, 56, 82, nivel_master, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4), tableSurface, screenSurface);
		copyBackground(183, 56, 138, nivel_voc, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4), tableSurface, screenSurface);
		copyBackground(183, 56, 194, nivel_cd, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4), tableSurface, screenSurface);

		setCursorTable();

		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

		updateEvents();

		if (button_dch == 1) {
			delay(100);
			break;
		}
		if (button_izq == 1) {
			delay(100);
			if (mouseX > 80 && mouseX < 121) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16;
				if (mouseY < nivel_master && vol < 15)
					vol++;
				if (mouseY > nivel_master && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol * 16);
			}

			if (mouseX > 136 && mouseX < 178) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16;
				if (mouseY < nivel_voc && vol < 15)
					vol++;
				if (mouseY > nivel_voc && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol * 16);
			}

			if (mouseX > 192 && mouseX < 233) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
				if (mouseY < nivel_cd && vol < 15)
					vol++;
				if (mouseY > nivel_cd && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol * 16);
			}
		}

	}

	updateEvents();
}

bool DrasculaEngine::saves() {
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

	loadAndDecompressPic("savescr.alg", drawSurface1, HALF_PAL);

	color_abc(kColorLightGreen);

	for (;;) {
		y = 27;
		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(names[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		setCursorTable();
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		y = 27;

		updateEvents();

		if (button_izq == 1) {
			delay(100);
			for (n = 0; n < NUM_SAVES; n++) {
				if (mouseX > 115 && mouseY > y + (9 * n) && mouseX < 115 + 175 && mouseY < y + 10 + (9 * n)) {
					strcpy(select, names[n]);

					if (strcmp(select, "*"))
						hay_seleccion = 1;
					else {
						enterName();
						strcpy(names[n], select);
						if (hay_seleccion == 1) {
							snprintf(file, 50, "%s%02d", _targetName.c_str(), n + 1);
							para_grabar(file);
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
					if (hay_seleccion == 1) {
						snprintf(file, 50, "%s%02d", _targetName.c_str(), n + 1);
					}
					num_sav = n;
				}
			}

			if (mouseX > 117 && mouseY > 15 && mouseX < 295 && mouseY < 24 && hay_seleccion == 1) {
				enterName();
				strcpy(names[num_sav], select);
				print_abc(select, 117, 15);
				y = 27;
				for (n2 = 0; n2 < NUM_SAVES; n2++) {
					print_abc(names[n2], 116, y);
					y = y + 9;
				}
			}

			if (mouseX > 125 && mouseY > 123 && mouseX < 199 && mouseY < 149 && hay_seleccion == 1) {
				if (!para_cargar(file))
					return false;
				break;
			} else if (mouseX > 208 && mouseY > 123 && mouseX < 282 && mouseY < 149 && hay_seleccion == 1) {
				para_grabar(file);
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
			else if (hay_seleccion == 0) {
				print_abc("elige una partida", 117, 15);
			}
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			delay(400);
		}
		y = 26;

		delay(10);
	}

	clearRoom();
	char rm[20];
	sprintf(rm, "%i.alg", roomNumber);
	loadAndDecompressPic(rm, drawSurface1, HALF_PAL);
	hay_seleccion = 0;

	return true;
}

void DrasculaEngine::print_abc(const char *said, int x_pantalla, int y_pantalla) {
	int pos_texto[8];
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

		pos_texto[0] = letterX;
		pos_texto[1] = letterY;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = CHAR_WIDTH;
		pos_texto[5] = CHAR_HEIGHT;

		copyRectClip(pos_texto, textSurface, screenSurface);

		x_pantalla = x_pantalla + CHAR_WIDTH;
		if (x_pantalla > 317) {
			x_pantalla = 0;
			y_pantalla = y_pantalla + CHAR_HEIGHT + 2;
		}
	}	// for
}

void DrasculaEngine::print_abc_opc(const char *said, int x_pantalla, int y_pantalla, int game) {
	int pos_texto[6];
	int y_de_signos, letterY, letterX = 0, h, length;
	length = strlen(said);

	for (h = 0; h < length; h++) {
		if (game == 1) {
			letterY = 6;
			y_de_signos = 15;
		} else if (game == 3) {
			letterY = 56;
			y_de_signos = 65;
		} else {
			letterY = 31;
			y_de_signos = 40;
		}

		int c = toupper(said[h]);

		for (int i = 0; i < CHARMAP_SIZE; i++) {
			if (c == charMap[i].inChar) {
				// Convert the mapped char of the normal font to the
				// mapped char of the dialogue font

				int multiplier = (charMap[i].mappedChar - 6) / 9;

				letterX = multiplier * 7 + 10;

				if (charMap[i].charType > 0)
					letterY = y_de_signos;
				break;
			}	// if
		}	// for

		pos_texto[0] = letterX;
		pos_texto[1] = letterY;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = CHAR_WIDTH_OPC;
		pos_texto[5] = CHAR_HEIGHT_OPC;

		copyRectClip(pos_texto, backSurface, screenSurface);

		x_pantalla = x_pantalla + CHAR_WIDTH_OPC;
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
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

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
	Common::File file;
	float coeff = 0, coeff2 = 0;
	int count = 0;
	int count2 = 0;
	int tempLine[320];
	int tempRow[200];

	clearRoom();

	loadAndDecompressPic("sv.alg", drawSurface1, HALF_PAL);

	// inicio_ghost();
	copia = (byte *)malloc(64000);
	ghost = (byte *)malloc(65536);

	// carga_ghost();
	file.open("ghost.drv");
	if (!file.isOpen())
		error("Cannot open file ghost.drv");

	file.read(ghost, 65536);
	file.close();

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
			if (tempLine[i] < 0)
				tempLine[i] += 200;
			if (tempLine[i] > 199)
				tempLine[i] -= 200;
		}

		coeff2 = coeff;
		for (int i = 0; i < 200; i++) {
			tempRow[i] = (int)(sin(coeff2) * 16);
			coeff2 += 0.02f;
			if (tempRow[i] < 0)
				tempRow[i] += 320;
			if (tempRow[i] > 319)
				tempRow[i] -= 320;
		}

		if (++count2 > 199)
			count2 = 0;

		int x1_, y1_, off1, off2;

		for (int i = 0; i < 200; i++) {
			for (int j = 0; j < 320; j++) {
				x1_ = j + tempRow[i];
				if (x1_ < 0)
					x1_ += 320;
				if (x1_ > 319)
					x1_ -= 319;

				y1_ = i + count2;
				if (y1_ < 0)
					y1_ += 200;
				if (y1_ > 199)
					y1_ -= 200;

				off1 = 320 * y1_ + x1_;

				x1_ = j + count;
				if (x1_ < 0)
					x1_ += 320;
				if (x1_ > 319)
					x1_ -= 320;

				y1_ = i + tempLine[j];
				if (y1_ < 0)
					y1_ += 200;
				if (y1_ > 199)
					y1_ -= 200;
				off2 = 320 * y1_ + x1_;

				VGA[320 * i + j] = ghost[drawSurface1[off2] + (copia[off1] << 8)];
			}
		}
		_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		_system->updateScreen();

		_system->delayMillis(20);

		// end of efecto()

		updateEvents();
		if (button_dch == 1 || button_izq == 1)
			break;
		if (mouseX != xr)
			break;
		if (mouseY != yr)
			break;
	}
	// fin_ghost();
	free(copia);
	free(ghost);

	char rm[20];
	sprintf(rm, "%i.alg", roomNumber);
	loadAndDecompressPic(rm, drawSurface1, HALF_PAL);
}

void DrasculaEngine::fliplay(const char *filefli, int vel) {
	openSSN(filefli, vel);
	while (playFrameSSN() && (!term_int)) {
		if (getScan() == Common::KEYCODE_ESCAPE)
			term_int = 1;
	}
	EndSSN();
}

void DrasculaEngine::fadeFromBlack(int fadeSpeed) {
	char fundido;
	unsigned int color, component;

	DacPalette256 palFundido;

	for (fundido = 0; fundido < 64; fundido++) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFundido[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fundido);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFundido);
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

tut:
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
		goto tut;
	strcpy(mb[conta_f], m1);

	if (!strcmp(m2, ""))
		goto imprimir;
	scumm_strrev(m2);
	m2[strlen(m2) - 1] = '\0';
	scumm_strrev(m2);
	strcpy(m1, m2);
	strcpy(m2, "");
	conta_f++;

	goto tut;

imprimir:

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

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(file);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
	}
	ctvd_init(2);
	ctvd_speaker(1);
	ctvd_output(sku);
}

bool DrasculaEngine::animate(const char *animationFile, int FPS) {
	Common::File FileIn;
	unsigned j;
	int NFrames = 1;
	int cnt = 2;
	int dataSize = 0;

	AuxBuffLast = (byte *)malloc(65000);
	AuxBuffDes = (byte *)malloc(65000);

	FileIn.open(animationFile);

	if (!FileIn.isOpen()) {
		error("Animation file %s not found", animationFile);
	}

	FileIn.read(&NFrames, sizeof(NFrames));
	FileIn.read(&dataSize, sizeof(dataSize));
	AuxBuffOrg = (byte *)malloc(dataSize);
	FileIn.read(AuxBuffOrg, dataSize);
	FileIn.read(cPal, 768);
	loadPCX(AuxBuffOrg);
	free(AuxBuffOrg);
	memcpy(VGA, AuxBuffDes, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
	set_dac(cPal);
	memcpy(AuxBuffLast, AuxBuffDes, 64000);
	WaitForNext(FPS);
	while (cnt < NFrames) {
		FileIn.read(&dataSize, sizeof(dataSize));
		AuxBuffOrg = (byte *)malloc(dataSize);
		FileIn.read(AuxBuffOrg, dataSize);
		FileIn.read(cPal, 768);
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
	FileIn.close();

	return ((term_int == 1) || (getScan() == Common::KEYCODE_ESCAPE));
}

void DrasculaEngine::animastopSound_corte() {
	if (hay_sb == 1) {
		ctvd_stop();
		delete sku;
		sku = NULL;
		ctvd_terminate();
	}
}

void DrasculaEngine::fadeToBlack(int fadeSpeed) {
	char fundido;
	unsigned int color, component;

	DacPalette256 palFundido;

	for (fundido = 63; fundido >= 0; fundido--) {
		for (color = 0; color < 256; color++) {
			for (component = 0; component < 3; component++) {
				palFundido[color][component] = adjustToVGA(gamePalette[color][component] - 63 + fundido);
			}
		}
		pause(fadeSpeed);

		setPalette((byte *)&palFundido);
	}
}

void DrasculaEngine::pause(int cuanto) {
	_system->delayMillis(cuanto * 30); // was originaly 2
}

void DrasculaEngine::placeIgor() {
	int pos_igor[6];

	pos_igor[0] = 1;
	if (num_ejec == 4) {
		pos_igor[1] = 138;
	} else {
		if (sentido_igor == 3)
			pos_igor[1] = 138;
		else if (sentido_igor == 1)
			pos_igor[1] = 76;
	}
	pos_igor[2] = x_igor;
	pos_igor[3] = y_igor;
	pos_igor[4] = 54;
	pos_igor[5] = 61;

	copyRectClip(pos_igor, frontSurface, screenSurface);
}

void DrasculaEngine::placeDrascula() {
	int pos_dr[6];

	if (sentido_dr == 1)
		pos_dr[0] = 47;
	else if (sentido_dr == 0)
		pos_dr[0] = 1;
	else if (sentido_dr == 3 && num_ejec == 1)
		pos_dr[0] = 93;
	pos_dr[1] = 122;
	pos_dr[2] = x_dr;
	pos_dr[3] = y_dr;
	pos_dr[4] = 45;
	pos_dr[5] = 77;

	if (num_ejec == 6)
		copyRectClip(pos_dr, drawSurface2, screenSurface);
	else
		copyRectClip(pos_dr, backSurface, screenSurface);
}

void DrasculaEngine::placeBJ() {
	int pos_bj[6];

	if (sentido_bj == 3)
		pos_bj[0] = 10;
	else if (sentido_bj == 0)
		pos_bj[0] = 37;
	pos_bj[1] = 99;
	pos_bj[2] = x_bj;
	pos_bj[3] = y_bj;
	pos_bj[4] = 26;
	pos_bj[5] = 76;

	copyRectClip(pos_bj, drawSurface3, screenSurface);
}

void DrasculaEngine::hiccup(int counter) {
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

	do {
		counter--;

		updateRoom();
		if (num_ejec == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (sentido == 0)
			y++;
		else
			y--;

		if (num_ejec == 3) {
			if (y == 1)
				sentido = 1;
			if (y == -1)
				sentido = 0;
		} else {
			if (y == 2)
				sentido = 1;
			if (y == 0)
				sentido = 0;
		}
	} while (counter > 0);

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
}

void DrasculaEngine::stopSound() {
	delay(1);

	if (hay_sb == 1) {
		while (soundIsActive());
		delete sku;
		sku = NULL;
	}
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
	if (num_ejec == 3) {
		if (flags[0] == 0)
			pon_hare();
		else
			copyRect(113, 54, hare_x - 20, hare_y - 1, 77, 89, drawSurface3, screenSurface);
	} else {
		pon_hare();
	}
	updateRefresh();
}

bool DrasculaEngine::loadGame(const char *gameName) {
	int l, n_ejec2;
	Common::InSaveFile *sav;

	if (!(sav = _saveFileMan->openForLoading(gameName))) {
		error("missing savegame file");
	}

	n_ejec2 = sav->readSint32LE();
	if (n_ejec2 != num_ejec) {
		strcpy(saveName, gameName);
		num_ejec = n_ejec2 - 1;
		hay_que_load = 1;
		return false;
	}
	sav->read(currentData, 20);
	hare_x = sav->readSint32LE();
	hare_y = sav->readSint32LE();
	sentido_hare = sav->readSint32LE();

	for (l = 1; l < 43; l++) {
		inventoryObjects[l] = sav->readSint32LE();
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		flags[l] = sav->readSint32LE();
	}

	takeObject = sav->readSint32LE();
	pickedObject = sav->readSint32LE();
	hay_que_load = 0;

	return true;
}

void DrasculaEngine::updateDoor(int doorNum) {
	if (num_ejec == 1 || num_ejec == 3 || num_ejec == 5 || num_ejec == 6)
		return;
	else if (num_ejec == 2) {
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
	} else if (num_ejec == 4) {
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

void DrasculaEngine::color_hare() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++) {
			gamePalette[color][component] = palHare[color][component];
		}
	}
	updatePalette();
}

void DrasculaEngine::funde_hare(int oscuridad) {
	char fundido;
	unsigned int color, component;

	for (fundido = oscuridad; fundido >= 0; fundido--) {
		for (color = 235; color < 253; color++) {
			for (component = 0; component < 3; component++)
				gamePalette[color][component] = adjustToVGA(gamePalette[color][component] - 8 + fundido);
		}
	}

	updatePalette();
}

void DrasculaEngine::paleta_hare_claro() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			palHareClaro[color][component] = gamePalette[color][component];
	}
}

void DrasculaEngine::paleta_hare_oscuro() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			palHareOscuro[color][component] = gamePalette[color][component];
	}
}

void DrasculaEngine::hare_claro() {
	int color, component;

	for (color = 235; color < 253; color++) {
		for (component = 0; component < 3; component++)
			gamePalette[color][component] = palHareClaro[color][component];
	}

	updatePalette();
}

void DrasculaEngine::startWalking() {
	characterMoved = 1;

	stepX = PASO_HARE_X;
	stepY = PASO_HARE_Y;

	if (num_ejec == 2) {
		if ((sitio_x < hare_x) && (sitio_y <= (hare_y + alto_hare)))
			quadrant_1();
		else if ((sitio_x < hare_x) && (sitio_y > (hare_y + alto_hare)))
			quadrant_3();
		else if ((sitio_x > hare_x + ancho_hare) && (sitio_y <= (hare_y + alto_hare)))
			quadrant_2();
		else if ((sitio_x > hare_x + ancho_hare) && (sitio_y > (hare_y + alto_hare)))
			quadrant_4();
		else if (sitio_y < hare_y + alto_hare)
			walkUp();
		else if (sitio_y > hare_y + alto_hare)
			walkDown();
	} else {
		if ((sitio_x < hare_x + ancho_hare / 2 ) && (sitio_y <= (hare_y + alto_hare)))
			quadrant_1();
		else if ((sitio_x < hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
			quadrant_3();
		else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y <= (hare_y + alto_hare)))
			quadrant_2();
		else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
			quadrant_4();
		else
			characterMoved = 0;
	}
	conta_vez = getTime();
}

void DrasculaEngine::pon_hare() {
	int pos_hare[6];
	int r;

	if (characterMoved == 1 && stepX == PASO_HARE_X) {
		for (r = 0; r < stepX; r++) {
			if (num_ejec != 2) {
				if (sentido_hare == 0 && sitio_x - r == hare_x + ancho_hare / 2) {
					characterMoved = 0;
					stepX = PASO_HARE_X;
					stepY = PASO_HARE_Y;
				}
				if (sentido_hare == 1 && sitio_x + r == hare_x + ancho_hare / 2) {
					characterMoved = 0;
					stepX = PASO_HARE_X;
					stepY = PASO_HARE_Y;
					hare_x = sitio_x - ancho_hare / 2;
					hare_y = sitio_y - alto_hare;
				}
			} else if (num_ejec == 2) {
				if (sentido_hare == 0 && sitio_x - r == hare_x) {
					characterMoved = 0;
					stepX = PASO_HARE_X;
					stepY = PASO_HARE_Y;
				}
				if (sentido_hare == 1 && sitio_x + r == hare_x + ancho_hare) {
					characterMoved = 0;
					stepX = PASO_HARE_X;
					stepY = PASO_HARE_Y;
					hare_x = sitio_x - ancho_hare + 4;
					hare_y = sitio_y - alto_hare;
				}
			}
		}
	}
	if (characterMoved == 1 && stepY == PASO_HARE_Y) {
		for (r = 0; r < stepY; r++) {
			if (sentido_hare == 2 && sitio_y - r == hare_y + alto_hare) {
				characterMoved = 0;
				stepX = PASO_HARE_X;
				stepY = PASO_HARE_Y;
			}
			if (sentido_hare == 3 && sitio_y + r == hare_y + alto_hare) {
				characterMoved = 0;
				stepX = PASO_HARE_X;
				stepY = PASO_HARE_Y;
			}
		}
	}

	if (num_ejec == 1 || num_ejec == 4 || num_ejec == 5 || num_ejec == 6) {
		if (hare_se_ve == 0) {
			increaseFrameNum();
			return;
		}
	}

	if (characterMoved == 0) {
		pos_hare[0] = 0;
		pos_hare[1] = DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		} else {
			pos_hare[4] = CHARACTER_WIDTH;
			pos_hare[5] = CHARACTER_HEIGHT;
		}

		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 2)
				copyRectClip(pos_hare, extraSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], extraSurface, screenSurface);
		} else if (sentido_hare == 1) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, extraSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], extraSurface, screenSurface);
		} else if (sentido_hare == 2) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, backSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], backSurface, screenSurface);
		} else {
			if (num_ejec == 2)
				copyRectClip(pos_hare, frontSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], frontSurface, screenSurface);
		}
	} else if (characterMoved == 1) {
		pos_hare[0] = frame_x[num_frame];
		pos_hare[1] = frame_y + DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		} else {
			pos_hare[4] = CHARACTER_WIDTH;
			pos_hare[5] = CHARACTER_HEIGHT;
		}
		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 2)
				copyRectClip(pos_hare, extraSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], extraSurface, screenSurface);
		} else if (sentido_hare == 1) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, extraSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], extraSurface, screenSurface);
		} else if (sentido_hare == 2) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, backSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], backSurface, screenSurface);
		} else {
			if (num_ejec == 2)
				copyRectClip(pos_hare, frontSurface, screenSurface);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], frontSurface, screenSurface);
		}
		increaseFrameNum();
	}
}

void DrasculaEngine::showMenu() {
	int h, n, x;
	char texto_icono[13];

	x = whichObject();
	strcpy(texto_icono, iconName[x]);

	for (n = 1; n < 43; n++) {
		h = inventoryObjects[n];

		if (h != 0) {
			if (num_ejec == 6)
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
		print_abc(texto_icono, itemLocations[x].x - 2, itemLocations[x].y - 7);
}

void DrasculaEngine::clearMenu() {
	int n, sobre_verbo = 1;

	for (n = 0; n < 7; n++) {
		if (mouseX > x_barra[n] && mouseX < x_barra[n + 1])
			sobre_verbo = 0;
		copyRect(OBJWIDTH * n, OBJHEIGHT * sobre_verbo, x_barra[n], 2,
						OBJWIDTH, OBJHEIGHT, backSurface, screenSurface);
		sobre_verbo = 1;
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
	char salgo[13];

	if (num_ejec == 1) {
		if (objectNum[l] == 105 && flags[0] == 0)
			talk(442);
		else {
			updateDoor(l);
			if (isDoor[l] != 0) {
				lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
				sentido_hare = sentidobj[l];
				updateRoom();
				updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
				characterMoved = 0;
				sentido_hare = sentido_alkeva[l];
				objExit = alapuertakeva[l];
				doBreak = 1;
				previousMusic = roomMusic;

				if (objectNum[l] == 105) {
					animation_2_1();
					return true;
				}
				clearRoom();
				strcpy(salgo, _targetSurface[l]);
				strcat(salgo, ".ald");
				hare_x = -1;
				carga_escoba(salgo);
			}
		}
	} else if (num_ejec == 2) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			characterMoved = 0;
			sentido_hare = sentido_alkeva[l];
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
				removeObject(23);
				addObject(11);
			}
			clearRoom();
			delete ald;
			ald = NULL;
			strcpy(salgo, _targetSurface[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 3) {
		updateDoor(l);
		if (isDoor[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			characterMoved = 0;
			sentido_hare = sentido_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			clearRoom();
			strcpy(salgo, _targetSurface[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 4) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			characterMoved = 0;
			sentido_hare = sentido_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;

			if (objectNum[l] == 108)
				lleva_al_hare(171, 78);
			clearRoom();
			strcpy(salgo, _targetSurface[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 5) {
		updateDoor(l);
		if (isDoor[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			characterMoved = 0;
			sentido_hare = sentido_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			hare_se_ve = 1;
			clearRoom();
			strcpy(salgo, _targetSurface[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 6) {
		updateDoor(l);
		if (isDoor[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
			characterMoved = 0;
			sentido_hare = sentido_alkeva[l];
			objExit = alapuertakeva[l];
			doBreak = 1;
			previousMusic = roomMusic;
			clearRoom();
			strcpy(salgo, _targetSurface[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);

			if (objExit == 105)
				animation_19_6();
		}
	}

	return false;
}

bool DrasculaEngine::pickupObject() {
	int h, n;
	h = pickedObject;
	checkFlags = 1;

	updateRoom();
	showMenu();
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

	// Objects with an ID smaller than 7 are the inventory verbs
	if (pickedObject >= 7) {
		for (n = 1; n < 43; n++) {
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
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
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
		hay_seleccion = 1;
	}
}

void DrasculaEngine::para_grabar(char gameName[]) {
	saveGame(gameName);
	playSound(99);
	stopSound();
}

void DrasculaEngine::openSSN(const char *Name, int Pause) {
	MiVideoSSN = (byte *)malloc(64256);
	globalSpeed = 1000 / Pause;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	_Session = new Common::File;
	_Session->open(Name);
	mSession = TryInMem(_Session);
	LastFrame = _system->getMillis();
}

int DrasculaEngine::playFrameSSN() {
	int Exit = 0;
	int Lengt;
	byte *BufferSSN;

	if (!UsingMem)
		_Session->read(&CHUNK, 1);
	else {
		memcpy(&CHUNK, mSession, 1);
		mSession += 1;
	}

	switch (CHUNK) {
	case kFrameSetPal:
		if (!UsingMem)
			_Session->read(dacSSN, 768);
		else {
			memcpy(dacSSN, mSession, 768);
			mSession += 768;
		}
		set_dacSSN(dacSSN);
		break;
	case kFrameEmptyFrame:
		WaitFrameSSN();
		break;
	case kFrameInit:
		if (!UsingMem) {
			_Session->read(&CMP, 1);
			_Session->read(&Lengt, 4);
		} else {
			memcpy(&CMP, mSession, 1);
			mSession += 1;
			memcpy(&Lengt, mSession, 4);
			mSession += 4;
		}
		if (CMP == kFrameCmpRle) {
			if (!UsingMem) {
				BufferSSN = (byte *)malloc(Lengt);
				_Session->read(BufferSSN, Lengt);
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
					_Session->read(BufferSSN, Lengt);
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

void DrasculaEngine::EndSSN() {
	free(MiVideoSSN);
	if (UsingMem)
		free(pointer);
	else {
		_Session->close();
		delete _Session;
	}
}

byte *DrasculaEngine::TryInMem(Common::File *Session) {
	int Lengt;

	Session->seek(0, SEEK_END);
	Lengt = Session->pos();
	Session->seek(0, SEEK_SET);
	pointer = (byte *)malloc(Lengt);
	if (pointer == NULL)
		return NULL;
	Session->read(pointer, Lengt);
	UsingMem = 1;
	Session->close();
	delete Session;
	return pointer;
}

void DrasculaEngine::set_dacSSN(byte *PalBuf) {
	setPalette((byte *)PalBuf);
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

void DrasculaEngine::set_dac(byte *dac) {
	setPalette((byte *)dac);
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
	float distance_x, distance_y;

	if (num_ejec == 2)
		distance_x = hare_x - sitio_x;
	else
		distance_x = hare_x + ancho_hare / 2 - sitio_x;

	distance_y = (hare_y + alto_hare) - sitio_y;

	if (distance_x < distance_y) {
		direccion_hare = 0;
		sentido_hare = 2;
		stepX = (int)(distance_x / (distance_y / PASO_HARE_Y));
	} else {
		direccion_hare = 7;
		sentido_hare = 0;
		stepY = (int)(distance_y / (distance_x / PASO_HARE_X));
	}
}

void DrasculaEngine::quadrant_2() {
	float distance_x, distance_y;

	if (num_ejec == 2)
		distance_x = abs(hare_x + ancho_hare - sitio_x);
	else
		distance_x = abs(hare_x + ancho_hare / 2 - sitio_x);

	distance_y = (hare_y + alto_hare) - sitio_y;

	if (distance_x < distance_y) {
		direccion_hare = 1;
		sentido_hare = 2;
		stepX = (int)(distance_x / (distance_y / PASO_HARE_Y));
	} else {
		direccion_hare = 2;
		sentido_hare = 1;
		stepY = (int)(distance_y / (distance_x / PASO_HARE_X));
	}
}

void DrasculaEngine::quadrant_3() {
	float distance_x, distance_y;

	if (num_ejec == 2)
		distance_x = hare_x - sitio_x;
	else
		distance_x = hare_x + ancho_hare / 2 - sitio_x;

	distance_y = sitio_y - (hare_y + alto_hare);

	if (distance_x < distance_y) {
		direccion_hare = 5;
		sentido_hare = 3;
		stepX = (int)(distance_x / (distance_y / PASO_HARE_Y));
	} else {
		direccion_hare = 6;
		sentido_hare = 0;
		stepY = (int)(distance_y / (distance_x / PASO_HARE_X));
	}
}

void DrasculaEngine::quadrant_4() {
	float distance_x, distance_y;

	if (num_ejec == 2)
		distance_x = abs(hare_x + ancho_hare - sitio_x);
	else
		distance_x = abs(hare_x + ancho_hare / 2 - sitio_x);

	distance_y = sitio_y - (hare_y + alto_hare);

	if (distance_x < distance_y) {
		direccion_hare = 4;
		sentido_hare = 3;
		stepX = (int)(distance_x / (distance_y / PASO_HARE_Y));
	} else {
		direccion_hare = 3;
		sentido_hare = 1;
		stepY = (int)(distance_y / (distance_x / PASO_HARE_X));
	}
}

void DrasculaEngine::saveGame(char gameName[]) {
	Common::OutSaveFile *out;
	int l;

	if (!(out = _saveFileMan->openForSaving(gameName))) {
		error("no puedo abrir el archivo");
	}
	out->writeSint32LE(num_ejec);
	out->write(currentData, 20);
	out->writeSint32LE(hare_x);
	out->writeSint32LE(hare_y);
	out->writeSint32LE(sentido_hare);

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
}

void DrasculaEngine::increaseFrameNum() {
	diff_vez = getTime() - conta_vez;

	if (diff_vez >= 6) {
		conta_vez = getTime();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (direccion_hare == 0 || direccion_hare == 7) {
			hare_x = hare_x - stepX;
			hare_y = hare_y - stepY;
		} else if (direccion_hare == 1 || direccion_hare == 2) {
			hare_x = hare_x + stepX;
			hare_y = hare_y - stepY;
		} else if (direccion_hare == 3 || direccion_hare == 4) {
			hare_x = hare_x + stepX;
			hare_y = hare_y + stepY;
		} else if (direccion_hare == 5 || direccion_hare == 6) {
			hare_x = hare_x - stepX;
			hare_y = hare_y + stepY;
		}
	}

	if (num_ejec != 2) {
		hare_y += (int)(alto_hare - newHeight);
		hare_x += (int)(ancho_hare - newWidth);
		alto_hare = (int)newHeight;
		ancho_hare = (int)newWidth;
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
	int h, n;

	for (n = 0; n < 43; n++) {
		if (whichObject() == n) {
			h = inventoryObjects[n];
			if (h != 0)
				if (checkFlag(h))
					return true;
		}
	}

	return false;
}

void DrasculaEngine::converse(const char *nom_fich) {
	int h;
	int game1 = 1, game2 = 1, game3 = 1, game4 = 1;
	char phrase1[78];
	char phrase2[78];
	char phrase3[87];
	char phrase4[78];
	char para_codificar[13];
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

	rompo_y_salgo = 0;

	strcpy(para_codificar, nom_fich);

	if (num_ejec == 5)
		withoutVerb();

	ald = new Common::File;
	ald->open(nom_fich);
	if (!ald->isOpen()) {
		error("missing data file");
	}
	int size = ald->size();

	getLine(ald, buffer, size);
	sscanf(buffer, "%s", phrase1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", phrase2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", phrase3);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", phrase4);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", sound1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", sound2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", sound3);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", sound4);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &answer1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &answer2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &answer3);
	delete ald;
	ald = NULL;

	if (num_ejec == 2 && !strcmp(nom_fich, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(phrase3, _text[_lang][405]);
		strcpy(sound3, "405.als");
		answer3 = 31;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_12.cal") && flags[7] == 1) {
		strcpy(phrase3, _text[_lang][273]);
		strcpy(sound3, "273.als");
		answer3 = 14;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_12.cal") && flags[10] == 1) {
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

	loadAndDecompressPic("car.alg", backSurface, 1);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this due lack backend functionality
	// from 1(top) to 31
	color_abc(kColorLightGreen);

bucle_opc:

	updateRoom();

	if (num_ejec == 1 || num_ejec == 4 || num_ejec == 6) {
		if (musicStatus() == 0 && flags[11] == 0)
			playMusic(roomMusic);
	} else if (num_ejec == 2) {
		if (musicStatus() == 0 && flags[11] == 0 && roomMusic != 0)
			playMusic(roomMusic);
	} else if (num_ejec == 3 || num_ejec == 5) {
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

	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

	if ((button_izq == 1) && (game1 == 2)) {
		delay(100);
		used1 = 1;
		talk(phrase1, sound1);
		if (num_ejec == 3)
			grr();
		else
			response(answer1);
	} else if ((button_izq == 1) && (game2 == 2)) {
		delay(100);
		used2 = 1;
		talk(phrase2, sound2);
		if (num_ejec == 3)
			grr();
		else
			response(answer2);
	} else if ((button_izq == 1) && (game3 == 2)) {
		delay(100);
		used3 = 1;
		talk(phrase3, sound3);
		if (num_ejec == 3)
			grr();
		else
			response(answer3);
	} else if ((button_izq == 1) && (game4 == 2)) {
		delay(100);
		talk(phrase4, sound4);
		rompo_y_salgo = 1;
	}

	if (button_izq == 1) {
		delay(100);
		color_abc(kColorLightGreen);
	}

	game1 = (used1 == 0) ? 1 : 3;
	game2 = (used2 == 0) ? 1 : 3;
	game3 = (used3 == 0) ? 1 : 3;
	game4 = 1;

	if (rompo_y_salgo == 0)
		goto bucle_opc;

	if (num_ejec == 2)
		loadAndDecompressPic(menuBackground, backSurface, 1);
	else
		loadAndDecompressPic("99.alg", backSurface, 1);
	if (num_ejec != 5)
		withoutVerb();
}

void DrasculaEngine::response(int function) {
	if (num_ejec == 1) {
		if (function == 10)
			talk_drunk(1);
		else if (function == 11)
			talk_drunk(2);
		else if (function == 12)
			talk_drunk(3);
	} else if (num_ejec == 2) {
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
	} else if (num_ejec == 4) {
		if (function == 2)
			animation_2_4();
		else if (function == 3)
			animation_3_4();
		else if (function == 4)
			animation_4_4();
	} else if (num_ejec == 5) {
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
	} else if (num_ejec == 6) {
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

void DrasculaEngine::addObject(int osj) {
	int h, position = 0;

	for (h = 1; h < 43; h++) {
		if (inventoryObjects[h] == osj)
			position = 1;
	}

	if (position == 0) {
		for (h = 1; h < 43; h++) {
			if (inventoryObjects[h] == 0) {
				inventoryObjects[h] = osj;
				position = 1;
				break;
			}
		}
	}
}

void DrasculaEngine::stopSound_corte() {
	if (hay_sb == 1) {
		ctvd_stop();
		delete sku;
		sku = NULL;
		ctvd_terminate();
	}
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

void DrasculaEngine::ctvd_end() {
	_mixer->stopHandle(_soundHandle);
}

void DrasculaEngine::ctvd_stop() {
	_mixer->stopHandle(_soundHandle);
}

void DrasculaEngine::ctvd_terminate() {
//	_mixer->stopHandle(_soundHandle);
}

void DrasculaEngine::ctvd_speaker(int flag) {}

void DrasculaEngine::ctvd_output(Common::File *file_handle) {}

void DrasculaEngine::ctvd_init(int b) {
	int soundSize = sku->size();
	byte *soundData = (byte *)malloc(soundSize);
	sku->seek(32);
	sku->read(soundData, soundSize);
	_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_soundHandle, soundData, soundSize - 64,
					11025, Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);
}

bool DrasculaEngine::soundIsActive() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void DrasculaEngine::updateData() {
	if (num_ejec == 1) {
		// nothing
	} else if (num_ejec == 2) {
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
	} else if (num_ejec == 3) {
		// nothing
	} else if (num_ejec == 4) {
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
	} else if (num_ejec == 5) {
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
	} else if (num_ejec == 6) {
		if (roomNumber == 58 && flags[8] == 0)
			isDoor[1] = 0;
		if (roomNumber == 58 && flags[8] == 1)
			isDoor[1] = 1;
		if (roomNumber == 59)
			isDoor[1] = 0;
		if (roomNumber == 60) {
			sentido_dr = 0;
			x_dr = 155;
			y_dr = 69;
		}
	}
}

void DrasculaEngine::walkDown() {
	direccion_hare = 4;
	sentido_hare = 3;
	stepX = 0;
}

void DrasculaEngine::walkUp() {
	direccion_hare = 0;
	sentido_hare = 2;
	stepX = 0;
}

void DrasculaEngine::pon_vb() {
	int pos_vb[6];

	if (vb_se_mueve == 0) {
		pos_vb[0] = 256;
		pos_vb[1] = 129;
		pos_vb[2] = vb_x;
		pos_vb[3] = 66;
		pos_vb[4] = 33;
		pos_vb[5] = 69;
		if (sentido_vb == 0)
			pos_vb[0] = 222;
		else if (sentido_vb == 1)
			pos_vb[0] = 188;
	} else {
		pos_vb[2] = vb_x;
		pos_vb[3] = 66;
		pos_vb[4] = 28;
		pos_vb[5] = 68;

		if (sentido_vb == 0) {
			pos_vb[0] = frame_vb;
			pos_vb[1] = 62;
		} else {
			pos_vb[0] = frame_vb;
			pos_vb[1] = 131;
		}

		frame_vb = frame_vb + 29;
		if (frame_vb > 146)
			frame_vb = 1;
	}

	copyRectClip(pos_vb, frontSurface, screenSurface);
}

void DrasculaEngine::lleva_vb(int pointX) {
	sentido_vb = (pointX < vb_x) ? 0 : 1;
	vb_se_mueve = 1;

	for (;;) {
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		if (sentido_vb == 0) {
			vb_x = vb_x - 5;
			if (vb_x <= pointX)
				break;
		} else {
			vb_x = vb_x + 5;
			if (vb_x >= pointX)
				break;
		}
		pause(5);
	}

	vb_se_mueve = 0;
}

void DrasculaEngine::hipo_sin_nadie(int counter){
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

	do {
		counter--;

		copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
		if (num_ejec == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (sentido == 0)
			y++;
		else
			y--;

		if (num_ejec == 3) {
			if (y == 1)
				sentido = 1;
			if (y == -1)
				sentido = 0;
		} else {
			if (y == 2)
				sentido = 1;
			if (y == 0)
				sentido = 0;
		}
	} while (counter > 0);

	copyBackground(0, 0, 0, 0, 320, 200, drawSurface1, screenSurface);
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
}

void DrasculaEngine::openDoor(int nflag, int doorNum) {
	if (flags[nflag] == 0) {
		if (num_ejec == 1 /*|| num_ejec == 4*/) {
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
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		stopSound();
		withoutVerb();
	}
}

void DrasculaEngine::showMap() {
	int l, veo = 0;

	for (l = 0; l < numRoomObjs; l++) {
		if (mouseX > x1[l] && mouseY > y1[l]
				&& mouseX < x2[l] && mouseY < y2[l]
				&& visible[l] == 1) {
			strcpy(textName, objName[l]);
			hasName = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hasName = 0;
}

void DrasculaEngine::grr() {
	int length = 30;

	color_abc(kColorDarkGreen);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open("s10.als");
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(4);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

	updateRoom();
	copyBackground(253, 110, 150, 65, 20, 30, drawSurface3, screenSurface);

	if (withVoices == 0)
		centerText(".groaaarrrrgghhh!", 153, 65);

	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);

bucless:
	if (!isTalkFinished(&length))
		goto bucless;

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
}

void DrasculaEngine::activatePendulum() {
	flags[1] = 2;
	hare_se_ve = 0;
	roomNumber = 102;
	loadAndDecompressPic("102.alg", drawSurface1, HALF_PAL);
	loadAndDecompressPic("an_p1.alg", drawSurface3, 1);
	loadAndDecompressPic("an_p2.alg", extraSurface, 1);
	loadAndDecompressPic("an_p3.alg", frontSurface, 1);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	conta_blind_vez = getTime();
}

void DrasculaEngine::closeDoor(int nflag, int doorNum) {
	if (flags[nflag] == 1) {
		playSound(4);
		flags[nflag] = 0;
		if (doorNum != NO_DOOR)
			updateDoor(doorNum);
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, screenSurface);
		stopSound();
		withoutVerb();
	}
}

} // End of namespace Drascula

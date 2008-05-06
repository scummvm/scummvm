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

static const int x_obj[44] = {0, X_OBJ1, X_OBJ2, X_OBJ3, X_OBJ4, X_OBJ5, X_OBJ6, X_OBJ7, X_OBJ8, X_OBJ9, X_OBJ10,
				X_OBJ11, X_OBJ12, X_OBJ13, X_OBJ14, X_OBJ15, X_OBJ16, X_OBJ17, X_OBJ18, X_OBJ19, X_OBJ20,
				X_OBJ21, X_OBJ22, X_OBJ23, X_OBJ24, X_OBJ25, X_OBJ26, X_OBJ27, X_OBJ28, X_OBJ29, X_OBJ30,
				X_OBJ31, X_OBJ32, X_OBJ33, X_OBJ34, X_OBJ35, X_OBJ36, X_OBJ37, X_OBJ38, X_OBJ39, X_OBJ40,
				X_OBJ41, X_OBJ42, X_OBJ43};
static const int y_obj[44] = {0, Y_OBJ1, Y_OBJ2, Y_OBJ3, Y_OBJ4, Y_OBJ5, Y_OBJ6, Y_OBJ7, Y_OBJ8, Y_OBJ9, Y_OBJ10,
				Y_OBJ11, Y_OBJ12, Y_OBJ13, Y_OBJ14, Y_OBJ15, Y_OBJ16, Y_OBJ17, Y_OBJ18, Y_OBJ19, Y_OBJ20,
				Y_OBJ21, Y_OBJ22, Y_OBJ23, Y_OBJ24, Y_OBJ25, Y_OBJ26, Y_OBJ27, Y_OBJ28, Y_OBJ29, Y_OBJ30,
				Y_OBJ31, Y_OBJ32, Y_OBJ33, Y_OBJ34, Y_OBJ35, Y_OBJ36, Y_OBJ37, Y_OBJ38, Y_OBJ39, Y_OBJ40,
				Y_OBJ41, Y_OBJ42, Y_OBJ43};
static const int x_pol[44] = {0, 1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
				1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
				247, 83, 165, 1, 206, 42, 124, 83, 1, 247,
				83, 165, 1, 206, 42, 124, 83, 1, 247, 42,
				1, 165, 206};
static const int y_pol[44] = {0, 1, 1, 1, 1, 1, 1, 1, 27, 27, 1,
						27, 27, 27, 27, 27, 27, 27, 1, 1, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						27, 1, 1};
static const int x_barra[] = {6, 51, 96, 141, 186, 232, 276, 321};
static const int x1d_menu[] = {280, 40, 80, 120, 160, 200, 240, 0, 40, 80, 120,
						160, 200, 240, 0, 40, 80, 120, 160, 200, 240, 0,
						40, 80, 120, 160, 200, 240, 0};
static const int y1d_menu[] = {0, 0, 0, 0, 0, 0, 0, 25, 25, 25, 25, 25, 25, 25,
						50, 50, 50, 50, 50, 50, 50, 75, 75, 75, 75, 75, 75, 75, 100};
static int frame_x[20] = {43, 87, 130, 173, 216, 259};

int DrasculaEngine::init() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	return 0;
}

int DrasculaEngine::go() {
	num_ejec = 1; // values from 1 to 6 will start each part of game
	hay_que_load = 0;

	for (;;) {
		VGA = (byte *)malloc(320 * 200);
		memset(VGA, 0, 64000);

		lleva_objeto = 0;
		menu_bar = 0; menu_scr = 0; hay_nombre = 0;
		frame_y = 0;
		hare_x = -1; hare_se_mueve = 0; sentido_hare = 3; num_frame = 0; hare_se_ve = 1;
		comprueba_flags = 1;
		rompo = 0; rompo2 = 0;
		anda_a_objeto = 0;
		paso_x = PASO_HARE_X; paso_y = PASO_HARE_Y;
		alto_hare = ALTO_PERSONAJE; ancho_hare = ANCHO_PERSONAJE; alto_pies = PIES_HARE;
		alto_talk = ALTO_TALK_HARE; ancho_talk = ANCHO_TALK_HARE;
		hay_respuesta = 0;
		conta_ciego_vez = 0;
		cambio_de_color = 0;
		rompo_y_salgo = 0;
		vb_x = 120; sentido_vb = 1; vb_se_mueve = 0; frame_vb = 1;
		frame_piano = 0;
		frame_borracho = 0;
		frame_velas = 0;
		cont_sv = 0;
		term_int = 0;
		corta_musica = 0;
		hay_seleccion = 0;
		Leng = 0;
		UsingMem = 0;
		GlobalSpeed = 0;
		frame_ciego = 0;
		frame_ronquido = 0;
		frame_murcielago = 0;
		c_mirar = 0;
		c_poder = 0;
		ald = NULL;
		sku = NULL;

		asigna_memoria();

		hay_sb = 1;
		con_voces = 0;
		hay_seleccion = 0;

		if (num_ejec != 6) {
			loadPic("95.alg");
			decompressPic(dir_mesa, 1);
		}

		if (num_ejec == 1) {
			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);
		} else if (num_ejec == 2) {
			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			loadPic("pts.alg");
			decompressPic(dir_dibujo2, 1);
		} else if (num_ejec == 3) {
			loadPic("aux13.alg");
			decompressPic(dir_dibujo1, COMPLETA);
			loadPic("96.alg");
			decompressPic(dir_hare_frente, 1);
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
		} else if (num_ejec == 4) {
			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			if (hay_que_load == 0)
				animation_rayo();
			loadPic("96.alg");
			decompressPic(dir_hare_frente, 1);
			clearRoom();
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);
		} else if (num_ejec == 5) {
			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
		} else if (num_ejec == 6) {
			x_igor = 105, y_igor = 85, sentido_igor = 1;
			x_dr = 62, y_dr = 99, sentido_dr = 1;
			frame_pen = 0;
			flag_tv = 0;

			dir_pendulo = dir_dibujo3;

			loadPic("96.alg");
			decompressPic(dir_hare_frente, COMPLETA);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);

			loadPic("95.alg");
			decompressPic(dir_mesa, 1);
		}
		memset(nombre_icono, 0, sizeof(nombre_icono));
		strcpy(nombre_icono[1], "look");
		strcpy(nombre_icono[2], "take");
		strcpy(nombre_icono[3], "open");
		strcpy(nombre_icono[4], "close");
		strcpy(nombre_icono[5], "talk");
		strcpy(nombre_icono[6], "push");

		paleta_hare();
		if (!escoba()) {
			salir_al_dos(0);
			break;
		}
		salir_al_dos(0);
		if (num_ejec == 6)
			break;

		num_ejec++;
	}

	return 0;
}

void DrasculaEngine::salir_al_dos(int r) {
	if (hay_sb == 1)
		ctvd_end();
	clearRoom();
	Negro();
	MusicFadeout();
	stopmusic();
	libera_memoria();
	free(VGA);
}

void DrasculaEngine::asigna_memoria() {
	dir_zona_pantalla = (byte *)malloc(64000);
	assert(dir_zona_pantalla);
	dir_dibujo1 = (byte *)malloc(64000);
	assert(dir_dibujo1);
	dir_hare_fondo = (byte *)malloc(64000);
	assert(dir_hare_fondo);
	dir_dibujo3 = (byte *)malloc(64000);
	assert(dir_dibujo3);
	dir_dibujo2 = (byte *)malloc(64000);
	assert(dir_dibujo2);
	dir_mesa = (byte *)malloc(64000);
	assert(dir_mesa);
	dir_hare_dch = (byte *)malloc(64000);
	assert(dir_hare_dch);
	dir_hare_frente = (byte *)malloc(64000);
	assert(dir_hare_frente);
}

void DrasculaEngine::libera_memoria() {
	free(dir_zona_pantalla);
	free(dir_dibujo1);
	free(dir_hare_fondo);
	free(dir_dibujo2);
	free(dir_mesa);
	free(dir_dibujo3);
	free(dir_hare_dch);
	free(dir_hare_frente);
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

	Buffer_pcx = (byte *)malloc(65000);
	auxPun = Buffer_pcx;
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

void DrasculaEngine::decompressPic(byte *dir_escritura, int plt) {
	memcpy(dir_escritura, Buffer_pcx, 64000);
	free(Buffer_pcx);
	asigna_rgb((byte *)cPal, plt);
	if (plt > 1)
		funde_rgb(plt);
}

void DrasculaEngine::paleta_hare() {
	int color, componente;

	for (color = 235; color < 253; color++)
		for (componente = 0; componente < 3; componente++)
			palHare[color][componente] = palJuego[color][componente];

}

void DrasculaEngine::hare_oscuro() {
	int color, componente;

	for (color = 235; color < 253; color++ )
		for (componente = 0; componente < 3; componente++)
			palJuego[color][componente] = palHareOscuro[color][componente];

	ActualizaPaleta();
}

void DrasculaEngine::asigna_rgb(byte *dir_lectura, int plt) {
	int x, cnt = 0;

	for (x = 0; x < plt; x++) {
		palJuego[x][0] = dir_lectura[cnt++] / 4;
		palJuego[x][1] = dir_lectura[cnt++] / 4;
		palJuego[x][2] = dir_lectura[cnt++] / 4;
	}
	ActualizaPaleta();
}

void DrasculaEngine::funde_rgb(int plt) {}

void DrasculaEngine::Negro() {
	int color, componente;
	DacPalette256 palNegra;

	for (color = 0; color < 256; color++)
		for (componente = 0; componente < 3; componente++)
			palNegra[color][componente] = 0;

	palNegra[254][0] = 0x3F;
	palNegra[254][1] = 0x3F;
	palNegra[254][2] = 0x15;

	setvgapalette256((byte *)&palNegra);
}

void DrasculaEngine::ActualizaPaleta() {
	setvgapalette256((byte *)&palJuego);
}

void DrasculaEngine::setvgapalette256(byte *PalBuf) {
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

void DrasculaEngine::copyBackground(int xorg, int yorg, int xdes, int ydes, int Ancho,
								  int Alto, byte *Origen, byte *Destino) {
	int x;
	Destino += xdes + ydes * 320;
	Origen += xorg + yorg * 320;
	for (x = 0; x < Alto; x++) {
		memcpy(Destino, Origen, Ancho);
		Destino += 320;
		Origen += 320;
	}
}

void DrasculaEngine::copyRect(int xorg, int yorg, int xdes, int ydes, int Ancho,
								   int Alto, byte *Origen, byte *Destino) {
	int y, x;

	Destino += xdes + ydes * 320;
	Origen += xorg + yorg * 320;

	for (y = 0; y < Alto; y++)
		for (x = 0; x < Ancho; x++)
			if (Origen[x + y * 320] != 255)
				Destino[x + y * 320] = Origen[x + y * 320];
}

void DrasculaEngine::copyRectClip(int *Array, byte *Origen, byte *Destino) {
	int y, x;
	int xorg = Array[0];
	int yorg = Array[1];
	int xdes = Array[2];
	int ydes = Array[3];
	int Ancho = Array[4];
	int Alto = Array[5];

	if (ydes < 0) {
		yorg += -ydes;
		Alto += ydes;
		ydes = 0;
	}
	if (xdes < 0) {
		xorg += -xdes;
		Ancho += xdes;
		xdes = 0;
	}
	if ((xdes + Ancho) > 319)
		Ancho -= (xdes + Ancho) - 320;
	if ((ydes + Alto) > 199)
		Alto -= (ydes + Alto) - 200;

	Destino += xdes + ydes * 320;
	Origen += xorg + yorg * 320;

	for (y = 0; y < Alto; y++)
		for (x = 0; x < Ancho; x++)
			if (Origen[x + y * 320] != 255)
				Destino[x + y * 320] = Origen[x + y * 320];
}

void DrasculaEngine::updateScreen(int xorg, int yorg, int xdes, int ydes, int Ancho, int Alto, byte *Buffer) {
	int x;
	byte *ptr = VGA;

	ptr += xdes + ydes * 320;
	Buffer += xorg + yorg * 320;
	for (x = 0; x < Alto; x++) {
		memcpy(ptr, Buffer, Ancho);
		ptr += 320;
		Buffer += 320;
	}

	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

bool DrasculaEngine::escoba() {
	int n;

	dir_texto = dir_mesa;

	musica_antes = -1;

	if (num_ejec != 2) {
		int soc = 0;
		for (n = 0; n < 6; n++) {
			soc = soc + ANCHO_PERSONAJE;
			frame_x[n] = soc;
		}
	}

	for (n = 1; n < 43; n++)
		objetos_que_tengo[n] = 0;

	for (n = 0; n < NUM_BANDERAS; n++)
		flags[n] = 0;

	if (num_ejec == 2) {
		flags[16] = 1;
		flags[17] = 1;
		flags[27] = 1;
	}

	for (n = 1; n < 7; n++)
		objetos_que_tengo[n] = n;

	if (num_ejec == 1) {
		agarra_objeto(28);
		buffer_teclado();

		if (hay_que_load == 0)
			animation_1_1();

		sin_verbo();
		loadPic("2aux62.alg");
		decompressPic(dir_dibujo2, 1);
		sentido_hare = 1;
		obj_saliendo = 104;
		if (hay_que_load != 0) {
			if (!para_cargar(nom_partida)) {
				return true;
			}
		} else {
			carga_escoba("62.ald");
			hare_x = -20;
			hare_y = 56;
			lleva_al_hare(65, 145);
		}
	} else if (num_ejec == 2) {
		suma_objeto(28);
		buffer_teclado();
		sentido_hare = 3;
		obj_saliendo = 162;
		if (hay_que_load == 0)
			carga_escoba("14.ald");
		else {
			if (!para_cargar(nom_partida)) {
				return true;
			}
		}
	} else if (num_ejec == 3) {
		suma_objeto(28);
		suma_objeto(11);
		suma_objeto(14);
		suma_objeto(22);
		suma_objeto(9);
		suma_objeto(20);
		suma_objeto(19);
		flags[1] = 1;
		buffer_teclado();
		sentido_hare = 1;
		obj_saliendo = 99;
		if (hay_que_load == 0)
			carga_escoba("20.ald");
		else {
			if (!para_cargar(nom_partida)) {
				return true;
			}
		}
	} else if (num_ejec == 4) {
		suma_objeto(28);
		suma_objeto(9);
		suma_objeto(20);
		suma_objeto(22);
		buffer_teclado();
		obj_saliendo = 100;
		if (hay_que_load == 0) {
			carga_escoba("21.ald");
			sentido_hare = 0;
			hare_x = 235;
			hare_y = 164;
		} else {
			if (!para_cargar(nom_partida)) {
				return true;
			}
		}
	} else if (num_ejec == 5) {
		suma_objeto(28);
		suma_objeto(7);
		suma_objeto(9);
		suma_objeto(11);
		suma_objeto(13);
		suma_objeto(14);
		suma_objeto(15);
		suma_objeto(17);
		suma_objeto(20);
		buffer_teclado();
		sentido_hare = 1;
		obj_saliendo = 100;
		if (hay_que_load == 0) {
			carga_escoba("45.ald");
		} else {
			if (!para_cargar(nom_partida)) {
				return true;
			}
		}
	} else if (num_ejec == 6) {
		suma_objeto(28);
		suma_objeto(9);

		buffer_teclado();
		sentido_hare = 1;
		obj_saliendo = 104;
		if (hay_que_load == 0) {
			carga_escoba("58.ald");
			animation_1_6();
		} else {
			if (!para_cargar(nom_partida)) {
				return true;
			}
			loadPic("auxdr.alg");
			decompressPic(dir_dibujo2, 1);
		}
	}

bucles:
	if (hare_se_mueve == 0) {
		paso_x = PASO_HARE_X;
		paso_y = PASO_HARE_Y;
	}
	if (hare_se_mueve == 0 && anda_a_objeto == 1) {
		sentido_hare = sentido_final;
		anda_a_objeto = 0;
	}

	if (num_ejec == 2) {
		if ((!strcmp(num_room, "3.alg")) && (hare_x == 279) && (hare_y + alto_hare == 101))
			animation_1_2();
		else if ((!strcmp(num_room, "14.alg")) && (hare_x == 214) && (hare_y + alto_hare == 121))
			lleva_al_hare(190, 130);
		else if ((!strcmp(num_room, "14.alg")) && (hare_x == 246) && (hare_y + alto_hare == 112))
			lleva_al_hare(190, 130);
	}

	mueve_cursor();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (num_ejec == 2) {
		if (music_status() == 0 && musica_room != 0)
			playmusic(musica_room);
	} else {
		if (music_status() == 0)
			playmusic(musica_room);
	}

	MirarRaton();

	if (menu_scr == 0 && lleva_objeto == 1)
		comprueba_objetos();

	if (boton_dch == 1 && menu_scr == 1) {
		delay(100);
		if (num_ejec == 2)
			loadPic(fondo_y_menu);
		else
			loadPic("99.alg");
		decompressPic(dir_hare_fondo, 1);
		setvgapalette256((byte *)&palJuego);
		menu_scr = 0;
		espera_soltar();
		if (num_ejec != 3)
			cont_sv = 0;
	}
	if (boton_dch == 1 && menu_scr == 0) {
		delay(100);
		hare_se_mueve = 0;
		if (sentido_hare == 2)
			sentido_hare = 1;
		if (num_ejec == 4)
			loadPic("icons2.alg");
		else if (num_ejec == 5)
			loadPic("icons3.alg");
		else if (num_ejec == 6)
			loadPic("iconsp.alg");
		else
			loadPic("icons.alg");
		decompressPic(dir_hare_fondo, 1);
		menu_scr = 1;
		espera_soltar();
		sin_verbo();
		if (num_ejec != 3)
			cont_sv = 0;
	}

	if (boton_izq == 1 && menu_bar == 1) {
		delay(100);
		elige_en_barra();
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (boton_izq == 1 && lleva_objeto == 0) {
		delay(100);
		if (comprueba1())
			return true;
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (boton_izq == 1 && lleva_objeto == 1) {
		if (comprueba2())
			return true;
		if (num_ejec != 3)
			cont_sv = 0;
	}

	if (y_raton < 24 && menu_scr == 0)
		menu_bar = 1;
	else
		menu_bar = 0;

	Common::KeyCode key = getscan();
	if (key == Common::KEYCODE_F1 && menu_scr == 0) {
		elige_verbo(1);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F2 && menu_scr == 0) {
		elige_verbo(2);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F3 && menu_scr == 0) {
		elige_verbo(3);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F4 && menu_scr == 0) {
		elige_verbo(4);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F5 && menu_scr == 0) {
		elige_verbo(5);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_F6 && menu_scr == 0) {
		elige_verbo(6);
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
		sin_verbo();
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_v) {
		con_voces = 1;
		print_abc(_textsys[_lang][2], 96, 86);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1410);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_t) {
		con_voces = 0;
		print_abc(_textsys[_lang][3], 94, 86);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1460);
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (key == Common::KEYCODE_ESCAPE) {
		if (!confirma_salir())
			return false;
		if (num_ejec != 3)
			cont_sv = 0;
	} else if (num_ejec == 6 && key == Common::KEYCODE_0 && !strcmp(num_room, "61.alg")) {
		 loadPic("alcbar.alg");
		 decompressPic(dir_dibujo1, 255);
	} else if (cont_sv == 1500) {
		salva_pantallas();
		if (num_ejec != 3)
			cont_sv = 0;
	} else {
		if (num_ejec != 3)
			cont_sv++;
	}
	goto bucles;
}

void DrasculaEngine::agarra_objeto(int objeto) {
	if (num_ejec == 6)
		loadPic("iconsp.alg");
	else if (num_ejec == 4)
		loadPic("icons2.alg");
	else if (num_ejec == 5)
		loadPic("icons3.alg");
	else
		loadPic("icons.alg");
	decompressPic(dir_hare_fondo, 1);
	elige_objeto(objeto);
	if (num_ejec == 2)
		loadPic(fondo_y_menu);
	else
		loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
}

void DrasculaEngine::elige_objeto(int objeto) {
	if (num_ejec == 5) {
		if (lleva_objeto == 1 && menu_scr == 0 && objeto_que_lleva != 16)
			suma_objeto(objeto_que_lleva);
	} else {
		if (lleva_objeto == 1 && menu_scr == 0)
			suma_objeto(objeto_que_lleva);
	}
	copyBackground(x1d_menu[objeto], y1d_menu[objeto], 0, 0, ANCHOBJ,ALTOBJ, dir_hare_fondo, dir_dibujo3);
	lleva_objeto = 1;
	objeto_que_lleva = objeto;
}

int DrasculaEngine::resta_objeto(int osj) {
	int h, q = 0;

	for (h = 1; h < 43; h++) {
		if (objetos_que_tengo[h] == osj) {
			objetos_que_tengo[h] = 0;
			q = 1;
			break;
		}
	}

	if (q == 1)
		return 0;
	else
		return 1;
}

void DrasculaEngine::sin_verbo() {
	int c = 171;
	if (menu_scr == 1)
		c = 0;
	if (num_ejec == 5) {
		if (lleva_objeto == 1 && objeto_que_lleva != 16)
			suma_objeto(objeto_que_lleva);
	} else {
		if (lleva_objeto == 1)
			suma_objeto(objeto_que_lleva);
	}
	copyBackground(0, c, 0, 0, ANCHOBJ,ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 0;
	hay_nombre = 0;
}

bool DrasculaEngine::para_cargar(char nom_game[]) {
	musica_antes = musica_room;
	menu_scr = 0;
	if (num_ejec != 1)
		clearRoom();
	if (!carga_partida(nom_game))
		return false;
	if (num_ejec == 2 || num_ejec == 3 || num_ejec == 5) {
		delete ald;
		ald = NULL;
	}
	carga_escoba(datos_actuales);
	sin_verbo();

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
			if (c == '\n')
				break;
			if (b - buf >= (len - 1))
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

	hay_nombre = 0;

	strcpy(para_codificar, nom_fich);
	strcpy(datos_actuales, nom_fich);

	buffer_teclado();

	ald = new Common::File;
	ald->open(nom_fich);
	if (!ald->isOpen()) {
		error("missing data file");
	}
	int size = ald->size();
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", num_room);
	strcat(num_room, ".alg");

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &musica_room);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", roomDisk);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &nivel_osc);

	if (num_ejec == 2) {
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &martin);
		if (martin == 0)
			goto martini;
		ancho_hare = martin;
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&alto_hare);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&alto_pies);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&paso_x);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d",&paso_y);

		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant1);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant2);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant3);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s",pant4);

		loadPic(pant2);
		decompressPic(dir_hare_dch, 1);
		loadPic(pant1);
		decompressPic(dir_hare_frente, 1);
		loadPic(pant4);
		decompressPic(dir_hare_fondo, 1);

		strcpy(fondo_y_menu, pant4);
	}

martini:

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &objs_room);

	for (l = 0; l < objs_room; l++) {
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &num_obj[l]);
		getLine(ald, buffer, size);
		sscanf(buffer, "%s", nombre_obj[l]);
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
		sscanf(buffer, "%d", &espuerta[l]);
		if (espuerta[l] != 0) {
			getLine(ald, buffer, size);
			sscanf(buffer, "%s", alapantallakeva[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &x_alakeva[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &y_alakeva[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &sentido_alkeva[l]);
			getLine(ald, buffer, size);
			sscanf(buffer, "%d", &alapuertakeva[l]);
			puertas_cerradas(l);
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
		sscanf(buffer, "%d", &lejos);
		getLine(ald, buffer, size);
		sscanf(buffer, "%d", &cerca);
	}
	delete ald;
	ald = NULL;

	if (num_ejec == 2) {
		if (martin == 0) {
			paso_x = PASO_HARE_X;
			paso_y = PASO_HARE_Y;
			alto_hare = ALTO_PERSONAJE;
			ancho_hare = ANCHO_PERSONAJE;
			alto_pies = PIES_HARE;
			loadPic("97.alg");
			decompressPic(dir_hare_dch, 1);
			loadPic("96.alg");
			decompressPic(dir_hare_frente, 1);
			loadPic("99.alg");
			decompressPic(dir_hare_fondo, 1);

			strcpy(fondo_y_menu, "99.alg");
		}
	}

	for (l = 0; l < objs_room; l++) {
		if (num_obj[l] == obj_saliendo)
			obj_salir = l;
	}

	if (num_ejec == 2) {
		if (hare_x == -1) {
			hare_x = x_alakeva[obj_salir];
			hare_y = y_alakeva[obj_salir] - alto_hare;
		}
		hare_se_mueve = 0;
	}
	loadPic(roomDisk);
	decompressPic(dir_dibujo3, 1);

	loadPic(num_room);
	decompressPic(dir_dibujo1, MEDIA);

	copyBackground(0, 171, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	color_hare();
	if (nivel_osc != 0)
		funde_hare(nivel_osc);
	paleta_hare_claro();
	color_hare();
	funde_hare(nivel_osc + 2);
	paleta_hare_oscuro();

	hare_claro();
	cambio_de_color = -1;

	if (num_ejec == 2)
		color_abc(VERDE_CLARO);

	if (num_ejec != 2) {
		for (l = 0; l <= suelo_y1; l++)
			factor_red[l] = lejos;
		for (l = suelo_y1; l <= 201; l++)
			factor_red[l] = cerca;

		chiquez = (float)(cerca - lejos) / (float)(suelo_y2 - suelo_y1);
		for (l = suelo_y1; l <= suelo_y2; l++) {
			factor_red[l] = (int)(lejos + pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (!strcmp(num_room, "24.alg")) {
		for (l = suelo_y1 - 1; l > 74; l--) {
			factor_red[l] = (int)(lejos - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (num_ejec == 5 && !strcmp(num_room, "54.alg")) {
		for (l = suelo_y1 - 1; l > 84; l--) {
			factor_red[l] = (int)(lejos - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (num_ejec != 2) {
		if (hare_x == -1) {
			hare_x = x_alakeva[obj_salir];
			hare_y = y_alakeva[obj_salir];
			alto_hare = (ALTO_PERSONAJE * factor_red[hare_y]) / 100;
			ancho_hare = (ANCHO_PERSONAJE * factor_red[hare_y]) / 100;
			hare_y = hare_y - alto_hare;
		} else {
			alto_hare = (ALTO_PERSONAJE * factor_red[hare_y]) / 100;
			ancho_hare = (ANCHO_PERSONAJE * factor_red[hare_y]) / 100;
		}
		hare_se_mueve = 0;
	}

	if (num_ejec == 2) {
		soc = 0;
		for (l = 0; l < 6; l++) {
			soc = soc + ancho_hare;
			frame_x[l] = soc;
		}
	}

	if (num_ejec == 5)
		hare_se_ve = 1;

	actualiza_datos();

	if (num_ejec == 1)
		espuerta[7] = 0;

	if (num_ejec == 2) {
		if (!strcmp(num_room, "14.alg") && flags[39] == 1)
			musica_room = 16;
		else if (!strcmp(num_room, "15.alg") && flags[39] == 1)
			musica_room = 16;
		if (!strcmp(num_room, "14.alg") && flags[5] == 1)
			musica_room = 0;
		else if (!strcmp(num_room, "15.alg") && flags[5] == 1)
			musica_room = 0;

		if (musica_antes != musica_room && musica_room != 0)
			playmusic(musica_room);
		if (musica_room == 0)
			stopmusic();
	} else {
		if (musica_antes != musica_room && musica_room != 0)
			playmusic(musica_room);
	}

	if (num_ejec == 2) {
		if ((!strcmp(num_room, "9.alg")) || (strcmp(num_room, "2.alg")) || (!strcmp(num_room, "14.alg")) || (!strcmp(num_room, "18.alg")))
			conta_ciego_vez = vez();
	}
	if (num_ejec == 4) {
		if (!strcmp(num_room, "26.alg"))
			conta_ciego_vez = vez();
	}

	if (num_ejec == 4 && !strcmp(num_room, "24.alg") && flags[29] == 1)
		animation_7_4();

	if (num_ejec == 5) {
		if (!strcmp(num_room, "45.alg"))
			hare_se_ve = 0;
		if (!strcmp(num_room, "49.alg") && flags[7] == 0)
			animation_4_5();
	}

	updateRoom();
}

void DrasculaEngine::clearRoom() {
	memset(VGA, 0, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void DrasculaEngine::lleva_al_hare(int punto_x, int punto_y) {
	if (num_ejec == 5 || num_ejec == 6) {
		if (hare_se_ve == 0) {
			hare_x = sitio_x;
			hare_y = sitio_y;
			goto fin;
		}
	}
	sitio_x = punto_x;
	sitio_y = punto_y;
	empieza_andar();

	for (;;) {
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if (hare_se_mueve == 0)
			break;
	}

	if (anda_a_objeto == 1) {
		anda_a_objeto = 0;
		sentido_hare = sentido_final;
	}
fin:
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::mueve_cursor() {
	int pos_cursor[8];

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	updateRefresh_pre();
	pon_hare();
	updateRefresh();

	if (!strcmp(texto_nombre, "hacker") && hay_nombre == 1) {
		if (_color != ROJO && menu_scr == 0)
			color_abc(ROJO);
	} else if (menu_scr == 0 && _color != VERDE_CLARO)
		color_abc(VERDE_CLARO);
	if (hay_nombre == 1 && menu_scr == 0)
		centra_texto(texto_nombre, x_raton, y_raton);
	if (menu_scr == 1)
		menu_sin_volcar();
	else if (menu_bar == 1)
		barra_menu();

	pos_cursor[0] = 0;
	pos_cursor[1] = 0;
	pos_cursor[2] = x_raton - 20;
	pos_cursor[3] = y_raton - 17;
	pos_cursor[4] = ANCHOBJ;
	pos_cursor[5] = ALTOBJ;
	copyRectClip(pos_cursor, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::comprueba_objetos() {
	int l, veo = 0;

	for (l = 0; l < objs_room; l++) {
		if (x_raton > x1[l] && y_raton > y1[l]
				&& x_raton < x2[l] && y_raton < y2[l]
				&& visible[l] == 1 && espuerta[l] == 0) {
			strcpy(texto_nombre, nombre_obj[l]);
			hay_nombre = 1;
			veo = 1;
		}
	}

	if (num_ejec == 2) {
		if (x_raton > hare_x + 2 && y_raton > hare_y + 2
				&& x_raton < hare_x + ancho_hare - 2 && y_raton < hare_y + alto_hare - 2) {
			strcpy(texto_nombre, "hacker");
			hay_nombre = 1;
			veo = 1;
		}
	} else {
		if (x_raton > hare_x + 2 && y_raton > hare_y + 2
				&& x_raton < hare_x + ancho_hare - 2 && y_raton < hare_y + alto_hare - 2 && veo == 0) {
			strcpy(texto_nombre, "hacker");
			hay_nombre = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hay_nombre = 0;
}

void DrasculaEngine::espera_soltar() {
	update_events();
}

void DrasculaEngine::MirarRaton() {
	update_events();
}

void DrasculaEngine::elige_en_barra() {
	int n, num_verbo = -1;

	for (n = 0; n < 7; n++)
		if (x_raton > x_barra[n] && x_raton < x_barra[n + 1])
			num_verbo = n;

	if (num_verbo < 1)
		sin_verbo();
	else
		elige_verbo(num_verbo);
}

bool DrasculaEngine::comprueba1() {
	int l;

	if (menu_scr == 1)
		saca_objeto();
	else {
		for (l = 0; l < objs_room; l++) {
			if (x_raton >= x1[l] && y_raton >= y1[l]
					&& x_raton <= x2[l] && y_raton <= y2[l] && rompo == 0) {
				if (sal_de_la_habitacion(l))
					return true;
				if (rompo == 1)
					break;
			}
		}

		if (x_raton > hare_x && y_raton > hare_y
				&& x_raton < hare_x + ancho_hare && y_raton < hare_y + alto_hare)
			rompo = 1;

		for (l = 0; l < objs_room; l++) {
			if (x_raton > x1[l] && y_raton > y1[l]
					&& x_raton < x2[l] && y_raton < y2[l] && rompo == 0) {
				sitio_x = sitiobj_x[l];
				sitio_y = sitiobj_y[l];
				sentido_final = sentidobj[l];
				rompo = 1;
				anda_a_objeto = 1;
				empieza_andar();
			}
		}

		if (rompo == 0) {
			sitio_x = x_raton;
			sitio_y = y_raton;

			if (sitio_x < suelo_x1)
				sitio_x = suelo_x1;
			if (sitio_x > suelo_x2)
				sitio_x = suelo_x2;
			if (sitio_y < suelo_y1 + alto_pies)
				sitio_y = suelo_y1 + alto_pies;
			if (sitio_y > suelo_y2)
				sitio_y = suelo_y2;

			empieza_andar();
		}
		rompo = 0;
	}

	return false;
}

bool DrasculaEngine::comprueba2() {
	int l;

	if (menu_scr == 1) {
		if (coge_objeto())
			return true;
	} else {
		if (!strcmp(texto_nombre, "hacker") && hay_nombre == 1) {
			if (banderas(50))
				return true;
		} else {
			for (l = 0; l < objs_room; l++) {
				if (x_raton > x1[l] && y_raton > y1[l]
						&& x_raton < x2[l] && y_raton < y2[l] && visible[l] == 1) {
					sentido_final = sentidobj[l];
					anda_a_objeto = 1;
					lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
					if (banderas(num_obj[l]))
						return true;
					if (num_ejec == 4)
						break;
				}
			}
		}
	}

	return false;
}

Common::KeyCode DrasculaEngine::getscan() {
	update_events();

	return _keyPressed.keycode;
}

void DrasculaEngine::update_events() {
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
			x_raton = event.mouse.x;
			y_raton = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			boton_izq = 1;
			break;
		case Common::EVENT_LBUTTONUP:
			boton_izq = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			boton_dch = 1;
			break;
		case Common::EVENT_RBUTTONUP:
			boton_dch = 0;
			break;
		case Common::EVENT_QUIT:
			// TODO
			salir_al_dos(0);
			exit(0);
			break;
		default:
			break;
		}
	}
}

void DrasculaEngine::elige_verbo(int verbo) {
	int c = 171;

	if (menu_scr == 1)
		c = 0;
	if (num_ejec == 5) {
		if (lleva_objeto == 1 && objeto_que_lleva != 16)
			suma_objeto(objeto_que_lleva);
	} else {
		if (lleva_objeto == 1)
			suma_objeto(objeto_que_lleva);
	}

	copyBackground(ANCHOBJ * verbo, c, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 1;
	objeto_que_lleva = verbo;
}

void DrasculaEngine::mesa() {
	int nivel_master, nivel_voc, nivel_cd;

	copyRect(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);
	updateScreen(73, 63, 73, 63, 177, 97, dir_zona_pantalla);

	for (;;) {
		nivel_master = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4);
		nivel_voc = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4);
		nivel_cd = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4);

		updateRoom();

		copyRect(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);

		copyBackground(183, 56, 82, nivel_master, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);
		copyBackground(183, 56, 138, nivel_voc, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);
		copyBackground(183, 56, 194, nivel_cd, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);

		cursor_mesa();

		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		MirarRaton();

		if (boton_dch == 1) {
			delay(100);
			break;
		}
		if (boton_izq == 1) {
			delay(100);
			if (x_raton > 80 && x_raton < 121) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16;
				if (y_raton < nivel_master && vol < 15)
					vol++;
				if (y_raton > nivel_master && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol * 16);
			}

			if (x_raton > 136 && x_raton < 178) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16;
				if (y_raton < nivel_voc && vol < 15)
					vol++;
				if (y_raton > nivel_voc && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol * 16);
			}

			if (x_raton > 192 && x_raton < 233) {
				int vol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
				if (y_raton < nivel_cd && vol < 15)
					vol++;
				if (y_raton > nivel_cd && vol > 0)
					vol--;
				_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol * 16);
			}
		}

	}

	espera_soltar();
}

bool DrasculaEngine::saves() {
	char nombres[10][23];
	char fichero[13];
	int n, n2, num_sav = 0, y = 27;
	Common::InSaveFile *sav;

	clearRoom();

	if (!(sav = _saveFileMan->openForLoading("saves.epa"))) {
		Common::OutSaveFile *epa;
		if (!(epa = _saveFileMan->openForSaving("saves.epa")))
			error("Can't open saves.epa file.");
		for (n = 0; n < NUM_SAVES; n++)
			epa->writeString("*\n");
		epa->finalize();
		delete epa;
		if (!(sav = _saveFileMan->openForLoading("saves.epa"))) {
			error("Can't open saves.epa file.");
		}
	}
	for (n = 0; n < NUM_SAVES; n++)
		sav->readLine(nombres[n], 23);
	delete sav;

	loadPic("savescr.alg");
	decompressPic(dir_dibujo1, MEDIA);

	color_abc(VERDE_CLARO);

	for (;;) {
		y = 27;
		copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(nombres[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		cursor_mesa();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		y = 27;

		MirarRaton();

		if (boton_izq == 1) {
			delay(100);
			for (n = 0; n < NUM_SAVES; n++) {
				if (x_raton > 115 && y_raton > y + (9 * n) && x_raton < 115 + 175 && y_raton < y + 10 + (9 * n)) {
					strcpy(select, nombres[n]);

					if (strcmp(select, "*"))
						hay_seleccion = 1;
					else {
						introduce_nombre();
						strcpy(nombres[n], select);
						if (hay_seleccion == 1) {
							sprintf(fichero, "gsave%02d", n + 1);
							para_grabar(fichero);
							Common::OutSaveFile *tsav;
							if (!(tsav = _saveFileMan->openForSaving("saves.epa"))) {
								error("Can't open saves.epa file.");
							}
							for (n = 0; n < NUM_SAVES; n++) {
								tsav->writeString(nombres[n]);
								tsav->writeString("\n");
							}
							tsav->finalize();
							delete tsav;
						}
					}

					print_abc(select, 117, 15);
					y = 27;
					for (n2 = 0; n2 < NUM_SAVES; n2++) {
						print_abc(nombres[n2], 116, y);
						y = y + 9;
					}
					if (hay_seleccion == 1) {
						sprintf(fichero, "gsave%02d", n + 1);
					}
					num_sav = n;
				}
			}

			if (x_raton > 117 && y_raton > 15 && x_raton < 295 && y_raton < 24 && hay_seleccion == 1) {
				introduce_nombre();
				strcpy(nombres[num_sav], select);
				print_abc(select, 117, 15);
				y = 27;
				for (n2 = 0; n2 < NUM_SAVES; n2++) {
					print_abc(nombres[n2], 116, y);
					y = y + 9;
				}
			}

			if (x_raton > 125 && y_raton > 123 && x_raton < 199 && y_raton < 149 && hay_seleccion == 1) {
				if (!para_cargar(fichero))
					return false;
				break;
			} else if (x_raton > 208 && y_raton > 123 && x_raton < 282 && y_raton < 149 && hay_seleccion == 1) {
				para_grabar(fichero);
				Common::OutSaveFile *tsav;
				if (!(tsav = _saveFileMan->openForSaving("saves.epa"))) {
					error("Can't open saves.epa file.");
				}
				for (n = 0; n < NUM_SAVES; n++) {
					tsav->writeString(nombres[n]);
					tsav->writeString("\n");
				}
				tsav->finalize();
				delete tsav;
			} else if (x_raton > 168 && y_raton > 154 && x_raton < 242 && y_raton < 180)
				break;
			else if (hay_seleccion == 0) {
				print_abc("elige una partida", 117, 15);
			}
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			delay(400);
		}
		y = 26;
	}

	clearRoom();
	loadPic(num_room);
	decompressPic(dir_dibujo1, MEDIA);
	buffer_teclado();
	hay_seleccion = 0;

	return true;
}

void DrasculaEngine::print_abc(const char *dicho, int x_pantalla, int y_pantalla) {
	int pos_texto[8];
	int y_de_letra = 0, x_de_letra = 0, h, longitud;
	longitud = strlen(dicho);

	for (h = 0; h < longitud; h++) {
		y_de_letra = Y_ABC;
		int c = toupper(dicho[h]);
		if (c == 'A')
			x_de_letra = X_A;
		else if (c == 'B')
			x_de_letra = X_B;
		else if (c == 'C')
			x_de_letra = X_C;
		else if (c == 'D')
			x_de_letra = X_D;
		else if (c == 'E')
			x_de_letra = X_E;
		else if (c == 'F')
			x_de_letra = X_F;
		else if (c == 'G')
			x_de_letra = X_G;
		else if (c == 'H')
			x_de_letra = X_H;
		else if (c == 'I')
			x_de_letra = X_I;
		else if (c == 'J')
			x_de_letra = X_J;
		else if (c == 'K')
			x_de_letra = X_K;
		else if (c == 'L')
			x_de_letra = X_L;
		else if (c == 'M')
			x_de_letra = X_M;
		else if (c == 'N')
			x_de_letra = X_N;
		else if (c == 'O')
			x_de_letra = X_O;
		else if (c == 'P')
			x_de_letra = X_P;
		else if (c == 'Q')
			x_de_letra = X_Q;
		else if (c == 'R')
			x_de_letra = X_R;
		else if (c == 'S')
			x_de_letra = X_S;
		else if (c == 'T')
			x_de_letra = X_T;
		else if (c == 'U')
			x_de_letra = X_U;
		else if (c == 'V')
			x_de_letra = X_V;
		else if (c == 'W')
			x_de_letra = X_W;
		else if (c == 'X')
			x_de_letra = X_X;
		else if (c == 'Y')
			x_de_letra = X_Y;
		else if (c == 'Z')
			x_de_letra = X_Z;
		else if (c == 0xa7 || c == ' ')
			x_de_letra = ESPACIO;
		else {
			y_de_letra = Y_SIGNOS;
			if (c == '.')
				x_de_letra = X_PUNTO;
			else if (c == ',')
				x_de_letra = X_COMA;
			else if (c == '-')
				x_de_letra = X_GUION;
			else if (c == '?')
				x_de_letra = X_CIERRA_INTERROGACION;
			else if (c == 0xa8)
				x_de_letra = X_ABRE_INTERROGACION;
//			else if (c == '\'') // FIXME
//				x_de_letra = ESPACIO; // space for now
			else if (c == '"')
				x_de_letra = X_COMILLAS;
			else if (c == '!')
				x_de_letra = X_CIERRA_EXCLAMACION;
			else if (c == 0xad)
				x_de_letra = X_ABRE_EXCLAMACION;
			else if (c == ';')
				x_de_letra = X_PUNTO_Y_COMA;
			else if (c == '>')
				x_de_letra = X_MAYOR_QUE;
			else if (c == '<')
				x_de_letra = X_MENOR_QUE;
			else if (c == '$')
				x_de_letra = X_DOLAR;
			else if (c == '%')
				x_de_letra = X_POR_CIENTO;
			else if (c == ':')
				x_de_letra = X_DOS_PUNTOS;
			else if (c == '&')
				x_de_letra = X_AND;
			else if (c == '/')
				x_de_letra = X_BARRA;
			else if (c == '(')
				x_de_letra = X_ABRE_PARENTESIS;
			else if (c == ')')
				x_de_letra = X_CIERRA_PARENTESIS;
			else if (c == '*')
				x_de_letra = X_ASTERISCO;
			else if (c == '+')
				x_de_letra = X_MAS;
			else if (c == '1')
				x_de_letra = X_N1;
			else if (c == '2')
				x_de_letra = X_N2;
			else if (c == '3')
				x_de_letra = X_N3;
			else if (c == '4')
				x_de_letra = X_N4;
			else if (c == '5')
				x_de_letra = X_N5;
			else if (c == '6')
				x_de_letra = X_N6;
			else if (c == '7')
				x_de_letra = X_N7;
			else if (c == '8')
				x_de_letra = X_N8;
			else if (c == '9')
				x_de_letra = X_N9;
			else if (c == '0')
				x_de_letra = X_N0;
		}

		pos_texto[0] = x_de_letra;
		pos_texto[1] = y_de_letra;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = ANCHO_LETRAS;
		pos_texto[5] = ALTO_LETRAS;

		copyRectClip(pos_texto, dir_texto, dir_zona_pantalla);

		x_pantalla = x_pantalla + ANCHO_LETRAS;
		if (x_pantalla > 317) {
			x_pantalla = 0;
			y_pantalla = y_pantalla + ALTO_LETRAS + 2;
		}
	}
}

void DrasculaEngine::delay(int ms) {
	_system->delayMillis(ms * 2); // originaly was 1
}

bool DrasculaEngine::confirma_salir() {
	byte key;

	color_abc(ROJO);
	updateRoom();
	centra_texto(_textsys[_lang][1], 160, 87);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	delay(100);
	for (;;) {
		key = getscan();
		if (key != 0)
			break;
	}

	if (key == Common::KEYCODE_ESCAPE) {
		stopmusic();
		return false;
	}

	return true;
}

void DrasculaEngine::salva_pantallas() {
	int xr, yr;

	// FIXME: that part (*.ghost) need RE from efecto.lib file for some gfx special effect
	// for now ignore
	return;

	clearRoom();

	loadPic("sv.alg");
	decompressPic(dir_dibujo1, MEDIA);
	//TODO inicio_ghost();
	//TODO carga_ghost();

	MirarRaton();
	xr = x_raton;
	yr = y_raton;

	for (;;) {
		//TODO efecto(dir_dibujo1);
		MirarRaton();
		if (boton_dch == 1 || boton_izq == 1)
			break;
		if (x_raton != xr)
			break;
		if (y_raton != yr)
			break;
	}
	//TODO fin_ghost();
	loadPic(num_room);
	decompressPic(dir_dibujo1, MEDIA);
}

void DrasculaEngine::fliplay(const char *filefli, int vel) {
	OpenSSN(filefli, vel);
	while (PlayFrameSSN() && (!term_int)) {
		if (getscan() == Common::KEYCODE_ESCAPE)
			term_int = 1;
	}
	EndSSN();
}

void DrasculaEngine::FundeDelNegro(int VelocidadDeFundido) {
	char fundido;
	unsigned int color, componente;

	DacPalette256 palFundido;

	for (fundido = 0; fundido < 64; fundido++) {
		for (color = 0; color < 256; color++) {
			for (componente = 0; componente < 3; componente++) {
				palFundido[color][componente] = LimitaVGA(palJuego[color][componente] - 63 + fundido);
			}
		}
		pause(VelocidadDeFundido);

		setvgapalette256((byte *)&palFundido);
	}
}

void DrasculaEngine::color_abc(int cl) {
	_color = cl;

	if (cl == 0) {
		palJuego[254][0] = 0;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0;
	} else if (cl == 1) {
		palJuego[254][0] = 0x10;
		palJuego[254][1] = 0x3E;
		palJuego[254][2] = 0x28;
	} else if (cl == 3) {
		palJuego[254][0] = 0x16;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x16;
	} else if (cl == 4) {
		palJuego[254][0] = 0x9;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x12;
	} else if (cl == 5) {
		palJuego[254][0] = 0x3F;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x15;
	} else if (cl == 7) {
		palJuego[254][0] = 0x38;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0;
	} else if (cl == 8) {
		palJuego[254][0] = 0x3F;
		palJuego[254][1] = 0x27;
		palJuego[254][2] = 0x0B;
	} else if (cl == 9) {
		palJuego[254][0] = 0x2A;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0x2A;
	} else if (cl == 10) {
		palJuego[254][0] = 0x30;
		palJuego[254][1] = 0x30;
		palJuego[254][2] = 0x30;
	} else if (cl == 11) {
		palJuego[254][0] = 98;
		palJuego[254][1] = 91;
		palJuego[254][2] = 100;
	};

	setvgapalette256((byte *)&palJuego);
}

char DrasculaEngine::LimitaVGA(char valor) {
	return (valor & 0x3F) * (valor > 0);
}

void DrasculaEngine::centra_texto(const char *mensaje, int x_texto, int y_texto) {
	char bb[200], m2[200], m1[200], mb[10][50];
	char m3[200];
	int h, fil, x_texto3, x_texto2, x_texto1, conta_f = 0, ya = 0;

	strcpy(m1, " ");
	strcpy(m2, " ");
	strcpy(m3, " ");
	strcpy(bb, " ");

	for (h = 0; h < 10; h++)
		strcpy(mb[h], " ");

	if (x_texto > 160)
		ya = 1;

	strcpy(m1, mensaje);
	if (x_texto < 60)
		x_texto = 60;
	if (x_texto > 255)
		x_texto = 255;

	x_texto1 = x_texto;

	if (ya == 1)
		x_texto1 = 315 - x_texto;

	x_texto2 = (strlen(m1) / 2) * ANCHO_LETRAS;

tut:
	strcpy(bb, m1);
	scumm_strrev(bb);

	if (x_texto1 < x_texto2) {
		strcpy(m3, strrchr(m1, ' '));
		strcpy(m1, strstr(bb, " "));
		scumm_strrev(m1);
		m1[strlen(m1) - 1] = '\0';
		strcat(m3, m2);
		strcpy(m2, m3);
	};

	x_texto2 = (strlen(m1) / 2) * ANCHO_LETRAS;
	if (x_texto1 < x_texto2)
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

	fil = y_texto - (((conta_f + 3) * ALTO_LETRAS));

	for (h = 0; h < conta_f + 1; h++) {
		x_texto3 = strlen(mb[h]) / 2;
		print_abc(mb[h], ((x_texto) - x_texto3 * ANCHO_LETRAS) - 1, fil);
		fil = fil + ALTO_LETRAS + 2;
	}
}

void DrasculaEngine::comienza_sound(const char *fichero) {
	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(fichero);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
	}
	ctvd_init(2);
	ctvd_speaker(1);
	ctvd_output(sku);
}

void DrasculaEngine::anima(const char *animation, int FPS) {
	Common::File FileIn;
	unsigned j;
	int NFrames = 1;
	int cnt = 2;

	AuxBuffLast = (byte *)malloc(65000);
	AuxBuffDes = (byte *)malloc(65000);

	FileIn.open(animation);

	if (!FileIn.isOpen()) {
		error("No encuentro un fichero de animation.");
	}

	FileIn.read(&NFrames, sizeof(NFrames));
	FileIn.read(&Leng, sizeof(Leng));
	AuxBuffOrg = (byte *)malloc(Leng);
	FileIn.read(AuxBuffOrg, Leng);
	FileIn.read(cPal, 768);
	carga_pcx(AuxBuffOrg);
	free(AuxBuffOrg);
	memcpy(VGA, AuxBuffDes, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
	set_dac(cPal);
	memcpy(AuxBuffLast, AuxBuffDes, 64000);
	WaitForNext(FPS);
	while (cnt < NFrames) {
		FileIn.read(&Leng, sizeof(Leng));
		AuxBuffOrg = (byte *)malloc(Leng);
		FileIn.read(AuxBuffOrg, Leng);
		FileIn.read(cPal, 768);
		carga_pcx(AuxBuffOrg);
		free(AuxBuffOrg);
		for (j = 0;j < 64000; j++) {
			VGA[j] = AuxBuffLast[j] = AuxBuffDes[j] ^ AuxBuffLast[j];
		}
		_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		_system->updateScreen();
		WaitForNext(FPS);
		cnt++;
		byte key = getscan();
		if (key == Common::KEYCODE_ESCAPE)
			term_int = 1;
		if (key != 0)
			break;
	}
	free(AuxBuffLast);
	free(AuxBuffDes);
	FileIn.close();
}

void DrasculaEngine::animafin_sound_corte() {
	if (hay_sb == 1) {
		ctvd_stop();
		delete sku;
		sku = NULL;
		ctvd_terminate();
	}
}

void DrasculaEngine::FundeAlNegro(int VelocidadDeFundido) {
	char fundido;
	unsigned int color, componente;

	DacPalette256 palFundido;

	for (fundido = 63; fundido >= 0; fundido--) {
		for (color = 0; color < 256; color++) {
			for (componente = 0; componente < 3; componente++) {
				palFundido[color][componente] = LimitaVGA(palJuego[color][componente] - 63 + fundido);
			}
		}
		pause(VelocidadDeFundido);

		setvgapalette256((byte *)&palFundido);
	}
}

void DrasculaEngine::pause(int cuanto) {
	_system->delayMillis(cuanto * 30); // was originaly 2
}

void DrasculaEngine::pon_igor() {
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

	copyRectClip(pos_igor, dir_hare_frente, dir_zona_pantalla);
}

void DrasculaEngine::pon_dr() {
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
		copyRectClip(pos_dr, dir_dibujo2, dir_zona_pantalla);
	else
		copyRectClip(pos_dr, dir_hare_fondo, dir_zona_pantalla);
}

void DrasculaEngine::pon_bj() {
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

	copyRectClip(pos_bj, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::hipo(int contador) {
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

comienza:
	contador--;

	updateRoom();
	if (num_ejec == 3)
		updateScreen(0, 0, 0, y, 320, 200, dir_zona_pantalla);
	else
		updateScreen(0, 1, 0, y, 320, 198, dir_zona_pantalla);

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
	if (contador > 0)
		goto comienza;

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::fin_sound() {
	delay(1);

	if (hay_sb == 1) {
		while (LookForFree() != 0);
		delete sku;
		sku = NULL;
	}
}

void DrasculaEngine::playmusic(int p) {
	AudioCD.stop();
	AudioCD.play(p - 1, 1, 0, 0);
}

void DrasculaEngine::stopmusic() {
	AudioCD.stop();
}

int DrasculaEngine::music_status() {
	return AudioCD.isPlaying();
}

void DrasculaEngine::updateRoom() {
	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateRefresh_pre();
	if (num_ejec == 3) {
		if (flags[0] == 0)
			pon_hare();
		else
			copyRect(113, 54, hare_x - 20, hare_y - 1, 77, 89, dir_dibujo3, dir_zona_pantalla);
	} else {
		pon_hare();
	}
	updateRefresh();
}

bool DrasculaEngine::carga_partida(const char *nom_game) {
	int l, n_ejec2;
	Common::InSaveFile *sav;

	if (!(sav = _saveFileMan->openForLoading(nom_game))) {
		error("missing savegame file");
	}

	n_ejec2 = sav->readSint32LE();
	if (n_ejec2 != num_ejec) {
		strcpy(nom_partida, nom_game);
		num_ejec = n_ejec2 - 1;
		hay_que_load = 1;
		return false;
	}
	sav->read(datos_actuales, 20);
	hare_x = sav->readSint32LE();
	hare_y = sav->readSint32LE();
	sentido_hare = sav->readSint32LE();

	for (l = 1; l < 43; l++) {
		objetos_que_tengo[l] = sav->readSint32LE();
	}

	for (l = 0; l < NUM_BANDERAS; l++) {
		flags[l] = sav->readSint32LE();
	}

	lleva_objeto = sav->readSint32LE();
	objeto_que_lleva = sav->readSint32LE();
	hay_que_load = 0;

	return true;
}

void DrasculaEngine::puertas_cerradas(int l) {
	if (num_ejec == 1 || num_ejec == 3 || num_ejec == 5 || num_ejec == 6)
		return;
	else if (num_ejec == 2) {
		if (num_obj[l] == 138 && flags[0] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 138 && flags[0] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 136 && flags[8] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 136 && flags[8] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 156 && flags[16] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 156 && flags[16] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 163 && flags[17] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 163 && flags[17] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 177 && flags[15] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 177 && flags[15] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 175 && flags[40] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 175 && flags[40] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 173 && flags[36] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 173 && flags[36] == 1)
			espuerta[l] = 1;
	} else if (num_ejec == 4) {
		if (num_obj[l] == 101 && flags[0] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 101 && flags[0] == 1 && flags[28] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 103 && flags[0] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 103 && flags[0] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 104 && flags[1] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 104 && flags[1] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 105 && flags[1] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 105 && flags[1] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 106 && flags[2] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 106 && flags[2] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 107 && flags[2] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 107 && flags[2] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 110 && flags[6] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 110 && flags[6] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 114 && flags[4] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 114 && flags[4] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 115 && flags[4] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 115 && flags[4] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 116 && flags[5] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 116 && flags[5] == 1 && flags[23] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 117 && flags[5] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 117 && flags[5] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 120 && flags[8] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 120 && flags[8] == 1)
			espuerta[l] = 1;
		else if (num_obj[l] == 122 && flags[7] == 0)
			espuerta[l] = 0;
		else if (num_obj[l] == 122 && flags[7] == 1)
			espuerta[l] = 1;
	}
}

void DrasculaEngine::color_hare() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++) {
			palJuego[color][componente] = palHare[color][componente];
		}
	}
	ActualizaPaleta();
}

void DrasculaEngine::funde_hare(int oscuridad) {
	char fundido;
	unsigned int color, componente;

	for (fundido = oscuridad; fundido >= 0; fundido--) {
		for (color = 235; color < 253; color++) {
			for (componente = 0; componente < 3; componente++)
				palJuego[color][componente] = LimitaVGA(palJuego[color][componente] - 8 + fundido);
		}
	}

	ActualizaPaleta();
}

void DrasculaEngine::paleta_hare_claro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palHareClaro[color][componente] = palJuego[color][componente];
	}
}

void DrasculaEngine::paleta_hare_oscuro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palHareOscuro[color][componente] = palJuego[color][componente];
	}
}

void DrasculaEngine::hare_claro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palJuego[color][componente] = palHareClaro[color][componente];
	}

	ActualizaPaleta();
}

void DrasculaEngine::empieza_andar() {
	hare_se_mueve = 1;

	paso_x = PASO_HARE_X;
	paso_y = PASO_HARE_Y;

	if (num_ejec == 2) {
		if ((sitio_x < hare_x) && (sitio_y <= (hare_y + alto_hare)))
			cuadrante_1();
		else if ((sitio_x < hare_x) && (sitio_y > (hare_y + alto_hare)))
			cuadrante_3();
		else if ((sitio_x > hare_x + ancho_hare) && (sitio_y <= (hare_y + alto_hare)))
			cuadrante_2();
		else if ((sitio_x > hare_x + ancho_hare) && (sitio_y > (hare_y + alto_hare)))
			cuadrante_4();
		else if (sitio_y < hare_y + alto_hare)
			anda_parriba();
		else if (sitio_y > hare_y + alto_hare)
			anda_pabajo();
	} else {
		if ((sitio_x < hare_x + ancho_hare / 2 ) && (sitio_y <= (hare_y + alto_hare)))
			cuadrante_1();
		else if ((sitio_x < hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
			cuadrante_3();
		else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y <= (hare_y + alto_hare)))
			cuadrante_2();
		else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
			cuadrante_4();
		else
			hare_se_mueve = 0;
	}
	conta_vez = vez();
}

void DrasculaEngine::pon_hare() {
	int pos_hare[6];
	int r;

	if (hare_se_mueve == 1 && paso_x == PASO_HARE_X) {
		for (r = 0; r < paso_x; r++) {
			if (num_ejec != 2) {
				if (sentido_hare == 0 && sitio_x - r == hare_x + ancho_hare / 2) {
					hare_se_mueve = 0;
					paso_x = PASO_HARE_X;
					paso_y = PASO_HARE_Y;
				}
				if (sentido_hare == 1 && sitio_x + r == hare_x + ancho_hare / 2) {
					hare_se_mueve = 0;
					paso_x = PASO_HARE_X;
					paso_y = PASO_HARE_Y;
					hare_x = sitio_x - ancho_hare / 2;
					hare_y = sitio_y - alto_hare;
				}
			} else if (num_ejec == 2) {
				if (sentido_hare == 0 && sitio_x - r == hare_x) {
					hare_se_mueve = 0;
					paso_x = PASO_HARE_X;
					paso_y = PASO_HARE_Y;
				}
				if (sentido_hare == 1 && sitio_x + r == hare_x + ancho_hare) {
					hare_se_mueve = 0;
					paso_x = PASO_HARE_X;
					paso_y = PASO_HARE_Y;
					hare_x = sitio_x - ancho_hare + 4;
					hare_y = sitio_y - alto_hare;
				}
			}
		}
	}
	if (hare_se_mueve == 1 && paso_y == PASO_HARE_Y) {
		for (r = 0; r < paso_y; r++) {
			if (sentido_hare == 2 && sitio_y - r == hare_y + alto_hare) {
				hare_se_mueve = 0;
				paso_x = PASO_HARE_X;
				paso_y = PASO_HARE_Y;
			}
			if (sentido_hare == 3 && sitio_y + r == hare_y + alto_hare) {
				hare_se_mueve = 0;
				paso_x = PASO_HARE_X;
				paso_y = PASO_HARE_Y;
			}
		}
	}

	if (num_ejec == 1 || num_ejec == 4 || num_ejec == 5 || num_ejec == 6) {
		if (hare_se_ve == 0)
			goto no_vuelco;
	}

	if (hare_se_mueve == 0) {
		pos_hare[0] = 0;
		pos_hare[1] = DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		} else {
			pos_hare[4] = ANCHO_PERSONAJE;
			pos_hare[5] = ALTO_PERSONAJE;
		}

		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_dch, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_dch, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 2) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_fondo, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_fondo, dir_zona_pantalla);
		} else {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_frente, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
		}
	} else if (hare_se_mueve == 1) {
		pos_hare[0] = frame_x[num_frame];
		pos_hare[1] = frame_y + DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		} else {
			pos_hare[4] = ANCHO_PERSONAJE;
			pos_hare[5] = ALTO_PERSONAJE;
		}
		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_dch, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_dch, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 2) {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_fondo, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_fondo, dir_zona_pantalla);
		} else {
			if (num_ejec == 2)
				copyRectClip(pos_hare, dir_hare_frente, dir_zona_pantalla);
			else
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
		}

no_vuelco:
		aumenta_num_frame();
	}
}

void DrasculaEngine::menu_sin_volcar() {
	int h, n, x;
	char texto_icono[13];

	x = sobre_que_objeto();
	strcpy(texto_icono, nombre_icono[x]);

	for (n = 1; n < 43; n++) {
		h = objetos_que_tengo[n];

		if (h != 0) {
			if (num_ejec == 6)
				copyBackground(x_pol[n], y_pol[n], x_obj[n], y_obj[n],
						ANCHOBJ, ALTOBJ, dir_mesa, dir_zona_pantalla);
			else
				copyBackground(x_pol[n], y_pol[n], x_obj[n], y_obj[n],
						ANCHOBJ, ALTOBJ, dir_hare_frente, dir_zona_pantalla);
		}
		copyRect(x1d_menu[h], y1d_menu[h], x_obj[n], y_obj[n],
				ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_zona_pantalla);
	}

	if (x < 7)
		print_abc(texto_icono, x_obj[x] - 2, y_obj[x] - 7);
}

void DrasculaEngine::barra_menu() {
	int n, sobre_verbo = 1;

	for (n = 0; n < 7; n++) {
		if (x_raton > x_barra[n] && x_raton < x_barra[n + 1])
			sobre_verbo = 0;
		copyRect(ANCHOBJ * n, ALTOBJ * sobre_verbo, x_barra[n], 2,
						ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_zona_pantalla);
		sobre_verbo = 1;
	}
}

void DrasculaEngine::saca_objeto() {
	int h = 0, n;

	updateRoom();

	for (n = 1; n < 43; n++){
		if (sobre_que_objeto() == n) {
			h = objetos_que_tengo[n];
			objetos_que_tengo[n] = 0;
			if (h != 0)
				lleva_objeto = 1;
		}
	}

	espera_soltar();

	if (lleva_objeto == 1)
		elige_objeto(h);
}

bool DrasculaEngine::sal_de_la_habitacion(int l) {
	char salgo[13];

	if (num_ejec == 1) {
		if (num_obj[l] == 105 && flags[0] == 0)
			talk(_text[_lang][442], "442.als");
		else {
			puertas_cerradas(l);
			if (espuerta[l] != 0) {
				lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
				sentido_hare = sentidobj[l];
				updateRoom();
				updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				hare_se_mueve = 0;
				sentido_hare = sentido_alkeva[l];
				obj_saliendo = alapuertakeva[l];
				rompo = 1;
				musica_antes = musica_room;

				if (num_obj[l] == 105) {
					animation_2_1();
					return true;
				}
				clearRoom();
				strcpy(salgo, alapantallakeva[l]);
				strcat(salgo, ".ald");
				hare_x = -1;
				carga_escoba(salgo);
			}
		}
	} else if (num_ejec == 2) {
		puertas_cerradas(l);
		if (espuerta[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			if (num_obj[l] == 136)
				animation_2_2();
			if (num_obj[l] == 124)
				animation_3_2();
			if (num_obj[l] == 173) {
				animation_35_2();
				return true;
			} if (num_obj[l] == 146 && flags[39] == 1) {
				flags[5] = 1;
				flags[11] = 1;
			}
			if (num_obj[l] == 176 && flags[29] == 1) {
				flags[29] = 0;
				resta_objeto(23);
				suma_objeto(11);
			}
			clearRoom();
			delete ald;
			ald = NULL;
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 3) {
		puertas_cerradas(l);
		if (espuerta[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			clearRoom();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 4) {
		puertas_cerradas(l);
		if (espuerta[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;

			if (num_obj[l] == 108)
				lleva_al_hare(171, 78);
			clearRoom();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 5) {
		puertas_cerradas(l);
		if (espuerta[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			hare_se_ve = 1;
			clearRoom();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);
		}
	} else if (num_ejec == 6) {
		puertas_cerradas(l);
		if (espuerta[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			updateRoom();
			updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			clearRoom();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);

			if (obj_saliendo == 105)
				animation_19_6();
		}
	}

	return false;
}

bool DrasculaEngine::coge_objeto() {
	int h, n;
	h = objeto_que_lleva;
	comprueba_flags = 1;

	updateRoom();
	menu_sin_volcar();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (objeto_que_lleva < 7)
		goto usando_verbos;

	for (n = 1; n < 43; n++) {
		if (sobre_que_objeto() == n && objetos_que_tengo[n] == 0) {
			objetos_que_tengo[n] = h;
			lleva_objeto = 0;
			comprueba_flags = 0;
		}
	}

usando_verbos:

	if (comprueba_flags == 1) {
		if (comprueba_banderas_menu())
			return true;
	}
	espera_soltar();
	if (lleva_objeto == 0)
		sin_verbo();

	return false;
}

bool DrasculaEngine::banderas(int fl) {
	hare_se_mueve = 0;
	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	hay_respuesta = 1;

	if (menu_scr == 1) {
		if (num_ejec == 1) {
			if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
		} else if (num_ejec == 2) {
			if ((objeto_que_lleva == LOOK && fl == 22 && flags[23] == 0)
					|| (objeto_que_lleva == OPEN && fl == 22 && flags[23] == 0)) {
				talk(_text[_lang][164], "164.als");
				flags[23] = 1;
				sin_verbo();
				suma_objeto(7);
				suma_objeto(18);
			} else if (objeto_que_lleva == LOOK && fl == 22 && flags[23] == 1)
				talk(_text[_lang][307], "307.als");
			else if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
			else if (objeto_que_lleva == LOOK && fl == 7)
				talk(_text[_lang][143], "143.als");
			else if (objeto_que_lleva == TALK && fl == 7)
				talk(_text[_lang][144], "144.als");
			else if (objeto_que_lleva == LOOK && fl == 8)
				talk(_text[_lang][145], "145.als");
			else if (objeto_que_lleva == TALK && fl == 8)
				talk(_text[_lang][146], "146.als");
			else if (objeto_que_lleva == LOOK && fl == 9)
				talk(_text[_lang][147], "147.als");
			else if (objeto_que_lleva == TALK && fl == 9)
				talk(_text[_lang][148], "148.als");
			else if (objeto_que_lleva == LOOK && fl == 10)
				talk(_text[_lang][151], "151.als");
			else if (objeto_que_lleva == LOOK && fl == 11)
				talk(_text[_lang][152], "152.als");
			else if (objeto_que_lleva == TALK && fl == 11)
				talk(_text[_lang][153], "153.als");
			else if (objeto_que_lleva == LOOK && fl == 12)
				talk(_text[_lang][154], "154.als");
			else if (objeto_que_lleva == LOOK && fl == 13)
				talk(_text[_lang][155], "155.als");
			else if (objeto_que_lleva == LOOK && fl == 14)
				talk(_text[_lang][157], "157.als");
			else if (objeto_que_lleva == LOOK && fl == 15)
				talk(_text[_lang][58], "58.als");
			else if (objeto_que_lleva == LOOK && fl == 16)
				talk(_text[_lang][158], "158.als");
			else if (objeto_que_lleva == LOOK && fl == 17)
				talk(_text[_lang][159], "159.als");
			else if (objeto_que_lleva == LOOK && fl == 18)
				talk(_text[_lang][160], "160.als");
			else if (objeto_que_lleva == LOOK && fl == 19)
				talk(_text[_lang][161], "161.als");
			else if (objeto_que_lleva == LOOK && fl == 20)
				talk(_text[_lang][162], "162.als");
			else if (objeto_que_lleva == LOOK && fl == 23)
				talk(_text[_lang][152], "152.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 3) {
			if (objeto_que_lleva == LOOK && fl == 22)
				talk(_text[_lang][307], "307.als");
			else if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
			else if (objeto_que_lleva == LOOK && fl == 7)
				talk(_text[_lang][143], "143.als");
			else if (objeto_que_lleva == TALK && fl == 7)
				talk(_text[_lang][144], "144.als");
			else if (objeto_que_lleva == LOOK && fl == 8)
				talk(_text[_lang][145], "145.als");
			else if (objeto_que_lleva == TALK && fl == 8)
				talk(_text[_lang][146], "146.als");
			else if (objeto_que_lleva == LOOK && fl == 9)
				talk(_text[_lang][147], "147.als");
			else if (objeto_que_lleva == TALK && fl == 9)
				talk(_text[_lang][148], "148.als");
			else if (objeto_que_lleva == LOOK && fl == 10)
				talk(_text[_lang][151], "151.als");
			else if (objeto_que_lleva == LOOK && fl == 11)
				talk(_text[_lang][152], "152.als");
			else if (objeto_que_lleva == TALK && fl == 11)
				talk(_text[_lang][153], "153.als");
			else if (objeto_que_lleva == LOOK && fl == 12)
				talk(_text[_lang][154], "154.als");
			else if (objeto_que_lleva == LOOK && fl == 13)
				talk(_text[_lang][155], "155.als");
			else if (objeto_que_lleva == LOOK && fl == 14)
				talk(_text[_lang][157], "157.als");
			else if (objeto_que_lleva == LOOK && fl == 15)
				talk(_text[_lang][58], "58.als");
			else if (objeto_que_lleva == LOOK && fl == 16)
				talk(_text[_lang][158], "158.als");
			else if (objeto_que_lleva == LOOK && fl == 17)
				talk(_text[_lang][159], "159.als");
			else if (objeto_que_lleva == LOOK && fl == 18)
				talk(_text[_lang][160], "160.als");
			else if (objeto_que_lleva == LOOK && fl == 19)
				talk(_text[_lang][161], "161.als");
			else if (objeto_que_lleva == LOOK && fl == 20)
				talk(_text[_lang][162], "162.als");
			else if (objeto_que_lleva == LOOK && fl == 23)
				talk(_text[_lang][152], "152.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 4) {
			if ((objeto_que_lleva == 18 && fl == 19) || (objeto_que_lleva == 19 && fl == 18)) {
				sin_verbo();
				elige_objeto(21);
				resta_objeto(18);
				resta_objeto(19);
			} else if ((objeto_que_lleva == 14 && fl == 19) || (objeto_que_lleva == 19 && fl == 14))
				talk(_text[_lang][484], "484.als");
			else if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
			else if (objeto_que_lleva == LOOK && fl == 7)
				talk(_text[_lang][478], "478.als");
			else if (objeto_que_lleva == LOOK && fl == 8)
				talk(_text[_lang][480], "480.als");
			else if (objeto_que_lleva == LOOK && fl == 9) {
				talk(_text[_lang][482], "482.als");
				talk(_text[_lang][483], "483.als");
			} else if (objeto_que_lleva == LOOK && fl == 10)
				talk(_text[_lang][485], "485.als");
			else if (objeto_que_lleva == LOOK && fl == 11)
				talk(_text[_lang][488], "488.als");
			else if (objeto_que_lleva == LOOK && fl == 12)
				talk(_text[_lang][486], "486.als");
			else if (objeto_que_lleva == LOOK && fl == 13)
				talk(_text[_lang][490], "490.als");
			else if (objeto_que_lleva == LOOK && fl == 14)
				talk(_text[_lang][122], "122.als");
			else if (objeto_que_lleva == LOOK && fl == 15)
				talk(_text[_lang][117], "117.als");
			else if (objeto_que_lleva == TALK && fl == 15)
				talk(_text[_lang][118], "118.als");
			else if (objeto_que_lleva == OPEN && fl == 15)
				talk(_text[_lang][119], "119.als");
			else if (objeto_que_lleva == LOOK && fl == 16)
				talk(_text[_lang][491], "491.als");
			else if (objeto_que_lleva == LOOK && fl == 17)
				talk(_text[_lang][478], "478.als");
			else if (objeto_que_lleva == LOOK && fl == 18)
				talk(_text[_lang][493], "493.als");
			else if (objeto_que_lleva == LOOK && fl == 19) {
				talk(_text[_lang][494], "494.als");
				talk(_text[_lang][495], "495.als");
			} else if (objeto_que_lleva == LOOK && fl == 20)
				talk(_text[_lang][162], "162.als");
			else if (objeto_que_lleva == LOOK && fl == 21)
				talk(_text[_lang][496], "496.als");
			else if (objeto_que_lleva == LOOK && fl == 22)
				talk(_text[_lang][161], "161.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 5) {
			if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
			else if (objeto_que_lleva == LOOK && fl == 7)
				talk(_text[_lang][478],"478.als");
			else if (objeto_que_lleva == LOOK && fl == 8)
				talk(_text[_lang][120], "120.als");
			else if (objeto_que_lleva == LOOK && fl == 9) {
				talk(_text[_lang][482], "482.als");
				talk(_text[_lang][483], "483.als");
			} else if (objeto_que_lleva == LOOK && fl == 11)
				talk(_text[_lang][488], "488.als");
			else if (objeto_que_lleva == LOOK && fl == 13)
				talk(_text[_lang][490], "490.als");
			else if (objeto_que_lleva == LOOK && fl == 14)
				talk(_text[_lang][121], "121.als");
			else if (objeto_que_lleva == LOOK && fl == 15)
				talk(_text[_lang][117], "117.als");
			else if (objeto_que_lleva == TALK && fl == 15)
				talk(_text[_lang][118], "118.als");
			else if (objeto_que_lleva == OPEN && fl == 15)
				talk(_text[_lang][119], "119.als");
			else if (objeto_que_lleva == LOOK && fl == 17)
				talk(_text[_lang][478], "478.als");
			else if (objeto_que_lleva == LOOK && fl == 20)
				talk(_text[_lang][162], "162.als"); 
			else
				hay_respuesta = 0;
		} else if (num_ejec == 6) {
			if (objeto_que_lleva == LOOK && fl == 28)
				talk(_text[_lang][328], "328.als");
			else if (objeto_que_lleva == LOOK && fl == 9) {
				talk(_text[_lang][482], "482.als");
				talk(_text[_lang][483], "483.als");
			} else if (objeto_que_lleva == LOOK && fl == 20)
				talk(_text[_lang][123], "123.als");
			else if (objeto_que_lleva == LOOK && fl == 21)
				talk(_text[_lang][441], "441.als");
			else
				hay_respuesta = 0;
		}
	} else {
		if (num_ejec == 1) {
			if (objeto_que_lleva == LOOK && fl == 50)
				talk(_text[_lang][308], "308.als");
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk(_text[_lang][310], "310.als");
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk(_text[_lang][311], "311.als");
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk(_text[_lang][312], "312.als");
			else if (objeto_que_lleva == PICK && fl == 50)
				talk(_text[_lang][313], "313.als");
			else if (objeto_que_lleva == TALK && fl == 50)
				talk(_text[_lang][314], "314.als");
			else if (!strcmp(num_room, "62.alg"))
				room_62(fl);
			else if (!strcmp(num_room, "63.alg"))
				room_63(fl);
			else
				hay_respuesta = 0;
		} else if (num_ejec == 2) {
			if (objeto_que_lleva == LOOK && fl == 50)
				talk(_text[_lang][308], "308.als");
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk(_text[_lang][310], "310.als");
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk(_text[_lang][311], "311.als");
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk(_text[_lang][312], "312.als");
			else if (objeto_que_lleva == PICK && fl == 50)
				talk(_text[_lang][313], "313.als");
			else if (objeto_que_lleva == TALK && fl == 50)
				talk(_text[_lang][314], "314.als");
			else if (objeto_que_lleva == 11 && fl == 50 && flags[22] == 0 && strcmp(num_room, "18.alg"))
				talk(_text[_lang][315], "315.als");
			else if (objeto_que_lleva == 13 && fl == 50)
				talk(_text[_lang][156], "156.als");
			else if (objeto_que_lleva == 20 && fl == 50)
				talk(_text[_lang][163], "163.als");
			else if (!strcmp(num_room, "1.alg"))
				room_1(fl);
			else if (!strcmp(num_room, "3.alg"))
				room_3(fl);
			else if (!strcmp(num_room, "4.alg"))
				room_4(fl);
			else if (!strcmp(num_room, "5.alg"))
				room_5(fl);
			else if (!strcmp(num_room, "6.alg"))
				room_6(fl);
			else if (!strcmp(num_room, "7.alg"))
				room_7(fl);
			else if (!strcmp(num_room, "8.alg"))
				room_8(fl);
			else if (!strcmp(num_room, "9.alg"))
				room_9(fl);
			else if (!strcmp(num_room, "12.alg"))
				room_12(fl);
			else if (!strcmp(num_room, "14.alg"))
				room_14(fl);
			else if (!strcmp(num_room, "15.alg"))
				room_15(fl);
			else if (!strcmp(num_room, "16.alg"))
				room_16(fl);
			else if (!strcmp(num_room, "17.alg"))
				room_17(fl);
			else if (!strcmp(num_room, "18.alg"))
				room_18(fl);
			else if (!strcmp(num_room, "19.alg"))
				room_19(fl);
			else
				hay_respuesta = 0;
		} else if (num_ejec == 3) {
			if (objeto_que_lleva == LOOK && fl == 50)
				talk(_text[_lang][309], "309.als");
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk(_text[_lang][310], "310.als");
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk(_text[_lang][311], "311.als");
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk(_text[_lang][312], "312.als");
			else if (objeto_que_lleva == PICK && fl == 50)
				talk(_text[_lang][313], "313.als");
			else if (objeto_que_lleva == TALK && fl == 50)
				talk(_text[_lang][314], "314.als");
			else if (!strcmp(num_room, "13.alg")) {
				if (room_13(fl))
					return true;
			} else
				hay_respuesta = 0;
		} else if (num_ejec == 4) {
			if (!strcmp(num_room, "28.alg"))
				talk(_text[_lang][178], "178.als");
			else if (objeto_que_lleva == LOOK && fl == 50)
				talk(_text[_lang][309], "309.als");
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk(_text[_lang][310], "310.als");
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk(_text[_lang][311], "311.als");
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk(_text[_lang][312], "312.als");
			else if (objeto_que_lleva == PICK && fl == 50)
				talk(_text[_lang][313], "313.als");
			else if (objeto_que_lleva == TALK && fl == 50)
				talk(_text[_lang][314], "314.als");
			else if (objeto_que_lleva == 8 && fl == 50 && flags[18] == 0)
				talk(_text[_lang][481], "481.als");
			else if (objeto_que_lleva == 9 && fl == 50)
				talk(_text[_lang][484], "484.als");
			else if (objeto_que_lleva == 12 && fl == 50 && flags[18] == 0)
				talk(_text[_lang][487], "487.als");
			else if (objeto_que_lleva == 20 && fl == 50)
				talk(_text[_lang][487], "487.als");
			else if (!strcmp(num_room, "21.alg")) {
				if (room_21(fl))
					return true;
			} else if (!strcmp(num_room, "22.alg"))
				room_22(fl);
			else if (!strcmp(num_room, "23.alg"))
				room_23(fl);
			else if (!strcmp(num_room, "24.alg"))
				room_24(fl);
			else if (!strcmp(num_room, "26.alg"))
				room_26(fl);
			else if (!strcmp(num_room, "27.alg"))
				room_27(fl);
			else if (!strcmp(num_room, "29.alg"))
				room_29(fl);
			else if (!strcmp(num_room, "30.alg"))
				room_30(fl);
			else if (!strcmp(num_room, "31.alg"))
				room_31(fl);
			else if (!strcmp(num_room, "34.alg"))
				room_34(fl);
			else if (!strcmp(num_room, "35.alg"))
				room_35(fl);
			else if (!strcmp(num_room, "44.alg"))
				room_44(fl);
			else
				hay_respuesta = 0;
		} else if (num_ejec == 5) {
			if (objeto_que_lleva == LOOK && fl == 50)
				talk("Cuanto mas me miro, mas me gusto", "54.als");
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk("y luego como me cierro", "19.als");
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk("Tendre que abrirme primero no", "19.als");
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk("Estoy bien donde estoy", "19.als");
			else if (objeto_que_lleva == PICK && fl == 50)
				talk("Ya me tengo", "11.als");
			else if (objeto_que_lleva == TALK && fl == 50)
				talk("hola yo", "16.als");
			else if (objeto_que_lleva == 20 && fl == 50)
				talk(_text[_lang][487], "487.als");
			else if (!strcmp(num_room, "49.alg"))
				room_49(fl);
			else if (!strcmp(num_room, "53.alg"))
				room_53(fl);
			else if (!strcmp(num_room, "54.alg"))
				room_54(fl);
			else if (!strcmp(num_room, "55.alg"))
				room_55(fl);
			else if (!strcmp(num_room, "56.alg")) {
				if (room_56(fl))
					return true;
			} else
				hay_respuesta = 0;
		} else if (num_ejec == 6) {
			if (objeto_que_lleva == LOOK && fl == 50 && flags[0] == 1)
				talk(_text[_lang][308], "308.als");
			else if (objeto_que_lleva == LOOK && fl == 50 && flags[0] == 0)
				talk(_text[_lang][310], "250.als" );
			else if (objeto_que_lleva == OPEN && fl == 50)
				talk(_text[_lang][310], "310.als" );
			else if (objeto_que_lleva == CLOSE && fl == 50)
				talk(_text[_lang][311], "311.als" );
			else if (objeto_que_lleva == MOVE && fl == 50)
				talk(_text[_lang][312], "312.als" );
			else if (objeto_que_lleva == PICK && fl == 50)
				talk(_text[_lang][313], "313.als" );
			else if (objeto_que_lleva == TALK && fl == 50)
				talk(_text[_lang][314], "314.als" );
			else if (!strcmp(num_room, "102.alg"))
				room_pendulo(fl);
			else if (!strcmp(num_room, "58.alg"))
				room_58(fl);
			else if (!strcmp(num_room, "59.alg"))
				room_59(fl);
			else if (!strcmp(num_room, "60.alg")) {
				if (room_60(fl))
					return true;
			} else if (!strcmp(num_room, "61.alg"))
				room_61(fl);
			else
				hay_respuesta = 0;
		}
	}
	if (hay_respuesta == 0 && hay_nombre == 1)
		room_0();
	else if (hay_respuesta == 0 && menu_scr == 1)
		room_0();

	return false;
}

void DrasculaEngine::cursor_mesa() {
	int pos_cursor[8];

	pos_cursor[0] = 225;
	pos_cursor[1] = 56;
	pos_cursor[2] = x_raton - 20;
	pos_cursor[3] = y_raton - 12;
	pos_cursor[4] = 40;
	pos_cursor[5] = 25;

	copyRectClip(pos_cursor, dir_mesa, dir_zona_pantalla);
}

void DrasculaEngine::introduce_nombre() {
	Common::KeyCode key;
	int v = 0, h = 0;
	char select2[23];
	strcpy(select2, "                      ");
	for (;;) {
		buffer_teclado();
		select2[v] = '-';
		copyBackground(115, 14, 115, 14, 176, 9, dir_dibujo1, dir_zona_pantalla);
		print_abc(select2, 117, 15);
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		key = getscan();
		delay(70);
		if (key != 0) {
			if (key == Common::KEYCODE_q)
				select2[v] = 'q';
			else if (key == Common::KEYCODE_w)
				select2[v] = 'w';
			else if (key == Common::KEYCODE_e)
				select2[v] = 'e';
			else if (key == Common::KEYCODE_r)
				select2[v] = 'r';
			else if (key == Common::KEYCODE_t)
				select2[v] = 't';
			else if (key == Common::KEYCODE_y)
				select2[v] = 'y';
			else if (key == Common::KEYCODE_u)
				select2[v] = 'u';
			else if (key == Common::KEYCODE_i)
				select2[v] = 'i';
			else if (key == Common::KEYCODE_o)
				select2[v] = 'o';
			else if (key == Common::KEYCODE_p)
				select2[v] = 'p';
			else if (key == Common::KEYCODE_a)
				select2[v] = 'a';
			else if (key == Common::KEYCODE_s)
				select2[v] = 's';
			else if (key == Common::KEYCODE_d)
				select2[v] = 'd';
			else if (key == Common::KEYCODE_f)
				select2[v] = 'f';
			else if (key == Common::KEYCODE_g)
				select2[v] = 'g';
			else if (key == Common::KEYCODE_h)
				select2[v] = 'h';
			else if (key == Common::KEYCODE_j)
				select2[v] = 'j';
			else if (key == Common::KEYCODE_k)
				select2[v] = 'k';
			else if (key == Common::KEYCODE_l)
				select2[v] = 'l';
			else if ((key == Common::KEYCODE_LCTRL) || (key == Common::KEYCODE_RCTRL))
				select2[v] = '\164';
			else if (key == Common::KEYCODE_z)
				select2[v] = 'z';
			else if (key == Common::KEYCODE_x)
				select2[v] = 'x';
			else if (key == Common::KEYCODE_c)
				select2[v] = 'c';
			else if (key == Common::KEYCODE_v)
				select2[v] = 'v';
			else if (key == Common::KEYCODE_b)
				select2[v] = 'b';
			else if (key == Common::KEYCODE_n)
				select2[v] = 'n';
			else if (key == Common::KEYCODE_m)
				select2[v] = 'm';
			else if (key == Common::KEYCODE_1)
				select2[v] = '1';
			else if (key == Common::KEYCODE_2)
				select2[v] = '2';
			else if (key == Common::KEYCODE_3)
				select2[v] = '3';
			else if (key == Common::KEYCODE_4)
				select2[v] = '4';
			else if (key == Common::KEYCODE_5)
				select2[v] = '5';
			else if (key == Common::KEYCODE_6)
				select2[v] = '6';
			else if (key == Common::KEYCODE_7)
				select2[v] = '7';
			else if (key == Common::KEYCODE_8)
				select2[v] = '8';
			else if (key == Common::KEYCODE_9)
				select2[v] = '9';
			else if (key == Common::KEYCODE_0)
				select2[v] = '0';
			else if (key == Common::KEYCODE_SPACE)
				select2[v] = '\167';
			else if (key == ESC)
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

void DrasculaEngine::para_grabar(char nom_game[]) {
	graba_partida(nom_game);
	comienza_sound("99.als");
	fin_sound();
}

void DrasculaEngine::OpenSSN(const char *Name, int Pause) {
	MiVideoSSN = (byte *)malloc(64256);
	GlobalSpeed = 1000 / Pause;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	_Sesion = new Common::File;
	_Sesion->open(Name);
	mSesion = TryInMem(_Sesion);
	LastFrame = _system->getMillis();
}

int DrasculaEngine::PlayFrameSSN() {
	int Exit = 0;
	int Lengt;
	byte *BufferSSN;

	if (!UsingMem)
		_Sesion->read(&CHUNK, 1);
	else {
		memcpy(&CHUNK, mSesion, 1);
		mSesion += 1;
	}
	switch (CHUNK) {
	case SET_PALET:
		if (!UsingMem)
			_Sesion->read(dacSSN, 768);
		else {
			memcpy(dacSSN, mSesion, 768);
			mSesion += 768;
		}
		set_dacSSN(dacSSN);
		break;
	case EMPTY_FRAME:
		WaitFrameSSN();
		break;
	case INIT_FRAME:
		if (!UsingMem) {
			_Sesion->read(&CMP, 1);
			_Sesion->read(&Lengt, 4);
		} else {
			memcpy(&CMP, mSesion, 1);
			mSesion += 1;
			memcpy(&Lengt, mSesion, 4);
			mSesion += 4;
		}
		if (CMP == CMP_RLE) {
			if (!UsingMem) {
				BufferSSN = (byte *)malloc(Lengt);
				_Sesion->read(BufferSSN, Lengt);
			} else {
				BufferSSN = (byte *)malloc(Lengt);
				memcpy(BufferSSN, mSesion, Lengt);
				mSesion += Lengt;
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
			if (CMP == CMP_OFF) {
				if (!UsingMem) {
					BufferSSN = (byte *)malloc(Lengt);
					_Sesion->read(BufferSSN, Lengt);
				} else {
					BufferSSN = (byte *)malloc(Lengt);
					memcpy(BufferSSN, mSesion, Lengt);
					mSesion += Lengt;
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
	case END_ANIM:
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
		_Sesion->close();
		delete _Sesion;
	}
}

byte *DrasculaEngine::TryInMem(Common::File *Sesion) {
	int Lengt;

	Sesion->seek(0, SEEK_END);
	Lengt = Sesion->pos();
	Sesion->seek(0, SEEK_SET);
	pointer = (byte *)malloc(Lengt);
	if (pointer == NULL)
		return NULL;
	Sesion->read(pointer, Lengt);
	UsingMem = 1;
	Sesion->close();
	delete Sesion;
	return pointer;
}

void DrasculaEngine::set_dacSSN(byte *PalBuf) {
	setvgapalette256((byte *)PalBuf);
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
	while ((now = _system->getMillis()) - LastFrame < ((uint32) GlobalSpeed))
		_system->delayMillis(GlobalSpeed - (now - LastFrame));
	LastFrame = LastFrame + GlobalSpeed;
}

byte *DrasculaEngine::carga_pcx(byte *NamePcc) {
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
	setvgapalette256((byte *)dac);
}

void DrasculaEngine::WaitForNext(int FPS) {
	_system->delayMillis(1000 / FPS);
}

int DrasculaEngine::vez() {
	return _system->getMillis() / 20; // originaly was 1
}

void DrasculaEngine::reduce_hare_chico(int xx1, int yy1, int xx2, int yy2, int ancho, int alto, int factor, byte *dir_inicio, byte *dir_fin) {
	float suma_x, suma_y;
	int n, m;
	float pixel_x, pixel_y;
	int pos_pixel[6];

	nuevo_ancho = (ancho * factor) / 100;
	nuevo_alto = (alto * factor) / 100;

	suma_x = ancho / nuevo_ancho;
	suma_y = alto / nuevo_alto;

	pixel_x = xx1;
	pixel_y = yy1;

	for (n = 0; n < nuevo_alto; n++) {
		for (m = 0; m < nuevo_ancho; m++) {
			pos_pixel[0] = (int)pixel_x;
			pos_pixel[1] = (int)pixel_y;
			pos_pixel[2] = xx2 + m;
			pos_pixel[3] = yy2 + n;
			pos_pixel[4] = 1;
			pos_pixel[5] = 1;

			copyRectClip(pos_pixel, dir_inicio, dir_fin);

			pixel_x = pixel_x + suma_x;
		}
		pixel_x = xx1;
		pixel_y = pixel_y + suma_y;
	}
}

char DrasculaEngine::codifica(char car) {
	return ~car;
}

void DrasculaEngine::cuadrante_1() {
	float distancia_x, distancia_y;

	if (num_ejec == 2)
		distancia_x = hare_x - sitio_x;
	else
		distancia_x = hare_x + ancho_hare / 2 - sitio_x;

	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 0;
		sentido_hare = 2;
		paso_x = (int)(distancia_x / (distancia_y / PASO_HARE_Y));
	} else {
		direccion_hare = 7;
		sentido_hare = 0;
		paso_y = (int)(distancia_y / (distancia_x / PASO_HARE_X));
	}
}

void DrasculaEngine::cuadrante_2() {
	float distancia_x, distancia_y;

	if (num_ejec == 2)
		distancia_x = abs(hare_x + ancho_hare - sitio_x);
	else
		distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);

	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 1;
		sentido_hare = 2;
		paso_x = (int)(distancia_x / (distancia_y / PASO_HARE_Y));
	} else {
		direccion_hare = 2;
		sentido_hare = 1;
		paso_y = (int)(distancia_y / (distancia_x / PASO_HARE_X));
	}
}

void DrasculaEngine::cuadrante_3() {
	float distancia_x, distancia_y;

	if (num_ejec == 2)
		distancia_x = hare_x - sitio_x;
	else
		distancia_x = hare_x + ancho_hare / 2 - sitio_x;

	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x < distancia_y) {
		direccion_hare = 5;
		sentido_hare = 3;
		paso_x = (int)(distancia_x / (distancia_y / PASO_HARE_Y));
	} else {
		direccion_hare = 6;
		sentido_hare = 0;
		paso_y = (int)(distancia_y / (distancia_x / PASO_HARE_X));
	}
}

void DrasculaEngine::cuadrante_4() {
	float distancia_x, distancia_y;

	if (num_ejec == 2)
		distancia_x = abs(hare_x + ancho_hare - sitio_x);
	else
		distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);

	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x < distancia_y) {
		direccion_hare = 4;
		sentido_hare = 3;
		paso_x = (int)(distancia_x / (distancia_y / PASO_HARE_Y));
	} else {
		direccion_hare = 3;
		sentido_hare = 1;
		paso_y = (int)(distancia_y / (distancia_x / PASO_HARE_X));
	}
}

void DrasculaEngine::graba_partida(char nom_game[]) {
	Common::OutSaveFile *out;
	int l;

	if (!(out = _saveFileMan->openForSaving(nom_game))) {
		error("no puedo abrir el archivo");
	}
	out->writeSint32LE(num_ejec);
	out->write(datos_actuales, 20);
	out->writeSint32LE(hare_x);
	out->writeSint32LE(hare_y);
	out->writeSint32LE(sentido_hare);

	for (l = 1; l < 43; l++) {
		out->writeSint32LE(objetos_que_tengo[l]);
	}

	for (l = 0; l < NUM_BANDERAS; l++) {
		out->writeSint32LE(flags[l]);
	}

	out->writeSint32LE(lleva_objeto);
	out->writeSint32LE(objeto_que_lleva);

	out->finalize();
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", nom_game);

	delete out;
}

void DrasculaEngine::aumenta_num_frame() {
	diff_vez = vez() - conta_vez;

	if (diff_vez >= 6) {
		conta_vez = vez();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (direccion_hare == 0) {
			hare_x = hare_x - paso_x;
			hare_y = hare_y - paso_y;
		} else if (direccion_hare == 7) {
			hare_x = hare_x - paso_x;
			hare_y = hare_y - paso_y;
		} else if (direccion_hare == 1) {
			hare_x = hare_x + paso_x;
			hare_y = hare_y - paso_y;
		} else if (direccion_hare == 2) {
			hare_x = hare_x + paso_x;
			hare_y = hare_y - paso_y;
		} else if (direccion_hare == 3) {
			hare_x = hare_x + paso_x;
			hare_y = hare_y + paso_y;
		} else if (direccion_hare == 4) {
			hare_x = hare_x + paso_x;
			hare_y = hare_y + paso_y;
		} else if (direccion_hare == 5) {
			hare_x = hare_x - paso_x;
			hare_y = hare_y + paso_y;
		} else if (direccion_hare == 6) {
			hare_x = hare_x - paso_x;
			hare_y = hare_y + paso_y;
		}
	}

	if (num_ejec != 2) {
		diferencia_y = (int)(alto_hare - nuevo_alto);
		diferencia_x = (int)(ancho_hare - nuevo_ancho);
		hare_y = hare_y + diferencia_y;
		hare_x = hare_x + diferencia_x;
		alto_hare = (int)nuevo_alto;
		ancho_hare = (int)nuevo_ancho;
	}
}

int DrasculaEngine::sobre_que_objeto() {
	int n = 0;

	for (n = 1; n < 43; n++) {
		if (x_raton > x_obj[n] && y_raton > y_obj[n]
				&& x_raton < x_obj[n] + ANCHOBJ && y_raton < y_obj[n] + ALTOBJ)
			break;
	}

	return n;
}

bool DrasculaEngine::comprueba_banderas_menu() {
	int h, n;

	for (n = 0; n < 43; n++) {
		if (sobre_que_objeto() == n) {
			h = objetos_que_tengo[n];
			if (h != 0)
				if (banderas(h))
					return true;
		}
	}

	return false;
}

void DrasculaEngine::conversa(const char *nom_fich) {
	int h;
	int juego1 = 1, juego2 = 1, juego3 = 1, juego4 = 1;
	char frase1[78];
	char frase2[78];
	char frase3[87];
	char frase4[78];
	char para_codificar[13];
	char suena1[13];
	char suena2[13];
	char suena3[13];
	char suena4[13];
	int longitud;
	int respuesta1;
	int respuesta2;
	int respuesta3;
	int usado1 = 0;
	int usado2 = 0;
	int usado3 = 0;
	char buffer[256];

	rompo_y_salgo = 0;

	strcpy(para_codificar, nom_fich);

	if (num_ejec == 5)
		sin_verbo();

	ald = new Common::File;
	ald->open(nom_fich);
	if (!ald->isOpen()) {
		error("missing data file");
	}
	int size = ald->size();

	getLine(ald, buffer, size);
	sscanf(buffer, "%s", frase1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", frase2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", frase3);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", frase4);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", suena1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", suena2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", suena3);
	getLine(ald, buffer, size);
	sscanf(buffer, "%s", suena4);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &respuesta1);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &respuesta2);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &respuesta3);
	delete ald;
	ald = NULL;

	if (num_ejec == 2 && !strcmp(nom_fich, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(frase3, _text[_lang][405]);
		strcpy(suena3, "405.als");
		respuesta3 = 31;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_12.cal") && flags[7] == 1) {
		strcpy(frase3, _text[_lang][273]);
		strcpy(suena3, "273.als");
		respuesta3 = 14;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_12.cal") && flags[10] == 1) {
		strcpy(frase3, " cuanto queda para que acabe el partido?");
		strcpy(suena3, "274.als");
		respuesta3 = 15;
	}

	longitud = strlen(frase1);
	for (h = 0; h < longitud; h++)
		if (frase1[h] == (char)0xa7)
			frase1[h] = ' ';

	longitud = strlen(frase2);
	for (h = 0; h < longitud; h++)
		if (frase2[h] == (char)0xa7)
			frase2[h] = ' ';

	longitud = strlen(frase3);
	for (h = 0; h < longitud; h++)
		if (frase3[h] == (char)0xa7)
			frase3[h] = ' ';

	longitud = strlen(frase4);
	for (h = 0; h < longitud; h++)
		if (frase4[h] == (char)0xa7)
			frase4[h] = ' ';

	loadPic("car.alg");
	decompressPic(dir_hare_fondo, 1);
	// TODO code here should limit y position for mouse in dialog menu,
	// but we can't implement this due lack backend functionality
	// from 1(top) to 31
	color_abc(VERDE_CLARO);

bucle_opc:

	updateRoom();

	if (num_ejec == 1 || num_ejec == 4 || num_ejec == 6) {
		if (music_status() == 0 && flags[11] == 0)
			playmusic(musica_room);
	} else if (num_ejec == 2) {
		if (music_status() == 0 && flags[11] == 0 && musica_room != 0)
			playmusic(musica_room);
	} else if (num_ejec == 3 || num_ejec == 5) {
		if (music_status() == 0)
			playmusic(musica_room);
	}

	MirarRaton();

	if (y_raton > 0 && y_raton < 9) {
		if (usado1 == 1 && _color != BLANCO)
			color_abc(BLANCO);
		else if (usado1 == 0 && _color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (y_raton > 8 && y_raton < 17) {
		if (usado2 == 1 && _color != BLANCO)
			color_abc(BLANCO);
		else if (usado2 == 0 && _color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (y_raton > 16 && y_raton < 25) {
		if (usado3 == 1 && _color != BLANCO)
			color_abc(BLANCO);
		else if (usado3 == 0 && _color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (_color != VERDE_CLARO)
		color_abc(VERDE_CLARO);

	if (y_raton > 0 && y_raton < 9)
		juego1 = 2;
	else if (y_raton > 8 && y_raton < 17)
		juego2 = 2;
	else if (y_raton > 16 && y_raton < 25)
		juego3 = 2;
	else if (y_raton > 24 && y_raton < 33)
		juego4 = 2;

	print_abc_opc(frase1, 1, 2, juego1);
	print_abc_opc(frase2, 1, 10, juego2);
	print_abc_opc(frase3, 1, 18, juego3);
	print_abc_opc(frase4, 1, 26, juego4);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if ((boton_izq == 1) && (juego1 == 2)) {
		delay(100);
		usado1 = 1;
		talk(frase1, suena1);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta1);
	} else if ((boton_izq == 1) && (juego2 == 2)) {
		delay(100);
		usado2 = 1;
		talk(frase2, suena2);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta2);
	} else if ((boton_izq == 1) && (juego3 == 2)) {
		delay(100);
		usado3 = 1;
		talk(frase3, suena3);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta3);
	} else if ((boton_izq == 1) && (juego4 == 2)) {
		delay(100);
		talk(frase4, suena4);
		rompo_y_salgo = 1;
	}

	if (boton_izq == 1) {
		delay(100);
		color_abc(VERDE_CLARO);
	}

	if (usado1 == 0)
		juego1 = 1;
	else
		juego1 = 3;
	if (usado2 == 0)
		juego2 = 1;
	else
		juego2 = 3;
	if (usado3 == 0)
		juego3 = 1;
	else
		juego3 = 3;

	juego4 = 1;

	if (rompo_y_salgo == 0)
		goto bucle_opc;

	if (num_ejec == 2)
		loadPic(fondo_y_menu);
	else
		loadPic("99.alg");
	decompressPic(dir_hare_fondo, 1);
	if (num_ejec != 5)
		sin_verbo();
}

void DrasculaEngine::print_abc_opc(const char *dicho, int x_pantalla, int y_pantalla, int juego) {
	int pos_texto[6];
	int y_de_signos, y_de_letra, x_de_letra = 0, h, longitud;
	longitud = strlen(dicho);

	for (h = 0; h < longitud; h++) {
		if (juego == 1) {
			y_de_letra = Y_ABC_OPC_1;
			y_de_signos = Y_SIGNOS_OPC_1;
		} else if (juego == 3) {
			y_de_letra = Y_ABC_OPC_3;
			y_de_signos = Y_SIGNOS_OPC_3;
		} else {
			y_de_letra = Y_ABC_OPC_2;
			y_de_signos = Y_SIGNOS_OPC_2;
		}

		int c = toupper(dicho[h]);
		if (c == 'A')
			x_de_letra = X_A_OPC;
		else if (c == 'B')
			x_de_letra = X_B_OPC;
		else if (c == 'C')
			x_de_letra = X_C_OPC;
		else if (c == 'D')
			x_de_letra = X_D_OPC;
		else if (c == 'E')
			x_de_letra = X_E_OPC;
		else if (c == 'F')
			x_de_letra = X_F_OPC;
		else if (c == 'G')
			x_de_letra = X_G_OPC;
		else if (c == 'H')
			x_de_letra = X_H_OPC;
		else if (c == 'I')
			x_de_letra = X_I_OPC;
		else if (c == 'J')
			x_de_letra = X_J_OPC;
		else if (c == 'K')
			x_de_letra = X_K_OPC;
		else if (c == 'L')
			x_de_letra = X_L_OPC;
		else if (c == 'M')
			x_de_letra = X_M_OPC;
		else if (c == 'N')
			x_de_letra = X_N_OPC;
		else if (c == 'O')
			x_de_letra = X_O_OPC;
		else if (c == 'P')
			x_de_letra = X_P_OPC;
		else if (c == 'Q')
			x_de_letra = X_Q_OPC;
		else if (c == 'R')
			x_de_letra = X_R_OPC;
		else if (c == 'S')
			x_de_letra = X_S_OPC;
		else if (c == 'T')
			x_de_letra = X_T_OPC;
		else if (c == 'U')
			x_de_letra = X_U_OPC;
		else if (c == 'V')
			x_de_letra = X_V_OPC;
		else if (c == 'W')
			x_de_letra = X_W_OPC;
		else if (c == 'X')
			x_de_letra = X_X_OPC;
		else if (c == 'Y')
			x_de_letra = X_Y_OPC;
		else if (c == 'Z')
			x_de_letra = X_Z_OPC;
		else if (c == ' ')
			x_de_letra = ESPACIO_OPC;
		else {
			y_de_letra = y_de_signos;
			if (c == '.')
				x_de_letra = X_PUNTO_OPC;
			else if (c == ',')
				x_de_letra = X_COMA_OPC;
			else if (c == '-')
				x_de_letra = X_GUION_OPC;
			else if (c == '?')
				x_de_letra = X_CIERRA_INTERROGACION_OPC;
			else if (c == 0xa8)
				x_de_letra = X_ABRE_INTERROGACION_OPC;
//			else if (c == '\'') // FIXME
//				x_de_letra = ESPACIO; // space for now
			else if (c == '"')
				x_de_letra = X_COMILLAS_OPC;
			else if (c == '!')
				x_de_letra = X_CIERRA_INTERROGACION_OPC;
			else if (c == 0xad)
				x_de_letra = X_ABRE_EXCLAMACION_OPC;
			else if (c == ';')
				x_de_letra = X_PUNTO_Y_COMA_OPC;
			else if (c == '>')
				x_de_letra = X_MAYOR_QUE_OPC;
			else if (c == '<')
				x_de_letra = X_MENOR_QUE_OPC;
			else if (c == '$')
				x_de_letra = X_DOLAR_OPC;
			else if (c == '%')
				x_de_letra = X_POR_CIENTO_OPC;
			else if (c == ':')
				x_de_letra = X_DOS_PUNTOS_OPC;
			else if (c == '&')
				x_de_letra = X_AND_OPC;
			else if (c == '/')
				x_de_letra = X_BARRA_OPC;
			else if (c == '(')
				x_de_letra = X_ABRE_PARENTESIS_OPC;
			else if (c == ')')
				x_de_letra = X_CIERRA_PARENTESIS_OPC;
			else if (c == '*')
				x_de_letra = X_ASTERISCO_OPC;
			else if (c == '+')
				x_de_letra = X_MAS_OPC;
			else if (c == '1')
				x_de_letra = X_N1_OPC;
			else if (c == '2')
				x_de_letra = X_N2_OPC;
			else if (c == '3')
				x_de_letra = X_N3_OPC;
			else if (c == '4')
				x_de_letra = X_N4_OPC;
			else if (c == '5')
				x_de_letra = X_N5_OPC;
			else if (c == '6')
				x_de_letra = X_N6_OPC;
			else if (c == '7')
				x_de_letra = X_N7_OPC;
			else if (c == '8')
				x_de_letra = X_N8_OPC;
			else if (c == '9')
				x_de_letra = X_N9_OPC;
			else if (c == '0')
				x_de_letra = X_N0_OPC;
		}

		pos_texto[0] = x_de_letra;
		pos_texto[1] = y_de_letra;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = ANCHO_LETRAS_OPC;
		pos_texto[5] = ALTO_LETRAS_OPC;

		copyRectClip(pos_texto, dir_hare_fondo, dir_zona_pantalla);

		x_pantalla = x_pantalla + ANCHO_LETRAS_OPC;
	}
}

void DrasculaEngine::responde(int funcion) {
	if (num_ejec == 1) {
		if (funcion == 10)
			talk_borracho(_textb[_lang][1], "B1.als");
		else if (funcion == 11)
			talk_borracho(_textb[_lang][2], "B2.als");
		else if (funcion == 12)
			talk_borracho(_textb[_lang][3], "B3.als");
	} else if (num_ejec == 2) {
		if (funcion == 8)
			animation_8_2();
		else if (funcion == 9)
			animation_9_2();
		else if (funcion == 10)
			animation_10_2();
		else if (funcion == 15)
			animation_15_2();
		else if (funcion == 16)
			animation_16_2();
		else if (funcion == 17)
			animation_17_2();
		else if (funcion == 19)
			animation_19_2();
		else if (funcion == 20)
			animation_20_2();
		else if (funcion == 21)
			animation_21_2();
		else if (funcion == 23)
			animation_23_2();
		else if (funcion == 28)
			animation_28_2();
		else if (funcion == 29)
			animation_29_2();
		else if (funcion == 30)
			animation_30_2();
		else if (funcion == 31)
			animation_31_2();
	} else if (num_ejec == 4) {
		if (funcion == 2)
			animation_2_4();
		else if (funcion == 3)
			animation_3_4();
		else if (funcion == 4)
			animation_4_4();
	} else if (num_ejec == 5) {
		if (funcion == 2)
			animation_2_5();
		else if (funcion == 3)
			animation_3_5();
		else if (funcion == 6)
			animation_6_5();
		else if (funcion == 7)
			animation_7_5();
		else if (funcion == 8)
			animation_8_5();
		else if (funcion == 15)
			animation_15_5();
		else if (funcion == 16)
			animation_16_5();
		else if (funcion == 17)
			animation_17_5();
	} else if (num_ejec == 6) {
		if (funcion == 2)
			animation_2_6();
		else if (funcion == 3)
			animation_3_6();
		else if (funcion == 4)
			animation_4_6();
		else if (funcion == 11)
			animation_11_6();
		else if (funcion == 12)
			animation_12_6();
		else if (funcion == 13)
			animation_13_6();
		else if (funcion == 14)
			animation_14_6();
		else if (funcion == 15)
			animation_15_6();
	}
}

void DrasculaEngine::suma_objeto(int osj) {
	int h, puesto = 0;

	for (h = 1; h < 43; h++) {
		if (objetos_que_tengo[h] == osj)
			puesto = 1;
	}

	if (puesto == 0) {
		for (h = 1; h < 43; h++) {
			if (objetos_que_tengo[h] == 0) {
				objetos_que_tengo[h] = osj;
				puesto = 1;
				break;
			}
		}
	}
}

void DrasculaEngine::fin_sound_corte() {
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
		update_events();
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

int DrasculaEngine::LookForFree() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void DrasculaEngine::actualiza_datos() {
	if (num_ejec == 1) {
		// nothing
	} else if (num_ejec == 2) {
		if (!strcmp(num_room,"2.alg") && flags[40] == 0)
			visible[3] = 0;
		else if (!strcmp(num_room, "3.alg") && flags[3] == 1)
			visible[8] = 0;
		else if (!strcmp(num_room, "6.alg") && flags[1] == 1 && flags[10] == 0) {
			visible[2] = 0;
			visible[4] = 1;
		} else if (!strcmp(num_room, "7.alg") && flags[35] == 1)
			visible[3] = 0;
		else if (!strcmp(num_room, "14.alg") && flags[5] == 1)
			visible[4] = 0;
		else if (!strcmp(num_room, "18.alg") && flags[28] == 1)
			visible[2] = 0;
	} else if (num_ejec == 3) {
		// nothing
	} else if (num_ejec == 4) {
		if (!strcmp(num_room, "23.alg") && flags[0] == 0 && flags[11] == 0)
			visible[2] = 1;
		if (!strcmp(num_room, "23.alg") && flags[0] == 1 && flags[11] == 0)
			visible[2] = 0;
		if (!strcmp(num_room, "21.alg") && flags[10] == 1)
			visible[2] = 0;
		if (!strcmp(num_room, "22.alg") && flags[26] == 1) {
			visible[2] = 0;
			visible[1] = 1;
		}
		if (!strcmp(num_room, "22.alg") && flags[27] == 1)
			visible[3] = 0;
		if (!strcmp(num_room, "26.alg") && flags[21] == 0)
			strcpy(nombre_obj[2], "HUNCHBACKED");
		if (!strcmp(num_room, "26.alg") && flags[18] == 1)
			visible[2] = 0;
		if (!strcmp(num_room, "26.alg") && flags[12] == 1)
			visible[1] = 0;
		if (!strcmp(num_room, "35.alg") && flags[14] == 1)
			visible[2] = 0;
		if (!strcmp(num_room, "35.alg") && flags[17] == 1)
			visible[3] = 1;
		if (!strcmp(num_room, "35.alg") && flags[15] == 1)
			visible[1] = 0;
	} else if (num_ejec == 5) {
		if (!strcmp(num_room,"49.alg") && flags[6] == 1)
			visible[2] = 0;
		if (!strcmp(num_room,"49.alg") && flags[6] == 0)
			visible[1] = 0;
		if (!strcmp(num_room,"49.alg") && flags[6] == 1)
			visible[1] = 1;
		if (!strcmp(num_room,"45.alg") && flags[6] == 1)
			visible[3] = 1;
		if (!strcmp(num_room,"53.alg") && flags[2] == 1)
			visible[3] = 0;
		if (!strcmp(num_room,"54.alg") && flags[13] == 1)
			visible[3] = 0;
		if (!strcmp(num_room,"55.alg") && flags[8] == 1)
			visible[1] = 0;
	} else if (num_ejec == 6) {
		if ((!strcmp(num_room, "58.alg")) && flags[8] == 0)
			espuerta[1] = 0;
		if ((!strcmp(num_room, "58.alg")) && flags[8] == 1)
			espuerta[1] = 1;
		if (!strcmp(num_room, "59.alg"))
			espuerta[1] = 0;
		if (!strcmp(num_room, "60.alg")) {
			sentido_dr = 0;
			x_dr = 155;
			y_dr = 69;
		}
	}
}

void DrasculaEngine::anda_pabajo() {
	direccion_hare = 4;
	sentido_hare = 3;
	paso_x = 0;
}

void DrasculaEngine::anda_parriba() {
	direccion_hare = 0;
	sentido_hare = 2;
	paso_x = 0;
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

	copyRectClip(pos_vb, dir_hare_frente, dir_zona_pantalla);
}

void DrasculaEngine::lleva_vb(int punto_x) {
	if (punto_x < vb_x)
		sentido_vb = 0;
	else
		sentido_vb = 1;

	vb_se_mueve = 1;

	for (;;) {
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if (sentido_vb == 0) {
			vb_x = vb_x - 5;
			if (vb_x <= punto_x)
				break;
		} else {
			vb_x = vb_x + 5;
			if (vb_x >= punto_x)
				break;
		}
		pause(5);
	}

	vb_se_mueve = 0;
}

void DrasculaEngine::hipo_sin_nadie(int contador){
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

comienza:
	contador--;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	if (num_ejec == 3)
		updateScreen(0, 0, 0, y, 320, 200, dir_zona_pantalla);
	else
		updateScreen(0, 1, 0, y, 320, 198, dir_zona_pantalla);

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
	if (contador > 0)
		goto comienza;

	copyBackground(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::abre_puerta(int nflag, int n_puerta) {
	if (flags[nflag] == 0) {
		if (num_ejec == 1 /*|| num_ejec == 4*/) {
			if (nflag != 7) {
				comienza_sound("s3.als");
				flags[nflag] = 1;
			}
		} else {
			comienza_sound("s3.als");
			flags[nflag] = 1;
		}

		if (n_puerta != NO_PUERTA)
			puertas_cerradas(n_puerta);
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fin_sound();
		sin_verbo();
	}
}

void DrasculaEngine::mapa() {
	int l, veo = 0;

	for (l = 0; l < objs_room; l++) {
		if (x_raton > x1[l] && y_raton > y1[l]
				&& x_raton < x2[l] && y_raton < y2[l]
				&& visible[l] == 1) {
			strcpy(texto_nombre, nombre_obj[l]);
			hay_nombre = 1;
			veo = 1;
		}
	}

	if (veo == 0)
		hay_nombre = 0;
}

void DrasculaEngine::grr() {
	int longitud;
	longitud = 30;

	buffer_teclado();

	color_abc(VERDE_OSCURO);

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
	copyBackground(253, 110, 150, 65, 20, 30, dir_dibujo3, dir_zona_pantalla);

	if (con_voces == 0)
		centra_texto(".groaaarrrrgghhh!", 153, 65);

	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

bucless:
	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		sku = NULL;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	updateRoom();
	updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::activa_pendulo() {
	flags[1] = 2;
	hare_se_ve = 0;
	strcpy(num_room, "102.alg");
	loadPic("102.alg");
	decompressPic(dir_dibujo1, MEDIA);
	loadPic("an_p1.alg");
	decompressPic(dir_dibujo3, 1);
	loadPic("an_p2.alg");
	decompressPic(dir_hare_dch, 1);
	loadPic("an_p3.alg");
	decompressPic(dir_hare_frente, 1);

	copyBackground(0, 171, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	conta_ciego_vez = vez();
}

void DrasculaEngine::cierra_puerta(int nflag, int n_puerta) {
	if (flags[nflag] == 1) {
		comienza_sound("s4.als");
		flags[nflag] = 0;
		if (n_puerta != NO_PUERTA)
			puertas_cerradas(n_puerta);
		updateRoom();
		updateScreen(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fin_sound();
		sin_verbo();
	}
}

} // End of namespace Drascula

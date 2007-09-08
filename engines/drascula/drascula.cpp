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

#include "common/stdafx.h"

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "drascula/drascula.h"
#include "drascula/texts.h"

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

DrasculaEngine::DrasculaEngine(OSystem *syst) : Engine(syst) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	const GameSettings *g;

	const char *gameid = ConfMan.get("gameid").c_str();
	for (g = drasculaSettings; g->gameid; ++g)
		if (!scumm_stricmp(g->gameid, gameid))
			_gameId = g->id;

	_rnd = new Common::RandomSource();

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->openCD(cd_num);
}

DrasculaEngine::~DrasculaEngine() {
	salir_al_dos(0);

	delete _rnd;
}

static int x_obj[44] = {0, X_OBJ1, X_OBJ2, X_OBJ3, X_OBJ4, X_OBJ5, X_OBJ6, X_OBJ7, X_OBJ8, X_OBJ9, X_OBJ10,
				X_OBJ11, X_OBJ12, X_OBJ13, X_OBJ14, X_OBJ15, X_OBJ16, X_OBJ17, X_OBJ18, X_OBJ19, X_OBJ20,
				X_OBJ21, X_OBJ22, X_OBJ23, X_OBJ24, X_OBJ25, X_OBJ26, X_OBJ27, X_OBJ28, X_OBJ29, X_OBJ30,
				X_OBJ31, X_OBJ32, X_OBJ33, X_OBJ34, X_OBJ35, X_OBJ36, X_OBJ37, X_OBJ38, X_OBJ39, X_OBJ40,
				X_OBJ41, X_OBJ42, X_OBJ43};
static int y_obj[44] = {0, Y_OBJ1, Y_OBJ2, Y_OBJ3, Y_OBJ4, Y_OBJ5, Y_OBJ6, Y_OBJ7, Y_OBJ8, Y_OBJ9, Y_OBJ10,
				Y_OBJ11, Y_OBJ12, Y_OBJ13, Y_OBJ14, Y_OBJ15, Y_OBJ16, Y_OBJ17, Y_OBJ18, Y_OBJ19, Y_OBJ20,
				Y_OBJ21, Y_OBJ22, Y_OBJ23, Y_OBJ24, Y_OBJ25, Y_OBJ26, Y_OBJ27, Y_OBJ28, Y_OBJ29, Y_OBJ30,
				Y_OBJ31, Y_OBJ32, Y_OBJ33, Y_OBJ34, Y_OBJ35, Y_OBJ36, Y_OBJ37, Y_OBJ38, Y_OBJ39, Y_OBJ40,
				Y_OBJ41, Y_OBJ42, Y_OBJ43};
static int x_pol[44] = {0, 1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
				1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
				247, 83, 165, 1, 206, 42, 124, 83, 1, 247,
				83, 165, 1, 206, 42, 124, 83, 1, 247, 42,
				1, 165, 206};
static int y_pol[44] = {0, 1, 1, 1, 1, 1, 1, 1, 27, 27, 1,
						27, 27, 27, 27, 27, 27, 27, 1, 1, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						27, 1, 1};
static int x_barra[] = {6, 51, 96, 141, 186, 232, 276, 321};
static int x1d_menu[] = {280, 40, 80, 120, 160, 200, 240, 0, 40, 80, 120,
						160, 200, 240, 0, 40, 80, 120, 160, 200, 240, 0,
						40, 80, 120, 160, 200, 240, 0};
static int y1d_menu[] = {0, 0, 0, 0, 0, 0, 0, 25, 25, 25, 25, 25, 25, 25,
						50, 50, 50, 50, 50, 50, 50, 75, 75, 75, 75, 75, 75, 75, 100};
static int frame_x[20] = {43, 87, 130, 173, 216, 259};
static int interf_x[] ={ 1, 65, 129, 193, 1, 65, 129 };
static int interf_y[] ={ 51, 51, 51, 51, 83, 83, 83 };
static char mirar_t[3][88] = {TEXT100, TEXT101, TEXT54};
static char mirar_v[3][14] = {"100.als", "101.als", "54.als"};
static char poder_t[6][88] = {TEXT11, TEXT109, TEXT111, TEXT110, TEXT115, TEXT116};
static char poder_v[6][14] = {"11.als", "109.als", "111.als", "110.als", "115.als", "116.als"};
 
int DrasculaEngine::init() {
	// Detect game
	if (!initGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	return 0;
}

int DrasculaEngine::go() {
	num_ejec = 1;

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
		alto_habla = ALTO_HABLA_HARE; ancho_habla = ANCHO_HABLA_HARE;
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
		cual_ejec = 0; hay_que_load = 0;
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

		asigna_memoria();
		carga_info();

		if (num_ejec != 6) {
			lee_dibujos("95.alg");
			descomprime_dibujo(dir_mesa, 1);
		}

		if (num_ejec == 6)
			dir_pendulo = dir_dibujo3;

		lee_dibujos("96.alg");
		descomprime_dibujo(dir_hare_frente, COMPLETA);
		if (num_ejec == 1) {
			con_voces = 0;
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
			lee_dibujos("97.alg");
			descomprime_dibujo(dir_hare_dch, 1);
		} else if (num_ejec == 2) {
			con_voces = 1;
			lee_dibujos("pts.alg");
			descomprime_dibujo(dir_dibujo2, 1);
		} else if (num_ejec == 3) {
			con_voces = 1;
			lee_dibujos("aux13.alg");
			descomprime_dibujo(dir_dibujo1, COMPLETA);
			lee_dibujos("97.alg");
			descomprime_dibujo(dir_hare_dch, 1);
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
		} else if (num_ejec == 4) {
			con_voces = 1;
			if (hay_que_load == 0)
				animacion_rayo();
			lee_dibujos("96.alg");
			descomprime_dibujo(dir_hare_frente, 1);
			borra_pantalla();
			lee_dibujos("97.alg");
			descomprime_dibujo(dir_hare_dch, 1);
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
		} else if (num_ejec == 5) {
			con_voces = 0;
			lee_dibujos("97.alg");
			descomprime_dibujo(dir_hare_dch, 1);
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
		} else if (num_ejec == 6) {
			con_voces = 1;
			x_igor = 105, y_igor = 85, sentido_igor = 1;
			x_dr = 62, y_dr = 99, sentido_dr = 1;
			frame_pen = 0;
			flag_tv = 0;

			dir_pendulo = dir_dibujo3;

			lee_dibujos("96.alg");
			descomprime_dibujo(dir_hare_frente, COMPLETA);
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo,1);
			lee_dibujos("97.alg");
			descomprime_dibujo(dir_hare_dch,1);

			lee_dibujos("95.alg");
			descomprime_dibujo(dir_mesa, 1);
 		}
		strcpy(nombre_icono[1], "look");
		strcpy(nombre_icono[2], "take");
		strcpy(nombre_icono[3], "open");
		strcpy(nombre_icono[4], "close");
		strcpy(nombre_icono[5], "talk");
		strcpy(nombre_icono[6], "push");

		paleta_hare();
		if (escoba()) {
			salir_al_dos(0);
			break;
		}
		salir_al_dos(0);
	}
	return 0;
}

void DrasculaEngine::salir_al_dos(int r) {
	if (hay_sb == 1)
		ctvd_end();
	borra_pantalla();
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

void DrasculaEngine::carga_info() {
	hay_sb = 1;
	con_voces = 0;
	hay_que_load = 0;
}

void DrasculaEngine::lee_dibujos(const char *NamePcc) {
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

void DrasculaEngine::descomprime_dibujo(byte *dir_escritura, int plt) {
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

void DrasculaEngine::DIBUJA_FONDO(int xorg, int yorg, int xdes, int ydes, int Ancho,
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

void DrasculaEngine::DIBUJA_BLOQUE(int xorg, int yorg, int xdes, int ydes, int Ancho,
								   int Alto, byte *Origen, byte *Destino) {
	int y, x;

	Destino += xdes + ydes * 320;
	Origen += xorg + yorg * 320;

	for (y = 0; y < Alto; y++)
		for (x = 0; x < Ancho; x++)
			if (Origen[x + y * 320] != 255)
				Destino[x + y * 320] = Origen[x + y * 320];
}

void DrasculaEngine::DIBUJA_BLOQUE_CUT(int *Array, byte *Origen, byte *Destino) {
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

void DrasculaEngine::VUELCA_PANTALLA(int xorg, int yorg, int xdes, int ydes, int Ancho, int Alto, byte *Buffer) {
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
	int soc, l, n;

	dir_texto = dir_mesa;

	musica_antes = -1;

	soc = 0;
	for (l = 0; l < 6; l++) {
		soc = soc + ANCHO_PERSONAJE;
		frame_x[l] = soc;
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
			animacion_1_1();

		sin_verbo();
		lee_dibujos("2aux62.alg");
		descomprime_dibujo(dir_dibujo2, 1);
		sentido_hare = 1;
		obj_saliendo = 104;
		if (hay_que_load != 0)
			para_cargar(nom_partida);
		else {
			carga_escoba_1("62.ald");
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
			carga_escoba_2("14.ald");
		else
			para_cargar(nom_partida);
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
			carga_escoba_1("20.ald");
		else
			para_cargar(nom_partida);
	} else if (num_ejec == 3) {
		suma_objeto(28);
		suma_objeto(9);
		suma_objeto(20);
		suma_objeto(22);
        buffer_teclado();
		obj_saliendo = 100;
		if (hay_que_load == 0) {
			carga_escoba_1("21.ald");
			sentido_hare = 0;
			hare_x = 235;
			hare_y = 164;
		} else
			para_cargar(nom_partida);
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
			carga_escoba_1("45.ald");
		} else
			para_cargar(nom_partida);
	} else if (num_ejec == 6) {
		suma_objeto(28);
		suma_objeto(9);

		buffer_teclado();
		sentido_hare = 1;
		obj_saliendo = 104;
        if (hay_que_load == 0)
			carga_escoba_1("58.ald");
        else
			para_cargar(nom_partida);
        if (hay_que_load == 0)
			animacion_1_6();
        else {
			lee_dibujos("auxdr.alg");
			descomprime_dibujo(dir_dibujo2, 1);
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
			animacion_1_2();
		else if ((!strcmp(num_room, "14.alg")) && (hare_x == 214) && (hare_y + alto_hare == 121))
			lleva_al_hare(190, 130);
		else if ((!strcmp(num_room, "14.alg")) && (hare_x == 246) && (hare_y + alto_hare == 112))
			lleva_al_hare(190, 130);
	}

	mueve_cursor();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (music_status() == 0 && musica_room != 0)
		playmusic(musica_room);

	MirarRaton();

	if (menu_scr == 0 && lleva_objeto == 1)
		comprueba_objetos();

	if (boton_dch == 1 && menu_scr == 1) {
		if (num_ejec == 1)
			lee_dibujos("99.alg");
		else if (num_ejec == 2)
			lee_dibujos(fondo_y_menu);
		descomprime_dibujo(dir_hare_fondo, 1);
		setvgapalette256((byte *)&palJuego);
		menu_scr = 0;
		espera_soltar();
		cont_sv = 0;
	}
	if (boton_dch == 1 && menu_scr == 0) {
		hare_se_mueve = 0;
		if (sentido_hare == 2)
			sentido_hare = 1;
		if (num_ejec == 4)
			lee_dibujos("icons2.alg");
		else if (num_ejec == 5)
			lee_dibujos("icons3.alg");
		else if (num_ejec == 6)
			lee_dibujos("icons3.alg");
		else
			lee_dibujos("iconsp.alg");
		descomprime_dibujo(dir_hare_fondo, 1);
		menu_scr = 1;
		espera_soltar();
		sin_verbo();
		cont_sv = 0;
	}

	if (boton_izq == 1 && menu_bar == 1) {
		elige_en_barra();
		cont_sv = 0;
	} else if (boton_izq == 1 && lleva_objeto == 0) {
		if (comprueba1())
			return true;
		cont_sv = 0;
	} else if (boton_izq == 1 && lleva_objeto == 1) {
		comprueba2();
		cont_sv = 0;
	}

	if (y_raton < 24 && menu_scr == 0)
		menu_bar = 1;
	else
		menu_bar = 0;

	Common::KeyCode key = getscan();
	if (key == Common::KEYCODE_F1 && menu_scr == 0) {
		elige_verbo(1);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F2 && menu_scr == 0) {
		elige_verbo(2);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F3 && menu_scr == 0) {
		elige_verbo(3);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F4 && menu_scr == 0) {
		elige_verbo(4);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F5 && menu_scr == 0) {
		elige_verbo(5);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F6 && menu_scr == 0) {
		elige_verbo(6);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F9) {
		mesa();
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F10) {
		saves();
		cont_sv = 0;
	} else if (key == Common::KEYCODE_F8) {
		sin_verbo();
		cont_sv = 0;
	} else if (key == Common::KEYCODE_v) {
		con_voces = 1;
		print_abc(SYS2, 96, 86);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1410);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_t) {
		con_voces = 0;
		print_abc(SYS3, 94, 86);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1460);
		cont_sv = 0;
	} else if (key == Common::KEYCODE_DELETE) {
		if (num_ejec == 4)
			carga_partida("gsave00");
		else
			confirma_go();
		cont_sv = 0;
	} else if (key == Common::KEYCODE_ESCAPE) {
		confirma_salir();
		cont_sv = 0;
	} else if (key == Common::KEYCODE_0 && !strcmp(num_room, "61.alg")) {
		 lee_dibujos("alcbar.alg");
		 descomprime_dibujo(dir_dibujo1, 255);
	} else if (cont_sv == 1500) {
		salva_pantallas();
		cont_sv = 0;
	} else
		cont_sv++;
	goto bucles;
}

void DrasculaEngine::agarra_objeto(int objeto) {
	if (num_ejec == 6)
		lee_dibujos("iconsp.alg");
	else if (num_ejec == 4 || num_ejec == 5)
		lee_dibujos("icons2.alg");
	else
		lee_dibujos("icons.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	elige_objeto(objeto);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::elige_objeto(int objeto) {
	if (num_ejec == 5) {
		if (lleva_objeto == 1 && menu_scr == 0  && objeto_que_lleva != 16)
			suma_objeto(objeto_que_lleva);
	} else {
		if (lleva_objeto == 1 && menu_scr == 0)
			suma_objeto(objeto_que_lleva);
	}
	DIBUJA_FONDO(x1d_menu[objeto], y1d_menu[objeto], 0, 0, ANCHOBJ,ALTOBJ, dir_hare_fondo, dir_dibujo3);
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

void DrasculaEngine::animacion_1_1() {
	int l, l2, p;
	int pos_pixel[6];

	while (term_int == 0) {
		playmusic(29);
		fliplay("logoddm.bin", 9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(600);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		delay(340);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playmusic(26);
		delay(500);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fliplay("logoalc.bin", 8);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("cielo.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		Negro();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(2);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(900);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_abc(ROJO);
		centra_texto("Transilvanya, 1993 d.c.", 160, 100);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1000);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(1200);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		fliplay("scrollb.bin", 9);

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		comienza_sound("s5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("scr2.bin", 17);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fin_sound_corte();
		anima("scr3.bin", 17);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("cielo2.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		lee_dibujos("96.alg");
		descomprime_dibujo(dir_hare_frente, COMPLETA);
		lee_dibujos("103.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		lee_dibujos("104.alg");
		descomprime_dibujo(dir_dibujo3, 1);
		lee_dibujos("aux104.alg");
		descomprime_dibujo(dir_dibujo2, 1);

		playmusic(4);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		delay(400);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
				DIBUJA_FONDO(interf_x[l], interf_y[l], 156, 45, 63, 31, dir_dibujo2, dir_zona_pantalla);
				VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				if (getscan() == Common::KEYCODE_ESCAPE) {
					term_int = 1;
					break;
				}
				pausa(3);
			}
			if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
				break;

		l2 = 0; p = 0;
		pos_pixel[3] = 45;
		pos_pixel[4] = 63;
		pos_pixel[5] = 31;

		for (l = 0; l < 180; l++) {
			DIBUJA_FONDO(0, 0, 320 - l, 0, l, 200, dir_dibujo3, dir_zona_pantalla);
			DIBUJA_FONDO(l, 0, 0, 0, 320 - l, 200, dir_dibujo1, dir_zona_pantalla);

			pos_pixel[0] = interf_x[l2];
			pos_pixel[1] = interf_y[l2];
			pos_pixel[2] = 156 - l;

			DIBUJA_BLOQUE_CUT(pos_pixel, dir_dibujo2, dir_zona_pantalla);
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			p++;
			if (p == 6) {
				p = 0;
				l2++;
			}
			if (l2 == 7)
				l2 = 0;
			if (getscan() == Common::KEYCODE_ESCAPE) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);

		habla_dr_grande(TEXTD1, "D1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		borra_pantalla();

		lee_dibujos("100.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		lee_dibujos("auxigor.alg");
		descomprime_dibujo(dir_hare_frente, 1);
		lee_dibujos("auxdr.alg");
		descomprime_dibujo(dir_hare_fondo, 1);
		sentido_dr = 0;
		x_dr = 129;
		y_dr = 95;
		sentido_igor = 1;
		x_igor = 66;
		y_igor = 97;

		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_igor_dch(TEXTI8, "I8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD2, "d2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD3, "d3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		color_solo = ROJO;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(10);
		habla_solo(TEXTD4,"d4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD5, "d5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("plan2.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		habla_solo(TEXTD6, "d6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXTD7, "d7.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD8, "d8.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("100.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		MusicFadeout();
		stopmusic();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI9, "I9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD9, "d9.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI10, "I10.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		playmusic(11);
		habla_dr_izq(TEXTD10, "d10.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("rayo1.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		comienza_sound("s5.als");
		anima("rayo2.bin", 15);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel2.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		fin_sound_corte();
		borra_pantalla();
		Negro();
		playmusic(23);
		FundeDelNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 1;
		habla_igor_dch(TEXTI1, "I1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_dch(TEXTD11, "d11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 0;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD12, "d12.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 1;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_igor_dch(TEXTI2, "I2.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(13);
		habla_dr_dch(TEXTD13,"d13.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_dr = 3;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(1);
		sentido_dr = 0;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD14, "d14.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI3, "I3.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD15, "d15.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI4, "I4.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_dr_izq(TEXTD16, "d16.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_dch(TEXTI5, "I5.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_igor = 3;
		habla_dr_izq(TEXTD17, "d17.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(18);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_igor_frente(TEXTI6, "I6.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		FundeAlNegro(0);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		playmusic(2);
		pausa(5);
		fliplay("intro.bin", 12);
		term_int = 1;
	}
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

bool DrasculaEngine::animacion_2_1() {
	int l;

	lleva_al_hare(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	for (;;) {
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		anima("ag.bin", 14);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lee_dibujos("an11y13.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		habla_tabernero(TEXTT22, "T22.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lee_dibujos("97.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		pausa(4);
		comienza_sound("s1.als");
		hipo(18);
		fin_sound();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		borra_pantalla();
		stopmusic();
		corta_musica = 1;
		memset(dir_zona_pantalla, 0, 64000);
		color_solo = BLANCO;
		pausa(80);

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXTBJ1, "BJ1.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();
		lee_dibujos("bj.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		Negro();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = AMARILLO;
		habla_solo(TEXT214, "214.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		borra_pantalla();

		lee_dibujos("16.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("auxbj.alg");
		descomprime_dibujo(dir_dibujo3, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		strcpy(num_room, "16.alg");

		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		for (l = 0; l < 200; l++)
			factor_red[l] = 99;
		x_bj = 170;
		y_bj = 90;
		sentido_bj = 0;
		hare_x = 91;
		hare_y = 95;
		sentido_hare = 1;
		hare_se_ve = 1;
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lee_dibujos("97g.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		anima("lev.bin", 15);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;

		lleva_al_hare(100 + ancho_hare / 2, 99 + alto_hare);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		hare_x = 100;
		hare_y = 95;

		habla_bj(TEXTBJ2, "BJ2.als");
		hablar(TEXT215, "215.als");
		habla_bj(TEXTBJ3, "BJ3.als");
		hablar(TEXT216, "216.als");
		habla_bj(TEXTBJ4, "BJ4.als");
		habla_bj(TEXTBJ5, "BJ5.als");
		habla_bj(TEXTBJ6, "BJ6.als");
		hablar(TEXT217, "217.als");
		habla_bj(TEXTBJ7, "BJ7.als");
		hablar(TEXT218, "218.als");
		habla_bj(TEXTBJ8, "BJ8.als");
		hablar(TEXT219, "219.als");
		habla_bj(TEXTBJ9, "BJ9.als");
		hablar(TEXT220, "220.als");
		hablar(TEXT221, "221.als");
		habla_bj(TEXTBJ10, "BJ10.als");
		hablar(TEXT222, "222.als");
		anima("gaf.bin", 15);
		anima("bjb.bin", 14);
		playmusic(9);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lee_dibujos("97.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(120);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_solo(TEXT223, "223.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		color_solo = BLANCO;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(110);
		habla_solo(TEXTBJ11, "BJ11.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(118);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		lleva_al_hare(132, 97 + alto_hare);
		pausa(60);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		hablar(TEXT224, "224.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		habla_bj(TEXTBJ12, "BJ12.als");
		lleva_al_hare(157, 98 + alto_hare);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		anima("bes.bin", 16);
		playmusic(11);
		anima("rap.bin", 16);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 3;
		strcpy(num_room, "no_bj.alg");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		pausa(8);
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT225, "225.als");
		pausa(76);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		sentido_hare = 1;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT226, "226.als");
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(30);
		if ((term_int == 1) || (getscan() == Common::KEYCODE_ESCAPE))
			break;
		hablar(TEXT227,"227.als");
		FundeAlNegro(0);
		break;
	}

	num_ejec = 2;
	return true;
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
	DIBUJA_FONDO(0, c, 0, 0, ANCHOBJ,ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 0;
	hay_nombre = 0;
}

void DrasculaEngine::para_cargar(char nom_game[]) {
	musica_antes = musica_room;
	menu_scr = 0;
	if (num_ejec == 2 || num_ejec == 3 || num_ejec == 4)
		borra_pantalla();
	carga_partida(nom_game);
	if (num_ejec == 1 || num_ejec == 4) {
		carga_escoba_1(datos_actuales);
	} else if (num_ejec == 2 || num_ejec == 3) {
		ald->close();
		carga_escoba_2(datos_actuales);
	}
	sin_verbo();
}

static char *getLine(Common::File *fp, char *buf, int len) {
	int c;
	char *b;

	for (;;) {
		b = buf;
		while (!fp->eos()) {
			c = fp->readByte() ^ 0xff;
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

void DrasculaEngine::carga_escoba_1(const char *nom_fich) {
	int l, obj_salir;
	float chiquez, pequegnez = 0;
	char para_codificar[13];
	char buffer[256];

	hay_nombre = 0;

	strcpy(para_codificar, nom_fich);
	canal_p(para_codificar);
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
	sscanf(buffer, "%s", pantalla_disco);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &nivel_osc);

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

	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &lejos);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &cerca);
	delete ald;

	canal_p(para_codificar);

	obj_salir = -1;
	for (l = 0; l < objs_room; l++) {
		if (num_obj[l] == obj_saliendo)
			obj_salir = l;
	}

	lee_dibujos(pantalla_disco);
	descomprime_dibujo(dir_dibujo3, 1);

	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);

	DIBUJA_FONDO(0, 171, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	color_hare();
	if (nivel_osc != 0)
		funde_hare(nivel_osc);
	paleta_hare_claro();
	color_hare();
	funde_hare(nivel_osc + 2);
	paleta_hare_oscuro();

	hare_claro();
	cambio_de_color = -1;

	for (l = 0; l <= suelo_y1; l++)
		factor_red[l] = lejos;
	for (l = suelo_y1; l <= 201; l++)
		factor_red[l] = cerca;

	chiquez = (float)(cerca-lejos) / (float)(suelo_y2 - suelo_y1);
	for (l = suelo_y1; l <= suelo_y2; l++) {
		factor_red[l] = (int)(lejos + pequegnez);
		pequegnez = pequegnez + chiquez;
	}

	if (!strcmp(num_room, "24.alg")) {
		for (l = suelo_y1 - 1; l > 74; l--) {
			factor_red[l] = (int)(lejos - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (!strcmp(num_room, "54.alg")) {
		for (l = suelo_y1 - 1; l > 84; l--) {
			factor_red[l] = (int)(lejos - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (hare_x == -1) {
		assert(obj_salir != -1);
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
	if (num_ejec == 5)
		hare_se_ve = 1;

	actualiza_datos();

	if (num_ejec <= 2)
		espuerta[7] = 0;

	if (!strcmp(num_room, "26.alg"))
		conta_ciego_vez = (int)vez();

	if (musica_antes != musica_room)
		playmusic(musica_room);

	if (!strcmp(num_room, "24.alg") && flags[29] == 1)
		animacion_7_4();

	if (!strcmp(num_room, "45.alg"))
		hare_se_ve = 0;
	if (!strcmp(num_room, "49.alg") && flags[7] == 0)
		animacion_4_5();
 
	refresca_pantalla();
}

void DrasculaEngine::carga_escoba_2(const char *nom_fich) {
	int soc, l, martin, obj_salir;
	char pant1[20], pant2[20], pant3[20], pant4[20];
	char para_codificar[20];
	char buffer[256];

	hay_nombre = 0;

	strcpy(para_codificar, nom_fich);
	canal_p(para_codificar);
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
	sscanf(buffer, "%s", pantalla_disco);
	getLine(ald, buffer, size);
	sscanf(buffer, "%d", &nivel_osc);

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

	lee_dibujos(pant2);
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos(pant1);
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos(pant4);
	descomprime_dibujo(dir_hare_fondo, 1);

	strcpy(fondo_y_menu, pant4);

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

	delete ald;

	canal_p(para_codificar);

	if (martin == 0) {
		paso_x = PASO_HARE_X;
		paso_y = PASO_HARE_Y;
		alto_hare = ALTO_PERSONAJE;
		ancho_hare = ANCHO_PERSONAJE;
		alto_pies = PIES_HARE;
		lee_dibujos("97.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		lee_dibujos("96.alg");
		descomprime_dibujo(dir_hare_frente, 1);
		lee_dibujos("99.alg");
		descomprime_dibujo(dir_hare_fondo, 1);

		strcpy(fondo_y_menu,"99.alg");
	}

	obj_salir = -1;
	for (l = 0; l < objs_room; l++) {
		if (num_obj[l] == obj_saliendo)
			obj_salir = l;
	}

	if (hare_x == -1) {
		assert(obj_salir != -1);
		hare_x = x_alakeva[obj_salir];
		hare_y = y_alakeva[obj_salir] - alto_hare;
	}
	hare_se_mueve = 0;

	lee_dibujos(pantalla_disco);
	descomprime_dibujo(dir_dibujo3, 1);

	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);

	DIBUJA_FONDO(0, 171, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	color_hare();
	if (nivel_osc != 0)
		funde_hare(nivel_osc);
	paleta_hare_claro();
	color_hare();
	funde_hare(nivel_osc + 2);
	paleta_hare_oscuro();

	hare_claro();
	cambio_de_color = -1;

	color_abc(VERDE_CLARO);

	soc = 0;
	for (l = 0; l < 6; l++) {
		soc = soc + ancho_hare;
		frame_x[l] = soc;
	}

	actualiza_datos();

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

	if ((!strcmp(num_room, "9.alg")) || (strcmp(num_room, "2.alg"))
			|| (!strcmp(num_room, "14.alg")) || (!strcmp(num_room, "18.alg")))
		conta_ciego_vez = (int)vez();

	refresca_pantalla();
}

void DrasculaEngine::borra_pantalla() {
	memset(VGA, 0, 64000);
	_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void DrasculaEngine::lleva_al_hare(int punto_x, int punto_y) {
	if (num_ejec == 5 || num_ejec == 6) {
		if (hare_se_ve==0) {
			hare_x = sitio_x;
			hare_y = sitio_y;
			goto fin;
		}
	}
	sitio_x = punto_x;
	sitio_y = punto_y;
	empieza_andar();

	for (;;) {
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if (hare_se_mueve == 0)
			break;
	}

	if (anda_a_objeto == 1) {
		anda_a_objeto = 0;
		sentido_hare = sentido_final;
	}
fin:
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::mueve_cursor() {
	int pos_cursor[8];

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();
	pon_hare();
	actualiza_refresco();

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
	DIBUJA_BLOQUE_CUT(pos_cursor, dir_dibujo3, dir_zona_pantalla);
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

	if (num_ejec == 1)
		if (x_raton > hare_x + 2 && y_raton > hare_y + 2
				&& x_raton < hare_x + ancho_hare - 2 && y_raton < hare_y + alto_hare - 2 && veo == 0) {
			strcpy(texto_nombre, "hacker");
			hay_nombre = 1;
			veo = 1;
		}
	else if (num_ejec == 2)
		if (x_raton > hare_x + 2 && y_raton > hare_y + 2
				&& x_raton < hare_x + ancho_hare - 2 && y_raton < hare_y + alto_hare - 2) {
			strcpy(texto_nombre, "hacker");
			hay_nombre = 1;
			veo = 1;
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

void DrasculaEngine::comprueba2() {
	int l;

	if (menu_scr == 1)
		coge_objeto();
	else {
		if (!strcmp(texto_nombre, "hacker") && hay_nombre == 1)
			banderas(50);
		else
			for (l = 0; l < objs_room; l++) {
				if (x_raton > x1[l] && y_raton > y1[l]
						&& x_raton < x2[l] && y_raton < y2[l] && visible[l] == 1) {
					sentido_final = sentidobj[l];
					anda_a_objeto = 1;
					lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
					banderas(num_obj[l]);
					if (num_ejec == 4)
						break;
				}
			}
	}
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

	DIBUJA_FONDO(ANCHOBJ * verbo, c, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 1;
	objeto_que_lleva = verbo;
}

void DrasculaEngine::mesa() {
	int nivel_master, nivel_voc, nivel_cd;

	DIBUJA_BLOQUE(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);
	VUELCA_PANTALLA(73, 63, 73, 63, 177, 97, dir_zona_pantalla);

	for (;;) {
		nivel_master = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4);
		nivel_voc = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4);
		nivel_cd = 72 + 61 - ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4);

		refresca_pantalla();

		DIBUJA_BLOQUE(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);

		DIBUJA_FONDO(183, 56, 82, nivel_master, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);
		DIBUJA_FONDO(183, 56, 138, nivel_voc, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);
		DIBUJA_FONDO(183, 56, 194, nivel_cd, 39, 2 + ((_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16) * 4), dir_mesa, dir_zona_pantalla);

		cursor_mesa();

		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		MirarRaton();

		if (boton_dch == 1)
			break;
		if (boton_izq == 1) {
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

void DrasculaEngine::saves() {
	char nombres[10][23];
	char fichero[13];
	int n, n2, num_sav = 0, y = 27;
	Common::InSaveFile *sav;

	borra_pantalla();

	if (!(sav = _saveFileMan->openForLoading("saves.epa"))) {
		error("Can't open saves.epa file.");
	}
	for (n = 0; n < NUM_SAVES; n++)
		sav->read(nombres[n], 23);
	delete sav;

	lee_dibujos("savescr.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);

	color_abc(VERDE_CLARO);

	for (;;) {
		y = 27;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(nombres[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		cursor_mesa();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		y = 27;

		MirarRaton();

		if (boton_izq == 1) {
			for (n = 0; n < NUM_SAVES; n++) {
				if (x_raton > 115 && y_raton > y + (9 * n) && x_raton < 115 + 175 && y_raton < y + 10 + (9 * n)) {
					strcpy(select, nombres[n]);
					
				// FIXME: The indention is wrong and misleading here!!! Or maybe there's simply a
				// closing brace missing here??? See below for a second FIXME of a similar kind...

				if (strcmp(select, "*"))
					hay_seleccion = 1;
				else {
					introduce_nombre();
					strcpy(nombres[n], select);
					if (hay_seleccion == 1) {
						// FIXME: Just use:
						//sprintf(fichero, "gsave%02d", n+1);
						if (n == 0)
							strcpy(fichero, "gsave01");
						if (n == 1)
							strcpy(fichero, "gsave02");
						if (n == 2)
							strcpy(fichero, "gsave03");
						if (n == 3)
							strcpy(fichero, "gsave04");
						if (n == 4)
							strcpy(fichero, "gsave05");
						if (n == 5)
							strcpy(fichero, "gsave06");
						if (n == 6)
							strcpy(fichero, "gsave07");
						if (n == 7)
							strcpy(fichero, "gsave08");
						if (n == 8)
							strcpy(fichero, "gsave09");
						if (n == 9)
							strcpy(fichero, "gsave10");
						para_grabar(fichero);
						Common::OutSaveFile *tsav;
						if (!(tsav = _saveFileMan->openForSaving("saves.epa"))) {
							error("Can't open saves.epa file.");
						}
						for (n = 0; n < NUM_SAVES; n++)
							tsav->write(nombres[n], 23);
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
					// FIXME: Just use:
					//sprintf(fichero, "gsave%02d", n+1);
					if (n == 0)
						strcpy(fichero, "gsave01");
					if (n == 1)
						strcpy(fichero, "gsave02");
					if (n == 2)
						strcpy(fichero, "gsave03");
					if (n == 3)
						strcpy(fichero, "gsave04");
					if (n == 4)
						strcpy(fichero, "gsave05");
					if (n == 5)
						strcpy(fichero, "gsave06");
					if (n == 6)
						strcpy(fichero, "gsave07");
					if (n == 7)
						strcpy(fichero, "gsave08");
					if (n == 8)
						strcpy(fichero, "gsave09");
					if (n == 9)
						strcpy(fichero, "gsave10");}	// FIXME: EVIL wrong place for closing brace!
					// In particular: is the assignment below maybe supposed to be inside the "if"
					// statement that was just closed?
					// Also note that the indention is wrong here, which is not immediately visible
					// due to the other indention mistake above. But is the indention wrong, or is
					// the brace placement wrong???
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
				para_cargar(fichero);
				break;
			} else if (x_raton > 208 && y_raton > 123 && x_raton < 282 && y_raton < 149 && hay_seleccion == 1) {
				para_grabar(fichero);
				Common::OutSaveFile *tsav;
				if (!(tsav = _saveFileMan->openForSaving("saves.epa"))) {
					error("Can't open saves.epa file.");
				}
				for (n = 0; n < NUM_SAVES; n++)
					tsav->write(nombres[n], 23);
				tsav->finalize();
				delete tsav;
			} else if (x_raton > 168 && y_raton > 154 && x_raton < 242 && y_raton < 180)
				break;
			else if (hay_seleccion == 0) {
				print_abc("elige una partida", 117, 15);
			}
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			delay(400);
		}
		y = 26;
	}

	borra_pantalla();
	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);
	buffer_teclado();
	hay_seleccion = 0;
}

void DrasculaEngine::print_abc(const char *dicho, int x_pantalla, int y_pantalla) {
	int pos_texto[8];
	int y_de_letra = 0, x_de_letra = 0, h, longitud;
	longitud = strlen(dicho);

	for (h = 0; h < longitud; h++) {
		y_de_letra = Y_ABC;
		char c = toupper(dicho[h]);
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
//TODO	else if (c == '¥')
//			x_de_letra = X_GN;
		else if (c == '¤')
			x_de_letra = X_GN;
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
		else if (/*c == 0xa7 ||*/ c == ' ')
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
//TODO			else if (c == '¨')
//				x_de_letra = X_ABRE_INTERROGACION;
			else if (c == '"')
				x_de_letra = X_COMILLAS;
			else if (c == '!')
				x_de_letra = X_CIERRA_EXCLAMACION;
//TODO			else if (c == '­')
//				x_de_letra = X_ABRE_EXCLAMACION;
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
			else
				y_de_letra = Y_ACENTOS;
/*
			if (c == ' ')
				x_de_letra = X_A;
			else if (c == '‚')
				x_de_letra = X_B;
			else if (c == '¡')
				x_de_letra = X_C;
			else if (c == '¢')
				x_de_letra = X_D;
			else if (c == '£')
				x_de_letra = X_E;
			else if (c == '…')
				x_de_letra = X_F;
			else if (c == 'Š')
				x_de_letra = X_G;
			else if (c == '')
				x_de_letra = X_H;
			else if (c == '•')
				x_de_letra = X_I;
			else if (c == '—')
				x_de_letra = X_J;
			else if (c == 'ƒ')
				x_de_letra = X_K;
			else if (c == 'ˆ')
				x_de_letra = X_L;
			else if (c == 'Œ')
				x_de_letra = X_M;
			else if (c == '“')
				x_de_letra = X_N;
			else if (c == '–')
				x_de_letra = X_GN;
			else if (c == '\'')
				x_de_letra = X_O;
			else if (c == '€')
				x_de_letra = X_P;
			else if (c == '‡')
				x_de_letra = X_P;
			else if (c == 'µ')
				x_de_letra = X_A;
			else if (c == '')
				x_de_letra = X_B;
			else if (c == 'Ö')
				x_de_letra = X_C;
			else if (c == 'à')
				x_de_letra = X_D;
			else if (c == 'é')
				x_de_letra = X_E;
			else if (c == '·')
				x_de_letra = X_F;
			else if (c == 'Ô')
				x_de_letra = X_G;
			else if (c == 'Þ')
				x_de_letra = X_H;
			else if (c == 'ã')
				x_de_letra = X_I;
			else if (c == 'ë')
				x_de_letra = X_J;
			else if (c == '¶')
				x_de_letra = X_K;
			else if (c == 'Ò')
				x_de_letra = X_L;
			else if (c == '×')
				x_de_letra = X_M;
			else if (c == 'â')
				x_de_letra = X_N;
			else if (c == 'ê')
				x_de_letra = X_GN;
*/		}

		pos_texto[0] = x_de_letra;
		pos_texto[1] = y_de_letra;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = ANCHO_LETRAS;
		pos_texto[5] = ALTO_LETRAS;

		DIBUJA_BLOQUE_CUT(pos_texto, dir_texto, dir_zona_pantalla);

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

void DrasculaEngine::confirma_go() {
	byte key;

	color_abc(ROJO);
	refresca_pantalla();
	centra_texto(SYS0, 160, 87);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (;;) {
		key = getscan();
		if (key != 0)
			break;
	}

	if (key == Common::KEYCODE_DELETE) {
		stopmusic();
		carga_partida("gsave00");
	}
}

void DrasculaEngine::confirma_salir() {
	byte key;

	color_abc(ROJO);
	refresca_pantalla();
	centra_texto(SYS1, 160, 87);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (;;) {
		key = getscan();
		if (key != 0)
			break;
	}

	if (key == Common::KEYCODE_ESCAPE) {
		stopmusic();
		salir_al_dos(0);
	}
}

void DrasculaEngine::salva_pantallas() {
	int xr, yr;

	borra_pantalla();

	lee_dibujos("sv.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
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
	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);
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
		pausa(VelocidadDeFundido);

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

void DrasculaEngine::anima(const char *animacion, int FPS) {
	Common::File FileIn;
	unsigned j;
	int NFrames = 1;
	int cnt = 2;

	AuxBuffLast = (byte *)malloc(65000);
	AuxBuffDes = (byte *)malloc(65000);

	FileIn.open(animacion);

	if (!FileIn.isOpen()) {
		error("No encuentro un fichero de animacion.");
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
		pausa(VelocidadDeFundido);

		setvgapalette256((byte *)&palFundido);
	}
}

void DrasculaEngine::pausa(int cuanto) {
	_system->delayMillis(cuanto * 30); // was originaly 2
}

void DrasculaEngine::habla_dr_grande(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	int x_habla[4] = {47, 93, 139, 185};
	int cara;
	int l = 0;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(3);
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_FONDO(interf_x[l] + 24, interf_y[l], 0, 45, 39, 31, dir_dibujo2, dir_zona_pantalla);
	DIBUJA_FONDO(x_habla[cara], 1, 171, 68, 45, 48, dir_dibujo2, dir_zona_pantalla);
	l++;
	if (l == 7)
		l = 0;

	if (con_voces == 0)
		centra_texto(dicho, 191, 69);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;

	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}
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

	DIBUJA_BLOQUE_CUT(pos_igor, dir_hare_frente, dir_zona_pantalla);
}

void DrasculaEngine::pon_dr() {
	int pos_dr[6];

	if (sentido_dr == 1)
		pos_dr[0] = 47;
	else if (sentido_dr == 0)
		pos_dr[0] = 1;
	else if (sentido_dr == 3)
		pos_dr[0] = 93;
	pos_dr[1] = 122;
	pos_dr[2] = x_dr;
	pos_dr[3] = y_dr;
	pos_dr[4] = 45;
	pos_dr[5] = 77;

	DIBUJA_BLOQUE_CUT(pos_dr, dir_hare_fondo, dir_zona_pantalla);
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

	DIBUJA_BLOQUE_CUT(pos_bj, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::habla_igor_dch(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 56, 82, 108, 134, 160, 186, 212, 238 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	pon_igor();
	pon_dr();
	DIBUJA_FONDO(x_igor + 17, y_igor, x_igor + 17, y_igor, 37, 24, dir_dibujo1, dir_zona_pantalla);

	DIBUJA_BLOQUE(x_habla[cara], 148, x_igor + 17, y_igor, 25, 24, dir_hare_frente, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_igor + 26, y_igor);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_dr_izq(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	pon_igor();
	pon_dr();

	DIBUJA_FONDO(x_dr, y_dr, x_dr, y_dr, 38, 31, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 90, x_dr, y_dr, 38, 31, dir_hare_fondo, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_dr + 19, y_dr);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_dr_dch(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();

	pon_igor();
	pon_dr();

	DIBUJA_FONDO(x_dr, y_dr, x_dr, y_dr, 45, 31, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 58, x_dr + 7, y_dr, 38, 31, dir_hare_fondo, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_dr + 19, y_dr);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_solo(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	color_abc(color_solo);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	if (con_voces == 0)
		centra_texto(dicho, 156, 90);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
		goto bucless;
	}
}

void DrasculaEngine::habla_igor_frente(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 56, 86, 116, 146, 176, 206, 236, 266 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	pon_igor();
	pon_dr();
	DIBUJA_FONDO(x_igor, y_igor, x_igor, y_igor, 29, 25, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 173, x_igor, y_igor, 29, 25, dir_hare_frente, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_igor + 26, y_igor);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_igor();
	pon_dr();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_tabernero(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[9] = { 1, 23, 45, 67, 89, 111, 133, 155, 177 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(MARRON);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	if (music_status() == 0)
		playmusic(musica_room);

	cara = _rnd->getRandomNumber(8);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 2, 121, 44, 21, 24, dir_hare_dch, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 132, 45);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete(sku);
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::hipo(int contador) {
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

comienza:
	contador--;

	refresca_pantalla();
	if (num_ejec <= 2)
		VUELCA_PANTALLA(0, 1, 0, y, 320, 198, dir_zona_pantalla);
	else if (num_ejec == 3)
		VUELCA_PANTALLA(0, 0, 0, y, 320, 200, dir_zona_pantalla);

	if (sentido == 0)
		y++;
	else
		y--;
	if (num_ejec <= 2) {
		if (y == 2)
			sentido = 1;
		if (y == 0)
			sentido = 0;
	} else if (num_ejec == 3) {
		if (y == 1)
			sentido = 1;
		if (y == -1)
			sentido = 0;
	}
	if (contador > 0)
		goto comienza;

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::fin_sound() {
	delay(1);

	if (hay_sb == 1) {
		while (LookForFree() != 0);
		delete sku;
	}
}

void DrasculaEngine::habla_bj(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[5] = { 64, 92, 120, 148, 176 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(4);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_bj + 2, y_bj - 1, x_bj + 2, y_bj - 1, 27, 40,
				dir_dibujo1, dir_zona_pantalla);

	DIBUJA_BLOQUE(x_habla[cara], 99, x_bj + 2, y_bj - 1, 27, 40,
				dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_bj + 7, y_bj);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key == Common::KEYCODE_ESCAPE)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::hablar(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int suma_1_pixel = 1;

	int y_mask_habla = 170;
	int x_habla_dch[6] = { 1, 25, 49, 73, 97, 121 };
	int x_habla_izq[6] = { 145, 169, 193, 217, 241, 265 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	if (num_ejec == 6) {
		if (flags[0] == 0 && (!strcmp(num_room, "102.alg"))) {
			habla_pen(dicho, filename);
			return;
		}
		if (flags[0] == 0 && (!strcmp(num_room, "58.alg"))) {
			habla_pen2(dicho, filename);
			return;
		}
	}

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	if (factor_red[hare_y + alto_hare] == 100)
		suma_1_pixel = 0;
	
	if (num_ejec == 2)
		buffer_teclado();

	if (num_ejec == 4) {
		if (strcmp(num_room, "24.alg") || flags[29] == 0)
			color_abc(AMARILLO);
	} else {
		color_abc(AMARILLO);
	}
	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(5);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();
	if (num_ejec == 1)
		DIBUJA_FONDO(hare_x, hare_y, ANCHOBJ + 1, 0, ancho_hare * factor_red[hare_y + alto_hare] / 100,
				(alto_habla - 1) * factor_red[hare_y + alto_hare] / 100, dir_zona_pantalla, dir_dibujo3);
	else if (num_ejec == 2)
		DIBUJA_FONDO(hare_x, hare_y, ANCHOBJ + 1, 0, ancho_hare, alto_habla - 1, dir_zona_pantalla, dir_dibujo3);
	pon_hare();

	if (num_ejec == 1)
		DIBUJA_FONDO(ANCHOBJ + 1, 0, hare_x, hare_y, ancho_hare * factor_red[hare_y + alto_hare] / 100,
					(alto_habla - 1) * factor_red[hare_y + alto_hare] / 100, dir_dibujo3, dir_zona_pantalla);
	else if (num_ejec == 2)
		DIBUJA_FONDO(ANCHOBJ + 1, 0, hare_x, hare_y,
				ancho_hare, alto_habla - 1,
				dir_dibujo3, dir_zona_pantalla);

	if (sentido_hare == 0) {
		if (num_ejec == 1)
			reduce_hare_chico(x_habla_izq[cara], y_mask_habla, hare_x + 8 * factor_red[hare_y + alto_hare] / 100,
						hare_y, ancho_habla, alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		else if (num_ejec == 2)
			DIBUJA_BLOQUE(x_habla_dch[cara], y_mask_habla, hare_x + 12, hare_y, ancho_habla, alto_habla,
						dir_hare_dch, dir_zona_pantalla);
		actualiza_refresco();
	} else if (sentido_hare == 1) {
		if (num_ejec == 1)
			reduce_hare_chico(x_habla_dch[cara], y_mask_habla, hare_x + 12 * factor_red[hare_y + alto_hare] / 100,
						hare_y, ancho_habla,alto_habla, factor_red[hare_y + alto_hare], 
						dir_hare_dch, dir_zona_pantalla);
		else if (num_ejec == 2)
			DIBUJA_BLOQUE(x_habla_dch[cara], y_mask_habla, hare_x + 8, hare_y, ancho_habla, alto_habla,
						dir_hare_dch, dir_zona_pantalla);
		actualiza_refresco();
	} else if (sentido_hare == 2) {
		reduce_hare_chico(x_habla_izq[cara], y_mask_habla,
						suma_1_pixel + hare_x + 12 * factor_red[hare_y + alto_hare] / 100,
						hare_y, ancho_habla, alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		actualiza_refresco();
	} else if (sentido_hare == 3) {
		reduce_hare_chico(x_habla_dch[cara], y_mask_habla,
						suma_1_pixel + hare_x + 8 * factor_red[hare_y + alto_hare] / 100,
						hare_y, ancho_habla,alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		actualiza_refresco();
	}

	if (con_voces == 0)
		centra_texto(dicho, hare_x, hare_y);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (num_ejec == 1)
		if (key == Common::KEYCODE_ESCAPE)
			term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (num_ejec == 1)
		if (music_status() == 0 && flags[11] == 0 && corta_musica == 0)
			playmusic(musica_room);
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

void DrasculaEngine::refresca_pantalla() {
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	if (num_ejec <= 2)
		pon_hare();
	else if (num_ejec == 3) {
		if (flags[0] == 0)
			pon_hare();
		else
			DIBUJA_BLOQUE(113, 54, hare_x - 20, hare_y - 1, 77, 89, dir_dibujo3, dir_zona_pantalla);
	}
	actualiza_refresco();
}

void DrasculaEngine::carga_partida(const char *nom_game) {
	int l, n_ejec2;
	Common::InSaveFile *sav;

	canal_p(nom_game);
	if (!(sav = _saveFileMan->openForLoading("nom_game"))) {
		error("missing savegame file");
	}

	n_ejec2 = sav->readSint32LE();
	if (n_ejec2 != num_ejec) {
		canal_p(nom_game);
		strcpy(nom_partida, nom_game);
		error("TODO");
		salir_al_dos(n_ejec2);
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

	canal_p(nom_game);
}

void DrasculaEngine::canal_p(const char *fich){
	return;
	// TODO

	Common::File ald2, ald3;

	char fich2[13];
	char car;

	strcpy(fich2, "top");

	ald3.open(fich);
	if (!ald3.isOpen()) {
		error("no puedo abrir el archivo codificado");
	}

	ald2.open(fich2, Common::File::kFileWriteMode);
	if (!ald2.isOpen()) {
		error("no puedo abrir el archivo destino");
	}

	car = ald3.readByte();
	while (!ald3.eos()) {
		ald2.writeByte(codifica(car));
		car = ald3.readByte();
	}

	ald2.close();
	ald3.close();
	remove(fich);
	rename(fich2, fich);
}

void DrasculaEngine::puertas_cerradas (int l) {
	if (num_ejec == 1)
		return;

	if (num_ejec == 4) {
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
		return;
	}

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

	if (num_ejec == 1) {
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
	} else if (num_ejec == 2) {
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
	}
	conta_vez = vez();
}

void DrasculaEngine::actualiza_refresco() {
	if (!strcmp(num_room, "63.alg"))
		refresca_63();
	else if (!strcmp(num_room, "62.alg"))
		refresca_62();
	else if (!strcmp(num_room, "60.alg"))
		refresca_60();
	else if (!strcmp(num_room, "61.alg"))
		refresca_61();
	else if (!strcmp(num_room, "58.alg"))
		refresca_58();
 	else if (!strcmp(num_room, "3.alg"))
		refresca_3();
	else if (!strcmp(num_room, "2.alg"))
		refresca_2();
	else if (!strcmp(num_room, "4.alg"))
		refresca_4();
	else if (!strcmp(num_room, "5.alg"))
		refresca_5();
	else if (!strcmp(num_room, "15.alg"))
		refresca_15();
	else if (!strcmp(num_room, "17.alg"))
		refresca_17();
	else if (!strcmp(num_room, "18.alg"))
		refresca_18();
	else if (!strcmp(num_room, "10.alg"))
		mapa();
	else if (!strcmp(num_room, "20.alg"))
		refresca_20();
	else if (!strcmp(num_room, "13.alg"))
		refresca_13();
	else if (!strcmp(num_room, "45.alg"))
		mapa();
	else if (!strcmp(num_room, "50.alg"))
		refresca_50();
	else if (!strcmp(num_room, "57.alg"))
		refresca_57();
 }

void DrasculaEngine::actualiza_refresco_antes() {
	if (!strcmp(num_room, "62.alg"))
		refresca_62_antes();
	else if (!strcmp(num_room, "102.alg"))
		refresca_pendulo();
	else if (!strcmp(num_room, "58.alg"))
		refresca_58_antes();
	else if (!strcmp(num_room, "59.alg"))
		refresca_59_antes();
	else if (!strcmp(num_room, "60.alg"))
		refresca_60_antes();
	else if (!strcmp(num_room, "1.alg"))
		refresca_1_antes();
	else if (!strcmp(num_room, "3.alg"))
		refresca_3_antes();
	else if (!strcmp(num_room, "5.alg"))
		refresca_5_antes();
	else if (!strcmp(num_room, "6.alg"))
		refresca_6_antes();
	else if (!strcmp(num_room, "7.alg"))
		refresca_7_antes();
	else if (!strcmp(num_room, "9.alg"))
		refresca_9_antes();
	else if (!strcmp(num_room, "12.alg"))
		refresca_12_antes();
	else if (!strcmp(num_room, "14.alg"))
		refresca_14_antes();
	else if (!strcmp(num_room, "16.alg"))
		if (num_ejec == 1)
			pon_bj();
		else if (num_ejec == 2) {
			refresca_16_antes();
		}
	else if (!strcmp(num_room,"17.alg"))
		refresca_17_antes();
	else if (!strcmp(num_room,"18.alg"))
		refresca_18_antes();
	else if (!strcmp(num_room,"49.alg"))
		refresca_49_antes();
	else if (!strcmp(num_room,"53.alg"))
		refresca_53_antes();
	else if (!strcmp(num_room,"54.alg"))
		refresca_54_antes();
	else if (!strcmp(num_room,"56.alg"))
		refresca_56_antes();
}

void DrasculaEngine::pon_hare() {
	int pos_hare[6];
	int r;

	if (hare_se_mueve == 1 && paso_x == PASO_HARE_X) {
		for (r = 0; r < paso_x; r++) {
			if (num_ejec == 1) {
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

	if (num_ejec == 1)
		if (hare_se_ve == 0)
			goto no_vuelco;

	if (hare_se_mueve == 0) {
		pos_hare[0] = 0;
		pos_hare[1] = DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 1) {
			pos_hare[4] = ANCHO_PERSONAJE;
			pos_hare[5] = ALTO_PERSONAJE;
		} else if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		}

		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1)
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_dch, dir_zona_pantalla);
		else if (sentido_hare == 2)
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_fondo, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_fondo, dir_zona_pantalla);
		else
			if (num_ejec == 1)
				reduce_hare_chico( pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_frente, dir_zona_pantalla);
	} else if (hare_se_mueve == 1) {
		pos_hare[0] = frame_x[num_frame];
		pos_hare[1] = frame_y + DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		if (num_ejec == 1) {
			pos_hare[4] = ANCHO_PERSONAJE;
			pos_hare[5] = ALTO_PERSONAJE;
		} else if (num_ejec == 2) {
			pos_hare[4] = ancho_hare;
			pos_hare[5] = alto_hare;
		}
		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1)
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_dch, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_dch, dir_zona_pantalla);
		else if (sentido_hare == 2)
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_fondo, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_fondo, dir_zona_pantalla);
		else
			if (num_ejec == 1)
				reduce_hare_chico(pos_hare[0], pos_hare[1], pos_hare[2], pos_hare[3], pos_hare[4], pos_hare[5],
									factor_red[hare_y + alto_hare], dir_hare_frente, dir_zona_pantalla);
			else if (num_ejec == 2)
				DIBUJA_BLOQUE_CUT(pos_hare, dir_hare_frente, dir_zona_pantalla);

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

		if (h != 0)
			if (num_ejec < 6)
				DIBUJA_FONDO(x_pol[n], y_pol[n], x_obj[n], y_obj[n],
						ANCHOBJ, ALTOBJ, dir_hare_frente, dir_zona_pantalla);
			else
				DIBUJA_FONDO(x_pol[n], y_pol[n], x_obj[n], y_obj[n],
						ANCHOBJ, ALTOBJ, dir_mesa, dir_zona_pantalla);

		DIBUJA_BLOQUE(x1d_menu[h], y1d_menu[h], x_obj[n], y_obj[n],
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
		DIBUJA_BLOQUE(ANCHOBJ * n, ALTOBJ * sobre_verbo, x_barra[n], 2,
						ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_zona_pantalla);
		sobre_verbo = 1;
	}
}

void DrasculaEngine::saca_objeto() {
	int h = 0, n;

	refresca_pantalla();

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
			hablar(TEXT442, "442.als");
		else {
			puertas_cerradas(l);

			if (espuerta[l] != 0) {
				lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
				sentido_hare = sentidobj[l];
				refresca_pantalla();
				VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				hare_se_mueve = 0;
				sentido_hare = sentido_alkeva[l];
				obj_saliendo = alapuertakeva[l];
				rompo = 1;
				musica_antes = musica_room;

				if (num_obj[l] == 105)
					if (animacion_2_1())
						return true;
				borra_pantalla();
				strcpy(salgo, alapantallakeva[l]);
				strcat(salgo, ".ald");
				hare_x = -1;
				carga_escoba_1(salgo);
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
				animacion_2_2();
			if (num_obj[l] == 124)
				animacion_3_2();
			if (num_obj[l] == 173)
				animacion_35();
			if (num_obj[l] == 146 && flags[39] == 1) {
				flags[5] = 1;
				flags[11] = 1;
			}
			if (num_obj[l] == 176 && flags[29] == 1) {
				flags[29] = 0;
				resta_objeto(23);
				suma_objeto(11);
			}
			borra_pantalla();
			ald->close();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba_2(salgo);
		}
	} else if (num_ejec == 3) {
		puertas_cerradas(l);
		if (espuerta[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			borra_pantalla();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x =- 1;
			carga_escoba_2(salgo);
		}
	} else if (num_ejec == 4) {
		puertas_cerradas(l);
		if (espuerta[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;

			if (num_obj[l] == 108)
				lleva_al_hare(171, 78);
			borra_pantalla();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba_1(salgo);
		}
	} else if (num_ejec == 5) {
		puertas_cerradas(l);
		if (espuerta[l] != 0 && visible[l] == 1) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			hare_se_ve = 1;
			borra_pantalla();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba_1(salgo);
        }
	} else if (num_ejec == 6) {
		puertas_cerradas(l);

		if (espuerta[l] != 0) {
			lleva_al_hare(sitiobj_x[l], sitiobj_y[l]);
			sentido_hare = sentidobj[l];
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_mueve = 0;
			sentido_hare = sentido_alkeva[l];
			obj_saliendo = alapuertakeva[l];
			rompo = 1;
			musica_antes = musica_room;
			borra_pantalla();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba_1(salgo);

			if (obj_saliendo == 105)
				animacion_19_6();
		}
	}

	return false;
}

void DrasculaEngine::coge_objeto() {
	int h, n;
	h = objeto_que_lleva;
	comprueba_flags = 1;

	refresca_pantalla();
	menu_sin_volcar();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

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
		comprueba_banderas_menu();
	}
	espera_soltar();
	if (lleva_objeto == 0)
		sin_verbo();
}

void DrasculaEngine::banderas(int fl) {
	hare_se_mueve = 0;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	hay_respuesta = 1;

	if (menu_scr == 1) {
		if (num_ejec == 4) {
			if (objeto_que_lleva == MIRAR && fl == 28)
				hablar(TEXT328, "328.als");
			else if (objeto_que_lleva == MIRAR && fl == 7)
				hablar(TEXT478, "478.als");
			else if (objeto_que_lleva == MIRAR && fl == 8)
				hablar(TEXT120, "120.als");
			else if (objeto_que_lleva == MIRAR && fl == 9) {
				hablar(TEXT482, "482.als");
				hablar(TEXT483, "483.als");
			} else if (objeto_que_lleva == MIRAR && fl == 11)
				hablar(TEXT488, "488.als");
			else if (objeto_que_lleva == MIRAR && fl == 13)
				hablar(TEXT490, "490.als");
			else if (objeto_que_lleva == MIRAR && fl == 14)
				hablar(TEXT121, "121.als");
			else if (objeto_que_lleva == MIRAR && fl == 15)
				hablar(TEXT117, "117.als" );
			else if (objeto_que_lleva == HABLAR && fl == 15)
				hablar(TEXT118, "118.als" );
			else if (objeto_que_lleva == ABRIR && fl == 15)
				hablar(TEXT119, "119.als" );
			else if (objeto_que_lleva == MIRAR && fl == 17)
				hablar(TEXT478, "478.als");
			else if (objeto_que_lleva == MIRAR && fl == 20)
				hablar(TEXT162, "162.als");
			else
				hay_respuesta = 0;
			return;
		}
		if (num_ejec == 3) {
			if (objeto_que_lleva == MIRAR && fl == 22)
				hablar(TEXT307, "307.als");
			else if (objeto_que_lleva == MIRAR && fl == 28)
				hablar(TEXT328, "328.als");
			else if (objeto_que_lleva == MIRAR && fl == 7)
				hablar(TEXT143, "143.als");
			else if (objeto_que_lleva == HABLAR && fl == 7)
				hablar(TEXT144, "144.als");
			else if (objeto_que_lleva == MIRAR && fl == 8)
				hablar(TEXT145, "145.als");
			else if (objeto_que_lleva == HABLAR && fl == 8)
				hablar(TEXT146, "146.als");
			else if (objeto_que_lleva == MIRAR && fl == 9)
				hablar(TEXT147, "147.als");
			else if (objeto_que_lleva == HABLAR && fl == 9)
				hablar(TEXT148, "148.als");
			else if (objeto_que_lleva == MIRAR && fl == 10)
				hablar(TEXT151, "151.als");
			else if (objeto_que_lleva == MIRAR && fl == 11)
				hablar(TEXT152, "152.als");
			else if (objeto_que_lleva == HABLAR && fl == 11)
				hablar(TEXT153, "153.als");
			else if (objeto_que_lleva == MIRAR && fl == 12)
				hablar(TEXT154, "154.als");
			else if (objeto_que_lleva == MIRAR && fl == 13)
				hablar(TEXT155, "155.als");
			else if (objeto_que_lleva == MIRAR && fl == 14)
				hablar(TEXT157, "157.als");
			else if (objeto_que_lleva == MIRAR && fl == 15)
				hablar(TEXT58, "58.als");
			else if (objeto_que_lleva == MIRAR && fl == 16)
				hablar(TEXT158, "158.als");
			else if (objeto_que_lleva == MIRAR && fl == 17)
				hablar(TEXT159, "159.als");
			else if (objeto_que_lleva == MIRAR && fl == 18)
				hablar(TEXT160, "160.als");
			else if (objeto_que_lleva == MIRAR && fl == 19)
				hablar(TEXT161, "161.als");
			else if (objeto_que_lleva == MIRAR && fl == 20)
				hablar(TEXT162, "162.als");
			else if (objeto_que_lleva == MIRAR && fl == 23)
				hablar(TEXT152, "152.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 1) {
			if (objeto_que_lleva == MIRAR && fl == 28)
				hablar(TEXT328, "328.als");
			if ((objeto_que_lleva == MIRAR && fl == 22 && flags[23] == 0)
					|| (objeto_que_lleva == ABRIR && fl == 22 && flags[23] == 0)) {
				hablar(TEXT164, "164.als");
				flags[23] = 1;
				sin_verbo();
				suma_objeto(7);
				suma_objeto(18);
			} else if (objeto_que_lleva == MIRAR && fl == 22 && flags[23] == 1)
				hablar(TEXT307, "307.als");
			else if (objeto_que_lleva == MIRAR && fl == 7)
				hablar(TEXT143, "143.als");
			else if (objeto_que_lleva == HABLAR && fl == 7)
				hablar(TEXT144, "144.als");
			else if (objeto_que_lleva == MIRAR && fl == 8)
				hablar(TEXT145, "145.als");
			else if (objeto_que_lleva == HABLAR && fl == 8)
				hablar(TEXT146, "146.als");
			else if (objeto_que_lleva == MIRAR && fl == 9)
				hablar(TEXT147, "147.als");
			else if (objeto_que_lleva == HABLAR && fl == 9)
				hablar(TEXT148, "148.als");
			else if (objeto_que_lleva == MIRAR && fl == 10)
				hablar(TEXT151, "151.als");
			else if (objeto_que_lleva == MIRAR && fl == 11)
				hablar(TEXT152, "152.als");
			else if (objeto_que_lleva == HABLAR && fl == 11)
				hablar(TEXT153, "153.als");
			else if (objeto_que_lleva == MIRAR && fl == 12)
				hablar(TEXT154, "154.als");
			else if (objeto_que_lleva == MIRAR && fl == 13)
				hablar(TEXT155, "155.als");
			else if (objeto_que_lleva == MIRAR && fl == 14)
				hablar(TEXT157, "157.als");
			else if (objeto_que_lleva == MIRAR && fl == 15)
				hablar(TEXT58, "58.als");
			else if (objeto_que_lleva == MIRAR && fl == 16)
				hablar(TEXT158, "158.als");
			else if (objeto_que_lleva == MIRAR && fl == 17)
				hablar(TEXT159, "159.als");
			else if (objeto_que_lleva == MIRAR && fl == 18)
				hablar(TEXT160, "160.als");
			else if (objeto_que_lleva == MIRAR && fl == 19)
				hablar(TEXT161, "161.als");
			else if (objeto_que_lleva == MIRAR && fl == 20)
				hablar(TEXT162, "162.als");
			else if (objeto_que_lleva == MIRAR && fl == 23)
				hablar(TEXT152, "152.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 4) {
			if ((objeto_que_lleva == 18 && fl == 19) || (objeto_que_lleva == 19 && fl == 18)) {
				sin_verbo();
				elige_objeto(21);
				resta_objeto(18);
				resta_objeto(19);
			} else if ((objeto_que_lleva == 14 && fl == 19) ||
					(objeto_que_lleva == 19 && fl == 14))
				hablar(TEXT484, "484.als");
			else if (objeto_que_lleva == MIRAR && fl == 28)
				hablar(TEXT328, "328.als");
			else if (objeto_que_lleva == MIRAR && fl == 7)
				hablar(TEXT478, "478.als");
			else if (objeto_que_lleva == MIRAR && fl == 8)
				hablar(TEXT480, "480.als");
			else if (objeto_que_lleva == MIRAR && fl == 9) {
				hablar(TEXT482, "482.als");
				hablar(TEXT483, "483.als");
			} else if (objeto_que_lleva == MIRAR && fl == 10)
				hablar(TEXT485, "485.als");
			else if (objeto_que_lleva == MIRAR && fl == 11)
				hablar(TEXT488, "488.als");
			else if (objeto_que_lleva == MIRAR && fl == 12)
				hablar(TEXT486, "486.als");
			else if (objeto_que_lleva == MIRAR && fl == 13)
				hablar(TEXT490, "490.als");
			else if (objeto_que_lleva == MIRAR && fl == 14)
				hablar(TEXT122, "122.als");
			else if (objeto_que_lleva == MIRAR && fl == 15)
				hablar(TEXT117, "117.als");
			else if (objeto_que_lleva == HABLAR && fl == 15)
				hablar(TEXT118, "118.als");
			else if (objeto_que_lleva == ABRIR && fl == 15)
				hablar(TEXT119, "119.als");
			else if (objeto_que_lleva == MIRAR && fl == 16)
				hablar(TEXT491, "491.als");
			else if (objeto_que_lleva == MIRAR && fl == 17)
				hablar(TEXT478, "478.als");
			else if (objeto_que_lleva == MIRAR && fl == 18)
				hablar(TEXT493, "493.als");
			else if (objeto_que_lleva == MIRAR && fl == 19) {
				hablar(TEXT494, "494.als");
				hablar(TEXT495, "495.als");
			} else if (objeto_que_lleva == MIRAR && fl == 20)
				hablar(TEXT162, "162.als");
			else if (objeto_que_lleva == MIRAR && fl == 21)
				hablar(TEXT496, "496.als");
			else if (objeto_que_lleva == MIRAR && fl == 22)
				hablar(TEXT161, "161.als");
			else
				hay_respuesta = 0;
		} else if (num_ejec == 5) {
			if (objeto_que_lleva == MIRAR && fl == 50)
				hablar("Cuanto mas me miro, mas me gusto.", "54.als");
			else if (objeto_que_lleva == ABRIR && fl == 50)
				hablar(".y luego como me cierro?.", "19.als");
			else if (objeto_que_lleva == CERRAR && fl == 50)
				hablar("Tendre que abrirme primero .no?.", "19.als");
			else if (objeto_que_lleva == MOVER && fl == 50)
				hablar("Estoy bien donde estoy.", "19.als");
			else if (objeto_que_lleva == COGER && fl == 50)
				hablar("Ya me tengo.", "11.als");
			else if (objeto_que_lleva == HABLAR && fl == 50)
				hablar("hola yo.", "16.als" );
			else if (objeto_que_lleva == 20 && fl == 50)
				hablar(TEXT487, "487.als" );
			else if (!strcmp(num_room, "49.alg"))
				pantalla_49(fl);
			else if (!strcmp(num_room, "53.alg"))
				pantalla_53(fl);
			else if (!strcmp(num_room, "54.alg"))
				pantalla_54(fl);
			else if (!strcmp(num_room, "55.alg"))
				pantalla_55(fl);
			else if (!strcmp(num_room, "56.alg"))
				pantalla_56(fl);
			else
				hay_respuesta = 0;
			return;
		} else if (num_ejec == 6) {
			if (objeto_que_lleva == MIRAR && fl == 28)
				hablar(TEXT328, "328.als");
			else if (objeto_que_lleva == MIRAR && fl == 9) {
				hablar(TEXT482, "482.als");
				hablar(TEXT483, "483.als");
			} else if (objeto_que_lleva == MIRAR && fl == 20)
				hablar(TEXT123, "123.als");
			else if (objeto_que_lleva == MIRAR && fl == 21)
				hablar(TEXT441, "441.als");
			else
				hay_respuesta = 0;
		}
	} else {
		if (objeto_que_lleva == MIRAR && fl == 50)
			if (num_ejec == 3)
				hablar(TEXT308, "308.als");
			else if (num_ejec == 6)
				if (flags[0] == 1)
					hablar(TEXT308, "308.als");
				else if (flags[0] == 0)
					hablar(TEXT250, "250.als");
			else
				hablar(TEXT309, "309.als");
		else if (objeto_que_lleva == ABRIR && fl == 50)
			hablar(TEXT310, "310.als" );
		else if (objeto_que_lleva == CERRAR && fl == 50)
			hablar(TEXT311, "311.als" );
		else if (objeto_que_lleva == MOVER && fl == 50)
			hablar(TEXT312, "312.als" );
		else if (objeto_que_lleva == COGER && fl == 50)
			hablar(TEXT313, "313.als" );
		else if (objeto_que_lleva == HABLAR && fl == 50)
			hablar(TEXT314, "314.als" );
		else if (!strcmp(num_room, "102.alg"))
			pantalla_pendulo(fl);
		else if (!strcmp(num_room, "58.alg"))
			pantalla_58(fl);
		else if (!strcmp(num_room, "59.alg"))
			pantalla_59(fl);
		else if (!strcmp(num_room, "60.alg"))
			pantalla_60(fl);
		else if (!strcmp(num_room, "61.alg"))
			pantalla_61(fl);
 		else if (!strcmp(num_room, "62.alg"))
			pantalla_62(fl);
		else if (!strcmp(num_room, "63.alg"))
			pantalla_63(fl);
		else if (!strcmp(num_room, "13.alg"))
			pantalla_13(fl);
		else
			hay_respuesta = 0;
	}
	if (hay_respuesta == 0 && hay_nombre == 1)
		pantalla_0();
	else if (hay_respuesta == 0 && menu_scr == 1)
		pantalla_0();
}

void DrasculaEngine::cursor_mesa() {
	int pos_cursor[8];

	pos_cursor[0] = 225;
	pos_cursor[1] = 56;
	pos_cursor[2] = x_raton - 20;
	pos_cursor[3] = y_raton - 12;
	pos_cursor[4] = 40;
	pos_cursor[5] = 25;

	DIBUJA_BLOQUE_CUT(pos_cursor, dir_mesa, dir_zona_pantalla);
}

void DrasculaEngine::introduce_nombre() {
	Common::KeyCode key;
	int v = 0, h = 0;
	char select2[23];
	strcpy(select2, "                      ");
	for (;;) {
		buffer_teclado();
		select2[v] = '-';
		DIBUJA_FONDO(115, 14, 115, 14, 176, 9, dir_dibujo1, dir_zona_pantalla);
		print_abc(select2, 117, 15);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		key = getscan();
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
				select2[v] = '¤';
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
				select2[v] = '§';
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
	GlobalSpeed = CLOCKS_PER_SEC / Pause;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	_Sesion = new Common::File;
	_Sesion->open(Name);
	mSesion = TryInMem(_Sesion);
	LastFrame = clock();
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
	while ((int)clock() < LastFrame + GlobalSpeed)
		;
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

float DrasculaEngine::vez() {
	// FIXME: This function is really silly. It first divides an int by an int (resulting
	// in an *int*, loosing precision), *then* converts the result to a float and returns
	// that -- only so that many calling functions have to convert it back to an int :-).
	// So: Either divide by 20.0 / cast to float *first*, if you absolutly need the precision.
	// Or: Just change this to return int!
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

			DIBUJA_BLOQUE_CUT(pos_pixel, dir_inicio, dir_fin);

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
	float distancia_x = 0, distancia_y;

	if (num_ejec == 1)
		distancia_x = hare_x + ancho_hare / 2 - sitio_x;
	else if (num_ejec == 2)
		distancia_x = hare_x + ancho_hare - sitio_x;

	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 0;
		sentido_hare = 2;
		paso_x = (int)distancia_x / ((int)distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 7;
		sentido_hare = 0;
		paso_y = (int)distancia_y / ((int)distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_2() {
	float distancia_x = 0, distancia_y;

	if (num_ejec == 1)
		distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);
	else if (num_ejec == 2)
		distancia_x = abs(hare_x + ancho_hare - sitio_x);

	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 1;
		sentido_hare = 2;
		paso_x = (int)distancia_x / ((int)distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 2;
		sentido_hare = 1;
		paso_y = (int)distancia_y / ((int)distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_3() {
	float distancia_x = 0, distancia_y;

	if (num_ejec == 1)
		distancia_x = hare_x + ancho_hare / 2 - sitio_x;
	else if (num_ejec == 2)
		distancia_x = hare_x + ancho_hare - sitio_x;

	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x < distancia_y) {
		direccion_hare = 5;
		sentido_hare = 3;
		paso_x = (int)distancia_x / ((int)distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 6;
		sentido_hare = 0;
		paso_y = (int)distancia_y / ((int)distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_4() {
	float distancia_x = 0, distancia_y;

	if (num_ejec == 1)
		distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);
	else if (num_ejec == 2)
		distancia_x = abs(hare_x + ancho_hare - sitio_x);

	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x <distancia_y) {
		direccion_hare = 4;
		sentido_hare = 3;
		paso_x = (int)distancia_x / ((int)distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 3;
		sentido_hare = 1;
		paso_y = (int)distancia_y / ((int)distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::refresca_62() {
	int borracho_x[] = { 1, 42, 83, 124, 165, 206, 247, 1 };

	DIBUJA_BLOQUE(1, 1, 0, 0, 62, 142, dir_dibujo2, dir_zona_pantalla);

	if (hare_y + alto_hare < 89) {
		DIBUJA_BLOQUE(205, 1, 180, 9, 82, 80, dir_dibujo3, dir_zona_pantalla);
		DIBUJA_FONDO(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);
	}
}

void DrasculaEngine::refresca_63() {
	DIBUJA_BLOQUE(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_62_antes() {
	int velas_y[] = { 158, 172, 186 };
	int cirio_x[] = { 14, 19, 24 };
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31 };
	int borracho_x[] = {1, 42, 83, 124, 165, 206, 247, 1 };
	int diferencia;

	DIBUJA_FONDO(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);
	DIBUJA_FONDO(cirio_x[frame_velas], 146, 311, 80, 4, 8, dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5)
		DIBUJA_FONDO(1, 149, 127, 52, 9, 5, dir_dibujo3, dir_zona_pantalla);

	if (hare_x > 101 && hare_x < 155)
		DIBUJA_FONDO(31, 138, 178, 51, 18, 16, dir_dibujo3, dir_zona_pantalla);

	if (flags[11] == 0)
		DIBUJA_FONDO(pianista_x[frame_piano], 157, 245, 130, 29, 42, dir_dibujo3, dir_zona_pantalla);
	else if (flags[5] == 0)
		DIBUJA_FONDO(145, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	else
		DIBUJA_FONDO(165, 140, 229, 117, 43, 59, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		DIBUJA_FONDO(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);

	diferencia = (int)vez() - conta_ciego_vez;
	if (diferencia > 6) {
		if (flags[12] == 1) {
			frame_borracho++;
			if (frame_borracho == 8) {
				frame_borracho = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		parpadeo = _rnd->getRandomNumber(10);
		conta_ciego_vez = (int)vez();
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
	canal_p(nom_game);
}

void DrasculaEngine::aumenta_num_frame() {
	diff_vez = vez() - conta_vez;

	if (diff_vez >= 5.7) {
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

	if (num_ejec == 1) {
		diferencia_y = alto_hare - (int)nuevo_alto;
		diferencia_x = ancho_hare - (int)nuevo_ancho;
		hare_y = hare_y + diferencia_y;
		hare_x = hare_x + diferencia_x;
		alto_hare = (int)nuevo_alto;
		ancho_hare = (int)nuevo_ancho;
	}
}

int DrasculaEngine::sobre_que_objeto() {
	int n;

	for (n = 1; n < 43; n++) {
		if (x_raton > x_obj[n] && y_raton > y_obj[n]
				&& x_raton < x_obj[n] + ANCHOBJ && y_raton < y_obj[n] + ALTOBJ)
			break;
	}

	return n;
}

void DrasculaEngine::comprueba_banderas_menu() {
	int h, n;

	for (n = 0; n < 43; n++) {
		if (sobre_que_objeto() == n) {
			h = objetos_que_tengo[n];
			if (h != 0)
				banderas(h);
		}
	}
}

void DrasculaEngine::pantalla_0() {
	if (num_ejec == 6) {
		if (objeto_que_lleva == MIRAR) {
			hablar(mirar_t[c_mirar], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVER)
			hablar(TEXT19, "19.als");
		else if (objeto_que_lleva == COGER) {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == ABRIR)
			hablar(TEXT9, "9.als");
		else if (objeto_que_lleva == CERRAR)
			hablar(TEXT9, "9.als");
		else if (objeto_que_lleva == HABLAR)
			hablar(TEXT16, "16.als");
		else {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	}
	if (num_ejec == 5) {
		if (objeto_que_lleva == MIRAR) {
			hablar(mirar_t[c_mirar], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		} else if (objeto_que_lleva == MOVER)
			hablar(TEXT19, "19.als");
		else if (objeto_que_lleva == COGER) {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		} else if (objeto_que_lleva == ABRIR)
			hablar(TEXT9, "9.als");
		else if (objeto_que_lleva == CERRAR)
			hablar(TEXT9, "9.als");
		else if (objeto_que_lleva == HABLAR)
			hablar(TEXT16, "16.als");
		else {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
 		return;
	}
	if (num_ejec == 3) {
		if (objeto_que_lleva == MIRAR)
			hablar(TEXT316, "316.als");
		else if (objeto_que_lleva == MOVER)
			hablar(TEXT317, "317.als");
		else if (objeto_que_lleva == COGER)
			hablar(TEXT318, "318.als");
		else if (objeto_que_lleva == ABRIR)
			hablar(TEXT319, "319.als");
		else if (objeto_que_lleva == CERRAR)
			hablar(TEXT319, "319.als");
		else if (objeto_que_lleva == HABLAR)
			hablar(TEXT320, "320.als");
		else
			hablar(TEXT318, "318.als");
		return;
 	}

	if (objeto_que_lleva == MIRAR) {
		if (num_ejec == 1)
			hablar(TEXT54, "54.als");
		else if (num_ejec == 2) {
			hablar(mirar_t[c_mirar], mirar_v[c_mirar]);
			c_mirar++;
			if (c_mirar == 3)
				c_mirar = 0;
		}
	} else if (objeto_que_lleva == MOVER)
		hablar(TEXT19, "19.als" );
	else if (objeto_que_lleva == COGER)
		if (num_ejec == 1)
			hablar(TEXT11, "11.als" );
		else if (num_ejec == 2) {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	else if (objeto_que_lleva == ABRIR)
		hablar(TEXT9, "9.als" );
	else if (objeto_que_lleva == CERRAR)
		hablar(TEXT9, "9.als" );
	else if (objeto_que_lleva == HABLAR)
		hablar(TEXT16, "16.als" );
	else {
		if (num_ejec == 1)
			hablar(TEXT11, "11.als");
		else if (num_ejec == 1) {
			hablar(poder_t[c_poder], poder_v[c_poder]);
			c_poder++;
			if (c_poder == 6)
				c_poder = 0;
		}
	}
}

void DrasculaEngine::pantalla_62(int fl) {
	if (objeto_que_lleva == HABLAR && fl == 53)
		conversa("op_13.cal");
	else if (objeto_que_lleva == HABLAR && fl == 52 && flags[0] == 0)
		animacion_3_2();
	else if (objeto_que_lleva == HABLAR && fl == 52 && flags[0] == 1)
		hablar(TEXT109, "109.als");
	else if (objeto_que_lleva == HABLAR && fl == 54)
		animacion_4_1();
	else if (objeto_que_lleva == MIRAR && fl == 100)
		hablar(TEXT168, "168.als");
	else if (objeto_que_lleva == HABLAR && fl == 100)
		hablar(TEXT169, "169.als");
	else if (objeto_que_lleva == COGER && fl == 100)
		hablar(TEXT170, "170.als");
	else if (objeto_que_lleva == MIRAR && fl == 101)
		hablar(TEXT171, "171.als");
	else if (objeto_que_lleva == MIRAR && fl == 102)
		hablar(TEXT167, "167.als");
	else if (objeto_que_lleva == MIRAR && fl == 103)
		hablar(TEXT166, "166.als");
	else hay_respuesta = 0;
}

void DrasculaEngine::pantalla_63(int fl) {
	if (objeto_que_lleva == MIRAR && fl == 110)
		hablar(TEXT172, "172.als");
	else if (objeto_que_lleva == MIRAR && fl == 109)
		hablar(TEXT173, "173.als");
	else if (objeto_que_lleva == MOVER && fl == 109)
		hablar(TEXT174, "174.als");
	else if (objeto_que_lleva == MIRAR && fl == 108)
		hablar(TEXT334, "334.als");
	else if (objeto_que_lleva == HABLAR && fl == 108)
		hablar(TEXT333, "333.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::conversa(const char *nom_fich) {
	int h;
	int juego1 = 1, juego2 = 1, juego3 = 1, juego4 = 1;
	char frase1[78];
	char frase2[78];
	char frase3[78];
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
	canal_p(para_codificar);

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
	canal_p(para_codificar);

	if (num_ejec == 2 && !strcmp(nom_fich, "op_5.cal") && flags[38] == 1 && flags[33] == 1) {
		strcpy(frase3, TEXT405);
		strcpy(suena3, "405.als");
		respuesta3 = 31;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_5.cal") && flags[7] == 1) {
		strcpy(frase3, TEXT273);
		strcpy(suena3, "273.als");
		respuesta3 = 14;
	}

	if (num_ejec == 6 && !strcmp(nom_fich, "op_5.cal") && flags[10] == 1) {
		strcpy(frase3, ".cuanto.queda.para.que.acabe.el.partido?");
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

	lee_dibujos("car.alg");
	descomprime_dibujo(dir_hare_fondo,1);
/* TODO this is limit for mouse, but we can't implement this
	ent.w.ax =  8;
	ent.w.cx =  1;
	ent.w.dx = 31;
	int386(0x33, &ent, &sal);
*/
	color_abc(VERDE_CLARO);

bucle_opc:

	refresca_pantalla();

	if (num_ejec == 1)
		if (music_status() == 0 && flags[11] == 0)
			playmusic(musica_room);
	else if (num_ejec == 2)
		if (music_status() == 0 && flags[11] == 0 && musica_room != 0)
			playmusic(musica_room);
	else if (num_ejec == 3)
		if (music_status() == 0)
			playmusic(musica_room);
	else if (num_ejec == 4)
		if (music_status() == 0)
			playmusic(musica_room);
	else if (num_ejec == 5)
		if (music_status() == 0)
			playmusic(musica_room);

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

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if ((boton_izq == 1) && (juego1 == 2)) {
		usado1 = 1;
		hablar(frase1, suena1);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta1);
	} else if ((boton_izq == 1) && (juego2 == 2)) {
		usado2 = 1;
		hablar(frase2, suena2);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta2);
	} else if ((boton_izq == 1) && (juego3 == 2)) {
		usado3 = 1;
		hablar(frase3, suena3);
		if (num_ejec == 3)
			grr();
		else
			responde(respuesta3);
	} else if ((boton_izq == 1) && (juego4 == 2)) {
		hablar(frase4, suena4);
		rompo_y_salgo = 1;
	}

	if (boton_izq == 1)
		color_abc(VERDE_CLARO);

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

	if (num_ejec == 1)
		lee_dibujos("99.alg");
	else if (num_ejec == 2)
		lee_dibujos(fondo_y_menu);
	descomprime_dibujo(dir_hare_fondo, 1);
	if (num_ejec < 5)
		sin_verbo();
}

void DrasculaEngine::animacion_3_1() {
	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT192, "192.als");
	habla_tabernero(TEXTT1, "t1.als");
	hablar(TEXT193, "193.als");
	habla_tabernero(TEXTT2, "t2.als");
	hablar(TEXT194, "194.als");
	habla_tabernero(TEXTT3, "t3.als");
	hablar(TEXT195, "195.als");
	habla_tabernero(TEXTT4, "t4.als");
	hablar(TEXT196, "196.als");
	habla_tabernero(TEXTT5, "t5.als");
	habla_tabernero(TEXTT6, "t6.als");
	hablar(TEXT197, "197.als");
	habla_tabernero(TEXTT7, "t7.als");
	hablar(TEXT198, "198.als");
	habla_tabernero(TEXTT8, "t8.als");
	hablar(TEXT199, "199.als");
	habla_tabernero(TEXTT9, "t9.als");
	hablar(TEXT200, "200.als");
	hablar(TEXT201, "201.als");
	hablar(TEXT202, "202.als");

	flags[0] = 1;

	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
}

void DrasculaEngine::animacion_4_1() {
	lee_dibujos("an12.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	hablar(TEXT205,"205.als");

	actualiza_refresco_antes();

	DIBUJA_FONDO(1, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	VUELCA_PANTALLA(228,112, 228,112, 47,60, dir_zona_pantalla);

	pausa(3);

	actualiza_refresco_antes();

	DIBUJA_FONDO(49, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	pon_hare();

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);
	stopmusic();
	flags[11] = 1;

	habla_pianista(TEXTP1, "p1.als");
	hablar(TEXT206, "206.als");
	habla_pianista(TEXTP2, "p2.als");
	hablar(TEXT207, "207.als");
	habla_pianista(TEXTP3, "p3.als");
	hablar(TEXT208, "208.als");
	habla_pianista(TEXTP4, "p4.als");
	hablar(TEXT209, "209.als");

	flags[11] = 0;
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
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

		char c = toupper(dicho[h]);
		if (c == 'A')
			x_de_letra = X_A_OPC;
//		else if (c == 'µ')
//			x_de_letra = X_A_OPC;
//		else if (c == '·')
//			x_de_letra = X_A_OPC;
//		else if (c == '¶')
//			x_de_letra = X_A_OPC;
		else if (c == 'B'
			)x_de_letra = X_B_OPC;
		else if (c == 'C')
			x_de_letra = X_C_OPC;
//		else if (c == '€')
//			x_de_letra = X_C_OPC;
//		else if (c == '‡')
//			x_de_letra = X_C_OPC;
		else if (c == 'D')
			x_de_letra = X_D_OPC;
		else if (c == 'E')
			x_de_letra = X_E_OPC;
//		else if (c == '')
//			x_de_letra = X_E_OPC;
//		else if (c == 'Ô')
//			x_de_letra = X_E_OPC;
//		else if (c == 'Ò')
//			x_de_letra = X_E_OPC;
		else if (c == 'F')
			x_de_letra = X_F_OPC;
		else if (c == 'G')
			x_de_letra = X_G_OPC;
		else if (c == 'H')
			x_de_letra = X_H_OPC;
		else if (c == 'I')
			x_de_letra = X_I_OPC;
//		else if (c == 'Ö')
//			x_de_letra = X_I_OPC;
//		else if (c == 'Þ')
//			x_de_letra = X_I_OPC;
//		else if (c == '×')
//			x_de_letra = X_I_OPC;
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
//		else if (c == ''')
//			x_de_letra = X_GN_OPC;
		else if (c == 'O')
			x_de_letra = X_O_OPC;
//		else if (c == 'à')
//			x_de_letra = X_O_OPC;
//		else if (c == 'â')
//			x_de_letra = X_O_OPC;
//		else if (c == 'ã')
//			x_de_letra = X_O_OPC;
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
//		else if (c == 'ë')
//			x_de_letra = X_U_OPC;
//		else if (c == 'ê')
//			x_de_letra = X_U_OPC;
//		else if (c == 'é')
//			x_de_letra = X_U_OPC;
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
//		else if (c == ' ')
//			x_de_letra = ESPACIO_OPC;
		else
			y_de_letra = y_de_signos;

		if (c == '.')
			x_de_letra = X_PUNTO_OPC;
		else if (c == ',')
			x_de_letra = X_COMA_OPC;
		else if (c == '-')
			x_de_letra = X_GUION_OPC;
		else if (c == '?')
			x_de_letra = X_CIERRA_INTERROGACION_OPC;
//		else if (c == '¨')
//			x_de_letra = X_ABRE_INTERROGACION_OPC;
		else if (c == '"')
			x_de_letra = X_COMILLAS_OPC;
//		else if (c == '!')
//			x_de_letra = X_CIERRA_EXCLAMACION_OPC;
		else if (c == '­')
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

		pos_texto[0] = x_de_letra;
		pos_texto[1] = y_de_letra;
		pos_texto[2] = x_pantalla;
		pos_texto[3] = y_pantalla;
		pos_texto[4] = ANCHO_LETRAS_OPC;
		pos_texto[5] = ALTO_LETRAS_OPC;

		DIBUJA_BLOQUE_CUT(pos_texto, dir_hare_fondo, dir_zona_pantalla);

		x_pantalla = x_pantalla + ANCHO_LETRAS_OPC;
	}
}

void DrasculaEngine::responde(int funcion) {
	if (num_ejec == 5) {
		if (funcion == 2)
			animacion_2_5();
		else if (funcion == 3)
			animacion_3_5();
		else if (funcion == 6)
			animacion_6_5();
		else if (funcion == 7)
			animacion_7_5();
		else if (funcion == 8)
			animacion_8_5();
		else if (funcion == 15)
			animacion_15_5();
		else if (funcion == 16)
			animacion_16_5();
		else if (funcion == 17)
			animacion_17_5();
		return;
	}
	if (num_ejec == 6) {
		if (funcion == 2)
			animacion_2_6();
		else if (funcion == 3)
			animacion_3_6();
		else if (funcion == 4)
			animacion_4_6();
		else if (funcion == 11)
			animacion_11_6();
		else if (funcion == 12)
			animacion_12_6();
		else if (funcion == 13)
			animacion_13_6();
		else if (funcion == 14)
			animacion_14_6();
		else if (funcion == 15)
			animacion_15_6();
		return;
	}
	if (funcion == 2)
		animacion_2_4();
	else if (funcion == 3)
		animacion_3_4();
	else if (funcion == 4)
		animacion_4_4();
	else if (funcion == 10)
		habla_borracho(TEXTB1, "B1.als");
	else if (funcion == 11)
		habla_borracho(TEXTB2, "B2.als");
	else if (funcion == 12)
		habla_borracho(TEXTB3, "B3.als");
	else if (funcion == 8)
		animacion_8();
	else if (funcion == 9)
		animacion_9();
	else if (funcion == 10)
		animacion_10();
	else if (funcion == 15)
		animacion_15();
	else if (funcion == 16)
		animacion_16();
	else if (funcion == 17)
		animacion_17();
	else if (funcion == 19)
		animacion_19();
	else if (funcion == 20)
		animacion_20();
	else if (funcion == 21)
		animacion_21();
	else if (funcion == 23)
		animacion_23();
	else if (funcion == 28)
		animacion_28();
	else if (funcion == 29)
		animacion_29();
	else if (funcion == 30)
		animacion_30();
	else if (funcion == 31)
		animacion_31();
}

void DrasculaEngine::habla_pianista(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	int x_habla[4] = { 97, 145, 193, 241 };
	int cara;
	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(3);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 139, 228, 112, 47, 60,
				dir_hare_dch, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 221, 128);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_borracho(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 1, 21, 41, 61, 81, 101, 121, 141 };
	int cara;
	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	lee_dibujos("an11y13.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	flags[13] = 1;

bebiendo:

	if (flags[12] == 1) {
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		goto bebiendo;
	}

	buffer_teclado();

	color_abc(VERDE_OSCURO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz\n");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 29, 177, 50, 19, 19, dir_hare_frente, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 181, 54);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	byte key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	flags[13] = 0;
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	if (num_ejec <= 3)
		if (music_status() == 0 && flags[11] == 0)
			playmusic(musica_room);
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
	if ((!strcmp(num_room, "58.alg")) && flags[8] == 0)
		espuerta[1] = 0;
	if ((!strcmp(num_room, "58.alg")) && flags[8] == 1)
		espuerta[1] = 1;
	if (!strcmp(num_room, "59.alg"))
		espuerta[1] = 0;
	if (!strcmp(num_room, "60.alg")) {
		sentido_dr = 0; x_dr = 155; y_dr = 69;
	}
}

void DrasculaEngine::animacion_1_2() {
	lleva_al_hare(178, 121);
	lleva_al_hare(169, 135);
}

void DrasculaEngine::animacion_2_2() {
	int n, x=0;

	sentido_hare = 0;
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	lee_dibujos("an2_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an2_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_FONDO(1, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 1, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 55, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}
	
	x = 0;

	for (n = 0; n < 6; n++){
		x++;
		DIBUJA_FONDO(x, 109, 201, 87, 50, 52, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;
	comienza_sound("s2.als");

	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 1, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(201,87, 201,87, 50,52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 55, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}
	x = 0;

	for (n = 0; n < 2; n++) {
		x++;
		DIBUJA_FONDO(x, 109, 201, 87, 50, 52, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(201, 87, 201, 87, 50, 52, dir_zona_pantalla);
		x = x + 50;
		pausa(3);
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	fin_sound();        

	pausa (4);

	comienza_sound("s1.als");
	hipo_sin_nadie(12);
	fin_sound();
 }

void DrasculaEngine::animacion_3_2() {
	lleva_al_hare(163, 106);
	lleva_al_hare(287, 101);
	sentido_hare = 0;
}

void DrasculaEngine::animacion_4_2() {
	stopmusic();
	flags[9] = 1;

	pausa(12);
	hablar(TEXTD56, "d56.als");
	pausa(8);

	borra_pantalla();
	lee_dibujos("ciego1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("ciego2.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("ciego3.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("ciego4.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("ciego5.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(10);

	habla_ciego(TEXTD68, "d68.als", "44472225500022227555544444664447222550002222755554444466");
	pausa(5);
	habla_hacker(TEXTD57, "d57.als");
	pausa(6);
	habla_ciego(TEXTD69,"d69.als","444722255000222275555444446655033336666664464402256555005504450005446");
	pausa(4);
	habla_hacker(TEXTD58,"d58.als");
	habla_ciego(TEXTD70,"d70.als", "4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046");
	delay(14);
	habla_hacker(TEXTD59,"d59.als");
	habla_ciego(TEXTD71,"d71.als", "550330227556444744446660004446655544444722255000222275555444446644444");
	habla_hacker(TEXTD60,"d60.als");
	habla_ciego(TEXTD72,"d72.als", "55033022755644455550444744400044504447222550002222755554444466000");
	habla_hacker(TEXTD61,"d61.als");
	habla_ciego(TEXTD73,"d73.als", "55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446");
	habla_hacker(TEXTD62,"d62.als");
	habla_ciego(TEXTD74,"d74.als", "55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666");
	habla_hacker(TEXTD63,"d63.als");
	habla_ciego(TEXTD75,"d75.als", "44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555");
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	_system->delayMillis(1);
	habla_hacker(TEXTD64, "d64.als");
	habla_ciego(TEXTD76, "d76.als", "5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444");

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(14);

	borra_pantalla();

	playmusic(musica_room);
	lee_dibujos("9.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("aux9.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sin_verbo();

	flags[9] = 0;
	flags[4] = 1;
}

void DrasculaEngine::animacion_8() {
	habla_pianista(TEXTP6, "P6.als");
	hablar(TEXT358, "358.als");
	habla_pianista(TEXTP7, "P7.als");
	habla_pianista(TEXTP8, "P8.als");
}

void DrasculaEngine::animacion_9() {
	habla_pianista(TEXTP9, "P9.als");
	habla_pianista(TEXTP10, "P10.als");
	habla_pianista(TEXTP11, "P11.als");
}

void DrasculaEngine::animacion_10() {
	habla_pianista(TEXTP12, "P12.als");
	hablar(TEXT361, "361.als");
	pausa(40);
	habla_pianista(TEXTP13, "P13.als");
	hablar(TEXT362, "362.als");
	habla_pianista(TEXTP14, "P14.als");
	hablar(TEXT363, "363.als");
	habla_pianista(TEXTP15, "P15.als");
	hablar(TEXT364, "364.als");
	habla_pianista(TEXTP16, "P16.als");
}

void DrasculaEngine::animacion_14() {
	int n, pos_cabina[6];
	int l = 0;

	lee_dibujos("an14_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an14_1.alg");

	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = -160;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;
 
	for (n = -160; n <= 0; n = n + 5 + l) {
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		actualiza_refresco_antes();
		pon_hare();
		pon_vb();
		pos_cabina[3] = n;
		DIBUJA_BLOQUE_CUT(pos_cabina, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		l = l + 1;
	}

	flags[24] = 1;

	descomprime_dibujo(dir_dibujo1, 1);

	comienza_sound("s7.als");
	hipo(15);

	fin_sound();        

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animacion_15() {
	habla_borracho(TEXTB8, "B8.als");
	pausa(7);
	habla_borracho(TEXTB9, "B9.als");
	habla_borracho(TEXTB10, "B10.als");
	habla_borracho(TEXTB11, "B11.als");
}

void DrasculaEngine::animacion_16() {
	int l;

	habla_borracho(TEXTB12, "B12.als");
	hablar(TEXT371, "371.als");

	borra_pantalla();

	playmusic(32);
	int key = getscan();
	if (key != 0)
		goto asco;

	color_abc(VERDE_OSCURO);

	lee_dibujos("his1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS1, 180, 180);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);
	key = getscan();
	if (key != 0)
		goto asco;

	borra_pantalla();
	lee_dibujos("his2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS2, 180, 180);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);
	key = getscan();
	if (key != 0)
		goto asco;

	borra_pantalla();
	lee_dibujos("his3.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	centra_texto(HIS3, 180, 180);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);
	key = getscan();
	if (key != 0)
		goto asco;

	FundeAlNegro(1);

	borra_pantalla();
	lee_dibujos("his4_1.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("his4_2.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo3, dir_zona_pantalla);
	centra_texto(HIS1, 180, 180);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	key = getscan();
	if (key != 0)
		goto asco;

	_system->delayMillis(4);   
	key = getscan();
	if (key != 0)
		goto asco;

	for (l = 1; l < 200; l++) {
		DIBUJA_FONDO(0, 0, 0, l, 320, 200 - l, dir_dibujo3, dir_zona_pantalla);
		DIBUJA_FONDO(0, 200 - l, 0, 0, 320, l, dir_dibujo1, dir_zona_pantalla);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		key = getscan();
		if (key != 0)
			goto asco;
	}

	pausa(5);
	FundeAlNegro(2);
	borra_pantalla();

asco:
	lee_dibujos(pantalla_disco);
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos(num_room);
	descomprime_dibujo(dir_dibujo1, MEDIA);
	Negro();
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	if (musica_room != 0)
		playmusic(musica_room);  
	else
		stopmusic();
}

void DrasculaEngine::animacion_17() {
	habla_borracho(TEXTB13, "B13.als");
	habla_borracho(TEXTB14, "B14.als");
	flags[40] = 1;
}

void DrasculaEngine::animacion_19() {
	habla_vbpuerta(TEXTVB5, "VB5.als");
}

void DrasculaEngine::animacion_20() {
	habla_vbpuerta(TEXTVB7, "VB7.als");
	habla_vbpuerta(TEXTVB8, "VB8.als");
	hablar(TEXT383, "383.als");
	habla_vbpuerta(TEXTVB9, "VB9.als");
	hablar(TEXT384, "384.als");
	habla_vbpuerta(TEXTVB10, "VB10.als");
	hablar(TEXT385, "385.als");
	habla_vbpuerta(TEXTVB11, "VB11.als");
	if (flags[23] == 0) {
		hablar(TEXT350, "350.als");
		habla_vbpuerta(TEXTVB57, "VB57.als");
	} else {
		hablar(TEXT386, "386.als");
		habla_vbpuerta(TEXTVB12, "VB12.als");
		flags[18] = 0;               
		flags[14] = 1;
		abre_puerta(15, 1);
		sal_de_la_habitacion(1);
		animacion_23();
		sal_de_la_habitacion(0);
		flags[21] = 0;
		flags[24] = 0;
		sentido_vb = 1;
		vb_x = 120;

		rompo_y_salgo = 1;
	}
}

void DrasculaEngine::animacion_21() {
	habla_vbpuerta(TEXTVB6, "VB6.als");
}

void DrasculaEngine::animacion_23() {
	lee_dibujos("an24.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	flags[21] = 1;

	if (flags[25] == 0) { 
		habla_vb(TEXTVB13, "VB13.als");
		habla_vb(TEXTVB14, "VB14.als");
		pausa(10);
		hablar(TEXT387, "387.als");
	}

	habla_vb(TEXTVB15, "VB15.als");
	lleva_vb(42);        
	sentido_vb = 1;
	habla_vb(TEXTVB16, "VB16.als");
	sentido_vb = 2;
	lleva_al_hare(157, 147);
	lleva_al_hare(131, 149);
	sentido_hare = 0;
	animacion_14();
	if (flags[25] == 0)
		habla_vb(TEXTVB17, "VB17.als");
	pausa(8);
	sentido_vb = 1;
	habla_vb(TEXTVB18, "VB18.als");

	if (flags[29] == 0)
		animacion_23_anexo();
	else
		animacion_23_anexo2();

	sentido_vb = 2;
	animacion_25();
	lleva_vb(99);

	if (flags[29] == 0) {        
		habla_vb(TEXTVB19, "VB19.als");
		if (flags[25] == 0) {
			habla_vb(TEXTVB20,"VB20.als");
			if (resta_objeto(7) == 0)
				flags[30] = 1;
			if (resta_objeto(18) == 0)
				flags[31] = 1;
			if (resta_objeto(19) == 0)
				flags[32] = 1;
		}
		habla_vb(TEXTVB21, "VB21.als");
	} else
		animacion_27();

	flags[25] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animacion_23_anexo() {
	int n, p_x = hare_x + 2, p_y = hare_y - 3;
	int x[] = {1, 38, 75, 112, 75, 112, 75, 112, 149, 112, 149, 112, 149, 186, 223, 260,
				1, 38, 75, 112, 149, 112, 149, 112, 149, 112, 149, 186, 223, 260, 260, 260, 260, 223};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 76, 76, 76, 76, 76, 76, 76,
				76, 76, 76, 76, 76, 76, 76, 1, 1, 1, 1};

	lee_dibujos("an23.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 34; n++) {
		DIBUJA_BLOQUE(p_x, p_y, p_x, p_y, 36, 74, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x[n], y[n], p_x, p_y, 36, 74, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		VUELCA_PANTALLA(p_x, p_y, p_x, p_y, 36, 74, dir_zona_pantalla);
		pausa(5);
	}

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animacion_23_anexo2() {
	int n, p_x = hare_x + 4, p_y = hare_y;
	int x[] = {1, 35, 69, 103, 137, 171, 205, 239, 273, 1, 35, 69, 103, 137};
	int y[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 73, 73, 73, 73, 73};

	pausa(50);

	lee_dibujos("an23_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 14; n++) {
		DIBUJA_BLOQUE(p_x, p_y, p_x, p_y, 33, 71, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x[n], y[n], p_x, p_y, 33, 71, dir_hare_fondo, dir_zona_pantalla);
		actualiza_refresco();
		VUELCA_PANTALLA(p_x,p_y, p_x,p_y, 33,71, dir_zona_pantalla);
		pausa(5);
	}

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo,1);
}

void DrasculaEngine::animacion_25() {
	int n, pos_cabina[6];

	lee_dibujos("an14_2.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("18.alg");
	descomprime_dibujo(dir_dibujo1, 1);
	  
	pos_cabina[0] = 150;
	pos_cabina[1] = 6;
	pos_cabina[2] = 69;
	pos_cabina[3] = 0;
	pos_cabina[4] = 158;
	pos_cabina[5] = 161;

	flags[24] = 0;
	  
	comienza_sound("s6.als");
	  
	for (n = 0; n >= -160; n = n - 8) {
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

		actualiza_refresco_antes();
		pon_hare();
		pon_vb();

		pos_cabina[3] = n;

		DIBUJA_BLOQUE_CUT(pos_cabina, dir_hare_fondo, dir_zona_pantalla);

		actualiza_refresco();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	}
	  
	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animacion_27() {        
	flags[22] = 1;

	sin_verbo();
	resta_objeto(23);
	suma_objeto(11);

	habla_vb(TEXTVB23, "VB23.als");
	habla_vb(TEXTVB24, "VB24.als");
	if (flags[30] == 1)
		suma_objeto(7);
	if (flags[31] == 1)
		suma_objeto(18);
	if (flags[32] == 1)
		suma_objeto(19);
	habla_vb(TEXTVB25,"VB25.als");
	habla_vb(TEXTVB26,"VB26.als");
}

void DrasculaEngine::animacion_28(){        
	habla_vb(TEXTVB27, "VB27.als");
	habla_vb(TEXTVB28, "VB28.als");
	habla_vb(TEXTVB29, "VB29.als");
	habla_vb(TEXTVB30, "VB30.als");
}

void DrasculaEngine::animacion_29(){        
	if (flags[33] == 0) {
		habla_vb(TEXTVB32, "VB32.als");
		hablar(TEXT398, "398.als");
		habla_vb(TEXTVB33, "VB33.als");
		hablar(TEXT399, "399.als");
		habla_vb(TEXTVB34, "VB34.als");
		habla_vb(TEXTVB35, "VB35.als");
		hablar(TEXT400, "400.als");
		habla_vb(TEXTVB36, "VB36.als");
		habla_vb(TEXTVB37, "VB37.als");
		hablar(TEXT386, "386.als");
		habla_vb(TEXTVB38, "VB38.als");
		habla_vb(TEXTVB39, "VB39.als");
		hablar(TEXT401, "401.als");
		habla_vb(TEXTVB40, "VB40.als");
		habla_vb(TEXTVB41, "VB41.als");
		flags[33] = 1;
	} else
		habla_vb(TEXTVB43, "VB43.als");

	hablar(TEXT402, "402.als");
	habla_vb(TEXTVB42, "VB42.als");

	if (flags[38] == 0) {
		hablar(TEXT403, "403.als");
		rompo_y_salgo = 1;
	} else
		hablar(TEXT386, "386.als");
}

void DrasculaEngine::animacion_30(){        
	habla_vb(TEXTVB31, "VB31.als");
	hablar(TEXT396, "396.als");
}

void DrasculaEngine::animacion_31(){        
	habla_vb(TEXTVB44, "VB44.als");
	lleva_vb(-50);
	pausa(15);
	lleva_al_hare(159, 140);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sentido_hare = 2;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(78);
	sentido_hare = 0;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(22);
	hablar(TEXT406, "406.als");
	lleva_vb(98);
	habla_vb(TEXTVB45, "VB45.als");
	habla_vb(TEXTVB46, "VB46.als"); 
	habla_vb(TEXTVB47, "VB47.als");
	hablar(TEXT407, "407.als");
	habla_vb(TEXTVB48, "VB48.als");
	habla_vb(TEXTVB49, "VB49.als"); 
	hablar(TEXT408, "408.als");
	habla_vb(TEXTVB50, "VB50.als");
	habla_vb(TEXTVB51, "VB51.als");
	hablar(TEXT409, "409.als");
	habla_vb(TEXTVB52, "VB52.als");
	habla_vb(TEXTVB53, "VB53.als");
	pausa(12);
	habla_vb(TEXTVB54, "VB54.als");
	habla_vb(TEXTVB55, "VB55.als");
	hablar(TEXT410, "410.als");
	habla_vb(TEXTVB56, "VB56.als");

	rompo_y_salgo = 1;

	flags[38] = 0;
	flags[36] = 1;
	sin_verbo();
	resta_objeto(8);
	resta_objeto(13);
	resta_objeto(15);
	resta_objeto(16);
	resta_objeto(17);
	suma_objeto(20);
}

void DrasculaEngine::animacion_35() {
	int n, x = 0;

	lleva_al_hare(96, 165);
	lleva_al_hare(79, 165);

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("an35_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("an35_2.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 1, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(70,90, 70,90, 46,80,dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 82, 70, 90, 46, 80, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(70, 90, 70, 90, 46, 80, dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	x = 0;
	for (n = 0; n < 6; n++) {
		x++;
		DIBUJA_FONDO(x, 1, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(70, 90, 70, 90, 46, 80, dir_zona_pantalla);

		x = x + 46;

		pausa(3);
	}

	x = 0;
	for (n = 0; n < 2; n++) {
		x++;
		DIBUJA_FONDO(x, 82, 70, 90, 46, 80, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(70, 90, 70,90, 46, 80,dir_zona_pantalla);
		x = x + 46;
		pausa(3);
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(19);

	comienza_sound("s1.als");
	hipo_sin_nadie(18);
	fin_sound();

	pausa(10);

	FundeAlNegro(2);

	// TODO
	error("hit part 3");
}

void DrasculaEngine::habla_vb(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[6] = {1, 27, 53, 79, 105, 131};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(VON_BRAUN);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

	DIBUJA_FONDO(vb_x + 5, 64, ANCHOBJ + 1, 0, 25, 27, dir_dibujo1, dir_dibujo3);

bucless:

	if (sentido_vb == 1) {
		cara = _rnd->getRandomNumber(5);
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

		pon_hare();
		pon_vb();

		DIBUJA_FONDO(ANCHOBJ + 1, 0, vb_x + 5, 64, 25, 27, dir_dibujo3, dir_zona_pantalla);
		DIBUJA_BLOQUE(x_habla[cara], 34, vb_x + 5, 64, 25, 27, dir_hare_frente, dir_zona_pantalla);
		actualiza_refresco();
	}

	if (con_voces == 0)
		centra_texto(dicho, vb_x, 66);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (music_status() == 0 && flags[11] == 0 && musica_room != 0)
		playmusic(musica_room);
}

void DrasculaEngine::habla_vbpuerta(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(VON_BRAUN);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	refresca_pantalla();
	if (con_voces == 0)
		centra_texto(dicho, 150, 80);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (music_status() == 0 && flags[11] == 0 && musica_room != 0)
		playmusic(musica_room);
}

void DrasculaEngine::habla_ciego(const char *dicho, const char *filename, const char *sincronia) {
	byte *num_cara;
	int p = 0;
	int pos_ciego[6];
	int cara = 0;

	int longitud;
	longitud = strlen(dicho);

	buffer_teclado();

	color_abc(VON_BRAUN);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

	pos_ciego[1] = 2;
	pos_ciego[2] = 73;
	pos_ciego[3] = 1;
	pos_ciego[4] = 126;
	pos_ciego[5] = 149;

bucless:
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	pos_ciego[5] = 149;
	char c = toupper(sincronia[p]);
	if (c == '0') 
		cara = 0;
	if (c == '1')
		cara = 1;
	if (c == '2')
		cara = 2;
	if (c == '3')
		cara = 3;
	if (c == '4')
		cara = 4;
	if (c == '5')
		cara = 5;
	if (c == '6')
		cara = 6;
	if (c == '7')
		cara = 7;

	if (cara == 0 || cara == 2 || cara == 4 || cara == 6)
		pos_ciego[0] = 1;
	else
		pos_ciego[0] = 132;

	if (cara == 0)
		num_cara = dir_dibujo3;
	else if (cara == 1)
		num_cara = dir_dibujo3;
	else if (cara == 2)
		num_cara = dir_hare_dch;
	else if (cara == 3)
		num_cara = dir_hare_dch;
	else if (cara == 4)
		num_cara = dir_hare_fondo;
	else if (cara == 5)
		num_cara = dir_hare_fondo;
	else {
		num_cara = dir_hare_frente;
		pos_ciego[5] = 146;
	}

	DIBUJA_BLOQUE_CUT( pos_ciego, num_cara, dir_zona_pantalla);

	if (con_voces == 0)
		centra_texto(dicho, 310, 71);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(2);
	p++;

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}
}

void DrasculaEngine::habla_hacker(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	
	color_abc(AMARILLO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:
	if (con_voces == 0)
		centra_texto(dicho, 156, 170);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	key = 0;
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

    DIBUJA_BLOQUE_CUT(pos_vb, dir_hare_frente, dir_zona_pantalla);
}

void DrasculaEngine::lleva_vb(int punto_x) {
	if (punto_x < vb_x)
		sentido_vb = 0;
	else
		sentido_vb = 1;

	vb_se_mueve = 1;

	for (;;) {
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if (sentido_vb == 0) {
			vb_x = vb_x - 5;
			if (vb_x <= punto_x)
				break;
		} else {
			vb_x = vb_x + 5;
			if (vb_x >= punto_x)
				break;
		}
		pausa(5);
	}

    vb_se_mueve = 0;
}

void DrasculaEngine::hipo_sin_nadie(int contador){
	int y = 0, sentido = 0;
	if (num_ejec == 3)
		y = -1;

comienza:
	contador--;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	if (num_ejec <= 2)
		VUELCA_PANTALLA(0, 1, 0, y, 320, 198, dir_zona_pantalla);
	else
		VUELCA_PANTALLA(0, 0, 0, y, 320, 200, dir_zona_pantalla);

	if (sentido == 0)
		y++;
	else
		y--;
	if (num_ejec <= 2) {
		if (y == 2)
			sentido = 1;
		if (y == 0)
			sentido = 0;
	} else if (num_ejec == 3) {
		if (y == 1)
			sentido = 1;
		if (y == -1)
			sentido = 0;
	}
	if (contador > 0)
		goto comienza;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::abre_puerta(int nflag, int n_puerta) {
	if (flags[nflag] == 0) {
		if (num_ejec == 5 || num_ejec == 6) {
			comienza_sound("s3.als");
			flags[nflag] = 1;
		}
		if (num_ejec == 1 && nflag == 7) 
			return;
		comienza_sound("s3.als");
		flags[nflag] = 1;
		if (n_puerta != NO_PUERTA)
			puertas_cerradas(n_puerta);
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		fin_sound();
		if (num_ejec != 5)
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

void DrasculaEngine::refresca_1_antes() {
	int cambio_col_antes = cambio_de_color;

	if (hare_x > 98 && hare_x < 153)
		cambio_de_color = 1;
	else
		cambio_de_color = 0;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();

	if (flags[8] == 0)
		DIBUJA_FONDO(2, 158, 208, 67, 27, 40, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_2(){
	int pos_murci[6];
	int diferencia;
	int murci_x[] = {0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190,
					0, 48, 96, 144, 192, 240,
					30, 88, 146, 204, 262,
					88, 146, 204, 262,
					88, 146, 204, 262};

	int murci_y[] = {179, 179, 179, 179, 179, 179, 179, 179,
					158, 158, 158, 158, 158, 158, 158, 158,
					137, 137, 137, 137, 137, 137,
					115, 115, 115, 115, 115, 115,
					78, 78, 78, 78, 78,
					41, 41, 41, 41,
					4, 4, 4, 4};

	if (frame_murcielago == 41)
		frame_murcielago = 0;

	pos_murci[0] = murci_x[frame_murcielago];
	pos_murci[1] = murci_y[frame_murcielago];

	if (frame_murcielago < 22) {
		pos_murci[4] = 37;
		pos_murci[5] = 21;
	} else if (frame_murcielago > 27) {
		pos_murci[4] = 57;
		pos_murci[5] = 36;
	} else {
		pos_murci[4] = 47;
		pos_murci[5] = 22;
	}

	pos_murci[2] = 239;
	pos_murci[3] = 19;

	DIBUJA_BLOQUE_CUT(pos_murci, dir_dibujo3, dir_zona_pantalla);
	diferencia = (int)vez() - conta_ciego_vez;
	if (diferencia >= 6) {
		frame_murcielago++;
		conta_ciego_vez = (int)vez();
	}

	DIBUJA_BLOQUE(29, 37, 58, 114, 57, 39, dir_dibujo3, dir_zona_pantalla);
	mapa();
}

void DrasculaEngine::refresca_3_antes() {
	if (flags[3] == 1)
		DIBUJA_FONDO(258, 110, 85, 44, 23, 53, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_3() {
	if (hare_y + alto_hare < 118)
		DIBUJA_BLOQUE(129, 110, 194, 36, 126, 88, dir_dibujo3, dir_zona_pantalla);
	DIBUJA_BLOQUE(47, 57, 277, 143, 43, 50, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_4() {
	int cambio_col_antes = cambio_de_color;
	if (hare_x > 190)
		cambio_de_color = 1;
	else
		cambio_de_color = 0;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();
}

void DrasculaEngine::refresca_5() {
	DIBUJA_BLOQUE(114, 130, 211, 87, 109, 69, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_15() {
	DIBUJA_BLOQUE(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_17() {
	DIBUJA_BLOQUE(48, 135, 78, 139, 80, 30, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_18() {
	if (flags[24] == 1)
		DIBUJA_BLOQUE(177, 1, 69, 29, 142, 130, dir_dibujo3, dir_zona_pantalla);
	DIBUJA_BLOQUE(105, 132, 109, 108, 196, 65, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_5_antes(){
	if (flags[8] == 0)
		DIBUJA_FONDO(256, 152, 208, 67, 27, 40, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_6_antes() {
	int cambio_col_antes = cambio_de_color;

	if ((hare_x > 149 && hare_y + alto_hare > 160 && hare_x < 220 && hare_y + alto_hare < 188) ||
			(hare_x > 75 && hare_y + alto_hare > 183 && hare_x < 145))
		cambio_de_color = 0;
	else
		cambio_de_color=1;

	if (cambio_col_antes != cambio_de_color && cambio_de_color == 1)
		hare_oscuro();
	if (cambio_col_antes != cambio_de_color && cambio_de_color == 0)
		hare_claro();

	if (flags[1] == 0)
		DIBUJA_FONDO(97, 117, 34, 148, 36, 31, dir_dibujo3, dir_zona_pantalla);
	if (flags[0] == 0)
		DIBUJA_FONDO(3, 103, 185, 69, 23, 76, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		DIBUJA_FONDO(28, 100, 219, 72, 64, 97, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_7_antes() {
	if (flags[35] == 0)
		DIBUJA_FONDO(1, 72, 158, 162, 19, 12, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_9_antes() {
	int ciego_x[] = {26, 68, 110, 152, 194, 236, 278, 26, 68};
	int ciego_y[] = {51, 51, 51, 51, 51, 51, 51, 127, 127};
	int diferencia;

	DIBUJA_BLOQUE(ciego_x[frame_ciego], ciego_y[frame_ciego], 122, 57, 41, 72, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 0) {
		diferencia = (int)vez() - conta_ciego_vez;
		if (diferencia >= 11) {
			frame_ciego++;
			conta_ciego_vez = (int)vez();
		}
		if (frame_ciego == 9)
			frame_ciego = 0;
	} else
		frame_ciego=3;
}

void DrasculaEngine::refresca_12_antes() {
	if (flags[16] == 0)
		DIBUJA_FONDO(1, 131, 106, 117, 55, 68, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_14_antes() {
	int velas_y[] = {158, 172, 186};
	int cirio_x[] = {14, 19, 24};
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31};
	int borracho_x[] = {1, 42, 83, 124, 165, 206, 247, 1};
	int diferencia;

	DIBUJA_FONDO(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);
	DIBUJA_FONDO(cirio_x[frame_velas], 146, 311, 80, 4, 8, dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5)
		DIBUJA_FONDO(1, 149, 127, 52, 9, 5, dir_dibujo3, dir_zona_pantalla);
	if (hare_x > 101 && hare_x < 155)
		DIBUJA_FONDO(31, 138, 178, 51, 18, 16, dir_dibujo3, dir_zona_pantalla);
	if (flags[11] == 0)
		DIBUJA_FONDO(pianista_x[frame_piano], 157, 245, 130, 29, 42, dir_dibujo3, dir_zona_pantalla);
	else if (flags[5] == 0)
		DIBUJA_FONDO(145, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	else
		DIBUJA_FONDO(165, 140, 229, 117, 43, 59, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		DIBUJA_FONDO(borracho_x[frame_borracho], 82, 170, 50, 40, 53, dir_dibujo3, dir_zona_pantalla);
	diferencia = (int)vez() - conta_ciego_vez;
	if (diferencia > 6) {
		if (flags[12] == 1) {
			frame_borracho++;
			if (frame_borracho == 8) {
				frame_borracho = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		parpadeo = _rnd->getRandomNumber(10);
		conta_ciego_vez = (int)vez();
	}
}

void DrasculaEngine::refresca_16_antes() {
	if (flags[17] == 0)
		DIBUJA_FONDO(1, 103, 24, 72, 33, 95, dir_dibujo3, dir_zona_pantalla);
	if (flags[19] == 1)
		DIBUJA_FONDO(37, 151, 224, 115, 56, 47, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_17_antes() {
	if (flags[15] == 1)
		DIBUJA_FONDO(1, 135, 108, 65, 44, 63, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_18_antes() {
	int diferencia;
	int ronquido_x[] = {95, 136, 95, 136, 95, 95, 95, 95, 136, 95, 95, 95, 95, 95, 95, 95};
	int ronquido_y[] = {18, 18, 56, 56, 94, 94, 94, 94, 94, 18, 18, 18, 18, 18, 18, 18};

	if (flags[21] == 0) {
		DIBUJA_FONDO(1, 69, 120, 58, 56, 61, dir_dibujo3, dir_zona_pantalla);
		DIBUJA_FONDO(ronquido_x[frame_ronquido], ronquido_y[frame_ronquido], 124, 59, 40, 37, dir_dibujo3, dir_zona_pantalla);
	} else
		pon_vb();

	diferencia = (int)vez() - conta_ciego_vez;
	if (diferencia > 9) {
		frame_ronquido++;
		if (frame_ronquido == 16)
			frame_ronquido = 0;
		conta_ciego_vez = (int)vez();
	}
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

	refresca_pantalla();
	DIBUJA_FONDO(253, 110, 150, 65, 20, 30, dir_dibujo3, dir_zona_pantalla);

	if (con_voces == 0)
		centra_texto(".groaaarrrrgghhh!", 153, 65);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

bucless:
	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::pantalla_13(int fl) {
	if (objeto_que_lleva == MIRAR && fl == 51) {
		hablar(TEXT411, "411.als");
		sentido_hare = 3;
		hablar(TEXT412, "412.als");
		strcpy(nombre_obj[1], "yoda");
	} else if (objeto_que_lleva == HABLAR && fl == 51)
		conversa("op_7.cal");
	else if (objeto_que_lleva == 19 && fl == 51)
		animacion_1_3();
	else if (objeto_que_lleva == 9 && fl == 51)
		animacion_2_3();
	else
		hay_respuesta = 0;
}

void DrasculaEngine::refresca_13() {
	if (hare_x > 55 && flags[3] == 0)
		animacion_6();
	if (flags[1] == 0)
		DIBUJA_BLOQUE(185, 110, 121, 65, 67, 88, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		DIBUJA_BLOQUE(185, 21, 121, 63, 67, 88, dir_dibujo3, dir_zona_pantalla);
	DIBUJA_BLOQUE(3, 127, 99, 102, 181, 71, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_20() {
	DIBUJA_BLOQUE(1, 137, 106, 121, 213, 61, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::animacion_1_3() {
	hablar(TEXT413, "413.als");
	grr();
	pausa(50);
	hablar(TEXT414, "414.als");
}

void DrasculaEngine::animacion_2_3() {
	flags[0] = 1;
	playmusic(13);
	animacion_3_3();
	playmusic(13);
	animacion_4_3();
	flags[1] = 1;
	refresca_pantalla();
	VUELCA_PANTALLA(120, 0, 120, 0, 200, 200, dir_zona_pantalla);
	animacion_5();
	flags[0] = 0;
	flags[1] = 1;

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	lleva_al_hare(332, 127);
	libera_memoria();
	// TODO
	error("4 segment hit");
}

void DrasculaEngine::animacion_3_3() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	lee_dibujos("an2y_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an2y_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an2y_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}
}

void DrasculaEngine::animacion_4_3() {
	int n, x = 0;
	int px = 120, py = 63;

	lee_dibujos("any_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("any_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("any_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++){
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 1, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 91, px, py, 77, 89, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 1, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

    x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 91, px, py, 77, 89, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 1, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 77, 89, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 91, px, py, 77, 89, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 77, 89, dir_zona_pantalla);
		x = x + 77;
		pausa(3);
	}
}

void DrasculaEngine::animacion_5() {
	int n, x = 0;
	int px = hare_x - 20, py = hare_y - 1;

	lee_dibujos("an3y_1.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("an3y_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an3y_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(px,py, px,py, 71,72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 2, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}

	x = 0;

	for (n = 0; n < 4; n++) {
		x++;
		DIBUJA_FONDO(px, py, px, py, 71, 72, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(x, 75, px, py, 71, 72, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 71, 72, dir_zona_pantalla);
		x = x + 71;
		pausa(3);
	}
}

void DrasculaEngine::animacion_6() {
	int frame = 0, px = 112, py = 62;
	int yoda_x[] = { 3 ,82, 161, 240, 3, 82 };
	int yoda_y[] = { 3, 3, 3, 3, 94, 94 };

	hare_se_mueve = 0;
	flags[3] = 1;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	flags[1] = 0;

	lee_dibujos("an4y.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (frame = 0; frame < 6; frame++) {
		pausa(3);
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(yoda_x[frame], yoda_y[frame], px, py,	78, 90,	dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(px, py, px, py, 78, 90, dir_zona_pantalla);
	}

	flags[2] = 1;

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::animacion_rayo() {
	lee_dibujos("anr_1.alg");
	descomprime_dibujo(dir_hare_frente, MEDIA);
	lee_dibujos("anr_2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("anr_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("anr_4.alg");
	descomprime_dibujo(dir_dibujo1, 1);
	lee_dibujos("anr_5.alg");
	descomprime_dibujo(dir_dibujo3, 1);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_hare_frente);

	pausa(50);

	comienza_sound("s5.als");

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_hare_dch);
	pausa(3);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pausa(3);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pausa(3);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_hare_fondo);
	pausa(3);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_dibujo3);
	pausa(3);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_hare_frente);
	fin_sound();
}

void DrasculaEngine::animacion_2_4() {
	habla_igor_sentado(TEXTI16, "I16.als");
	hablar(TEXT278, "278.als");
	habla_igor_sentado(TEXTI17, "I17.als");
	hablar(TEXT279, "279.als");
	habla_igor_sentado(TEXTI18, "I18.als");
}

void DrasculaEngine::animacion_3_4() {
	habla_igor_sentado(TEXTI19, "I19.als");
	habla_igor_sentado(TEXTI20, "I20.als");
	hablar(TEXT281, "281.als");
}

void DrasculaEngine::animacion_4_4() {
	hablar(TEXT287, "287.als");
	habla_igor_sentado(TEXTI21, "I21.als");
	hablar(TEXT284, "284.als");
	habla_igor_sentado(TEXTI22, "I22.als");
	hablar(TEXT285, "285.als");
	habla_igor_sentado(TEXTI23, "I23.als");
}

void DrasculaEngine::habla_igor_sentado(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[4] = { 80, 102, 124, 146 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(3);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 109, 207, 92, 21, 23, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 221, 102);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::animacion_7_4() {
	Negro();
	hablar(TEXT427, "427.als");
	FundeDelNegro(1);
	resta_objeto(8);
	resta_objeto(10);
	resta_objeto(12);
	resta_objeto(16);
	suma_objeto(17);
	flags[30] = 0;
	flags[29] = 0;
}

void DrasculaEngine::animacion_1_5() {
	if (flags[0] == 0) {
		hablar(TEXT430, "430.als");
		habla_bj(TEXTBJ16, "BJ16.als");
		habla_bj(TEXTBJ17, "BJ17.als");
		habla_bj(TEXTBJ18, "BJ18.als");
		hablar(TEXT217, "217.als");
		habla_bj(TEXTBJ19, "BJ19.als");
		hablar(TEXT229, "229.als");
		pausa(5);
		lleva_al_hare(114, 170);
		sentido_hare = 3;
		hablar(TEXT431, "431.als");
		habla_bj(TEXTBJ20, "BJ20.als");
		sentido_hare = 2;
		pausa(4);
		hablar(TEXT438, "438.als");
		sitio_x = 120;
		sitio_y = 157;
		anda_a_objeto = 1;
		sentido_final = 1;
		empieza_andar();
		habla_bj(TEXTBJ21, "BJ21.als");

		for (;;) {
			if (hare_se_mueve == 0)
				break;
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		sentido_hare = 1;
		hablar(TEXT229, "229.als");
		flags[0] = 1;
	}

	sentido_hare = 1;
	conversa("op_8.cal");
}

void DrasculaEngine::animacion_2_5() {
	habla_bj(TEXTBJ22, "BJ22.als");
}

void DrasculaEngine::animacion_3_5() {
	habla_bj(TEXTBJ23, "BJ23.als");
	agarra_objeto(10);
	rompo_y_salgo = 1;
}

void DrasculaEngine::animacion_4_5() {
	flags[7] = 1;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	hablar(TEXT228, "228.als");
	habla_lobo(TEXTL1, "L1.als");
	habla_lobo(TEXTL2, "L2.als");
	pausa(23);
	hablar(TEXT229, "229.als");
	habla_lobo(TEXTL3, "L3.als");
	habla_lobo(TEXTL4, "L4.als");
	hablar(TEXT230, "230.als");
	habla_lobo(TEXTL5, "L5.als");
	hablar(TEXT231, "231.als");
	habla_lobo(TEXTL6, "L6.als");
	habla_lobo(TEXTL7, "L7.als");
	pausa(33);
	hablar(TEXT232, "232.als");
	habla_lobo(TEXTL8, "L8.als");
}

void DrasculaEngine::animacion_5_5(){
	int h;
	int frame = 0;
	int hueso_x[] = {1, 99, 197, 1, 99, 197, 1, 99, 197};
	int hueso_y[] = {1, 1, 1, 66, 66, 66, 131, 131, 131};
	int vuela_x[] = {1, 63, 125, 187, 249};
	int pixel_x = hare_x - 53, pixel_y = hare_y - 9;

	sin_verbo();
	resta_objeto(8);

	lleva_al_hare(hare_x - 19, hare_y + alto_hare);
	sentido_hare = 1;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("3an5_1.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("3an5_2.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	for (frame = 0; frame < 9; frame++) {
		pausa(3);
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(pixel_x, pixel_y, pixel_x,pixel_y, 97,64, dir_zona_pantalla);
	}

	DIBUJA_FONDO(52, 161, 198, 81, 26, 24, dir_dibujo3, dir_zona_pantalla);
	VUELCA_PANTALLA(198, 81, 198, 81, 26, 24, dir_zona_pantalla);

	for (frame = 0; frame < 9; frame++) {
		pausa(3);
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(hueso_x[frame], hueso_y[frame], pixel_x, pixel_y, 97, 64, dir_hare_frente, dir_zona_pantalla);
		VUELCA_PANTALLA(pixel_x, pixel_y, pixel_x,pixel_y, 97, 64, dir_zona_pantalla);
	}

	flags[6] = 1;
	actualiza_datos();
	pausa(12);

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	for (h = 0; h < (200 - 18); h++)
		DIBUJA_FONDO(0, 53, 0, h, 320, 19, dir_hare_frente,  dir_zona_pantalla);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	lee_dibujos("101.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("3an5_3.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("3an5_4.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_dibujo1);
	pausa(9);
	for (frame = 0; frame < 5; frame++) {
		pausa(3);
		DIBUJA_FONDO(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	for (frame = 0; frame < 5; frame++) {
		pausa(3);
		DIBUJA_FONDO(vuela_x[frame], 1, 174, 79, 61, 109, dir_hare_dch, dir_zona_pantalla);
		VUELCA_PANTALLA(174, 79, 174, 79, 61, 109, dir_zona_pantalla);
	}
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_dibujo1);

	comienza_sound("s1.als");
	fin_sound();

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	borra_pantalla();

	lee_dibujos("49.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
}

void DrasculaEngine::animacion_6_5() {
	habla_lobo(TEXTL9, "L9.als");
	hablar(TEXT234, "234.als");
}

void DrasculaEngine::animacion_7_5() {
	habla_lobo(TEXTL10, "L10.als");
	hablar(TEXT236, "236.als");
	habla_lobo(TEXTL11, "L11.als");
	habla_lobo(TEXTL12, "L12.als");
	habla_lobo(TEXTL13, "L13.als");
	pausa(34);
	habla_lobo(TEXTL14, "L14.als");
}

void DrasculaEngine::animacion_8_5() {
	habla_lobo(TEXTL15, "L15.als");
	hablar(TEXT238, "238.als");
	habla_lobo(TEXTL16, "L16.als");
}

void DrasculaEngine::animacion_9_5() {
	flags[4] = 1;
	hablar(TEXT401, "401.als");
	sin_verbo();
	resta_objeto(15);
}

void DrasculaEngine::animacion_10_5() {
	flags[3] = 1;
	hablar(TEXT401, "401.als");
	sin_verbo();
	resta_objeto(12);
}

void DrasculaEngine::animacion_11_5() {
	flags[9] = 1;
	if (flags[2] == 1 && flags[3] == 1 && flags[4] == 1)
		animacion_12_5();
	else {
		flags[9] = 0;
		hablar(TEXT33, "33.als");
	}
}

void DrasculaEngine::animacion_12_5() {
	DacPalette256 palFondo1;
	DacPalette256 palFondo2;
	DacPalette256 palFondo3;

	int frame;
	const int rayo_x[] = {1, 46, 91, 136, 181, 226, 271, 181};
	const int frusky_x[] = {100, 139, 178, 217, 100, 178, 217, 139, 100, 139};
	const int elfrusky_x[] = {1, 68, 135, 1, 68, 135, 1, 68, 135, 68, 1, 135, 68, 135, 68};
	//const int humo_x[] = {1, 29, 57, 85, 113, 141, 169, 197, 225};
	int color, componente;
	char fundido;

	playmusic(26);
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(27);
	anima("rayo1.bin", 23);
	comienza_sound("s5.als");
	anima("rayo2.bin", 17);
	sentido_hare = 1;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	hare_oscuro();

	for (color = 0; color < 255; color++)
		for (componente = 0; componente < 3; componente++) {
			palFondo1[color][componente] = palJuego[color][componente];
			palFondo2[color][componente] = palJuego[color][componente];
			palFondo3[color][componente] = palJuego[color][componente];
		}

		for (fundido = 1; fundido >= 0; fundido--) {
			for (color = 0; color < 128; color++)
				for (componente = 0; componente < 3; componente++)
					palFondo1[color][componente] = LimitaVGA(palFondo1[color][componente] - 8 + fundido);
		}

		for (fundido = 2; fundido >= 0; fundido--) {
			for (color = 0; color < 128; color++)
				for (componente = 0; componente < 3; componente++)
					palFondo2[color][componente] = LimitaVGA(palFondo2[color][componente] - 8 + fundido);
		}

		for (fundido = 3; fundido >= 0; fundido--) {
			for (color = 0; color < 128; color++)
				for (componente = 0; componente < 3; componente++)
					palFondo3[color][componente] = LimitaVGA(palFondo3[color][componente] - 8 + fundido);
		}

		lee_dibujos("3an11_1.alg");
		descomprime_dibujo(dir_hare_fondo, 1);

		for (frame = 0; frame < 8; frame++) {
			if (frame == 2 || frame == 4 || frame == 8 || frame==10)
				setvgapalette256((byte *)&palFondo1);
			else if (frame == 1 || frame == 5 || frame == 7 || frame == 9)
				setvgapalette256((byte *)&palFondo2);
			else
				setvgapalette256((byte *)&palFondo3);

			pausa(4);
			refresca_pantalla();
			DIBUJA_BLOQUE(rayo_x[frame], 1, 41, 0, 44, 44, dir_hare_fondo, dir_zona_pantalla);
			DIBUJA_BLOQUE(frusky_x[frame], 113, 205, 50, 38, 86, dir_dibujo3, dir_zona_pantalla);
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		fin_sound_corte();

		for (frame = 0; frame < 15; frame++) {
			if (frame == 2 || frame == 4 || frame == 7 || frame == 9)
				setvgapalette256((byte *)&palFondo1);
			else if (frame == 1 || frame == 5)
				setvgapalette256((byte *)&palJuego);
			else
				setvgapalette256((byte *)&palFondo2);

			pausa(4);
			refresca_pantalla();
			DIBUJA_BLOQUE(elfrusky_x[frame], 47, 192, 39, 66, 106, dir_hare_fondo, dir_zona_pantalla);
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		}

		anima("frel.bin", 16);
		borra_pantalla();
		hare_claro();
		ActualizaPaleta();

		flags[1] = 1;

		animacion_13_5();
		comienza_sound("s1.als");
		hipo(12);
		fin_sound();

		lee_dibujos("99.alg");
		descomprime_dibujo(dir_hare_fondo, 1);

		lleva_al_hare(40, 169);
		lleva_al_hare(-14, 175);

		rompo = 1;
		musica_antes = musica_room;
		hare_se_ve = 1;
		borra_pantalla();
		sentido_hare = 1;
		hare_se_mueve = 0;
		hare_x = -1;
		obj_saliendo = 104;
		sin_verbo();
		carga_escoba_1("57.ald");
}

void DrasculaEngine::animacion_13_5() {
	int frank_x = 199;
	int frame = 0;
	int frus_x[] = {1, 46, 91, 136, 181, 226, 271};
	int frus_y[] = {1, 1, 1, 1, 1, 1, 1, 89};
	int pos_frusky[6];

	lee_dibujos("auxfr.alg");
	descomprime_dibujo(dir_hare_fondo, 1);

	pos_frusky[3] = 81;
	pos_frusky[4] = 44;
	pos_frusky[5] = 87;
	pos_frusky[0] = 1;
	pos_frusky[1] = 1;
	pos_frusky[2] = frank_x;
	refresca_pantalla();
	DIBUJA_BLOQUE_CUT(pos_frusky, dir_hare_fondo, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(15);

	playmusic(18);

	for (;;) {
		refresca_pantalla();
		pos_frusky[0] = frus_x[frame];
		pos_frusky[1] = frus_y[frame];
		pos_frusky[2] = frank_x;
		DIBUJA_BLOQUE_CUT( pos_frusky, dir_hare_fondo, dir_zona_pantalla);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		frank_x = frank_x - 5;
		frame++;
		if (frank_x <= -45)
			break;
		if (frame == 7) {
			frame = 0;
			sentido_hare = 3;
		}
		pausa(6);
	}
}

void DrasculaEngine::animacion_14_5() {
	flags[11] = 1;
	comienza_sound("s3.als");
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0,0 , 320, 200, dir_zona_pantalla);
	fin_sound();
	pausa(17);
	sentido_hare = 3;
	hablar(TEXT246,"246.als");
	lleva_al_hare(89, 160);
	flags[10] = 1;
	comienza_sound("s7.als");
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	pausa(14);
	sentido_hare = 3;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	habla_solo(TEXTD18, "d18.als");
	FundeAlNegro(1);
	error("part 6 hit");
}

void DrasculaEngine::animacion_15_5() {
	habla_mus(TEXTE4, "E4.als");
	habla_mus(TEXTE5, "E5.als");
	habla_mus(TEXTE6, "E6.als");
	hablar(TEXT291, "291.als");
	habla_mus(TEXTE7, "E7.als");
}

void DrasculaEngine::animacion_16_5() {
	habla_mus(TEXTE8, "E8.als");
}

void DrasculaEngine::animacion_17_5() {
	habla_mus(TEXTE9, "E9.als");
}

void DrasculaEngine::refresca_49_antes() {
	if (flags[6] == 0)
		DIBUJA_FONDO(2, 136, 176, 81, 49, 62, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_53_antes() {
	if (flags[1] == 0)
		DIBUJA_BLOQUE(2, 113, 205, 50, 38, 86, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0)
		DIBUJA_FONDO(41, 159, 27, 117, 25, 40, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 1)
		DIBUJA_FONDO(67, 184, 56, 93, 32, 15, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_54_antes() {
	if (flags[5] == 1)
		DIBUJA_FONDO(168, 156, 187, 111, 7, 11, dir_dibujo3, dir_zona_pantalla);
	if (flags[12] == 1)
		DIBUJA_FONDO(16, 156, 190, 64, 18, 24, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_56_antes() {
	if (flags[10] == 0)
		DIBUJA_FONDO(2, 126, 42, 67, 57, 67, dir_dibujo3, dir_zona_pantalla);
	if (flags[11] == 1)
		DIBUJA_FONDO(60, 160, 128, 97, 103, 38, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_50() {
	DIBUJA_BLOQUE(4, 153, 118, 95, 67, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_57() {
	DIBUJA_BLOQUE(7, 113, 166, 61, 62, 82, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::pantalla_49(int fl){
	if (objeto_que_lleva == HABLAR && fl ==51)
		conversa("op_9.cal");
	else if (objeto_que_lleva == MIRAR && fl == 51)
		hablar(TEXT132, "132.als");
	else if ((objeto_que_lleva == 8 && fl == 51) || (objeto_que_lleva == 8 && fl == 203))
		animacion_5_5();
	else if (objeto_que_lleva == MIRAR && fl == 200)
		hablar(TEXT133, "133.als");
	else if (objeto_que_lleva == HABLAR && fl == 200)
		hablar(TEXT134, "134.als");
	else if (objeto_que_lleva == MIRAR && fl == 201)
		hablar(TEXT135, "135.als");
	else if (objeto_que_lleva == MIRAR && fl == 203)
		hablar(TEXT137, "137.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_53(int fl) {
	if (objeto_que_lleva == COGER && fl == 120) {
		agarra_objeto(16);
		visible[3] = 0;
	} else if (objeto_que_lleva == MIRAR && fl == 121)
		hablar(TEXT128, "128.als");
	else if (objeto_que_lleva == MIRAR && fl == 209)
		hablar(TEXT129, "129.als");
	else if (objeto_que_lleva == MOVER && fl == 123)
		animacion_11_5();
	else if (objeto_que_lleva == MIRAR && fl == 52)
		hablar(TEXT447, "447.als");
	else if (objeto_que_lleva == HABLAR && fl == 52)
		hablar(TEXT131, "131.als");
	else if (objeto_que_lleva == 12 && fl == 52)
		animacion_10_5();
	else if (objeto_que_lleva == 15 && fl == 52)
		animacion_9_5();
	else if (objeto_que_lleva == 16 && fl == 121) {
		flags[2] = 1;
		sin_verbo();
		actualiza_datos();
	} else if (objeto_que_lleva == 16) {
		hablar(TEXT439, "439.als");
		sin_verbo();
		visible[3] = 1;
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_54(int fl) {
	if ((objeto_que_lleva == HABLAR && fl == 118) || (objeto_que_lleva == MIRAR && fl == 118 && flags[0] == 0))
		animacion_1_5();
	else if (objeto_que_lleva == MIRAR && fl == 118 && flags[0]==1)
		hablar(TEXT124, "124.als");
	else if (objeto_que_lleva == MIRAR && fl == 53)
		hablar(TEXT127, "127.als");
	else if (objeto_que_lleva == HABLAR && fl == 53 && flags[14] == 0) {
		hablar(TEXT288, "288.als");
		flags[12] = 1;
		pausa(10);
		habla_mus(TEXTE1, "E1.als");
		hablar(TEXT289, "289.als");
		habla_mus(TEXTE2, "E2.als");
		habla_mus(TEXTE3, "E3.als");
		conversa("op_10.cal");
		flags[12] = 0;
		flags[14] = 1;
	} else if (objeto_que_lleva == HABLAR && fl == 53 && flags[14] == 1)
		hablar(TEXT109, "109.als");
	else if (objeto_que_lleva == COGER && fl == 9999 && flags[13] == 0) {
		agarra_objeto(8);
		flags[13] = 1;
		habla_mus(TEXTE10, "e10.als");
		actualiza_datos();
	} else if (objeto_que_lleva == ABRIR && fl == 119)
		hablar(TEXT125, "125.als");
	else if (objeto_que_lleva == MIRAR && fl == 119)
		hablar(TEXT126, "126.als");
	else if (objeto_que_lleva == 10 && fl == 119) {
		pausa(4);
		hablar(TEXT436, "436.als");
		sin_verbo();
		resta_objeto(10);
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_55(int fl) {
	if (objeto_que_lleva == COGER && fl == 122) {
		agarra_objeto(12);
		flags[8] = 1;
		actualiza_datos();
	} else if (objeto_que_lleva == MIRAR && fl == 122)
		hablar(TEXT138, "138.als");
	else if (objeto_que_lleva == MIRAR && fl == 204)
		hablar(TEXT139, "139.als");
	else if (objeto_que_lleva == MIRAR && fl == 205)
		hablar(TEXT140, "140.als");
	else if (fl == 206) {
		comienza_sound("s11.als");
		anima("det.bin", 17);
		fin_sound();
		lleva_al_hare(hare_x - 3, hare_y + alto_hare + 6);
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_56(int fl) {
	if (objeto_que_lleva == ABRIR && fl == 124)
		animacion_14_5();
	else if (objeto_que_lleva == MIRAR && fl == 124)
		hablar(TEXT450, "450.als");
	else if (objeto_que_lleva == ABRIR && fl == 207)
		hablar(TEXT141, "141.als");
	else if (objeto_que_lleva == MIRAR && fl == 208)
		hablar(TEXT142, "142.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::habla_lobo(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[9] = {52, 79, 106, 133, 160, 187, 214, 241, 268};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	color_abc(ROJO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(8);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 136, 198, 81, 26, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0) 
		centra_texto(dicho, 203, 78);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_mus(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 16, 35, 54, 73, 92, 111, 130, 149};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 156, 190, 64, 18, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 197, 64);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
	if (longitud > 0)
		goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::animacion_1_6() {
	int l;

	sentido_hare = 0;
	hare_x = 103;
	hare_y = 108;
	flags[0] = 1;
	for (l = 0; l < 200; l++)
		factor_red[l] = 98;

	lee_dibujos("auxig2.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("auxdr.alg");
	descomprime_dibujo(dir_dibujo2, 1);
	lee_dibujos("car.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	habla_dr_dch(TEXTD19, "D19.als");
	hablar(TEXT247, "247.als");
	habla_dr_dch(TEXTD20, "d20.als");
	habla_dr_dch(TEXTD21, "d21.als");
	hablar(TEXT248, "248.als");
	habla_dr_dch(TEXTD22, "d22.als");
	hablar(TEXT249, "249.als");
	habla_dr_dch(TEXTD23, "d23.als");
	conversa("op_11.cal");
	habla_dr_dch(TEXTD26, "d26.als");

	anima("fum.bin", 15);

	habla_dr_dch(TEXTD27, "d27.als");
	hablar(TEXT254, "254.als");
	habla_dr_dch(TEXTD28, "d28.als");
	hablar(TEXT255, "255.als");
	habla_dr_dch(TEXTD29, "d29.als");
	FundeAlNegro(1);
	borra_pantalla();
	lee_dibujos("time1.alg");
	descomprime_dibujo(dir_zona_pantalla, 1);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(930);
	borra_pantalla();
	Negro();
	hare_se_ve = 0;
	flags[0] = 0;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	hablar(TEXT256, "256.als");
	habla_dr_dch(TEXTD30, "d30.als");
	hablar(TEXT257, "257.als");
	FundeAlNegro(0);
	borra_pantalla();
	lee_dibujos("time1.alg");
	descomprime_dibujo(dir_zona_pantalla,1);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	delay(900);
	borra_pantalla();
	Negro();
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(1);
	hablar(TEXT258, "258.als");
	habla_dr_dch(TEXTD31, "d31.als");
	animacion_5_6();
	habla_dr_dch(TEXTD32, "d32.als");
	habla_igor_dch(TEXTI11, "I11.als");
	sentido_igor = 3;
	habla_dr_dch(TEXTD33, "d33.als");
	habla_igor_frente(TEXTI12, "I12.als");
	habla_dr_dch(TEXTD34, "d34.als");
	sentido_dr = 0;
	habla_dr_izq(TEXTD35,"d35.als");
	borra_pantalla();
	carga_escoba_1("102.ald");
	activa_pendulo();
}

void DrasculaEngine::animacion_2_6() {
	habla_dr_dch(TEXTD24, "d24.als");
}

void DrasculaEngine::animacion_3_6() {
	habla_dr_dch(TEXTD24, "d24.als");
}

void DrasculaEngine::animacion_4_6() {
	habla_dr_dch(TEXTD25, "d25.als");
}

void DrasculaEngine::animacion_5_6() {
	int n, pos_pen[6];

	pos_pen[0] = 1;
	pos_pen[1] = 29;
	pos_pen[2] = 204;
	pos_pen[3] = -125;
	pos_pen[4] = 18;
	pos_pen[5] = 125;

	anima("man.bin", 14);

	for (n = -125; n <= 0; n = n + 2) {
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		actualiza_refresco_antes();
		pos_pen[3] = n;
		DIBUJA_BLOQUE_CUT(pos_pen, dir_dibujo3, dir_zona_pantalla);

		actualiza_refresco();

		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(2);
	}

	flags[3] = 1;
}

void DrasculaEngine::animacion_6_6() {
	anima("rct.bin", 11);
	borra_pantalla();
	sin_verbo();
	resta_objeto(20);
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_frente, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	rompo = 1;
	obj_saliendo = 104;
	hare_x = -1;
	sin_verbo();
	carga_escoba_1("58.ald");
	hare_se_ve = 1;
	sentido_hare = 1;
	anima("hbp.bin", 14);

	sentido_hare = 3;
	flags[0] = 1;
	flags[1] = 0;
	flags[2] = 1;
}

void DrasculaEngine::animacion_7_6() {
	flags[8] = 1;
	actualiza_datos();
}

void DrasculaEngine::animacion_9_6() {
	int v_cd;

	anima("fin.bin", 14);
	playmusic(13);
	flags[5] = 1;
	anima("drf.bin", 16);
	FundeAlNegro(0);
	borra_pantalla();
	hare_x = -1;
	obj_saliendo = 108;
	carga_escoba_1("59.ald");
	strcpy(num_room, "nada.alg");
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	Negro();
	sentido_hare = 1;
	hare_x -= 21;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	pausa(96);
	lleva_al_hare(116, 178);
	sentido_hare = 2;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playmusic(9);
	borra_pantalla();
	lee_dibujos("nota.alg");
	descomprime_dibujo(dir_dibujo1, COMPLETA);
	color_abc(BLANCO);
	habla_solo(TEXTBJ24, "bj24.als");
	habla_solo(TEXTBJ25, "bj25.als");
	habla_solo(TEXTBJ26, "bj26.als");
	habla_solo(TEXTBJ27, "bj27.als");
	habla_solo(TEXTBJ28, "bj28.als");
	sentido_hare = 3;
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	hablar(TEXT296, "296.als");
	hablar(TEXT297, "297.als");
	hablar(TEXT298, "298.als");
	sentido_hare = 1;
	hablar(TEXT299, "299.als");
	hablar(TEXT300, "300.als");
	refresca_pantalla();
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	color_abc(VERDE_CLARO);
	habla_solo("GOOOOOOOOOOOOOOOL", "s15.als");
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, 1);
	sentido_hare = 0;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	hablar(TEXT301, "301.als");
	v_cd = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 16;
	v_cd = v_cd + 4;
	playmusic(17);
	FundeAlNegro(1);
	borra_pantalla();
	fliplay("qpc.bin", 1);
	MusicFadeout();
	stopmusic();
	borra_pantalla();
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, v_cd * 16);
	playmusic(3);
	fliplay("crd.bin", 1);
	stopmusic();
	error("end of game ?");
}

void DrasculaEngine::animacion_10_6() {
	comienza_sound ("s14.als");
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	DIBUJA_FONDO(164, 85, 155, 48, 113, 114, dir_dibujo3, dir_zona_pantalla);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	fin_sound();
	habla_taber2(TEXTT23, "t23.als");
	flags[7] = 1;
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	FundeDelNegro(0);
	pausa(96);
	lleva_al_hare(116, 178);
	sentido_hare = 2;
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	playmusic(9);
	borra_pantalla();
	lee_dibujos("nota.alg");
	descomprime_dibujo(dir_dibujo1, COMPLETA);
	color_abc(BLANCO);
	habla_solo(TEXTBJ24, "bj24.als");
	habla_solo(TEXTBJ25, "bj25.als");
	habla_solo(TEXTBJ26, "bj26.als");
	habla_solo(TEXTBJ27, "bj27.als");
	habla_solo(TEXTBJ28, "bj28.als");
	sentido_hare = 3;
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	hablar(TEXT296, "296.als");
	hablar(TEXT297, "297.als");
	hablar(TEXT298, "298.als");
	sentido_hare = 1;
	hablar(TEXT299, "299.als");
	hablar(TEXT300, "300.als");
	refresca_pantalla();
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	color_abc(VERDE_CLARO);
	habla_solo("GOOOOOOOOOOOOOOOL", "s15.als");
	lee_dibujos("nota2.alg");
	descomprime_dibujo(dir_dibujo1, 1);
}

void DrasculaEngine::animacion_11_6() {
	habla_taber2(TEXTT10, "t10.als");
	hablar(TEXT268, "268.als");
	habla_taber2(TEXTT11, "t11.als");
}

void DrasculaEngine::animacion_12_6() {
	habla_taber2(TEXTT12, "t12.als");
	hablar(TEXT270, "270.als");
	habla_taber2(TEXTT13, "t13.als");
	habla_taber2(TEXTT14, "t14.als");
}

void DrasculaEngine::animacion_13_6() {
	habla_taber2(TEXTT15, "t15.als");
}

void DrasculaEngine::animacion_14_6() {
	habla_taber2(TEXTT24, "t24.als");
	suma_objeto(21);
	flags[10] = 1;
	rompo_y_salgo = 1;
}

void DrasculaEngine::animacion_15_6() {
	habla_taber2(TEXTT16, "t16.als");
}

void DrasculaEngine::animacion_18_6() {
	sin_verbo();
	resta_objeto(21);
	anima("beb.bin", 10);
}

void DrasculaEngine::animacion_19_6() {
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_FONDO(140, 23, 161, 69, 35, 80, dir_dibujo3, dir_zona_pantalla);

	actualiza_refresco_antes();
	pon_hare();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(6);
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	comienza_sound("s4.als");
	pausa(6);
	fin_sound();
}

void DrasculaEngine::activa_pendulo() {
	flags[1] = 2;
	hare_se_ve = 0;
	strcpy(num_room, "102.alg");
	lee_dibujos("102.alg");
	descomprime_dibujo(dir_dibujo1, MEDIA);
	lee_dibujos("an_p1.alg");
	descomprime_dibujo(dir_dibujo3, 1);
	lee_dibujos("an_p2.alg");
	descomprime_dibujo(dir_hare_dch, 1);
	lee_dibujos("an_p3.alg");
	descomprime_dibujo(dir_hare_frente, 1);

	DIBUJA_FONDO(0, 171, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	conta_ciego_vez = (int)vez();
}

void DrasculaEngine::habla_pen(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[8] = {112, 138, 164, 190, 216, 242, 268, 294};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	flags[1] = 1;

	refresca_pantalla();
	DIBUJA_BLOQUE(44, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(AMARILLO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(7);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_BLOQUE(x_habla[cara], 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 160, 105);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	flags[1] = 0;
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_pen2(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[5]={122, 148, 174, 200, 226};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	flags[1] = 1;

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(AMARILLO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(4);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 195, 107);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	flags[1] = 0;
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_taber2(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[6] = {1, 23, 45, 67, 89, 111};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(MARRON);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(5);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 130, 151, 43, 21, 24, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 132, 45);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::refresca_58() {
	if (hare_se_ve == 1)
		DIBUJA_BLOQUE(67, 139, 140, 147, 12, 16, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_60() {
	if (hare_y - 10 < y_dr && flags[5] == 0)
		pon_dr();
}

void DrasculaEngine::refresca_61() {
	DIBUJA_BLOQUE(1, 154, 83, 122, 131, 44, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_58_antes() {
	if (flags[0] == 0)
		DIBUJA_FONDO(1, 156, 143, 120, 120, 43, dir_dibujo3, dir_zona_pantalla);
	if (flags[1] == 2)
		DIBUJA_BLOQUE(252, 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);
	if (flags[1] == 0 && flags[0] == 0)
		DIBUJA_BLOQUE(278, 171, 173, 116, 25, 28, dir_dibujo3, dir_zona_pantalla);
	if (flags[2] == 0) {
		pon_igor();
		pon_dr();
	}
	if (flags[3] == 1)
		DIBUJA_BLOQUE(1, 29, 204, 0, 18, 125, dir_dibujo3, dir_zona_pantalla);
	if (flags[8] == 1)
		DIBUJA_FONDO(20, 60, 30, 64, 46, 95, dir_dibujo3, dir_zona_pantalla);
}

void DrasculaEngine::refresca_59_antes() {
	if (flags[4] == 0)
		DIBUJA_BLOQUE(1, 146, 65, 106, 83, 40, dir_dibujo3, dir_zona_pantalla);
	if (flags[9] == 1) {
		DIBUJA_FONDO(65, 103, 65, 103, 49, 38, dir_dibujo1, dir_zona_pantalla);
		DIBUJA_BLOQUE(1, 105, 65, 103, 49, 38, dir_dibujo3, dir_zona_pantalla);
	}
}

void DrasculaEngine::refresca_60_antes() {
	int velas_y[] = {158, 172, 186};
	int diferencia;

	if (flags[5] == 0)
		pon_dr();

	DIBUJA_FONDO(123, velas_y[frame_velas], 142, 14, 39, 13, dir_dibujo3, dir_zona_pantalla);

	if (flag_tv == 1)
		DIBUJA_FONDO(114, 158, 8, 30, 8, 23, dir_dibujo3, dir_zona_pantalla);

	diferencia = (int)vez() - conta_ciego_vez;
	parpadeo = _rnd->getRandomNumber(7);
	if (parpadeo == 5 && flag_tv == 0)
		flag_tv = 1;
	else if (parpadeo == 5 && flag_tv == 1)
		flag_tv = 0;
	if (diferencia > 6) {
		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		conta_ciego_vez = (int)vez();
	}
}

void DrasculaEngine::pantalla_58(int fl) {
	if (objeto_que_lleva == MOVER && fl == 103)
		animacion_7_6();
	else if (objeto_que_lleva == MIRAR && fl == 104)
		hablar(TEXT454, "454.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_59(int fl) {
	if ((objeto_que_lleva == HABLAR && fl == 51) || (objeto_que_lleva == MIRAR && fl == 51)) {
		flags[9] = 1;
		hablar(TEXT259, "259.als");
		habla_bj_cama(TEXTBJ13, "bj13.als");
		hablar(TEXT263, "263.als");
		habla_bj_cama(TEXTBJ14, "bj14.als");
		pausa(40);
		hablar(TEXT264, "264.als");
		habla_bj_cama(TEXTBJ15, "BJ15.als");
		hablar(TEXT265, "265.als");
		flags[9] = 0;
		if (flags[11] == 0) {
			comienza_sound("s12.als");
			delay(40);
			fin_sound();
			delay(10);
			lleva_al_hare(174, 168);
			sentido_hare = 2;
			refresca_pantalla();
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			pausa(40);
			comienza_sound("s12.als");
			pausa(19);
			fin_sound_corte();
			hare_se_ve = 0;
			refresca_pantalla();
			DIBUJA_BLOQUE(101, 34, hare_x - 4, hare_y - 1, 37, 70, dir_dibujo3, dir_zona_pantalla);
			DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);
			VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
			hare_se_ve = 1;
			borra_pantalla();
			lee_dibujos("tlef0.alg");
			descomprime_dibujo(dir_dibujo1, COMPLETA);
			lee_dibujos("tlef1.alg");
			descomprime_dibujo(dir_dibujo3, 1);
			lee_dibujos("tlef2.alg");
			descomprime_dibujo(dir_hare_frente, 1);
			lee_dibujos("tlef3.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
			habla_htel(TEXT240, "240.als");

			color_abc(VON_BRAUN);
			habla_solo(TEXTVB58, "VB58.als");
			habla_htel(TEXT241, "241.als");
			color_abc(VON_BRAUN);
			habla_solo(TEXTVB59, "VB59.als");
			habla_htel(TEXT242, "242.als");
			color_abc(VON_BRAUN);
			habla_solo(TEXTVB60, "VB60.als");
			habla_htel(TEXT196, "196.als");
			color_abc(VON_BRAUN);
			habla_solo(TEXTVB61,"VB61.als");
			habla_htel(TEXT244, "244.als");
			color_abc(VON_BRAUN);
			habla_solo(TEXTVB62, "VB62.als");
			borra_pantalla();
			lee_dibujos("aux59.alg");
			descomprime_dibujo(dir_dibujo3, 1);
			lee_dibujos("96.alg");
			descomprime_dibujo(dir_hare_frente, COMPLETA);
			lee_dibujos("99.alg");
			descomprime_dibujo(dir_hare_fondo, 1);
			lee_dibujos("59.alg");
			descomprime_dibujo(dir_dibujo1, MEDIA);
			sentido_hare = 3;
			hablar(TEXT245, "245.als");
			sin_verbo();
			flags[11] = 1;
		}
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_60(int fl) {
	if (objeto_que_lleva == MOVER && fl == 112)
		animacion_10();
	else if (objeto_que_lleva == MIRAR && fl == 112)
		hablar(TEXT440, "440.als");
	else if (objeto_que_lleva == HABLAR && fl == 52) {
		hablar(TEXT266, "266.als");
		habla_taber2(TEXTT1, "t1.als");
		conversa("op_12.cal");
		sin_verbo();
		objeto_que_lleva = 0;
	} else if (objeto_que_lleva == HABLAR && fl == 115)
		hablar(TEXT455, "455.als");
	else if (objeto_que_lleva == HABLAR && fl == 56)
		hablar(TEXT455, "455.als");
	else if (objeto_que_lleva == MIRAR && fl == 114)
		hablar(TEXT167, "167.als");
	else if (objeto_que_lleva == MIRAR && fl == 113)
		hablar(TEXT168, "168.als");
	else if (objeto_que_lleva == COGER && fl == 113)
		hablar(TEXT170, "170.als");
	else if (objeto_que_lleva == MOVER && fl == 113)
		hablar(TEXT170, "170.als");
	else if (objeto_que_lleva == HABLAR && fl == 113)
		hablar(TEXT169, "169.als");
	else if (objeto_que_lleva == 21 && fl == 56)
		animacion_18_6();
	else if (objeto_que_lleva == 9 && fl == 56 && flags[6] == 1)
		animacion_9();
	else if (objeto_que_lleva == 9 && fl == 56 && flags[6] == 0) {
		anima("cnf.bin", 14);
		hablar(TEXT455, "455.als");
	} else
		hay_respuesta = 0;
}

void DrasculaEngine::pantalla_61(int fl) {
	if (objeto_que_lleva == MIRAR && fl == 116)
		hablar(TEXT172, "172.als");
	else if (objeto_que_lleva == MIRAR && fl == 117)
		hablar(TEXT173, "173.als");
	else if (objeto_que_lleva == MOVER && fl == 117)
		hablar(TEXT174, "174.als");
	else if (objeto_que_lleva == ABRIR && fl == 117)
		hablar(TEXT174, "174.als");
	else
		hay_respuesta = 0;
}

void DrasculaEngine::habla_bj_cama(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;

	int x_habla[5] = {51, 101, 151, 201, 251};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(4);

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(65, 103, 65, 103, 49, 38, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 105, 65, 103, 49, 38, dir_dibujo3, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 104, 102);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}
	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::habla_htel(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	char *num_cara;

	int x_habla[3] = {1, 94, 187};
	int cara, pantalla;

	int longitud;
	longitud = strlen(dicho);

	tiempol = _system->getMillis();
	tiempou = (unsigned int)tiempol / 2;
	_rnd->setSeed(tiempou);

	buffer_teclado();

	color_abc(AMARILLO);

	if (hay_sb == 1) {
		sku = new Common::File;
		sku->open(filename);
		if (!sku->isOpen()) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = _rnd->getRandomNumber(2);
	pantalla = _rnd->getRandomNumber(2);

	if (cara == 0 && pantalla == 0)
		num_cara = (char *)dir_dibujo3;
	else if (pantalla == 1)
		num_cara = (char *)dir_hare_frente;
	else
		num_cara = (char *)dir_hare_fondo;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	DIBUJA_FONDO(x_habla[cara], 1, 45, 24, 92, 108, (byte *)num_cara, dir_zona_pantalla);

	if (con_voces == 0)
		centra_texto(dicho, 90, 50);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	pausa(3);

	int key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		delete sku;
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	}

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::refresca_pendulo() {
	int pendulo_x[] = {40, 96, 152, 208, 264, 40, 96, 152, 208, 208, 152, 264, 40, 96, 152, 208, 264};
	int diferencia;

	if (frame_pen <= 4)
		dir_pendulo = dir_dibujo3;
	else if (frame_pen <= 11)
		dir_pendulo = dir_hare_dch;
	else
		dir_pendulo = dir_hare_frente;

	DIBUJA_FONDO(pendulo_x[frame_pen], 19, 152, 0, 55, 125, dir_pendulo, dir_zona_pantalla);

	if (flags[1] == 2)
		DIBUJA_BLOQUE(18, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	if (flags[1] == 0)
		DIBUJA_BLOQUE(44, 145, 145, 105, 25, 29, dir_dibujo3, dir_zona_pantalla);

	diferencia = (int)vez() - conta_ciego_vez;
	if (diferencia > 8) {
		frame_pen++;
		if (frame_pen == 17)
			frame_pen = 0;
		conta_ciego_vez = (int)vez();
	}
}

void DrasculaEngine::pantalla_pendulo(int fl) {
	if (objeto_que_lleva == MIRAR && fl == 100)
		hablar(TEXT452, "452.als");
	else if (objeto_que_lleva == MIRAR && fl == 101)
		hablar (TEXT123, "123.als");
	else if (objeto_que_lleva == COGER && fl == 101)
		agarra_objeto(20);
	else if (objeto_que_lleva == 20 && fl == 100)
		animacion_6_6();
	else if (objeto_que_lleva == COGER || objeto_que_lleva == ABRIR)
		hablar(TEXT453, "453.als");
	else
		hay_respuesta = 0;
}


} // End of namespace Drascula

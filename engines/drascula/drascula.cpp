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

}

DrasculaEngine::~DrasculaEngine() {
	salir_al_dos(0);

	free(VGA);

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
static int frame_x[6] = {43, 87, 130, 173, 216, 259};
static int interf_x[] ={ 1, 65, 129, 193, 1, 65, 129 };
static int interf_y[] ={ 51, 51, 51, 51, 83, 83, 83 };

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

	VGA = (char *)malloc(320 * 200);
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
	num_ejec = 1;
	cual_ejec = 0; hay_que_load = 0;
	corta_musica = 0;
	hay_seleccion = 0;
	Leng = 0;
	UsingMem = 0;
	GlobalSpeed = 0;



	asigna_memoria();
	carga_info();


	return 0;
}


int DrasculaEngine::go() {
	lee_dibujos("95.alg");
	descomprime_dibujo(dir_mesa, 1);  

	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);  
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	lee_dibujos("97.alg");
	descomprime_dibujo(dir_hare_dch, 1);

	strcpy(nombre_icono[1], "look");
	strcpy(nombre_icono[2], "take");
	strcpy(nombre_icono[3], "open");
	strcpy(nombre_icono[4], "close");
	strcpy(nombre_icono[5], "talk");
	strcpy(nombre_icono[6], "push");

	paleta_hare();
	escoba();    

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
	if (r == 2)
		error("Game reach next segment");
}

void DrasculaEngine::asigna_memoria() {
	dir_zona_pantalla = (char *)malloc(64000);
	assert(dir_zona_pantalla);
	dir_dibujo1 = (char *)malloc(64000);
	assert(dir_dibujo1);
	dir_hare_fondo = (char *)malloc(64000);
	assert(dir_hare_fondo);
	dir_dibujo3 = (char *)malloc(64000);
	assert(dir_dibujo3);
	dir_dibujo2 = (char *)malloc(64000);
	assert(dir_dibujo2);
	dir_mesa = (char *)malloc(64000);
	assert(dir_mesa);
	dir_hare_dch = (char *)malloc(64000);
	assert(dir_hare_dch);
	dir_hare_frente = (char *)malloc(64000);
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
	unsigned char ch,rep;
	Common::File file;
	char *auxPun;

	file.open(NamePcc);
	if (!file.isOpen())
		error("missing game data %s %c", NamePcc, 7);

	Buffer_pcx = (char *)malloc(65000);
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

void DrasculaEngine::descomprime_dibujo(char *dir_escritura, int plt) {
	memcpy(dir_escritura, Buffer_pcx, 64000);
	free(Buffer_pcx);
	asigna_rgb((unsigned char *)cPal, plt); // TODO
	if (plt > 1)
		funde_rgb(plt);
}

void DrasculaEngine::paleta_hare() {
	int color, componente;

	for (color = 235; color < 253; color++)
		for (componente = 0; componente < 4; componente++)
			palHare[color][componente] = palJuego[color][componente];

}

void DrasculaEngine::asigna_rgb(unsigned char *dir_lectura, int plt) {
	int x, cnt = 0;

	for (x = 0; x < plt; x++) {
		palJuego[x][0] = dir_lectura[cnt++] / 4;
		palJuego[x][1] = dir_lectura[cnt++] / 4;
		palJuego[x][2] = dir_lectura[cnt++] / 4;
		palJuego[x][3] = 0;
	}
	ActualizaPaleta();
}

void DrasculaEngine::funde_rgb(int plt) {
/* TODO: ???
	unsigned int n;

	for (n = 0; n < plt; n++) {
		palJuego[n][0] = inp(0x3c9);
		palJuego[n][1] = inp(0x3c9);
		palJuego[n][2] = inp(0x3c9);
		palJuego[n][3] = 0;
	}
	ActualizaPaleta();
*/
}

void DrasculaEngine::Negro() {
	int color, componente;
	DacPalette256 palNegra;

	for (color = 0; color < 256; color++)
		for (componente = 0; componente < 4; componente++)
			palNegra[color][componente] = 0;

	palNegra[254][0] = 0x3F;
	palNegra[254][1] = 0x3F;
	palNegra[254][2] = 0x15;
	palNegra[254][0] = 0;

	setvgapalette256(&palNegra);
}

void DrasculaEngine::ActualizaPaleta() {
	setvgapalette256(&palJuego);
}

void DrasculaEngine::setvgapalette256(DacPalette256 *PalBuf) {
	g_system->setPalette((byte *)PalBuf, 0, 256);
}

void DrasculaEngine::DIBUJA_FONDO(int xorg, int yorg, int xdes, int ydes, int Ancho,
				int Alto, char *Origen, char *Destino) {
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
				int Alto, char *Origen, char *Destino) {
	int y, x;

	Destino += xdes + ydes * 320;
	Origen += xorg + yorg * 320;

	for (y = 0; y < Alto; y++)
		for (x = 0; x < Ancho; x++)
			if (Origen[x + y * 320] != 255)
				Destino[x + y * 320] = Origen[x + y * 320];
}

void DrasculaEngine::DIBUJA_BLOQUE_CUT(int *Array, char *Origen, char *Destino) {
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

void DrasculaEngine::VUELCA_PANTALLA(int xorg, int yorg, int xdes, int ydes, int Ancho, int Alto, char *Buffer) {
	int x;

	VGA += xdes + ydes * 320;
	Buffer += xorg + yorg * 320;
	for (x = 0; x < Alto; x++) {
		memcpy(VGA, Buffer, Ancho);
		VGA += 320;
		Buffer += 320;
	}

	g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
}

void DrasculaEngine::escoba() { 
	int soc, l, n;

	dir_texto = dir_mesa;

	musica_antes = -1;

	soc = 0;
	for (l = 0; l < 6; l++) {
		soc=soc+ANCHO_PERSONAJE;
		frame_x[l]=soc;        
	}

	for (n = 1; n < 43; n++)
		objetos_que_tengo[n] = 0;

	for (n = 0; n < NUM_BANDERAS; n++)
		flags[n] = 0;
	
	for (n = 1; n < 7; n++)
		objetos_que_tengo[n] = n;

	agarra_objeto(28);

	buffer_teclado();

	if (hay_que_load == 0)
		animacion_1();

	sin_verbo();
	lee_dibujos("2aux62.alg");
	descomprime_dibujo(dir_dibujo2, 1);
	sentido_hare = 1;
	obj_saliendo = 104;
	if (hay_que_load != 0)
		para_cargar(nom_partida);
	else {
		carga_escoba("62.ald");
		hare_x = -20;
		hare_y = 56;
		lleva_al_hare(65, 145);
	}

bucles:
	if (hare_se_mueve == 0) {
		paso_x=PASO_HARE_X;
		paso_y=PASO_HARE_Y;
	}
	if (hare_se_mueve == 0 && anda_a_objeto==1) {
		sentido_hare = sentido_final;
		anda_a_objeto = 0;
	}

	mueve_cursor();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
	if (music_status() == 0)
		playmusic(musica_room);

	MirarRaton();

	if (menu_scr == 0 && lleva_objeto == 1)
		comprueba_objetos();

	if (boton_dch == 1 && menu_scr == 1) {
		lee_dibujos("99.alg");
		descomprime_dibujo(dir_hare_fondo, 1);
		setvgapalette256(&palJuego);
		menu_scr = 0;
		espera_soltar();
		cont_sv = 0;
	}
	if (boton_dch == 1 && menu_scr == 0) {
		hare_se_mueve = 0;
		if (sentido_hare == 2)
			sentido_hare = 1;
		lee_dibujos("icons.alg");
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
		comprueba1(); 
		cont_sv = 0;
	} else if (boton_izq == 1 && lleva_objeto == 1) {
		comprueba2(); 
		cont_sv = 0;
	}

	if (y_raton < 24 && menu_scr == 0)
		menu_bar = 1;
	else
		menu_bar = 0;

	key = getscan();
	if (key == F1 && menu_scr == 0) {
		elige_verbo(1); 
		cont_sv = 0;
	} else if (key == F2 && menu_scr == 0) {
		elige_verbo(2);
		cont_sv = 0;
	} else if (key == F3 && menu_scr == 0) {
		elige_verbo(3);
		cont_sv = 0;
	} else if (key == F4 && menu_scr == 0) {
		elige_verbo(4);
		cont_sv = 0;
	} else if (key == F5 && menu_scr == 0) {
		elige_verbo(5);
		cont_sv = 0;
	} else if (key == F6 && menu_scr == 0) {
		elige_verbo(6);
		cont_sv = 0;
	} else if (key == F9) {
		mesa();
		cont_sv = 0;
	} else if (key == F10) {
		saves();
		cont_sv = 0;
	} else if (key == F8) {
		sin_verbo();
		cont_sv = 0;
	} else if (key == 47) {
		con_voces = 1;
		print_abc(SYS2, 96, 86);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1410);
		cont_sv = 0;
	} else if (key == 20) {
		con_voces = 0;
		print_abc(SYS3, 94, 86);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		delay(1460);
		cont_sv = 0;
	} else if (key == 83) {
		confirma_go();
		cont_sv = 0;
	} else if (key == ESC) {
		confirma_salir();
		cont_sv = 0;
	} else if (cont_sv == 1500) {
		salva_pantallas();
		cont_sv = 0;
	} else cont_sv++;
	goto bucles;
}

void DrasculaEngine::agarra_objeto(int objeto) {
	lee_dibujos("icons.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	elige_objeto(objeto);
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::elige_objeto(int objeto) {
	if (lleva_objeto == 1 && menu_scr == 0)
		suma_objeto(objeto_que_lleva); 
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

void DrasculaEngine::animacion_1() {
	int l, l2, p;        
	int pos_pixel[6];

	while (term_int == 0) {
		playmusic(29);
		fliplay("logoddm.bin",9);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		delay(600);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		delay(340);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		playmusic(26);
		delay(500);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		fliplay("logoalc.bin", 8);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		lee_dibujos("cielo.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		Negro();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(2);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		delay(900);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		color_abc(ROJO);
		centra_texto("Transilvanya, 1993 d.c.", 160, 100);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		delay(1000);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		delay(1200);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		fliplay("scrollb.bin", 9);

		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		comienza_sound("s5.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("scr2.bin", 17);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		fin_sound_corte();
		anima("scr3.bin", 17);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lee_dibujos("cielo2.alg");
		descomprime_dibujo(dir_zona_pantalla, 256);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		FundeAlNegro(1);
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;
		delay(400);        
		if ((term_int == 1) || (getscan() == ESC))
			break;

		for (l2 = 0; l2 < 3; l2++)
			for (l = 0; l < 7; l++) {
				DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
				DIBUJA_FONDO(interf_x[l], interf_y[l], 156, 45, 63, 31, dir_dibujo2, dir_zona_pantalla);
				VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
				if (getscan() == ESC) {
					term_int = 1;
					break;
				}
				pausa(3);
			}
			if ((term_int == 1) || (getscan() == ESC))
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
			if (getscan() == ESC) {
				term_int = 1;
				break;
			}
		}
		if ((term_int == 1) || (getscan() == ESC))
			break;
		DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_zona_pantalla, dir_dibujo1);

		habla_dr_grande(TEXTD1, "D1.als");
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;
	    DIBUJA_FONDO(0, 0, 0,0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		pon_igor();
		pon_dr();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_dr_izq(TEXTD2, "d2.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_dr_izq(TEXTD3, "d3.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("lib.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		color_solo = ROJO;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(10);
		habla_solo(TEXTD4,"d4.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lee_dibujos("plan1.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD5, "d5.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		lee_dibujos("plan2.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		habla_solo(TEXTD6, "d6.als");
		if ((term_int ==1) || (getscan() == ESC))
			break;
		anima("lib2.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(20);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_solo(TEXTD7, "d7.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lee_dibujos("plan3.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		habla_solo(TEXTD8, "d8.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		lee_dibujos("100.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		MusicFadeout();
		stopmusic();
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_dch(TEXTI9, "I9.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_dr_izq(TEXTD9, "d9.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_dch(TEXTI10, "I10.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		playmusic(11);
		habla_dr_izq(TEXTD10, "d10.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("rayo1.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		comienza_sound("s5.als");
		anima("rayo2.bin", 15);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("frel2.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("frel.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		fin_sound_corte();
		borra_pantalla();
		Negro();
		playmusic(23);
		FundeDelNegro(0);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		sentido_dr = 1;
		habla_igor_dch(TEXTI1, "I1.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_dr_dch(TEXTD11, "d11.als");
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;
		pausa(13);
		habla_dr_dch(TEXTD13,"d13.als");
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_dch(TEXTI3, "I3.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_dr_izq(TEXTD15, "d15.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_dch(TEXTI4, "I4.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_dr_izq(TEXTD16, "d16.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_dch(TEXTI5, "I5.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		sentido_igor = 3;
		habla_dr_izq(TEXTD17, "d17.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		pausa(18);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_igor_frente(TEXTI6, "I6.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		FundeAlNegro(0);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();

		playmusic(2);
		pausa(5);
		fliplay("intro.bin", 12);
		term_int=1;
	}
	borra_pantalla();
	lee_dibujos("96.alg");
	descomprime_dibujo(dir_hare_frente, COMPLETA);  
	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
}

void DrasculaEngine::animacion_2() {
	int l;

	lleva_al_hare(231, 91);
	hare_se_ve = 0;

	term_int = 0;

	for (;;) {
		if ((term_int == 1) || (getscan() == ESC))
			break;

		anima("ag.bin", 14);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		lee_dibujos("an11y13.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		habla_tabernero(TEXTT22, "T22.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;

		lee_dibujos("97.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		pausa(4);
		comienza_sound("s1.als");
		hipo(18);
		fin_sound();  
		if ((term_int == 1) || (getscan() == ESC))
			break;

		borra_pantalla();
		stopmusic();
		corta_musica = 1;
		memset(dir_zona_pantalla, 0, 64000);
		color_solo = BLANCO;
		pausa(80);

		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_solo(TEXTBJ1, "BJ1.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();
		lee_dibujos("bj.alg");
		descomprime_dibujo(dir_zona_pantalla, MEDIA);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		Negro();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		FundeDelNegro(1);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		color_solo = AMARILLO;
		habla_solo(TEXT214, "214.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		borra_pantalla();

		lee_dibujos("16.alg");
		descomprime_dibujo(dir_dibujo1, MEDIA);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lee_dibujos("auxbj.alg");
		descomprime_dibujo(dir_dibujo3, 1);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		strcpy(num_room,"16.alg");

		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;

		lee_dibujos("97g.alg");
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		anima("lev.bin", 15);
		if ((term_int == 1) || (getscan() == ESC))
			break;

		lleva_al_hare(100 + ancho_hare / 2, 99 + alto_hare);
		if ((term_int == 1) || (getscan() == ESC))
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
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lee_dibujos("97.alg");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		descomprime_dibujo(dir_hare_dch, 1);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		pausa(120);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_solo(TEXT223, "223.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		color_solo = BLANCO;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == ESC))
			break;
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(110);
		habla_solo(TEXTBJ11, "BJ11.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		refresca_pantalla();
		if ((term_int == 1) || (getscan() == ESC))
			break;
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		pausa(118);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		lleva_al_hare(132, 97 + alto_hare);
		pausa(60);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		hablar(TEXT224, "224.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		habla_bj(TEXTBJ12, "BJ12.als");
		lleva_al_hare(157, 98 + alto_hare);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		anima("bes.bin", 16);
		playmusic(11);
		anima("rap.bin", 16);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		sentido_hare = 3;
		strcpy(num_room, "no_bj.alg");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		pausa(8);
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT225, "225.als");
		pausa(76);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		sentido_hare = 1;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		hablar(TEXT226, "226.als");
		if ((term_int == 1) || (getscan() == ESC))
			break;
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		pausa(30);
		if ((term_int == 1) || (getscan() == ESC))
			break;
		hablar(TEXT227,"227.als");
		FundeAlNegro(0);
		break;
	}
	salir_al_dos(2);
}

void DrasculaEngine::sin_verbo() {
	int c = 171;
	if (menu_scr == 1)
		c = 0;
	if (lleva_objeto==1)
		suma_objeto(objeto_que_lleva);
		DIBUJA_FONDO(0, c, 0, 0, ANCHOBJ,ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 0;
	hay_nombre = 0;
}

void DrasculaEngine::para_cargar(char nom_game[]) {
	musica_antes = musica_room;
	menu_scr = 0;
	carga_partida(nom_game);
	carga_escoba(datos_actuales);
	sin_verbo();
}

void DrasculaEngine::carga_escoba(char nom_fich[13]) {
	int l, obj_salir; 
	float chiquez, pequegnez = 0;
	char para_codificar[13];

	hay_nombre = 0;

	strcpy(para_codificar, nom_fich);
	canal_p(para_codificar);
	strcpy(datos_actuales, nom_fich);

	buffer_teclado();

	// TODO
	if ((ald = fopen(nom_fich, "rb")) == NULL)  {
  		error("missing data file");
	}
	fscanf(ald, "%s", num_room);
	strcat(num_room,".alg");
	fscanf(ald, "%d", &musica_room);
	fscanf(ald, "%s", pantalla_disco);
	fscanf(ald, "%d", &nivel_osc);

	fscanf(ald, "%d", &objs_room);

	for (l = 0; l < objs_room;l++) {
		fscanf(ald, "%d", &num_obj[l]);
		fscanf(ald, "%s", nombre_obj[l]);
		fscanf(ald, "%d", &x1[l]);
		fscanf(ald, "%d", &y1[l]);
		fscanf(ald, "%d", &x2[l]);
		fscanf(ald, "%d", &y2[l]);
		fscanf(ald, "%d", &sitiobj_x[l]);
		fscanf(ald, "%d", &sitiobj_y[l]);
		fscanf(ald, "%d", &sentidobj[l]);
		fscanf(ald, "%d", &visible[l]);
		fscanf(ald, "%d", &espuerta[l]);
		if (espuerta[l] != 0) {
			fscanf(ald, "%s", alapantallakeva[l]);
			fscanf(ald, "%d", &x_alakeva[l]);
			fscanf(ald, "%d", &y_alakeva[l]);
			fscanf(ald, "%d", &sentido_alkeva[l]);
			fscanf(ald, "%d", &alapuertakeva[l]);
			puertas_cerradas(l);
		}
	}

	fscanf(ald, "%d", &suelo_x1);
	fscanf(ald, "%d", &suelo_y1);
	fscanf(ald, "%d", &suelo_x2);
	fscanf(ald, "%d", &suelo_y2);

	fscanf(ald, "%d", &lejos);
	fscanf(ald, "%d", &cerca);

	fclose(ald);
	canal_p(para_codificar);

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
	for (l = suelo_y1; l <= suelo_y2; l++){ 
		factor_red[l] = lejos + pequegnez;
		pequegnez = pequegnez + chiquez;
	}

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
	hare_se_mueve=0;
	
	actualiza_datos();

	espuerta[7] = 0;

	if (musica_antes != musica_room)
		playmusic(musica_room);  
	refresca_pantalla();
}

void DrasculaEngine::borra_pantalla() {
	memset(VGA, 0, 64000);
	g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
}

void DrasculaEngine::lleva_al_hare(int punto_x, int punto_y) {
	sitio_x=punto_x;
	sitio_y=punto_y;
	empieza_andar();

	for(;;) {
		refresca_pantalla();
		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
		if (hare_se_mueve == 0)
			break;
	}

	if (anda_a_objeto == 1) {
		anda_a_objeto = 0;
		sentido_hare = sentido_final;
	}
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
		if (color != ROJO && menu_scr == 0)
			color_abc(ROJO);
	} else if (menu_scr == 0 && color != VERDE_CLARO)
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
			hay_nombre=1;
			veo=1;
		}
	}
	
	if (x_raton > hare_x + 2 && y_raton > hare_y + 2
			&& x_raton < hare_x + ancho_hare - 2 && y_raton < hare_y + alto_hare - 2 && veo == 0) {
		strcpy(texto_nombre, "hacker");
		hay_nombre=1;
		veo=1;
	}  

	if (veo == 0)
		hay_nombre = 0;
}

void DrasculaEngine::espera_soltar() {
	// TODO
	//boton_izq=sal.w.bx & 1;
	//boton_dch=(sal.w.bx >> 1) & 1;
	//x_raton=sal.w.cx >> 1;
	//y_raton=sal.w.dx;
}

void DrasculaEngine::MirarRaton() {
	// TODO
	//boton_izq=sal.w.bx & 1;
	//boton_dch=(sal.w.bx >> 1) & 1;
	//x_raton=sal.w.cx >> 1;
	//y_raton=sal.w.dx;
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

void DrasculaEngine::comprueba1() {
	int l;

	if (menu_scr == 1)
		saca_objeto();    
	else {
		for (l = 0; l < objs_room; l++) {
			if (x_raton >= x1[l] && y_raton >= y1[l]
					&& x_raton <= x2[l] && y_raton <= y2[l] && rompo == 0) {
				sal_de_la_habitacion(l);
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
				 }
			}
	}  
}

char DrasculaEngine::getscan() {
	// TODO
	return 0;
}    

void DrasculaEngine::elige_verbo(int verbo) {
	int c = 171;

	if (menu_scr == 1)
		c = 0;
	if (lleva_objeto == 1)
		suma_objeto(objeto_que_lleva);

	DIBUJA_FONDO(ANCHOBJ * verbo, c, 0, 0, ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_dibujo3);

	lleva_objeto = 1;
	objeto_que_lleva = verbo;
}

void DrasculaEngine::mesa() {
	int nivel_master, nivel_voc, nivel_cd;

	DIBUJA_BLOQUE(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);
	VUELCA_PANTALLA(73, 63, 73, 63, 177, 97, dir_zona_pantalla);

	for (;;) {
		nivel_master = 72 + 61 - (12/*Master*/ * 4);
		nivel_voc = 72 + 61 - (12/*Voc*/ * 4);
		nivel_cd = 72 + 61 - (10/*CD*/ * 4);

		refresca_pantalla();

		DIBUJA_BLOQUE(1, 56, 73, 63, 177, 97, dir_mesa, dir_zona_pantalla);
	
		DIBUJA_FONDO(183, 56, 82, nivel_master, 39, 2 + (12/*Master*/ * 4), dir_mesa, dir_zona_pantalla);
		DIBUJA_FONDO(183, 56, 138, nivel_voc, 39, 2 + (12/*Voc*/ * 4), dir_mesa, dir_zona_pantalla);
		DIBUJA_FONDO(183, 56, 194, nivel_cd, 39, 2 + (10/*CD*/ * 4), dir_mesa, dir_zona_pantalla);

		cursor_mesa();

		VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

		MirarRaton();

		if (boton_dch == 1)
			break;
		if (boton_izq == 1) {
			if (x_raton > 80 && x_raton < 121) {
// TODO
//				if (y_raton < nivel_master && Master < 15)
//					Master++;
//				if (y_raton > nivel_master && Master > 0)
//					Master--;
//				SetMasterVolume(Master);
			}

			if (x_raton > 136 && x_raton < 178) {
// TODO
//				if (y_raton < nivel_voc && Voc < 15)
//					Voc++;
//				if (y_raton > nivel_voc && Voc > 0) 
//					Voc--;
//				SetVocVolume(Voc);
			}

			if (x_raton > 192 && x_raton < 233) {
// TODO
//				if (y_raton < nivel_cd && CD < 15)
//					CD++;
//				if (y_raton > nivel_cd && CD > 0)
//					CD--;
//				SetCDVolume(CD);
			}
		}

	}

	espera_soltar();
}

void DrasculaEngine::saves() {
	char nombres[10][23];
	char fichero[13];
	int n, n2, num_sav, y = 27;
	FILE *sav;

	borra_pantalla();

	if ((sav = fopen("saves.epa", "r")) == NULL) {
		error("Can't open saves.epa file.");
	}
	for (n = 0; n < NUM_SAVES; n++)
		fscanf(sav, "%s", nombres[n]);
	fclose(sav);

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

				if (strcmp(select, "*"))
					hay_seleccion = 1;
				else {
					introduce_nombre();
					strcpy(nombres[n], select);
					if (hay_seleccion == 1) {
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
						// TODO
						if ((sav = fopen("saves.epa", "w")) == NULL) {
							error("no puedo abrir el archivo de partidas.");
						}
						for (n = 0; n < NUM_SAVES; n++)
							fprintf(sav, "%s\n", nombres[n]);
						fclose(sav);
					}
				}

				print_abc(select, 117, 15);
				y = 27;
				for (n2 = 0; n2 < NUM_SAVES; n2++) {
					print_abc(nombres[n2], 116, y);
					y = y + 9;
				}
				if (hay_seleccion == 1) {
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
						strcpy(fichero, "gsave10");}
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
				if ((sav = fopen("saves.epa", "w")) == NULL) {
					error("no puedo abrir el archivo de partidas.");
				}
				for (n = 0; n < NUM_SAVES; n++)
					fprintf(sav, "%s\n", nombres[n]);
				fclose(sav);
			} else if (x_raton > 168 && y_raton > 154 && x_raton < 242 && y_raton < 180)
				break;
			else if (hay_seleccion == 0) {
				print_abc("elige una partida",117,15);
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

void DrasculaEngine::print_abc(char dicho[], int x_pantalla, int y_pantalla) {
	int pos_texto[8];
	int i, y_de_letra = 0, x_de_letra = 0, h, longitud;
	longitud = strlen(dicho);

	for (i = 0; dicho[i]; i++)
		dicho[i] = toupper(dicho[i]);

	for (h = 0; h < longitud; h++) {
		y_de_letra = Y_ABC;
/*
		if (dicho[h] == 'A')
			x_de_letra = X_A;
		else if (dicho[h] == 'B')
			x_de_letra = X_B;
		else if (dicho[h] == 'C')
			x_de_letra = X_C;
		else if (dicho[h] == 'D')
			x_de_letra = X_D;
		else if (dicho[h] == 'E')
			x_de_letra = X_E;
		else if (dicho[h] == 'F')
			x_de_letra = X_F;
		else if (dicho[h] == 'G')
			x_de_letra = X_G;
		else if (dicho[h] == 'H')
			x_de_letra = X_H;
		else if (dicho[h] == 'I')
			x_de_letra = X_I;
		else if (dicho[h] == 'J')
			x_de_letra = X_J;
		else if (dicho[h] == 'K')
			x_de_letra = X_K;
		else if (dicho[h] == 'L')
			x_de_letra = X_L;
		else if (dicho[h] == 'M')
			x_de_letra = X_M;
		else if (dicho[h] == 'N')
			x_de_letra = X_N;
		else if (dicho[h] == '¥')
			x_de_letra = X_GN;
		else if (dicho[h] == '¤')
			x_de_letra = X_GN;
		else if (dicho[h] == 'O')
			x_de_letra = X_O;
		else if (dicho[h] == 'P')
			x_de_letra = X_P;
		else if (dicho[h] == 'Q')
			x_de_letra = X_Q;
		else if (dicho[h] == 'R')
			x_de_letra = X_R;
		else if (dicho[h] == 'S')
			x_de_letra = X_S;
		else if (dicho[h] == 'T')
			x_de_letra = X_T;
		else if (dicho[h] == 'U')
			x_de_letra = X_U;
		else if (dicho[h] == 'V')
			x_de_letra = X_V;
		else if (dicho[h] == 'W')
			x_de_letra = X_W;
		else if (dicho[h] == 'X')
			x_de_letra = X_X;
		else if (dicho[h] == 'Y')
			x_de_letra = X_Y;
		else if (dicho[h] == 'Z')
			x_de_letra = X_Z;
		else if (dicho[h] == '§' || dicho[h] == ' ')
			x_de_letra = ESPACIO;
		else {
			y_de_letra = Y_SIGNOS;
			if (dicho[h] == '.')
				x_de_letra = X_PUNTO;
			else if (dicho[h] == ',')
				x_de_letra = X_COMA;
			else if (dicho[h] == '-')
				x_de_letra = X_GUION;
			else if (dicho[h] == '?')
				x_de_letra = X_CIERRA_INTERROGACION;
			else if (dicho[h] == '¨')
				x_de_letra = X_ABRE_INTERROGACION;
			else if (dicho[h] == '"')
				x_de_letra = X_COMILLAS;
			else if (dicho[h] == '!')
				x_de_letra = X_CIERRA_EXCLAMACION;
			else if (dicho[h] == '­')
				x_de_letra = X_ABRE_EXCLAMACION;
			else if (dicho[h] == ';')
				x_de_letra = X_PUNTO_Y_COMA;
			else if (dicho[h] == '>')
				x_de_letra = X_MAYOR_QUE;
			else if (dicho[h] == '<')
				x_de_letra = X_MENOR_QUE;
			else if (dicho[h] == '$')
				x_de_letra = X_DOLAR;
			else if (dicho[h] == '%')
				x_de_letra = X_POR_CIENTO;
			else if (dicho[h] == ':')
				x_de_letra = X_DOS_PUNTOS;
			else if (dicho[h] == '&')
				x_de_letra = X_AND;
			else if (dicho[h] == '/')
				x_de_letra = X_BARRA;
			else if (dicho[h] == '(')
				x_de_letra = X_ABRE_PARENTESIS;
			else if (dicho[h] == ')')
				x_de_letra = X_CIERRA_PARENTESIS;
			else if (dicho[h] == '*')
				x_de_letra = X_ASTERISCO;
			else if (dicho[h] == '+')
				x_de_letra = X_MAS;
			else if (dicho[h] == '1')
				x_de_letra = X_N1;
			else if (dicho[h] == '2')
				x_de_letra = X_N2;
			else if (dicho[h] == '3')
				x_de_letra = X_N3;
			else if (dicho[h] == '4')
				x_de_letra = X_N4;
			else if (dicho[h] == '5')
				x_de_letra = X_N5;
			else if (dicho[h] == '6')
				x_de_letra = X_N6;
			else if (dicho[h] == '7')
				x_de_letra = X_N7;
			else if (dicho[h] == '8')
				x_de_letra = X_N8;
			else if (dicho[h] == '9')
				x_de_letra = X_N9;
			else if (dicho[h] == '0')
				x_de_letra = X_N0;
			else
				y_de_letra = Y_ACENTOS;

			if (dicho[h] == ' ')
				x_de_letra = X_A;
			else if (dicho[h] == '‚')
				x_de_letra = X_B;
			else if (dicho[h] == '¡')
				x_de_letra = X_C;
			else if (dicho[h] == '¢')
				x_de_letra = X_D;
			else if (dicho[h] == '£')
				x_de_letra = X_E;
			else if (dicho[h] == '…')
				x_de_letra = X_F;
			else if (dicho[h] == 'Š')
				x_de_letra = X_G;
			else if (dicho[h] == '')
				x_de_letra = X_H;
			else if (dicho[h] == '•')
				x_de_letra = X_I;
			else if (dicho[h] == '—')
				x_de_letra = X_J;
			else if (dicho[h] == 'ƒ')
				x_de_letra = X_K;
			else if (dicho[h] == 'ˆ')
				x_de_letra = X_L;
			else if (dicho[h] == 'Œ')
				x_de_letra = X_M;
			else if (dicho[h] == '“')
				x_de_letra = X_N;
			else if (dicho[h] == '–')
				x_de_letra = X_GN;
			else if (dicho[h] == '\'')
				x_de_letra = X_O;
			else if (dicho[h] == '€')
				x_de_letra = X_P;
			else if (dicho[h] == '‡')
				x_de_letra = X_P;
			else if (dicho[h] == 'µ')
				x_de_letra = X_A;
			else if (dicho[h] == '')
				x_de_letra = X_B;
			else if (dicho[h] == 'Ö')
				x_de_letra = X_C;
			else if (dicho[h] == 'à')
				x_de_letra = X_D;
			else if (dicho[h] == 'é')
				x_de_letra = X_E;
			else if (dicho[h] == '·')
				x_de_letra = X_F;
			else if (dicho[h] == 'Ô')
				x_de_letra = X_G;
			else if (dicho[h] == 'Þ')
				x_de_letra = X_H;
			else if (dicho[h] == 'ã')
				x_de_letra = X_I;
			else if (dicho[h] == 'ë')
				x_de_letra = X_J;
			else if (dicho[h] == '¶')
				x_de_letra = X_K;
			else if (dicho[h] == 'Ò')
				x_de_letra = X_L;
			else if (dicho[h] == '×')
				x_de_letra = X_M;
			else if (dicho[h] == 'â')
				x_de_letra = X_N;
			else if (dicho[h] == 'ê')
				x_de_letra = X_GN;
		}                    
*/
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
	g_system->delayMillis(ms);
}

void DrasculaEngine::confirma_go() {
	color_abc(ROJO);
	refresca_pantalla();
	centra_texto(SYS0, 160, 87);        
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (;;) {
		key = getscan();
		if (key != 0)
			break;
	}

	if (key == 83) {
		stopmusic();
		carga_partida("gsave00");
	}
}

void DrasculaEngine::confirma_salir() {
	color_abc(ROJO);
	refresca_pantalla();
	centra_texto(SYS1, 160, 87);        
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	for (;;) {
		key = getscan();
		if (key != 0) 
			break;
	}

	if (key == ESC) {
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

void DrasculaEngine::fliplay(char filefli[], int vel) {
	return;
	// TODO
	OpenSSN(filefli, vel);
	while (PlayFrameSSN() && (!term_int)) {
		if (chkkey() == 27)
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
				palFundido[color][3] = 0;
			}
		}
		pausa(VelocidadDeFundido);

		setvgapalette256(&palFundido);
	}
}

void DrasculaEngine::color_abc(int cl) {
	color = cl;

	if (cl == 0) {
		palJuego[254][0] = 0;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0;
		palJuego[254][3] = 0;
	} else if (cl == 1) {
		palJuego[254][0] = 0x10;
		palJuego[254][1] = 0x3E;
		palJuego[254][2] = 0x28;
		palJuego[254][3] = 0;
	} else if (cl == 3) {
		palJuego[254][0] = 0x16;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x16;
		palJuego[254][3] = 0;
	} else if (cl == 4) {
		palJuego[254][0] = 0x9;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x12;
		palJuego[254][3] = 0;
	} else if (cl == 5) {
		palJuego[254][0] = 0x3F;
		palJuego[254][1] = 0x3F;
		palJuego[254][2] = 0x15;
		palJuego[254][3] = 0;
	} else if (cl == 7) {
		palJuego[254][0] = 0x38;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0;
		palJuego[254][3] = 0;
	} else if (cl == 8) {
		palJuego[254][0] = 0x3F;
		palJuego[254][1] = 0x27;
		palJuego[254][2] = 0x0B;
		palJuego[254][3] = 0;
	} else if (cl == 9) {
		palJuego[254][0] = 0x2A;
		palJuego[254][1] = 0;
		palJuego[254][2] = 0x2A;
		palJuego[254][3] = 0;
	} else if (cl == 10) {
		palJuego[254][0] = 0x30;
		palJuego[254][1] = 0x30;
		palJuego[254][2] = 0x30;
		palJuego[254][3] = 0;
	} else if (cl == 11) {
		palJuego[254][0] = 98;
		palJuego[254][1] = 91;
		palJuego[254][2] = 100;
		palJuego[254][3] = 0;
	};

	setvgapalette256(&palJuego);
}

char DrasculaEngine::LimitaVGA(char valor) {
	return (valor & 0x3F) * (valor > 0);
}

void DrasculaEngine::centra_texto(const char *mensaje, int x_texto, int y_texto) {
	char bb[190], m2[190], m1[190] ,mb[10][40];
	char m3[190];
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
		x_texto=60;
	if (x_texto > 255)
		x_texto=255;

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

void DrasculaEngine::comienza_sound(char fichero[]) {
	if (hay_sb == 1) { 
		if ((sku = fopen(fichero, "rb")) == NULL)  {
			error("no puedo abrir archivo de voz");
		}
	}
	ctvd_init(2);
	ctvd_speaker(1);
	ctvd_output(sku);
}

void DrasculaEngine::anima(char animacion[], int FPS) {
	Common::File FileIn;
	unsigned Org = 0, Des = 0, j, TotDes = 0;
	int NFrames = 1, New = 1;
	int cnt = 2;

	TimeMed = CLOCKS_PER_SEC / FPS;
	AuxBuffLast = (char *)malloc(65000);
	AuxBuffDes = (char *)malloc(65000);

	FileIn.open(animacion);

	if (!FileIn.isOpen()) {
		error("No encuentro un fichero de animacion.");
	}

	FileIn.read(&NFrames, sizeof(NFrames));
	FileIn.read(&Leng, sizeof(Leng));
	AuxBuffOrg = (char *)malloc(Leng);
	FileIn.read(AuxBuffOrg, Leng);
	FileIn.read(cPal, 768);
	carga_pcx(AuxBuffOrg);
	free(AuxBuffOrg);
	memcpy(VGA, AuxBuffDes, 64000);
	g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
	set_dac(cPal);
	memcpy(AuxBuffLast, AuxBuffDes, 64000);
	WaitForNext(TimeMed);
	while (cnt < NFrames) {
		FileIn.read(&Leng, sizeof(Leng));
		AuxBuffOrg = (char *)malloc(Leng);
		FileIn.read(AuxBuffOrg, Leng);
		FileIn.read(cPal, 768);
		carga_pcx(AuxBuffOrg);
		free(AuxBuffOrg);
		for (j = 0;j < 64000; j++) {
			VGA[j] = AuxBuffLast[j] = AuxBuffDes[j] ^ AuxBuffLast[j];
		}
		g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
		WaitForNext(TimeMed);
		cnt++;
		key = getscan();
		if (key == 0x01)
			term_int = 1;
		if (key !=0)
			break;
	}
	free(AuxBuffLast);
	free(AuxBuffDes);
	FileIn.close();
}

void DrasculaEngine::animafin_sound_corte() {
	if (hay_sb == 1) {
		ctvd_stop();
		fclose(sku);
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
				palFundido[color][3] = 0;
			}
		}
		pausa(VelocidadDeFundido);

		setvgapalette256(&palFundido);
	}
}

void DrasculaEngine::pausa(int cuanto) {
	int diferencia, conta_antes;

	conta_antes = vez();

	for (;;) {
		diferencia = vez() - conta_antes;
		if (diferencia >= 2 * cuanto)
			break;
	}
}

void DrasculaEngine::habla_dr_grande(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;
	int x_habla[4] = {47, 93, 139, 185};
	int cara;
	int l = 0;

	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) {
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 4;
    DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_FONDO(interf_x[l] + 24, interf_y[l], 0, 45, 39, 31, dir_dibujo2, dir_zona_pantalla);
	DIBUJA_FONDO(x_habla[cara], 1, 171, 68, 45, 48, dir_dibujo2, dir_zona_pantalla);
	l++;
	if (l == 7)
		l=0;

	if (con_voces == 0)
		centra_texto(dicho, 191, 69);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key == ESC)
		term_int = 1;

	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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
	if (sentido_igor == 3)
		pos_igor[1] = 138;
	else if (sentido_igor == 1)
		pos_igor[1] = 76;
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

void DrasculaEngine::habla_igor_dch(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 56, 82, 108, 134, 160, 186, 212, 238};
	int cara;
	
	int longitud;
	longitud = strlen(dicho);

	tiempol = time (NULL);
	tiempou = (unsigned int)tiempol / 2;    
	srand(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 8;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	pon_igor();
	pon_dr();
	DIBUJA_FONDO(x_igor + 17, y_igor, x_igor + 17, y_igor, 37, 24,
				dir_dibujo1, dir_zona_pantalla);

	DIBUJA_BLOQUE(x_habla[cara], 148, x_igor + 17, y_igor, 25, 24,
					dir_hare_frente, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_igor + 26, y_igor);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);
	
	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

void DrasculaEngine::habla_dr_izq(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;
	
	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL); // TODO
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) { 
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara=rand() % 8;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	pon_igor();
	pon_dr();

	DIBUJA_FONDO(x_dr, y_dr, x_dr, y_dr, 38, 31, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 90, x_dr, y_dr, 38, 31,
				 dir_hare_fondo, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_dr + 19, y_dr);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

void DrasculaEngine::habla_dr_dch(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 1, 40, 79, 118, 157, 196, 235, 274 };
	int cara;

	int longitud;
	longitud = strlen(dicho);

	// TODO
	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(ROJO);

	if (hay_sb == 1) {
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 8;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();

	pon_igor();
	pon_dr();

	DIBUJA_FONDO(x_dr, y_dr, x_dr, y_dr, 45, 31, dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 58, x_dr + 7, y_dr, 38, 31,
				dir_hare_fondo, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_dr + 19, y_dr);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

void DrasculaEngine::habla_solo(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	color_abc(color_solo);

	if (hay_sb == 1) { 
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
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

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
		goto bucless;
	}
}

void DrasculaEngine::habla_igor_frente(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int x_habla[8] = { 56, 86, 116, 146, 176, 206, 236, 266};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	// TODO
	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 8;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
		
	actualiza_refresco_antes();

	pon_igor();
	pon_dr();
	DIBUJA_FONDO(x_igor, y_igor, x_igor, y_igor, 29, 25,
				dir_dibujo1, dir_zona_pantalla);
	DIBUJA_BLOQUE(x_habla[cara], 173, x_igor, y_igor, 29, 25,
				dir_hare_frente, dir_zona_pantalla);

	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, x_igor + 26, y_igor);

	VUELCA_PANTALLA(0,0, 0,0, 320,200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

	int x_habla[9] = { 1, 23, 45, 67, 89, 111, 133, 155, 177};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(MARRON);

	if (hay_sb == 1) {
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	if (music_status() == 0)
		playmusic(musica_room);

	cara = rand() % 9;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 2, 121, 44, 21, 24, dir_hare_dch, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 132, 45);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

	contador = contador;

comienza:
	contador--;

	refresca_pantalla();        
	VUELCA_PANTALLA(0, 1, 0, y, 320, 198, dir_zona_pantalla);

	if (sentido == 0)
		y++;
	else
		y--;
	if (y == 2)
		sentido = 1;
	if (y == 0)
		sentido = 0;
	if (contador > 0)
		goto comienza;

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);
}

void DrasculaEngine::fin_sound() {
   delay(1);

	if (hay_sb == 1) {
		while (LookForFree() != 0);
		fclose(sku);
	}
}

void DrasculaEngine::habla_bj(char dicho[], char filename[]) {
	int tiempou;
	long tiempol;

	int x_habla[5] = { 64, 92, 120, 148, 176};
	int cara;

	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 5;

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

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	if (factor_red[hare_y + alto_hare] == 100)
		suma_1_pixel = 0;

	color_abc(AMARILLO);

	if (hay_sb == 1) {
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 6;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();
	DIBUJA_FONDO(hare_x, hare_y, ANCHOBJ+1, 0,
				 ((float)ancho_hare / 100) * factor_red[hare_y + alto_hare],
				 ((float)(alto_habla - 1) / 100) * factor_red[hare_y + alto_hare],
				 dir_zona_pantalla, dir_dibujo3);
	pon_hare();
	
	DIBUJA_FONDO(ANCHOBJ + 1, 0, hare_x, hare_y, 
				((float)ancho_hare / 100) * factor_red[hare_y + alto_hare],
				((float)(alto_habla - 1) / 100) * factor_red[hare_y + alto_hare],
				dir_dibujo3, dir_zona_pantalla);

	if (sentido_hare == 0) {
		reduce_hare_chico(x_habla_izq[cara], y_mask_habla,
						hare_x + ((float)8 / 100) * factor_red[hare_y + alto_hare],
						hare_y, ancho_habla, alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);

		actualiza_refresco();
	} else if (sentido_hare == 1) {
		reduce_hare_chico(x_habla_dch[cara], y_mask_habla,
						hare_x + ((float)12 / 100) * factor_red[hare_y + alto_hare],
						hare_y, ancho_habla,alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		actualiza_refresco();
	} else if (sentido_hare == 2) {
		reduce_hare_chico(x_habla_izq[cara], y_mask_habla,
						suma_1_pixel + hare_x + ((float)12 / 100) * factor_red[hare_y + alto_hare],
						hare_y, ancho_habla, alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		actualiza_refresco();
	} else if (sentido_hare == 3) {
		reduce_hare_chico(x_habla_dch[cara], y_mask_habla,
						suma_1_pixel + hare_x + ((float)8 / 100) * factor_red[hare_y + alto_hare],
						hare_y, ancho_habla,alto_habla, factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
		actualiza_refresco();
	}

	if (con_voces == 0)
		centra_texto(dicho, hare_x, hare_y);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key == ESC)
		term_int = 1;
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
		ctvd_terminate();
	} else {
		longitud = longitud - 2;
		if (longitud > 0)
			goto bucless;
	} 

	refresca_pantalla();
	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	if (music_status() == 0 && flags[11] == 0 && corta_musica == 0)
		playmusic(musica_room);
}

void DrasculaEngine::playmusic(int p) {
// TODO
/*	unsigned short v;
	stopmusic();

	v=GetCDVolume();
	if ((p==12 || p==21) && !reducido) {
		SetCDVolume(v-2);
		reducido = 1;
	}
	cd_track_length(p, &min, &sec, &frame);
	cd_set_track (p);
	get_musicpos();
	cd_play_audio(startpos, endpos);
	Playing=1;
*/
}

void DrasculaEngine::stopmusic() {            
//TODO
/*	unsigned short v;

	cd_stop_audio ();
	/v=GetCDVolume();
	if (reducido)
      {
        SetCDVolume(v+2);
        reducido=0;
      }
      cd_done_play ();
      Playing=0;
*/
}

int DrasculaEngine::music_status() {
	// TODO
	//cd_status();
	//return ((cdrom_data.error & BUSY) != 0);
	return 0;
}

void DrasculaEngine::refresca_pantalla() {
	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);
	actualiza_refresco_antes();
	pon_hare();
	actualiza_refresco();
}

void DrasculaEngine::carga_partida(char nom_game[]) {
	int l, n_ejec2;

	canal_p(nom_game);
	// TODO
	if ((sku = fopen(nom_game, "rb")) == NULL) {
		error("no puedo abrir el archivo");
	}
	fscanf(sku, "%d", &n_ejec2);
	if (n_ejec2 != num_ejec) {
		fclose(sku);
		canal_p(nom_game);
		strcpy(nom_partida, nom_game);
		salir_al_dos(n_ejec2);
	}
	fscanf(sku, "%s", datos_actuales);
	fscanf(sku, "%d", &hare_x);
	fscanf(sku, "%d", &hare_y);
	fscanf(sku, "%d", &sentido_hare);

	for (l = 1; l < 43; l++) {
		fscanf(sku, "%d", &objetos_que_tengo[l]);
	}

	for (l = 0; l < NUM_BANDERAS; l++) {
		fscanf(sku, "%d", &flags[l]);
	}

	fscanf(sku, "%d", &lleva_objeto);
	fscanf(sku, "%d", &objeto_que_lleva);

	fclose(sku);
	canal_p(nom_game);
}

void DrasculaEngine::canal_p(char fich[13]){
	Common::File ald2, ald3;

	char fich2[13];
	char car;

	strcpy(fich2, "top");

	// TODO
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
	// TODO
	remove(fich);
	rename(fich2, fich);
}

void DrasculaEngine::puertas_cerradas (int l) {}

void DrasculaEngine::color_hare() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++) {
			palJuego[color][componente] = palHare[color][componente];
		}
		palJuego[color][3] = 0;
	}
	ActualizaPaleta();
}

void DrasculaEngine::funde_hare(int oscuridad) {
	char fundido;
	unsigned int color, componente;

	for (fundido = oscuridad; fundido >= 0; fundido--) {
		for (color = 235; color < 253; color++) {
			for (componente = 0; componente < 3; componente++)
				palJuego[color][componente] = LimitaVGA( palJuego[color][componente] - 8 + fundido );
			palJuego[color][3] = 0;
		}
	}

	ActualizaPaleta();
}

void DrasculaEngine::paleta_hare_claro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palHareClaro[color][componente] = palJuego[color][componente];
		palHareClaro[color][3] = 0;
	}
}

void DrasculaEngine::paleta_hare_oscuro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palHareOscuro[color][componente] = palJuego[color][componente];
		palHareOscuro[color][3] = 0;
	}
}

void DrasculaEngine::hare_claro() {
	int color, componente;

	for (color = 235; color < 253; color++) {
		for (componente = 0; componente < 3; componente++)
			palJuego[color][componente] = palHareClaro[color][componente];
		palJuego[color][3] = 0;
	}

	ActualizaPaleta();
}

void DrasculaEngine::empieza_andar() {
	hare_se_mueve = 1;

	paso_x = PASO_HARE_X;
	paso_y = PASO_HARE_Y;

	if ((sitio_x < hare_x + ancho_hare / 2 ) && (sitio_y <= (hare_y + alto_hare)))
		cuadrante_1();                        
	else if ((sitio_x < hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
		cuadrante_3();                        
	else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y <= (hare_y + alto_hare)))
		cuadrante_2();                        
	else if ((sitio_x > hare_x + ancho_hare / 2) && (sitio_y > (hare_y + alto_hare)))
		cuadrante_4();                        
	else
		hare_se_mueve=0;

	conta_vez = vez();
}

void DrasculaEngine::actualiza_refresco() {	
	if (!strcmp(num_room, "63.alg"))
		refresca_63();
	else if (!strcmp(num_room, "62.alg"))
		refresca_62();
}

void DrasculaEngine::actualiza_refresco_antes() {
	if (!strcmp(num_room, "62.alg"))
		refresca_62_antes();
	else if (!strcmp(num_room, "16.alg"))
		pon_bj();
}

void DrasculaEngine::pon_hare() {
	int pos_hare[6];
	int r;

	if (hare_se_mueve == 1 && paso_x == PASO_HARE_X) {
		for (r = 0; r < paso_x; r++) {
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

	if (hare_se_ve == 0)
		goto no_vuelco; 

	if (hare_se_mueve == 0) {
		pos_hare[0] = 0;
		pos_hare[1] = DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		pos_hare[4] = ANCHO_PERSONAJE;
		pos_hare[5] = ALTO_PERSONAJE;

		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1)
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		else if (sentido_hare == 2)
			reduce_hare_chico( pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_fondo, dir_zona_pantalla);
		else 
			reduce_hare_chico( pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);
	} else if (hare_se_mueve == 1) {
		pos_hare[0] = frame_x[num_frame];
		pos_hare[1] = frame_y + DIF_MASK_HARE;
		pos_hare[2] = hare_x;
		pos_hare[3] = hare_y;
		pos_hare[4] = ANCHO_PERSONAJE;
		pos_hare[5] = ALTO_PERSONAJE;
		if (sentido_hare == 0) {
			pos_hare[1] = 0;
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		} else if (sentido_hare == 1)
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_dch, dir_zona_pantalla);
		else if (sentido_hare == 2)
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_fondo, dir_zona_pantalla);
		else 
			reduce_hare_chico(pos_hare[0], pos_hare[1],
						pos_hare[2], pos_hare[3],
						pos_hare[4], pos_hare[5],
						factor_red[hare_y + alto_hare],
						dir_hare_frente, dir_zona_pantalla);

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
			DIBUJA_FONDO(x_pol[n], y_pol[n], x_obj[n], y_obj[n],
					ANCHOBJ, ALTOBJ, dir_hare_frente,  dir_zona_pantalla);

		DIBUJA_BLOQUE(x1d_menu[h], y1d_menu[h], x_obj[n], y_obj[n],
				ANCHOBJ, ALTOBJ, dir_hare_fondo, dir_zona_pantalla);
	}

	if (x < 7)
		print_abc(texto_icono,x_obj[x] - 2, y_obj[x] - 7);
}

void DrasculaEngine::barra_menu() {
	int n, sobre_verbo = 1;

	for (n = 0; n < 7; n++) {
		if (x_raton > x_barra[n] && x_raton < x_barra[n + 1])
			sobre_verbo = 0;
		DIBUJA_BLOQUE(ANCHOBJ * n, ALTOBJ * sobre_verbo,
				 x_barra[n], 2, ANCHOBJ, ALTOBJ,
				 dir_hare_fondo,  dir_zona_pantalla);
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

void DrasculaEngine::sal_de_la_habitacion(int l) {
	char salgo[13];
	
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
				animacion_2();
			borra_pantalla();
			strcpy(salgo, alapantallakeva[l]);
			strcat(salgo, ".ald");
			hare_x = -1;
			carga_escoba(salgo);
		}
	}
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

	if (objeto_que_lleva == MIRAR && fl == 28)
		hablar(TEXT328, "328.als");
	} else {     
		if (objeto_que_lleva == MIRAR && fl == 50)
			hablar(TEXT308, "308.als");
		else if (objeto_que_lleva == ABRIR && fl == 50)
			hablar(TEXT310, "310.als" );
		else if (objeto_que_lleva == CERRAR && fl == 50)
			hablar(TEXT311, "311.als" );
		else if (objeto_que_lleva == MOVER && fl == 50)
			hablar(TEXT312, "312.als" );
		else if (objeto_que_lleva == COGER && fl == 50)
			hablar(TEXT313, "313.als" ); 
		else if (objeto_que_lleva == HABLAR && fl == 50)
			hablar(TEXT314,"314.als" );
		else if (!strcmp(num_room, "62.alg"))
			pantalla_62(fl);  
		else if (!strcmp(num_room, "63.alg"))
			pantalla_63(fl);  
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
	char key;
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
/*			if (key == 16)
				select2[v] = 'q';
			else if (key == 17)
				select2[v] = 'w';
			else if (key == 18)
				select2[v] = 'e';
			else if (key == 19)
				select2[v] = 'r';
			else if (key == 20)
				select2[v] = 't';
			else if (key == 21)
				select2[v] = 'y';
			else if (key == 22)
				select2[v] = 'u';
			else if (key == 23)
				select2[v] = 'i';
			else if (key == 24)
				select2[v] = 'o';
			else if (key == 25)
				select2[v] = 'p';
			else if (key == 30)
				select2[v] = 'a';
			else if (key == 31)
				select2[v] = 's';
			else if (key == 32)
				select2[v] = 'd';
			else if (key == 33)
				select2[v] = 'f';
			else if (key == 34)
				select2[v] = 'g';
			else if (key == 35)
				select2[v] = 'h';
			else if (key == 36)
				select2[v] = 'j';
			else if (key == 37)
				select2[v] = 'k';
			else if (key == 38)
				select2[v] = 'l';
			else if (key == 39)
				select2[v] = '¤';
			else if (key == 44)
				select2[v] = 'z';
			else if (key == 45)
				select2[v] = 'x';
			else if (key == 46)
				select2[v] = 'c';
			else if (key == 47)
				select2[v] = 'v';
			else if (key == 48)
				select2[v] = 'b';
			else if (key == 49)
				select2[v] = 'n';
			else if (key == 50)
				select2[v] = 'm';
			else if (key == 2)
				select2[v] = '1';
			else if (key == 3)
				select2[v] = '2';
			else if (key == 4)
				select2[v] = '3';
			else if (key == 5)
				select2[v] = '4';
			else if (key == 6)
				select2[v] = '5';
			else if (key == 7)
				select2[v] = '6';
			else if (key == 8)
				select2[v] = '7';
			else if (key == 9)
				select2[v] = '8';
			else if (key == 10)
				select2[v] = '9';
			else if (key == 11)
				select2[v] = '0';
			else if (key == 57)
				select2[v] = '§';
			else if (key == ESC)
				break;
			else if (key == 0x1C) {
					select2[v] = '\0';
					h = 1;
					break;
			} else if (key == 0x0E)
				select2[v] = '\0'; 
			else
				v--;
*/
			if (key == 0x0E)
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

int DrasculaEngine::LookForFree() {
	delay(10);
	// TODO GAME_Poll(gamev, 10);
	//return(!SDEV_ChannelFree(gamev->EffectDev, 0));
	return 1;
}

void DrasculaEngine::OpenSSN(char *Name, int Pause) {
	MiVideoSSN = (char *)malloc(64256);
	GlobalSpeed = CLOCKS_PER_SEC / Pause;
	FrameSSN = 0;
	UsingMem = 0;
	if (MiVideoSSN == NULL)
		return;
	Sesion = new Common::File;
	Sesion->open(Name);
	mSesion = TryInMem(Sesion);
	LastFrame = clock();
}

int DrasculaEngine::PlayFrameSSN() {
	int Exit = 0;
	int Lengt;
	char *BufferSSN;

	if (!UsingMem)
		Sesion->read(&CHUNK, 1);
	else {
		memcpy(&CHUNK, mSesion, 1);
		mSesion += 1;
	}
	switch (CHUNK) {
		case SET_PALET:                                
			if (!UsingMem)
				Sesion->read(dacSSN, 768);
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
				Sesion->read(&CMP, 1);
				Sesion->read(&Lengt, 4);                                        
			} else {
				memcpy(&CMP, mSesion, 1);
				mSesion += 1;
				memcpy(&Lengt, mSesion, 4);
				mSesion += 4;
			}                                
			if (CMP == CMP_RLE) {
				if (!UsingMem) {
					BufferSSN = (char *)malloc(Lengt);
					Sesion->read(BufferSSN, Lengt);
				} else {
					BufferSSN = (char *)malloc(Lengt);
					memcpy(BufferSSN, mSesion, Lengt);
					mSesion += Lengt;
				}                                         
				Des_RLE(BufferSSN, MiVideoSSN);
				free(BufferSSN);
				if (FrameSSN) {
					WaitFrameSSN();
					MixVideo(VGA, MiVideoSSN);
					g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
				} else {
					WaitFrameSSN();
					memcpy(VGA, MiVideoSSN, 64000);
					g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
				}
				FrameSSN++;
			} else {
				if (CMP == CMP_OFF) {                                        
					if (!UsingMem) {
						BufferSSN = (char *)malloc(Lengt);
						Sesion->read(BufferSSN, Lengt);
					} else {
						BufferSSN = (char *)malloc(Lengt);
						memcpy(BufferSSN, mSesion, Lengt);
						mSesion += Lengt;
					}
					Des_OFF(BufferSSN, MiVideoSSN, Lengt);                                        
					free(BufferSSN);
					if (FrameSSN) {
						WaitFrameSSN();
						MixVideo(VGA, MiVideoSSN);
						g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
					} else {
						WaitFrameSSN();
						memcpy(VGA, MiVideoSSN, 64000);
						g_system->copyRectToScreen((const byte *)VGA, 320, 0, 0, 320, 200);
					}
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
		Sesion->close();
		delete Sesion;
	}
}

char *DrasculaEngine::TryInMem(Common::File *Sesion) {
	int Lengt;

	Sesion->seek(0, SEEK_END);
	Lengt = Sesion->pos();        
	Sesion->seek(0, SEEK_SET);
	pointer = (char *)malloc(Lengt);        
	if (pointer == NULL)
		return NULL;
	Sesion->read(pointer, Lengt);
	UsingMem = 1;
	Sesion->close();
	delete Sesion;
	return pointer;
}

void DrasculaEngine::set_dacSSN(char *dacSSN) {
	int n = 0;
// TODO
/*
	while (inp(0x3da)&8){};
	while ((inp(0x3da)&8)==0){};

  outp(0x3c8,0);
  do {
        outp(0x3c9,dacSSN[n++]);
        outp(0x3c9,dacSSN[n++]);
        outp(0x3c9,dacSSN[n++]);
        } while (n<768);
*/
}

void DrasculaEngine::Des_OFF(char *BufferOFF, char *MiVideoOFF, int Lenght) {
	int x = 0, Reps;
	int Offset;

	memset(MiVideoSSN, 0, 64000);
	while (x < Lenght) {
		Offset = BufferOFF[x] + BufferOFF[x + 1] * 256;
		Reps = BufferOFF[x + 2];
		memcpy(MiVideoOFF + Offset, &BufferOFF[x + 3], Reps);
		x += 3 + Reps;
	}
}

void DrasculaEngine::Des_RLE(char *BufferRLE, char *MiVideoRLE) {
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

void DrasculaEngine::MixVideo(char *OldScreen,char *NewScreen) {
	int x;
    for (x = 0; x < 64000; x++)
		OldScreen[x] ^= NewScreen[x];
}

void DrasculaEngine::WaitFrameSSN() {
	while (clock() < LastFrame + GlobalSpeed) {};
	LastFrame = LastFrame + GlobalSpeed;
}

int DrasculaEngine::chkkey() {
	//TODO
/*
	union REGS registros;

    registros.h.ah=0x06;
    registros.h.dl=0xff;
    intdos(&registros,&registros);
    return(registros.h.al);
*/
	return 0;
}

char *DrasculaEngine::carga_pcx(char *NamePcc) {
	signed int con = 0;
	unsigned int X = 0;
	unsigned int fExit = 0;
	char ch, rep;
	char *AuxPun;

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

void DrasculaEngine::set_dac(char *dac) {
	int n = 0;
	/*retrazo();
	// TODO
	outp(0x3c8,0);
	do {

		outp(0x3c9,dac[n++]);
		outp(0x3c9,dac[n++]);
		outp(0x3c9,dac[n++]);

	} while (n<768);
	*/
}

void DrasculaEngine::WaitForNext(long TimeMed) {
	TimeLast = clock();
	while (clock() < (TimeLast + TimeMed)) {}
	TimeLast = clock();
}

float DrasculaEngine::vez() {
/* TODO
	float trasvase;
	union REGS in, out;

    in.h.ah=0;
        int386(0x1A,&in,&out);
        trasvase=((out.w.cx*65536)+out.w.dx)/0.182;
	return(trasvase);
*/
	return 0;
}

void DrasculaEngine::reduce_hare_chico(int x1,int y1, int x2,int y2, int ancho,int alto, int factor, char *dir_inicio, char *dir_fin) {
	float suma_x, suma_y;
	int n, m;
	float pixel_x, pixel_y;
	int pos_pixel[6];        

	nuevo_ancho = (ancho * factor) / 100;
	nuevo_alto = (alto * factor) / 100;

	suma_x = ancho / nuevo_ancho;
	suma_y = alto / nuevo_alto;

	pixel_x = x1;
	pixel_y = y1;

	for (n = 0;n < nuevo_alto; n++){
		for (m = 0; m < nuevo_ancho; m++){
		      pos_pixel[0] = pixel_x;
		      pos_pixel[1] = pixel_y;
		      pos_pixel[2] = x2 + m;
		      pos_pixel[3] = y2 + n;
		      pos_pixel[4] = 1;
		      pos_pixel[5] = 1;

			  DIBUJA_BLOQUE_CUT(pos_pixel, dir_inicio, dir_fin);

			  pixel_x = pixel_x + suma_x;
		}
		pixel_x = x1;
		pixel_y = pixel_y + suma_y;
	}
}

char DrasculaEngine::codifica(char car) {
	return ~car;
}

void DrasculaEngine::cuadrante_1() {
	float distancia_x, distancia_y;

	distancia_x = hare_x + ancho_hare / 2 - sitio_x;
	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 0;
		sentido_hare = 2;
		paso_x = distancia_x / (distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 7;
		sentido_hare = 0;
		paso_y = distancia_y / (distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_2() {
	float distancia_x, distancia_y;

	distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);
	distancia_y = (hare_y + alto_hare) - sitio_y;

	if (distancia_x < distancia_y) {
		direccion_hare = 1;
		sentido_hare = 2;
		paso_x = distancia_x / (distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 2;
		sentido_hare = 1;
		paso_y = distancia_y / (distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_3() {
	float distancia_x, distancia_y;
      
	distancia_x = hare_x + ancho_hare / 2 - sitio_x;
	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x < distancia_y) {
		direccion_hare = 5;
		sentido_hare = 3;
		paso_x = distancia_x / (distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 6;
		sentido_hare = 0;
		paso_y = distancia_y / (distancia_x / PASO_HARE_X);
	}
}

void DrasculaEngine::cuadrante_4() {
	float distancia_x, distancia_y;

	distancia_x = abs(hare_x + ancho_hare / 2 - sitio_x);
	distancia_y = sitio_y - (hare_y + alto_hare);

	if (distancia_x <distancia_y) {
		direccion_hare = 4;
		sentido_hare = 3;
		paso_x = distancia_x / (distancia_y / PASO_HARE_Y);
	} else {
		direccion_hare = 3;
		sentido_hare = 1;
		paso_y = distancia_y / (distancia_x / PASO_HARE_X);
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
	int velas_y[] = { 158, 172, 186};
	int cirio_x[] = { 14, 19, 24 };
	int pianista_x[] = {1, 91, 61, 31, 91, 31, 1, 61, 31 };
	int borracho_x[] = {1, 42, 83, 124, 165, 206, 247, 1 };
	int diferencia;          

	DIBUJA_FONDO(123, velas_y[frame_velas], 142, 14, 39, 13,
				dir_dibujo3, dir_zona_pantalla);
	DIBUJA_FONDO(cirio_x[frame_velas], 146, 311, 80, 4, 8,
				dir_dibujo3, dir_zona_pantalla);

	if (parpadeo == 5)
		DIBUJA_FONDO(1, 149, 127, 52, 9, 5, dir_dibujo3, dir_zona_pantalla);

	if (hare_x > 101 && hare_x < 155)
		DIBUJA_FONDO(31, 138, 178, 51, 18, 16, dir_dibujo3, dir_zona_pantalla);

	if (flags[11] == 0)
		DIBUJA_FONDO(pianista_x[frame_piano], 157, 245, 130, 29, 42,
					dir_dibujo3, dir_zona_pantalla);
	else if (flags[5] == 0)
		DIBUJA_FONDO(145, 139, 228, 112, 47, 60, dir_hare_dch, dir_zona_pantalla);
	else
		DIBUJA_FONDO(165, 140, 229, 117, 43, 59, dir_dibujo3, dir_zona_pantalla);

	if (flags[12] == 1)
		DIBUJA_FONDO(borracho_x[frame_borracho], 82, 170, 50, 40, 53,
					dir_dibujo3, dir_zona_pantalla);

	diferencia = vez() - conta_ciego_vez;
	if (diferencia > 6) {
		if (flags[12] == 1) {
			frame_borracho++;
			if (frame_borracho == 8) {
				frame_borracho = 0;
				flags[12] = 0;
			}
		} else if (((rand() % 95) == 15) && (flags[13] == 0))
			flags[12] = 1;
		 
		frame_velas++;
		if (frame_velas == 3)
			frame_velas = 0;
		frame_piano++;
		if (frame_piano == 9)
			frame_piano = 0;
		parpadeo = rand() % 11;          
		conta_ciego_vez = vez();
	}
}

void DrasculaEngine::graba_partida(char nom_game[]) {
	FILE *scu;
	int l;

	// TODO
	if ((scu = fopen(nom_game, "w")) == NULL)  {
		error("no puedo abrir el archivo");
	}
	fprintf(scu, "%d\n", num_ejec);
	fprintf(scu, "%s\n", datos_actuales);
	fprintf(scu, "%d\n", hare_x);
	fprintf(scu, "%d\n", hare_y);
	fprintf(scu, "%d\n", sentido_hare);

	for (l = 1; l < 43; l++) {
		fprintf(scu,"%d\n", objetos_que_tengo[l]);
	}

	for (l = 0; l < NUM_BANDERAS; l++) {
		fprintf(scu, "%d\n", flags[l]);
	}

	fprintf(scu, "%d\n", lleva_objeto);
	fprintf(scu, "%d\n", objeto_que_lleva);

	fclose(scu);
	canal_p(nom_game);
}

void DrasculaEngine::aumenta_num_frame() {
	diff_vez=vez()-conta_vez;

	if (diff_vez >= 5.7) {
		conta_vez = vez();
		num_frame++;
		if (num_frame == 6)
			num_frame = 0;

		if (direccion_hare == 0){ 
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

	diferencia_y = alto_hare - nuevo_alto;
	diferencia_x = ancho_hare - nuevo_ancho;
	hare_y = hare_y + diferencia_y;
	hare_x = hare_x + diferencia_x;
	alto_hare = nuevo_alto;
	ancho_hare = nuevo_ancho;
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

	for (n = 0; n < 43; n++){
		if (sobre_que_objeto() == n) {
			h = objetos_que_tengo[n];
			if (h != 0)
				banderas(h);
		}
	}
}

void DrasculaEngine::pantalla_0() {
	if (objeto_que_lleva == MIRAR)
		hablar(TEXT54, "54.als");
	else if (objeto_que_lleva == MOVER)
		hablar(TEXT19, "19.als" );
	else if (objeto_que_lleva == COGER)
		hablar(TEXT11, "11.als" ); 
	else if (objeto_que_lleva == ABRIR)
		hablar(TEXT9, "9.als" ); 
	else if (objeto_que_lleva == CERRAR)
		hablar(TEXT9, "9.als" );
	else if (objeto_que_lleva == HABLAR)
		hablar(TEXT16, "16.als" );
	else
		hablar(TEXT11, "11.als");
}

void DrasculaEngine::pantalla_62(int fl) {
	if (objeto_que_lleva == HABLAR && fl == 53)
		conversa("op_13.cal");
	else if (objeto_que_lleva == HABLAR && fl == 52 && flags[0] == 0)
		animacion_3();
	else if (objeto_que_lleva == HABLAR && fl == 52 && flags[0] == 1)
		hablar(TEXT109, "109.als");
	else if (objeto_que_lleva == HABLAR && fl == 54)
		animacion_4();
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

	rompo_y_salgo = 0;
	
	strcpy(para_codificar, nom_fich);
	canal_p(para_codificar); 

	// TODO
	if ((ald = fopen(nom_fich, "rb")) == NULL) {
		error("no puedo abrir el archivo");
	}
	fscanf(ald, "%s", frase1);
	fscanf(ald, "%s", frase2);
	fscanf(ald, "%s", frase3);
	fscanf(ald, "%s", frase4);
	fscanf(ald, "%s", suena1);
	fscanf(ald, "%s", suena2);
	fscanf(ald, "%s", suena3);
	fscanf(ald, "%s", suena4);
	fscanf(ald, "%d", &respuesta1);
	fscanf(ald, "%d", &respuesta2);
	fscanf(ald, "%d", &respuesta3);
	fclose(ald);
	canal_p(para_codificar); 

	longitud = strlen(frase1);
/*	for (h = 0; h < longitud; h++)
		if (frase1[h] == '§')
			frase1[h] = ' ';

	longitud = strlen(frase2);
	for (h=  0; h < longitud; h++)
		if (frase2[h] == '§')
			frase2[h] = ' ';

	longitud = strlen(frase3);
	for (h = 0; h < longitud; h++)
		if (frase3[h] == '§')
			frase3[h] = ' ';

	longitud = strlen(frase4);
	for (h = 0; h < longitud; h++)
		if (frase4[h] == '§')
			frase4[h] = ' ';
*/
	lee_dibujos("car.alg");
	descomprime_dibujo(dir_hare_fondo,1);
/* TODO
	ent.w.ax =   8;
	ent.w.cx =   1;
	ent.w.dx =  31;
	int386(0x33, &ent, &sal);
*/	
	color_abc(VERDE_CLARO);

bucle_opc:

	refresca_pantalla();
  
	if (music_status() == 0 && flags[11] == 0)
		playmusic(musica_room);  

	MirarRaton();

	if ( y_raton > 0 && y_raton < 9) {
		if (usado1 == 1 && color != BLANCO)
			color_abc(BLANCO);
		else if (usado1 == 0 && color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (y_raton > 8 && y_raton < 17){ 
		if (usado2==1 && color!=BLANCO)
			color_abc(BLANCO);
		else if (usado2 == 0 && color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (y_raton > 16 && y_raton < 25) {
		if (usado3 == 1 && color != BLANCO)
			color_abc(BLANCO);
		else if (usado3 == 0 && color != VERDE_CLARO)
			color_abc(VERDE_CLARO);
	} else if (color != VERDE_CLARO)
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
		responde(respuesta1);
	} else if ((boton_izq == 1) && (juego2 == 2)) { 
		usado2 = 1;
		hablar(frase2, suena2);
		responde(respuesta2);
	} else if ((boton_izq == 1) && (juego3 == 2)) { 
		usado3 = 1;
		hablar(frase3, suena3);
		responde(respuesta3);
	} else if ((boton_izq == 1) && (juego4 == 2)) {
		hablar(frase4, suena4);
		rompo_y_salgo=1;
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

	lee_dibujos("99.alg");
	descomprime_dibujo(dir_hare_fondo, 1);
	sin_verbo();
}

void DrasculaEngine::animacion_3() {
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

void DrasculaEngine::animacion_4() {
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

void DrasculaEngine::print_abc_opc(char dicho[],int x_pantalla, int y_pantalla, int juego) {
	int pos_texto[6];
	int i, y_de_signos, y_de_letra, x_de_letra = 0, h, longitud;
	longitud = strlen(dicho);

	for (i = 0; dicho[i]; i++)
		dicho[i] = toupper(dicho[i]);

	for (h = 0; h < longitud; h++) {
		if (juego == 1) {
			y_de_letra = Y_ABC_OPC_1;
			y_de_signos=Y_SIGNOS_OPC_1;
		} else if (juego == 3) {
			y_de_letra = Y_ABC_OPC_3;
			y_de_signos = Y_SIGNOS_OPC_3;
		} else {
			y_de_letra = Y_ABC_OPC_2;
			y_de_signos = Y_SIGNOS_OPC_2;
		}
/*
		if (dicho[h] == 'A')
			x_de_letra = X_A_OPC;
		else if (dicho[h] == 'µ')
			x_de_letra = X_A_OPC;
		else if (dicho[h] == '·')
			x_de_letra = X_A_OPC;
		else if (dicho[h] == '¶')
			x_de_letra = X_A_OPC;
		else if (dicho[h] == 'B'
			)x_de_letra = X_B_OPC;
		else if (dicho[h] == 'C')
			x_de_letra = X_C_OPC;
		else if (dicho[h] == '€')
			x_de_letra = X_C_OPC;
		else if (dicho[h] == '‡')
			x_de_letra = X_C_OPC;
		else if (dicho[h] == 'D')
			x_de_letra = X_D_OPC;
		else if (dicho[h] == 'E')
			x_de_letra = X_E_OPC;
		else if (dicho[h] == '')
			x_de_letra = X_E_OPC;
		else if (dicho[h] == 'Ô')
			x_de_letra = X_E_OPC;
		else if (dicho[h] == 'Ò')
			x_de_letra = X_E_OPC;
		else if (dicho[h] == 'F')
			x_de_letra = X_F_OPC;
		else if (dicho[h] == 'G')
			x_de_letra = X_G_OPC;
		else if (dicho[h] == 'H')
			x_de_letra = X_H_OPC;
		else if (dicho[h] == 'I')
			x_de_letra = X_I_OPC;
		else if (dicho[h] == 'Ö')
			x_de_letra = X_I_OPC;
		else if (dicho[h] == 'Þ')
			x_de_letra = X_I_OPC;
		else if (dicho[h] == '×')
			x_de_letra = X_I_OPC;
		else if (dicho[h] == 'J')
			x_de_letra = X_J_OPC;
		else if (dicho[h] == 'K')
			x_de_letra = X_K_OPC;
		else if (dicho[h] == 'L')
			x_de_letra = X_L_OPC;
		else if (dicho[h] == 'M')
			x_de_letra = X_M_OPC;
		else if (dicho[h] == 'N')
			x_de_letra = X_N_OPC;
		else if (dicho[h] == '\'')
			x_de_letra = X_GN_OPC;
		else if (dicho[h] == 'O')
			x_de_letra = X_O_OPC;
		else if (dicho[h] == 'à')
			x_de_letra = X_O_OPC;
		else if (dicho[h] == 'â')
			x_de_letra = X_O_OPC;
		else if (dicho[h] == 'ã')
			x_de_letra = X_O_OPC;
		else if (dicho[h] == 'P')
			x_de_letra = X_P_OPC;
		else if (dicho[h] == 'Q')
			x_de_letra = X_Q_OPC;
		else if (dicho[h] == 'R')
			x_de_letra = X_R_OPC;
		else if (dicho[h] == 'S')
			x_de_letra = X_S_OPC;
		else if (dicho[h] == 'T')
			x_de_letra = X_T_OPC;
		else if (dicho[h] == 'U')
			x_de_letra = X_U_OPC;
		else if (dicho[h] == 'ë')
			x_de_letra = X_U_OPC;
		else if (dicho[h] == 'ê')
			x_de_letra = X_U_OPC;
		else if (dicho[h] == 'é')
			x_de_letra = X_U_OPC;
		else if (dicho[h] == 'V')
			x_de_letra = X_V_OPC;
		else if (dicho[h] == 'W')
			x_de_letra = X_W_OPC;
		else if (dicho[h] == 'X')
			x_de_letra = X_X_OPC;
		else if (dicho[h] == 'Y')
			x_de_letra = X_Y_OPC;
		else if (dicho[h] == 'Z')
			x_de_letra = X_Z_OPC;
		else if (dicho[h] == ' ')
			x_de_letra = ESPACIO_OPC;
		else
			y_de_letra = y_de_signos;

		if (dicho[h] == '.')
			x_de_letra = X_PUNTO_OPC;
		else if (dicho[h] == ',')
			x_de_letra = X_COMA_OPC;
		else if (dicho[h] == '-')
			x_de_letra = X_GUION_OPC;
		else if (dicho[h] == '?')
			x_de_letra = X_CIERRA_INTERROGACION_OPC;
		else if (dicho[h] == '¨')
			x_de_letra = X_ABRE_INTERROGACION_OPC;
		else if (dicho[h] == '"')
			x_de_letra = X_COMILLAS_OPC;
		else if (dicho[h] == '!')
			x_de_letra = X_CIERRA_EXCLAMACION_OPC;
		else if (dicho[h] == '­')
			x_de_letra = X_ABRE_EXCLAMACION_OPC;
		else if (dicho[h] == ';')
			x_de_letra = X_PUNTO_Y_COMA_OPC;
		else if (dicho[h] == '>')
			x_de_letra = X_MAYOR_QUE_OPC;
		else if (dicho[h] == '<')
			x_de_letra = X_MENOR_QUE_OPC;
		else if (dicho[h] == '$')
			x_de_letra = X_DOLAR_OPC;
		else if (dicho[h] == '%')
			x_de_letra = X_POR_CIENTO_OPC;
		else if (dicho[h] == ':')
			x_de_letra = X_DOS_PUNTOS_OPC;
		else if (dicho[h] == '&')
			x_de_letra = X_AND_OPC;
		else if (dicho[h] == '/')
			x_de_letra = X_BARRA_OPC;
		else if (dicho[h] == '(')
			x_de_letra = X_ABRE_PARENTESIS_OPC;
		else if (dicho[h] == ')')
			x_de_letra = X_CIERRA_PARENTESIS_OPC;
		else if (dicho[h] == '*')
			x_de_letra = X_ASTERISCO_OPC;
		else if (dicho[h] == '+')
			x_de_letra = X_MAS_OPC;
		else if (dicho[h] == '1')
			x_de_letra = X_N1_OPC;
		else if (dicho[h] == '2')
			x_de_letra = X_N2_OPC;
		else if (dicho[h] == '3')
			x_de_letra = X_N3_OPC;
		else if (dicho[h] == '4')
			x_de_letra = X_N4_OPC;
		else if (dicho[h] == '5')
			x_de_letra = X_N5_OPC;
		else if (dicho[h] == '6')
			x_de_letra = X_N6_OPC;
		else if (dicho[h] == '7')
			x_de_letra = X_N7_OPC;
		else if (dicho[h] == '8')
			x_de_letra = X_N8_OPC;
		else if (dicho[h] == '9')
			x_de_letra = X_N9_OPC;
		else if (dicho[h] == '0')
			x_de_letra = X_N0_OPC;
*/
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
	if (funcion == 10)
		habla_borracho(TEXTB1, "B1.als"); 
	else if (funcion == 11)
		habla_borracho(TEXTB2, "B2.als");
	else if (funcion == 12)
		habla_borracho(TEXTB3, "B3.als");
}

void DrasculaEngine::habla_pianista(const char *dicho, const char *filename) {
	int tiempou;
	long tiempol;
	int x_habla[4] = { 97, 145, 193, 241};
	int cara;
	int longitud;
	longitud = strlen(dicho);

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

	buffer_teclado();

	color_abc(BLANCO);

	if (hay_sb == 1) {
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 4;

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

	key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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

	tiempol = time(NULL);
	tiempou = (unsigned int)tiempol / 2;
	srand(tiempou);

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
		// TODO
		if ((sku = fopen(filename, "rb")) == NULL) {
			error("no puedo abrir archivo de voz\n");
		}
		ctvd_init(2);
		ctvd_speaker(1);
		ctvd_output(sku);
	}

bucless:

	cara = rand() % 8;

	DIBUJA_FONDO(0, 0, 0, 0, 320, 200, dir_dibujo1, dir_zona_pantalla);

	actualiza_refresco_antes();

	DIBUJA_FONDO(x_habla[cara], 29, 177, 50, 19, 19, dir_hare_frente, dir_zona_pantalla);
	pon_hare();
	actualiza_refresco();

	if (con_voces == 0)
		centra_texto(dicho, 181, 54);

	VUELCA_PANTALLA(0, 0, 0, 0, 320, 200, dir_zona_pantalla);

	pausa(3);

	key = getscan();
	if (key != 0)
		ctvd_stop();
	buffer_teclado();
	if (hay_sb == 1) {
		if (LookForFree() != 0)
			goto bucless;
		fclose(sku);
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
				objetos_que_tengo[h]=osj;
				puesto = 1;
				break;
			}
		}
	}
}

void DrasculaEngine::fin_sound_corte() {
	if (hay_sb == 1) { 
		ctvd_stop();
		fclose(sku);
		ctvd_terminate();
	}
}

void DrasculaEngine::MusicFadeout() {
	//TODO
}

void DrasculaEngine::ctvd_end() {
	//TODO
}

void DrasculaEngine::ctvd_stop() {
	//TODO
}

void DrasculaEngine::ctvd_terminate() {
	//TODO
}

void DrasculaEngine::ctvd_speaker(int flag) {}

void DrasculaEngine::ctvd_output(FILE *file_handle) {}

void DrasculaEngine::ctvd_init(int b) {
	//TODO
}


} // End of namespace Drascula

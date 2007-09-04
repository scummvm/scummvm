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

#ifndef DRASCULA_H
#define DRASCULA_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Drascula {

enum DrasculaGameFeatures {
};

struct DrasculaGameDescription;

#define NUM_SAVES     10
#define NUM_BANDERAS  50
#define ESC          0x01
#define F1           0x3B
#define F2           0x3C
#define F3           0x3D
#define F4           0x3E
#define F5           0x3F
#define F6           0x40
#define F8           0x42
#define F9           0x43
#define F10          0x44
#define MIRAR           1
#define COGER           2
#define ABRIR           3
#define CERRAR          4
#define HABLAR          5
#define MOVER           6
#define INICISOUND      6
#define FINALSOUND      8
#define FINDRV          9
#define DIF_MASK       55
#define ANCHOBJ        40
#define ALTOBJ         25

#define X_OBJ1         5
#define Y_OBJ1         10
#define X_OBJ2         50
#define Y_OBJ2         10
#define X_OBJ3         95
#define Y_OBJ3         10
#define X_OBJ4         140
#define Y_OBJ4         10
#define X_OBJ5         185
#define Y_OBJ5         10
#define X_OBJ6         230
#define Y_OBJ6         10
#define X_OBJ7         275
#define Y_OBJ7         10
#define X_OBJ8         5
#define Y_OBJ8         40
#define X_OBJ9         50
#define Y_OBJ9         40
#define X_OBJ10        95
#define Y_OBJ10        40
#define X_OBJ11        140
#define Y_OBJ11        40
#define X_OBJ12        185
#define Y_OBJ12        40
#define X_OBJ13        230
#define Y_OBJ13        40
#define X_OBJ14        275
#define Y_OBJ14        40
#define X_OBJ15        5
#define Y_OBJ15        70
#define X_OBJ16        50
#define Y_OBJ16        70
#define X_OBJ17        95
#define Y_OBJ17        70
#define X_OBJ18        140
#define Y_OBJ18        70
#define X_OBJ19        185
#define Y_OBJ19        70
#define X_OBJ20        230
#define Y_OBJ20        70
#define X_OBJ21        275
#define Y_OBJ21        70
#define X_OBJ22        5
#define Y_OBJ22        100
#define X_OBJ23        50
#define Y_OBJ23        100
#define X_OBJ24        95
#define Y_OBJ24        100
#define X_OBJ25        140
#define Y_OBJ25        100
#define X_OBJ26        185
#define Y_OBJ26        100
#define X_OBJ27        230
#define Y_OBJ27        100
#define X_OBJ28        275
#define Y_OBJ28        100
#define X_OBJ29        5
#define Y_OBJ29        130
#define X_OBJ30        50
#define Y_OBJ30        130
#define X_OBJ31        95
#define Y_OBJ31        130
#define X_OBJ32        140
#define Y_OBJ32        130
#define X_OBJ33        185
#define Y_OBJ33        130
#define X_OBJ34        230
#define Y_OBJ34        130
#define X_OBJ35        275
#define Y_OBJ35        130
#define X_OBJ36        5
#define Y_OBJ36        160
#define X_OBJ37        50
#define Y_OBJ37        160
#define X_OBJ38        95
#define Y_OBJ38        160
#define X_OBJ39        140
#define Y_OBJ39        160
#define X_OBJ40        185
#define Y_OBJ40        160
#define X_OBJ41        230
#define Y_OBJ41        160
#define X_OBJ42        275
#define Y_OBJ42        160
#define X_OBJ43        275
#define Y_OBJ43        160

#define DIF_MASK_HARE   72
#define DIF_MASK_ABC    22
#define ANCHO_LETRAS     8
#define ALTO_LETRAS      6

#define Y_ABC            158
#define Y_SIGNOS         169
#define Y_ACENTOS        180

#define X_A              6
#define X_B              15
#define X_C              24
#define X_D              33
#define X_E              42
#define X_F              51
#define X_G              60
#define X_H              69
#define X_I              78
#define X_J              87
#define X_K              96
#define X_L             105
#define X_M             114
#define X_N             123
#define X_GN            132
#define X_O             141
#define X_P             150
#define X_Q             159
#define X_R             168
#define X_S             177
#define X_T             186
#define X_U             195
#define X_V             204
#define X_W             213
#define X_X             222
#define X_Y             231
#define X_Z             240
#define X_PUNTO          6
#define X_COMA           15
#define X_GUION          24
#define X_CIERRA_INTERROGACION        33
#define X_ABRE_INTERROGACION          42
#define X_COMILLAS       51
#define X_CIERRA_EXCLAMACION         60
#define X_ABRE_EXCLAMACION              69
#define X_PUNTO_Y_COMA              78
#define X_MAYOR_QUE              87
#define X_MENOR_QUE              96
#define X_DOLAR             105
#define X_POR_CIENTO             114
#define X_DOS_PUNTOS             123
#define X_AND            132
#define X_BARRA             141
#define X_ABRE_PARENTESIS             150
#define X_CIERRA_PARENTESIS             159
#define X_ASTERISCO             168
#define X_MAS             177
#define X_N1             186
#define X_N2            195
#define X_N3            204
#define X_N4            213
#define X_N5            222
#define X_N6            231
#define X_N7            240
#define X_N8            249
#define X_N9            258
#define X_N0            267
#define ESPACIO         250
#define ALTO_HABLA_HARE  25
#define ANCHO_HABLA_HARE 23
#define VON_BRAUN        1
#define AZUL_OSCURO       2
#define VERDE_CLARO       3
#define VERDE_OSCURO      4
#define AMARILLO          5
#define NARANJA           6
#define ROJO              7
#define MARRON            8
#define MORADO            9
#define BLANCO           10
#define ROSA             11
#define PASO_HARE_X       8
#define PASO_HARE_Y       3
#define ALTO_PERSONAJE   70
#define ANCHO_PERSONAJE  43
#define PIES_HARE        12

#define ANCHO_LETRAS_OPC     6
#define ALTO_LETRAS_OPC      5
#define Y_ABC_OPC_1          6
#define Y_SIGNOS_OPC_1       15
#define Y_ABC_OPC_2          31
#define Y_SIGNOS_OPC_2       40
#define Y_ABC_OPC_3          56
#define Y_SIGNOS_OPC_3       65
#define X_A_OPC              10
#define X_B_OPC              17
#define X_C_OPC              24
#define X_D_OPC              31
#define X_E_OPC              38
#define X_F_OPC              45
#define X_G_OPC              52
#define X_H_OPC              59
#define X_I_OPC              66
#define X_J_OPC              73
#define X_K_OPC              80
#define X_L_OPC              87
#define X_M_OPC              94
#define X_N_OPC             101
#define X_GN_OPC            108
#define X_O_OPC             115
#define X_P_OPC             122
#define X_Q_OPC             129
#define X_R_OPC             136
#define X_S_OPC             143
#define X_T_OPC             150
#define X_U_OPC             157
#define X_V_OPC             164
#define X_W_OPC             171
#define X_X_OPC             178
#define X_Y_OPC             185
#define X_Z_OPC             192
#define ESPACIO_OPC         199
#define X_PUNTO_OPC          10
#define X_COMA_OPC           17
#define X_GUION_OPC          24
#define X_CIERRA_INTERROGACION_OPC        31
#define X_ABRE_INTERROGACION_OPC          38
#define X_COMILLAS_OPC       45
#define X_CIERRA_EXCLAMACION_OPC         52
#define X_ABRE_EXCLAMACION_OPC              59
#define X_PUNTO_Y_COMA_OPC              66
#define X_MAYOR_QUE_OPC              73
#define X_MENOR_QUE_OPC              80
#define X_DOLAR_OPC             87
#define X_POR_CIENTO_OPC            94
#define X_DOS_PUNTOS_OPC             101
#define X_AND_OPC            108
#define X_BARRA_OPC             115
#define X_ABRE_PARENTESIS_OPC             122
#define X_CIERRA_PARENTESIS_OPC             129
#define X_ASTERISCO_OPC             136
#define X_MAS_OPC             143
#define X_N1_OPC             150
#define X_N2_OPC            157
#define X_N3_OPC            164
#define X_N4_OPC            171
#define X_N5_OPC            178
#define X_N6_OPC            185
#define X_N7_OPC            192
#define X_N8_OPC            199
#define X_N9_OPC            206
#define X_N0_OPC            213
#define NO_PUERTA            99

#define INIT_FRAME  0
#define CMP_RLE     1
#define CMP_OFF     2
#define END_ANIM    3
#define SET_PALET   4
#define MOUSE_KEY   5
#define EMPTY_FRAME 6

#define COMPLETA    256
#define MEDIA       128

class DrasculaEngine : public ::Engine {
	int _gameId;
	Common::KeyState _keyPressed;

protected:

	int init();
	int go();
//	void shutdown();

	bool initGame();

public:
	DrasculaEngine(OSystem *syst);
	virtual ~DrasculaEngine();
	int getGameId() {
		return _gameId;
	}

	Common::RandomSource *_rnd;
	const DrasculaGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	void update_events();

	Audio::SoundHandle _soundHandle;

	void asigna_memoria();
	void libera_memoria();
	void carga_info();
	void salir_al_dos(int r);

	void lee_dibujos(const char *);
	void descomprime_dibujo(byte *dir_escritura, int plt);

	typedef char DacPalette256[256][3];

	void asigna_rgb(byte *dir_lectura, int plt);
	void funde_rgb(int plt);
	void paleta_hare();
	void ActualizaPaleta();
	void setvgapalette256(byte *PalBuf);
	void DIBUJA_FONDO(int xorg, int yorg, int xdes, int ydes, int Ancho,
				int Alto, byte *Origen, byte *Destino);
	void DIBUJA_BLOQUE(int xorg, int yorg, int xdes, int ydes, int Ancho,
				int Alto, byte *Origen, byte *Destino);
	void DIBUJA_BLOQUE_CUT(int *Array, byte *Origen, byte *Destino);
	void VUELCA_PANTALLA(int xorg, int yorg, int xdes, int ydes, int Ancho, int Alto, byte *Buffer);

	DacPalette256 palJuego;
	DacPalette256 palHare;
	DacPalette256 palHareClaro;
	DacPalette256 palHareOscuro;

	byte *VGA;

	byte *dir_dibujo1;
	byte *dir_hare_fondo;
	byte *dir_dibujo3;
	byte *dir_dibujo2;
	byte *dir_mesa;
	byte *dir_hare_dch;
	byte *dir_zona_pantalla;
	byte *dir_hare_frente;
	byte *dir_texto;
	byte *dir_pendulo;

	byte cPal[768];
	byte *Buffer_pcx;
	long LenFile;

	Common::File *ald, *sku;

	int hay_sb;
	int nivel_osc, musica_antes, musica_room;
	char num_room[20], pantalla_disco[20];
	char datos_actuales[20];
	int objs_room;
	char fondo_y_menu[20];

	char nombre_obj[30][20];
	char nombre_icono[14][20];

	int num_obj[40], visible[40], espuerta[40];
	int sitiobj_x[40], sitiobj_y[40], sentidobj[40];
	int objetos_que_tengo[50];
	char alapantallakeva[40][20];
	int x_alakeva[40], y_alakeva[40], sentido_alkeva[40], alapuertakeva[40];
	int x1[40], y1[40], x2[40], y2[40];
	int lleva_objeto , objeto_que_lleva;
	int con_voces;
	int menu_bar, menu_scr, hay_nombre;
	char texto_nombre[20];
	int frame_ciego;
	int frame_ronquido;
	int frame_murcielago;
	int c_mirar;
	int c_poder;

	int flags[NUM_BANDERAS];

	int frame_y;
	int hare_x, hare_y, hare_se_mueve, direccion_hare, sentido_hare, num_frame, hare_se_ve;
	int sitio_x, sitio_y, comprueba_flags;
	int rompo, rompo2;
	int paso_x, paso_y;
	int alto_hare, ancho_hare, alto_pies;
	int alto_habla, ancho_habla;
	int suelo_x1, suelo_y1, suelo_x2, suelo_y2;
	int cerca, lejos;
	int sentido_final, anda_a_objeto;
	int obj_saliendo;
	float diff_vez, conta_vez;
	int hay_respuesta;
	int conta_ciego_vez;
	int cambio_de_color;
	int rompo_y_salgo;
	int vb_x, sentido_vb, vb_se_mueve, frame_vb;
	float nuevo_alto, nuevo_ancho;
	int diferencia_x, diferencia_y;
	int factor_red[202];
	int frame_piano;
	int frame_borracho;
	int frame_velas;
	int color_solo;
	int parpadeo;
	int x_igor, y_igor, sentido_igor;
	int x_dr, y_dr, sentido_dr;
	int x_bj, y_bj, sentido_bj;
	int cont_sv;
	int term_int;
	int num_ejec;
	int cual_ejec, hay_que_load;
	char nom_partida[13];
	int _color;
	int corta_musica;
	char select[23];
	int hay_seleccion;
	int x_raton;
	int y_raton;
	int y_raton_ant;
	int boton_izq;
	int boton_dch;

	bool escoba();
	void Negro();
	void habla_vb(const char *, const char *);
	void habla_vbpuerta(const char *dicho, const char *filename);
	void habla_ciego(const char *, const char *, const char *);
	void habla_hacker(const char *, const char *);
	void agarra_objeto(int);
	void anda_parriba();
	void anda_pabajo();
	void pon_vb();
	void lleva_vb(int punto_x);
	void hipo_sin_nadie(int contador);
	void abre_puerta(int nflag, int n_puerta);
	void mapa();
	void buffer_teclado() { }
	void animacion_1_1();
	bool animacion_2_1();
	void animacion_1_2();
	void animacion_2_2();
	void animacion_3_1();
	void animacion_4_1();
	void animacion_3_2();
	void animacion_4_2();
	void animacion_8();
	void animacion_9();
	void animacion_10();
	void animacion_11();
	void animacion_12();
	void animacion_13();
	void animacion_14();
	void animacion_15();
	void animacion_16();
	void animacion_17();
	void animacion_18();
	void animacion_19();
	void animacion_20();
	void animacion_21();
	void animacion_22();
	void animacion_23();
	void animacion_23_anexo();
	void animacion_23_anexo2();
	void animacion_24();
	void animacion_25();
	void animacion_26();
	void animacion_27();
	void animacion_28();
	void animacion_29();
	void animacion_30();
	void animacion_31();
	void animacion_32();
	void animacion_33();
	void animacion_34();
	void animacion_35();
	void animacion_36();

	void refresca_1_antes();
	void refresca_2();
	void refresca_3();
	void refresca_3_antes();
	void refresca_4();
	void refresca_5();
	void refresca_5_antes();
	void refresca_6_antes();
	void refresca_7_antes();
	void refresca_9_antes();
	void refresca_12_antes();
	void refresca_14_antes();
	void refresca_15();
	void refresca_16_antes();
	void refresca_17_antes();
	void refresca_17();
	void refresca_18_antes();
	void refresca_18();
	void hare_oscuro();


	void sin_verbo();
	void para_cargar(char[]);
	void carga_escoba_1(const char *);
	void carga_escoba_2(const char *);
	void borra_pantalla();
	void lleva_al_hare(int, int);
	void mueve_cursor();
	void comprueba_objetos();
	void espera_soltar();
	void MirarRaton();
	void elige_en_barra();
	bool comprueba1();
	void comprueba2();
	Common::KeyCode getscan();
	void elige_verbo(int);
	void mesa();
	void saves();
	void print_abc(const char *, int, int);
	void delay(int ms);
	void confirma_go();
	void confirma_salir();
	void salva_pantallas();
	void elige_objeto(int objeto);
	void suma_objeto(int);
	int resta_objeto(int osj);
	void fliplay(const char *filefli, int vel);
	void FundeDelNegro(int VelocidadDeFundido);
	char LimitaVGA(char valor);
	void color_abc(int cl);
	void centra_texto(const char *,int,int);
	void comienza_sound(const char *);
	void anima(const char *animacion, int FPS);
	void fin_sound_corte();
	void FundeAlNegro(int VelocidadDeFundido);
	void pausa(int);
	void habla_dr_grande(const char *dicho, const char *filename);
	void pon_igor();
	void pon_bj();
	void pon_dr();
	void habla_igor_dch(const char *dicho, const char *filename);
	void habla_dr_dch(const char *dicho, const char *filename);
	void habla_dr_izq(const char *dicho, const char *filename);
	void habla_solo(const char *, const char *);
	void habla_igor_frente(const char *, const char *);
	void habla_tabernero(const char *dicho, const char *filename);
	void hipo(int);
	void fin_sound();
	void habla_bj(const char *, const char *);
	void hablar(const char *, const char *);
	void playmusic(int p);
	void stopmusic();
	int music_status();
	void refresca_pantalla();
	void carga_partida(const char *);
	void canal_p(const char *);
	void puertas_cerradas(int);
	void animafin_sound_corte();
	void color_hare();
	void funde_hare(int oscuridad);
	void paleta_hare_claro();
	void paleta_hare_oscuro();
	void hare_claro();
	void actualiza_datos();
	void empieza_andar();
	void actualiza_refresco();
	void actualiza_refresco_antes();
	void pon_hare();
	void menu_sin_volcar();
	void barra_menu();
	void saca_objeto();
	bool sal_de_la_habitacion(int);
	void coge_objeto();
	void banderas(int);
	void cursor_mesa();
	void introduce_nombre();
	void para_grabar(char[]);
	int LookForFree();
	void OpenSSN(const char *Name, int Pause);
	void WaitFrameSSN();
	void MixVideo(byte *OldScreen, byte *NewScreen);
	void Des_RLE(byte *BufferRLE, byte *MiVideoRLE);
	void Des_OFF(byte *BufferOFF, byte *MiVideoOFF, int Lenght);
	void set_dacSSN(byte *dacSSN);
	byte *TryInMem(Common::File *Sesion);
	void EndSSN();
	int PlayFrameSSN();

	byte *AuxBuffOrg;
	byte *AuxBuffLast;
	byte *AuxBuffDes;
	int Leng;

	byte *pointer;
	int UsingMem;
	Common::File *_Sesion;
	byte CHUNK;
	byte CMP, dacSSN[768];
	byte *MiVideoSSN;
	byte *mSesion;
	int FrameSSN;
	int GlobalSpeed;
	int LastFrame;

	byte *carga_pcx(byte *NamePcc);
	void set_dac(byte *dac);
	void WaitForNext(int FPS);
	float vez();
	void reduce_hare_chico(int, int, int, int, int, int, int, byte *, byte *);
	char codifica(char);
	void cuadrante_1();
	void cuadrante_2();
	void cuadrante_3();
	void cuadrante_4();
	void refresca_62();
	void refresca_62_antes();
	void refresca_63();
	void graba_partida(char[]);
	void aumenta_num_frame();
	int sobre_que_objeto();
	void comprueba_banderas_menu();
	void pantalla_0();
	void pantalla_62(int);
	void pantalla_63(int);
	void conversa(const char *);
	void print_abc_opc(const char *, int, int, int);
	void responde(int);
	void habla_borracho(const char *dicho, const char *filename);
	void habla_pianista(const char *dicho, const char *filename);

	void MusicFadeout();
	void ctvd_end();
	void ctvd_stop();
	void ctvd_terminate();
	void ctvd_speaker(int flag);
	void ctvd_output(Common::File *file_handle);
	void ctvd_init(int b);
	void grr();
	void pantalla_13(int fl);
	void refresca_13();
	void refresca_20();
	void animacion_1_3();
	void animacion_2_3();
	void animacion_3_3();
	void animacion_4_3();
	void animacion_5();
	void animacion_6();
	void animacion_7_4();
	void animacion_rayo();
	void animacion_2_4();
	void animacion_3_4();
	void animacion_4_4();
	void animacion_1_5();
	void animacion_2_5();
	void animacion_3_5();
	void animacion_4_5();
	void animacion_5_5();
	void animacion_6_5();
	void animacion_7_5();
	void animacion_8_5();
	void animacion_9_5();
	void animacion_10_5();
	void animacion_11_5();
	void animacion_12_5();
	void animacion_13_5();
	void animacion_14_5();
	void animacion_15_5();
	void animacion_16_5();
	void animacion_17_5();
	void pantalla_49(int);
	void pantalla_53(int);
	void pantalla_54(int);
	void pantalla_55(int);
	void pantalla_56(int);
	void refresca_53_antes();
	void refresca_54_antes();
	void refresca_49_antes();
	void refresca_56_antes();
	void refresca_50();
	void refresca_57();
	void habla_igor_sentado(const char *, const char *);
	void habla_lobo(char dicho[], char filename[]);
	void habla_mus(char dicho[], char filename[]);


private:

public:

};

} // End of namespace Drascula

#endif /* DRASCULA_H */

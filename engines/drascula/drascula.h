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

enum Languages {
	kEnglish = 0,
	kSpanish = 1,
	kGerman = 2,
	kFrench = 3,
	kItalian = 4
};

#define TEXTD_START 68

struct DrasculaGameDescription;

#define NUM_SAVES     10
#define NUM_FLAGS     50
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
#define LOOK            1
#define PICK            2
#define OPEN            3
#define CLOSE           4
#define TALK            5
#define MOVE            6
#define DIF_MASK       55
#define OBJWIDTH        40
#define OBJHEIGHT         25

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
#define CHAR_WIDTH     8
#define CHAR_HEIGHT      6

#define Y_ABC            158
#define Y_ABC_ESP        149
#define Y_SIGNOS         169
#define Y_SIGNOS_ESP     160
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
#define X_DOT             6
#define X_COMA           15
#define X_HYPHEN          24
#define X_CIERRA_INTERROGACION        33
#define X_ABRE_INTERROGACION          42
#define X_COMILLAS       51
#define X_CIERRA_EXCLAMACION         60
#define X_ABRE_EXCLAMACION              69
#define X_PUNTO_Y_COMA              78
#define X_GREATER_THAN              87
#define X_LESSER_THAN              96
#define X_DOLAR             105
#define X_PERCENT             114
#define X_DOS_PUNTOS             123
#define X_AND            132
#define X_BARRA             141
#define X_BRACKET_OPEN             150
#define X_BRACKET_CLOSE             159
#define X_ASTERISCO             168
#define X_PLUS             177
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
#define SPACE           250
#define ALTO_TALK_HARE  25
#define ANCHO_TALK_HARE 23
#define VON_BRAUN        1
#define DARK_BLUE         2
#define LIGHT_GREEN       3
#define DARK_GREEN        4
#define YELLOW            5
#define ORANGE            6
#define RED               7
#define MAROON            8
#define PURPLE            9
#define WHITE            10
#define PINK             11
#define PASO_HARE_X       8
#define PASO_HARE_Y       3
#define CHARACTER_HEIGHT   70
#define CHARACTER_WIDTH  43
#define PIES_HARE        12

#define CHAR_WIDTH_OPC     6
#define CHAR_HEIGHT_OPC      5
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
#define SPACE_OPC           199
#define X_DOT_OPC            10
#define X_COMA_OPC           17
#define X_HYPHEN_OPC          24
#define X_CIERRA_INTERROGACION_OPC        31
#define X_ABRE_INTERROGACION_OPC          38
#define X_COMILLAS_OPC       45
#define X_CIERRA_EXCLAMACION_OPC         52
#define X_ABRE_EXCLAMACION_OPC              59
#define X_PUNTO_Y_COMA_OPC              66
#define X_GREATER_THAN_OPC              73
#define X_LESSER_THAN_OPC              80
#define X_DOLAR_OPC             87
#define X_PERCENT_OPC            94
#define X_DOS_PUNTOS_OPC             101
#define X_AND_OPC            108
#define X_BARRA_OPC             115
#define X_BRACKET_OPEN_OPC             122
#define X_BRACKET_CLOSE_OPC             129
#define X_ASTERISCO_OPC             136
#define X_PLUS_OPC             143
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
#define NO_DOOR              99

#define INIT_FRAME  0
#define CMP_RLE     1
#define CMP_OFF     2
#define END_ANIM    3
#define SET_PAL     4
#define MOUSE_KEY   5
#define EMPTY_FRAME 6

#define COMPLETE_PAL   256
#define HALF_PAL       128

class DrasculaEngine : public ::Engine {
	int _gameId;
	Common::KeyState _keyPressed;

protected:
	int init();
	int go();
//	void shutdown();

public:
	DrasculaEngine(OSystem *syst, const DrasculaGameDescription *gameDesc);
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
	Common::Language getLanguage() const;
	void updateEvents();

	Audio::SoundHandle _soundHandle;

	void allocMemory();
	void freeMemory();
	void releaseGame();

	void loadPic(const char *);
	void decompressPic(byte *dir_escritura, int plt);

	typedef char DacPalette256[256][3];

	void setRGB(byte *dir_lectura, int plt);
	void paleta_hare();
	void updatePalette();
	void setPalette(byte *PalBuf);
	void copyBackground(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);
	void copyRect(int xorg, int yorg, int xdes, int ydes, int width,
				int height, byte *src, byte *dest);
	void copyRectClip(int *Array, byte *src, byte *dest);
	void updateScreen(int xorg, int yorg, int xdes, int ydes, int width, int height, byte *buffer);

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
	int nivel_osc, previousMusic, roomMusic;
	char num_room[20], roomDisk[20];
	char currentData[20];
	int numRoomObjs;
	char fondo_y_menu[20];

	char objName[30][20];
	char iconName[44][13];

	int num_obj[40], visible[40], isDoor[40];
	int sitiobj_x[40], sitiobj_y[40], sentidobj[40];
	int objetos_que_tengo[43];
	char alapantallakeva[40][20];
	int x_alakeva[40], y_alakeva[40], sentido_alkeva[40], alapuertakeva[40];
	int x1[40], y1[40], x2[40], y2[40];
	int lleva_objeto, objeto_que_lleva;
	int withVoices;
	int menu_bar, menu_scr, hay_nombre;
	char texto_nombre[20];
	int frame_ciego;
	int frame_ronquido;
	int frame_murcielago;
	int c_mirar;
	int c_poder;

	int flags[NUM_FLAGS];

	int frame_y;
	int hare_x, hare_y, hare_se_mueve, direccion_hare, sentido_hare, num_frame, hare_se_ve;
	int sitio_x, sitio_y, comprueba_flags;
	int rompo, rompo2;
	int step_x, step_y;
	int alto_hare, ancho_hare, alto_pies;
	int alto_talk, ancho_talk;
	int suelo_x1, suelo_y1, suelo_x2, suelo_y2;
	int near, far;
	int sentido_final, anda_a_objeto;
	int obj_saliendo;
	int diff_vez, conta_vez;
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
	int hay_que_load;
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
	void black();
	void talk_vb(const char *, const char *);
	void talk_vbpuerta(const char *said, const char *filename);
	void talk_ciego(const char *, const char *, const char *);
	void talk_hacker(const char *, const char *);
	void pickObject(int);
	void anda_parriba();
	void anda_pabajo();
	void pon_vb();
	void lleva_vb(int punto_x);
	void hipo_sin_nadie(int counter);
	void openDoor(int nflag, int n_puerta);
	void mapa();
	void animation_1_1();
	void animation_2_1();
	void animation_1_2();
	void animation_2_2();
	void animation_3_1();
	void animation_4_1();
	void animation_3_2();
	void animation_4_2();
	void animation_5_2();
	void animation_6_2();
	void animation_7_2();
	void animation_8_2();
	void animation_9_2();
	void animation_10_2();
	void animation_11_2();
	void animation_12_2();
	void animation_13_2();
	void animation_14_2();
	void animation_15_2();
	void animation_16_2();
	void animation_17_2();
	void animation_18_2();
	void animation_19_2();
	void animation_20_2();
	void animation_21_2();
	void animation_22_2();
	void animation_23_2();
	void animation_23_anexo();
	void animation_23_anexo2();
	void animation_24_2();
	void animation_25_2();
	void animation_26_2();
	void animation_27_2();
	void animation_28_2();
	void animation_29_2();
	void animation_30_2();
	void animation_31_2();
	void animation_32_2();
	void animation_33_2();
	void animation_34_2();
	void animation_35_2();
	void animation_36_2();

	void update_1_pre();
	void update_2();
	void update_3();
	void update_3_pre();
	void update_4();
	void update_5();
	void update_5_pre();
	void update_6_pre();
	void update_7_pre();
	void update_9_pre();
	void update_12_pre();
	void update_14_pre();
	void update_15();
	void update_16_pre();
	void update_17_pre();
	void update_17();
	void update_18_pre();
	void update_18();
	void update_21_pre();
	void update_22_pre();
	void update_23_pre();
	void update_24_pre();
	void update_26_pre();
	void update_26();
	void update_27();
	void update_27_pre();
	void update_29();
	void update_29_pre();
	void update_30_pre();
	void update_31_pre();
	void update_34_pre();
	void update_35_pre();
	void update_31();
	void update_34();
	void update_35();
	void hare_oscuro();


	void withoutVerb();
	bool para_cargar(char[]);
	void carga_escoba(const char *);
	void clearRoom();
	void lleva_al_hare(int, int);
	void mueve_cursor();
	void comprueba_objetos();
	void espera_soltar();
	void MirarRaton();
	void elige_en_barra();
	bool comprueba1();
	bool comprueba2();
	Common::KeyCode getscan();
	void elige_verbo(int);
	void mesa();
	bool saves();
	void print_abc(const char *, int, int);
	void delay(int ms);
	bool confirma_salir();
	void salva_pantallas();
	void chooseObject(int objeto);
	void suma_objeto(int);
	int resta_objeto(int osj);
	void fliplay(const char *filefli, int vel);
	void FundeDelNegro(int VelocidadDeFundido);
	char LimitaVGA(char valor);
	void color_abc(int cl);
	void centra_texto(const char *,int,int);
	void comienza_sound(const char *);
	bool anima(const char *animation, int FPS);
	void fin_sound_corte();
	void FundeAlNegro(int VelocidadDeFundido);
	void pause(int);
	void talk_dr_grande(const char *said, const char *filename);
	void pon_igor();
	void pon_bj();
	void pon_dr();
	void talk_igor_dch(const char *said, const char *filename);
	void talk_dr_dch(const char *said, const char *filename);
	void talk_dr_izq(const char *said, const char *filename);
	void talk_solo(const char *, const char *);
	void talk_igor_frente(const char *, const char *);
	void talk_tabernero(const char *said, const char *filename);
	void talk_igorpuerta(const char *said, const char *filename);
	void talk_igor_peluca(const char *said, const char *filename);
	void hipo(int);
	void fin_sound();
	void talk_bj(const char *, const char *);
	void talk_baul(const char *said, const char *filename);
	void talk(const char *, const char *);
	void talk_sinc(const char *, const char *, const char *);
	void cierra_puerta(int nflag, int n_puerta);
	void playMusic(int p);
	void stopMusic();
	int musicStatus();
	void updateRoom();
	bool carga_partida(const char *);
	void puertas_cerradas(int);
	void animafin_sound_corte();
	void color_hare();
	void funde_hare(int oscuridad);
	void paleta_hare_claro();
	void paleta_hare_oscuro();
	void hare_claro();
	void updateData();
	void empieza_andar();
	void updateRefresh();
	void updateRefresh_pre();
	void pon_hare();
	void menu_sin_volcar();
	void barra_menu();
	void saca_objeto();
	bool sal_de_la_habitacion(int);
	bool coge_objeto();
	bool banderas(int);
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
	uint32 LastFrame;

	int frame_pen;
	int flag_tv;

	byte *loadPCX(byte *NamePcc);
	void set_dac(byte *dac);
	void WaitForNext(int FPS);
	int vez();
	void reduce_hare_chico(int, int, int, int, int, int, int, byte *, byte *);
	char codifica(char);
	void cuadrante_1();
	void cuadrante_2();
	void cuadrante_3();
	void cuadrante_4();
	void update_62();
	void update_62_pre();
	void update_63();
	void saveGame(char[]);
	void aumenta_num_frame();
	int sobre_que_objeto();
	bool comprueba_banderas_menu();
	void room_0();
	void room_1(int);
	void room_2(int);
	void room_3(int);
	void room_4(int);
	void room_5(int);
	void room_6(int);
	void room_7(int);
	void room_8(int);
	void room_9(int);
	void room_12(int);
	void room_14(int);
	void room_15(int);
	void room_16(int);
	void room_17(int);
	void room_18(int);
	void room_19(int);
	bool room_21(int);
	void room_22(int);
	void room_23(int);
	void room_24(int);
	void room_26(int);
	void room_27(int);
	void room_29(int);
	void room_30(int);
	void room_31(int);
	void room_34(int);
	void room_35(int);
	void room_44(int);
	void room_62(int);
	void room_63(int);
	void conversa(const char *);
	void print_abc_opc(const char *, int, int, int);
	void responde(int);
	void talk_borracho(const char *said, const char *filename);
	void talk_pianista(const char *said, const char *filename);

	void MusicFadeout();
	void ctvd_end();
	void ctvd_stop();
	void ctvd_terminate();
	void ctvd_speaker(int flag);
	void ctvd_output(Common::File *file_handle);
	void ctvd_init(int b);
	void grr();
	bool room_13(int fl);
	void update_13();
	void update_20();
	void animation_1_3();
	void animation_2_3();
	void animation_3_3();
	void animation_4_3();
	void animation_5_3();
	void animation_6_3();
	void animation_rayo();
	void animation_1_4();
	void animation_2_4();
	void animation_3_4();
	void animation_4_4();
	void animation_5_4();
	void animation_6_4();
	void animation_7_4();
	void animation_8_4();
	void animation_9_4();
	void animation_1_5();
	void animation_2_5();
	void animation_3_5();
	void animation_4_5();
	void animation_5_5();
	void animation_6_5();
	void animation_7_5();
	void animation_8_5();
	void animation_9_5();
	void animation_10_5();
	void animation_11_5();
	void animation_12_5();
	void animation_13_5();
	void animation_14_5();
	void animation_15_5();
	void animation_16_5();
	void animation_17_5();
	void room_49(int);
	void room_53(int);
	void room_54(int);
	void room_55(int);
	bool room_56(int);
	void update_53_pre();
	void update_54_pre();
	void update_49_pre();
	void update_56_pre();
	void update_50();
	void update_57();
	void talk_igor_sentado(const char *, const char *);
	void talk_lobo(const char *said, const char *filename);
	void talk_mus(const char *said, const char *filename);
	void room_58(int);
	void room_59(int);
	bool room_60(int);
	void room_61(int);
	void room_pendulo(int);
	void update_pendulo();
	void update_58();
	void update_58_pre();
	void update_59_pre();
	void update_60_pre();
	void update_60();
	void update_61();
	void animation_1_6();
	void animation_2_6();
	void animation_3_6();
	void animation_4_6();
	void animation_5_6();
	void animation_6_6();
	void animation_7_6();
	void animation_9_6();
	void animation_10_6();
	void animation_11_6();
	void animation_12_6();
	void animation_13_6();
	void animation_14_6();
	void animation_15_6();
	void animation_18_6();
	void animation_19_6();
	void activa_pendulo();
	void talk_pen(const char *, const char *);
	void talk_pen2(const char *, const char *);
	void talk_taber2(const char *, const char *);
	void talk_bj_cama(const char *said, const char * filename);
	void talk_htel(const char *said, const char *filename);

private:
	int _lang;
};

extern const char *_text[][501];
extern const char *_textd[][84];
extern const char *_textb[][15];
extern const char *_textbj[][29];
extern const char *_texte[][24];
extern const char *_texti[][33];
extern const char *_textl[][32];
extern const char *_textp[][20];
extern const char *_textt[][25];
extern const char *_textvb[][63];
extern const char *_textsys[][4];
extern const char *_texthis[][5];
extern const char *_textverbs[][6];
extern const char *_textmisc[][2];
extern const char *_textd1[][11];

} // End of namespace Drascula

#endif /* DRASCULA_H */

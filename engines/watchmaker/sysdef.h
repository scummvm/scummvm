/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WATCHMAKER_SYSDEF_H
#define WATCHMAKER_SYSDEF_H

namespace Watchmaker {

#define WM_INIT_PACK_FILENAME           "Data.wm"           // file con le strutture di gioco
#define WM_GAMEDATA_PACK_FILENAME       "GameData.wm"       // t3d,bnd,cam,tmaps,etc

// STRUTTURE DI GIOCO
#define MAX_ROOMS                   75              // stanze del gioco
#define MAX_OBJS                    1624        // oggetti del gioco
#define MAX_ANIMS                   1360            // animazioni
#define MAX_ACTIONS                 3               // numero di azioni in tutto il gioco
#define MAX_ICONS                   86              // oggetti d'inventario
#define MAX_SOUNDS                  1120            // numero massimo di sample nel gioco
#define MAX_MUSICS                  24              // numero massimo di musiche nel gioco
#define MAX_DIALOGS                 114             // numero dialoghi e full-motion
#define MAX_DIARIES                 44              // numero di diari dei personaggi
#define MAX_DLG_MENUS               100             // numero di menu e sotto menu per i dialoghi
#define MAX_DLG_ITEMS               341             // numero massimo item per dialoghi
#define MAX_PLAYERS                 2               // numero personaggi principali giocanti
#define MAX_ENVIRONMENTS            32              // numero massimo di environments presenti
#define MAX_MODIFIED_MESH           1536            // numero mesh modifiers

#define MAX_OBJS_IN_ROOM            99              // oggetti in stanza
#define MAX_ANIMS_IN_ROOM           10              // numero di animazioni per stanza
#define MAX_ACTIONS_IN_ROOM         2               // numero di azioni per stanza
#define MAX_SOUNDS_IN_ROOM          15              // suoni per stanza
#define MAX_ICONS_IN_INV            100             // icone nell'inventario
#define MAX_SHOWN_ICONS             24              // icone visibili nell'inventario

#define MAX_IC_PER_DLG_ITEM         200             // numero massimo di item commands associate ad un item dei dialoghi
#define MAX_ALTERNATES              3               // numero massimo alternate nei dialoghi
#define MAX_ALT_ANIMS               16              // numero massimo animazioni alternative nei dialoghi per ogni alternate
#define MAX_DIARY_ITEMS             20              // numero massimo di elementi per ogni diario
#define MAX_ANIMS_PER_DIARY_ITEM    20              // numero massimo di animazioni per elemento diario

#define MAX_D3D_RECTS               20              // numero massimo rettangoli 3D visualizzati
#define MAX_D3D_TRIANGLES           20              // numero massimo triangoli 3D visualizzati
#define MAX_D3D_BITMAPS             20              // numero massimo bitmaps 3D visualizzate
#define MAX_DD_BITMAPS              200             // numero massimo bitmaps 2D visualizzati
#define MAX_DD_TEXTS                50              // numero massimo testi 2D visualizzati
#define MAX_REND_TEXTS              MAX_DD_TEXTS+10 // numero massimo testi prerenderizzati

#define INV_MARG_UP                 88              // posizione superiore della barra dell'inventario
#define INV_MARG_DOWN               INV_MARG_UP+ICON_DY*MAX_SHOWN_ICONS // posizione inferiore della barra dell'inventario
#define INV_MARG_SX                 42              // posizione sinistra della barra dell'inventario
#define INV_MARG_DX                 200             // posizione destra della barra dell'inventario
#define ICON_DY                     20              // altezza di una casella icona

#define DIAG1_MARG_SX               20              // posizione sinistra della barra 1 del dialogo
#define DIAG1_MARG_UP               391             // posizione superiore della barra 1 del dialogo
#define DIAG1_MARG_DX               182             // posizione destra della barra 1 del dialogo
#define DIAG1_MARG_DOWN             315             // posizione inferiore della barra 1 del dialogo
#define DIAG2_MARG_SX               248             // posizione sinistra della barra 2 del dialogo
#define DIAG2_MARG_UP               527             // posizione superiore della barra 2 del dialogo
#define DIAG2_MARG_DX               736             // posizione destra della barra 2 del dialogo
#define DIAG2_MARG_DOWN             550             // posizione inferiore della barra 2 del dialogo
#define MAX_DIAG1_ITEMS             9               // numero massimo di elementi possibili in diag1
#define MAX_DIAG2_ITEMS             3               // numero massimo di elementi possibili in diag2
#define DIAG_DY                     22              // Spaziatura dialogo
#define DIAG2_DY                    18              // Spaziatura dialogo
#define MAX_SAVE_SLOTS              16              // numero massimo di save

#define MAX_OBJ_USER_SENTS          5               // numero sent aggiuntive per oggetto
#define MAX_ICON_USER_SENTS         4               // numero sent aggiuntive per icona
#define MAX_OBJ_MESHLINKS           16              // link oggetto -> mesh
#define MAX_SUBANIMS                30              // link anim -> mesh
#define MAX_SOUND_MESHLINKS         16              // link suono -> mesh
#define MAX_SUB_MUSICS              10              // sotto musiche

#define MAX_ATFRAMES                24              // numero di atframe
#define MAX_ATF_DO                  80              // numero di eventi particolari

#define MAX_SCRIPTS                 100             // numero di script
#define MAX_SCRIPT_FRAMES           500             // numero di frames per script

#define MAX_MESSAGES                255             // numero messaggi eventi

#define TEXT_BUCKET_SIZE            512000L         // massima occupazione tutte le stringhe di WM
#define MAX_OBJ_NAMES               1400            // numero di nomi oggetto
#define MAX_SENTENCES               5000            // numero di frasi per oggetti
#define MAX_SYS_SENTS               100             // numero frasi di sistema
#define MAX_TOOLTIP_SENTS           100             // numero frasi per i tooltips del 2D
#define MAX_PDALOGS                 128             // numero massimo di log del PDA
#define MAX_PDA_LINES               3               // numero di righe che puo' occupare un log nel PDA
#define MAX_PDA_INFO                5               // numero massimo info nel pda item
#define MAX_STRING_LEN              400             // lunghezza massima stringa

#define MAX_TEXT_LINES              10              // numero massimo di linee per la formattazione
#define MAX_TEXT_CHARS              160             // numero massimo di caratteri per linea per la formattazione


// GESTIONE DELL'USA CON..
#define USED                        0
#define WITH                        1
#define UW_OFF                      0
#define UW_ON                       1
#define UW_USEDI                    2
#define UW_WITHI                    4
// GESTIONE DELL'INVENTARIO
#define INV_OFF                     0               // Inventario spento
#define INV_ON                      1               // Inventario attivo
#define INV_MODE1                   2               // Inventario con selettore icone a sinistra
#define INV_MODE2                   4               // Inventario fullscreen con icona grossa
#define INV_MODE3                   8               // Inventario con icona piccola in basso
#define INV_MODE4                   16              // Inventario in swap mode
#define INV_MODE5                   32              // Inventario in save mode

#define SCREEN_RES_X        800                     // Screen resolution X
#define SCREEN_RES_Y        600                     // Screen resolution Y

#define FRAME_PER_SECOND            76              // A quanto dovrebbe girare tutto il gioco
#define BACK_BUFFER                 0               // Identificativo del backbuffer
#define PLAYER_IDLE_TIME            10000L          // Dopo quanto tempo parte animazione di idle
// Altezze Darrell predefinite
#define MAX_HEIGHT                  450.0f          // Altezza predefinita omino
#define EYES_HEIGHT                 420.0f          // Altezza predefinita occhi
#define SHOULDERS_HEIGHT            350.0f          // Altezza predefinita spalla
#define CHEST_HEIGHT                240.0f          // Altezza predefinita ombelico
#define KNEE_HEIGHT                 130.0f          // Altezza predefinita ginocchio
#define ONE_STEP                    173.0f          // Lunghezza predefinita un passo
#define HALF_STEP                   60.0f           // Lunghezza predefinita mezzo passo
// Limiti di movimento e di velocita' della testa
#define MAX_HEAD_ANGLE_X            30.0f
#define MAX_HEAD_ANGLE_Y            30.0f
#define MAX_HEAD_SPEED              90.0f/FRAME_PER_SECOND
// Aperture predefinite della camera
#define CAMERA_FOV                  58.0f
#define CAMERA_FOV_1ST              74.0f
#define CAMERA_FOV_ICON             50.0f
// Colori predefiniti del font
enum FontColor {
	WHITE_FONT = 0,
	RED_FONT = 1,
	GREEN_FONT = 2,
	BLUE_FONT = 3,
	CYAN_FONT = 4,
	MAGENTA_FONT = 5,
	YELLOW_FONT = 6,
	GRAY_FONT = 7,
	BLACK_FONT = 8,
	MAX_FONT_COLORS = 9
};

enum class FontKind {
	Standard,
	Computer,
	PDA
};

#define MAX_GOPHERS                 3

} // End of namespace Watchmaker

#endif // WATCHMAKER_SYSDEF_H

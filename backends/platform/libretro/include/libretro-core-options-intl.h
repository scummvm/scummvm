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

#ifndef LIBRETRO_CORE_OPTIONS_INTL_H__
#define LIBRETRO_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable : 4566)
#endif

#include <libretro.h>

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
 */

/* RETRO_LANGUAGE_JAPANESE */

/* RETRO_LANGUAGE_FRENCH */

/* RETRO_LANGUAGE_SPANISH */

/* RETRO_LANGUAGE_GERMAN */

/* RETRO_LANGUAGE_ITALIAN */

struct retro_core_option_v2_category option_cats_it[] = {
	{
		"video",
		NULL,
		"Configura le impostazioni video"
	},
	{
		"cursor",
		"Cursore",
		"Impostazioni relative al movimento del cursore"
	},
	{
		"frameskip",
		"Salto dei fotogrammi",
		"Impostazioni per il salto dei fotogrammi"
	},
	{
		"timing",
		NULL,
		"Impostazioni relative al timing"
	},
	{
		"retropad",
		"Mappatura RetroPad",
		"Configura la mappatura del RetroPad"
	},
	{ NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_it[] = {
	{
		"scummvm_gamepad_cursor_only",
		"Cursore > Controllo esclusivo del cursore con RetroPad",
		"Controllo esclusivo del cursore con RetroPad",
		"Consente di usare solo RetroPad per il controllo del cursore del mouse, escludento gli altri input (es. mouse fisico, touch screen).",
		NULL,
		"cursor",
		{
			{"disabled", NULL},
			{"enabled", NULL},
			{NULL, NULL},
		},
		"disabled"
	},
	{
		"scummvm_gamepad_cursor_speed",
		"Cursore > Velocità del cursore",
		"Velocità del cursore",
		"Moltiplicatore della velocità del cursore del mouse quando si usa la leva analogica sinistra o il D-Pad del RetroPad. Il valore di default di '1.0' è ottimizzato per i giochi con risoluzione nativa di '320x200' o '320x240'. Per i giochi ad 'alta definizione' con risoluzione di '640x400' or '640x480', si raccomanda il valore di '2.0'",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_gamepad_cursor_acceleration_time",
		"Cursore > Accelerazione del cursore",
		"Accelerazione del cursore",
		"Il tempo (in secondi) necessario al cursore del mouse per raggiungere la piena velocità quando si usa la leva analogica sinistra o il D-Pad del RetroPad.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_analog_response",
		"Cursore > Risposta analogica del cursore",
		"Risposta analogica del cursore",
		"Modalità di risposta della velocità del cursore del mouse allo spostamento della leva analogica sinistra del RetroPad. 'Lineare': La velocità è direttamente proporzionale allo spostamento della leva. Questa è l'impostazione di default adatta alla maggior parte degli utenti. 'Quadratica': La velocità aumenta con il quadrato dello spostamento della leva. Questo permette maggior precisione nei piccoli movimenti senza sacrificare il raggiungimento della velocità massima a spostamento completo. Questa modalità può richiedere pratica per un uso efficace.",
		NULL,
		NULL,
		{
			{"linear", "Lineare"},
			{"quadratic", "Quadratica"},
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_analog_deadzone",
		"Cursore > Zona morta analogica",
		"Zona morta analogica",
		"Zona morta percentuale delle leve analogiche del RetroPad. Può essere usata per eliminare scorrimenti indesiderati del cursore.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_mouse_speed",
		"Cursore > Velocità del mouse",
		"Velocità del mouse",
		"Moltiplicatore della velocità del cursore del mouse quando si usa RetroMouse.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_mouse_fine_control_speed_reduction",
		"Cursore > Riduzione velocità con controllo fine del mouse",
		"Riduzione velocità con controllo fine del mouse",
		"Riduzione della velocità del cursore del mouse come percentuale della velocità normale quando il controllo fine è attivato.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_frameskip_type",
		"Frameskip > Salto dei fotogrammi",
		"Salto dei fotogrammi",
		"Salto dei fotogrammi per evitare buffer under-run audio (crackling). Migliora le prestazioni a discapito della fluidità video. 'Auto' salta i fotogrammi su indicazione del frontend, 'Soglia' usa l'impostazione di 'Soglia minima buffer audio (%)', 'Fisso' usa l'impostazione 'Salto dei fotogrammi fisso'.",
		NULL,
		NULL,
		{
			{ "disabled", NULL },
			{ "fixed", "Fisso" },
			{ "auto", "Auto" },
			{ "manual", "Soglia" },
			{ NULL, NULL },
		},
		NULL
	},
	{
		"scummvm_frameskip_threshold",
		"Frameskip > Soglia minima buffer audio (%)",
		"Soglia minima buffer audio (%)",
		"Quando 'Salto dei fotogrammi' è impostato su 'Soglia', specifica la soglia minima del buffer audio al di sotto della quale il fotogramma viene saltato. Valori più alti riducono il rischio di crackling al costo di un salto di fotogrammi più frequente.",
		NULL,
		NULL,
		{
			{ NULL, NULL },
		},
		NULL
	},
	{
		"scummvm_frameskip_no",
		"Frameskip > Salto dei fotogrammi fisso",
		"Salto dei fotogrammi fisso",
		"Quando la modalità di 'Salto dei fotogrammi' è 'Fisso', o il frontend non supporta una delle altre modalità selezionate, salta costantemente X fotogrammi ogni X+1.",
		NULL,
		NULL,
		{
			{ "0", "Nessun fotogramma saltato" },
			{ "1", "Salto di 1 fotogramma su 2" },
			{ "2", "Salto di 2 fotogrammi su 3" },
			{ "3", "Salto di 3 fotogrammi su 4" },
			{ "4", "Salto di 4 fotogrammi su 5" },
			{ "5", "Salto di 5 fotogrammi su 6" },
			{ NULL, NULL },
		},
		NULL
	},
	{
		"scummvm_allow_timing_inaccuracies",
		"Timing > Consenti inaccuratezze di timing",
		"Consenti inaccuratezze di timing",
		"Consente inaccuratezze di timing che riducono significativamente le richeste di CPU. Anche se la maggior parte delle inaccuratezze sono impercettibili, in alcuni casi potrebbe introdurre problemi di sincronizzazione audio, quindi questa opzione andrebbe abilitata solo se il raggiungimento della piena velocità non è possibile in altro modo.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},

	{
		"scummvm_framerate",
		"Timing > Tetto frequenza dei fotogrammi",
		"Tetto frequenza dei fotogrammi",
		"Imposta il limite superiore della frequenza dei fotogrammi. Il cambio di questa impostazione causerà il reset del core.",
		NULL,
		NULL,
		{
			{ NULL, NULL },
		},
		NULL
	},
	{
		"scummvm_samplerate",
		"Timing > Frequenza di campionamento",
		"Frequenza di campionamento",
		"Imposta la frequenza di campionamento. Il cambio di questa impostazione causerà il reset del core.",
		NULL,
		NULL,
		{
			{ NULL, NULL },
		},
		NULL
	},
	/* Button mappings */
	{
		"scummvm_mapper_up",
		"RetroPad > Su",
		"Su",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_down",
		"RetroPad > Giù",
		"Giù",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_left",
		"RetroPad > Sinistra",
		"Sinistra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_right",
		"RetroPad > Destra",
		"Destra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_a",
		"RetroPad > A",
		"A",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_b",
		"RetroPad > B",
		"B",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_x",
		"RetroPad > X",
		"X",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_y",
		"RetroPad > Y",
		"Y",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_select",
		"RetroPad > Select",
		"Select",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_start",
		"RetroPad > Start",
		"Start",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_l",
		"RetroPad > L",
		"L",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_r",
		"RetroPad > R",
		"R",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_l2",
		"RetroPad > L2",
		"L2",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_r2",
		"RetroPad > R2",
		"R2",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_l3",
		"RetroPad > L3",
		"L3",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_r3",
		"RetroPad > R3",
		"R3",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	/* Left Stick */
	{
		"scummvm_mapper_lu",
		"RetroPad > Leva Analogica Sinistra > Su",
		"Leva Analogica Sinistra > Su",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_ld",
		"RetroPad > Leva Analogica Sinistra > Giù",
		"Leva Analogica Sinistra > Giù",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_ll",
		"RetroPad > Leva Analogica Sinistra > Sinistra",
		"Leva Analogica Sinistra > Sinistra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_lr",
		"RetroPad > Leva Analogica Sinistra > Destra",
		"Leva Analogica Sinistra > Destra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	/* Right Stick */
	{
		"scummvm_mapper_ru",
		"RetroPad > Leva Analogica Destra > Su",
		"Leva Analogica Destra > Su",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_rd",
		"RetroPad > Leva Analogica Destra > Giù",
		"Leva Analogica Destra > Giù",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_rl",
		"RetroPad > Leva Analogica Destra > Sinistra",
		"Leva Analogica Destra > Sinistra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_mapper_rr",
		"RetroPad > Leva Analogica Destra > Leva Analogica Destra",
		"Leva Analogica Destra > Destra",
		NULL,
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_video_hw_acceleration",
		"Video > Accelerazione hardware",
		"Accelerazione hardware",
		"Richiede accelerazione hardware (OpenGL or OpenGLES2) al frontend, se supportata. È necessario ricaricare il core per rendere effettiva questa opzione",
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
#ifdef USE_HIGHRES
	{
		"scummvm_gui_aspect_ratio",
		"Video > Rapporto aspetto GUI",
		"Rapporto aspetto ScummVM Launcher",
		"Imposta il rapporto d'aspetto per ScummVM Launcher.",
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
	{
		"scummvm_gui_h_res",
		"Video > Risoluzione GUI",
		"Risoluzione ScummVM Launcher",
		"Imposta la risoluzione per ScummVM Launcher.",
		NULL,
		NULL,
		{
			{ NULL, NULL }
		},
		NULL,
	},
#endif
	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_it = {
	option_cats_it,
	option_defs_it
};


/* List has been reduced to fit RETRO_NUM_CORE_OPTION_VALUES_MAX.
 * Latest element {NULL, NULL} has been omitted in this case as the array is exactly sized
 * RETRO_NUM_CORE_OPTION_VALUES_MAX; in case the array size will be reduced, {NULL, NULL}
 * element to be uncommented.
 */

struct retro_core_option_value retro_keys_label_it [] = {
	{"---", "---"},
	{"RETROKE_VKBD", "Attiva/disattiva Tastiera Virtuale"},
	{"RETROKE_LEFT", "Cursore Mouse Sinistra"},
	{"RETROKE_UP", "Cursore Mouse Su"},
	{"RETROKE_DOWN", "Cursore Mouse Giù"},
	{"RETROKE_RIGHT", "Cursore Mouse Destra"},
	{"RETROKE_LEFT_BUTTON", "Tasto Mouse Sinistra"},
	{"RETROKE_RIGHT_BUTTON", "Tasto Mouse Destra"},
	{"RETROKE_FINE_CONTROL", "Controllo Fine Cursore Mouse"},
	{"RETROKE_SCUMMVM_GUI", "ScummVM GUI"},
	{"RETROKE_SHIFT_MOD", "Tastiera Shift (Modificatore)"},
	{"RETROKE_CTRL_MOD", "Tastiera Control (Modificatore)"},
	{"RETROKE_ALT_MOD", "Tastiera Alt (Modificatore)"},
	{"RETROK_BACKSPACE", "Tastiera Backspace"},
	{"RETROK_TAB", "Tastiera Tab"},
	{"RETROK_CLEAR", "Tastiera Clear"},
	{"RETROK_RETURN", "Tastiera Return"},
	{"RETROK_PAUSE", "Tastiera Pause"},
	{"RETROK_ESCAPE", "Tastiera Escape"},
	{"RETROK_SPACE", "Tastiera Space"},
	{"RETROK_EXCLAIM", "Tastiera !"},
	{"RETROK_QUOTEDBL", "Tastiera \""},
	{"RETROK_HASH", "Tastiera #"},
	{"RETROK_DOLLAR", "Tastiera $"},
	{"RETROK_AMPERSAND", "Tastiera &"},
	{"RETROK_QUOTE", "Tastiera \'"},
	{"RETROK_LEFTPAREN", "Tastiera ("},
	{"RETROK_RIGHTPAREN", "Tastiera )"},
	{"RETROK_ASTERISK", "Tastiera *"},
	{"RETROK_PLUS", "Tastiera +"},
	{"RETROK_COMMA", "Tastiera ,"},
	{"RETROK_MINUS", "Tastiera -"},
	{"RETROK_PERIOD", "Tastiera ."},
	{"RETROK_SLASH", "Tastiera /"},
	{"RETROK_0", "Tastiera 0"},
	{"RETROK_1", "Tastiera 1"},
	{"RETROK_2", "Tastiera 2"},
	{"RETROK_3", "Tastiera 3"},
	{"RETROK_4", "Tastiera 4"},
	{"RETROK_5", "Tastiera 5"},
	{"RETROK_6", "Tastiera 6"},
	{"RETROK_7", "Tastiera 7"},
	{"RETROK_8", "Tastiera 8"},
	{"RETROK_9", "Tastiera 9"},
	{"RETROK_COLON", "Tastiera :"},
	{"RETROK_SEMICOLON", "Tastiera ;"},
	{"RETROK_LESS", "Tastiera <"},
	{"RETROK_EQUALS", "Tastiera ="},
	{"RETROK_GREATER", "Tastiera >"},
	{"RETROK_QUESTION", "Tastiera ?"},
	{"RETROK_AT", "Tastiera @"},
	{"RETROK_LEFTBRACKET", "Tastiera ["},
	{"RETROK_BACKSLASH", "Tastiera \\"},
	{"RETROK_RIGHTBRACKET", "Tastiera ]"},
	{"RETROK_CARET", "Tastiera ^"},
	{"RETROK_UNDERSCORE", "Tastiera _"},
	{"RETROK_BACKQUOTE", "Tastiera `"},
	{"RETROK_a", "Tastiera a"},
	{"RETROK_b", "Tastiera b"},
	{"RETROK_c", "Tastiera c"},
	{"RETROK_d", "Tastiera d"},
	{"RETROK_e", "Tastiera e"},
	{"RETROK_f", "Tastiera f"},
	{"RETROK_g", "Tastiera g"},
	{"RETROK_h", "Tastiera h"},
	{"RETROK_i", "Tastiera i"},
	{"RETROK_j", "Tastiera j"},
	{"RETROK_k", "Tastiera k"},
	{"RETROK_l", "Tastiera l"},
	{"RETROK_m", "Tastiera m"},
	{"RETROK_n", "Tastiera n"},
	{"RETROK_o", "Tastiera o"},
	{"RETROK_p", "Tastiera p"},
	{"RETROK_q", "Tastiera q"},
	{"RETROK_r", "Tastiera r"},
	{"RETROK_s", "Tastiera s"},
	{"RETROK_t", "Tastiera t"},
	{"RETROK_u", "Tastiera u"},
	{"RETROK_v", "Tastiera v"},
	{"RETROK_w", "Tastiera w"},
	{"RETROK_x", "Tastiera x"},
	{"RETROK_y", "Tastiera y"},
	{"RETROK_z", "Tastiera z"},
	{"RETROK_KP0", "Tastiera Numpad 0"},
	{"RETROK_KP1", "Tastiera Numpad 1"},
	{"RETROK_KP2", "Tastiera Numpad 2"},
	{"RETROK_KP3", "Tastiera Numpad 3"},
	{"RETROK_KP4", "Tastiera Numpad 4"},
	{"RETROK_KP5", "Tastiera Numpad 5"},
	{"RETROK_KP6", "Tastiera Numpad 6"},
	{"RETROK_KP7", "Tastiera Numpad 7"},
	{"RETROK_KP8", "Tastiera Numpad 8"},
	{"RETROK_KP9", "Tastiera Numpad 9"},
	{"RETROK_KP_PERIOD", "Tastiera Numpad ."},
	{"RETROK_KP_DIVIDE", "Tastiera Numpad /"},
	{"RETROK_KP_MULTIPLY", "Tastiera Numpad *"},
	{"RETROK_KP_MINUS", "Tastiera Numpad -"},
	{"RETROK_KP_PLUS", "Tastiera Numpad +"},
	{"RETROK_KP_ENTER", "Tastiera Numpad Enter"},
	{"RETROK_KP_EQUALS", "Tastiera Numpad ="},
	{"RETROK_UP", "Tastiera Su"},
	{"RETROK_DOWN", "Tastiera Giù"},
	{"RETROK_LEFT", "Tastiera Sinistra"},
	{"RETROK_RIGHT", "Tastiera Destra"},
	{"RETROK_INSERT", "Tastiera Insert"},
	{"RETROK_DELETE", "Tastiera Delete"},
	{"RETROK_HOME", "Tastiera Home"},
	{"RETROK_END", "Tastiera End"},
	{"RETROK_PAGEUP", "Tastiera PageUp"},
	{"RETROK_PAGEDOWN", "Tastiera PageDown"},
	{"RETROK_F1", "Tastiera F1"},
	{"RETROK_F2", "Tastiera F2"},
	{"RETROK_F3", "Tastiera F3"},
	{"RETROK_F4", "Tastiera F4"},
	{"RETROK_F5", "Tastiera F5"},
	{"RETROK_F6", "Tastiera F6"},
	{"RETROK_F7", "Tastiera F7"},
	{"RETROK_F8", "Tastiera F8"},
	{"RETROK_F9", "Tastiera F9"},
	{"RETROK_F10", "Tastiera F10"},
	{"RETROK_F11", "Tastiera F11"},
	{"RETROK_F12", "Tastiera F12"},
//{"RETROK_F13","Tastiera F13"},
//{"RETROK_F14","Tastiera F14"},
//{"RETROK_F15","Tastiera F15"},
//{"RETROK_NUMLOCK","Tastiera NumLock"},
//{"RETROK_CAPSLOCK","Tastiera Caps Lock"},
//{"RETROK_SCROLLOCK","Tastiera Scroll Lock"},
	{"RETROK_LSHIFT", "Tastiera Shift Sinistra"},
	{"RETROK_RSHIFT", "Tastiera Shift Destra"},
	{"RETROK_LCTRL", "Tastiera Control Sinistra"},
	{"RETROK_RCTRL", "Tastiera Control Destra"},
	{"RETROK_LALT", "Tastiera Alt Sinistra"},
	{"RETROK_RALT", "Tastiera Alt Destra"},
//{"RETROK_LMETA","Tastiera Left Meta"},
//{"RETROK_RMETA","Tastiera Right Meta"},
//{"RETROK_LSUPER","Tastiera Left Super"},
//{"RETROK_RSUPER","Tastiera Right Super"},
//{"RETROK_MODE","Tastiera Mode"},
//{"RETROK_COMPOSE","Tastiera Compose"},
//{"RETROK_HELP","Tastiera Help"},
//{"RETROK_PRINT","Tastiera Print"},
//{"RETROK_SYSREQ","Tastiera SysReq"},
//{"RETROK_BREAK","Tastiera Break"},
//{"RETROK_MENU","Tastiera Menu"},
//{"RETROK_POWER","Tastiera Power"},
//{"RETROK_EURO","Tastiera Euro"},
//{"RETROK_UNDO","Tastiera Undo"},
};

/* RETRO_LANGUAGE_DUTCH */

/* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */

/* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */

/* RETRO_LANGUAGE_RUSSIAN */

/* RETRO_LANGUAGE_KOREAN */

/* RETRO_LANGUAGE_CHINESE_TRADITIONAL */

/* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */

/* RETRO_LANGUAGE_ESPERANTO */

/* RETRO_LANGUAGE_POLISH */

/* RETRO_LANGUAGE_VIETNAMESE */

/* RETRO_LANGUAGE_ARABIC */

/* RETRO_LANGUAGE_GREEK */

/* RETRO_LANGUAGE_TURKISH */

#ifdef __cplusplus
}
#endif

#endif

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
		"cursor",
		"Movimento del cursore",
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
	{ NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_it[] = {
	{
		"scummvm_gamepad_cursor_speed",
		"Velocità del cursore",
		NULL,
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
		"Accelerazione del cursore",
		NULL,
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
		"Risposta analogica del cursore",
		NULL,
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
		"Zona morta analogica (percentuale)",
		NULL,
		"Zona morta della leva analogica sinistra del RetroPad. Può essere usata per eliminare scorrimenti indesiderati del cursore.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_mouse_speed",
		"Velocità del mouse",
		NULL,
		"Moltiplicatore della velocità del cursore del mouse quando si usa RetroMouse.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_frameskip_type",
		"Salto dei fotogrammi",
		NULL,
		"Salto dei fotogrammi per evitare buffer under-run audio (crackling). Migliora le prestazioni a discapito della fluidità video. 'Auto' salta i fotogrammi su indicazione del frontend, 'Soglia' usa l'impostazione di 'Soglia minima buffer audio (%)', 'Fisso' usa l'impostazione 'Salto dei fotogrammi fisso'.",
		NULL,
		"frameskip",
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
		"Soglia minima buffer audio (%)",
		NULL,
		"Quando 'Salto dei fotogrammi' è impostato su 'Soglia', specifica la soglia minima del buffer audio al di sotto della quale il fotogramma viene saltato. Valori più alti riducono il rischio di crackling al costo di un salto di fotogrammi più frequente.",
		NULL,
		"frameskip",
		{
			{ NULL, NULL },
		},
		NULL
	},
	{
		"scummvm_frameskip_no",
		"Salto dei fotogrammi fisso",
		NULL,
		"Quando la modalità di 'Salto dei fotogrammi' è 'Fisso', o il frontend non supporta una delle altre modalità selezionate, salta costantemente X fotogrammi ogni X+1.",
		NULL,
		"frameskip",
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
		"Consenti inaccuratezze di timing",
		NULL,
		"Consente inaccuratezze di timing che riducono significativamente le richeste di CPU. Anche se la maggior parte delle inaccuratezze sono impercettibili, in alcuni casi potrebbe introdurre problemi di sincronizzazione audio, quindi questa opzione andrebbe abilitata solo se il raggiungimento della piena velocità non è possibile in altro modo.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{
		"scummvm_auto_performance_tuner",
		"Regolazione automatica performance",
		NULL,
		"Cambio automatico delle impostazioni di timing e salto dei fotogrammi se vengono rilevate performance scadenti durante il gioco. Le impostazioni di timing/frameskip saranno temporaneamente cambiate in sequenza, se saranno rilevati audio buffer underrun e per la sola sessione di gioco, e ripristinati in sequenza in caso di recupero del buffer audio. Le singole impostazioni salvate non saranno modificate ma saranno ignorati.",
		NULL,
		NULL,
		{
			{NULL, NULL},
		},
		NULL
	},
	{ NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_it = {
	option_cats_it,
	option_defs_it
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

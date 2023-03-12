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
		"scummvm_speed_hack",
		"Speed Hack (riavvio necessario)",
		NULL,
		"Modalità che riduce significativamente le richeste di CPU consentendo lievi inaccuratezze di timing. Questo 'hack' è considerato 'sicuro' - non dovrebbe causare errori e la maggior parte delle inaccuratezze sono impercettibili. Rimane comunque un hack, e gli utenti con macchine di classe desktop dovrebbero lasciarlo disabilitato. Su hardware a basse prestazioni (vecchi device Android, computer su singola scheda), questo hack è essenziale per il funzionamento del core a velocità piena.",
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

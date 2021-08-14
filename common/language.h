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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_LANGUAGE_H
#define COMMON_LANGUAGE_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_language Language
 * @ingroup common
 *
 * @brief API for managing game language.
 *
 *
 * @{
 */

class String;

/**
 * List of game language.
 */
enum Language {
	AR_ARB,
	CA_ESP,
	CZ_CZE,
	DA_DAN,
	DE_DEU,
	EN_ANY,     // Generic English (when only one game version exist)
	EN_GRB,
	EN_USA,
	ES_ESP,
	ET_EST,
	FI_FIN,
	FA_IRN,
	FR_FRA,
	GR_GRE,
	HE_ISR,
	HR_HRV,
	HU_HUN,
	IT_ITA,
	JA_JPN,
	KO_KOR,
	LV_LAT,
	NL_BEL,
	NL_NLD,
	NB_NOR,
	PL_POL,
	PT_BRA,
	PT_POR,
	RU_RUS,
	SE_SWE,
	SK_SVK,
	SR_SER,
	TR_TUR,
	UA_UKR,
	ZH_ANY,     // Generic Chinese (when only one game version exist)
	ZH_CNA,
	ZH_TWN,

	UNK_LANG = -1	// Use default language (i.e. none specified)
};

struct LanguageDescription {
	const char *code;
	const char *unixLocale;
	const char *description;
	Language id;
};

extern const LanguageDescription g_languages[];


/** Convert a string containing a language name into a Language enum value. */
extern Language parseLanguage(const String &str);
extern Language parseLanguageFromLocale(const char *locale);
extern const char *getLanguageCode(Language id);
extern const char *getLanguageLocale(Language id);
extern const char *getLanguageDescription(Language id);

// TODO: Document this GUIO related function
const String getGameGUIOptionsDescriptionLanguage(Common::Language lang);

// TODO: Document this GUIO related function
bool checkGameGUIOptionLanguage(Common::Language lang, const String &str);

/** @} */

} // End of namespace Common

#endif

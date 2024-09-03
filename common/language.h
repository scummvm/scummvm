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

#ifndef COMMON_LANGUAGE_H
#define COMMON_LANGUAGE_H

#include "common/scummsys.h"
#include "common/list.h"

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
enum Language : int8 {
	AR_ARB,
	BG_BUL,
	CA_ESP,
	CS_CZE,
	DA_DNK,
	DE_DEU,
	EL_GRC,
	EN_ANY,     // Generic English (when only one game version exist)
	EN_GRB,
	EN_USA,
	ES_ESP,
	EU_ESP,
	ET_EST,
	FA_IRN,
	FI_FIN,
	FR_FRA,
	HE_ISR,
	HR_HRV,
	HU_HUN,
	IT_ITA,
	JA_JPN,
	KO_KOR,
	LT_LTU,
	LV_LVA,
	NL_BEL,
	NL_NLD,
	NB_NOR,
	PL_POL,
	PT_BRA,
	PT_PRT,
	RU_RUS,
	SE_SWE,
	SK_SVK,
	SR_SRB,
	TR_TUR,
	UA_UKR,
	ZH_ANY,     // Generic Chinese (when only one game version exist)
	ZH_CHN,
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
extern const LanguageDescription g_obsoleteLanguages[];

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

List<String> getLanguageList();

/** @} */

} // End of namespace Common

#endif

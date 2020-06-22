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

#include "common/language.h"
#include "common/gui_options.h"
#include "common/str.h"

namespace Common {

const LanguageDescription g_languages[] = {
	{    "nz",    "nz", "Chinese", ZH_ANY }, // Generic Chinese (when only one game version exist)
	{ "zh-cn", "zh_CN", "Chinese (China)", ZH_CNA },
	{    "zh", "zh_TW", "Chinese (Taiwan)", ZH_TWN },
	{    "hr", "hr_HR", "Croatian", HR_HRV },
	{    "cz", "cs_CZ", "Czech", CZ_CZE },
	{    "da",    "da", "Danish", DA_DAN },
	{    "nl", "nl_NL", "Dutch", NL_NLD },
	{    "en",    "en", "English", EN_ANY }, // Generic English (when only one game version exist)
	{    "gb", "en_GB", "English (GB)", EN_GRB },
	{    "us", "en_US", "English (US)", EN_USA },
	{    "et", "et_EE", "Estonian", ET_EST },
	{    "fi", "fi_FI", "Finnish", FI_FIN },
	{    "fr", "fr_FR", "French", FR_FRA },
	{    "de", "de_DE", "German", DE_DEU },
	{    "gr", "el_GR", "Greek", GR_GRE },
	{    "he", "he_IL", "Hebrew", HE_ISR },
	{    "hb", "he_IL", "Hebrew", HE_ISR }, // Deprecated
	{    "hu", "hu_HU", "Hungarian", HU_HUN },
	{    "it", "it_IT", "Italian", IT_ITA },
	{    "jp", "ja_JP", "Japanese", JA_JPN },
	{    "kr", "ko_KR", "Korean", KO_KOR },
	{    "lv", "lv_LV", "Latvian", LV_LAT },
	{    "nb", "nb_NO", "Norwegian Bokm\xE5l", NB_NOR },
	{    "fa", "fa_IR", "Persian (Iran)", FA_IRN },
	{    "pl", "pl_PL", "Polish", PL_POL },
	{    "br", "pt_BR", "Portuguese (Brazil)", PT_BRA },
	{    "pt", "pt_PT", "Portuguese (Portugal)", PT_POR },
	{    "ru", "ru_RU", "Russian", RU_RUS },
	{    "sk", "sk_SK", "Slovak", SK_SVK },
	{    "es", "es_ES", "Spanish", ES_ESP },
	{    "se", "sv_SE", "Swedish", SE_SWE },
	{    "tr", "tr_TR", "Turkish", TR_TUR },
	{    "uk", "uk_UA", "Ukrainian", UA_UKR },
	{    "sr", "sr_SP", "Serbian", SR_SER },
	{ nullptr, nullptr, nullptr, UNK_LANG }
};

Language parseLanguage(const String &str) {
	if (str.empty())
		return UNK_LANG;

	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code))
			return l->id;
	}

	return UNK_LANG;
}

Language parseLanguageFromLocale(const char *locale) {
	if (!locale || !*locale)
		return UNK_LANG;

	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (!strcmp(l->unixLocale, locale))
			return l->id;
	}

	return UNK_LANG;
}

const char *getLanguageCode(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return nullptr;
}

const char *getLanguageLocale(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->unixLocale;
	}
	return nullptr;
}

const char *getLanguageDescription(Language id) {
	const LanguageDescription *l = g_languages;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return nullptr;
}

bool checkGameGUIOptionLanguage(Language lang, const String &str) {
	if (!str.contains("lang_")) // If no languages are specified
		return true;

	if (str.contains(getGameGUIOptionsDescriptionLanguage(lang)))
		return true;

	return false;
}

const String getGameGUIOptionsDescriptionLanguage(Language lang) {
	if (lang == UNK_LANG)
		return "";

	return String("lang_") + getLanguageDescription(lang);
}

} // End of namespace Common

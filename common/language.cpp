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

#include "common/language.h"
#include "common/str.h"
#include "common/algorithm.h"

namespace Common {

/*
	Entries should be sorted by English name of language

	Explanation of language parameters:
		1. two character macro language (use ISO-639 standard)
		2. POSIX locale code
		3. English name of language
		4. two character language code followed by three character locale (use ISO-3166)

	If more than one dialect or locale is needed:
		1. two character locale (use ISO-3166 standard)
		2. POSIX locale code
		3. English name of language with locale/dialect name in parens
		4. two character language code followed by three character locale (use ISO-3166)

	If necessary, add entry for generic dialect
		1. two character macro language (use ISO-639 standard)
		2. repeat two character macro language
		3. English name of language
		4. two character language code followed by ANY

	Additional notes:
		* ZH_CHN (Hans 500) used for mainland China, Singapore, Malaysia
		* ZH_TWN (Hant 502) used for Taiwan, Hong Kong, Macau, Philippines
*/

const LanguageDescription g_languages[] = {
	{ "ar",    "ar", "Arabic", AR_ARB }, // Modern Standard Arabic
	{ "bg", "bg_BG", "Bulgarian", BG_BUL },
	{ "ca", "ca_ES", "Catalan", CA_ESP },
	{ "zh",    "zh", "Chinese", ZH_ANY }, // Generic Chinese (when only one game version exist)
	{ "cn", "zh_CN", "Chinese (Simplified)", ZH_CHN },
	{ "tw", "zh_TW", "Chinese (Traditional)", ZH_TWN },
	{ "hr", "hr_HR", "Croatian", HR_HRV },
	{ "cs", "cs_CZ", "Czech", CS_CZE },
	{ "da", "da_DK", "Danish", DA_DNK },
	{ "nl", "nl_NL", "Dutch", NL_NLD },
	{ "en",    "en", "English", EN_ANY }, // Generic English (when only one game version exist)
	{ "gb", "en_GB", "English (GB)", EN_GRB },
	{ "us", "en_US", "English (US)", EN_USA },
	{ "et", "et_EE", "Estonian", ET_EST },
	{ "fi", "fi_FI", "Finnish", FI_FIN },
	{ "be", "nl_BE", "Flemish", NL_BEL },
	{ "fr", "fr_FR", "French", FR_FRA },
	{ "de", "de_DE", "German", DE_DEU },
	{ "el", "el_GR", "Greek", EL_GRC },
	{ "he", "he_IL", "Hebrew", HE_ISR },
	{ "hu", "hu_HU", "Hungarian", HU_HUN },
	{ "it", "it_IT", "Italian", IT_ITA },
	{ "ja", "ja_JP", "Japanese", JA_JPN },
	{ "ko", "ko_KR", "Korean", KO_KOR },
	{ "lt", "lt_LT", "Lithuanian", LT_LTU },
	{ "lv", "lv_LV", "Latvian", LV_LVA },
	{ "nb", "nb_NO", "Norwegian (Bokm\xC3\xA5l)", NB_NOR },
	{ "fa", "fa_IR", "Persian", FA_IRN },
	{ "pl", "pl_PL", "Polish", PL_POL },
	{ "br", "pt_BR", "Portuguese (Brazil)", PT_BRA },
	{ "pt", "pt_PT", "Portuguese (Portugal)", PT_PRT },
	{ "ru", "ru_RU", "Russian", RU_RUS },
	{ "sr", "sr_RS", "Serbian", SR_SRB },
	{ "sk", "sk_SK", "Slovak", SK_SVK },
	{ "es", "es_ES", "Spanish", ES_ESP },
	{ "eu", "eu_ES", "Basque", EU_ESP },
	{ "se", "sv_SE", "Swedish", SE_SWE },
	{ "tr", "tr_TR", "Turkish", TR_TUR },
	{ "uk", "uk_UA", "Ukrainian", UA_UKR },
	{ nullptr, nullptr, nullptr, UNK_LANG }
};

const LanguageDescription g_obsoleteLanguages[] = {
	{	 "cz", "cs_CZ", "Czech", CS_CZE },
	{	 "gr", "el_GR", "Greek", EL_GRC },
	{	 "hb", "he_IL", "Hebrew", HE_ISR },
	{	 "jp", "ja_JP", "Japanese", JA_JPN },
	{	 "kr", "ko_KR", "Korean", KO_KOR },
	{	 "nz",    "zh", "Chinese", ZH_ANY },
	{ "zh-cn", "zh_CN", "Chinese (Simplified)", ZH_CHN },
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

	const LanguageDescription *ol = g_obsoleteLanguages;
	for (; ol->code; ++ol) {
		if (str.equalsIgnoreCase(ol->code))
			return ol->id;
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

List<String> getLanguageList() {
	List<String> list;

	for (const LanguageDescription *l = g_languages; l->code; ++l)
		list.push_back(l->code);

	 Common::sort(list.begin(), list.end());

	 return list;
}

} // End of namespace Common

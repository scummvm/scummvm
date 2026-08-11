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

#include "darkseed/darkseed.h"
#include "darkseed/langtext.h"

namespace Darkseed {

Common::U32String getI18NText(const I18nText &text) {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return Common::U32String(text.es);
	case Common::FR_FRA : return Common::U32String(text.fr);
	case Common::DE_DEU : return Common::U32String(text.de);
	case Common::KO_KOR : return text.ko ? convertToU32String(text.ko, Common::KO_KOR) : Common::U32String(text.en);
	case Common::ZH_ANY : return text.zh ? convertToU32String(text.zh, Common::ZH_ANY) : Common::U32String(text.en);
	default : return Common::U32String(text.en);
	}
}

const TextWithPosition &getI18NTextWithPosition(const I18NTextWithPosition &i18nTextWithPosition) {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return i18nTextWithPosition.es;
	case Common::FR_FRA : return i18nTextWithPosition.fr;
	case Common::DE_DEU : return i18nTextWithPosition.de;
	case Common::KO_KOR : return i18nTextWithPosition.ko;
	case Common::ZH_ANY : return i18nTextWithPosition.zh;
	default : return i18nTextWithPosition.en;
	}
}

Common::U32String convertToU32String(const char *text, Common::Language language) {
	size_t len = strlen(text);
	switch (language) {
	case Common::ZH_ANY:
	case Common::KO_KOR: {
		Common::U32String str;
		for (uint i = 0; i < len; i++) {
			uint8 byte = text[i];
			if (byte & 0x80) {
				if (i < len - 1) {
					uint8 byte2 = text[i + 1];
					str += (int)byte << 8 | byte2;
					i++;
				}
			} else {
				str += byte;
			}
		}
		return str;
	}
	default : break;
	}
	return Common::U32String(text);
}

Common::U32String formatInjectStrings(const Common::u32char_type_t *format, ...) {
	Common::U32String outString;
	va_list args;
	va_start(args, format);

	for (const Common::u32char_type_t *itr = format; *itr; itr++) {
		if (*itr == '%') {
			itr++;
			if (!*itr) {
				outString += '%';
				break;
			}
			if (*itr == 's') {
				auto text = va_arg(args, char32_t *);
				outString += text;
			} else {
				outString += *itr;
			}
		} else {
			outString += *itr;
		}
	}
	va_end(args);
	return outString;
}

} // End of namespace Darkseed

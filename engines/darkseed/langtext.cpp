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

const char *getI18NText(const I18nText &text) {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return text.es;
	case Common::FR_FRA : return text.fr;
	case Common::DE_DEU : return text.de;
	default : return text.en;
	}
}

const TextWithPosition &getI18NTextWithPosition(const I18NTextWithPosition &i18nTextWithPosition) {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return i18nTextWithPosition.es;
	case Common::FR_FRA : return i18nTextWithPosition.fr;
	case Common::DE_DEU : return i18nTextWithPosition.de;
	case Common::KO_KOR : return i18nTextWithPosition.ko;
	default : return i18nTextWithPosition.en;
	}
}

Common::U32String convertToU32String(const char *text, Common::Language language) {
	uint len = strlen(text);
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

} // End of namespace Darkseed

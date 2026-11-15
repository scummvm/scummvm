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


#ifndef SNATCHER_FANTRANSLATIONS_DETECTION_H
#define SNATCHER_FANTRANSLATIONS_DETECTION_H

#include "common/language.h"
#include "common/list.h"
#include "common/platform.h"
#include "common/str-enc.h"
#include "common/str.h"

namespace Snatcher {

class FanTranslationHandlerDetection {
public:
	enum TranslatorMode {
		kDetect = 0,
		kTranslate = 1,
		kCollect = 2
	};

	struct FontProperties {
		FontProperties() : codepage(Common::kCodePageInvalid), rtl(false) {}
		Common::CodePage codepage;
		Common::String fontfile;
		bool rtl;
	};

	static Common::List<Common::Language> listTranslations(Common::Platform platform, Common::Language *defaultLang = nullptr);

protected:
	struct PerFileTranslateableData {
		PerFileTranslateableData() {}
		~PerFileTranslateableData() {
			stringTable.clear();
			addressTable.clear();
		}

		Common::List<Common::String> stringTable;
		Common::List<uint32> addressTable;
	};

	static void updateDataTable(uint32 addr, const uint8 *data, PerFileTranslateableData **table, TranslatorMode mode, uint dataSize = 256, int file = -1);
	static void parseTranslations(Common::Platform platform, Common::Language lang, TranslatorMode mode, int fileNumber = -1,
		PerFileTranslateableData **table = nullptr, Common::List<Common::Language> *langList = nullptr, Common::Language *defaultLang = nullptr, uint32 *fileOffsetList = nullptr, FontProperties *fontProperties = nullptr);
};

} // End of namespace Snatcher

#endif // SNATCHER_FANTRANSLATIONS_H


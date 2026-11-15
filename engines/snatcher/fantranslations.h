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


#ifndef SNATCHER_FANTRANSLATIONS_H
#define SNATCHER_FANTRANSLATIONS_H

#include "common/language.h"
#include "common/platform.h"

#include "engines/snatcher/fantranslations_detection.h"

namespace Snatcher {

class FanTranslationHandler : public FanTranslationHandlerDetection {
public:
	enum ResourceType {
		kInvalid = -1,
		kModule = 0,
		kScript = 1,
		kSCD = 2
	};

	FanTranslationHandler(Common::Platform platform, Common::Language defaultLang, Common::Language fanLang, TranslatorMode mode);
	~FanTranslationHandler();

	const uint8 *process(uint32 addr, const uint8 *data);
	void loadFile(int number, ResourceType rtype);

	const FontProperties &getFontProperties() const;

private:
	void saveFile();

	PerFileTranslateableData **_gameTranslateableData = nullptr;
	uint32 *_fileOffsetList = nullptr;
	FontProperties *_fontProperties = nullptr;

	int _gfxFileNo = -1;
	int _scriptFileNo = -1;
	int _scdFileNo = -1;
	const TranslatorMode _mode;
	const Common::Language _defaultLang;
	const Common::Platform _platform;
};

} // End of namespace Snatcher

#endif // SNATCHER_FANTRANSLATIONS_H

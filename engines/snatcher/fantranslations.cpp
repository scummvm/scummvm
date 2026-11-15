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


#include "snatcher/fantranslations.h"
#include "snatcher/mem_mapping.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"

namespace Snatcher {

#define SNATCHER_FANTRANSLATIONS_CURRENT_VERSION		1


FanTranslationHandler::FanTranslationHandler(Common::Platform platform, Common::Language defaultLang, Common::Language fanLang, TranslatorMode mode) : _mode(mode), _defaultLang(defaultLang), _platform(platform) {
	if (mode == kDetect)
		error("%s(): Unsupported mode 'kDetect' specified", __FUNCTION__);
	if (mode == kCollect && fanLang != Common::UNK_LANG)
		error("%s(): 'kCollect' mode works only with the default language, not with a fan translation", __FUNCTION__);
	if (fanLang != Common::UNK_LANG || mode == kCollect)
		_gameTranslateableData = new PerFileTranslateableData*[100]();
	if (fanLang != Common::UNK_LANG)
		_fileOffsetList = new uint32[100]();
	_fontProperties = new FontProperties();

	parseTranslations(platform, fanLang, _mode, -1, _gameTranslateableData, nullptr, nullptr, _fileOffsetList, _fontProperties);
}

FanTranslationHandler::~FanTranslationHandler() {
	if (_mode == kCollect)
		saveFile();
	if (_gameTranslateableData) {
		for (int i = 0; i < 100; ++i)
			delete _gameTranslateableData[i];
		delete[] _gameTranslateableData;
		_gameTranslateableData = nullptr;
	}
	delete[] _fileOffsetList;
	_fileOffsetList = nullptr;
	delete _fontProperties;
	_fontProperties = nullptr;
}

const uint8 *FanTranslationHandler::process(uint32 addr, const uint8 *data) {
	if (_mode == kTranslate && _gameTranslateableData) {
		for (int i = 0; i < 100; ++i) {
			if (!_gameTranslateableData[i])
				continue;

			const PerFileTranslateableData &d = *_gameTranslateableData[i];
			Common::List<Common::String>::const_iterator j = d.stringTable.begin();
			for (Common::List<uint32>::const_iterator it = d.addressTable.begin(); it != d.addressTable.end(); ++it, ++j) {
				if (*it == addr)
					return reinterpret_cast<const uint8 *>(j->c_str());
			}
		}
	} else if (_mode == kCollect) {
		int fileNo = (addr < MemMapping::MEM_SCRIPT_LOCATION) ? _scdFileNo : (addr >= MemMapping::MEM_MODULE_LOCATION ? _gfxFileNo : _scriptFileNo);
		updateDataTable(addr, data, _gameTranslateableData, _mode, 256, fileNo);
	}
	return data;
}

void FanTranslationHandler::loadFile(int number, ResourceType rtype) {
	if (rtype == kInvalid)
		return;

	int &fileNo = (rtype == kScript) ? _scriptFileNo : (rtype == kSCD ? _scdFileNo : _gfxFileNo);
	if (fileNo == number)
		return;

	fileNo = number;

	if (_gameTranslateableData == nullptr || _fileOffsetList == nullptr || _mode == kCollect)
		return;

	if (fileNo != -1 && _gameTranslateableData[fileNo]) {
		delete _gameTranslateableData[fileNo];
		_gameTranslateableData[fileNo] = nullptr;
	}
	
	if (_gameTranslateableData[number] != nullptr)
		return;

	parseTranslations(Common::kPlatformUnknown, Common::UNK_LANG, _mode, number, _gameTranslateableData, nullptr, nullptr, _fileOffsetList, nullptr);
}

const FanTranslationHandler::FontProperties &FanTranslationHandler::getFontProperties() const {
	if (_fontProperties == nullptr)
		error("%s(): Font properties not initialized", __FUNCTION__);
	return *_fontProperties;
}

void FanTranslationHandler::saveFile() {
	const char l1[] =	"; This file is used for Snatcher fan translations.\n\n"
						"; It is grouped into platform and language tags [[ PLATFORM: <x> ][ LANGUAGE: <x> ]]. The first language\n"
						"; tag for each platform [ LANGUAGE: <x> [DEFAULT] ] is the built in language for the platform. It serves\n"
						"; as the template for the actual translations. Under the platform and language tags are tags for each file\n"
						"; (*** FILE: ***) and underneath those are the text strings.\n\n"
						"; The language tag may have subtags like [ CODEPAGE: <x> ] or [RTL: TRUE ] or [ FONTFILE: <x> ]]. So the\n"
						"; whole tag would be : [[ PLATFORM: <x> ] [ LANGUAGE:<x> [ CODEPAGE:<x> ] [ RTL:TRUE ] [ FONTFILE:<x> ]]].\n"
						"; These subtags are optional and currently only get parsed, but do not have any actual effect.\n\n"
						"; To add a new translation, copy the whole block (starting with the platform and language tag), paste it\n"
						"; underneath, change the language inside that newly inserted tag with the appropriate language descriptor\n"
						"; in the same format as our Common::Language enum, e. g. FR_FRA, IT_ITA, etc. Remove the [ DEFAULT ] sub\n"
						"; tag from the new language tag.\n\n"
						"; For the actual translation, only replace the text strings inside the quotation marks on the right of\n"
						"; the '<hex num>:'. If there is a leading number that apparently doesn't seem to be part of the displayed\n"
						"; sentence (e. g. '2Mika'), leave it there: This is a text color setting. Also, leave the special control\n"
						"; markers (like '\\xFx') and similar things unchanged. Stick to regular characters that can actually be\n"
						"; drawn with the game font (no ç, ä, ü, ø, etc.). The '\\xF2' command is a line break marker, so try to\n"
						"; format your translation to have a similar width and height. Put an end marker '\\xFF' at the end of\n"
						"; each line (although we check that and add it if it's missing).\n\n";
	const char l2[] = "[ VERSION: %04d ]\n";
	const char l3[] = "\n[[ PLATFORM: %s ] [ LANGUAGE: %s ]]\n";
	const char l4[] = "\n*** FILE: %02d ***\n";
	const char l5[] = "[ DEFAULT ]";

	Common::Path filePath = Common::Path("snatcher_translations_new.dat");
	if (ConfMan.hasKey("extrapath"))
		filePath = ConfMan.getPath("extrapath").toString() + filePath.toString();

	Common::DumpFile outfile;
	if (!outfile.open(filePath))
		error("%s(): Could not open translation file '%s' for writing.", __FUNCTION__, filePath.toString().c_str());

	outfile.write(l1, sizeof(l1) - 1);

	Common::Platform platforms[] = {
		Common::kPlatformSegaCD,
		Common::kPlatformPCEngine,
		Common::kPlatformPC98,
		Common::kPlatformSaturn,
		Common::kPlatformPSX
	};

	/* Common::Language languages[] = {
		Common::EN_ANY,
		Common::JA_JPN
	};*/

	assert(_gameTranslateableData);

	PerFileTranslateableData **tempTranslateableData = new PerFileTranslateableData*[100]();

	Common::String ostr = Common::String::format(l2, SNATCHER_FANTRANSLATIONS_CURRENT_VERSION);
	outfile.write(ostr.c_str(), ostr.size());

	for (int i = 0; i < ARRAYSIZE(platforms); ++i) {
		Common::Language defaultLang = Common::UNK_LANG;
		Common::List<Common::Language> langList = listTranslations(platforms[i], &defaultLang);
		if (defaultLang != Common::UNK_LANG)
			langList.push_front(defaultLang);

		for (Common::List<Common::Language>::const_iterator lng = langList.begin(); lng != langList.end(); ++lng) {
			PerFileTranslateableData **data = nullptr;
			if (_platform == platforms[i] && _defaultLang == *lng) {
				data = _gameTranslateableData;
			} else {
				data = tempTranslateableData;
				for (int ii = 0; ii < 100; ++ii) {
					delete data[ii];
					data[ii] = nullptr;
				}
				parseTranslations(platforms[i], *lng, kCollect, -1, data, nullptr, nullptr, nullptr, nullptr);
			}

			Common::String langStr(Common::getLanguageCode(*lng));
			if (_defaultLang == *lng)
				langStr = langStr + " " + l5;
			langStr.toUppercase();

			Common::String platformStr(Common::getPlatformCode(platforms[i]));
			platformStr.toUppercase();

			ostr = Common::String::format(l3, platformStr.c_str(), langStr.c_str());
			outfile.write(ostr.c_str(), ostr.size());

			for (int iii = 0; iii < 100; ++iii) {
				if (!data[iii])
					continue;
				ostr = Common::String::format(l4, iii);
				outfile.write(ostr.c_str(), ostr.size());

				PerFileTranslateableData &d = *data[iii];
				Common::List<Common::String>::iterator j = d.stringTable.begin();
				for (Common::List<uint32>::iterator it = d.addressTable.begin(); it != d.addressTable.end(); ++it, ++j) {
					ostr = Common::String::format("%06X: \"%s\"\n", *it, j->c_str());
					outfile.write(ostr.c_str(), ostr.size());
				}
			}
		}
	}

	if (tempTranslateableData) {
		for (int i = 0; i < 100; ++i)
			delete tempTranslateableData[i];
		delete[] tempTranslateableData;
		tempTranslateableData = nullptr;
	}

	outfile.finalize();
	outfile.close();
}

} // End of namespace Snatcher

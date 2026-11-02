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

Common::List<Common::Language> FanTranslationHandler::listTranslations(Common::Platform platform, Common::Language *defaultLang) {
	Common::List<Common::Language> res;
	parseTranslations(platform, Common::UNK_LANG, kDetect, -1, nullptr, &res, defaultLang, nullptr, nullptr);
	return res;
}

const FanTranslationHandler::FontProperties &FanTranslationHandler::getFontProperties() const {
	if (_fontProperties == nullptr)
		error("%s(): Font properties not initialized", __FUNCTION__);
	return *_fontProperties;
}

void FanTranslationHandler::updateDataTable(uint32 addr, const uint8 *data, PerFileTranslateableData **table, TranslatorMode mode, uint dataSize, int file) {
	Common::String tmp(reinterpret_cast<const char *>(data), dataSize);
	size_t tt = tmp.findFirstOf(-1);
	tmp.chop(tmp.size() - tt - 1);

	if (mode == kCollect) {
		for (uint i = 0; i < tmp.size(); ++i) {
			uint8 c = tmp[i];
			if (c < 32 || c > 127) {
				tmp.replace(i, 1, Common::String::format("\\x%02X", static_cast<uint8>(c)));
				i += 3;
			} else if (c == '\"') {
				tmp.replace(i, 1, "\\\"");
				i++;
			}
		}
	}

	assert(table);
	if (!table[file])
		table[file] = new PerFileTranslateableData();

	PerFileTranslateableData &d = *table[file];

	Common::List<Common::String>::iterator j = d.stringTable.begin();
	for (Common::List<uint32>::iterator i = d.addressTable.begin(); i != d.addressTable.end(); ++i, ++j) {
		if (addr == *i) {
			addr = 0;
			break;
		}
		if (addr < *i) {
			d.addressTable.insert(i, addr);
			d.stringTable.insert(j, tmp);
			if (file == -1)
				debug("0x%06x=\"%s\"", addr, tmp.c_str());
			addr = 0;
			break;
		}
	}

	if (addr != 0) {
		d.addressTable.push_back(addr);
		d.stringTable.push_back(tmp);
		if (file == -1)
			debug("0x%06x=\"%s\"", addr, tmp.c_str());
	}
}

const struct CPMapping {
	Common::CodePage cp;
	const char str[5];
} cpMapping[] = {
	{Common::kWindows1250, "1250"},
	{Common::kWindows1251, "1251"},
	{Common::kWindows1252, "1252"},
	{Common::kWindows1253, "1253"},
	{Common::kWindows1254, "1254"},
	{Common::kWindows1255, "1255"},
	{Common::kWindows1256, "1256"},
	{Common::kWindows1257, "1257"},
	{Common::kWindows932, "932"},
	{Common::kWindows936, "936"},
	{Common::kWindows949, "949"},
	{Common::kWindows950, "950"},
	{Common::kDos850, "850"},
	{Common::kDos862, "862"},
	{Common::kDos866, "866"},
};

void FanTranslationHandler::parseTranslations(Common::Platform platform, Common::Language lang, TranslatorMode mode, int fileNumber,
	PerFileTranslateableData **table, Common::List<Common::Language> *langList, Common::Language *defaultLang, uint32 *fileOffsetList, FontProperties *fontProperties) {

	Common::Path fileName = Common::Path("snatcher_translations.dat");
	Common::File file;
	if (!file.open(fileName)) {

		// Some lower level magic for the detection/metaengine phase where the normal file search hasn't been initiated yet.
		const char *keys[] = {"path", "extrapath"};
		for (int i = 0; i < ARRAYSIZE(keys); ++i) {
			if (ConfMan.hasKey(keys[i])) {
				Common::Path filePath(ConfMan.getPath(keys[i]));
				Common::FSList list;
				Common::FSNode(filePath).getChildren(list, Common::FSNode::kListFilesOnly);
				for (Common::FSList::const_iterator it = list.begin(); it != list.end(); ++it) {
					if (it->getName().equalsIgnoreCase(fileName.toString())) {
						file.open(*it);
						break;
					}
				}
				if (file.isOpen())
					break;
			}
		}

		if (!file.isOpen())
			return;
	}

	if (platform == Common::Platform::kPlatformUnknown && !fileOffsetList)
		error("%s(): Platform must be specified to load a translation", __FUNCTION__);

	uint16 blockSize = 5000;
	uint16 extraBytes = 5;
	uint8 *buff = new uint8[blockSize + extraBytes]();

	bool leftPos = true;
	bool skipLine = false;
	int parseTag = 0;
	bool parseFile = false;
	bool parseMapping = false;
	bool parseText = false;
	bool saveOffset = false;
	bool isDefault = false;

	Common::String tmp;
	Common::String versionStr;
	Common::String platformStr;
	Common::String languageStr;
	Common::String fileNoStr;
	Common::String currentLine;
	Common::String extraCp;
	Common::String extraRTL;
	Common::String extraFont;
	Common::Language *defLang = nullptr;

	int fileNo = -1;
	uint32 addr = 0;
	Common::String *dstr = &tmp;
	uint16 bytesLeft = file.size();
	uint16 readSize = 0;
	int diff = 0;

	if (fileNumber != -1 && fileOffsetList != nullptr && (uint)fileNumber < 100) {
		uint32 offs = fileOffsetList[fileNumber];
		if (offs == 0)	// Many module files do not actually contain any text.
			return;
		file.seek(offs);
		bytesLeft -= offs;
		fileNo = fileNumber;
		parseMapping = true;
		dstr = &currentLine;
	}

	while (bytesLeft > 0) {
		diff = MAX<int>(readSize + diff - blockSize, 0);
		if (diff)
			memmove(buff, buff + blockSize + extraBytes - diff, diff);
		readSize = file.read(buff + diff, blockSize + extraBytes - diff);
		bytesLeft -= readSize;
		if (bytesLeft == 0)
			extraBytes = 0;

		const char *p = reinterpret_cast<const char *>(buff);
		const char *last = nullptr;
		const char *end = p + readSize - extraBytes + diff;

		while (p < end) {
			if (p == last) // avoid infinite loop on invalid data
				error("%s(): Fan translation file '%s' is corrupt or invalid. Error at pos: 0x%04x", __FUNCTION__, fileName.toString().c_str(), static_cast<int>(file.pos()) - readSize - diff + static_cast<uint32>(reinterpret_cast<const uint8*>(p) - buff));
			last = p;

			if ((leftPos || parseTag || parseFile) && (*p == ' ' || *p == '*')) {
				if (*p++ == '*') {
					if (leftPos && !parseFile) {
						fileNo = -1;
						parseFile = true;
					}
					if (!fileNoStr.empty()) {
						parseMapping = true;
						fileNo = fileNoStr.asUint64();
						fileNoStr.clear();
						dstr = &currentLine;
						parseFile = false;
						saveOffset = (platform != Common::kPlatformUnknown && lang != Common::UNK_LANG && mode == kTranslate && fileOffsetList != nullptr && fileNumber == -1);
					}
				}
				continue;
			}
			if (*p == '\r' || *p == '\n') {
				skipLine = false;
				if (!fileNoStr.empty()) {
					parseMapping = true;
					fileNo = fileNoStr.asUint64();
					fileNoStr.clear();
					dstr = &currentLine;
					parseFile = false;
					saveOffset = (platform != Common::kPlatformUnknown && lang != Common::UNK_LANG && mode == kTranslate && fileOffsetList != nullptr && fileNumber == -1);
				}
				if (parseText) {
					if (currentLine.lastChar() != '\xFF')
						currentLine += '\xFF';
					if ((mode == kCollect || fileNumber != -1) && table != nullptr)
						updateDataTable(addr, reinterpret_cast<const uint8*>(currentLine.c_str()), table, mode, currentLine.size(), fileNo);
					addr = 0;
					parseText = false;
				}
				currentLine.clear();
				leftPos = true;
				++p;
				if (saveOffset)
					fileOffsetList[fileNo] = file.pos() - readSize - diff + static_cast<uint32>(reinterpret_cast<const uint8*>(p) - buff);
				saveOffset = false;
				continue;
			}
			leftPos = false;
			if (*p == ';') {
				skipLine = true;
				++p;
				continue;
			}
			if (skipLine) {
				++p;
				continue;
			}
			if (*p == '[') {
				if (++parseTag == 1) {
					platformStr.clear();
					languageStr.clear();
					versionStr.clear();
					extraCp.clear();
					extraRTL.clear();
					extraFont.clear();
					tmp.clear();
					fileNo = -1;
					parseFile = false;
					isDefault = false;
					dstr = &tmp;
					if (fileNumber != -1) {
						bytesLeft = 0;
						p = end;
					}
				}
				if (parseTag == 3) {
					tmp.clear();
					dstr = &tmp;
				}
				++p;
				continue;
			}
			if (*p == ']') {
				if (--parseTag == 0) {
					if (!versionStr.empty()) {
						int version = versionStr.asUint64();
						if (version < SNATCHER_FANTRANSLATIONS_CURRENT_VERSION)
							warning("Translation file '%s' is outdated (version %d, expected %d)", fileName.toString().c_str(), version, SNATCHER_FANTRANSLATIONS_CURRENT_VERSION);
						versionStr.clear();
					}
					if (Common::parsePlatform(platformStr) == platform) {
						if ((mode == kCollect && lang == Common::UNK_LANG) || fileNumber != -1 || (lang != Common::UNK_LANG && (Common::parseLanguage(languageStr) == lang || Common::parseLanguage(languageStr.substr(0, 2)) == lang))) {
							parseFile = true;
						} else if (mode == kDetect && lang == Common::UNK_LANG) {
							Common::Language l = Common::parseLanguage(languageStr);
							if (l == Common::UNK_LANG)
								l = Common::parseLanguage(languageStr.substr(0, 2));
							if (l != Common::UNK_LANG) {
								if (defLang != nullptr)
									*defLang = l;
								else if (langList != nullptr)
									langList->push_back(l);
							}
						}
						languageStr.clear();
						defLang = nullptr;
					}
				}
				if (parseTag == 2) {
					if (isDefault)
						defLang = defaultLang;
					bool processResult = (defLang == nullptr && (Common::parseLanguage(languageStr) != Common::UNK_LANG || Common::parseLanguage(languageStr.substr(0, 2)) != Common::UNK_LANG) && fileOffsetList != nullptr);
					if (!extraCp.empty()) {
						if (processResult) {
							for (uint i = 0; i < ARRAYSIZE(cpMapping); ++i) {
								if (extraCp.equalsIgnoreCase(cpMapping[i].str)) {
									fontProperties->codepage = cpMapping[i].cp;
									break;
								}
							}
						}
						extraCp.clear();
					}
					if (!extraFont.empty()) {
						if (processResult)
							fontProperties->fontfile = extraFont;
						extraFont.clear();
					}
					if (!extraRTL.empty()) {
						if (processResult)
							fontProperties->rtl = extraRTL.equalsIgnoreCase("TRUE") || extraRTL.equalsIgnoreCase("YES") || extraRTL.equalsIgnoreCase("1");
						extraRTL.clear();
					}
				}
				if (parseTag < 1)
					parseTag = 0;
				tmp.clear();
				dstr = &tmp;
				++p;
				continue;
			}
			if (parseTag) {
				*dstr += *p++;
				if (dstr->equalsIgnoreCase("VERSION:"))
					dstr = &versionStr;
				else if (dstr->equalsIgnoreCase("PLATFORM:"))
					dstr = &platformStr;
				else if (dstr->equalsIgnoreCase("LANGUAGE:"))
					dstr = &languageStr;
				else if (dstr->equalsIgnoreCase("CODEPAGE:"))
					dstr = &extraCp;
				else if (dstr->equalsIgnoreCase("FONTFILE:"))
					dstr = &extraFont;
				else if (dstr->equalsIgnoreCase("RTL:"))
					dstr = &extraRTL;
				else if (dstr->equalsIgnoreCase("DEFAULT"))
					isDefault = true;
				continue;
			}
			if (parseFile) {
				*dstr += *p++;
				if (dstr->equalsIgnoreCase("FILE:"))
					dstr = &fileNoStr;
				continue;
			}
			if (parseMapping) {
				if (!parseText && (*p == ' ' || *p == ':')) {
					if (addr == 0) {
						if (currentLine.substr(0, 2).equalsIgnoreCase("0x") == false)
							currentLine = "0x" + currentLine;
						addr = currentLine.asUint64Ext();
					}
					currentLine.clear();
					++p;
					continue;
				}
				if (parseText) {
					if (*p == '\\') {
						++p;
						if (*p == 'x' || *p == 'X') {
							*dstr += static_cast<char>(Common::String("0x" + Common::String(p + 1, 2)).asUint64Ext());
							p += 3;
						} else {
							if (*p == '\"')
								*dstr += '\"';
							++p;
						}
						continue;
					}
				}
				if (*p == '\"') {
					if (parseText) {
						if (currentLine.lastChar() != '\xFF')
							currentLine += '\xFF';
						if (((mode == kCollect && (lang == Common::UNK_LANG) == isDefault) || fileNumber != -1) && table != nullptr)
							updateDataTable(addr, reinterpret_cast<const uint8*>(currentLine.c_str()), table, mode, currentLine.size(), fileNo);
						currentLine.clear();
						addr = 0;
						parseText = false;
					} else {
						parseText = true;
					}
					++p;
					continue;
				}

				*dstr += *p++;
				continue;
			}
			++p;
		}
		diff -= (p - end);
	}

	delete[] buff;
	file.close();
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

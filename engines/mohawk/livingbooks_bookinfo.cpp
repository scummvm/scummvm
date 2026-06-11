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

#include "mohawk/livingbooks.h"

#include "common/textconsole.h"

namespace Mohawk {

void MohawkEngine_LivingBooks::loadBookInfo(const Common::Path &filename) {
	_bookInfoFile.allowNonEnglishCharacters();
	if (!_bookInfoFile.loadFromFile(filename))
		error("Could not open %s as a config file", filename.toString().c_str());

	_title = getStringFromConfig("BookInfo", "title");
	_copyright = getStringFromConfig("BookInfo", "copyright");

	_numPages = getIntFromConfig("BookInfo", "nPages");
	_numLanguages = getIntFromConfig("BookInfo", "nLanguages");
	_screenWidth = getIntFromConfig("BookInfo", "xRes");
	_screenHeight = getIntFromConfig("BookInfo", "yRes");
	// nColors is here too, but it's always 256 anyway...

	// this is 1 in The New Kid on the Block, changes the hardcoded UI
	// v2 games changed the flag name to fPoetry
	if (getGameType() == GType_LIVINGBOOKSV1)
		_poetryMode = (getIntFromConfig("BookInfo", "poetry") == 1);
	else
		_poetryMode = (getIntFromConfig("BookInfo", "fPoetry") == 1);

	// The later Living Books games add some more options:
	//     - fNeedPalette                (always true?)
	//     - fUse254ColorPalette         (always true?)
	//     - nKBRequired                 (4096, RAM requirement?)
	//     - fDebugWindow                (always 0?)

	if (_bookInfoFile.hasSection("Globals")) {
		const Common::INIFile::SectionKeyList globals = _bookInfoFile.getKeys("Globals");
		for (Common::INIFile::SectionKeyList::const_iterator i = globals.begin(); i != globals.end(); i++) {
			Common::String command = Common::String::format("%s = %s", i->key.c_str(), i->value.c_str());
			LBCode tempCode(this, 0);
			uint offset = tempCode.parseCode(command);
			tempCode.runCode(nullptr, offset);
		}
	}
}

Common::String MohawkEngine_LivingBooks::stringForMode(LBMode mode) {
	Common::String language = getStringFromConfig("Languages", Common::String::format("Language%d", _curLanguage));

	switch (mode) {
	case kLBIntroMode:
		return "Intro";
	case kLBControlMode:
		return "Control";
	case kLBCreditsMode:
		return "Credits";
	case kLBPreviewMode:
		return "Preview";
	case kLBReadMode:
		return language + ".Read";
	case kLBPlayMode:
		return language + ".Play";
	default:
		error("unknown game mode %d", (int)mode);
	}
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(const Common::String &section, const Common::String &key) {
	Common::String x, leftover;
	_bookInfoFile.getKey(key, section, x);
	Common::String tmp = removeQuotesFromString(x, leftover);
	if (!leftover.empty())
		warning("while parsing config key '%s' from section '%s', string '%s' was left after '%s'",
			key.c_str(), section.c_str(), leftover.c_str(), tmp.c_str());
	return tmp;
}

Common::String MohawkEngine_LivingBooks::getStringFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover) {
	Common::String x;
	_bookInfoFile.getKey(key, section, x);
	return removeQuotesFromString(x, leftover);
}

int MohawkEngine_LivingBooks::getIntFromConfig(const Common::String &section, const Common::String &key) {
	return atoi(getStringFromConfig(section, key).c_str());
}

Common::String MohawkEngine_LivingBooks::getFileNameFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover) {
	Common::String string = getStringFromConfig(section, key, leftover);

	if (string.hasPrefix("//")) {
		// skip "//CD-ROM Title/" prefixes which we don't care about
		uint i = 3;
		while (i < string.size() && string[i - 1] != '/')
			i++;

		// Already uses slashes, no need to convert further
		return string.c_str() + i;
	}

	return (getPlatform() == Common::kPlatformMacintosh) ? convertMacFileName(string) : convertWinFileName(string);
}

Common::String MohawkEngine_LivingBooks::removeQuotesFromString(const Common::String &string, Common::String &leftover) {
	if (string.empty())
		return string;

	char quoteChar = string[0];
	if (quoteChar != '\"' && quoteChar != '\'')
		return string;

	Common::String tmp;
	bool inLeftover = false;
	for (uint32 i = 1; i < string.size(); i++) {
		if (inLeftover)
			leftover += string[i];
		else if (string[i] == quoteChar)
			inLeftover = true;
		else
			tmp += string[i];
	}

	return tmp;
}

Common::String MohawkEngine_LivingBooks::convertMacFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (i == 0 && string[i] == ':') // First character should be ignored (another colon)
			continue;
		if (string[i] == ':') // Directory separator
			filename += '/';
		else if (string[i] == '/') // Literal slash
			filename += ':'; // Replace by colon, as used by macOS for slash
		else
			filename += string[i];
	}

	return filename;
}

Common::String MohawkEngine_LivingBooks::convertWinFileName(const Common::String &string) {
	Common::String filename;

	for (uint32 i = 0; i < string.size(); i++) {
		if (i == 0 && (string[i] == '/' || string[i] == '\\')) // ignore slashes at start
			continue;
		if (string[i] == '\\')
			filename += '/';
		else
			filename += string[i];
	}

	return filename;
}

} // End of namespace Mohawk

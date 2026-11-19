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

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/language.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/text_renderer.h"
#include "harold/bcb/translation.h"

namespace BCB {
namespace {

Common::String getTranslationStringFileName(const Common::String &code) {
	return Common::String::format("strings_%s.yml", code.c_str());
}

bool getTranslationPackageCode(const Common::String &fileName, Common::String &code) {
	static const char *const prefix = "bcb-translation-";
	static const char *const suffix = ".dat";

	Common::String lowerFileName = fileName;
	lowerFileName.toLowercase();
	if (lowerFileName.hasPrefix("bcb_translation_"))
		lowerFileName = Common::String(prefix) + lowerFileName.substr(strlen("bcb_translation_"));
	if (!lowerFileName.hasPrefix(prefix) || !lowerFileName.hasSuffix(suffix))
		return false;

	code = lowerFileName.substr(strlen(prefix), lowerFileName.size() - strlen(prefix) - strlen(suffix));
	return !code.empty();
}

Common::String getTranslationPackageDisplayName(const Common::String &code) {
	const Common::Language language = Common::parseLanguage(code);
	const char *description = Common::getLanguageDescription(language);
	return description ? Common::String(description) : code;
}

bool packageHasMatchingStrings(const Common::FSNode &packageNode, const Common::String &code) {
	Common::Archive *archive = Common::makeZipArchive(packageNode, false);
	if (!archive)
		return false;

	const bool hasStrings = archive->hasFile(Common::Path(getTranslationStringFileName(code), '/'));
	delete archive;
	return hasStrings;
}

int parseHexByte(const char *text) {
	uint value = 0;
	if (sscanf(text, "%02x", &value) != 1)
		return -1;
	return value & 0xff;
}

Common::String unescapeQuotedString(Common::String value) {
	if (value.hasPrefix("\"") && value.hasSuffix("\"")) {
		value.deleteChar(0);
		value.deleteLastChar();
	}

	for (uint i = 0; i < value.size(); ++i) {
		if (value[i] != '\\' || i + 1 >= value.size())
			continue;

		const char escaped = value[i + 1];
		if (escaped == 'n' || escaped == 'r') {
			value.deleteChar(i);
			value.setChar('\n', i);
		} else if (escaped == 't') {
			value.deleteChar(i);
			value.setChar('\t', i);
		} else if (escaped == '\\' || escaped == '"') {
			value.deleteChar(i);
		} else if (escaped == 'x' && i + 3 < value.size()) {
			const int byteValue = parseHexByte(value.c_str() + i + 2);
			if (byteValue < 0)
				error("BCB translation string has invalid hex escape near \"%s\"", value.c_str() + i);
			value.deleteChar(i);
			value.deleteChar(i);
			value.deleteChar(i);
			value.setChar((char)byteValue, i);
		}
	}

	return value;
}

} // End of anonymous namespace

bool TextStringTable::load(Common::SeekableReadStream *stream) {
	Common::ScopedPtr<Common::SeekableReadStream> textStream(stream);
	if (!textStream)
		return false;

	clear();

	Common::Array<Common::String> prefixKeys;
	while (!textStream->eos()) {
		Common::String line = textStream->readLine();
		Common::String lineTrimmed = line;
		lineTrimmed.trim();
		if (lineTrimmed.empty() || lineTrimmed.hasPrefix("#"))
			continue;

		size_t indent = 0;
		while (line.hasPrefix("\t")) {
			line.deleteChar(0);
			++indent;
		}

		const size_t separator = line.findFirstOf(":");
		if (separator == Common::String::npos)
			error("BCB translation string line encountered without colon");

		Common::String key(line.c_str(), line.c_str() + separator);
		Common::String value(line.c_str() + separator + 1);
		key.trim();
		value.trim();

		if (indent > prefixKeys.size())
			error("BCB translation string table has incorrect indentation");
		prefixKeys.resize(indent);

		if (value.empty()) {
			prefixKeys.push_back(key);
			continue;
		}

		Common::String fullKey;
		for (uint i = 0; i < prefixKeys.size(); ++i) {
			fullKey += prefixKeys[i];
			fullKey += ".";
		}
		fullKey += key;

		(*this)[fullKey] = unescapeQuotedString(value);
	}

	return true;
}

const char *TextStringTable::getString(const Common::String &key) const {
	const_iterator entry = find(key);
	return entry == end() ? nullptr : entry->_value.c_str();
}

TextTranslationData::TextTranslationData()
	: packageArchive(nullptr) {
}

TextTranslationData::~TextTranslationData() {
	clear();
}

void TextTranslationData::clear() {
	for (auto &entry : imageFonts)
		delete entry._value;
	imageFonts.clear();
	strings.clear();
	delete packageArchive;
	packageArchive = nullptr;
}

HighQualityTextRenderer *TextTranslationData::getImageFont(const Common::String &name) const {
	HighQualityTextRenderer *renderer = imageFonts.getValOrDefault(name, nullptr);
	if (!renderer) {
		renderer = new HighQualityTextRenderer(name);
		imageFonts[name] = renderer;
		if (!renderer->isValid())
			warning("BCB: Could not load translation font '%s'", name.c_str());
	}
	return renderer;
}

Common::String makeTextStringKey(TextTranslationType type, uint primaryId, uint secondaryId) {
	switch (type) {
	case kTextTranslationSceneChoice:
		return Common::String::format("scene_choices.%u.%u", primaryId, secondaryId);
	case kTextTranslationMapButton:
		return Common::String::format("map.%u", primaryId);
	case kTextTranslationGameOver:
		return Common::String::format("game_over.%u", primaryId);
	case kTextTranslationSceneChoiceMove:
		return "system.scene_choice_move";
	}

	return "";
}

static TranslationPackageDefinition &findOrAddTranslation(Common::Array<TranslationPackageDefinition> &packages, const Common::String &code) {
	for (auto &package : packages)
		if (package.code.equalsIgnoreCase(code))
			return package;
	TranslationPackageDefinition definition;
	definition.code = code;
	definition.displayName = getTranslationPackageDisplayName(code);
	packages.push_back(definition);
	return packages.back();
}

static void findLooseTranslations(const Common::FSNode &dir, Common::Array<TranslationPackageDefinition> &packages, const Common::String &directoryCode = Common::String()) {
	Common::FSList children;
	if (!dir.isDirectory() || !dir.getChildren(children, Common::FSNode::kListFilesOnly))
		return;
	for (const auto &child : children) {
		Common::String name = child.getName();
		name.toLowercase();
		if (name.hasPrefix("strings_") && name.hasSuffix(".yml") && name.size() > 12) {
			const Common::String code = name.substr(8, name.size() - 12);
			if (directoryCode.empty() || code.equalsIgnoreCase(directoryCode))
				findOrAddTranslation(packages, code);
		}
	}
}

static Common::FSNode findLanguageDirectory(const Common::FSNode &dir, const Common::String &code) {
	Common::FSList children;
	if (dir.isDirectory() && dir.getChildren(children, Common::FSNode::kListDirectoriesOnly))
		for (const auto &child : children)
			if (child.getName().equalsIgnoreCase(code))
				return child;
	return Common::FSNode();
}

static void findTranslationSources(const Common::FSNode &root, Common::Array<TranslationPackageDefinition> &packages) {
	Common::FSList children;
	if (root.isDirectory() && root.getChildren(children, Common::FSNode::kListFilesOnly)) {
		for (const auto &child : children) {
			Common::String code;
			if (getTranslationPackageCode(child.getName(), code) && packageHasMatchingStrings(child, code)) {
				TranslationPackageDefinition &package = findOrAddTranslation(packages, code);
				if (package.archivePath.empty())
					package.archivePath = child.getPath();
			}
		}
	}
	const Common::FSNode dir = root.getChild("translation");
	findLooseTranslations(dir, packages);
	children.clear();
	if (dir.isDirectory() && dir.getChildren(children, Common::FSNode::kListDirectoriesOnly))
		for (const auto &child : children)
			findLooseTranslations(child, packages, child.getName());
}

void listAvailableTranslationPackages(const Common::FSNode &gameDataDir, Common::Array<TranslationPackageDefinition> &packages, const Common::String &target) {
	packages.clear();
	findTranslationSources(gameDataDir, packages);
	if (ConfMan.hasKey("extrapath", target))
		findTranslationSources(Common::FSNode(ConfMan.getPath("extrapath", target)), packages);
}

bool isTranslationPackageAvailable(const Common::FSNode &gameDataDir, const Common::String &code) {
	Common::Array<TranslationPackageDefinition> packages;
	listAvailableTranslationPackages(gameDataDir, packages);
	for (const auto &package : packages)
		if (package.code.equalsIgnoreCase(code))
			return true;
	return false;
}

bool loadTranslationPackage(const Common::FSNode &gameDataDir, const Common::String &code, TextTranslationData &translation) {
	translation.clear();

	Common::Array<TranslationPackageDefinition> packages;
	listAvailableTranslationPackages(gameDataDir, packages);
	Common::Path archivePath;
	bool found = false;
	for (const auto &package : packages) {
		if (package.code.equalsIgnoreCase(code)) {
			archivePath = package.archivePath;
			found = true;
			break;
		}
	}
	if (!found)
		return false;
	Common::SearchSet *sources = new Common::SearchSet();
	translation.packageArchive = sources;
	if (!archivePath.empty()) {
		Common::Archive *archive = Common::makeZipArchive(Common::FSNode(archivePath), false);
		if (archive)
			sources->add("translation archive", archive, 0);
	}
	const Common::FSNode looseDir = gameDataDir.getChild("translation");
	if (looseDir.isDirectory()) {
		sources->addDirectory("game translation", looseDir, 1, 3);
		const Common::FSNode languageDir = findLanguageDirectory(looseDir, code);
		if (languageDir.isDirectory())
			sources->addDirectory("game language translation", languageDir, 2, 3);
	}
	if (ConfMan.hasKey("extrapath")) {
		const Common::FSNode extraDir = Common::FSNode(ConfMan.getPath("extrapath")).getChild("translation");
		if (extraDir.isDirectory()) {
			sources->addDirectory("extra translation", extraDir, 3, 3);
			const Common::FSNode languageDir = findLanguageDirectory(extraDir, code);
			if (languageDir.isDirectory())
				sources->addDirectory("extra language translation", languageDir, 4, 3);
		}
	}

	if (!translation.strings.load(translation.packageArchive->createReadStreamForMember(Common::Path(getTranslationStringFileName(code), '/')))) {
		translation.clear();
		return false;
	}
	static const char *const defaultFonts[] = {
		"LiberationSerif-BoldItalic.ttf",
		"LiberationSans-Regular.ttf",
		"LiberationSans-BoldItalic.ttf",
		"LiberationSerif-Regular.ttf"
	};
	for (const char *font : defaultFonts)
		translation.getImageFont(font);
	for (const auto &entry : translation.strings) {
		if (entry._key.hasSuffix(".font") && !entry._value.empty())
			translation.getImageFont(entry._value);
	}
	return true;
}

const char *getTextTranslation(const TextTranslationData &translation, TextTranslationType type, uint primaryId, uint secondaryId) {
	return translation.strings.getString(makeTextStringKey(type, primaryId, secondaryId));
}

const char *getSystemTextTranslation(const TextTranslationData &translation, const char *key) {
	return translation.strings.getString(Common::String::format("system.%s", key));
}

Common::U32String BCBEngine::translateTextForDisplay(TextTranslationType type, uint primaryId, uint secondaryId, const Common::U32String &originalText, bool *translated) const {
	if (translated)
		*translated = false;

	if (!_selectedTextLanguagePackageAvailable)
		return originalText;

	const char *translatedText = getTextTranslation(_textTranslation, type, primaryId, secondaryId);
	if (!translatedText || !translatedText[0])
		return originalText;

	if (translated)
		*translated = true;
	return Common::U32String(translatedText);
}

Common::U32String BCBEngine::translateSystemTextForDisplay(const char *key, const Common::U32String &originalText, bool *translated) const {
	if (translated)
		*translated = false;

	if (!_selectedTextLanguagePackageAvailable)
		return originalText;

	const char *translatedText = getSystemTextTranslation(_textTranslation, key);
	if (!translatedText || !translatedText[0])
		return originalText;

	if (translated)
		*translated = true;
	return Common::U32String(translatedText);
}

Common::U32String BCBEngine::formatSystemTextForDisplay(const char *key, const Common::String &originalSjisFormat, int arg1) const {
	const char *translatedFormat = _selectedTextLanguagePackageAvailable ? getSystemTextTranslation(_textTranslation, key) : nullptr;
	if (translatedFormat && translatedFormat[0]) {
		const Common::String formatted = Common::String::format(translatedFormat, arg1);
		return Common::U32String(formatted.c_str());
	}

	const Common::String formatted = Common::String::format(originalSjisFormat.c_str(), arg1);
	return decodeExecutableTextForDisplay(formatted);
}

Common::U32String BCBEngine::formatSystemClockTextForDisplay(const char *key, const Common::String &originalSjisFormat, int slot, int savedDay, int savedClockTicks) const {
	const char *translatedFormat = _selectedTextLanguagePackageAvailable ? getSystemTextTranslation(_textTranslation, key) : nullptr;
	if (translatedFormat && translatedFormat[0]) {
		const int clockTicks = savedClockTicks % (24 * 60);
		const int hour24 = clockTicks / 60;
		const int minute = clockTicks % 60;
		const int hour12 = hour24 % 12 == 0 ? 12 : hour24 % 12;
		Common::String formatted(translatedFormat);
		Common::replace(formatted, "{slot}", Common::String::format("%d", slot));
		Common::replace(formatted, "{day}", Common::String::format("%d", 10 + savedDay));
		Common::replace(formatted, "{hour24}", Common::String::format("%d", hour24));
		Common::replace(formatted, "{hour12}", Common::String::format("%d", hour12));
		Common::replace(formatted, "{min}", Common::String::format("%02d", minute));
		Common::replace(formatted, "{ampm}", hour24 < 12 ? "AM" : "PM");
		return Common::U32String(formatted.c_str());
	}

	const int hour = savedClockTicks / 60;
	const int minute = savedClockTicks % 60;
	const Common::String formatted = Common::String::format(originalSjisFormat.c_str(), slot, savedDay, hour, minute);
	return decodeExecutableTextForDisplay(formatted);
}

} // End of namespace BCB

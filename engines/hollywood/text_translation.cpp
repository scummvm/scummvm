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
#include "common/ptr.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "hollywood/image_translation.h"
#include "hollywood/text_translation.h"

namespace Hollywood {

TextTranslation::TextTranslation() : _loaded(false) {
}

TextTranslation::~TextTranslation() {
	SearchMan.remove("monsters-translated-graphics");
}

bool TextTranslation::load(const Common::Path &fileName, const Common::Path &originalArchive) {
	_loaded = false;
	_text.clear();
	_language.clear();
	_languages.clear();
	SearchMan.remove("monsters-translated-graphics");
#ifdef USE_TRANSLATION
	_catalog.reset();
	_graphics.reset();
	_archive.reset();
	Common::ScopedPtr<Common::Archive> archive(Common::makeZipArchive(fileName));
	if (!archive || !archive->hasFile("monsters.dat")) {
		warning("Unable to open text translation: %s", fileName.toString().c_str());
		return false;
	}

	SearchMan.add("monsters-translated-text", archive.get(), 1, false);
	Common::ScopedPtr<Common::TranslationManager> catalog(new Common::TranslationManager("monsters.dat"));
	SearchMan.remove("monsters-translated-text");
	Common::ScopedPtr<ImageTranslationArchive> graphics(new ImageTranslationArchive(archive.get()));
	if (!graphics->load(originalArchive)) {
		warning("Invalid image translation in %s", fileName.toString().c_str());
		return false;
	}

	const Common::TLangArray languages = catalog->getSupportedLanguageNames();
	for (uint i = 0; i < languages.size(); ++i) {
		const Common::String code = catalog->getLangById(languages[i].id);
		if (!code.hasPrefix("monsters-") || code.size() == 9)
			continue;
		Language language;
		language.code = code.substr(9);
		if (!decodeText(languages[i].name, language.name))
			language.name = language.code;
		_languages.push_back(language);
	}
	if (_languages.empty())
		return false;
	_archive.reset(archive.release());
	_graphics.reset(graphics.release());
	_catalog.reset(catalog.release());
	return true;
#else
	warning("Alternative subtitles require translation support in this build");
	return false;
#endif
}

bool TextTranslation::setLanguage(const Common::String &language) {
	if (language == _language)
		return true;
#ifdef USE_TRANSLATION
	if (!language.empty()) {
		if (!_catalog || _catalog->parseLanguage("monsters-" + language) == Common::kTranslationBuiltinId)
			return false;
		SearchMan.add("monsters-translated-text", _archive.get(), 1, false);
		Common::ScopedPtr<Common::TranslationManager> catalog(new Common::TranslationManager("monsters.dat"));
		catalog->setLanguage("monsters-" + language);
		SearchMan.remove("monsters-translated-text");
		if (catalog->currentIsBuiltinLanguage() || catalog->getContexts().empty()) {
			warning("Unable to load Hollywood subtitles for %s", language.c_str());
			return false;
		}
		_catalog.reset(catalog.release());
	}
	SearchMan.remove("monsters-translated-graphics");
	if (language == "en")
		SearchMan.add("monsters-translated-graphics", _graphics.get(), 1, false);
	_text.clear();
	_language = language;
	_loaded = !language.empty();
	return true;
#else
	return false;
#endif
}

const Common::String *TextTranslation::findText(const Common::String &key) const {
	if (!_loaded)
		return nullptr;
	Common::StringMap::const_iterator entry = _text.find(key);
	if (entry != _text.end())
		return &entry->_value;
#ifdef USE_TRANSLATION
	const Common::U32String value = _catalog->getTranslation(key);
	if (value == Common::U32String(key))
		return nullptr;
	Common::String text;
	if (!decodeText(value, text)) {
		warning("Invalid translated text: %s", key.c_str());
		return nullptr;
	}
	Common::String &cached = _text[key];
	cached = text;
	return &cached;
#else
	return nullptr;
#endif
}

bool TextTranslation::decodeText(const Common::U32String &value, Common::String &text) const {
	text.clear();
	for (uint i = 0; i < value.size(); ++i) {
		uint32 chr = value[i];
		switch (chr) {
		case '\'': chr = '%'; break;
		case 0x201c: chr = 0xf0; break;
		case 0x201d: chr = 0xf1; break;
		default:
			if ((chr < 32 && chr != '\t' && chr != '\v') || chr >= 127)
				return false;
			break;
		}
		text += (char)chr;
	}
	return true;
}

void TextTranslation::applyRows(const Common::String &section, Common::Array<byte> &rows,
		uint rowSize, uint firstRow, uint firstRecord) const {
	if (!_loaded)
		return;

	if (rowSize == 0 || rows.size() % rowSize != 0) {
		warning("Invalid text row size in %s", section.c_str());
		return;
	}

	Common::Array<byte> translated = rows;
	for (uint row = firstRow; row < rows.size() / rowSize; ++row) {
		const uint record = firstRecord + row - firstRow;
		const Common::String key = Common::String::format("%s.%u", section.c_str(), record);
		const Common::String *text = findText(key);
		if (!text)
			continue;
		if (text->size() >= rowSize) {
			warning("Invalid translated record %u in %s", record, section.c_str());
			return;
		}
		memset(translated.data() + row * rowSize, 0, rowSize);
		memcpy(translated.data() + row * rowSize, text->c_str(), text->size());
	}
	rows.swap(translated);
}

const char *TextTranslation::interfaceText(const Common::String &key, const char *fallback) const {
	const Common::String *text = findText("interface." + key);
	return text ? text->c_str() : fallback;
}

} // End of namespace Hollywood

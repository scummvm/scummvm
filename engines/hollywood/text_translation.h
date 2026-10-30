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

#ifndef HOLLYWOOD_TEXT_TRANSLATION_H
#define HOLLYWOOD_TEXT_TRANSLATION_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/ustr.h"

namespace Common {
class Archive;
class TranslationManager;
}

namespace Hollywood {

class ImageTranslationArchive;

class TextTranslation {
public:
	struct Language {
		Common::String code;
		Common::String name;
	};

	TextTranslation();
	~TextTranslation();

	bool load(const Common::Path &fileName, const Common::Path &originalArchive);
	bool setLanguage(const Common::String &language);
	const Common::String &language() const { return _language; }
	const Common::Array<Language> &languages() const { return _languages; }
	void applyRows(const Common::String &section, Common::Array<byte> &rows,
		uint rowSize, uint firstRow, uint firstRecord) const;
	const char *interfaceText(const Common::String &key, const char *fallback) const;

private:
	const Common::String *findText(const Common::String &key) const;
	bool decodeText(const Common::U32String &value, Common::String &text) const;

#ifdef USE_TRANSLATION
	Common::ScopedPtr<Common::Archive> _archive;
	Common::ScopedPtr<ImageTranslationArchive> _graphics;
	Common::ScopedPtr<Common::TranslationManager> _catalog;
#endif
	Common::String _language;
	Common::Array<Language> _languages;
	mutable Common::StringMap _text;
	bool _loaded;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_TEXT_TRANSLATION_H

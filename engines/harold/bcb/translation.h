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

#ifndef BCB_TRANSLATION_H
#define BCB_TRANSLATION_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class Archive;
class FSNode;
class SeekableReadStream;
} // namespace Common

namespace Graphics {
struct Surface;
}

namespace BCB {

struct TranslationPackageDefinition {
	Common::String code;
	Common::String displayName;
	Common::String fileName;
};

enum TextTranslationType {
	kTextTranslationSceneChoice,
	kTextTranslationMapButton,
	kTextTranslationGameOver,
	kTextTranslationSceneChoiceMove
};

class TextStringTable : public Common::StringMap {
public:
	bool load(Common::SeekableReadStream *stream);
	const char *getString(const Common::String &key) const;
};

class TextTranslationData {
public:
	TextTranslationData();
	~TextTranslationData();

	void clear();

	TextStringTable strings;
	Common::Archive *packageArchive;
};

Common::String makeTextStringKey(TextTranslationType type, uint primaryId, uint secondaryId);
void listAvailableTranslationPackages(const Common::FSNode &gameDataDir, Common::Array<TranslationPackageDefinition> &packages);
bool isTranslationPackageAvailable(const Common::FSNode &gameDataDir, const Common::String &code);
bool loadTranslationPackage(const Common::FSNode &gameDataDir, const Common::String &code, TextTranslationData &translation);
void applyTranslatedStillImage(Graphics::Surface &still, int stillIndex, const TextTranslationData &translation);
void applyTranslatedTitleMenu(Graphics::Surface &title, const TextTranslationData &translation);
void applyTranslatedComputerButtons(Graphics::Surface &computer, const TextTranslationData &translation);

const char *getTextTranslation(const TextTranslationData &translation, TextTranslationType type, uint primaryId, uint secondaryId);
const char *getSystemTextTranslation(const TextTranslationData &translation, const char *key);

} // End of namespace BCB

#endif

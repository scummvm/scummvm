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

#ifndef MACS2_TRANSLATION_H
#define MACS2_TRANSLATION_H

#include "common/array.h"
#include "common/fs.h"
#include "common/str.h"

namespace Macs2 {

static const char *const kTranslationConfigKey = "macs2_translation";

struct TranslationPackage {
	Common::String code;
	Common::String displayName;
	Common::String fileName;
};

Common::Array<TranslationPackage> listTranslationPackages(const Common::FSNode &gameDataDir, const Common::String &gameId);
Common::String normalizeTranslationString(const Common::String &text);
uint16 remapDemoTranslationObjectId(uint16 objectId);
int remapDemoTranslationSceneStringIndex(int sceneId, int stringIndex);

} // End of namespace Macs2

#endif // MACS2_TRANSLATION_H

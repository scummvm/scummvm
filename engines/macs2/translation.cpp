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

#include "macs2/translation.h"
#include "common/algorithm.h"
#include "common/language.h"
#include "common/ptr.h"
#include "common/stream.h"

namespace Macs2 {

Common::String normalizeTranslationString(const Common::String &text) {
	Common::String result;
	for (char c : text) {
		if ((byte)c != 0xf0) // CP850 soft hyphen; also used to pad unused dialogue lines.
			result += c;
	}
	// An empty DAT entry falls back to German; keep an intentional blank nonempty.
	if (!text.empty() && result.empty())
		result = " ";
	return result;
}

uint16 remapDemoTranslationObjectId(uint16 objectId) {
	// The DOS demo renumbers inventory objects, retaining the NPC IDs.
	static const uint16 fullGameIds[] = {
		0, 1, 85, 87, 96, 97, 6, 98, 99, 101, 102, 103, 12, 107, 108, 15,
		109, 113, 114, 19, 115, 117, 118, 119, 120, 126, 130, 134, 138, 139, 141
	};
	return objectId < ARRAYSIZE(fullGameIds) ? fullGameIds[objectId] : objectId;
}

int remapDemoTranslationSceneStringIndex(int sceneId, int stringIndex) {
	// The DOS demo omits some full-game dialogue and inserts a chapter-end message.
	if (sceneId == 30 && stringIndex == 249)
		return -1;

	static const struct {
		int sceneId;
		int firstString;
		int offset;
	} ranges[] = {
		{30, 250, -1}, {30, 297, 1}, {30, 299, 3}, {30, 304, 5},
		{30, 308, 23}, {30, 313, 26}, {30, 318, 32},
		{31, 77, 17}, {31, 80, 25},
		{32, 3, 2}, {32, 4, 5}, {32, 66, 10}, {32, 70, 17},
		{32, 73, 21}, {32, 75, 25},
		{33, 6, 3}, {33, 82, 11}
	};
	int offset = 0;
	for (const auto &range : ranges) {
		if (range.sceneId == sceneId && stringIndex >= range.firstString)
			offset = range.offset;
	}
	return stringIndex + offset;
}

Common::Array<TranslationPackage> listTranslationPackages(const Common::FSNode &gameDataDir, const Common::String &gameId) {
	Common::Array<TranslationPackage> packages;
	Common::FSList files;
	if (!gameDataDir.getChildren(files, Common::FSNode::kListFilesOnly))
		return packages;

	Common::String prefix = gameId + "-translation-";
	prefix.toLowercase();
	const Common::String suffix = ".dat";
	for (const Common::FSNode &file : files) {
		Common::String name = file.getName();
		name.toLowercase();
		if (!name.hasPrefix(prefix) || !name.hasSuffix(suffix))
			continue;

		const Common::String code = name.substr(prefix.size(), name.size() - prefix.size() - suffix.size());
		if (code.empty())
			continue;

		Common::ScopedPtr<Common::SeekableReadStream> stream(file.createReadStream());
		if (!stream || stream->size() < 14 || stream->readUint32BE() != MKTAG('M', 'C', 'S', '2') || stream->readUint16LE() != 1)
			continue;

		TranslationPackage package;
		package.code = code;
		const char *description = Common::getLanguageDescription(Common::parseLanguage(code));
		package.displayName = description ? description : code;
		package.fileName = file.getName();
		packages.push_back(package);
	}

	Common::sort(packages.begin(), packages.end(), [](const TranslationPackage &a, const TranslationPackage &b) {
		return a.code < b.code;
	});
	return packages;
}

} // End of namespace Macs2

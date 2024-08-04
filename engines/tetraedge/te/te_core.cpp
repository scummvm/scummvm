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

#include "common/file.h"
#include "common/fs.h"
#include "common/debug.h"
#include "common/config-manager.h"
#include "common/language.h"

#include "tetraedge/te/te_core.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_png.h"
#include "tetraedge/te/te_images_sequence.h"
#include "tetraedge/te/te_jpeg.h"
#include "tetraedge/te/te_zlib_jpeg.h"
#include "tetraedge/te/te_theora.h"
#include "tetraedge/te/te_tga.h"

namespace Tetraedge {

TeCore::TeCore() : _loc(nullptr), _coreNotReady(true), _resourcesRoot("") {
	create();
}

void TeCore::addLoc(TeILoc *loc) {
	if (_loc) {
		warning("TeCore::addLoc: There is already a loc");
	}
	_loc = loc;
}

void TeCore::create() {
	const char *langCode = getLanguageCode(g_engine->getGameLanguage());
	const Common::String confLang = ConfMan.get("language");
	Common::String useLang = "en";
	if (langCode)
		useLang = langCode;
	if (!confLang.empty())
		useLang = confLang;
	language(useLang);
	_coreNotReady = false;
	_activityTrackingTimer.alarmSignal().add(this, &TeCore::onActivityTrackingAlarm);
	warning("TODO: TeCore::create: Finish implementing me.");

	const Common::FSNode gameRoot(ConfMan.getPath("path"));
	if (!gameRoot.isDirectory())
		error("Game directory should be a directory");
	const Common::FSNode resNode = (g_engine->getGamePlatform() == Common::kPlatformMacintosh
										? gameRoot.getChild("Resources")
										: gameRoot);
	if (!resNode.isDirectory())
		error("Resources directory should exist in game");

	_resourcesRoot = Common::FSDirectory(resNode, 5, false, false, true);
}

Common::FSNode TeCore::getFSNode(const Common::Path &path) const {
	return Common::FSNode(Common::Path(_resourcesRoot.getFSNode().getPath()).join(path));
}

TeICodec *TeCore::createVideoCodec(const Common::String &extn) {
	// The original engine has more formats and even checks for alpha maps,
	// but it never uses them.
	if (TePng::matchExtension(extn)) {
		// png codec needs to know extension
		return new TePng(extn);
	} else if (TeJpeg::matchExtension(extn)) {
		return new TeJpeg();
	} else if (TeZlibJpeg::matchExtension(extn)) {
		return new TeZlibJpeg();
	} else if (TeTheora::matchExtension(extn)) {
		return new TeTheora();
	} else if (TeTga::matchExtension(extn)) {
		return new TeTga();
	} else if (TeImagesSequence::matchExtension(extn)) {
		return new TeImagesSequence();
	}
	return nullptr;
}

TeICodec *TeCore::createVideoCodec(const Common::Path &path) {
	const Common::String filename = path.baseName();
	if (!filename.contains('.'))
		return nullptr;
	Common::String extn = filename.substr(filename.findLastOf('.') + 1);
	extn.toLowercase();
	TeICodec *codec = createVideoCodec(extn);
	if (!codec)
		error("TTeCore::createVideoCodec: Unrecognised format %s", filename.c_str());
	return codec;
}

const Common::String &TeCore::fileFlagSystemFlag(const Common::String &name) const {
	return _fileSystemFlags.find(name)->_value;
}

void TeCore::fileFlagSystemSetFlag(const Common::String &name, const Common::String &val) {
	// TODO: Impmenent this fully to check possible values
	_fileSystemFlags.setVal(name, val);
}

bool TeCore::fileFlagSystemFlagsContains(const Common::String &name) const {
	error("TODO: Implement TeCore::fileFlagSystemFlagsContains");
	return false;
}

Common::Array<Common::String> TeCore::fileFlagSystemPossibleFlags() {
	error("TODO: Implement TeCore::fileFlagSystemPossibleFlags");
	return Common::Array<Common::String>();
}

bool TeCore::fileFlagSystemPossibleFlagsContains(const Common::String &name) const {
	error("TODO: Implement TeCore::fileFlagSystemPossibleFlagsContains");
	return false;
}

const Common::String &TeCore::language() const {
	return fileFlagSystemFlag("language");
}

void TeCore::language(const Common::String &val) {
	return fileFlagSystemSetFlag("language", val);
}

bool TeCore::onActivityTrackingAlarm() {
	error("TODO: Implement TeCore::onActivityTrackingAlarm");
}

Common::Path TeCore::findFile(const Common::Path &path) const {
	if (Common::File::exists(path))
		return path;

	Common::String fname = path.baseName();

	// Slight HACK: Remove 'comments' used to specify animated pngs
	if (fname.contains('#'))
		fname = fname.substr(0, fname.find('#'));
	const Common::Path dir = path.getParent();

	static const Common::Path pathSuffixes[] = {
		"", // no suffix
		"PC-MacOSX",
		"PC-PS3-Android-MacOSX",
		"PC-MacOSX-Android-iPhone-iPad",
		"PC-MacOSX-Android-iPhone-iPad/HD",
		"PC-Android-MacOSX-iPhone-iPad",
		"PC-MacOSX-Xbox360-PS3",
		"PC-MacOSX-PS3-Xbox360",
		"PC-MacOSX-Xbox360-PS3/PC-MacOSX",
		"PC-MacOSX-MacOSXAppStore-Android-iPhone-iPad",
		"PC-MacOSX-MacOSXAppStore-Xbox360-Android-iPad-iPhone",
		"Android-iPhone-iPad-PC-MacOSX",
		"Android-MacOSX",
		"Full",
		"Part1-Full",
		"Part2-Full-Part1",
		"Part3-Full-Part1",
		"HD",
		"HD/PC-MacOSX-Xbox360-PS3",
		"PC-PS3-Android-MacOSX-iPhone-iPad",	// iOS Syb 1
		"Android-iPhone-iPad",					// iOS Syb 1
		"Android-iPhone-iPad/HD",				// iOS Syb 1
		"HD/Android-iPhone-iPad",				// iOS Syb 1
		"iPhone-iPad",							// iOS Syb 1
		"iPhone-iPad/HD",						// iOS Syb 1
		"iPhone-iPad/HD/Freemium",				// iOS Syb 1
		"Android-MacOSX-iPhone-iPad",			// iOS Syb 1
		"Freemium-BUKAFree/HD",					// iOS Syb 1
		"Part3-Full",							// iOS Syb 1 paid
		"DefaultDistributor-Freemium",			// iOS Syb 1 paid
		"iPhone-iPad/DefaultDistributor",		// iOS Syb 1 paid
		"Android-iPhone-iPad/iPhone-iPad",		// iOS Syb 2
		"PC-MacOSX-Android-iPhone-iPad",		// iOS Syb 2
		"Part2-Full",							// Amerzone
		"Part3-Full",							// Amerzone
		"Full/HD",								// Amerzone
		"Part1-Full/PC-MacOSX/DefaultDistributor", // Amerzone
		"Part2-Full/PC-MacOSX/DefaultDistributor", // Amerzone
		"Part3-Full/PC-MacOSX/DefaultDistributor", // Amerzone
		"Part1-Full/iPhone-iPad-Android", // Amerzone
		"Part2-Full/iPhone-iPad-Android", // Amerzone
		"Part3-Full/iPhone-iPad-Android", // Amerzone
		"Part1-Part2-Part3-Full/HD",			// Amerzone
		"Part1-Part2-Part3-Full",				// Amerzone
		"Part1-Full/HD",						// Amerzone
		"Part2-Full/HD",						// Amerzone
		"Part3-Full/HD",						// Amerzone
	};

	static const Common::Path langs[] = {
		Common::Path(language()),
		"en",
		"de-es-fr-it-en",
		"en-es-fr-de-it",
		"es-en-fr-de-it",
		"de-en-es-fr-it",
		""
	};

	// Note: the audio files for a few videos have a weird path
	// structure where the language is first, followed by some other
	// part names, followed by the file.
	// Dialogs have part stuff followed by lang, so we have to try
	// adding language before *and* after the suffix.

	for (int langtype = 0; langtype < ARRAYSIZE(langs); langtype++) {
		const Common::Path &lang = langs[langtype];
		for (int i = 0; i < ARRAYSIZE(pathSuffixes); i++) {
			Common::Path suffix = pathSuffixes[i];

			Common::Path testPath = dir;
			if (!suffix.empty())
				testPath.joinInPlace(suffix);
			if (!lang.empty())
				testPath.joinInPlace(lang);
			testPath.joinInPlace(fname);
			if (Common::File::exists(testPath))
				return testPath;

			// also try the other way around
			if (!lang.empty() && !suffix.empty()) {
				testPath = dir.join(lang).joinInPlace(suffix).join(fname);
				if (Common::File::exists(testPath))
					return testPath;
			}
		}
	}

	// Didn't find it at all..
	debug("TeCore::findFile Searched but didn't find %s", path.toString(Common::Path::kNativeSeparator).c_str());
	return path;
}

} // end namespace Tetraedge

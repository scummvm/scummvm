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

TeCore::TeCore() : _loc(nullptr), _coreNotReady(true) {
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
	const Common::String filename = path.getLastComponent().toString();
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

void TeCore::warnUnfoundFile(const Common::Path &path) {
	debug("TeCore::findFile Searched but didn't find %s", path.toString().c_str());
}

TetraedgeFSNode TeCore::findFile(const Common::Path &path, bool quiet) const {
	Common::Array<TetraedgeFSNode> dirNodes;
	const Common::Path dir = path.getParent();

	TetraedgeFSNode node;

	const Common::Array<Common::Archive *> &roots = g_engine->getRootArchives();
	for (Common::Array<Common::Archive *>::const_iterator it = roots.begin();
	     it != roots.end(); it++) {
		TetraedgeFSNode archiveNode(*it);
		node = archiveNode.getChild(path);
		if (node.exists())
			return node;
		dirNodes.push_back(archiveNode.getChild(dir));
	}

	Common::String fname = path.getLastComponent().toString();

	// Slight HACK: Remove 'comments' used to specify animated pngs
	if (fname.contains('#'))
		fname = fname.substr(0, fname.find('#'));

	static const char *pathSuffixes[] = {
		nullptr, // no suffix
		"PC-MacOSX",
		"PC-PS3-Android-MacOSX",
		"PC-MacOSX-Android-iPhone-iPad",
		"PC-Android-MacOSX-iPhone-iPad",
		"PC-MacOSX-Xbox360-PS3",
		"PC-MacOSX-PS3-Xbox360",
		"PC-MacOSX-Xbox360-PS3/PC-MacOSX",
		"PC-MacOSX-MacOSXAppStore-Android-iPhone-iPad",
		"PC-MacOSX-MacOSXAppStore-Xbox360-Android-iPad-iPhone",
		"PC-MacOSX-PS3-Xbox360-iPhone-iPad-Android",
		"PC-MacOSX-PS3-Xbox360-Android-iPhone-iPad",
		"PC-PS3-Android-MacOSX-iPhone-iPad-NX",
		"PC-PS3-Android",
		"PC-Android",
		"PS3",
		"Android",
		"Android/HD",
		"Android-iPhone-iPad",
		"Android-iPhone-iPad/Android",
		"Android-iPhone-iPad-PC-MacOSX",
		"Android-MacOSX-iPhone-iPad/padDisabled",
		"padDisabled",
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
		"Freemium-DefaultDistributor-BUKA-BUKAFree/Android",
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

	const Common::Path langs[] = {
		"",
		language(),
		"en",
		"de-es-fr-it-en",
		"en-es-fr-de-it",
		"es-en-fr-de-it",
		"de-en-es-fr-it",
		"de-en-es-fr-ru",
	};

	// Note: the audio files for a few videos have a weird path
	// structure where the language is first, followed by some other
	// part names, followed by the file.
	// Dialogs have part stuff followed by lang, so we have to try
	// adding language before *and* after the suffix.

	for (uint dirNode = 0; dirNode < dirNodes.size(); dirNode++)
		for (int langtype = 0; langtype < ARRAYSIZE(langs); langtype++) {
			const Common::Path &lang = langs[langtype];
			for (int i = 0; i < ARRAYSIZE(pathSuffixes); i++) {
				const char *suffix = pathSuffixes[i];

				Common::Path testPath = "";
				if (suffix)
					testPath.joinInPlace(suffix);
				if (!lang.empty())
					testPath.joinInPlace(lang);
				testPath.joinInPlace(fname);
				node = dirNodes[dirNode].getChild(testPath);
				if (node.exists())
					return node;

				// also try the other way around
				if (!lang.empty() && suffix) {
					testPath = Common::Path(lang).joinInPlace(suffix).join(fname);
					node = dirNodes[dirNode].getChild(testPath);
					if (node.exists()) {
						return node;
					}
				}
			}
		}

	// Didn't find it at all..
	if (!quiet)
		warnUnfoundFile(path);
	return TetraedgeFSNode(nullptr, path);
}

TeLuaFileDesc TeCore::findScript(const Common::Path &path) const {
	TetraedgeFSNode cand(findFile(path, true));
	if (cand.exists())
		return TeLuaFileDesc(cand, false);
	Common::String fname = path.getLastComponent().toString();
	if (fname.hasSuffixIgnoreCase(".lua"))
		fname = fname.substr(0, fname.size() - 4);
	TetraedgeFSNode cand2(findFile(path.getParent().join(fname + ".data"), true));
	if (cand2.exists())
		return TeLuaFileDesc(cand2, true);
	warnUnfoundFile(path);
	return TeLuaFileDesc(cand, false);
}

} // end namespace Tetraedge

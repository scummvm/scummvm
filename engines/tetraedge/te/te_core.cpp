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

#include "tetraedge/te/te_core.h"

#include "tetraedge/te/te_png.h"
#include "tetraedge/te/te_images_sequence.h"
#include "tetraedge/te/te_jpeg.h"
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
	// TODO: Get language from the game definition.  For now just default to en.
	language("en");
	_coreNotReady = false;
	_activityTrackingTimer.alarmSignal().add(this, &TeCore::onActivityTrackingAlarm);
	warning("TODO: TeCore::create: Finish implementing me.");
}

TeICodec *TeCore::createVideoCodec(const Common::FSNode &node) {
	const Common::String filename = node.getName();
	if (!filename.contains('.'))
		return nullptr;
	Common::String extn = filename.substr(filename.findFirstOf('.') + 1);
	extn.toLowercase();
	// The original engine has more formats and even checks for alpha maps,
	// but it never uses them.
	if (TePng::matchExtension(extn)) {
		return new TePng();
	} else if (TeJpeg::matchExtension(extn)) {
		return new TeJpeg();
	} else if (TeTheora::matchExtension(extn)) {
		return new TeTheora();
	} else if (TeTga::matchExtension(extn)) {
		return new TeTga();
	} else if (TeImagesSequence::matchExtension(extn)) {
		return new TeImagesSequence();
	}
	error("TTeCore::createVideoCodec: Unrecognised format %s", node.getName().c_str());
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

static Common::FSNode _findSubPath(const Common::FSNode &parent, const Common::Path &childPath) {
	if (childPath.empty())
		return parent;
	Common::FSNode childNode = parent;
	const Common::StringArray comps = childPath.splitComponents();
	unsigned int i;
	for (i = 0; i < comps.size(); i++) {
		childNode = childNode.getChild(comps[i]);
		if (!childNode.exists())
			break;
	}
	if (i == comps.size())
		return childNode;
	return Common::FSNode();
}


Common::FSNode TeCore::findFile(const Common::Path &path) {
	Common::FSNode node(path);
	if (node.exists())
		return node;

	const Common::FSNode gameRoot(ConfMan.get("path"));
	if (!gameRoot.isDirectory())
		error("Game directory should be a directory");
	const Common::FSNode resNode = gameRoot.getChild("Resources");
	if (!resNode.isDirectory())
		error("Resources directory should exist in game");

	const Common::Path fname = path.getLastComponent();
	const Common::Path dir = path.getParent();

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
		"Android-iPhone-iPad-PC-MacOSX",
		"Full",
		"Part1-Full",
		"Part2-Full-Part1",
		"Part3-Full-Part1",
		"HD",
		"HD/PC-MacOSX-Xbox360-PS3"
	};

	const Common::Path langs[] = {
		"",
		language(),
		"en",
		"de-es-fr-it-en"
	};

	// Note: the audio files for a few videos have a weird path
	// structure where the language is first, followed by some other
	// part names, followed by the file.
	// Dialogs have part stuff followed by lang, so we have to try
	// adding language before *and* after the suffix.

	for (int langtype = 0; langtype < ARRAYSIZE(langs); langtype++) {
		const Common::Path &lang = langs[langtype];
		for (int i = 0; i < ARRAYSIZE(pathSuffixes); i++) {
			const char *suffix = pathSuffixes[i];

			Common::Path testPath = dir;
			if (suffix)
				testPath.joinInPlace(suffix);
			if (!lang.empty())
				testPath.joinInPlace(lang);
			testPath.joinInPlace(fname);
			node = _findSubPath(resNode, testPath);
			if (node.exists())
				return node;

			// also try the other way around
			if (!lang.empty() && suffix) {
				testPath = dir.join(lang).joinInPlace(suffix).join(fname);
				node = _findSubPath(resNode, testPath);
				if (node.exists())
					return node;
			}
		}
	}

	// Didn't find it at all..
	debug("TeCore::findFile Searched but didn't find %s", path.toString().c_str());
	return Common::FSNode(path);
}

} // end namespace Tetraedge

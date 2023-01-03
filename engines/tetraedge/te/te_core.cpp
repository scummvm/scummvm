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

TeICodec *TeCore::createVideoCodec(const Common::Path &path) {
	const Common::String filename = path.getLastComponent().toString();
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
	error("TTeCore::createVideoCodec: Unrecognised format %s", path.toString().c_str());
}

const Common::String &TeCore::fileFlagSystemFlag(const Common::String &name) const {
	return _fileSystemFlags.find(name)->_value;
}

void TeCore::fileFlagSystemSetFlag(const Common::String &name, const Common::String &val) {
	// TODO: Impmenent this fully to check possible values
	_fileSystemFlags.setVal(name, val);
}

bool TeCore::fileFlagSystemFlagsContains(const Common::String &name) const {
	// TODO: Implement me
	return false;
}

Common::Array<Common::String> TeCore::fileFlagSystemPossibleFlags() {
	// TODO: Implement me
	return Common::Array<Common::String>();
}

bool TeCore::fileFlagSystemPossibleFlagsContains(const Common::String &name) const {
	// TODO: Implement me
	return false;
}

const Common::String &TeCore::language() const {
	return fileFlagSystemFlag("language");
}

void TeCore::language(const Common::String &val) {
	return fileFlagSystemSetFlag("language", val);
}

bool TeCore::onActivityTrackingAlarm() {
	error("TODO: Implement me");
}


Common::Path TeCore::findFile(const Common::Path &path) {
	if (Common::File::exists(path))
		return path;

	const Common::String gamePath = ConfMan.get("path");
	const Common::Path resPath = Common::Path(gamePath).join("Resources");
	const Common::Path absolutePath = resPath.join(path);
	if (Common::FSNode(absolutePath).isDirectory())
		return absolutePath;

	const Common::Path fname = path.getLastComponent();
	const Common::Path dir = path.getParent();

	static const Common::Path pathSuffixes[] = {
		"",
		"PC-MacOSX",
		"PC-PS3-Android-MacOSX",
		"PC-MacOSX-Xbox360-PS3",
		"PC-MacOSX-PS3-Xbox360",
		"PC-MacOSX-Xbox360-PS3/PC-MacOSX",
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

	for (int langtype = 0; langtype < ARRAYSIZE(langs); langtype++) {
		for (int i = 0; i < ARRAYSIZE(pathSuffixes); i++) {
			Common::Path testPath = dir.join(pathSuffixes[i]);
			if (!langs[langtype].empty()) {
				testPath.joinInPlace(langs[langtype]);
			}
			testPath.joinInPlace(fname);
			//debug("check for %s", testPath.toString());
			if (Common::File::exists(testPath) || Common::FSNode(testPath).exists())
				return testPath;
		}
	}

	// Didn't find it at all..
	warning("TeCore::findFile Searched but didn't find %s", path.toString().c_str());
	return path;
}

} // end namespace Tetraedge

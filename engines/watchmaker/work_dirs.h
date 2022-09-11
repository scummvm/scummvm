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

#ifndef WATCHMAKER_WORK_DIRS_H
#define WATCHMAKER_WORK_DIRS_H

#include "watchmaker/ll/ll_ffile.h"
#include "watchmaker/globvar.h"

namespace Watchmaker {

class WorkDirs {
	Common::SharedPtr<FastFile> _fastFile;
public:
	Common::String _gameDir = "./";
	Common::String _halfLightmapsDir = "./";
	Common::String _halfMapsDir = "./";
	Common::String _initDir = "./";
	Common::String _define = "./";

	// Setup Directories
	Common::String _t3dDir = "./T3D/";
	Common::String _bndDir = "./BND/";
	Common::String _camDir = "./CAM/";
	Common::String _mapsDir = "./TMaps/";
	Common::String _moviesDir = "./Movies/"; //lo aggiorno in GetRegistrySettings()
	Common::String _speechDir = ""; //lo aggiorno in GetRegistrySettings()
	Common::String _lightmapsDir = "./LMaps/";
	Common::String _miscDir = "./Misc/";
	Common::String _a3dDir = "./A3D/";
	Common::String _midiDir = "./MIDI/";
	Common::String _savesDir = "./Saves/";
	Common::String _wavDir = "./WAV/";

	Common::String join(const Common::String &first, const Common::String &second, Common::String extReplacement = "") {
		Common::String result =  first + second;

		// Hacky, and assumes the extensions are the same length.
		return result.substr(0, result.size() - extReplacement.size()) + extReplacement;;
	}

	Common::String unwindowsify(const Common::String &str) const {
		Common::String result; // TODO Rewrite
		for (int i = 0; i < str.size(); i++) {
			if (str[i] != '\\') {
				result += str[i];
			} else {
				result += "/";
			}
		}
		return result;
	}

	WorkDirs(const Common::String &filename);
	Common::SharedPtr<Common::SeekableReadStream> resolveFile(const Common::String &path, bool noFastFile = false);
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_WORK_DIRS_H

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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/work_dirs.h"
#include "watchmaker/ll/ll_system.h"

namespace Watchmaker {

Common::SharedPtr<Common::SeekableReadStream> WorkDirs::resolveFile(const Common::String &path, bool noFastFile) {
	Common::String converted = path;
	for (int i = 0; i < path.size(); i++) {
		if (converted[i] == '\\') {
			converted.setChar('/', i);
		}
	}
	Common::SharedPtr<Common::SeekableReadStream> stream = nullptr;
	if (!noFastFile) {
		stream = _fastFile->resolve(converted.c_str());
		if (stream) {
			return stream;
		}
	}
	return Common::SharedPtr<Common::SeekableReadStream>(openFile(converted));
}

static bool readKeyValuePair(Common::SeekableReadStream &stream, Common::String &key, Common::String &value) {
#define GETEQUALWORD(d,s) while((s[i]!='=') &&(s[i]!='\n') &&(s[i]!='\0')) d[j++]=s[i++]; d[j]=0; j=0; i++;
#define GETENDWORD(d,s)  memset(d,0,200); while((s[i]!='\n') && (s[i]>30)&&(s[i]!='\0')) d[j++]=s[i++];

	// TODO: Rewrite
	char str[200], dest[200];
	memset(str, 0, 200);
	memset(dest, 0, 200);
	stream.readLine(str, 200);
	int i = 0, j = 0;
	while ((str[i] != '=') && (str[i] != '\n') && (str[i] != '\0')) dest[j++] = str[i++];
	dest[j] = 0;
	j = 0;
	i++;
	key = dest;
	memset(dest, 0, 200);
	while ((str[i] != '\n') && (str[i] > 30) && (str[i] != '\0')) dest[j++] = str[i++];
	value = dest;
	return !stream.eos();
}

WorkDirs::WorkDirs(const Common::String &filename) {
	_fastFile = Common::SharedPtr<FastFile>(new FastFile(WmGameDataPak_FilePath));
	auto fcfg = openFile(filename.c_str());
	if (fcfg) {

		Common::String key, value;
		while (readKeyValuePair(*fcfg, key, value)) {
			if (key == "WmGame") {
				_gameDir = unwindowsify(value);
			} else if (key == "Wmt3d") {
				_t3dDir = unwindowsify(value);
			} else if (key == "WmBnd") {
				_bndDir = unwindowsify(value);
			} else if (key == "Wmcam") {
				_camDir = unwindowsify(value);
			} else if (key == "WmMaps") {
				_mapsDir = unwindowsify(value);
			} else if (key == "WmMovies") {
				_moviesDir = unwindowsify(value);
			} else if (key == "WmLightmaps") {
				_lightmapsDir = unwindowsify(value);
			} else if (key == "WmMisc") {
				_miscDir = unwindowsify(value);
			} else if (key == "WmHalfLightmaps") {
				_halfLightmapsDir = unwindowsify(value);
			} else if (key == "WmHalfmaps") {
				_halfMapsDir = unwindowsify(value);
			} else if (key == "WmA3d") {
				_a3dDir = unwindowsify(value);
			} else if (key == "WmInit") {
				_initDir = unwindowsify(value);
			} else if (key == "WmWav") {
				_wavDir = unwindowsify(value);
			} else if (key == "WmSpeech") {
				_speechDir = unwindowsify(value);
			} else if (key == "WmMidi") {
				_midiDir = unwindowsify(value);
			} else if (key == "WMSaves") {
				_savesDir = unwindowsify(value);
			} else if (key == "WmDefine") {
				_define = unwindowsify(value);
			}
		}
	}
}

} // End of namespace Watchmaker

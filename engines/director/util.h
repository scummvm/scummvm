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

#ifndef DIRECTOR_UTIL_H
#define DIRECTOR_UTIL_H

namespace Common {
class String;
}

namespace Director {

int castNumToNum(const char *str);
char *numToCastNum(int num);

bool isAbsolutePath(Common::String &path);

Common::String convertPath(Common::String &path);

Common::String unixToMacPath(const Common::String &path);

Common::String getPath(Common::String path, Common::String cwd);

bool testPath(Common::String &path, bool directory = false);

Common::String pathMakeRelative(Common::String path, bool recursive = true, bool addexts = true, bool directory = false);

Common::String wrappedPathMakeRelative(Common::String path, bool recursive = true, bool addexts = true, bool directory = false, bool absolute = false);

bool hasExtension(Common::String filename);

Common::String testExtensions(Common::String component, Common::String initialPath, Common::String convPath);

Common::String getFileName(Common::String path);

Common::String stripMacPath(const char *name);

Common::String convertMacFilename(const char *name);

Common::String dumpScriptName(const char *prefix, int type, int id, const char *ext);
Common::String dumpFactoryName(const char *prefix, const char *name, const char *ext);

bool isButtonSprite(SpriteType spriteType);

class RandomState {
public:
	uint32 _seed;
	uint32 _mask;
	uint32 _len;

	RandomState() {
		_seed = _mask = _len = 0;
	}

	void setSeed(int seed);
	uint32 getSeed() { return _seed; }
	int32 getRandom(int32 range);

private:
	void init(int len);
	int32 genNextRandom();
	int32 perlin(int32 val);
};

uint32 readVarInt(Common::SeekableReadStream &stream);

Common::SeekableReadStreamEndian *readZlibData(Common::SeekableReadStream &stream, unsigned long len, unsigned long *outLen, bool bigEndian);

uint16 humanVersion(uint16 ver);

Common::Platform platformFromID(uint16 id);

Common::CodePage getEncoding(Common::Platform platform, Common::Language language);
Common::CodePage detectFontEncoding(Common::Platform platform, uint16 fontId);

int charToNum(Common::u32char_type_t ch);
Common::u32char_type_t numToChar(int num);
int compareStrings(const Common::String &s1, const Common::String &s2);

// Our implementation of strstr() with Director character order
const char *d_strstr(const char *str, const char *substr);

Common::String encodePathForDump(const Common::String &path);

Common::String utf8ToPrintable(const Common::String &str);

Common::String decodePlatformEncoding(Common::String input);

Common::String formatStringForDump(const Common::String &str);

inline byte lerpByte(byte a, byte b, int alpha, int span) {
	int ai = static_cast<int>(a);
	int bi = static_cast<int>(b);
	span = CLIP<int>(span, 1, span);
	alpha = CLIP<int>(alpha, 0, span);
	return static_cast<byte>((bi * alpha + ai * (span - alpha)) / span);
}

inline void lerpPalette(byte *target, byte *palA, int palALength, byte *palB, int palBLength, int alpha, int span) {
	for (int i = 0; i < 768; i++) {
		target[i] = lerpByte(
			i < palALength * 3 ? palA[i] : 0,
			i < palBLength * 3 ? palB[i] : 0,
			alpha,
			span
		);
	}
}

} // End of namespace Director

#endif

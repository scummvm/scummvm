/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "gui/theme.h"
#include "common/file.h"
#include "common/archive.h"
#include "common/unzip.h"

namespace GUI {

Theme::Theme() : _loadedThemeX(0), _loadedThemeY(0) {}

Theme::~Theme() {}

const Graphics::Font *Theme::loadFontFromArchive(const Common::String &filename) {
	Common::Archive *arch = 0;
	const Graphics::NewFont *font = 0;

	if (getThemeFileName().hasSuffix(".zip")) {
#ifdef USE_ZLIB
		Common::ZipArchive *zip = new Common::ZipArchive(getThemeFileName());
		if (!zip || !zip->isOpen())
			return 0;

		arch = zip;
#else
		return 0;
#endif
	} else {
		Common::FSDirectory *dir = new Common::FSDirectory(getThemeFileName());
		if (!dir || !dir->getFSNode().isDirectory())	
			return 0;

		arch = dir;
	}

	Common::SeekableReadStream *stream(arch->openFile(filename));
	if (stream) {
		font = Graphics::NewFont::loadFromCache(*stream);
		delete stream;
	}

	delete arch;
	return font;
}

const Graphics::Font *Theme::loadFont(const Common::String &filename) {
	const Graphics::Font *font = 0;
	Common::String cacheFilename = genCacheFilename(filename.c_str());
	Common::File fontFile;

	if (!cacheFilename.empty()) {
		if (fontFile.open(cacheFilename))
			font = Graphics::NewFont::loadFromCache(fontFile);

		if (font)
			return font;

		if ((font = loadFontFromArchive(cacheFilename)))
			return font;
	}

	// normal open
	if (fontFile.open(filename)) {
		font = Graphics::NewFont::loadFont(fontFile);
	}

	if (!font) {
		font = loadFontFromArchive(filename);
	}

	if (font) {
		if (!cacheFilename.empty()) {
			if (!Graphics::NewFont::cacheFontData(*(const Graphics::NewFont*)font, cacheFilename)) {
				warning("Couldn't create cache file for font '%s'", filename.c_str());
			}
		}
	}

	return font;
}

Common::String Theme::genCacheFilename(const char *filename) {
	Common::String cacheName(filename);
	for (int i = cacheName.size() - 1; i >= 0; --i) {
		if (cacheName[i] == '.') {
			while ((uint)i < cacheName.size() - 1) {
				cacheName.deleteLastChar();
			}

			cacheName += "fcc";
			return cacheName;
		}
	}

	return "";
}

bool Theme::isThemeLoadingRequired() {
	int x = g_system->getOverlayWidth(), y = g_system->getOverlayHeight();

	if (_loadedThemeX == x && _loadedThemeY == y)
		return false;

	_loadedThemeX = x;
	_loadedThemeY = y;

	return true;
}

bool Theme::themeConfigParseHeader(Common::String header, Common::String &themeName) {	
	header.trim();
	
	if (header[0] != '[' || header.lastChar() != ']')
		return false;
		
	header.deleteChar(0);
	header.deleteLastChar();
	
	Common::StringTokenizer tok(header, ":");
	
	if (tok.nextToken() != SCUMMVM_THEME_VERSION_STR)
		return false;
		
	themeName = tok.nextToken();
	Common::String author = tok.nextToken();

	return tok.empty();
}

bool Theme::themeConfigUseable(const Common::FSNode &node, Common::String &themeName) {
	Common::File stream;
	bool foundHeader = false;
		
	if (node.getName().hasSuffix(".zip")) {
#ifdef USE_ZLIB
		Common::ZipArchive zipArchive(node);
		if (zipArchive.hasFile("THEMERC")) {
			stream.open("THEMERC", zipArchive);
		}
#endif
	} else if (node.isDirectory()) {			
		Common::FSNode headerfile = node.getChild("THEMERC");
		if (!headerfile.exists() || !headerfile.isReadable() || headerfile.isDirectory())
			return false;
		stream.open(headerfile);
	}
	
	if (stream.isOpen()) {
		Common::String stxHeader = stream.readLine();
		foundHeader = themeConfigParseHeader(stxHeader, themeName);
	}

	return foundHeader;
}

} // End of namespace GUI


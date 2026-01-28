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

#include "scumm/he/font_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/fs.h"

#include "graphics/surface.h"

#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif

namespace Scumm {

HEFont::HEFont(ScummEngine_v99he *vm) : _vm(vm), _fontsEnumerated(false) {
	_fontContextList.size();
}

HEFont::~HEFont() {
	for (Common::List<HEFontContextElement *>::iterator it = _fontContextList.begin();
		it != _fontContextList.end(); ++it) {
		delete *it;
	}

	_fontContextList.clear();
	_fontEntries.clear();
}

HEFontContextElement *HEFont::findFontContext(int imageNum) {
	for (Common::List<HEFontContextElement *>::iterator it = _fontContextList.begin();
		it != _fontContextList.end(); ++it) {

		if ((*it)->imageNumber == imageNum) {
			return *it;
		}
	}

	return nullptr;
}

bool HEFont::startFont(int imageNum) {
	// Check if a context already exists for this image...
	HEFontContextElement *existing = findFontContext(imageNum);
	if (existing) {
		debug(1, "HEFont::startFont(): Warning - font system exists for image %d. Destroying existing.", imageNum);
		endFont(imageNum);
	}

	// Verify the image is suitable...
	WizSimpleBitmap bitmap;
	if (!_vm->_wiz->dwSetSimpleBitmapStructFromImage(imageNum, 0, &bitmap)) {
		debug(1, "HEFont::startFont(): Image %d is not suitable for rendering (must not be compressed)", imageNum);
		return false;
	}

	// Create new font context...
	HEFontContextElement *newContext = new HEFontContextElement();
	newContext->imageNumber = imageNum;

	_fontContextList.push_back(newContext);

	return true;
}

bool HEFont::endFont(int imageNum) {
	for (Common::List<HEFontContextElement *>::iterator it = _fontContextList.begin();
		it != _fontContextList.end(); ++it) {

		if ((*it)->imageNumber == imageNum) {
			delete *it;
			_fontContextList.erase(it);
			return true;
		}
	}

	debug(1, "HEFont::endFont(): Font system not started for image %d", imageNum);
	return false;
}

bool HEFont::createFont(int imageNum, const char *fontName, int fgColor, int bgColor, int style, int size) {
	HEFontContextElement *ctx = findFontContext(imageNum);
	if (!ctx) {
		debug(1, "HEFont::createFont(): Font system not created for image %d", imageNum);
		return false;
	}

	// Destroy any previously created font...
	if (ctx->font) {
		delete ctx->font;
		ctx->font = nullptr;
	}

#ifdef USE_FREETYPE2
	// Find the filename for this font name...
	Common::String fontFileName;
	for (uint i = 0; i < _fontEntries.size(); i++) {
		if (_fontEntries[i].fontName.equalsIgnoreCase(fontName)) {
			fontFileName = _fontEntries[i].fileName + ".ttf";
			break;
		}
	}

	// Fallback: try using fontName directly as filename...
	if (fontFileName.empty()) {
		fontFileName = Common::String(fontName) + ".ttf";
	}

	// Try loading from fonts.dat...
	ctx->font = Graphics::loadTTFFontFromArchive(fontFileName,
												 size,
												 Graphics::kTTFSizeModeCell,
												 0, 0,
												 Graphics::kTTFRenderModeLight,
												 nullptr);

	// If not in archive, we're probably searching for a game-specific font, so try via SearchMan...
	if (!ctx->font) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(Common::Path(fontFileName));

		if (stream) {
			ctx->font = Graphics::loadTTFFont(stream,
											  DisposeAfterUse::YES,
											  size,
											  Graphics::kTTFSizeModeCell,
											  0, 0,
											  Graphics::kTTFRenderModeLight,
											  nullptr);
		}
	}
#endif

	if (!ctx->font) {
		debug(1, "HEFont::createFont(): Could not create font '%s'", fontName);
		return false;
	}

	// Store colors
	ctx->fgColor = fgColor;
	ctx->bgColor = bgColor;
	ctx->transparentBg = (bgColor == _vm->VAR(_vm->VAR_WIZ_TRANSPARENT_COLOR));

	// Set alignment
	ctx->align = Graphics::kTextAlignCenter; // Default
	if (style & kHEFontStyleLeft) {
		ctx->align = Graphics::kTextAlignLeft;
	} else if (style & kHEFontStyleRight) {
		ctx->align = Graphics::kTextAlignRight;
	}

	return true;
}

bool HEFont::renderString(int imageNum, int imageState, int xPos, int yPos, const char *string) {
	HEFontContextElement *ctx = findFontContext(imageNum);
	if (!ctx) {
		debug(1, "HEFont::renderString(): Font system not created for image %d", imageNum);
		return false;
	}

	if (!ctx->font) {
		debug(1, "HEFont::renderString(): No font selected for image %d", imageNum);
		return false;
	}

	// Get the bitmap from the current image...
	WizSimpleBitmap bitmap;
	if (!_vm->_wiz->dwSetSimpleBitmapStructFromImage(imageNum, imageState, &bitmap)) {
		debug(1, "HEFont::renderString(): Could not get bitmap from image %d", imageNum);
		return false;
	}

	// Wrap it in a Surface, which we can assume is a 8-bit paletted one for our use case...
	Graphics::Surface surface;
	surface.init(bitmap.bitmapWidth, bitmap.bitmapHeight,
				 bitmap.bitmapWidth,
				 bitmap.bufferPtr(),
				 Graphics::PixelFormat::createFormatCLUT8());

	int stringWidth = ctx->font->getStringWidth(string);
	int stringHeight = ctx->font->getFontHeight();

	// If background is not transparent, fill the background rectangle first...
	if (!ctx->transparentBg) {
		Common::Rect bgRect(xPos, yPos, xPos + stringWidth, yPos + stringHeight);
		surface.fillRect(bgRect, ctx->bgColor);
	}

	// Draw the string!
	ctx->font->drawString(&surface, string, xPos, yPos, stringWidth, ctx->fgColor, ctx->align);

	return true;
}

int HEFont::getStringWidth(int imageNum, const char *string) {
	HEFontContextElement *ctx = findFontContext(imageNum);
	if (!ctx) {
		debug(1, "HEFont::getStringWidth(): Font system not created for image %d", imageNum);
		return 0;
	}

	if (!ctx->font) {
		debug(1, "HEFont::getStringWidth(): No font selected for image %d", imageNum);
		return 0;
	}

	return ctx->font->getStringWidth(string);
}

int HEFont::getStringHeight(int imageNum, const char *string) {
	HEFontContextElement *ctx = findFontContext(imageNum);
	if (!ctx) {
		debug(1, "HEFont::getStringHeight(): Font system not created for image %d", imageNum);
		return 0;
	}

	if (!ctx->font) {
		debug(1, "HEFont::getStringHeight(): No font selected for image %d", imageNum);
		return 0;
	}

	return ctx->font->getFontHeight();
}

void HEFont::enumerateFonts() {
	if (_fontsEnumerated) {
		return;
	}

	_fontEntries.clear();

#ifdef USE_FREETYPE2
	Common::Array<Common::String> candidateFonts;

	// Open fonts.dat...
	Common::SeekableReadStream *archiveStream = nullptr;

	if (ConfMan.hasKey("extrapath")) {
		Common::FSDirectory extrapath(ConfMan.getPath("extrapath"));
		archiveStream = extrapath.createReadStreamForMember("fonts.dat");
	}

	if (!archiveStream) {
		archiveStream = SearchMan.createReadStreamForMember("fonts.dat");
	}

	if (archiveStream) {
		Common::Archive *archive = Common::makeZipArchive(archiveStream);
		if (archive) {
			Common::ArchiveMemberList members;
			archive->listMembers(members);

			for (Common::ArchiveMemberList::const_iterator it = members.begin();
				 it != members.end(); ++it) {
				Common::String name = (*it)->getName();
				if (name.hasSuffixIgnoreCase(".ttf")) {
					Common::String fontName = name.substr(0, name.size() - 4);
					candidateFonts.push_back(fontName);
				}
			}

			delete archive;
		}
	}

	// If not in archive, we're probably searching for a game-specific font, so try via SearchMan...
	Common::ArchiveMemberList fontFiles;
	SearchMan.listMatchingMembers(fontFiles, "*.ttf");

	// Use filenames for fonts.dat fonts...
	for (uint i = 0; i < candidateFonts.size(); i++) {
		Common::String fontFileName = candidateFonts[i] + ".ttf";

		Graphics::Font *testFont = Graphics::loadTTFFontFromArchive(
			fontFileName,
			12,
			Graphics::kTTFSizeModeCharacter
		);

		if (testFont) {
			HEFontEntry entry;
			entry.fileName = candidateFonts[i];
			entry.fontName = candidateFonts[i];
			_fontEntries.push_back(entry);
			delete testFont;
		}
	}

	// Use actual font name from metadata for game fonts...
	for (Common::ArchiveMemberList::const_iterator it = fontFiles.begin();
		 it != fontFiles.end(); ++it) {
		Common::String name = (*it)->getName();
		Common::String fileBaseName = name.substr(0, name.size() - 4);

		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(Common::Path(name));
		if (stream) {
			Graphics::Font *testFont = Graphics::loadTTFFont(stream,
															 DisposeAfterUse::YES,
															 12,
															 Graphics::kTTFSizeModeCharacter
			);

			if (testFont) {
				HEFontEntry entry;
				entry.fileName = fileBaseName;
				entry.fontName = testFont->getFontName();

				// If there's no metadata name, just fallback to the filename...
				if (entry.fontName.empty()) {
					entry.fontName = fileBaseName;
				}

				_fontEntries.push_back(entry);
				delete testFont;
			}
		}
	}
#endif

	// Sort alphabetically...
	Common::sort(_fontEntries.begin(), _fontEntries.end(),
				 [](const HEFontEntry &a, const HEFontEntry &b) {
					 return a.fontName.compareToIgnoreCase(b.fontName) < 0;
				 }
	);

	_fontsEnumerated = true;
}

int HEFont::enumInit() {
	enumerateFonts();
	return _fontEntries.size();
}

void HEFont::enumDestroy() {
	_fontEntries.clear();
	_fontsEnumerated = false;
}

const char *HEFont::enumGet(int index) {
	if (!_fontsEnumerated) {
		debug(1, "HEFont::enumGet(): Font enumeration not initialized");
		return nullptr;
	}

	if (index < 0 || index >= (int)_fontEntries.size()) {
		debug(1, "HEFont::enumGet(): Index %d out of range (0-%d)", index, (int)_fontEntries.size() - 1);
		return nullptr;
	}

	return _fontEntries[index].fontName.c_str();
}

int HEFont::enumFind(const char *fontName) {
	if (!_fontsEnumerated) {
		debug(1, "HEFont::enumFind(): Font enumeration not initialized");
		return -1;
	}

	for (uint i = 0; i < _fontEntries.size(); i++) {
		if (_fontEntries[i].fontName.equalsIgnoreCase(fontName)) {
			return i;
		}
	}

	debug(1, "HEFont::enumFind(): Could not find font '%s'", fontName);
	return -1;
}

} // End of namespace Scumm

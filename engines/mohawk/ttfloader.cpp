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

#if defined(WIN32)
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#endif

#include "mohawk/resource.h"

#include "common/compression/installshield_cab.h"
#include "common/compression/installshieldv3_archive.h"
#include "common/fs.h"
#include "common/platform.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "gui/message.h"

#include "mohawk/ttfloader.h"

namespace Mohawk {

TTFLoader::TTFLoader(const Common::String &fileName, const Common::String &faceName,
					 bool bold, int32 faceIndex, Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, Common::String(), bold, faceIndex, sizeMode) {
}

TTFLoader::TTFLoader(const Common::String &fileName, const Common::String &faceName,
					 const Common::String &srcInst, bool bold, int32 faceIndex,
					 Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, srcInst, bold, faceIndex,
				Graphics::kTTFRenderModeMonochrome, sizeMode) {
}

TTFLoader::TTFLoader(const Common::String &fileName, const Common::String &faceName,
					 const Common::String &srcInst, bool bold, int32 faceIndex,
					 Graphics::TTFRenderMode renderMode, Graphics::TTFSizeMode sizeMode)
	: _filePath(Common::Path(fileName)), _faceName(faceName), _bold(bold),
	  _faceIndex(faceIndex), _srcInst(srcInst), _renderMode(renderMode),
	  _sizeMode(sizeMode) {
}

Common::String TTFLoader::getCacheName(int point) const {
	switch (_renderMode) {
	case Graphics::kTTFRenderModeLight:
		if (_sizeMode == Graphics::kTTFSizeModeCell)
			return Common::String::format("%s@%d-cell-light", _faceName.c_str(), point);
		return Common::String::format("%s@%d-char-light", _faceName.c_str(), point);
	case Graphics::kTTFRenderModeMonochrome:
	default:
		if (_sizeMode == Graphics::kTTFSizeModeCell)
			return Common::String::format("%s@%d-cell-mono", _faceName.c_str(), point);
		return Common::String::format("%s@%d-char-mono", _faceName.c_str(), point);
	}
}

const Graphics::Font *TTFLoader::loadFont(int point) {
	const Graphics::Font *font = getCachedFont(point);
	if (font)
		return font;

	font = loadFontCore(point);
	if (font)
		cacheFont(point, font);

	return font;
}

bool TTFLoader::usesAntialiasing() const {
	return _renderMode != Graphics::kTTFRenderModeMonochrome;
}

bool TTFLoader::isFontNameEqual(TTFLoader *other) {
	if (!other)
		return false;

	return _filePath.equalsIgnoreCase(other->_filePath);
}

const Graphics::Font *TTFLoader::getCachedFont(int point) const {
	return FontMan.getFontByName(getCacheName(point));
}

void TTFLoader::cacheFont(int point, const Graphics::Font *font) {
	FontMan.assignFontToName(getCacheName(point), font);
}

FileTTFLoader::FileTTFLoader(const Common::String &fileName, const Common::String &faceName,
							 bool bold, int32 faceIndex, Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, Common::String(), bold, faceIndex, sizeMode) {
}

FileTTFLoader::FileTTFLoader(const Common::String &fileName, const Common::String &faceName,
							 const Common::String &srcInst, bool bold, int32 faceIndex,
							 Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, srcInst, bold, faceIndex, sizeMode) {
}

bool FileTTFLoader::hasStream() {
	return Common::File::exists(_filePath);
}

Common::SeekableReadStream *FileTTFLoader::getStream() {
	Common::File *f = new Common::File();
	if (!f->open(_filePath)) {
		delete f;
		return nullptr;
	}
	return f;
}

const Graphics::Font *FileTTFLoader::loadFontCore(int point) {
	const Graphics::Font *font = nullptr;
#ifdef USE_FREETYPE2
#if 0
	// FIXME: Calling findTTFFace with gulim.ttc causes segfault
	Common::Array<Common::Path> files;
	files.push_back(Common::Path(fontFile));
	font = Graphics::findTTFace(files, fontFace, true, false, pointSize, 0, 0, renderMode);
#else
	Common::SeekableReadStream *stream = getStream();
	if (stream) {
		font = Graphics::loadTTFFont(stream, DisposeAfterUse::YES, point,
									 _sizeMode, 0, 0, _renderMode, 0, false,
									 _faceIndex, _bold, false);
	}
#endif
#endif
	return font;
}

ISCabTTFLoader::ISCabTTFLoader(const Common::String &iscabFile,
							   const Common::String &fileName, const Common::String &faceName,
							   bool bold, int32 faceIndex, Graphics::TTFSizeMode sizeMode)
	: ISCabTTFLoader(iscabFile, fileName, faceName, Common::String(), bold, faceIndex, sizeMode) {
}

ISCabTTFLoader::ISCabTTFLoader(const Common::String &iscabFile,
							   const Common::String &fileName, const Common::String &faceName,
							   const Common::String &srcInst, bool bold, int32 faceIndex,
							   Graphics::TTFSizeMode sizeMode)
	: FileTTFLoader(fileName, faceName, srcInst, bold, faceIndex, sizeMode),
	  _iscabPath(Common::Path(iscabFile)),
	  _inPath(Common::Path(fileName, Common::Path::kNoSeparator)) {
}

ISCabTTFLoader::~ISCabTTFLoader() {
	closeArchive();
}

bool ISCabTTFLoader::hasStream() {
	// Check InstallShield cab file (Ex: data1.cab)
	if (!openArchive())
		return false;
	return _iscabArchive->hasFile(_inPath);
}

Common::SeekableReadStream *ISCabTTFLoader::getStream() {
	// Check InstallShield cab file (Ex: data1.cab)
	if (!openArchive())
		return nullptr;
	if (_iscabArchive->hasFile(_inPath))
		return _iscabArchive->createReadStreamForMember(_inPath);
	return nullptr;
}

bool ISCabTTFLoader::openArchive() {
	if (!_iscabArchive)
		_iscabArchive = Common::makeInstallShieldArchive(_iscabPath);
	return _iscabArchive != nullptr;
}

void ISCabTTFLoader::closeArchive() {
	if (_iscabArchive) {
		delete _iscabArchive;
		_iscabArchive = nullptr;
	}
}

ISZTTFLoader::ISZTTFLoader(const Common::String &iszFile,
						   const Common::String &fileName, const Common::String &faceName,
						   bool bold, int32 faceIndex, Graphics::TTFSizeMode sizeMode)
	: ISZTTFLoader(iszFile, fileName, faceName, Common::String(), bold, faceIndex, sizeMode) {
}

ISZTTFLoader::ISZTTFLoader(const Common::String &iszFile,
						   const Common::String &fileName, const Common::String &faceName,
						   const Common::String &srcInst, bool bold, int32 faceIndex,
						   Graphics::TTFSizeMode sizeMode)
	: FileTTFLoader(fileName, faceName, srcInst, bold, faceIndex, sizeMode),
	  _iszPath(Common::Path(iszFile)),
	  _inPath(Common::Path(fileName, Common::Path::kNoSeparator)) {
}

ISZTTFLoader::~ISZTTFLoader() {
	closeArchive();
}

bool ISZTTFLoader::hasStream() {
	// Check InstallShield Z file (Ex: ZBARC32.Z, ZBARC16.Z)
	if (!openArchive())
		return false;
	return _iszArchive->hasFile(_inPath);
}

Common::SeekableReadStream *ISZTTFLoader::getStream() {
	// Check InstallShield cab file (Ex: ZBARC32.Z, ZBARC16.Z)
	if (!openArchive())
		return nullptr;
	if (_iszArchive->hasFile(_inPath))
		return _iszArchive->createReadStreamForMember(_inPath);
	return nullptr;
}

bool ISZTTFLoader::openArchive() {
	if (_iszArchive) {
		return true;
	} else {
		Common::InstallShieldV3 *iszArchive = new Common::InstallShieldV3();
		_iszArchive = iszArchive;
		return iszArchive->open(_iszPath);
	}
}

void ISZTTFLoader::closeArchive() {
	if (_iszArchive) {
		delete _iszArchive;
		_iszArchive = nullptr;
	}
}

WinSysTTFLoader::WinSysTTFLoader(const Common::String &fileName,
								 const Common::String &faceName, bool bold, int32 faceIndex,
								 Graphics::TTFSizeMode sizeMode)
	: FileTTFLoader(fileName, faceName, bold, faceIndex, sizeMode) {
}

WinSysTTFLoader::WinSysTTFLoader(const Common::String &fileName,
								 const Common::String &faceName, const Common::String &srcInst,
								 bool bold, int32 faceIndex, Graphics::TTFSizeMode sizeMode)
	: FileTTFLoader(fileName, faceName, srcInst, bold, faceIndex, sizeMode) {
}

bool WinSysTTFLoader::hasStream() {
	// Check Windows system font archive (Ex: C:\Windows\Fonts\gulim.ttc)
	Common::FSNode node;
	return getFileNode(node);
}

Common::SeekableReadStream *WinSysTTFLoader::getStream() {
	// Check Windows system font archive (Ex: C:\Windows\Fonts\gulim.ttc)
	Common::FSNode node;
	if (!getFileNode(node))
		return nullptr;

	Common::SeekableReadStream *stream = node.createReadStream();
	if (stream && !_hasShownSystemFontWarning && !_srcInst.empty()) {
		_hasShownSystemFontWarning = true;
		const Common::U32String warningMessage = _srcInst.decode(Common::kUtf8);
		warning("%s", warningMessage.encode(Common::kUtf8).c_str());
		GUI::MessageDialog dialog(warningMessage);
		dialog.runModal();
	}

	return stream;
}

bool WinSysTTFLoader::getFileNode(Common::FSNode &node) {
	// FIXME: Is this kind of host system access is allowed in ScummVM?
	//        If not, disable this code and explicitly make sure that users have to source the font themselves.
#if defined(WIN32)
	const char *winDir = getenv("WINDIR");
	if (!winDir)
		return false;
	Common::FSNode winDirNode = Common::FSNode(winDir);
	if (!winDirNode.exists() || !winDirNode.isDirectory())
		return false;
	Common::FSNode fontDirNode = winDirNode.getChild("Fonts");
	if (!fontDirNode.exists() || !fontDirNode.isDirectory())
		return false;
	Common::FSNode fontFileNode = fontDirNode.getChild(_filePath.baseName());
	if (!fontFileNode.exists() || fontFileNode.isDirectory())
		return false;
	node = fontFileNode;
	return true;
#else
	return false;
#endif
}

ArchiveTTFLoader::ArchiveTTFLoader(const Common::String &fileName,
								   const Common::String &faceName, bool bold, int32 faceIndex,
								   Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, Common::String(), bold, faceIndex, sizeMode) {
}

ArchiveTTFLoader::ArchiveTTFLoader(const Common::String &fileName,
								   const Common::String &faceName,
								   Graphics::TTFRenderMode renderMode,
								   Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, Common::String(), false, 0, renderMode, sizeMode) {
}

ArchiveTTFLoader::ArchiveTTFLoader(const Common::String &fileName,
								   const Common::String &faceName,
								   const Common::String &srcInst, bool bold, int32 faceIndex,
								   Graphics::TTFSizeMode sizeMode)
	: TTFLoader(fileName, faceName, srcInst, bold, faceIndex, sizeMode) {
}

const Graphics::Font *ArchiveTTFLoader::loadFontCore(int point) {
	const Graphics::Font *font = nullptr;
#ifdef USE_FREETYPE2
	font = Graphics::loadTTFFontFromArchive(_filePath.baseName(), point,
											_sizeMode, 0, 0, _renderMode);
#endif
	return font;
}

} // End of namespace Mohawk

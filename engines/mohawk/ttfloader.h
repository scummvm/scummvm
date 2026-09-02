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

#ifndef MOHAWK_TTFLOADER_H
#define MOHAWK_TTFLOADER_H

#include "common/fs.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "graphics/fonts/ttf.h"

namespace Common {

class SeekableReadStream;
class Archive;
class InstallShieldV3;

} // End of namespace Common

namespace Mohawk {

/**
 * The font loader which supports multiple stream backend.
 */
class TTFLoader {
public:
	Common::Path _filePath;
	Common::String _faceName;
	bool _bold = false;
	int32 _faceIndex = 0;
	Common::String _srcInst;

	TTFLoader() {}
	TTFLoader(const Common::String &fileName, const Common::String &faceName,
			  bool bold = false, int32 faceIndex = 0,
			  Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	TTFLoader(const Common::String &fileName, const Common::String &faceName,
			  const Common::String &srcInst, bool bold = false, int32 faceIndex = 0,
			  Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	virtual ~TTFLoader() {}

	Common::String getCacheName(int point) const;
	const Graphics::Font *loadFont(int point);
	/** Return whether this loader preserves grayscale glyph coverage. */
	bool usesAntialiasing() const;
	bool isFontNameEqual(TTFLoader *other);

protected:
	TTFLoader(const Common::String &fileName, const Common::String &faceName,
			  const Common::String &srcInst, bool bold, int32 faceIndex,
			  Graphics::TTFRenderMode renderMode, Graphics::TTFSizeMode sizeMode);
	virtual const Graphics::Font *loadFontCore(int point) = 0;

	const Graphics::Font *getCachedFont(int point) const;
	void cacheFont(int point, const Graphics::Font *font);

	/** Coverage mode used to load and cache this font. */
	Graphics::TTFRenderMode _renderMode = Graphics::kTTFRenderModeMonochrome;
	/** Interpretation of the requested font size. */
	Graphics::TTFSizeMode _sizeMode = Graphics::kTTFSizeModeCharacter;
};

/**
 * Load font from local TTF/TTC file.
 */
class FileTTFLoader : public TTFLoader {
public:
	FileTTFLoader() {}
	FileTTFLoader(const Common::String &fileName, const Common::String &faceName,
				  bool bold = false, int32 faceIndex = 0,
				  Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	FileTTFLoader(const Common::String &fileName, const Common::String &faceName,
				  const Common::String &srcInst, bool bold = false, int32 faceIndex = 0,
				  Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	virtual ~FileTTFLoader() {}

	virtual bool hasStream();

protected:
	const Graphics::Font *loadFontCore(int point) override;
	virtual Common::SeekableReadStream *getStream();
};

/**
 * Load TTF font from InstallShield cab file.
 */
class ISCabTTFLoader : public FileTTFLoader {
public:
	Common::Path _iscabPath;
	Common::Archive *_iscabArchive = nullptr;

	ISCabTTFLoader() {}
	ISCabTTFLoader(const Common::String &iscabFile, const Common::String &fileName,
				   const Common::String &faceName, bool bold = false, int32 faceIndex = 0,
				   Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	ISCabTTFLoader(const Common::String &iscabFile, const Common::String &fileName,
				   const Common::String &faceName, const Common::String &srcInst,
				   bool bold = false, int32 faceIndex = 0,
				   Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	~ISCabTTFLoader() override;

	bool hasStream() override;

protected:
	Common::SeekableReadStream *getStream() override;

	Common::Path _inPath;
	bool openArchive();
	void closeArchive();
};

/**
 * Load TTF font from InstallShield Z file.
 */
class ISZTTFLoader : public FileTTFLoader {
public:
	Common::Path _iszPath;
	Common::InstallShieldV3 *_iszArchive = nullptr;

	ISZTTFLoader() {}
	ISZTTFLoader(const Common::String &iszFile, const Common::String &fileName,
				 const Common::String &faceName, bool bold = false, int32 faceIndex = 0,
				 Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	ISZTTFLoader(const Common::String &iszFile, const Common::String &fileName,
				 const Common::String &faceName, const Common::String &srcInst,
				 bool bold = false, int32 faceIndex = 0,
				 Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	~ISZTTFLoader() override;

	bool hasStream() override;

protected:
	Common::SeekableReadStream *getStream() override;

	Common::Path _inPath;
	bool openArchive();
	void closeArchive();
};

/**
 * Load TTF/TTC font from Windows system font archive.
 * - To discourage use of system fonts, show a warning if this is used.
 */
class WinSysTTFLoader : public FileTTFLoader {
public:
	WinSysTTFLoader() {}
	WinSysTTFLoader(const Common::String &fileName, const Common::String &faceName,
					bool bold = false, int32 faceIndex = 0,
					Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	WinSysTTFLoader(const Common::String &fileName, const Common::String &faceName,
					const Common::String &srcInst, bool bold = false, int32 faceIndex = 0,
					Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	~WinSysTTFLoader() override {}

	bool hasStream() override;
	Common::SeekableReadStream *getStream() override;

protected:
	bool getFileNode(Common::FSNode &node);
	bool _hasShownSystemFontWarning = false;
};

/**
 * Fallback loader which loads font from the ScummVM internal font archive
 */
class ArchiveTTFLoader : public TTFLoader {
public:
	ArchiveTTFLoader() {}
	ArchiveTTFLoader(const Common::String &fileName, const Common::String &faceName,
					 bool bold = false, int32 faceIndex = 0,
					 Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	ArchiveTTFLoader(const Common::String &fileName, const Common::String &faceName,
					 Graphics::TTFRenderMode renderMode,
					 Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	ArchiveTTFLoader(const Common::String &fileName, const Common::String &faceName,
					 const Common::String &srcInst, bool bold = false, int32 faceIndex = 0,
					 Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	virtual ~ArchiveTTFLoader() {}

	const Graphics::Font *loadFontCore(int point) override;
};

} // End of namespace Mohawk

#endif

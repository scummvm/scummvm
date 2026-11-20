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
 * Base interface for loading TrueType/OpenType fonts from different sources.
 */
class TTFLoader {
public:
	/** Path of the font source. */
	Common::Path _filePath;
	/** Face name used to identify cached font instances. */
	Common::String _faceName;
	/** Fake bold when the bold face is not available. */
	bool _fakeBold = false;
	/** Fake italic when the italic face is not available. */
	bool _fakeItalic = false;
	/**
	 * The index of face to load when loading a TTC file.
	 * Otherwise, always set to 0.
	 */
	int32 _faceIndex = 0;
	/**
	 * Font source instructions to display when a font stream is not available.
	 * Leave it empty when this feature is not wanted.
	 */
	Common::String _srcInst;

	TTFLoader() {}
	/**
	 * Configure a font source, face selection and rendering options.
	 *
	 * @param fileName    Path of the font source.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	TTFLoader(const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a font source, face selection and rendering options.
	 *
	 * @param fileName    Path of the font source.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	TTFLoader(const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	virtual ~TTFLoader() {}

	/** Build a cache key of the font. */
	Common::String getCacheName(int point) const;
	/** Create a font instance. */
	const Graphics::Font *loadFont(int point);
	/** Return whether anti-aliasing is enabled by the configured rendering mode. */
	bool isAntialiasingEnabled() const;
	/** Select the rendering mode. */
	void setRenderMode(Graphics::TTFRenderMode renderMode);

protected:
	virtual const Graphics::Font *loadFontCore(int point) = 0;

	const Graphics::Font *getCachedFont(int point) const;
	void cacheFont(int point, const Graphics::Font *font);

	/** Glyph anti-aliasing and hinting mode. */
	Graphics::TTFRenderMode _renderMode = Graphics::kTTFRenderModeMonochrome;
	/** Controls interpretation of the requested point size. */
	Graphics::TTFSizeMode _sizeMode = Graphics::kTTFSizeModeCharacter;
};

/**
 * Load font from local TTF/TTC file.
 */
class FileTTFLoader : public TTFLoader {
public:
	FileTTFLoader() {}
	/**
	 * Configure a local font file for loading on demand.
	 *
	 * @param fileName    Path of the local font file.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	FileTTFLoader(const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a local font file for loading on demand.
	 *
	 * @param fileName    Path of the local font file.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	FileTTFLoader(const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	virtual ~FileTTFLoader() {}

	virtual bool hasStream();

protected:
	const Graphics::Font *loadFontCore(int point) override;
	virtual Common::SeekableReadStream *getStream();
};

/**
 * Load an 'sfnt' font resource from a Macintosh application's resource fork.
 * 
 * @remarks
 * The inherited @ref TTFLoader::_filePath identifies the application.
 */
class MacResForkTTFLoader : public FileTTFLoader {
public:
	/**
	 * Configure an 'sfnt' font resource from a Macintosh application.
	 *
	 * @param appPath     Path of the Macintosh application containing the font resource.
	 * @param resourceId  ID of the 'sfnt' resource in the application's resource fork.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 */
	MacResForkTTFLoader(const Common::String &appPath, uint16 resourceId, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter);
	bool hasStream() override;

protected:
	Common::SeekableReadStream *getStream() override;

private:
	uint16 _resourceId;
};

/**
 * Load TTF font from InstallShield cab file.
 */
class ISCabTTFLoader : public FileTTFLoader {
public:
	Common::Path _iscabPath;
	Common::Archive *_iscabArchive = nullptr;

	ISCabTTFLoader() {}
	/**
	 * Configure a font member in an InstallShield cabinet.
	 *
	 * @param iscabFile   Path of the InstallShield cabinet containing the font.
	 * @param fileName    Path of the font file within the InstallShield cabinet.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ISCabTTFLoader(const Common::String &iscabFile, const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a font member in an InstallShield cabinet.
	 *
	 * @param iscabFile   Path of the InstallShield cabinet containing the font.
	 * @param fileName    Path of the font file within the InstallShield cabinet.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ISCabTTFLoader(const Common::String &iscabFile, const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
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
	/**
	 * Configure a font member in an InstallShield Z archive.
	 *
	 * @param iszFile     Path of the InstallShield Z archive containing the font.
	 * @param fileName    Path of the font file within the InstallShield Z archive.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ISZTTFLoader(const Common::String &iszFile, const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a font member in an InstallShield Z archive.
	 *
	 * @param iszFile     Path of the InstallShield Z archive containing the font.
	 * @param fileName    Path of the font file within the InstallShield Z archive.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ISZTTFLoader(const Common::String &iszFile, const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
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
 * The inherited @ref TTFLoader::_srcInst is shown once as a warning when the font stream is opened.
 */
class WinSysTTFLoader : public FileTTFLoader {
public:
	WinSysTTFLoader() {}
	/**
	 * Configure a font from the Windows system font directory.
	 *
	 * @param fileName    Name of the font file in the Windows system font directory.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	WinSysTTFLoader(const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a font from the Windows system font directory.
	 *
	 * @param fileName    Name of the font file in the Windows system font directory.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	WinSysTTFLoader(const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
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
	/**
	 * Configure a fallback font from the ScummVM font archive.
	 *
	 * @param fileName    Name of the font file in the ScummVM font archive.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ArchiveTTFLoader(const Common::String &fileName, const Common::String &faceName, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	/**
	 * Configure a fallback font from the ScummVM font archive.
	 *
	 * @param fileName    Name of the font file in the ScummVM font archive.
	 * @param faceName    Font face name used to identify cached font instances.
	 * @param srcInst     Font source instructions to display when a font stream is not available.
	 * @param sizeMode    Selects the point size definition.
	 * @param renderMode  Glyph anti-aliasing and hinting mode;
	 *                    monochrome rendering disables anti-aliasing.
	 * @param faceIndex   The index of face to load when loading a TTC file.
	 *                    Otherwise, always set to 0.
	 * @param fakeBold    Fake bold when the bold face is not available.
	 * @param fakeItalic  Fake italic when the italic face is not available.
	 */
	ArchiveTTFLoader(const Common::String &fileName, const Common::String &faceName, const Common::String &srcInst, Graphics::TTFSizeMode sizeMode = Graphics::kTTFSizeModeCharacter, Graphics::TTFRenderMode renderMode = Graphics::kTTFRenderModeMonochrome, int32 faceIndex = 0, bool fakeBold = false, bool fakeItalic = false);
	virtual ~ArchiveTTFLoader() {}

	const Graphics::Font *loadFontCore(int point) override;
};

} // End of namespace Mohawk

#endif

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

#ifndef GRIM_FONT_H
#define GRIM_FONT_H

#include "engines/grim/pool.h"

#include "graphics/font.h"
#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class SaveGame;

class Font {
public:
	virtual ~Font() {}

	virtual int32 getKernedHeight() const = 0;
	virtual int32 getFontWidth() const = 0;
	virtual int getKernedStringLength(const Common::String &text) const = 0;
	virtual int32 getBaseOffsetY() const = 0;
	virtual void render(Graphics::Surface &buf, const Common::String &currentLine, const Graphics::PixelFormat &pixelFormat, uint32 blackColor, uint32 color, uint32 colorKey) const = 0;
	virtual int32 getCharKernedWidth(unsigned char c) const = 0;
	virtual int getPoolId() const = 0;
	virtual int32 getPoolTag() const = 0;
	virtual bool is8Bit() const = 0;
	const Common::String &getFilename() const { return _filename; }

	static Font *getByFileName(const Common::String& fileName);
	static Font *getFirstFont();
	static void save(const Font *font, SaveGame *state);
	static Font *load(SaveGame *state);

protected:
	Common::String _filename;
};

class BitmapFont : public Font, public PoolObject<BitmapFont> {
public:
	BitmapFont();
	~BitmapFont();

	static int32 getStaticTag() { return MKTAG('F', 'O', 'N', 'T'); }
	int getPoolId() const override { return getId(); }
	int32 getPoolTag() const override { return getStaticTag(); }

	void load(const Common::String &filename, Common::SeekableReadStream *data);


	const Common::String &getFilename() const { return _filename; }
	int32 getKernedHeight() const override { return _kernedHeight; }
	int32 getFontWidth() const override { return getCharKernedWidth('w'); }
	int32 getBaseOffsetY() const override { return _baseOffsetY; }
	void render(Graphics::Surface &buf, const Common::String &currentLine, const Graphics::PixelFormat &pixelFormat, uint32 blackColor, uint32 color, uint32 colorKey) const override;
	virtual int32 getCharBitmapWidth(unsigned char c) const { return _charHeaders[getCharIndex(c)].bitmapWidth; }
	virtual int32 getCharBitmapHeight(unsigned char c) const { return _charHeaders[getCharIndex(c)].bitmapHeight; }
	int32 getCharKernedWidth(unsigned char c) const override { return _charHeaders[getCharIndex(c)].kernedWidth; }
	virtual int32 getCharStartingCol(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingCol; }
	virtual int32 getCharStartingLine(unsigned char c) const { return _charHeaders[getCharIndex(c)].startingLine; }
	virtual int32 getCharOffset(unsigned char c) const { return _charHeaders[getCharIndex(c)].offset; }
	const byte *getCharData(unsigned char c) const { return _fontData + (_charHeaders[getCharIndex(c)].offset); }

	const byte *getFontData() const { return _fontData; }
	uint32 getDataSize() const { return _dataSize; }
	bool is8Bit() const override { return true; }

	virtual int getKernedStringLength(const Common::String &text) const;
	virtual int getBitmapStringLength(const Common::String &text) const;
	virtual int getStringHeight(const Common::String &text) const;

	const void *getUserData() const { return _userData; }
	void setUserData(void *data) { _userData = data; }

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

	static const uint8 emerFont[][13];
private:

	uint16 getCharIndex(unsigned char c) const;
	struct CharHeader {
		int32 offset;
		int8  kernedWidth;
		int8  startingCol;
		int8  startingLine;
		int32 bitmapWidth;
		int32 bitmapHeight;
	};

	uint32 _numChars;
	uint32 _dataSize;
	uint32 _kernedHeight, _baseOffsetY;
	uint32 _firstChar, _lastChar;
	uint16 *_charIndex;
	CharHeader *_charHeaders;
	byte *_fontData;
	void *_userData;
};

class FontTTF : public Font, public PoolObject<FontTTF> {
public:
	void loadTTF(const Common::String &filename, Common::SeekableReadStream *data, int size);

	static int32 getStaticTag() { return MKTAG('T', 'T', 'F', ' '); }
	int getPoolId() const override { return getId(); }
	int32 getPoolTag() const override { return getStaticTag(); }

	int32 getKernedHeight() const override { return _font->getFontHeight(); }
	int32 getBaseOffsetY() const override { return 0; }
	int32 getCharKernedWidth(unsigned char c) const override { return _font->getCharWidth(c); }
	int32 getFontWidth() const override { return getCharKernedWidth('w'); }

	int getKernedStringLength(const Common::String &text) const override { return _font->getStringWidth(text); }
	void render(Graphics::Surface &buf, const Common::String &currentLine, const Graphics::PixelFormat &pixelFormat, uint32 blackColor, uint32 color, uint32 colorKey) const override;
	bool is8Bit() const override { return false; }

	void saveState(SaveGame *state) const;
	void restoreState(SaveGame *state);

private:
	Graphics::Font *_font;
	int _size;
};

} // end of namespace Grim

#endif

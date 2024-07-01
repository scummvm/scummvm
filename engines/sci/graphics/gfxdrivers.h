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

#ifndef SCI_GRAPHICS_GFXDRIVERS_H
#define SCI_GRAPHICS_GFXDRIVERS_H

#include "common/rect.h"

namespace Sci {

class GfxDriver {
public:
	GfxDriver(uint16 screenWidth, uint16 screenHeight, int numColors, int horizontalAlignment) : _screenW(screenWidth), _screenH(screenHeight), _numColors(numColors), _hAlign(horizontalAlignment) {}
	virtual ~GfxDriver() {}

	uint16 screenWidth() const { return _screenW; }
	uint16 screenHeight() const { return _screenH; }
	uint16 numColors() const { return _numColors; }
	byte hAlignment() const { return _hAlign; }
	
	virtual bool allowRGBRendering() const = 0;
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	virtual void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) = 0;
	virtual void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) = 0;
	virtual Common::Point getMousePos() const;
	virtual void clearRect(const Common::Rect &r) const;

protected:
	const uint16 _screenW;
	const uint16 _screenH;
	uint16 _numColors;
	const byte _hAlign;
};

class GfxDefaultDriver final : public GfxDriver {
public:
	GfxDefaultDriver(uint16 screenWidth, uint16 screenHeight);
	~GfxDefaultDriver() override {}
	bool allowRGBRendering() const override { return true; }
	void setPalette(const byte *colors, uint start, uint num) override;
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
};

class SCI0_DOSPreVGADriver : public GfxDriver {
public:
	SCI0_DOSPreVGADriver(int numColors, int screenW, int screenH, int horizontalAlignment);
	~SCI0_DOSPreVGADriver() override;
	bool allowRGBRendering() const override { return false; }
	void setPalette(const byte*, uint, uint) override;
protected:
	static bool checkDriver(const char *const *driverNames, int listSize);
	void assignPalette(const byte *colors);
	byte *_compositeBuffer;
private:
	bool _palNeedUpdate;
	const byte *_colors;
};

class SCI0_CGADriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_CGADriver(bool emulateCGAModeOnEGACard);
	~SCI0_CGADriver() override;
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	static bool validateMode() { return checkDriver(&_driverFile, 1); }
private:
	uint16 *_cgaPatterns;
	byte _palette[12];
	const bool _disableMode5;
	static const char *_driverFile;
};

class SCI0_CGABWDriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_CGABWDriver(uint32 monochromeColor);
	~SCI0_CGABWDriver() override;
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getMousePos() const override;
	void clearRect(const Common::Rect &r) const override;
	static bool validateMode() { return checkDriver(_driverFiles, 2); }
private:
	byte _monochromePalette[6];
	const byte *_monochromePatterns;
	bool _earlyVersion;
	static const char *_driverFiles[2];
};

class SCI0_HerculesDriver final : public SCI0_DOSPreVGADriver {
public:
	SCI0_HerculesDriver(uint32 monochromeColor, bool cropImage);
	~SCI0_HerculesDriver() override;
	void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h) override;
	void replaceCursor(const void *cursor, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor) override;
	Common::Point getMousePos() const override;
	void clearRect(const Common::Rect &r) const override;
	static bool validateMode() { return checkDriver(&_driverFile, 1); }
private:
	const uint16 _centerX;
	const uint16 _centerY;
	byte _monochromePalette[6];
	const byte *_monochromePatterns;
	static const char *_driverFile;
};

} // End of namespace Sci

#endif // SCI_GRAPHICS_GFXDRIVERS_H

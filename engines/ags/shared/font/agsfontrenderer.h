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
 */

#ifndef AGS_SHARED_FONT_AGSFONTRENDERER_H
#define AGS_SHARED_FONT_AGSFONTRENDERER_H

struct BITMAP;

// WARNING: this interface is exposed for plugins and declared for the second time in agsplugin.h
class IAGSFontRenderer {
public:
	virtual bool LoadFromDisk(int fontNumber, int fontSize) = 0;
	virtual void FreeMemory(int fontNumber) = 0;
	virtual bool SupportsExtendedCharacters(int fontNumber) = 0;
	virtual int GetTextWidth(const char *text, int fontNumber) = 0;
	// Get actual height of the given line of text
	virtual int GetTextHeight(const char *text, int fontNumber) = 0;
	virtual void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) = 0;
	virtual void AdjustYCoordinateForFont(int *ycoord, int fontNumber) = 0;
	virtual void EnsureTextValidForFont(char *text, int fontNumber) = 0;
protected:
	IAGSFontRenderer() = default;
	~IAGSFontRenderer() = default;
};

// Font render params, mainly for dealing with various compatibility issues and
// broken fonts. NOTE: currently left empty as a result of rewrite, but may be
// used again in the future.
struct FontRenderParams {
	// Font's render multiplier
	int SizeMultiplier = 1;
};

// NOTE: this extending interface is not yet exposed to plugins
class IAGSFontRenderer2 {
public:
	virtual bool IsBitmapFont() = 0;
	// Load font, applying extended font rendering parameters
	virtual bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) = 0;
protected:
	IAGSFontRenderer2() = default;
	~IAGSFontRenderer2() = default;
};

#endif

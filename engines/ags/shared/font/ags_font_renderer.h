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

#ifndef AGS_SHARED_FONT_AGS_FONT_RENDERER_H
#define AGS_SHARED_FONT_AGS_FONT_RENDERER_H

namespace AGS3 {

class BITMAP;

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
	IAGSFontRenderer() {}
	~IAGSFontRenderer() {}
};

// Font render params, mainly for dealing with various compatibility issues.
struct FontRenderParams {
	// Font's render multiplier
	int SizeMultiplier = 1;
	int LoadMode = 0; // contains font flags from FFLG_LOADMODEMASK
};

// Describes loaded font's properties
struct FontMetrics {
	int Height = 0; // formal font height value
	int RealHeight = 0; // real graphical height of a font
	int CompatHeight = 0; // either formal or real height, depending on compat settings
};

// NOTE: this extending interface is not yet exposed to plugins
class IAGSFontRenderer2 {
public:
	// Tells if this is a bitmap font (otherwise it's a vector font)
	virtual bool IsBitmapFont() = 0;
	// Load font, applying extended font rendering parameters
	virtual bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params,
		FontMetrics *metrics) = 0;
	// Gets font's name; must return an empty string if no name is available
	virtual const char *GetName(int fontNumber) = 0;
	// Perform any necessary adjustments when the AA mode is toggled
	virtual void AdjustFontForAntiAlias(int fontNumber, bool aa_mode) = 0;
protected:
	IAGSFontRenderer2() {}
	~IAGSFontRenderer2() {}
};

} // namespace AGS3

#endif

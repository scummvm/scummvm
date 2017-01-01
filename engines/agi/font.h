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

#ifndef AGI_FONT_H
#define AGI_FONT_H

namespace Agi {

class GfxFont {
public:
	GfxFont(AgiBase *vm);
	~GfxFont();

private:
	AgiBase *_vm;

public:
	void init();
	const byte *getFontData();
	bool isFontHires();

private:
	void overwriteSaveRestoreDialogCharacter();
	void overwriteExtendedWithRussianSet();

	void loadFontScummVMFile(Common::String fontFilename);
	void loadFontMickey();
	void loadFontAmigaPseudoTopaz();
	void loadFontAppleIIgs();
	void loadFontAtariST(Common::String fontFilename);
	void loadFontHercules();

	const uint8 *_fontData; // pointer to the currently used font
	uint8 *_fontDataAllocated;
	bool _fontIsHires;
};

} // End of namespace Agi

#endif /* AGI_FONT_H */

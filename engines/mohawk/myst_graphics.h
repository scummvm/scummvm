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

#ifndef MOHAWK_MYST_GRAPHICS_H
#define MOHAWK_MYST_GRAPHICS_H

#include "mohawk/graphics.h"

#include "common/file.h"
#include "graphics/font.h"

namespace Mohawk {

class MystBitmap;
class MohawkEngine_Myst;

enum RectState {
	kRectEnabled,
	kRectDisabled,
	kRectUnreachable
};

class MystGraphics : public GraphicsManager {
public:
	explicit MystGraphics(MohawkEngine_Myst *vm);
	~MystGraphics() override;

	void copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest);
	void copyImageToScreen(uint16 image, Common::Rect dest);
	void copyImageToBackBuffer(uint16 image, Common::Rect dest);
	void copyBackBufferToScreen(Common::Rect r);
	void runTransition(TransitionType type, Common::Rect rect, uint16 steps, uint16 delay);
	void drawRect(Common::Rect rect, RectState state);
	void drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color);
	void fadeToBlack();
	void fadeFromBlack();
	void clearScreen();

	void clearScreenPalette();
	void setPaletteToScreen();
	const byte *getPalette() const { return _palette; }

	void saveStateForMainMenu();
	void restoreStateForMainMenu();
	Graphics::Surface *getThumbnailForMainMenu() const;

	void loadMenuFont();
	Common::Rect getTextBoundingBox(const Common::U32String &text, const Common::Rect &dest, Graphics::TextAlign align);
	void drawText(uint16 image, const Common::U32String &text, const Common::Rect &dest, uint8 r, uint8 g, uint8 b, Graphics::TextAlign align, int16 deltaY);

	void replaceImageWithRect(uint16 destImage, uint16 sourceImage, const Common::Rect &sourceRect);

protected:
	MohawkSurface *decodeImage(uint16 id) override;
	MohawkEngine *getVM() override { return (MohawkEngine *)_vm; }

private:
	MohawkEngine_Myst *_vm;
	MystBitmap *_bmpDecoder;

	Graphics::Surface *_backBuffer;
	Graphics::PixelFormat _pixelFormat;
	Common::Rect _viewport;
	byte _palette[256 * 3];

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _mainMenuBackupScreen;
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _mainMenuBackupScreenThumbnail;
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> _mainMenuBackupBackBuffer;

	void transitionDissolve(Common::Rect rect, uint step);
	void transitionSlideToLeft(Common::Rect rect, uint16 steps, uint16 delay);
	void transitionSlideToRight(Common::Rect rect, uint16 steps, uint16 delay);
	void transitionSlideToTop(Common::Rect rect, uint16 steps, uint16 delay);
	void transitionSlideToBottom(Common::Rect rect, uint16 steps, uint16 delay);
	void transitionPartialToRight(Common::Rect rect, uint32 width, uint32 steps);
	void transitionPartialToLeft(Common::Rect rect, uint32 width, uint32 steps);

	void remapSurfaceToSystemPalette(MohawkSurface *mhkSurface);
	byte getColorIndex(const byte *palette, byte red, byte green, byte blue);

	void applyImagePatches(uint16 id, const MohawkSurface *mhkSurface) const;

	Graphics::Font *_menuFont;

	const Graphics::Font *getMenuFont() const;
};

} // End of namespace Mohawk

#endif

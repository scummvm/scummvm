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

#ifndef MOHAWK_RIVEN_GRAPHICS_H
#define MOHAWK_RIVEN_GRAPHICS_H

#include "mohawk/graphics.h"

namespace Mohawk {

class MohawkEngine_Riven;

class RivenGraphics : public GraphicsManager {
public:
	RivenGraphics(MohawkEngine_Riven *vm);
	~RivenGraphics();

	// Screen updates
	void beginScreenUpdate();
	void applyScreenUpdate(bool force = false);

	void copyImageToScreen(uint16 image, uint32 left, uint32 top, uint32 right, uint32 bottom);
	void updateScreen(Common::Rect updateRect = Common::Rect(0, 0, 608, 392));
	void drawRect(Common::Rect rect, bool active);
	void drawImageRect(uint16 id, Common::Rect srcRect, Common::Rect dstRect);
	void drawExtrasImage(uint16 id, Common::Rect dstRect);

	// Water Effect
	void scheduleWaterEffect(uint16);
	void clearWaterEffects();
	bool runScheduledWaterEffects();

	// Transitions
	void scheduleTransition(uint16 id, Common::Rect rect = Common::Rect(0, 0, 608, 392));
	void runScheduledTransition();
	void fadeToBlack();
	void setTransitionSpeed(uint32 speed) { _transitionSpeed = speed; }

	// Inventory
	void showInventory();
	void hideInventory();

	// Credits
	void beginCredits();
	void updateCredits();
	uint getCurCreditsImage() { return _creditsImage; }

protected:
	MohawkSurface *decodeImage(uint16 id);
	MohawkEngine *getVM() { return (MohawkEngine *)_vm; }

private:
	MohawkEngine_Riven *_vm;
	MohawkBitmap *_bitmapDecoder;
	int _screenUpdateNesting;
	bool _screenUpdateRunning;

	// Water Effects
	struct SFXERecord {
		// Record values
		uint16 frameCount;
		Common::Rect rect;
		uint16 speed;
		Common::Array<Common::SeekableReadStream *> frameScripts;

		// Cur frame
		uint16 curFrame;
		uint32 lastFrameTime;
	};
	Common::Array<SFXERecord> _waterEffects;

	// Transitions
	int16 _scheduledTransition;
	Common::Rect _transitionRect;
	uint32 _transitionSpeed;

	// Inventory
	void clearInventoryArea();
	void drawInventoryImage(uint16 id, const Common::Rect *rect);
	bool _inventoryDrawn;

	// Screen Related
	Graphics::Surface *_mainScreen;
	bool _dirtyScreen;
	Graphics::PixelFormat _pixelFormat;
	void clearMainScreen();

	// Credits
	uint _creditsImage, _creditsPos;
};

} // End of namespace Mohawk

#endif

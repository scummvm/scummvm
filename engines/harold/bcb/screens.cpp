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

#include "common/system.h"
#include "graphics/font.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

namespace BCB {

static const int kGameOverTextLineGap = 2;

bool BCBEngine::showInitialLogoScreen() {
	Graphics::Surface *logo = loadBitmap("16mlogo.bmp");
	if (!logo)
		return false;

	blitScreenWithLineReveal(*logo);

	logo->free();
	delete logo;

	_currentDisplayState = 10;
	_titleButtonsActive = false;
	_systemButtonsVisible = false;
	_initialLogoElapsedTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	return true;
}

bool BCBEngine::showTitleScreen() {
	stopGameOverMusic();
	if (_titleBitmap && _titleBitmap->format != g_system->getScreenFormat()) {
		_titleBitmap->free();
		delete _titleBitmap;
		_titleBitmap = nullptr;
	}

	if (!_titleBitmap)
		_titleBitmap = loadBitmap("16mtitle.bmp");
	if (!_titleBitmap)
		return false;

	blitScreenWithLineReveal(*_titleBitmap);

	_currentDisplayState = 0;
	_titleButtonsActive = true;
	_systemButtonsVisible = false;
	_titleLoadSlotsActive = false;
	return true;
}

void BCBEngine::restoreTitleScreenRegion(const Common::Rect &rect, bool present) {
	if (!_titleBitmap)
		return;

	const Common::Rect titleBounds(0, 0, _titleBitmap->w, _titleBitmap->h);
	Common::Rect clippedRect = rect;
	clippedRect.clip(titleBounds);
	if (clippedRect.isEmpty())
		return;

	g_system->copyRectToScreen(_titleBitmap->getBasePtr(clippedRect.left, clippedRect.top), _titleBitmap->pitch,
							   clippedRect.left, clippedRect.top, clippedRect.width(), clippedRect.height());
	if (present)
		g_system->updateScreen();
}

bool BCBEngine::transitionFromLogoToTitle() {
	if (!showTitleScreen())
		return false;

	startMenuBackgroundMusic();
	return true;
}

void BCBEngine::drawTitleButton(uint buttonIndex, bool pressed) {
	if (!_titleBitmap || buttonIndex == 0 || buttonIndex >= ARRAYSIZE(kTitleButtonTable))
		return;

	const ButtonControlRecord &button = kTitleButtonTable[buttonIndex];
	const int sourceX = pressed ? 0xa2 : button.x;
	const int sourceY = pressed ? (int)((buttonIndex + 0x16) * 0x20) : button.y;

	if (sourceX + button.width > _titleBitmap->w || sourceY + button.height > _titleBitmap->h)
		return;

	g_system->copyRectToScreen(_titleBitmap->getBasePtr(sourceX, sourceY), _titleBitmap->pitch, button.x, button.y, button.width, button.height);
	g_system->updateScreen();
}

void BCBEngine::blitScreenWithLineReveal(const Graphics::Surface &surface, bool presentFinalFrame) {
	const int width = MIN<int>(surface.w, 640);
	const int height = MIN<int>(surface.h, 480);
	const int revealSteps = (height + 1) / 2;

	const uint32 revealStartMillis = g_system->getMillis();
	int revealedSteps = 0;
	while (revealedSteps < revealSteps && !shouldQuit()) {
		uint32 elapsedMillis = g_system->getMillis() - revealStartMillis;
		int targetSteps = MIN<int>(elapsedMillis / 2 + 1, revealSteps);
		if (targetSteps <= revealedSteps) {
			pollEvents(false);
			serviceCursorRefresh();
			g_system->delayMillis(1);
			continue;
		}

		for (; revealedSteps < targetSteps; ++revealedSteps) {
			const int topY = revealedSteps * 2;
			const int bottomY = height - 1 - revealedSteps * 2;
			if (topY < height)
				g_system->copyRectToScreen(surface.getBasePtr(0, topY), surface.pitch, 0, topY, width, 1);
			if (bottomY != topY && bottomY >= 0)
				g_system->copyRectToScreen(surface.getBasePtr(0, bottomY), surface.pitch, 0, bottomY, width, 1);
		}

		if (presentFinalFrame || revealedSteps < revealSteps)
			g_system->updateScreen();
		pollEvents(false);
	}
}

} // End of namespace BCB

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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Screen texts */

void PrisonerEngine::setupScreenText(ScreenText *screenText, TextResource *textResource, int16 chunkIndex) {

	Common::StringArray *chunk = textResource->getText(screenText->textIndex)->chunks[chunkIndex];

	screenText->lineCount = chunk->size();
	screenText->width = 0;

	for (int16 i = 0; i < screenText->lineCount; i++) {
		int16 width = getTextWidth((*chunk)[i]);
		if (width > screenText->width)
			screenText->width = width;
	}

	if (screenText->width > 619)
		screenText->width = 619;

	screenText->height = screenText->fontHeight * screenText->lineCount + getActiveFontUnk2();

	if (!screenText->screenText) {
		if (screenText->actorIndex != -1) {
			screenText->x = screenText->x0 - screenText->width / 2 - _cameraX;
			screenText->y = screenText->y0 + _actors[screenText->actorIndex].actorSprite->boundsY1 - screenText->height - _cameraY;
		} else if (!screenText->screenCenter) {
			screenText->x = screenText->x0 - screenText->width / 2;
			screenText->y = screenText->y0 - screenText->height / 2;
		} else {
			screenText->x = (640 - screenText->width) / 2 - 6;
			screenText->y = (316 - screenText->height) / 2 + 72;
		}
		if (screenText->x + screenText->width > 619)
			screenText->x = 639 - screenText->width - 20;
		if (screenText->x < 20)
			screenText->x = 20;
		if (screenText->y < screenText->fontHeight + 82)
			screenText->y = screenText->fontHeight + 82;
		if (screenText->y + screenText->height > 398)
			screenText->y = 398 - screenText->height;
	} else {
		screenText->x = screenText->x0;
		screenText->y = screenText->y0;
	}

}

void PrisonerEngine::addScreenText(ScreenText *screenText, int16 resourceCacheSlot, Common::String &identifier) {

	TextResource *textResource = _res->get<TextResource>(resourceCacheSlot);
	const TextItem *textItem;

	screenText->used = 1;
	screenText->inventoryActive = _inventoryActive;
	screenText->identifier = identifier;
	screenText->resourceCacheSlot = resourceCacheSlot;
	screenText->textIndex = textResource->getIndex(identifier);
	textItem = textResource->getText(screenText->textIndex);
	screenText->chunkCount = textItem->chunks.size();
	screenText->chunkIndex = 0;
	screenText->finishedTime = 0;

	if (!screenText->screenText) {
		if (textItem->hasSpeech) {
			screenText->speechPakSlot = textItem->pakSlot;
			_screenTextHasSpeech = true;
		} else {
			screenText->speechPakSlot = -1;
			_screenTextHasSpeech = false;
		}
		_screenTextShowing = true;
		if (screenText->actorIndex != -1) {
			Actor *actor = &_actors[screenText->actorIndex];
			screenText->fontIndex = actor->textFontNumber;
			screenText->fontOutlineColor = actor->fontOutlineColor;
			screenText->fontInkColor = actor->fontInkColor;
		} else {
			screenText->fontIndex = _textFont;
			if (!_inventoryActive) {
				screenText->fontOutlineColor = _screenTextFontColor.outlineColor;
				screenText->fontInkColor = _screenTextFontColor.inkColor;
			} else {
				screenText->fontOutlineColor = _inventoryScreenTextFontColor.outlineColor;
				screenText->fontInkColor = _inventoryScreenTextFontColor.inkColor;
			}
		}
	} else {
		screenText->speechPakSlot = -1;
	}

	setActiveFont(screenText->fontIndex);
	screenText->fontHeight = getActiveFontHeight();

	setupScreenText(screenText, textResource, 0);

}

void PrisonerEngine::unloadScreenText(int16 screenTextIndex) {
	ScreenText *screenText = &_screenTexts[screenTextIndex];
	screenText->used = 0;
	_res->unload(screenText->resourceCacheSlot);
	if (!screenText->screenText) {
		_screenTextShowing = false;
		stopTalkieSpeech();
		stopLipSync();
	} else {
		unloadFont(screenText->fontIndex);
	}
}

void PrisonerEngine::unloadScreenTexts() {
	for (int16 screenTextIndex = 0; screenTextIndex < kMaxScreenTexts; screenTextIndex++) {
		ScreenText *screenText = &_screenTexts[screenTextIndex];
		if (screenText->used == 1) {
			unloadScreenText(screenTextIndex);
		}
	}
	_screenTextActive = false;
}

int16 PrisonerEngine::addActorScreenText(int16 actorIndex, int16 resourceCacheSlot, Common::String &identifier) {
	int16 screenTextIndex = _screenTexts.getFreeSlot();
	ScreenText *screenText = &_screenTexts[screenTextIndex];
	screenText->actorIndex = actorIndex;
	screenText->x0 = _actors[actorIndex].actorSprite->x;
	screenText->y0 = _actors[actorIndex].actorSprite->y;
	screenText->screenCenter = false;
	screenText->screenText = false;
	addScreenText(screenText, resourceCacheSlot, identifier);
	return screenTextIndex;
}

int16 PrisonerEngine::addLooseScreenText(int16 x, int16 y, int16 resourceCacheSlot, Common::String &identifier) {
	int16 screenTextIndex = _screenTexts.getFreeSlot();
	ScreenText *screenText = &_screenTexts[screenTextIndex];
	screenText->x0 = x;
	screenText->y0 = y;
	screenText->actorIndex = -1;
	screenText->screenCenter = false;
	screenText->screenText = false;
	addScreenText(screenText, resourceCacheSlot, identifier);
	return screenTextIndex;
}

int16 PrisonerEngine::addCenteredScreenText(int16 resourceCacheSlot, Common::String &identifier) {
	int16 screenTextIndex = _screenTexts.getFreeSlot();
	ScreenText *screenText = &_screenTexts[screenTextIndex];
	screenText->actorIndex = -1;
	screenText->screenCenter = true;
	screenText->screenText = false;
	addScreenText(screenText, resourceCacheSlot, identifier);
	return screenTextIndex;
}

void PrisonerEngine::updateScreenTexts() {

	for (int16 screenTextIndex = 0; screenTextIndex < kMaxScreenTexts; screenTextIndex++) {
		ScreenText *screenText = &_screenTexts[screenTextIndex];
		if (screenText->used == 0 || screenText->inventoryActive != _inventoryActive)
			continue;

		updateTalkieSpeech(screenText->speechPakSlot, screenText->finishedTime, screenText->resourceCacheSlot);

		setFontColors(screenText->fontIndex, screenText->fontOutlineColor, screenText->fontInkColor);
		setActiveFont(screenText->fontIndex);

		if (!_talkieEnabled || screenText->screenText || !_screenTextHasSpeech) {
			int16 x1, y1, x2, y2;
			TextResource *textResource = _res->get<TextResource>(screenText->resourceCacheSlot);
			const TextItem *textItem = textResource->getText(screenText->textIndex);
			Common::StringArray *chunk = textItem->chunks[screenText->chunkIndex];

			x1 = screenText->x;
			y1 = screenText->y;
			x2 = screenText->x + screenText->width;
			y2 = screenText->y;

			if (screenText->screenText) {
				x1 -= _cameraX;
				y1 -= _cameraY;
				x2 -= _cameraX;
				y2 -= _cameraY;
			}

			if (screenText->screenCenter) {
				_screen->drawTransparentRect(x1 - 6, y1 - screenText->fontHeight - 10, x2 + 6, y1 + screenText->height - screenText->fontHeight + 10);
			}

			for (int16 lineIndex = 0; lineIndex < screenText->lineCount; lineIndex++) {
				drawTextEx(x1, x2, y2, -1, (*chunk)[lineIndex]);
				y2 += screenText->fontHeight;
			}

			if (!screenText->screenCenter/*TODO:Other flags*/) {
				// TODO: Dirty rectangle
			} else {
				// TODO: Dirty rectangle
				_screen->setClipRect(0, 82, 639, 397);
				if (_inventoryActive) {
					// TODO: Draw frame for inventory text
					AnimationResource *anim = _res->get<AnimationResource>(_inventoryBoxResourceCacheSlot);
					int16 rx1 = x1 - 6;
					int16 rx2 = rx1 + screenText->width;
					int16 ry1 = y1 - screenText->fontHeight - 10;
					int16 ry2 = ry1 + screenText->height;
					addDirtyRect(rx1 - 3, ry1 - 3, screenText->width + 18, screenText->height + 26, 1);
					_screen->frameRect(rx1 - 2, ry1 - 2, rx2 + 14, ry2 + 22, 155); // TODO: Color
					_screen->frameRect(rx1 - 1, ry1 - 1, rx2 + 13, ry2 + 21, 153); // TODO: Color
					_screen->drawAnimationElement(anim, 0, rx1, ry1);
					_screen->drawAnimationElement(anim, 1, rx2 + 12, ry1);
					_screen->drawAnimationElement(anim, 2, rx2 + 12, ry2 + 20);
					_screen->drawAnimationElement(anim, 3, rx1, ry2 + 20);
				} else {
					addDirtyRect(x1 - 6, y1 - screenText->fontHeight - 10, (screenText->width + 6) * 2, (screenText->height + 10) * 2, 1);
				}
				_screen->setClipRect(0, 82, 639, 479);
			}

		}

		if (!screenText->screenText && screenText->speechPakSlot != -1 && !_loadingSavegame) {
			// TODO: Talkie code/auto advance text
			//debug("getTicks = %d; finishedTime = %d", getTicks(), screenText->finishedTime);
			// TODO: More...
			if (getTicks() >= screenText->finishedTime)
				_autoAdvanceScreenTexts = true;
		}

	}

}

void PrisonerEngine::advanceScreenTexts() {

	_autoAdvanceScreenTexts = false;

	for (int16 screenTextIndex = 0; screenTextIndex < kMaxScreenTexts; screenTextIndex++) {
		ScreenText *screenText = &_screenTexts[screenTextIndex];
		if (screenText->used == 0 || screenText->screenText)
			continue;
		_screenTextActive = true;
		screenText->chunkIndex++;
		if (screenText->chunkIndex == screenText->chunkCount) {
			unloadScreenText(screenTextIndex);
		} else {
			TextResource *textResource = _res->get<TextResource>(screenText->resourceCacheSlot);
			stopTalkieSpeech();
			setupScreenText(screenText, textResource, screenText->chunkIndex);
		}
	}

}

} // End of namespace Prisoner

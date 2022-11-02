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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/writings.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

PuzzleWritings::PuzzleWritings(AsylumEngine *engine) : Puzzle(engine) {
	_frameIndex = 0;
	_hasGlassMagnifier = false;
}

PuzzleWritings::~PuzzleWritings() {
	_textSurface.free();
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleWritings::init(const AsylumEvent &)  {
	if (getScene()->getActor()->inventory.getSelectedItem() == 3)
		_hasGlassMagnifier = true;
	else
		_hasGlassMagnifier = false;

	getScreen()->setPalette(getWorld()->graphicResourceIds[10]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[10]);

	if (_hasGlassMagnifier) {
		getCursor()->hide();
		getSharedData()->setFlag(kFlag1, true);
		_vm->setGameFlag(kGameFlag481);
	} else {
		getCursor()->set(getWorld()->graphicResourceIds[61], -1, kCursorAnimationNone, 7);
	}

	_textSurface.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	// Draw background
	getScreen()->fillRect(0, 0, 640, 480, 253);
	getScreen()->draw(getWorld()->graphicResourceIds[5], 0, Common::Point(0, 0), kDrawFlagNone, true);

	// Draw all lines of text
	int textId;
	switch (_vm->getLanguage()) {
	default:
	case Common::EN_ANY:
		textId = 1825;
		break;

	case Common::DE_DEU:
		textId = 1748;
		break;

	case Common::FR_FRA:
		textId = 1729;
		break;
	}

	getText()->loadFont(getWorld()->graphicResourceIds[42]);
	for (int i = 0; i < 10; i++, textId++)
		getText()->draw(0, 99, kTextNormal, Common::Point(70 + 60 * (i & 1), 45 + 30 * i), 16, 590,
						getText()->get(MAKE_RESOURCE(kResourcePackText, textId)));

	getText()->drawCentered(Common::Point(10, 375), 590, getText()->get(MAKE_RESOURCE(kResourcePackText, textId++)));
	getText()->drawCentered(Common::Point(10, 405), 590, getText()->get(MAKE_RESOURCE(kResourcePackText, textId)));

	_textSurface.copyFrom(getScreen()->getSurface());

	return false;
}

bool PuzzleWritings::update(const AsylumEvent &)  {
	// Adjust palette
	if (rnd(10) < 7) {
		getScreen()->setPalette(getWorld()->graphicResourceIds[6]);
		getScreen()->setGammaLevel(getWorld()->graphicResourceIds[6]);
	} else {
		getScreen()->setPalette(getWorld()->graphicResourceIds[10]);
		getScreen()->setGammaLevel(getWorld()->graphicResourceIds[10]);
	}

	// Compute frame index
	Common::Point mousePos = getCursor()->position();

	if (mousePos.x > 50 && mousePos.x < 530 && mousePos.y > 20 && mousePos.y < 430) {
		if (_frameIndex < 26)
			++_frameIndex;
	} else {
		if (_frameIndex > 0)
			--_frameIndex;
	}

	// Draw background
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 253);
	getScreen()->draw(getWorld()->graphicResourceIds[4], 0, Common::Point(0, 0), kDrawFlagNone, true);

	if (_hasGlassMagnifier) {
		mousePos -= Common::Point(50, 50);
		Common::Rect eyeBall = Common::Rect(0, 0, 640, 480).findIntersectingRect(Common::Rect(mousePos.x +  20, mousePos.y +  20,
																							  mousePos.x + 100, mousePos.y + 100));
		Graphics::Surface subArea, *subArea1;
		subArea  = _textSurface.getSubArea(eyeBall);
		subArea1 = subArea.scale(3 * eyeBall.width() / 4, 3 * eyeBall.height() / 4);
		eyeBall.left += 9;
		eyeBall.top  += 9;

		int16 dw, dh;
		dw = MAX(0, eyeBall.left + subArea1->w - 640);
		dh = MAX(0, eyeBall.top  + subArea1->h - 480);
		getScreen()->copyToBackBuffer((byte *)subArea1->getPixels(), subArea1->pitch, eyeBall.left, eyeBall.top, subArea1->w - dw, subArea1->h - dh);

		subArea1->free();
		delete subArea1;

		getScreen()->addGraphicToQueueMasked(getWorld()->graphicResourceIds[9], 0, mousePos, getWorld()->graphicResourceIds[8], mousePos, kDrawFlagNone, 2);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[7], (uint32)_frameIndex, mousePos, kDrawFlagNone, 0, 1);
	}

	getScreen()->drawGraphicsInQueue();
	getScreen()->copyBackBufferToScreen();

	return true;
}

bool PuzzleWritings::mouseRightUp(const AsylumEvent &) {
	getCursor()->hide();
	getSharedData()->setFlag(kFlag1, true);
	getScreen()->stopPaletteFade(0, 0, 0);

	_vm->switchEventHandler(getScene());

	return false;
}

} // End of namespace Asylum

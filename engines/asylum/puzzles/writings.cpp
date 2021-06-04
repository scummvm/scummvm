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
	getText()->loadFont(getWorld()->graphicResourceIds[42]);
	getText()->draw(0, 99, kTextNormal, Common::Point( 70,  45), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1825)));
	getText()->draw(0, 99, kTextNormal, Common::Point(130,  75), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1826)));
	getText()->draw(0, 99, kTextNormal, Common::Point( 70, 105), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1827)));
	getText()->draw(0, 99, kTextNormal, Common::Point(130, 135), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1828)));
	getText()->draw(0, 99, kTextNormal, Common::Point( 70, 165), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1829)));
	getText()->draw(0, 99, kTextNormal, Common::Point(130, 195), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1830)));
	getText()->draw(0, 99, kTextNormal, Common::Point( 70, 225), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1831)));
	getText()->draw(0, 99, kTextNormal, Common::Point(130, 255), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1832)));
	getText()->draw(0, 99, kTextNormal, Common::Point( 70, 285), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1833)));
	getText()->draw(0, 99, kTextNormal, Common::Point(130, 315), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1834)));
	getText()->draw(0, 99, kTextCenter, Common::Point( 20, 375), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1835)));
	getText()->draw(0, 99, kTextCenter, Common::Point( 20, 405), 16, 590, getText()->get(MAKE_RESOURCE(kResourcePackText, 1836)));

	_textSurface.copyFrom(*getScreen()->getSurface());

	return false;
}

bool PuzzleWritings::update(const AsylumEvent &)  {
	int16 x1, y1, x2, y2;

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

	mousePos.x -= 50;
	mousePos.y -= 50;
	x1 = mousePos.x + 20;
	y1 = mousePos.y + 20;
	x2 = mousePos.x + 95;
	y2 = mousePos.y + 90;
	if (y1 < 0)
		y1 = 0;
	if (y2 > 480)
		y2 = 480;

	// Draw background
	getScreen()->clearGraphicsInQueue();
	getScreen()->fillRect(0, 0, 640, 480, 253);
	getScreen()->draw(getWorld()->graphicResourceIds[4], 0, Common::Point(0, 0), kDrawFlagNone, true);

	if (_hasGlassMagnifier) {
		getScreen()->copyToBackBuffer(
				((byte *)_textSurface.getPixels()) + y1 * _textSurface.pitch + x1 * _textSurface.format.bytesPerPixel,
				_textSurface.pitch, x1, y1, (uint16)(x2 - x1), (uint16)(y2 - y1));

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

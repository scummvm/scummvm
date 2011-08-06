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

#include "asylum/views/scenetitle.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

SceneTitle::SceneTitle(AsylumEngine *engine): _vm(engine),
	_start(0), _ticks(0), _done(false), _spinnerFrameIndex(0), _spinnerProgress(0), _spinnerFrameCount(0), _showMouseState(false) {
}

SceneTitle::~SceneTitle() {
	// Zero passed pointers
	_vm = NULL;
}

void SceneTitle::load() {
	_start = _vm->getTick();

	getScreen()->clear();
	getScreen()->setPalette(getWorld()->sceneTitlePaletteResourceId);
	getScreen()->paletteFade(0, 1, 1);
	getScreen()->setGammaLevel(getWorld()->sceneTitlePaletteResourceId);

	getText()->loadFont(MAKE_RESOURCE(kResourcePackSound, 18));

	_spinnerProgress = 0;
	_spinnerFrameIndex = 0;
	_spinnerFrameCount = GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackSound, 17));

	_done = false;

	update(_start);
}

void SceneTitle::update(int32 tick) {
	if (_done)
		return;

	getScreen()->draw(getWorld()->sceneTitleGraphicResourceId);
	getScreen()->draw(MAKE_RESOURCE(kResourcePackSound, 17), _spinnerFrameIndex, Common::Point((int16)(((_spinnerProgress / 590.0) * 580.0) - 290), 0), kDrawFlagNone, false);
	getText()->drawCentered(Common::Point(320, 30), 24, MAKE_RESOURCE(kResourcePackText, getWorld()->chapter + 1811));

	// This is not from the original. It's just some arbitrary math to throttle the progress indicator.
	//
	// In the game, the scene loading progress indicated how far the engine had progressed in terms
	// of buffering the various scene resource. Since we don't actually buffer content like the original,
	// but load on demand from offset/length within a ResourcePack, the progress indicator is effectively
	// useless. It's just in here as "eye candy" :P
	if ((tick - _start) % 500 > 100) {
#ifdef DEBUG
		_spinnerProgress += 10;
#else
		_spinnerProgress += 35;
#endif
	}

	_spinnerFrameIndex++;

	if (_spinnerFrameIndex > _spinnerFrameCount - 1)
		_spinnerFrameIndex = 0;

	if (_spinnerProgress > 590) {
		_done = true;

		getScreen()->paletteFade(0, 5, 80);
	}
}

} // End of namespace Asylum

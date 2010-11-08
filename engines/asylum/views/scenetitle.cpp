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

#include "asylum/views/scenetitle.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

SceneTitle::SceneTitle(AsylumEngine *engine): _vm(engine), _bg(NULL), _progress(NULL) {
}

SceneTitle::~SceneTitle() {
	delete _bg;
	delete _progress;

	// Zero passed pointers
	_vm = NULL;
}

void SceneTitle::load() {
	_start = _vm->getTick();

	_bg = new GraphicResource(_vm, getWorld()->sceneTitleGraphicResourceId);
	getScreen()->setPalette(getWorld()->sceneTitlePaletteResourceId);

	_progress = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackSound, 17));
	_spinnerProgress = 0;
	_spinnerFrame = 0;

	getText()->loadFont(MAKE_RESOURCE(kResourcePackSound, 18));

	_done = false;
	_showMouseState = g_system->showMouse(false);
}

void SceneTitle::update(int32 tick) {

	// This is not from the original. It's just some arbitrary math to throttle the progress indicator.
	//
	// In the game, the scene loading progress indicated how far the engine had progressed in terms
	// of buffering the various scene resource. Since we don't actually buffer content like the original,
	// but load on demand from offset/length within a ResourcePack, the progress indicator is effectively
	// useless. It's just in here as "eye candy" :P
	if ((tick - _start) % 500 > 100)
		_spinnerProgress += 20;

	GraphicFrame *bgFrame = _bg->getFrame(0);

	getScreen()->copyToBackBuffer(((byte *)bgFrame->surface.pixels),
	                              bgFrame->surface.w,
	                              0, 0, 640, 480);

	ResourceId resourceId = MAKE_RESOURCE(getScene()->getPackId(), 1797);
	int32 resWidth = getText()->getResTextWidth(resourceId);
	getText()->drawResTextCentered(320 - resWidth * 24, 30, resWidth, resourceId);

	GraphicFrame *frame = _progress->getFrame(_spinnerFrame);

	getScreen()->copyRectToScreenWithTransparency(((byte*)frame->surface.pixels),
	                                              frame->surface.w,
	                                              frame->x - 290 + _spinnerProgress,
	                                              frame->y,
	                                              frame->surface.w,
	                                              frame->surface.h);

	_spinnerFrame++;

	if (_spinnerFrame > _progress->getFrameCount() - 1)
		_spinnerFrame = 0;

    if (_spinnerProgress > 590) {
		_done = true;
        g_system->showMouse(_showMouseState);
    }
}

} // End of namespace Asylum

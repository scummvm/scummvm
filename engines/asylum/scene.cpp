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

#include "asylum/scene.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

Scene::Scene(Screen *screen, Sound *sound, uint8 sceneIdx): _screen(screen), _sound(sound) {
    _sceneIdx = sceneIdx;
    _sceneResource = new SceneResource;
    if (_sceneResource->load(_sceneIdx)) {
        _text = new Text(_screen);
        _resPack = new ResourcePack(sceneIdx);

        char musPackFileName[10];
	    sprintf(musPackFileName, "mus.%03d", sceneIdx);
        _musPack = new ResourcePack(musPackFileName);

		_bgResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_commonRes.backgroundImage);
		_background = _bgResource->getFrame(0);

		// Statue animation, used for testing
		_actorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_actorsDef[0].graphicResId);
    }

	_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_commonRes.curMagnifyingGlass);

	_actorAnimCurFrame = 0;
	_startX = _startY = 0;
	_leftClick = false;
}

Scene::~Scene() {
	delete _actorResource;
	delete _cursorResource;
	delete _bgResource;
    delete _musPack;
    delete _resPack;
    delete _text;
    delete _sceneResource;
 }

void Scene::enterScene() {
	_screen->setPalette(_resPack, _sceneResource->getWorldStats()->_commonRes.palette);

	_screen->copyToBackBuffer(((byte *)_background->surface.pixels) + _startY * _background->surface.w + _startX, _background->surface.w, 0, 0, 640, 480);

	_cursorStep = 1;
	_curMouseCursor = 0;
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	// TEST
	// Draw the actor facing north
	_sceneResource->getMainActor()->setAction(_resPack, 6);
	copyActorToScreen();
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
		break;
	case Common::EVENT_LBUTTONUP:
		//_leftClick = true;	// TODO
		break;
	}

	if (doUpdate || _leftClick)
		update();
}

void Scene::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	_screen->setCursor(_cursorResource, _curMouseCursor);
}

void Scene::update() {
	bool scrollScreen = false;
	GraphicFrame *bg = _bgResource->getFrame(0);

	updateCursor();

	// Proof of concept for animation showing
	GraphicFrame *actor = _actorResource->getFrame(_actorAnimCurFrame);
	_actorAnimCurFrame++;
	if (_actorAnimCurFrame >= _actorResource->getFrameCount())
		_actorAnimCurFrame = 0;

	// Hardcoded coords
	copyToSceneBackground(actor, 655, 0);

	// Proof of concept for screen scrolling

	// TESTING
	// Main Actor
	copyActorToScreen();

	// Horizontal scrolling
	if (_mouseX < SCREEN_EDGES && _startX >= SCROLL_STEP) {
		_startX -= SCROLL_STEP;
		scrollScreen = true;
	} else if (_mouseX > 640 - SCREEN_EDGES && _startX <= bg->surface.w - 640 - SCROLL_STEP) {
		_startX += SCROLL_STEP;
		scrollScreen = true;
	}

	// Vertical scrolling
	if (_mouseY < SCREEN_EDGES && _startY >= SCROLL_STEP) {
		_startY -= SCROLL_STEP;
		scrollScreen = true;
	} else if (_mouseY > 480 - SCREEN_EDGES && _startY <= bg->surface.h - 480 - SCROLL_STEP) {
		_startY += SCROLL_STEP;
		scrollScreen = true;
	}

	//if (scrollScreen)
		_screen->copyToBackBuffer(((byte *)bg->surface.pixels) + _startY * bg->surface.w + _startX, bg->surface.w, 0, 0, 640, 480);

	// TODO
}

void Scene::copyToSceneBackground(GraphicFrame *frame, int x, int y) {
	int h = frame->surface.h;
    int w = frame->surface.w;
	byte *buffer = (byte *)frame->surface.pixels;
	byte *dest = ((byte *)_background->surface.pixels) + y * _background->surface.w + x;

	while (h--) {
		memcpy(dest, buffer, w);
		dest += _background->surface.w;
		buffer += frame->surface.w;
	}
}

void Scene::copyActorToScreen() {
	GraphicFrame *frame = _sceneResource->getMainActor()->getFrame();

	_screen->copyRectToScreenWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			_sceneResource->getMainActor()->x,
			_sceneResource->getMainActor()->y,
			frame->surface.w,
			frame->surface.h );
}
} // end of namespace Asylum

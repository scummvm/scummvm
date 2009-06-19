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

		_sceneResource->getMainActor()->setResourcePack(_resPack);

        char musPackFileName[10];
	    sprintf(musPackFileName, "mus.%03d", sceneIdx);
        _musPack = new ResourcePack(musPackFileName);

		_bgResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_commonRes.backgroundImage);
    }

	_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_commonRes.curMagnifyingGlass);

	_background = 0;
	_startX = _startY = 0;
	_leftClick = false;
}

Scene::~Scene() {
	delete _cursorResource;
	delete _bgResource;
    delete _musPack;
    delete _resPack;
    delete _text;
    delete _sceneResource;
 }

void Scene::enterScene() {
	_screen->setPalette(_resPack, _sceneResource->getWorldStats()->_commonRes.palette);

	_background = _bgResource->getFrame(0);
	_screen->copyToBackBuffer(((byte *)_background->surface.pixels) + _startY * _background->surface.w + _startX, _background->surface.w, 0, 0, 640, 480);

	_cursorStep = 1;
	_curMouseCursor = 0;
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	// Music testing: play the first music track
	_sound->playMusic(_musPack, 0);

	// TEST
	// Draw the actor walking towards the north
	_sceneResource->getMainActor()->setAction(6);
	_sceneResource->getMainActor()->drawActorAt(_screen, 200, 200);
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

	// Proof of concept for screen scrolling

	// TESTING
	// Main Actor
	if (_sceneResource->getMainActor()->_actorX != 150 || _sceneResource->getMainActor()->_actorY != 150) {
		_sceneResource->getMainActor()->walkTo(_screen, 150, 150);
	} else {
		_sceneResource->getMainActor()->setAction(15);	// face south
		_sceneResource->getMainActor()->drawActorAt(_screen, 150, 150);
	}

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

	if (scrollScreen)
		_screen->copyToBackBuffer(((byte *)bg->surface.pixels) + _startY * bg->surface.w + _startX, bg->surface.w, 0, 0, 640, 480);

	updateActor(_screen, _resPack, 7);	// the "crazy prisoner banging head" anim

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

void Scene::copyToBackBufferClipped(GraphicFrame *frame, int x, int y) {
	Common::Rect screenRect(_startX, _startY, _startX + 640, _startY + 480);
	Common::Rect animRect(x, y, x + frame->surface.w, y + frame->surface.h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate anim rectangle
		animRect.translate(-_startX, -_startY);

		int startX = animRect.right == 640 ? 0 : frame->surface.w - animRect.width();
		_screen->copyToBackBuffer(((byte*)frame->surface.pixels) +
								  (frame->surface.h - animRect.height()) * frame->surface.pitch + startX,
								  frame->surface.pitch,
								  animRect.left,
								  animRect.top,
								  animRect.width(),
								  animRect.height());
	}
}

void Scene::updateActor(Screen *screen, ResourcePack *res, uint8 actorIndex) {
	ActorDefinitions actor = _sceneResource->getWorldStats()->_actorsDef[actorIndex];
	GraphicResource *gra = new GraphicResource(res, actor.resId);
	GraphicFrame *fra = gra->getFrame(actor.tickCount);

    // DEBUG bounding box 
    // FIXME this should be a generic method which draws for an entire graphicResource and not for single graphicFrames
    fra->surface.drawLine(actor.boundingBox.top, actor.boundingBox.left, actor.boundingBox.top, actor.boundingBox.right, 0xFFFFFF);
    fra->surface.drawLine(actor.boundingBox.top, actor.boundingBox.right-1, actor.boundingBox.bottom, actor.boundingBox.right-1, 0xFFFFFF);
    fra->surface.drawLine(actor.boundingBox.bottom-1, actor.boundingBox.left, actor.boundingBox.bottom-1, actor.boundingBox.right, 0xFFFFFF);
    fra->surface.drawLine(actor.boundingBox.top, actor.boundingBox.left, actor.boundingBox.bottom, actor.boundingBox.left, 0xFFFFFF);

	copyToBackBufferClipped(fra, actor.x, actor.y);

	if (actor.tickCount < actor.frameCount - 1) {
		actor.tickCount++;
	}else{
		actor.tickCount = actor.frameIdx;
	}
    
	_sceneResource->getWorldStats()->_actorsDef[actorIndex] = actor;

    delete gra;
}

} // end of namespace Asylum

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

Scene::Scene(Screen *screen, Sound *sound, uint8 sceneIdx): _screen(screen), _sound(sound) {
    _sceneIdx = sceneIdx;
    _sceneResource = new SceneResource;
    if (_sceneResource->load(_sceneIdx)) {
        _text = new Text(_screen);
        _resPack = new ResourcePack(sceneIdx);
        
        char musPackFileName[10];
	    sprintf(musPackFileName, "mus.%03d", sceneIdx);
        _musPack = new ResourcePack(musPackFileName);
    }

	_cursorResource = new GraphicResource(_resPack, kCursorUpLeftArrow);
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

	_bgResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->_commonRes.backgroundImage);
	GraphicFrame *bg = _bgResource->getFrame(0);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, 0, 0, bg->surface.w, bg->surface.h);

	_cursorStep = 1;
	_curMouseCursor = 0;
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
		break;
	case Common::EVENT_LBUTTONUP:
		_leftClick = true;
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
	// TODO
	//updateCursor();
}

} // end of namespace Asylum

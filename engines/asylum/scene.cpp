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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include "asylum/sceneres.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugPolygons;
int g_debugBarriers;

Scene::Scene(Screen *screen, Sound *sound, uint8 sceneIdx): _screen(screen), _sound(sound) {
	_sceneIdx		= sceneIdx;
	_sceneResource	= new SceneResource;

	if (_sceneResource->load(_sceneIdx)) {
		_text 		= new Text(_screen);
		_resPack 	= new ResourcePack(sceneIdx);
		_speechPack = new ResourcePack(3);

		_sceneResource->getMainActor()->setResourcePack(_resPack);
		
		_text->loadFont(_resPack, _sceneResource->getWorldStats()->commonRes.font1);

		char musPackFileName[10];
		sprintf(musPackFileName, "mus.%03d", sceneIdx);
		_musPack 	= new ResourcePack(musPackFileName);
		_bgResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.backgroundImage);
	}

	_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.curMagnifyingGlass);

	_background  = 0;
	_startX      = 0;
	_startY      = 0;
	_leftClick   = false;
	_rightButton = false;
	_isActive    = false;

	g_debugPolygons = 0;
	g_debugBarriers = 0;
}

Scene::~Scene() {
	delete _cursorResource;
	delete _bgResource;
	delete _musPack;
	delete _speechPack;
	delete _resPack;
	delete _text;
	delete _sceneResource;
 }

void Scene::enterScene() {
	_screen->setPalette(_resPack, _sceneResource->getWorldStats()->commonRes.palette);

	_background = _bgResource->getFrame(0);
	_screen->copyToBackBuffer(((byte *)_background->surface.pixels) + _startY * _background->surface.w + _startX, _background->surface.w, 0, 0, 640, 480);

	_cursorStep		= 1;
	_curMouseCursor	= 0;
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	// Music testing: play the first music track
	_sound->playMusic(_musPack, 0);

	// TEST
	// Draw the actor facing south
	_sceneResource->getMainActor()->setAction(15);
	_sceneResource->getMainActor()->drawActorAt(_screen, 200, 200);
	_isActive = true;
}

int Scene::getDefaultActionIndex() {
	if (_sceneResource)
		return _sceneResource->getWorldStats()->actionListIdx;
	else
		return -1;
}

ActionDefinitions * Scene::getActionList(int actionListIndex) {
	if ((actionListIndex >= 0) && (actionListIndex < (int)_sceneResource->getWorldStats()->numActions))
		return &_sceneResource->getActionList()->actions[actionListIndex];
	else
		return 0;
}

void Scene::setActorPosition(int actorIndex, int x, int y) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		_sceneResource->getWorldStats()->actors[actorIndex].boundingRect.left = x;
		_sceneResource->getWorldStats()->actors[actorIndex].boundingRect.top = y;
	}
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		_sceneResource->getMainActor()->_actorX = x;
		_sceneResource->getMainActor()->_actorY = y;
	}
}

void Scene::setActorAction(int actorIndex, int action) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		_sceneResource->getWorldStats()->actors[actorIndex].direction = action;
	}
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		if(_sceneResource->getMainActor())
		_sceneResource->getMainActor()->setActionByIndex(action);  // The action appears to be an index rather than a direct resId
	}
}

void Scene::actorVisible(int actorIndex, bool visible) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		if(visible) //	TODO - enums for flags (0x01 is visible)
			_sceneResource->getWorldStats()->actors[actorIndex].flags |= 0x01;
		else
			_sceneResource->getWorldStats()->actors[actorIndex].flags &= 0xFFFFFFFE;
	}

	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		//if(_sceneResource->getMainActor())
		//_sceneResource->getMainActor()->setAction(action);
	}
}

bool Scene::actorVisible(int actorIndex) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors))
		return _sceneResource->getWorldStats()->actors[actorIndex].flags & 0x01;	//	TODO - enums for flags (0x01 is visible)
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		//if(_sceneResource->getMainActor())
		//_sceneResource->getMainActor()->setAction(action);
	}
	
	return false;
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
	case Common::EVENT_RBUTTONUP:
		delete _cursorResource;
		_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.curMagnifyingGlass);
		_rightButton    = false;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_rightButton = true;
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
	//bool scrollScreen = false;
	GraphicFrame *bg         = _bgResource->getFrame(0);
	MainActor    *mainActor  = _sceneResource->getMainActor();
	WorldStats   *worldStats = _sceneResource->getWorldStats();

	int32 curHotspot = -1;

	// Horizontal scrolling
	if (_mouseX < SCREEN_EDGES && _startX >= SCROLL_STEP) {
		_startX -= SCROLL_STEP;
		//scrollScreen = true;
	} else if (_mouseX > 640 - SCREEN_EDGES && _startX <= bg->surface.w - 640 - SCROLL_STEP) {
		_startX += SCROLL_STEP;
		//scrollScreen = true;
	}

	// Vertical scrolling
	if (_mouseY < SCREEN_EDGES && _startY >= SCROLL_STEP) {
		_startY -= SCROLL_STEP;
		//scrollScreen = true;
	} else if (_mouseY > 480 - SCREEN_EDGES && _startY <= bg->surface.h - 480 - SCROLL_STEP) {
		_startY += SCROLL_STEP;
		//scrollScreen = true;
	}

	// Copy the background to the back buffer before updating the scene animations
	_screen->copyToBackBuffer(((byte *)bg->surface.pixels) + _startY * bg->surface.w + _startX, bg->surface.w, 0, 0, 640, 480);

	updateBarrier(_screen, _resPack, 1);	// inside the middle room
	for(uint b=0; b < _sceneResource->getWorldStats()->barriers.size(); b++) {
		if ((_sceneResource->getWorldStats()->barriers[b].flags & 0x20) != 0)	//	TODO - enums for flags (0x20 is visible/playing?)
			updateBarrier(_screen, _resPack, b);
	}
	/*
	updateBarrier(_screen, _resPack, 0);	// the "statue with fireworks" animation
	//updateBarrier(_screen, _resPack, 1);	// inside the middle room
	//updateBarrier(_screen, _resPack, 2);	// the lit candles at the base of the statue
	updateBarrier(_screen, _resPack, 3);	// the rat animation (in front of the statue)
	updateBarrier(_screen, _resPack, 4);	// inside the bottom room
	updateBarrier(_screen, _resPack, 6);	// inside the top room (should be shown before the rat animation)
	updateBarrier(_screen, _resPack, 5);	// the rat animation (outside the second room)
	updateBarrier(_screen, _resPack, 7);	// the "crazy prisoner banging head" animation
	//updateBarrier(_screen, _resPack, 8);	// going up the ladder
	//updateBarrier(_screen, _resPack, 9);	// going down the ladder
	updateBarrier(_screen, _resPack, 59);	// Wobbly Guy
	*/

	// TESTING
	// Main actor walking
	if (!_rightButton) {
		//mainActor->setAction(15);	// face south
		//mainActor->drawActorAt(_screen, mainActor->_actorX, mainActor->_actorY);
		if (_sceneResource->getWorldStats()->actors[0].flags & 0x01)	// TESTING - only draw if visible flag
			mainActor->drawActor(_screen);
	} else {
		mainActor->walkTo(_screen, _mouseX, _mouseY);

		uint32 newCursor = 0;

		// Change cursor
		switch (mainActor->getCurrentAction()) {
		case kWalkN:
			newCursor = worldStats->commonRes.curScrollUp;
			break;
		case kWalkNE:
			newCursor = worldStats->commonRes.curScrollUpRight;
			break;
		case kWalkNW:
			newCursor = worldStats->commonRes.curScrollUpLeft;
			break;
		case kWalkS:
			newCursor = worldStats->commonRes.curScrollDown;
			break;
		case kWalkSE:
			newCursor = worldStats->commonRes.curScrollDownRight;
			break;
		case kWalkSW:
			newCursor = worldStats->commonRes.curScrollDownLeft;
			break;
		case kWalkW:
			newCursor = worldStats->commonRes.curScrollLeft;
			break;
		case kWalkE:
			newCursor = worldStats->commonRes.curScrollRight;
			break;
		}

		if (_cursorResource->getEntryNum() != newCursor) {
			delete _cursorResource;
			_cursorResource = new GraphicResource(_resPack, newCursor);
		}
	}

	if (g_debugPolygons)
		ShowPolygons();
	if (g_debugBarriers)
		ShowBarriers();

	// Update cursor if it's in a hotspot
	for (uint32 p = 0; p < _sceneResource->getGamePolygons()->numEntries; p++) {
		PolyDefinitions poly = _sceneResource->getGamePolygons()->polygons[p];
		if (poly.boundingRect.contains(_mouseX + _startX, _mouseY + _startY)) {
			if (pointInPoly(&poly, _mouseX + _startX, _mouseY + _startY)) {
				curHotspot = (int32)p;
				updateCursor();
				break;
			}
		}
	}

	if (_leftClick) {
		_leftClick = false;

		if (curHotspot >= 0) {
			for (uint32 a = 0; a < worldStats->numActions; a++) {
				if (worldStats->actions[a].polyIdx == curHotspot) {
					debug(0, "Hotspot: 0x%X - \"%s\", poly %d, action lists %d/%d, action type %d, sound res %d\n",
							worldStats->actions[a].id, 
							worldStats->actions[a].name,
							worldStats->actions[a].polyIdx,
							worldStats->actions[a].actionListIdx1,
							worldStats->actions[a].actionListIdx2,
							worldStats->actions[a].actionType,
							worldStats->actions[a].soundResId);
					// Play the SFX associated with the hotspot
					// TODO: The hotspot sound res id is 0, seems like we need to get it from the associated action list
					//_sound->playSfx(_resPack, worldStats->_actors[a].soundResId);

					// TODO: This should all be moved to a script related class
					ActionDefinitions actionDefs = _sceneResource->getActionList()->actions[worldStats->actions[a].actionListIdx1];
					for (int command = 0; command < 161; command++) {
						if (actionDefs.commands[command].opcode == 65) {	// play voice
							_sound->playSfx(_speechPack, actionDefs.commands[command].param1);
							//_text->drawResText(actionDefs.commands[command].param1+83);
							// TODO: param2 (usually 1) -> number of loops?
						}
					}
					break;
				}
			}
		}
	}

}

#if 0
void Scene::copyToSceneBackground(GraphicFrame *frame, int x, int y) {
	int h = frame->surface.h;
	int w = frame->surface.w;
	byte *buffer = (byte *)frame->surface.pixels;
	byte *dest   = ((byte *)_background->surface.pixels) + y * _background->surface.w + x;

	while (h--) {
		memcpy(dest, buffer, w);
		dest   += _background->surface.w;
		buffer += frame->surface.w;
	}
}
#endif

void Scene::copyToBackBufferClipped(Graphics::Surface *surface, int x, int y) {
	Common::Rect screenRect(_startX, _startY, _startX + 640, _startY + 480);
	Common::Rect animRect(x, y, x + surface->w, y + surface->h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate anim rectangle
		animRect.translate(-_startX, -_startY);

		int startX = animRect.right == 640 ? 0 : surface->w - animRect.width();
		int startY = animRect.bottom == 480 ? 0 : surface->h - animRect.height();

		if (surface->w > 640)
			startX = _startX;
		if (surface->h > 480)
			startY = _startY;
		
		_screen->copyToBackBufferWithTransparency(((byte*)surface->pixels) +
												  startY * surface->pitch + 
												  startX * surface->bytesPerPixel,
												  surface->pitch,
												  animRect.left,
												  animRect.top,
												  animRect.width(),
												  animRect.height());
	}
}

void Scene::updateBarrier(Screen *screen, ResourcePack *res, uint8 barrierIndex) {
	BarrierItem barrier  = _sceneResource->getWorldStats()->barriers[barrierIndex];
	GraphicResource *gra = new GraphicResource(res, barrier.resId);
	if (!gra->getFrameCount())
		return;

	GraphicFrame *fra = gra->getFrame(barrier.tickCount);

#if 0
	// DEBUG bounding box 
	// FIXME this should be a generic method which draws for an entire graphicResource and not for single graphicFrames
	fra->surface.frameRect(barrier.boundingRect, 0xFF);
#endif

	copyToBackBufferClipped(&fra->surface, barrier.x, barrier.y);

	if (barrier.tickCount < barrier.frameCount - 1) {
		barrier.tickCount++;
	} else {
		barrier.tickCount = barrier.frameIdx;
	}
	
	_sceneResource->getWorldStats()->barriers[barrierIndex] = barrier;

	delete gra;
}

bool Scene::pointInPoly(PolyDefinitions *poly, int x, int y) {
	// Copied from backends/vkeybd/polygon.cpp
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	Common::Point *vtx0 = &poly->points[poly->numPoints - 1];
	Common::Point *vtx1 = &poly->points[0];

	yflag0 = (vtx0->y >= y);
	for (pt = 0; pt < poly->numPoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}


// POLYGONS DEBUG
void Scene::ShowPolygons() {
	for (uint32 p = 0; p < _sceneResource->getGamePolygons()->numEntries; p++) {
		Graphics::Surface surface;
		PolyDefinitions poly = _sceneResource->getGamePolygons()->polygons[p];
		surface.create(poly.boundingRect.right - poly.boundingRect.left + 1, poly.boundingRect.bottom - poly.boundingRect.top + 1, 1);
		
		// Draw all lines in Polygon
		for (uint32 i=0; i < poly.numPoints; i++) {
			surface.drawLine(
				poly.points[i].x - poly.boundingRect.left, 
				poly.points[i].y - poly.boundingRect.top, 
				poly.points[(i+1) % poly.numPoints].x - poly.boundingRect.left, 
				poly.points[(i+1) % poly.numPoints].y - poly.boundingRect.top, 0xFF);
		}
		
		// Draw Bounding Box
		//surface.frameRect(Common::Rect(0, 0, surface.w, surface.h), 0xFF);   

		copyToBackBufferClipped(&surface, poly.boundingRect.left, poly.boundingRect.top);

		surface.free();
	}
}

// BARRIER DEBUGGING
void Scene::ShowBarriers() {
	for (uint32 p = 0; p < _sceneResource->getWorldStats()->numBarriers; p++) {
		Graphics::Surface surface;
		BarrierItem b = _sceneResource->getWorldStats()->barriers[p];

		if (b.flags & 0x20) {
			surface.create(b.boundingRect.right - b.boundingRect.left + 1, b.boundingRect.bottom - b.boundingRect.top + 1, 1);
			surface.frameRect(b.boundingRect, 0x22);
			copyToBackBufferClipped(&surface, b.x, b.y);
		}

		surface.free();
	}
}


} // end of namespace Asylum

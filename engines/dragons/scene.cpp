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
#include "scene.h"

#include "actor.h"
#include "background.h"
#include "dragonini.h"
#include "screen.h"

namespace Dragons {

Scene::Scene(Screen *screen, BigfileArchive *bigfileArchive, ActorManager *actorManager, DragonRMS *dragonRMS, DragonINIResource *dragonINIResource)
		: _screen(screen), _stage(0), _bigfileArchive(bigfileArchive), _actorManager(actorManager), _dragonRMS(dragonRMS), _dragonINIResource(dragonINIResource) {
	_backgroundLoader = new BackgroundResourceLoader(_bigfileArchive, _dragonRMS);
}

void Scene::loadScene(uint32 sceneId, uint32 cameraPointId) {
	_stage = _backgroundLoader->load(sceneId);

	_camera = _stage->getPoint2(cameraPointId);

	debug("Flicker: (%X, %X)", _camera.x, _camera.y);

	if (_camera.x > 160) {
		_camera.x -= 160;
	} else {
		_camera.x = 0;
	}

	if (_camera.y > 100) {
		_camera.y -= 100;
	} else {
		_camera.y = 0;
	}

	if (_camera.x + 320 >= _stage->getWidth()) {
		_camera.x = _stage->getWidth() - 320;
	}

	if (_camera.y + 200 >= _stage->getHeight()) {
		_camera.y = _stage->getHeight() - 200;
	}

	debug("Camera: (%d, %d)", _camera.x, _camera.y);

	for(int i=0;i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _dragonINIResource->getRecord(i);
		if (ini->sceneId == sceneId && (ini->field_1a_flags_maybe & 1)) {
			Actor *actor = _actorManager->loadActor(ini->actorResourceId, ini->frameIndexId_maybe, ini->x, ini->y, 0);

			if (ini->frameIndexId_maybe & 0x1000) {
				actor->frame_flags |= 0x10;
			} else {
				if (ini->field_1a_flags_maybe & 0x2000) {
					actor->frame_flags |= 0x20;
				} else {
					actor->frame_flags &= 0xffef;
				}
			}

			actor->_sequenceID2 = ini->field_20_actor_field_14;

			if (ini->field_1a_flags_maybe & 2) {
				actor->flags |= 0x80;
			} else {
				actor->flags &= 0xfeff;
			}

			if (ini->field_1a_flags_maybe & 0x20) {
				actor->flags |= 0x100;
			} else {
				actor->flags &= 0xfeff;
			}

			if (ini->field_1a_flags_maybe & 4) {
				actor->flags |= 0x8000;
			} else {
				actor->flags &= 0x7fff;
			}

			if (ini->field_1a_flags_maybe & 0x100) {
				actor->flags |= 0x4000;
			} else {
				actor->flags &= 0xbfff;
			}

			Graphics::Surface *s = actor->getCurrentFrame();
			int x = ini->x - actor->frame_vram_x;
			int y = ini->y - actor->frame_vram_y;
			if (x >= 0 && y >= 0 && x + s->w < 320 && y + s->h < 200) {
				debug("Actor %d %d (%d, %d)", ini->actorResourceId, ini->field_1a_flags_maybe, ini->x, ini->y);
				_stage->getFgLayer()->copyRectToSurface(*s, x, y, Common::Rect(s->w,s->h));
			}
			_stage->getFgLayer()->drawLine(ini->x, ini->y, ini->x + 8, ini->y + 8, 0x7c00);
			//break;
		}
	}

}

void Scene::draw() {
	Common::Rect rect(_camera.x, _camera.y, 320, 200);
	_screen->copyRectToSurface(*_stage->getBgLayer(), 0, 0, rect);
	_screen->copyRectToSurface(*_stage->getMgLayer(), 0, 0, rect);
	_screen->copyRectToSurface(*_stage->getFgLayer(), 0, 0, rect);
}

} // End of namespace Dragons

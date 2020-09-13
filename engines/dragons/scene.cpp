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
#include "dragons/scene.h"
#include "dragons/dragons.h"
#include "dragons/actor.h"
#include "dragons/background.h"
#include "dragons/cursor.h"
#include "dragons/credits.h"
#include "dragons/dragonini.h"
#include "dragons/dragonimg.h"
#include "dragons/font.h"
#include "dragons/inventory.h"
#include "dragons/screen.h"
#include "dragons/actorresource.h"
#include "dragons/scriptopcodes.h"

namespace Dragons {


Scene::Scene(DragonsEngine *vm, Screen *screen, ScriptOpcodes *scriptOpcodes, ActorManager *actorManager, DragonRMS *dragonRMS, DragonINIResource *dragonINIResource, BackgroundResourceLoader *backgroundResourceLoader)
		: _vm(vm), _screen(screen), _scriptOpcodes(scriptOpcodes), _stage(0), _actorManager(actorManager), _dragonRMS(dragonRMS), _dragonINIResource(dragonINIResource), _backgroundLoader(backgroundResourceLoader) {
	_mapTransitionEffectSceneID = 2;
	_data_800633ee = 0;

	_currentSceneId = -1;
}
void Scene::loadScene(uint32 sceneId, uint32 cameraPointId) {
	if (!_vm->isFlagSet(ENGINE_FLAG_40)) {
		_vm->fadeToBlack();
	}
	bool unkFlag2Set = _vm->isUnkFlagSet(ENGINE_UNK1_FLAG_2);
	bool flag8set = _vm->isFlagSet(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);

	for (int i = 0; i < 8; i++) {
		_vm->_paletteCyclingTbl[i].updateInterval = 0;
	}

	// TODO
	_vm->reset_screen_maybe();

	loadSceneData(sceneId, cameraPointId);

	if (flag8set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}

	if (!_vm->isFlagSet(ENGINE_FLAG_8000000) && sceneId != 4) {
		_vm->_cursor->updateSequenceID((int16)_vm->_cursor->_sequenceID);
	}
	_vm->waitForFrames(2);
	_vm->fadeFromBlack();
	if (!unkFlag2Set) {
		_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);
	}
	_data_800633ee = 0;

	if (!(sceneId & 0x8000)) {
		byte *obd = _dragonRMS->getAfterSceneLoadedScript(sceneId);
		ScriptOpCall scriptOpCall(obd + 4, READ_LE_UINT32(obd));
		_scriptOpcodes->runScript(scriptOpCall);
	}
	DragonINI *ini = _dragonINIResource->getRecord(0xc4);
	ini->objectState = 0;
}

void Scene::loadSceneData(uint32 sceneId, uint32 cameraPointId) {
	bool isUnkFlag2Set = _vm->isUnkFlagSet(ENGINE_UNK1_FLAG_2);

	_vm->setUnkFlags(ENGINE_UNK1_FLAG_2 | Dragons::ENGINE_UNK1_FLAG_8);

	for (int i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _dragonINIResource->getRecord(i);
		ini->counter = -1;
		ini->flags &= ~INI_FLAG_10;
	}

	uint16 sceneIdStripped = (uint16)sceneId & ~0x8000;
	if (sceneIdStripped == 0x18 || sceneIdStripped == 0x26 || sceneIdStripped == 0x7 ||
			sceneIdStripped == 0x17 || sceneIdStripped == 0x5 || sceneIdStripped == 0x19 ||
			sceneIdStripped == 0x34 || sceneIdStripped == 0x1d || sceneIdStripped == 0x6) {
//		buf2048bytes = buf2048bytes + 0x1800;
		// error("0x8002f404"); //TODO do we need this logic?
	}

	_screen->setScreenShakeOffset(0, 0);

	if (!(sceneId & 0x8000)) {
		byte *obd = _dragonRMS->getBeforeSceneDataLoadedScript(sceneId);
		ScriptOpCall scriptOpCall(obd + 4, READ_LE_UINT32(obd));
		uint16 oldSceneId = _currentSceneId;
		_currentSceneId = -1;
		_scriptOpcodes->runScript(scriptOpCall);
		_currentSceneId = oldSceneId;
	}

	_actorManager->clearActorFlags(2);
	//TODO stopAndCloseSceneVab()

	_vm->_cursor->setActorFlag400();
	_vm->_inventory->setActorFlag400();
	_vm->clearFlags(ENGINE_FLAG_200);

	resetActorFrameFlags();

	// Loading animation logic would go here. 0x8002f538

	_vm->clearFlags(ENGINE_FLAG_20);
	_vm->setUnkFlags(ENGINE_UNK1_FLAG_10);

	_vm->fadeFromBlack();
	// TODO 0x8002f7c4

	_vm->_cursor->updatePosition(160, 100);

	_vm->clearFlags(ENGINE_FLAG_100000);
	_vm->clearFlags(ENGINE_FLAG_200000);

	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();

	if (flicker == nullptr || flicker->sceneId == 0) {
		_vm->getINI(1)->sceneId = 0;
	} else {
		_currentSceneId = (uint16)(sceneId & 0x7fff);
		flicker->sceneId = _currentSceneId;
		_vm->getINI(1)->sceneId = _currentSceneId;
	}

	_vm->loadCurrentSceneMsf();

	_stage = _backgroundLoader->load(sceneId);
	if (!_vm->isFlagSet(ENGINE_FLAG_800)) {
		byte *cursorPalette = _vm->_cursor->getPalette();
		byte *stagePalette = _stage->getPalette();
		for (int i = 0xc0; i < 0x100; i++) {
			stagePalette[i * 2] = cursorPalette[(i-0xc0) * 2];
			stagePalette[i * 2 + 1] = cursorPalette[(i-0xc0) * 2 + 1];
		}
	}
	for (int i = 1; i < 0x100; i ++) {
		byte *stagePalette = _stage->getPalette();
		uint16 c = READ_LE_INT16(stagePalette + i * 2);
		if ((c & 0x7fff) == 0) {
			stagePalette[i * 2 + 1] |= 0x80;
		}
	}
	_screen->loadPalette(0, _stage->getPalette());

	for (int i = 1; i < 0x100; i ++) {
		byte *stagePalette = _stage->getPalette();
		uint16 c = READ_LE_INT16(stagePalette + i * 2);
		if ((c & 0x7fff) == 0) {
			stagePalette[i * 2] = 1;
			stagePalette[i * 2 + 1] = 0;
		}
	}

	_camera = _stage->getPoint2(cameraPointId);

	if (flicker && !(sceneId & 0x8000)) {
		flicker->x = _camera.x;
		flicker->y = _camera.y;
		_vm->getINI(1)->x = _camera.x;
		_vm->getINI(1)->y = _camera.y;
	}

	debug(3, "Flicker: (%X, %X)", _camera.x, _camera.y);

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

	debug(3, "Camera: (%d, %d)", _camera.x, _camera.y);

	// 0x8002ff80
	_vm->fadeToBlack();
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_10);
	_vm->setFlags(ENGINE_FLAG_20);
	// TODO reset vsync_updater_function

	_vm->setFlags(ENGINE_FLAG_200);
	_actorManager->clearActorFlags(2);
	_vm->_isLoadingDialogAudio = false;
	// TODO 0x8002fff0

	for (int i = 0; i < _dragonINIResource->totalRecords(); i++) {
		DragonINI *ini = _dragonINIResource->getRecord(i);
		if (ini->sceneId == sceneIdStripped) {
			if (ini->flags & 1) {
				Actor *actor = _actorManager->loadActor(ini->actorResourceId, ini->sequenceId, ini->x, ini->y, 0);

				if (actor) {
					ini->actor = actor;
					if (ini->flags & 0x1000) {
						actor->_frame_flags |= 0x10;
					} else {
						if (ini->flags & 0x2000) {
							actor->_frame_flags |= 0x20;
						} else {
							actor->_frame_flags &= ~0x10;
						}
					}

					actor->_direction = ini->direction2;

					if (ini->flags & 2) {
						actor->_flags |= ACTOR_FLAG_80;
					} else {
						actor->_flags &= 0xfeff;
					}

					if (ini->flags & 0x20) {
						actor->_flags |= ACTOR_FLAG_100;
					} else {
						actor->_flags &= 0xfeff;
					}

					if (ini->flags & 4) {
						actor->_flags |= ACTOR_FLAG_8000;
					} else {
						actor->_flags &= 0x7fff;
					}

					if (ini->flags & 0x100) {
						actor->_flags |= ACTOR_FLAG_4000;
					} else {
						actor->_flags &= 0xbfff;
					}
//
//				Graphics::Surface *s = actor->getCurrentFrame();
//				int x = ini->x - actor->_frame_vram_x;
//				int y = ini->y - actor->_frame_vram_y;
//				if (x >= 0 && y >= 0 && x + s->w < 320 && y + s->h < 200) {
//					debug("Actor %d, %d %d (%d, %d)", actor->_actorID, ini->actorResourceId, ini->flags, ini->x, ini->y);
//					_stage->getFgLayer()->copyRectToSurface(*s, x, y, Common::Rect(s->w, s->h));
//				}
				}
				// _stage->getFgLayer()->drawLine(ini->x, ini->y, ini->x + 8, ini->y + 8, 0x7c00);
				//break;
			} else {
				if (ini->iptIndex_maybe != -1) {
					loadImageOverlay(ini->iptIndex_maybe);
				}
			}
		}
	}

	// 0x80030458
	DragonINI *ini = _vm->getINI(1);
	if (ini->actor && _vm->_dragonINIResource->getFlickerRecord() && _vm->_dragonINIResource->getFlickerRecord()->sceneId == _currentSceneId) {
		ini->actor->setFlag(ACTOR_FLAG_100);
		ini->actor->_priorityLayer = 0;
	}


	if (flicker && flicker->sceneId != 0) {
		flicker->direction2 = _vm->_flickerInitialSceneDirection;
		if (flicker->actor) {
			flicker->actor->_direction = _vm->_flickerInitialSceneDirection;
			flicker->actor->setFlag(ACTOR_FLAG_4);
		}
	}

	// 0x800305bc
	_vm->_inventory->loadScene(_currentSceneId);

	// 0x8003070c
	// TODO sub_80013b3c(); // palette related.

	if (_vm->_inventory->isOpen()) {
		_vm->_inventory->close();
	}

	if (!_vm->isFlagSet(ENGINE_FLAG_10000)) {
		_vm->setFlags(ENGINE_FLAG_10);
	}

	_vm->setFlags(ENGINE_FLAG_1);
	_vm->setFlags(ENGINE_FLAG_200);
	_vm->setFlags(ENGINE_FLAG_4000000);

	if (flicker && flicker->sceneId == _currentSceneId) {

		flicker->actor->updateSequence((uint16)flicker->actor->_direction);
	}

	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_2);
	_vm->clearUnkFlags(ENGINE_UNK1_FLAG_8);

	if (isUnkFlag2Set) {
		_vm->setUnkFlags(ENGINE_UNK1_FLAG_2);
	}

	if (!(sceneId & 0x8000)) {
		byte *obd = _dragonRMS->getAfterSceneDataLoadedScript(sceneId);
		ScriptOpCall scriptOpCall(obd + 4, READ_LE_UINT32(obd));
		_scriptOpcodes->runScript(scriptOpCall);
	}

}

void Scene::draw() {
	Common::Rect rect(_camera.x, _camera.y, _camera.x + 320, _camera.y + 200);
	_vm->_screen->clearScreen();

	for (uint16 priority = 1; priority < 16; priority++) {
		if (_vm->isInMenu() || (priority == 7 && _vm->isFlagSet(ENGINE_FLAG_200))) {
			_vm->_fontManager->updatePalette();
			_vm->_fontManager->draw();
		}

		if (_vm->isFlagSet(ENGINE_FLAG_200)) {
			if (priority == 5) {
				if (_vm->isFlagSet(ENGINE_FLAG_80)) {
					_vm->_inventory->draw();
				}
			}

			if (priority == _stage->getFgLayerPriority()) {
				drawBgLayer(2, rect, _stage->getFgLayer());
			}
			if (priority == _stage->getMgLayerPriority()) {
				drawBgLayer(1, rect, _stage->getMgLayer());
			}
			if (priority == _stage->getBgLayerPriority()) {
				drawBgLayer(0, rect, _stage->getBgLayer());
			}
		}

		_screen->drawFlatQuads(priority);

		for (int16 i = 0; i < DRAGONS_ENGINE_NUM_ACTORS; i++) {
			Actor *actor = _actorManager->getActorByDisplayOrder(i);
			if (actor->_x_pos == -100 && actor->_y_pos == 100) {
				actor->_priorityLayer = 0;
				continue;
			}

			if (actor->_flags & ACTOR_FLAG_40 &&
					!(actor->_flags & ACTOR_FLAG_400) &&
					actor->_surface &&
					actor->_frame->width != 0 &&
					actor->_frame->height != 0) {
				Graphics::Surface *s = actor->_surface;
				if (actor->_priorityLayer == priority) { //} && x + s->w < 320 && y + s->h < 200) {
					if (!actor->isFlagSet(ACTOR_FLAG_80)) {
						actor->_scale = _stage->getScaleLayer()->getScale(actor->_y_pos);
					}
					int x = actor->_x_pos - (actor->_frame->xOffset * actor->_scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) - (actor->isFlagSet(ACTOR_FLAG_200) ? 0 : _camera.x);
					int y = actor->_y_pos - (actor->_frame->yOffset * actor->_scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) - (actor->isFlagSet(ACTOR_FLAG_200) ? 0 : _camera.y);

					debug(5, "Actor %d %s (%d, %d) w:%d h:%d Priority: %d Scale: %d", actor->_actorID, actor->_actorResource->getFilename(), x,
						  y,
						  s->w, s->h, actor->_priorityLayer, actor->_scale);
					_screen->copyRectToSurface8bpp(*s, actor->getPalette(), x, y, Common::Rect(s->w, s->h), (bool)(actor->_frame->flags & FRAME_FLAG_FLIP_X), actor->isFlagSet(ACTOR_FLAG_8000) ? NONE : NORMAL, actor->_scale);
					if (_vm->isDebugMode()) {
						_screen->drawRect(0x7fff, Common::Rect(x, y, x + s->w, y + s->h), actor->_actorID);
						drawActorNumber(x + s->w, y + 8, actor->_actorID);
					}
				}
			}
		}
	}
	if (_vm->_credits->isRunning()) {
		_vm->_credits->draw();
	}

	if (_vm->isDebugMode()) {
		_vm->_fontManager->clearText();
	}
}

int16 Scene::getPriorityAtPosition(Common::Point pos) {
	return _stage->getPriorityAtPoint(pos);
}

bool Scene::contains(DragonINI *ini) {
	assert(ini);
	return ini->sceneId == _currentSceneId;
}

byte *Scene::getPalette() {
	assert(_stage);
	return _stage->getPalette();
}

uint16 Scene::getSceneId() {
	return (uint16)_currentSceneId;
}

Common::Point Scene::getPoint(uint32 pointIndex) {
	return _stage->getPoint2(pointIndex);
}

uint16 Scene::getStageWidth() {
	return _stage->getWidth();
}

uint16 Scene::getStageHeight() {
	return _stage->getHeight();
}

void Scene::loadImageOverlay(uint16 iptId) {
	Img *img =_vm->_dragonImg->getImg(iptId);
	if (img->h != 0) {
		if (img->field_e <= 2) {
			_stage->overlayImage(img->layerNum - 1, img->data, img->x, img->y, img->w, img->h);
		}

		if (img->field_e == 2 || img->field_e == 0) {
			_stage->overlayPriorityTileMap(img->data + img->w * img->h * 2, img->x, img->y, img->w, img->h);
		}
	}
}

void Scene::removeImageOverlay(uint16 iptId) {
	Img *img =_vm->_dragonImg->getImg(iptId);
	_stage->restoreTiles(img->layerNum - 1, img->x, img->y, img->w, img->h);
	_stage->restorePriorityTileMap(img->x, img->y, img->w, img->h);
}

void Scene::setSceneId(int16 newSceneId) {
	_currentSceneId = newSceneId;
}

void Scene::resetActorFrameFlags() {
	for (int i = 0; i < 0x17; i++) {
		Actor *actor = _vm->_actorManager->getActor(i);
		actor->_frame_flags &= ~ACTOR_FRAME_FLAG_10;
		actor->_frame_flags &= ~ACTOR_FRAME_FLAG_20;
	}
}

void Scene::setBgLayerPriority(uint8 newPriority) {
	_stage->setBgLayerPriority(newPriority);
}

void Scene::setMgLayerPriority(uint8 newPriority) {
	_stage->setMgLayerPriority(newPriority);
}

void Scene::setFgLayerPriority(uint8 newPriority) {
	_stage->setFgLayerPriority(newPriority);
}

void Scene::setStagePalette(byte *newPalette) {
	_stage->setPalette(newPalette);
}


void Scene::drawActorNumber(int16 x, int16 y, uint16 actorId) {
	uint16 text[30];
	char text8[15];

	sprintf(text8, "%d", actorId);

	for (uint i = 0; i < strlen(text8); i++) {
		text[i] = text8[i];
	}
	_vm->_fontManager->addText(x, y, text, strlen(text8), 1);
}

void Scene::setLayerOffset(uint8 layerNumber, Common::Point offset) {
	_stage->setLayerOffset(layerNumber, offset);
}

Common::Point Scene::getLayerOffset(uint8 layerNumber) {
	return _stage->getLayerOffset(layerNumber);
}

void Scene::drawBgLayer(uint8 layerNumber, Common::Rect rect, Graphics::Surface *surface) {
	Common::Point offset = _stage->getLayerOffset(layerNumber);
//	Common::Rect clippedRect = _screen->clipRectToRect(offset.x, offset.y, rect, Common::Rect(_stage->getBgLayer()->w, _stage->getBgLayer()->h));
	rect.left += rect.left + offset.x < 0 ? -(rect.left + offset.x) : offset.x;
	if (rect.right + offset.x > surface->w) {
		rect.right = surface->w - 1;
	} else {
		rect.right += offset.x;
	}
//	clippedRect.right += offset.x < 0 ? -offset.x : 0;
	rect.top += rect.top + offset.y < 0 ? -(rect.top + offset.y) : offset.y;
	if (rect.bottom + offset.y > surface->h) {
		rect.bottom = surface->h - 1;
	} else {
		rect.bottom += offset.y;
	}
//	clippedRect.bottom += offset.y < 0 ? -offset.y : 0;
	_screen->copyRectToSurface8bppWrappedX(*surface, _screen->getPalette(0), rect, _stage->getLayerAlphaMode(layerNumber));
}

ScaleLayer *Scene::getScaleLayer() {
	return _stage->getScaleLayer();
}

void Scene::setLayerAlphaMode(uint8 layerNumber, AlphaBlendMode mode) {
	_stage->setLayerAlphaMode(layerNumber, mode);
}

} // End of namespace Dragons

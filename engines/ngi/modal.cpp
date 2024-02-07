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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ngi/ngi.h"

#include "ngi/constants.h"
#include "ngi/gameloader.h"
#include "ngi/messages.h"
#include "ngi/modal.h"
#include "ngi/motion.h"
#include "ngi/objectnames.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "engines/savestate.h"

#include "graphics/paletteman.h"
#include "graphics/surface.h"

namespace NGI {

ModalIntro::ModalIntro() {
	_field_8 = 0;
	_countDown = 0;
	_stillRunning = 0;

	if (g_vars->sceneIntro_skipIntro) {
		_introFlags = 4;
	} else {
		_introFlags = 33;
		_countDown = 150;

		PictureObject *pict = g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0);
		pict->setFlags(pict->_flags & 0xFFFB);
	}

	g_vars->sceneIntro_skipIntro = false;
	_sfxVolume = g_nmi->_sfxVolume;
}

ModalIntro::~ModalIntro() {
	g_nmi->stopAllSounds();
	g_nmi->_sfxVolume = _sfxVolume;
}

bool ModalIntro::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	if (message->_messageNum != 36)
		return false;

	if (message->_param != 13 && message->_param != 27 && message->_param != 32)
		return false;

	if (_stillRunning) {
		if (!(_introFlags & 0x10)) {
			_countDown = 0;
			g_vars->sceneIntro_needBlackout = true;
			return true;
		}
		g_vars->sceneIntro_playing = false;
		g_vars->sceneIntro_needBlackout = true;
	}

	return true;
}

bool ModalIntro::init(int counterdiff) {
	if (!g_vars->sceneIntro_playing) {
		if (!_stillRunning) {
			finish();
			return false;
		}

		if (_introFlags & 0x10)
			g_nmi->_gameLoader->updateSystems(42);

		_introFlags |= 2;

		return true;
	}

	if (_introFlags & 4) {
		ModalVideoPlayer *player = new ModalVideoPlayer();

		g_nmi->_modalObject = player;
		player->_parentObj = this;
		player->play("intro.avi");

		_countDown--;

		if (_countDown > 0 )
			return true;

		if (_stillRunning <= 0) {
			_countDown = 0;
			_stillRunning = 0;
			_introFlags = (_introFlags & 0xfb) | 0x40;

			return true;
		}

		_introFlags |= 2;
		return true;
	}

	if (_introFlags & 0x40) {
		ModalVideoPlayer *player = new ModalVideoPlayer();

		g_nmi->_modalObject = player;
		player->_parentObj = this;
		player->play("intro2.avi");

		_countDown--;
		if (_countDown > 0)
			return true;

		if (_stillRunning <= 0) {
			_countDown = 50;
			_stillRunning = 0;
			_introFlags = (_introFlags & 0xbf) | 9;

			return true;
		}

		_introFlags |= 2;
		return true;
	}

	if (_introFlags & 8) {
		_countDown--;

		if (_countDown > 0 )
			return true;

		if (_stillRunning > 0) {
			_introFlags |= 2;
			return true;
		}

		_countDown = 150;
		_introFlags = (_introFlags & 0xf7) | 0x21;
		g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0)->_flags &= 0xfffb;
	}

	if (!(_introFlags & 0x20)) {
		if (_introFlags & 0x10) {
			if (!_stillRunning) {
				_introFlags |= 1;

				g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0)->_flags &= 0xfffb;
				g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_GAMETITLE, 0)->_flags &= 0xfffb;

				chainQueue(QU_INTR_STARTINTRO, 1);
			}
			g_nmi->_gameLoader->updateSystems(42);
		}
		return true;
	}

	_countDown--;

	if (_countDown <= 0) {
		if (_stillRunning > 0) {
			_introFlags |= 2;

			return true;
		}

		_introFlags = (_introFlags & 0xdf) | 0x10;

		g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_GAMETITLE, 0)->_flags &= 0xfffb;

		_stillRunning = 0;
	}

	return true;
}

void ModalIntro::update() {
	if (g_nmi->_currentScene) {
		if (_introFlags & 1) {
			g_nmi->sceneFade(g_nmi->_currentScene, true);
			_stillRunning = 255;
			_introFlags &= 0xfe;

			if (_introFlags & 0x20)
				g_nmi->playSound(SND_INTR_019, 0);
		} else if (_introFlags & 2) {
			if (g_vars->sceneIntro_needBlackout) {
				g_nmi->drawAlphaRectangle(0, 0, 800, 600, 0);
				g_vars->sceneIntro_needBlackout = 0;
				_stillRunning = 0;
				_introFlags &= 0xfd;
			} else {
				g_nmi->sceneFade(g_nmi->_currentScene, false);
				_stillRunning = 0;
				_introFlags &= 0xfd;
			}
		} else if (_stillRunning) {
			g_nmi->_currentScene->draw();
		}
	}
}

void ModalIntro::finish() {
	g_nmi->_gameLoader->unloadScene(SC_INTRO2);

	g_nmi->_currentScene = g_nmi->accessScene(SC_INTRO1);
	g_nmi->_gameLoader->preloadScene(SC_INTRO1, TrubaDown);

	if (g_nmi->_currentScene)
		g_nmi->_gameLoader->updateSystems(42);
}

ModalIntroDemo::ModalIntroDemo() {
	_field_8 = 0;
	_countDown = 50;
	_stillRunning = 0;
	_introFlags = 9;
	g_vars->sceneIntro_skipIntro = false;
	_sfxVolume = g_nmi->_sfxVolume;
}

ModalIntroDemo::~ModalIntroDemo() {
	g_nmi->stopAllSounds();
	g_nmi->_sfxVolume = _sfxVolume;
}

bool ModalIntroDemo::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	if (message->_messageNum != 36)
		return false;

	if (message->_param != 13 && message->_param != 27 && message->_param != 32)
		return false;

	if (_introFlags & 0x8) {
		_countDown = 0;
		g_vars->sceneIntro_needBlackout = true;
		return true;
	} else if (_stillRunning) {
		g_vars->sceneIntro_playing = false;
		g_vars->sceneIntro_needBlackout = true;
	}

	return true;
}

bool ModalIntroDemo::init(int counterdiff) {
	if (!g_vars->sceneIntro_playing) {
		if (!_stillRunning) {
			finish();
			return false;
		}

		if (_introFlags & 0x10)
			g_nmi->_gameLoader->updateSystems(42);

		_introFlags |= 2;

		return true;
	}

	if (_introFlags & 8) {
		_countDown--;

		if (_countDown > 0)
			return true;

		if (_stillRunning > 0) {
			_introFlags |= 2;
			return true;
		}

		_countDown = 150;
		_introFlags = (_introFlags & 0xf7) | 0x21;
		g_nmi->accessScene(SC_INTRO1)->getPictureObjectById(522, 0)->_flags &= 0xfffb;
	} else {
		if (!(_introFlags & 0x20))
			return true;

		_countDown--;

		if (_countDown > 0)
			return true;

		if (_stillRunning > 0) {
			_introFlags |= 2;
			return true;
		}

		_introFlags &= 0xDF;

		g_vars->sceneIntro_playing = false;
		_stillRunning = 0;
	}

	return true;
}

void ModalIntroDemo::update() {
	if (g_nmi->_currentScene) {
		if (_introFlags & 1) {
			if (g_vars->sceneIntro_needBlackout) {
				g_nmi->drawAlphaRectangle(0, 0, 800, 600, 0);
				g_vars->sceneIntro_needBlackout = 0;
			} else {
				g_nmi->sceneFade(g_nmi->_currentScene, true);
			}
			_stillRunning = 255;
			_introFlags &= 0xfe;

			if (_introFlags & 0x20)
				g_nmi->playSound(SND_INTR_019, 0);
		} else if (_introFlags & 2) {
			if (g_vars->sceneIntro_needBlackout) {
				g_nmi->drawAlphaRectangle(0, 0, 800, 600, 0);
				g_vars->sceneIntro_needBlackout = 0;
				_stillRunning = 0;
				_introFlags &= 0xfd;
			} else {
				g_nmi->sceneFade(g_nmi->_currentScene, false);
				_stillRunning = 0;
				_introFlags &= 0xfd;
			}
		} else if (_stillRunning) {
			g_nmi->_currentScene->draw();
		}
	}
}

void ModalIntroDemo::finish() {
	g_nmi->_currentScene = g_nmi->accessScene(SC_INTRO1);
	g_nmi->_gameLoader->preloadScene(SC_INTRO1, TrubaDown);

	if (g_nmi->_currentScene)
		g_nmi->_gameLoader->updateSystems(42);
}

static bool checkSkipVideo(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_SPACE:
			return true;
		default:
			return false;
		}
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		return true;
	default:
		return false;
	}
}

void ModalVideoPlayer::play(const char *filename) {
	if (!_decoder.loadFile(filename))
		return;

	uint16 x = (g_system->getWidth() - _decoder.getWidth()) / 2;
	uint16 y = (g_system->getHeight() - _decoder.getHeight()) / 2;

	_decoder.start();

	while (!g_nmi->shouldQuit() && !_decoder.endOfVideo()) {
		if (_decoder.needsUpdate()) {
			const Graphics::Surface *frame = _decoder.decodeNextFrame();
			if (frame) {
				Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> tmpFrame;
				if (frame->format != g_system->getScreenFormat()) {
					tmpFrame.reset(frame->convertTo(g_system->getScreenFormat()));
					frame = tmpFrame.get();
				}
				g_nmi->_system->copyRectToScreen(frame->getPixels(), frame->pitch,
					x, y, frame->w, frame->h);

				if (_decoder.hasDirtyPalette())
					g_nmi->_system->getPaletteManager()->setPalette(_decoder.getPalette(), 0, 256);

				g_nmi->_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_nmi->_system->getEventManager()->pollEvent(event)) {
			if (checkSkipVideo(event)) {
				goto finish;
			}
		}

		g_nmi->_system->delayMillis(_decoder.getTimeToNextFrame());
	}

finish:
	_decoder.close();
}

ModalMap::ModalMap() {
	_mapScene = nullptr;
	_pic = nullptr;
	_picI03 = nullptr;
	_highlightedPic = nullptr;
	_isRunning = false;
	_rect1 = g_nmi->_sceneRect;
	_x = g_nmi->_currentScene->_x;
	_y = g_nmi->_currentScene->_y;
	_flag = 0;
	_mouseX = 0;
	_mouseY = 0;
	_dragX = 0;
	_dragY = 0;
	_hotSpotDelay = 12;
	_rect2.top = 0;
	_rect2.left = 0;
	_rect2.bottom = 600;
	_rect2.right = 800;
}

ModalMap::~ModalMap() {
	g_nmi->_gameLoader->unloadScene(SC_MAP);

	g_nmi->_sceneRect = _rect1;

	g_nmi->_currentScene->_x = _x;
	g_nmi->_currentScene->_y = _y;
}

bool ModalMap::init(int counterdiff) {
	if (_picI03)
		return init2(counterdiff);

	g_nmi->setCursor(PIC_CSR_ITN);

	if (_flag) {
		_rect2.left = _mouseX + _dragX - g_nmi->_mouseScreenPos.x;
		_rect2.top = _mouseY + _dragY - g_nmi->_mouseScreenPos.y;
		_rect2.right = _rect2.left + 800;
		_rect2.bottom = _rect2.top + 600;

		g_nmi->_sceneRect = _rect2;

		_mapScene->updateScrolling2();

		_rect2 = g_nmi->_sceneRect;
	}

	_hotSpotDelay--;

	if (_hotSpotDelay <= 0) {
		_hotSpotDelay = 12;

		if (_pic)
			_pic->_flags ^= 4;
	}

	return _isRunning;
}

bool ModalMap::init2(int counterdiff) {
	g_nmi->setCursor(PIC_CSR_DEFAULT);

	_dragX = (int)((double)_dragX * 0.6666666666666666);
	_dragY = (int)((double)_dragY * 0.6666666666666666);

	if (800 - g_nmi->_mouseScreenPos.x < 67) {
		g_nmi->setCursor(PIC_CSR_GOR);

		_dragX = g_nmi->_mouseScreenPos.x - 733;
		_dragY = (int)((double)_dragY * 0.6666666666666666);
	}

	if (g_nmi->_mouseScreenPos.x < 67) {
		g_nmi->setCursor(PIC_CSR_GOL);

		this->_dragX = g_nmi->_mouseScreenPos.x - 67;
		this->_dragY = (int)((double)_dragY * 0.6666666666666666);
	}

	if (g_nmi->_mouseScreenPos.y < 67) {
		g_nmi->setCursor(PIC_CSR_GOU);

		_dragX = (int)((double)_dragX * 0.6666666666666666);
		_dragY = g_nmi->_mouseScreenPos.y - 67;
	}

	if (600 - g_nmi->_mouseScreenPos.y < 87) {
		g_nmi->setCursor(PIC_CSR_GOD);

		_dragX = (int)((double)_dragX * 0.6666666666666666);
		_dragY = g_nmi->_mouseScreenPos.y - 513;
	}

	g_nmi->_sceneRect.translate(_dragX, _dragY);
	_mapScene->updateScrolling2();
	_rect2 = g_nmi->_sceneRect;

	PictureObject *hpic = getSceneHPicture(_mapScene->getPictureObjectAtPos(g_nmi->_mouseVirtX, g_nmi->_mouseVirtY));

	if (hpic != _highlightedPic) {
		if (_highlightedPic) {
			_highlightedPic->_flags &= 0xFFFB;
			_picI03->_flags &= 0xFFFB;
		}

		_highlightedPic = hpic;

		if (hpic) {
			PreloadItem pitem;

			pitem.preloadId1 = g_nmi->_currentScene->_sceneId;
			pitem.sceneId = findMapSceneId(hpic->_id);

			if (pitem.preloadId1 == pitem.sceneId || checkScenePass(&pitem)) {
				_highlightedPic->_flags |= 4;

				g_nmi->playSound(SND_CMN_070, 0);
			} else {
				const Dims d1 = _picI03->getDimensions();
				const Dims d2 = _highlightedPic->getDimensions();

				_picI03->setOXY(_highlightedPic->_ox + d2.x / 2 - d1.x / 2, _highlightedPic->_oy + d2.y / 2 - d1.y / 2);
				_picI03->_flags |= 4;
			}
		}
	}

	if (this->_highlightedPic) {
		g_nmi->setCursor(PIC_CSR_ITN);

		_hotSpotDelay--;

		if (_hotSpotDelay <= 0) {
			_hotSpotDelay = 12;

			if (_pic)
				_pic->_flags ^= 4;
		}
	}

	return _isRunning;
}

int ModalMap::findMapSceneId(int picId) {
	for (uint i = 0; i < g_nmi->_gameLoader->_preloadItems.size(); i++) {
		PreloadItem &pitem = g_nmi->_gameLoader->_preloadItems[i];

		if (pitem.preloadId1 == SC_MAP && pitem.preloadId2 == picId) {
			return pitem.sceneId;
		}
	}

	return 0;
}


void ModalMap::update() {
	g_nmi->_sceneRect = _rect2;

	_mapScene->draw();

	g_nmi->drawArcadeOverlay(1);
}

bool ModalMap::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return false;

	switch (cmd->_messageNum) {
	case 29:
		if (_picI03) {
			if (_highlightedPic)
				clickButton(_highlightedPic);

			return false;
		}

		_flag = 1;
		_mouseX = g_nmi->_mouseScreenPos.x;
		_mouseY = g_nmi->_mouseScreenPos.y;

		_dragX = _rect2.left;
		_dragY = _rect2.top;

		return false;

	case 30:
		if (_picI03)
			return false;

		_flag = 0;
		return false;

	case 36:
		if (cmd->_param != 9 && cmd->_param != 27)
			return false;

		break;

	case 107:
		break;

	default:
		return false;
	}

	_isRunning = 0;

	return true;
}

void ModalMap::initMap() {
	_isRunning = 1;

	_mapScene = g_nmi->accessScene(SC_MAP);

	if (!_mapScene)
		error("ModalMap::initMap(): error accessing scene SC_MAP");

	PictureObject *pic;

	for (int i = 0; i < 200; i++) {
		if (!(g_nmi->_mapTable[i] >> 16))
			break;

		pic = _mapScene->getPictureObjectById(g_nmi->_mapTable[i] >> 16, 0);

		if ((g_nmi->_mapTable[i] & 0xffff) == 1)
			pic->_flags |= 4;
		else
			pic->_flags &= 0xfffb;
	}

	pic = getScenePicture(g_nmi->_currentScene->_sceneId);

	if (pic) {
		const Dims dims = pic->getDimensions();
		Dims dims2;

		_rect2.left = dims.x / 2 + pic->_ox - 400;
		_rect2.top = dims.y / 2 + pic->_oy - 300;
		_rect2.right = _rect2.left + 800;
		_rect2.bottom = _rect2.top + 600;

		g_nmi->_sceneRect = _rect2;

		_mapScene->updateScrolling2();

		_pic = _mapScene->getPictureObjectById(PIC_MAP_I02, 0);
		dims2 = _pic->getDimensions();

		_pic->setOXY(pic->_ox + dims.x / 2 - dims2.x / 2, dims.y - dims2.y / 2 + pic->_oy - 24);
		_pic->_flags |= 4;

		_pic = _mapScene->getPictureObjectById(PIC_MAP_I01, 0);
		dims2 = _pic->getDimensions();

		_pic->setOXY(pic->_ox + dims.x / 2 - dims2.x / 2, dims.y - dims2.y / 2 + pic->_oy - 25);
		_pic->_flags |= 4;
	}

	_picI03 = _mapScene->getPictureObjectById(PIC_MAP_I03, 0);

	if (_picI03) {
		_picI03->_flags &= 0xFFFB;
	}

	g_system->warpMouse(400, 300);
	g_nmi->_mouseScreenPos.x = 400;
	g_nmi->_mouseScreenPos.y = 300;

	g_nmi->setArcadeOverlay(PIC_CSR_MAP);
}

void ModalMap::clickButton(PictureObject *pic) {
	if (g_nmi->_currentScene == g_nmi->_loaderScene) {
		_isRunning = 0;
		return;
	}

	PreloadItem *pitem = nullptr;

	for (uint i = 0; i < g_nmi->_gameLoader->_preloadItems.size(); i++)
		if (g_nmi->_gameLoader->_preloadItems[i].preloadId2 == SC_MAP) {
			pitem = &g_nmi->_gameLoader->_preloadItems[i];
			break;
		}

	if (!pitem) {
		PreloadItem preload;

		preload.preloadId2 = SC_MAP;
		g_nmi->_gameLoader->addPreloadItem(preload);
		pitem = &g_nmi->_gameLoader->_preloadItems[g_nmi->_gameLoader->_preloadItems.size() - 1];
	}

	PreloadItem *pitem2 = nullptr;

	for (uint i = 0; i < g_nmi->_gameLoader->_preloadItems.size(); i++)
		if (g_nmi->_gameLoader->_preloadItems[i].preloadId1 == SC_MAP &&
				g_nmi->_gameLoader->_preloadItems[i].preloadId2 == pic->_id) {
			pitem2 = &g_nmi->_gameLoader->_preloadItems[i];
			break;
		}

	if (pitem && pitem2) {
		pitem->preloadId1 = g_nmi->_currentScene->_sceneId;
		pitem->sceneId = pitem2->sceneId;
		if (pitem->sceneId == SC_30 && pitem->preloadId2 == SC_MAP) {
			pitem->param = TrubaRight;
		} else {
			pitem->param = pitem2->param;
		}


		if (pitem->preloadId1 == pitem2->sceneId) {
			_isRunning = 0;
		} else if (checkScenePass(pitem)) {
			_isRunning = 0;

			if (!g_nmi->isSaveAllowed()) {
				//g_nmi->_gameLoader->loadAndDecryptSave("savetmp.sav");
			}
			g_nmi->_gameLoader->preloadScene(pitem->preloadId1, SC_MAP);
		} else {
			g_nmi->playSound(SND_CMN_056, 0);
		}
	}
}

PictureObject *ModalMap::getScenePicture(int sceneId) {
	int picId = 0;

	switch (sceneId) {
	case SC_1:
		picId = PIC_MAP_S01;
		break;
	case SC_2:
		picId = PIC_MAP_S02;
		break;
	case SC_3:
		picId = PIC_MAP_S03;
		break;
	case SC_4:
		picId = PIC_MAP_S04;
		break;
	case SC_5:
		picId = PIC_MAP_S05;
		break;
	case SC_6:
		picId = PIC_MAP_S06;
		break;
	case SC_7:
		picId = PIC_MAP_S07;
		break;
	case SC_8:
		picId = PIC_MAP_S08;
		break;
	case SC_9:
		picId = PIC_MAP_S09;
		break;
	case SC_10:
		picId = PIC_MAP_S10;
		break;
	case SC_11:
		picId = PIC_MAP_S11;
		break;
	case SC_12:
		picId = PIC_MAP_S12;
		break;
	case SC_13:
		picId = PIC_MAP_S13;
		break;
	case SC_14:
		picId = PIC_MAP_S14;
		break;
	case SC_15:
		picId = PIC_MAP_S15;
		break;
	case SC_16:
		picId = PIC_MAP_S16;
		break;
	case SC_17:
		picId = PIC_MAP_S17;
		break;
	case SC_18:
	case SC_19:
		picId = PIC_MAP_S1819;
		break;
	case SC_20:
		picId = PIC_MAP_S20;
		break;
	case SC_21:
		picId = PIC_MAP_S21;
		break;
	case SC_22:
		picId = PIC_MAP_S22;
		break;
	case SC_23:
		picId = PIC_MAP_S23_1;
		break;
	case SC_24:
		picId = PIC_MAP_S24;
		break;
	case SC_25:
		picId = PIC_MAP_S25;
		break;
	case SC_26:
		picId = PIC_MAP_S26;
		break;
	case SC_27:
		picId = PIC_MAP_S27;
		break;
	case SC_28:
		picId = PIC_MAP_S28;
		break;
	case SC_29:
		picId = PIC_MAP_S29;
		break;
	case SC_30:
		picId = PIC_MAP_S30;
		break;
	case SC_31:
		picId = PIC_MAP_S31_1;
		break;
	case SC_32:
		picId = PIC_MAP_S32_1;
		break;
	case SC_33:
		picId = PIC_MAP_S33;
		break;
	case SC_34:
		picId = PIC_MAP_S34;
		break;
	case SC_35:
		picId = PIC_MAP_S35;
		break;
	case SC_36:
		picId = PIC_MAP_S36;
		break;
	case SC_37:
		picId = PIC_MAP_S37;
		break;
	case SC_38:
		picId = PIC_MAP_S38;
		break;
	case SC_FINAL1:
		picId = PIC_MAP_S38;
		break;
	default:
		break;
	}

	if (picId)
		return _mapScene->getPictureObjectById(picId, 0);

	error("ModalMap::getScenePicture(): Unknown scene id: %d", g_nmi->_currentScene->_sceneId);
}

PictureObject *ModalMap::getSceneHPicture(PictureObject *obj) {
	if (!obj)
		return nullptr;

	switch (obj->_id) {
	case PIC_MAP_S01:
 		return _mapScene->getPictureObjectById(PIC_MAP_H01, 0);
	case PIC_MAP_S02:
		return _mapScene->getPictureObjectById(PIC_MAP_H02, 0);
	case PIC_MAP_S03:
		return _mapScene->getPictureObjectById(PIC_MAP_H03, 0);
	case PIC_MAP_S04:
		return _mapScene->getPictureObjectById(PIC_MAP_H04, 0);
	case PIC_MAP_S05:
		return _mapScene->getPictureObjectById(PIC_MAP_H05, 0);
	case PIC_MAP_S06:
		return _mapScene->getPictureObjectById(PIC_MAP_H06, 0);
	case PIC_MAP_S07:
		return _mapScene->getPictureObjectById(PIC_MAP_H07, 0);
	case PIC_MAP_S09:
		return _mapScene->getPictureObjectById(PIC_MAP_H09, 0);
	case PIC_MAP_S08:
		return _mapScene->getPictureObjectById(PIC_MAP_H08, 0);
	case PIC_MAP_S10:
		return _mapScene->getPictureObjectById(PIC_MAP_H10, 0);
	case PIC_MAP_S11:
		return _mapScene->getPictureObjectById(PIC_MAP_H11, 0);
	case PIC_MAP_S12:
		return _mapScene->getPictureObjectById(PIC_MAP_H12, 0);
	case PIC_MAP_S13:
		return _mapScene->getPictureObjectById(PIC_MAP_H13, 0);
	case PIC_MAP_S14:
		return _mapScene->getPictureObjectById(PIC_MAP_H14, 0);
	case PIC_MAP_S15:
		return _mapScene->getPictureObjectById(PIC_MAP_H15, 0);
	case PIC_MAP_S16:
		return _mapScene->getPictureObjectById(PIC_MAP_H16, 0);
	case PIC_MAP_S17:
		return _mapScene->getPictureObjectById(PIC_MAP_H17, 0);
	case PIC_MAP_S1819:
		return _mapScene->getPictureObjectById(PIC_MAP_H18, 0);
	case PIC_MAP_S20:
		return _mapScene->getPictureObjectById(PIC_MAP_H20, 0);
	case PIC_MAP_S21:
		return _mapScene->getPictureObjectById(PIC_MAP_H21, 0);
	case PIC_MAP_S22:
		return _mapScene->getPictureObjectById(PIC_MAP_H22, 0);
	case PIC_MAP_S23_1:
	case PIC_MAP_S23_2:
		return _mapScene->getPictureObjectById(PIC_MAP_H23, 0);
	case PIC_MAP_S24:
		return _mapScene->getPictureObjectById(PIC_MAP_H24, 0);
	case PIC_MAP_S25:
		return _mapScene->getPictureObjectById(PIC_MAP_H25, 0);
	case PIC_MAP_S26:
		return _mapScene->getPictureObjectById(PIC_MAP_H26, 0);
	case PIC_MAP_S27:
		return _mapScene->getPictureObjectById(PIC_MAP_H27, 0);
	case PIC_MAP_S28:
		return _mapScene->getPictureObjectById(PIC_MAP_H28, 0);
	case PIC_MAP_S29:
		return _mapScene->getPictureObjectById(PIC_MAP_H29, 0);
	case PIC_MAP_S30:
		return _mapScene->getPictureObjectById(PIC_MAP_H30, 0);
	case PIC_MAP_S31_1:
	case PIC_MAP_S31_2:
		return _mapScene->getPictureObjectById(PIC_MAP_H31, 0);
	case PIC_MAP_S32_1:
	case PIC_MAP_S32_2:
		return _mapScene->getPictureObjectById(PIC_MAP_H32, 0);
	case PIC_MAP_S33:
		return _mapScene->getPictureObjectById(PIC_MAP_H33, 0);
	case PIC_MAP_S34:
		return _mapScene->getPictureObjectById(PIC_MAP_H34, 0);
	case PIC_MAP_S35:
		return _mapScene->getPictureObjectById(PIC_MAP_H35, 0);
	case PIC_MAP_S36:
		return _mapScene->getPictureObjectById(PIC_MAP_H36, 0);
	case PIC_MAP_S37:
		return _mapScene->getPictureObjectById(PIC_MAP_H37, 0);
	case PIC_MAP_S38:
		return _mapScene->getPictureObjectById(PIC_MAP_H38, 0);
	default:
		return nullptr;
	}
}

bool ModalMap::isSceneEnabled(int sceneId) {
	int id = getScenePicture(sceneId)->_id;

	for (int i = 0; i < 200; i++) {
		int mapPic = g_nmi->_mapTable[i] >> 16;
		if (!mapPic)
			return false;

		if (mapPic == id)
			return (g_nmi->_mapTable[i] & 0xffff) == 1;
	}

	return false;
}

bool ModalMap::checkScenePass(PreloadItem *item) {
	bool res = true;

	switch (item->preloadId1) {
	case SC_13:
		if (!isSceneEnabled(SC_14))
			res = false;
		break;

	case SC_27:
		if (item->sceneId == SC_25) {
			item->param = TrubaRight;
		} else {
			res = false;
		}
		break;

	case SC_25:
		if (g_nmi->getObjectState(sO_Board_25) != g_nmi->getObjectEnumState(sO_Board_25, sO_NearDudesStairs)) {
			res = false;
		}
		break;

	default:
		break;
	}

	switch (item->sceneId) {
	case SC_13:
		if (isSceneEnabled(SC_14)) {
			item->param = TrubaLeft;
			break;
		}
		item->param = TrubaUp;
		break;

	case SC_27:
		res = false;
		break;

	case SC_25:
		if (g_nmi->getObjectState(sO_Pool) != g_nmi->getObjectEnumState(sO_Pool, sO_Empty)) {
			if (g_nmi->getObjectState(sO_Pool) != g_nmi->getObjectEnumState(sO_Pool, sO_HalfFull))
				res = false;
		}
		break;

	case SC_29:
		if (isSceneEnabled(SC_30)) {
			item->param = TrubaLeft;
			break;
		}
		item->param = TrubaUp;
		break;

	default:
		break;
	}

	if ((item->sceneId != SC_37 && item->preloadId1 != SC_37)
		|| (g_nmi->getObjectState(sO_Jawcrucnher) != g_nmi->getObjectEnumState(sO_Jawcrucnher, sO_WithoutCarpet))) {
			return res;
	} else {
		res = false;
	}
	return res;
}

void NGIEngine::openMap() {
	if (!_modalObject) {
		ModalMap *map = new ModalMap;

		_modalObject = map;

		map->initMap();
	}
}

ModalFinal::ModalFinal() {
	_flags = 0;
	_counter = 255;
	_sfxVolume = g_nmi->_sfxVolume;
}

ModalFinal::~ModalFinal() {
	if (g_vars->sceneFinal_var01) {
		g_nmi->_gameLoader->unloadScene(SC_FINAL2);
		g_nmi->_gameLoader->unloadScene(SC_FINAL3);
		g_nmi->_gameLoader->unloadScene(SC_FINAL4);

		g_nmi->_currentScene = g_nmi->accessScene(SC_FINAL1);

		g_nmi->stopAllSounds();

		g_vars->sceneFinal_var01 = 0;
	}

	g_nmi->_sfxVolume = _sfxVolume;
}

bool ModalFinal::init(int counterdiff) {
	if (g_vars->sceneFinal_var01) {
		g_nmi->_gameLoader->updateSystems(42);

		return true;
	}

	if (_counter > 0) {
		_flags |= 2u;

		g_nmi->_gameLoader->updateSystems(42);

		return true;
	}

	unloadScenes();

	g_nmi->_modalObject = new ModalCredits();

	return true;
}

void ModalFinal::unloadScenes() {
	g_nmi->_gameLoader->unloadScene(SC_FINAL2);
	g_nmi->_gameLoader->unloadScene(SC_FINAL3);
	g_nmi->_gameLoader->unloadScene(SC_FINAL4);

	g_nmi->_currentScene = g_nmi->accessScene(SC_FINAL1);

	g_nmi->stopAllSounds();
}

bool ModalFinal::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17 && cmd->_messageNum == 36 && cmd->_param == 27) {
		g_nmi->_modalObject = new ModalMainMenu();
		g_nmi->_modalObject->_parentObj = this;

		return true;
	}

	return false;
}

void ModalFinal::update() {
	if (g_nmi->_currentScene) {
		g_nmi->_currentScene->draw();

		if (_flags & 1) {
			g_nmi->drawAlphaRectangle(0, 0, 800, 600, 0xff - _counter);

			_counter += 10;

			if (_counter >= 255) {
				_counter = 255;
				_flags &= 0xfe;
			}
		} else {
			if (!(_flags & 2))
				return;

			g_nmi->drawAlphaRectangle(0, 0, 800, 600, 0xff - _counter);
			_counter -= 10;

			if (_counter <= 0) {
				_counter = 0;
				_flags &= 0xFD;
			}
		}

		g_nmi->_sfxVolume = _counter * (_sfxVolume + 3000) / 255 - 3000;

		g_nmi->updateSoundVolume();
	}
}

ModalCredits::ModalCredits() {
	_sceneTitles = g_nmi->accessScene(SC_TITLES);

	_creditsPic = _sceneTitles->getPictureObjectById(PIC_TTL_CREDITS, 0);
	_creditsPic->_flags |= 4;

	_fadeIn = true;
	_fadeOut = false;

	const Dims dims = _creditsPic->getDimensions();

	_countdown = dims.y / 2 + 470;
	_sfxVolume = g_nmi->_sfxVolume;

	_currY = 630;
	_maxY = -1000 - dims.y;

	_currX = 400 - dims.x / 2;

	_creditsPic->setOXY(_currX, _currY);
}

ModalCredits::~ModalCredits() {
	g_nmi->_gameLoader->unloadScene(SC_TITLES);

	g_nmi->_sfxVolume = _sfxVolume;
}

bool ModalCredits::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17 && cmd->_messageNum == 36 && cmd->_param == 27) {
		_fadeIn = false;

		return true;
	}

	return false;
}

bool ModalCredits::init(int counterdiff) {
	if (_fadeIn || _fadeOut) {
		_countdown--;

		if (_countdown < 0)
			_fadeIn = false;

		_creditsPic->setOXY(_currX, _currY);

		if (_currY > _maxY)
			_currY -= 2;
	} else {
		if (_parentObj)
			return 0;

		ModalMainMenu *menu = new ModalMainMenu;

		g_nmi->_modalObject = menu;

		menu->_mfield_34 = 1;
	}

	return true;
}

void ModalCredits::update() {
	if (_fadeOut) {
		if (_fadeIn) {
			_sceneTitles->draw();

			return;
		}
	} else if (_fadeIn) {
		g_nmi->sceneFade(_sceneTitles, true);
		_fadeOut = 1;

		return;
	}

	if (_fadeOut) {
		g_nmi->sceneFade(_sceneTitles, false);
		_fadeOut = 0;
		return;
	}

	_sceneTitles->draw();
}

ModalMainMenu::ModalMainMenu() {
	_lastArea = nullptr;
	_hoverAreaId = 0;
	_mfield_34 = 0;
	_scene = g_nmi->accessScene(SC_MAINMENU);
	_debugKeyCount = 0;
	_sliderOffset = 0;
	_screct.left = g_nmi->_sceneRect.left;
	_screct.top = g_nmi->_sceneRect.top;
	_screct.right = g_nmi->_sceneRect.right;
	_screct.bottom = g_nmi->_sceneRect.bottom;

	if (g_nmi->_currentScene) {
		_bgX = g_nmi->_currentScene->_x;
		_bgY = g_nmi->_currentScene->_y;
	} else {
		_bgX = 0;
		_bgY = 0;
	}

	g_nmi->_sceneRect.top = 0;
	g_nmi->_sceneRect.left = 0;
	g_nmi->_sceneRect.right = 800;
	g_nmi->_sceneRect.bottom = 600;

	MenuArea *area;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_EXIT_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_CONTINUE_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	if (isSaveAllowed()) {
		_areas.push_back(MenuArea());
		area = &_areas.back();
		area->picIdL = PIC_MNU_SAVE_L;
		area->picObjD = nullptr;
		area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
		area->picObjL->_flags &= 0xFFFB;
	}

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_LOAD_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_RESTART_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_AUTHORS_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_SLIDER_L;
	area->picObjD = _scene->getPictureObjectById(PIC_MNU_SLIDER_D, 0);
	area->picObjD->_flags |= 4;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_menuSliderIdx = _areas.size() - 1;

	_areas.push_back(MenuArea());
	area = &_areas.back();
	area->picIdL = PIC_MNU_MUSICSLIDER_L;
	area->picObjD = _scene->getPictureObjectById(PIC_MNU_MUSICSLIDER_D, 0);
	area->picObjD->_flags |= 4;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	_musicSliderIdx = _areas.size() - 1;

	if (g_nmi->_mainMenu_debugEnabled)
		enableDebugMenuButton();

	setSliderPos();
}

void ModalMainMenu::update() {
	_scene->draw();
}

bool ModalMainMenu::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	if (!_scene)
		return false;

	Common::Point point;

	if (message->_messageNum == 29) {
		point.x = message->_x;
		point.y = message->_y;

		int numarea = checkHover(point);

		if (numarea >= 0) {
			if (numarea == _menuSliderIdx) {
				_lastArea = &_areas[_menuSliderIdx];
				_sliderOffset = _lastArea->picObjL->_ox - point.x;

				return false;
			}

			if (numarea == _musicSliderIdx) {
				_lastArea = &_areas[_musicSliderIdx];
				_sliderOffset = _lastArea->picObjL->_ox - point.x;

				return false;
			}

			_hoverAreaId = _areas[numarea].picIdL;
		}

		return false;
	}

	if (message->_messageNum == 30) {
		if (_lastArea)
			_lastArea = nullptr;

		return false;
	}

	if (message->_messageNum != 36)
		return false;

	if (message->_param == 27)
		_hoverAreaId = PIC_MNU_CONTINUE_L;
	else
		enableDebugMenu(message->_param);

	return false;
}

bool ModalMainMenu::init(int counterdiff) {
	switch (_hoverAreaId) {
	case PIC_MNU_RESTART_L:
		g_nmi->restartGame();

		if (this == g_nmi->_modalObject)
			return false;

		delete this;
		break;

	case PIC_MNU_EXIT_L:
		{
			ModalQuery *mq = new ModalQuery();

			g_nmi->_modalObject = mq;

			mq->_parentObj = this;
			mq->create(_scene, _scene, PIC_MEX_BGR);

			_hoverAreaId = 0;

			return true;
		}

	case PIC_MNU_DEBUG_L:
		g_nmi->_gameLoader->unloadScene(SC_MAINMENU);
		_scene = nullptr;
		g_nmi->_sceneRect = _screct;

		if (!g_nmi->_currentScene)
			error("ModalMainMenu::init: Bad state");

		g_nmi->_currentScene->_x = _bgX;
		g_nmi->_currentScene->_y = _bgY;

		g_nmi->_gameLoader->preloadScene(g_nmi->_currentScene->_sceneId, SC_DBGMENU);

		return false;

	case PIC_MNU_CONTINUE_L:
		if (!_mfield_34) {
			g_nmi->_gameLoader->unloadScene(SC_MAINMENU);
			_areas.clear();
			_scene = nullptr;
			g_nmi->_sceneRect = _screct;

			if (g_nmi->_currentScene) {
				g_nmi->_currentScene->_x = _bgX;
				g_nmi->_currentScene->_y = _bgY;
			}

			return false;
		}

		g_nmi->restartGame();

		if (this == g_nmi->_modalObject)
			return false;

		delete this;
		break;

	case PIC_MNU_AUTHORS_L:
		g_nmi->_modalObject = new ModalCredits();
		g_nmi->_modalObject->_parentObj = this;

		_hoverAreaId = 0;

		return true;

	case PIC_MNU_SAVE_L:
	case PIC_MNU_LOAD_L:
		{
			ModalSaveGame *sg = new ModalSaveGame();

			g_nmi->_modalObject = sg;
			g_nmi->_modalObject->_parentObj = _parentObj;

			int mode = 0;
			if (_hoverAreaId == PIC_MNU_SAVE_L)
				mode = 1;

			sg->setup(g_nmi->accessScene(SC_MAINMENU), mode);
			sg->setScene(g_nmi->accessScene(SC_MAINMENU));

			sg->_rect = _screct;
			sg->_oldBgX = _bgX;
			sg->_oldBgY = _bgY;

			delete this;
		}

		break;

	default:
		if (_lastArea) {
			updateSliderPos();
		} else {
			g_nmi->_cursorId = PIC_CSR_DEFAULT;

			int idx = checkHover(g_nmi->_mouseScreenPos);

			if (idx < 0)
				goto LABEL_40;

			g_nmi->_cursorId = PIC_CSR_DEFAULT;

			if (idx != this->_menuSliderIdx && idx != this->_musicSliderIdx )
				goto LABEL_40;
		}

		g_nmi->_cursorId = PIC_CSR_LIFT;

	LABEL_40:
		g_nmi->setCursor(g_nmi->_cursorId);

		updateVolume();

		return true;
	}

	return true;
}

void ModalMainMenu::updateVolume() {
	if (g_nmi->_soundEnabled) {
		for (int s = 0; s < g_nmi->_currSoundListCount; s++)
			for (int i = 0; i < g_nmi->_currSoundList1[s]->getCount(); i++) {
				updateSoundVolume(g_nmi->_currSoundList1[s]->getSoundByIndex(i));
			}
	}
}

void ModalMainMenu::updateSoundVolume(Sound &snd) {
	if (!snd._objectId)
		return;

	StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(snd._objectId, -1);
	if (!ani)
		return;

	int a, b;

	if (ani->_ox >= _screct.left) {
		int par, pan;

		if (ani->_ox <= _screct.right) {
			int dx;

			if (ani->_oy <= _screct.bottom) {
				if (ani->_oy >= _screct.top) {
					snd.setPanAndVolume(g_nmi->_sfxVolume, 0);

					return;
				}
				dx = _screct.top - ani->_oy;
			} else {
				dx = ani->_oy - _screct.bottom;
			}

			par = 0;

			if (dx > 800) {
				snd.setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			a = g_nmi->_sfxVolume - (-3500);
			b = 800 - dx;
		} else {
			int dx = ani->_ox - _screct.right;

			if (dx > 800) {
				snd.setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			par = dx * (-3500) / -800;
			a = g_nmi->_sfxVolume - (-3500);
			b = 800 - dx;
		}

		int32 pp = b * a;

		snd.setPanAndVolume(pan + pp / 800, par);

		return;
	}

	int dx = _screct.left - ani->_ox;
	if (dx <= 800) {
		int32 s = (800 - dx) * (g_nmi->_sfxVolume - (-3500));
		int32 p = -3500 + s / 800;

		if (p > g_nmi->_sfxVolume)
			p = g_nmi->_sfxVolume;

		snd.setPanAndVolume(p, dx * (-3500) / 800);
	} else {
		snd.setPanAndVolume(-3500, 0);
	}
}

void ModalMainMenu::updateSliderPos() {
	if (_lastArea->picIdL == PIC_MNU_SLIDER_L) {
		int x = g_nmi->_mouseScreenPos.x + _sliderOffset;

		if (x >= 65) {
			if (x > 238)
				x = 238;
		} else {
			x = 65;
		}

		_lastArea->picObjD->setOXY(x, _lastArea->picObjD->_oy);
		_lastArea->picObjL->setOXY(x, _lastArea->picObjD->_oy);

		int vol = 1000 * (3 * x - 195);
		g_nmi->_sfxVolume = vol / 173 - 3000;

		if (!(vol / 173))
			g_nmi->_sfxVolume = -10000;

		g_nmi->updateSoundVolume();
	} else if (_lastArea->picIdL == PIC_MNU_MUSICSLIDER_L) {
		int x = g_nmi->_mouseScreenPos.x + _sliderOffset;

		if (x >= 65) {
			if (x > 238)
				x = 238;
		} else {
			x = 65;
		}

		_lastArea->picObjD->setOXY(x, _lastArea->picObjD->_oy);
		_lastArea->picObjL->setOXY(x, _lastArea->picObjD->_oy);

		g_nmi->setMusicVolume(255 * (x - 65) / 173);
	}
}

int ModalMainMenu::checkHover(Common::Point &point) {
	for (uint i = 0; i < _areas.size(); i++) {
		if (_areas[i].picObjL->isPixelHitAtPos(point.x, point.y)) {
			_areas[i].picObjL->_flags |= 4;

			return i;
		} else {
			_areas[i].picObjL->_flags &= 0xFFFB;
		}
	}

	if (isOverArea(_areas[_menuSliderIdx].picObjL, &point)) {
		_areas[_menuSliderIdx].picObjL->_flags |= 4;

		return _menuSliderIdx;
	}

	if (isOverArea(_areas[_musicSliderIdx].picObjL, &point)) {
		_areas[_musicSliderIdx].picObjL->_flags |= 4;

		return _musicSliderIdx;
	}

	return -1;
}

bool ModalMainMenu::isOverArea(PictureObject *obj, Common::Point *point) {
	const Dims dims = obj->getDimensions();

	int left = point->x - 8;
	int right = point->x + 12;
	int down = point->y - 11;
	int up = point->y + 9;

	if (left >= obj->_ox && right < obj->_ox + dims.x && down >= obj->_oy && up < obj->_oy + dims.y)
		return true;

	return false;
}

bool ModalMainMenu::isSaveAllowed() {
	if (!g_nmi->_isSaveAllowed)
		return false;

	if (g_nmi->_aniMan->_flags & 0x100)
		return false;

	for (Common::Array<MessageQueue *>::iterator s = g_nmi->_globalMessageQueueList->begin(); s != g_nmi->_globalMessageQueueList->end(); ++s) {
		if (!(*s)->_isFinished && ((*s)->getFlags() & 1))
			return false;
	}

	return true;
}

void ModalMainMenu::enableDebugMenu(char c) {
	const char deb[] = "debuger";

	if (c == deb[_debugKeyCount]) {
		_debugKeyCount++;

		if (deb[_debugKeyCount] )
			return;

		enableDebugMenuButton();
	}

	_debugKeyCount = 0;
}

void ModalMainMenu::enableDebugMenuButton() {
	for (uint i = 0; i < _areas.size(); i++)
		if (_areas[i].picIdL == PIC_MNU_DEBUG_L)
			return;

	_areas.push_back(MenuArea());
	MenuArea *area = &_areas.back();
	area->picIdL = PIC_MNU_DEBUG_L;
	area->picObjD = nullptr;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;

	g_nmi->_mainMenu_debugEnabled = true;
}

void ModalMainMenu::setSliderPos() {
	int x = 173 * (g_nmi->_sfxVolume + 3000) / 3000 + 65;
	PictureObject *obj = _areas[_menuSliderIdx].picObjD;

	if (x >= 65) {
		if (x > 238)
			x = 238;
	} else {
		x = 65;
	}

	obj->setOXY(x, obj->_oy);
	_areas[_menuSliderIdx].picObjL->setOXY(x, obj->_oy);

	x = 173 * g_nmi->_musicVolume / 255 + 65;
	obj = _areas[_musicSliderIdx].picObjD;

	if (x >= 65) {
		if (x > 238)
			x = 238;
	} else {
		x = 65;
	}

	obj->setOXY(x, obj->_oy);
	_areas[_musicSliderIdx].picObjL->setOXY(x, obj->_oy);
}

ModalHelp::ModalHelp() {
	_mainMenuScene = nullptr;
	_bg = nullptr;
	_isRunning = false;
	_rect = g_nmi->_sceneRect;
	_hx = g_nmi->_currentScene->_x;
	_hy = g_nmi->_currentScene->_y;

	g_nmi->_sceneRect.left = 0;
	g_nmi->_sceneRect.bottom = 600;
	g_nmi->_sceneRect.top = 0;
	g_nmi->_sceneRect.right = 800;
}

ModalHelp::~ModalHelp() {
	g_nmi->_gameLoader->unloadScene(SC_MAINMENU);

	g_nmi->_sceneRect = _rect;

	g_nmi->_currentScene->_x = _hx;
	g_nmi->_currentScene->_y = _hy;
}

bool ModalHelp::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17) {
		int msg = cmd->_messageNum;

		if (msg == 29 || msg == 36 || msg == 107) {
			_isRunning = 0;

			return true;
		}
	}

	return false;
}

bool ModalHelp::init(int counterdiff) {
	g_nmi->setCursor(PIC_CSR_DEFAULT);

	return _isRunning;
}

void ModalHelp::update() {
	g_nmi->_sceneRect.left = 0;
	g_nmi->_sceneRect.top = 0;
	g_nmi->_sceneRect.right = 800;
	g_nmi->_sceneRect.bottom = 600;

	_bg->draw(0, 0, 0, 0);
}

void ModalHelp::launch() {
	_mainMenuScene = g_nmi->accessScene(SC_MAINMENU);

	if (_mainMenuScene) {
		if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS)
			_bg = _mainMenuScene->getPictureObjectById(364, 0)->_picture;
		else
			_bg = _mainMenuScene->getPictureObjectById(PIC_HLP_BGR, 0)->_picture;
		_isRunning = 1;
	}
}

ModalQuery::ModalQuery() {
	_bgScene = nullptr;
	_bg = nullptr;
	_okBtn = nullptr;
	_cancelBtn = nullptr;
	_queryResult = -1;
}

ModalQuery::~ModalQuery() {
	_bg->_flags &= 0xFFFB;
	_cancelBtn->_flags &= 0xFFFB;
	_okBtn->_flags &= 0xFFFB;
}

bool ModalQuery::create(Scene *sc, Scene *bgScene, int id) {
	if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS) {
		_bg = sc->getPictureObjectById(386, 0);

		if (!_bg)
			return false;

		_okBtn = sc->getPictureObjectById(392, 0);

		if (!_okBtn)
			return false;

		_cancelBtn = sc->getPictureObjectById(396, 0);

		if (!_cancelBtn)
			return 0;

		_queryResult = -1;
		_bgScene = bgScene;

		return true;
	}

	if (id == PIC_MEX_BGR) {
		_bg = sc->getPictureObjectById(PIC_MEX_BGR, 0);

		if (!_bg)
			return false;

		_okBtn = sc->getPictureObjectById(PIC_MEX_OK, 0);

		if (!_okBtn)
			return false;

		_cancelBtn = sc->getPictureObjectById(PIC_MEX_CANCEL, 0);

		if (!_cancelBtn)
			return false;
	} else {
		if (id != PIC_MOV_BGR)
			return false;

		_bg = sc->getPictureObjectById(PIC_MOV_BGR, 0);

		if (!_bg)
			return false;

		_okBtn = sc->getPictureObjectById(PIC_MOV_OK, 0);

		if (!_okBtn)
			return false;

		_cancelBtn = sc->getPictureObjectById(PIC_MOV_CANCEL, 0);

		if (!_cancelBtn)
			return false;
	}

	_queryResult = -1;
	_bgScene = bgScene;

	return true;
}

void ModalQuery::update() {
	if (_bgScene)
		_bgScene->draw();

	_bg->draw();

	if (_okBtn->_flags & 4)
		_okBtn->draw();

	if (_cancelBtn->_flags & 4)
		_cancelBtn->draw();
}

bool ModalQuery::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17) {
		if (cmd->_messageNum == 29) {
			if (_okBtn->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y)) {
				_queryResult = 1;

				return false;
			}

			if (_cancelBtn->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
				_queryResult = 0;
		} else if (cmd->_messageNum == 36 && cmd->_param == 27) {
			_queryResult = 0;

			return false;
		}
	}

	return false;
}

bool ModalQuery::init(int counterdiff) {
	if (_okBtn->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
		_okBtn->_flags |= 4;
	else
		_okBtn->_flags &= 0xFFFB;

	if (_cancelBtn->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
		_cancelBtn->_flags |= 4;
	else
		_cancelBtn->_flags &= 0xFFFB;

	if (_queryResult == -1) {
		return true;
	} else {
		if (g_nmi->isDemo() && g_nmi->getLanguage() == Common::RU_RUS) {
			if (!_queryResult)
				return false;

			ModalDemo *demo = new ModalDemo;
			demo->launch();

			g_nmi->_modalObject = demo;

			return true;
		}

		if (_bg->_id == PIC_MEX_BGR) {
			_cancelBtn->_flags &= 0xFFFB;
			_okBtn->_flags &= 0xFFFB;

			if (_queryResult == 1) {
				if (_bgScene)
					g_nmi->sceneFade(_bgScene, false);

				g_nmi->_gameContinue = false;

				return false;
			}
		}
	}

	return false;
}

ModalSaveGame::ModalSaveGame() {
	_oldBgX = 0;
	_oldBgY = 0;

	_bgr = nullptr;
	_okD = nullptr;
	_okL = nullptr;
	_cancelD = nullptr;
	_cancelL = nullptr;
	_emptyD = nullptr;
	_emptyL = nullptr;
	_fullD = nullptr;
	_fullL = nullptr;
	_menuScene = nullptr;
	_queryRes = -1;
	_rect = g_nmi->_sceneRect;
	_queryDlg = nullptr;
	_mode = 1;

	_objtype = kObjTypeModalSaveGame;
}

ModalSaveGame::~ModalSaveGame() {
	g_nmi->_sceneRect = _rect;
}

void ModalSaveGame::setScene(Scene *sc) {
	_queryRes = -1;
	_menuScene = sc;
}

void ModalSaveGame::processKey(int key) {
	if (key == 27)
		_queryRes = 0;
}

bool ModalSaveGame::init(int counterdiff) {
	if (_queryDlg) {
		if (!_queryDlg->init(counterdiff)) {
			if (!_queryDlg->getQueryResult())
				_queryRes = -1;

			delete _queryDlg;
			_queryDlg = nullptr;
		}

		return true;
	}

	if (_queryRes == -1)
		return true;

	g_nmi->_sceneRect = _rect;

	if (g_nmi->_currentScene) {
		g_nmi->_currentScene->_x = _oldBgX;
		g_nmi->_currentScene->_y = _oldBgY;
	}

	if (!_queryRes) {
		ModalMainMenu *m = new ModalMainMenu;

		g_nmi->_modalObject = m;

		m->_parentObj = _parentObj;
		m->_screct = _rect;
		m->_bgX = _oldBgX;
		m->_bgY = _oldBgY;

		delete this;

		return true;
	}

	return false;
}

void ModalSaveGame::setup(Scene *sc, int mode) {
	_files.clear();
	_arrayL.clear();
	_arrayD.clear();
	_mode = mode;

	if (mode) {
		_bgr = sc->getPictureObjectById(PIC_MSV_BGR, 0);
		_cancelD = sc->getPictureObjectById(PIC_MSV_CANCEL_D, 0);
		_cancelL = sc->getPictureObjectById(PIC_MSV_CANCEL_L, 0);
		_okD = sc->getPictureObjectById(PIC_MSV_OK_D, 0);
		_okL = sc->getPictureObjectById(PIC_MSV_OK_L, 0);
		_emptyD = sc->getPictureObjectById(PIC_MSV_EMPTY_D, 0);
		_emptyL = sc->getPictureObjectById(PIC_MSV_EMPTY_L, 0);
	} else {
		_bgr = sc->getPictureObjectById(PIC_MLD_BGR, 0);
		_cancelD = sc->getPictureObjectById(PIC_MLD_CANCEL_D, 0);
		_cancelL = sc->getPictureObjectById(PIC_MLD_CANCEL_L, 0);
		_okD = sc->getPictureObjectById(PIC_MLD_OK_D, 0);
		_okL = sc->getPictureObjectById(PIC_MLD_OK_L, 0);
		_emptyD = sc->getPictureObjectById(PIC_MSV_EMPTY_D, 0);
		_emptyL = sc->getPictureObjectById(PIC_MSV_EMPTY_D, 0);
	}

	_fullD = sc->getPictureObjectById(PIC_MSV_FULL_D, 0);
	_fullL = sc->getPictureObjectById(PIC_MSV_FULL_L, 0);
	_queryRes = -1;

	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_0_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_0_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_1_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_1_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_2_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_2_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_3_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_3_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_4_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_4_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_5_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_5_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_6_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_6_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_7_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_7_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_8_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_8_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_9_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_9_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_DOTS_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_DOTS_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_DOT_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_DOT_L, 0));
	_arrayL.push_back(sc->getPictureObjectById(PIC_MSV_SPACE_D, 0));
	_arrayD.push_back(sc->getPictureObjectById(PIC_MSV_SPACE_L, 0));

	int x = _bgr->_ox + _bgr->getDimensions().x / 2;
	int y = _bgr->_oy + 90;
	int w;

	_files.clear();
	_files.resize(7);
	for (int i = 0; i < 7; i++) {
		FileInfo &fileinfo = _files[i];

		Common::strlcpy(fileinfo.filename, getSavegameFile(i), sizeof(fileinfo.filename));

		if (!getFileInfo(i, &fileinfo)) {
			fileinfo.empty = true;
			w = _emptyD->getDimensions().x;
		} else {
			w = 0;

			for (uint j = 0; j < _arrayL.size(); j++) {
				w += _arrayL[j]->getDimensions().x + 2;
			}
		}

		fileinfo.fx1 = x - w / 2;
		fileinfo.fx2 = x + w / 2;
		fileinfo.fy1 = y;
		fileinfo.fy2 = y + _emptyD->getDimensions().y;
		y = fileinfo.fy2 + 3;
	}
}

char *ModalSaveGame::getSaveName() {
	if (_queryRes < 0)
		return nullptr;

	return _files[_queryRes - 1].filename;
}

bool ModalSaveGame::getFileInfo(int slot, FileInfo *fileinfo) {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(
		NGI::getSavegameFile(slot)));

	if (!f)
		return false;

	NGI::FullpipeSavegameHeader header;
	if (!NGI::readSavegameHeader(f.get(), header))
		return false;

	// Create the return descriptor
	SaveStateDescriptor desc(g_nmi->getMetaEngine(), slot, header.description);
	char res[17];

	NGI::parseSavegameHeader(header, desc);

	snprintf(res, sizeof(res), "%s %s", desc.getSaveDate().c_str(), desc.getSaveTime().c_str());

	for (int i = 0; i < 16; i++) {
		switch (res[i]) {
		case '-':
		case '.':
			fileinfo->date[i] = 11;
			break;
		case ' ':
			fileinfo->date[i] = 12;
			break;
		case ':':
			fileinfo->date[i] = 10;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			fileinfo->date[i] = res[i] - '0';
			break;
		default:
			error("Incorrect date format: %s", res);
		}
	}

	return true;
}

void ModalSaveGame::update() {
	if (_menuScene)
		_menuScene->draw();

	_bgr->draw();

	if (_queryDlg) {
		_queryDlg->update();

		return;
	}

	g_nmi->_cursorId = PIC_CSR_DEFAULT;

	g_nmi->setCursor(g_nmi->_cursorId);

	for (uint i = 0; i < _files.size(); i++) {
		if (g_nmi->_mouseScreenPos.x < _files[i].fx1 || g_nmi->_mouseScreenPos.x > _files[i].fx2 ||
			g_nmi->_mouseScreenPos.y < _files[i].fy1 || g_nmi->_mouseScreenPos.y > _files[i].fy2 ) {
			if (_files[i].empty) {
				_emptyD->setOXY(_files[i].fx1, _files[i].fy1);
				_emptyD->draw();
			} else {
				int x = _files[i].fx1;

				for (int j = 0; j < 16; j++) {
					_arrayL[_files[i].date[j]]->setOXY(x + 1, _files[i].fy1);
					_arrayL[_files[i].date[j]]->draw();

					x += _arrayL[_files[i].date[j]]->getDimensions().x + 2;
				}
			}
		} else {
			if (_files[i].empty) {
				_emptyL->setOXY(_files[i].fx1, _files[i].fy1);
				_emptyL->draw();
			} else {
				int x = _files[i].fx1;

				for (int j = 0; j < 16; j++) {
					_arrayD[_files[i].date[j]]->setOXY(x + 1, _files[i].fy1);
					_arrayD[_files[i].date[j]]->draw();

					x += _arrayD[_files[i].date[j]]->getDimensions().x + 2;
				}
			}
		}
	}
	if (_cancelL->isPixelHitAtPos(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
		_cancelL->draw();
	else if (_okL->isPixelHitAtPos(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
		_okL->draw();
}

bool ModalSaveGame::handleMessage(ExCommand *cmd) {
	if (_queryDlg)
		return _queryDlg->handleMessage(cmd);

	if (cmd->_messageNum == 29)
		processMouse(cmd->_x, cmd->_y);
	else if (cmd->_messageNum == 36)
		processKey(cmd->_param);

	return false;
}

void ModalSaveGame::processMouse(int x, int y) {
	for (uint i = 0; i < _files.size(); i++) {
		if (x >= _files[i].fx1 && x <= _files[i].fx2 && y >= _files[i].fy1 && y <= _files[i].fy2) {
			_queryRes = i + 1;

			if (_mode) {
				if (!_files[i].empty) {
					_queryDlg = new ModalQuery;

					_queryDlg->create(_menuScene, nullptr, PIC_MOV_BGR);
				}
			}

			return;
		}
	}

	if (_cancelL->isPixelHitAtPos(x, y))
		_queryRes = 0;
}

void ModalSaveGame::saveload() {
	if (_objtype != kObjTypeModalSaveGame)
		return;

	if (_mode) {
		if (getSaveName()) {
			bool allowed = true;

			for (Common::Array<MessageQueue *>::iterator s = g_nmi->_globalMessageQueueList->begin(); s != g_nmi->_globalMessageQueueList->end(); ++s) {
				if (!(*s)->_isFinished && ((*s)->getFlags() & 1))
					allowed = false;
			}

			if (g_nmi->_isSaveAllowed && allowed)
				g_nmi->_gameLoader->writeSavegame(g_nmi->_currentScene, getSaveName(), "");
		}
	} else {
		if (getSaveName()) {
			if (_parentObj) {
				delete _parentObj;

				_parentObj = nullptr;
			}

			g_nmi->stopAllSoundStreams();
			g_nmi->stopSoundStream2();

			g_nmi->_gameLoader->readSavegame(getSaveName());
		}
	}
}

ModalDemo::ModalDemo() {
	_bg = nullptr;
	_button = nullptr;
	_text = nullptr;

	if (g_nmi->getLanguage() == Common::RU_RUS) {
		_clickedQuit = 0;
		_countdown = -10;
	} else {
		_clickedQuit = -1;
		_countdown = 1000;
	}
	_scene = nullptr;
}

ModalDemo::~ModalDemo() {
	if (_bg)
		_bg->_flags &= 0xFFFB;

	_button->_flags &= 0xFFFB;
	_text->_flags &= 0xFFFB;
}

bool ModalDemo::launch() {
	Scene *sc = g_nmi->accessScene(SC_MAINMENU);

	if (g_nmi->getLanguage() == Common::RU_RUS) {
		_scene = sc;

		for (uint i = 1; i < sc->_picObjList.size(); i++) {
			if (sc->_picObjList[i]->_id == 399)
				sc->_picObjList[i]->_flags |= 4;
			else
				sc->_picObjList[i]->_flags &= 0xFFFB;
		}

		_button = sc->getPictureObjectById(443, 0);
		_text = sc->getPictureObjectById(402, 0);

		_countdown = -10;

		return true;
	}

	_bg = sc->getPictureObjectById(PIC_POST_BGR, 0);

	if (!_bg)
		return false;

	_button = sc->getPictureObjectById(PIC_POST_BUTTON, 0);
	_text = sc->getPictureObjectById(PIC_POST_TEXT, 0);

	_clickedQuit = -1;

	// fadeout
	warning("STUB: ModelDemo: fadeout");
	update();

	g_nmi->stopAllSoundStreams();
	g_nmi->stopAllSounds();
	g_nmi->playSound(SND_CMN_056, 0);
	g_nmi->playSound(SND_CMN_069, 1);

	return true;
}

bool ModalDemo::init(int counterDiff) {
	if (g_nmi->getLanguage() == Common::RU_RUS)
		return init2(counterDiff);

	g_nmi->_cursorId = PIC_CSR_DEFAULT;

	if (_button->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y)) {
		if (!(_button->_flags & 4))
			g_nmi->playSound(SND_CMN_070, 0);

		_button->_flags |= 4;

		g_nmi->_cursorId = PIC_CSR_ITN;
	} else {
		_button->_flags &= 0xFFFB;
	}

	g_nmi->setCursor(g_nmi->_cursorId);

	_countdown -= counterDiff;

	if (_countdown <= 0)
		_countdown = 1000;

	if (_clickedQuit == -1)
		return true;

	g_system->openUrl("http://www.amazon.de/EuroVideo-Bildprogramm-GmbH-Full-Pipe/dp/B003TO51YE/ref=sr_1_1");

	g_nmi->_gameContinue = false;

	return false;
}

bool ModalDemo::init2(int counterDiff) {
	if (_clickedQuit) {
		g_system->openUrl("http://pipestudio.ru/fullpipe/");

		g_nmi->_gameContinue = false;

		return false;
	}

	if (_countdown > 0) {
		_countdown--;
	} else {
		_text->_flags ^= 4;
		_countdown = 24;
	}

	if (_button->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y)) {
		_button->_flags |= 4;

		g_nmi->_cursorId = PIC_CSR_ITN;
	} else {
		_button->_flags &= 0xFFFB;

		g_nmi->_cursorId = PIC_CSR_DEFAULT;
	}

	return true;
}

void ModalDemo::update() {
	if (g_nmi->getLanguage() == Common::RU_RUS) {
		if (_countdown == -10)
			g_nmi->sceneFade(_scene, true);

		_scene->draw();

		return;
	}

	_bg->draw();

	if (_button->_flags & 4)
		_button->draw();

	if (_text->_flags & 4)
		_text->draw();
}

bool ModalDemo::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return false;

	if (cmd->_messageNum == 29) {
		if (_button->isPointInside(g_nmi->_mouseScreenPos.x, g_nmi->_mouseScreenPos.y))
			_clickedQuit = 1;
	} else if (cmd->_messageNum == 36 && (cmd->_param == 27 || g_nmi->getLanguage() == Common::RU_RUS)) {
		_clickedQuit = 1;
	}

	return false;
}

void NGIEngine::openHelp() {
	if (!_modalObject) {
		ModalHelp *help = new ModalHelp;

		_modalObject = help;

		help->launch();
	}
}

void NGIEngine::openMainMenu() {
	if (isDemo() && getLanguage() == Common::RU_RUS) {
		ModalQuery *q = new ModalQuery;

		Scene *sc = accessScene(SC_MAINMENU);

		q->create(sc, nullptr, 0);

		g_nmi->_modalObject = q;

		return;
	}
	ModalMainMenu *menu = new ModalMainMenu;

	menu->_parentObj = g_nmi->_modalObject;

	g_nmi->_modalObject = menu;
}

} // End of namespace NGI

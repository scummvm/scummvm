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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"
#include "fullpipe/messages.h"
#include "fullpipe/constants.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/statics.h"
#include "fullpipe/modal.h"

#include "fullpipe/constants.h"

#include "graphics/palette.h"
#include "video/avi_decoder.h"

#include "engines/savestate.h"

namespace Fullpipe {

ModalIntro::ModalIntro() {
	_field_8 = 0;
	_countDown = 0;
	_stillRunning = 0;

	if (g_vars->sceneIntro_skipIntro) {
		_introFlags = 4;
	} else {
		_introFlags = 33;
		_countDown = 150;

		PictureObject *pict = g_fp->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0);
		pict->setFlags(pict->_flags & 0xFFFB);
	}

	g_vars->sceneIntro_skipIntro = false;
	_sfxVolume = g_fp->_sfxVolume;
}

ModalIntro::~ModalIntro() {
	g_fp->stopAllSounds();
	g_fp->_sfxVolume = _sfxVolume;
}

bool ModalIntro::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	if (message->_messageNum != 36)
		return false;

	if (message->_keyCode != 13 && message->_keyCode != 27 && message->_keyCode != 32)
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
			g_fp->_gameLoader->updateSystems(42);

		_introFlags |= 2;

		return true;
	}

	if (_introFlags & 4) {
		ModalVideoPlayer *player = new ModalVideoPlayer();

		g_fp->_modalObject = player;
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

		g_fp->_modalObject = player;
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
		g_fp->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0)->_flags &= 0xfffb;
	}

	if (!(_introFlags & 0x20)) {
		if (_introFlags & 0x10) {
			if (!_stillRunning) {
				_introFlags |= 1;

				g_fp->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0)->_flags &= 0xfffb;
				g_fp->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_GAMETITLE, 0)->_flags &= 0xfffb;

				chainQueue(QU_INTR_STARTINTRO, 1);
			}
			g_fp->_gameLoader->updateSystems(42);
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

		g_fp->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_GAMETITLE, 0)->_flags &= 0xfffb;

		_stillRunning = 0;
	}

	return true;
}

void ModalIntro::update() {
	warning("STUB: ModalIntro::update()");

	if (g_fp->_currentScene) {
		if (_introFlags & 1) {
			//sceneFade(virt, g_currentScene, 1);
			_stillRunning = 255;
			_introFlags &= 0xfe;

			if (_introFlags & 0x20)
				g_fp->playSound(SND_INTR_019, 0);
		} else if (_introFlags & 2) {
			if (g_vars->sceneIntro_needBlackout) {
				//vrtRectangle(*(_DWORD *)virt, 0, 0, 0, 800, 600);
				g_vars->sceneIntro_needBlackout = 0;
				_stillRunning = 0;
				_introFlags &= 0xfd;
			} else {
				//sceneFade(virt, g_currentScene, 0);
				_stillRunning = 0;
				_introFlags &= 0xfd;
			}
		} else if (_stillRunning) {
			g_fp->_currentScene->draw();
		}
	}
}

void ModalIntro::finish() {
	g_fp->_gameLoader->unloadScene(SC_INTRO2);
	g_fp->_currentScene = g_fp->accessScene(SC_INTRO1);
	g_fp->_gameLoader->preloadScene(SC_INTRO1, TrubaDown);

	if (g_fp->_currentScene)
		g_fp->_gameLoader->updateSystems(42);
}

void ModalVideoPlayer::play(const char *filename) {
	// TODO: Videos are encoded using Intel Indeo 5 (IV50), which isn't supported yet

	Video::AVIDecoder *aviDecoder = new Video::AVIDecoder();

	if (!aviDecoder->loadFile(filename))
		return;

	uint16 x = (g_system->getWidth() - aviDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - aviDecoder->getHeight()) / 2;
	bool skipVideo = false;

	aviDecoder->start();

	while (!g_fp->shouldQuit() && !aviDecoder->endOfVideo() && !skipVideo) {
		if (aviDecoder->needsUpdate()) {
			const Graphics::Surface *frame = aviDecoder->decodeNextFrame();
			if (frame) {
				g_fp->_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);

				if (aviDecoder->hasDirtyPalette())
					g_fp->_system->getPaletteManager()->setPalette(aviDecoder->getPalette(), 0, 256);

				g_fp->_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_fp->_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && (event.kbd.keycode == Common::KEYCODE_ESCAPE ||
														 event.kbd.keycode == Common::KEYCODE_RETURN ||
														 event.kbd.keycode == Common::KEYCODE_SPACE))
				 || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_fp->_system->delayMillis(aviDecoder->getTimeToNextFrame());
	}
}

ModalMap::ModalMap() {
	_mapScene = 0;
	_pic = 0;
	_isRunning = false;
	_rect1 = g_fp->_sceneRect;
	_x = g_fp->_currentScene->_x;
	_y = g_fp->_currentScene->_y;
	_flag = 0;
	_mouseX = 0;
	_mouseY = 0;
	_field_38 = 0;
	_field_3C = 0;
	_field_40 = 12;
	_rect2.top = 0;
	_rect2.left = 0;
	_rect2.bottom = 600;
	_rect2.right = 800;
}

ModalMap::~ModalMap() {
	g_fp->_gameLoader->unloadScene(SC_MAP);

	g_fp->_sceneRect = _rect1;

	g_fp->_currentScene->_x = _x;
	g_fp->_currentScene->_y = _y;
}

bool ModalMap::init(int counterdiff) {
	g_fp->setCursor(PIC_CSR_ITN);

	if (_flag) {
		_rect2.left = _mouseX + _field_38 - g_fp->_mouseScreenPos.x;
		_rect2.top = _mouseY + _field_3C - g_fp->_mouseScreenPos.y;;
		_rect2.right = _rect2.left + 800;
		_rect2.bottom = _rect2.top + 600;

		g_fp->_sceneRect =_rect2;

		_mapScene->updateScrolling2();

		_rect2 = g_fp->_sceneRect;
	}

	_field_40--;

	if (_field_40 <= 0) {
		_field_40 = 12;

		if (_pic)
			_pic->_flags ^= 4;
	}

	return _isRunning;
}

void ModalMap::update() {
	g_fp->_sceneRect = _rect2;

	_mapScene->draw();

	g_fp->drawArcadeOverlay(1);
}

bool ModalMap::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return false;

	switch (cmd->_messageNum) {
	case 29:
		_flag = 1;
		_mouseX = g_fp->_mouseScreenPos.x;
		_mouseY = g_fp->_mouseScreenPos.x;

		_field_3C = _rect2.top;
		_field_38 = _rect2.left;

		break;

	case 30:
		_flag = 0;
		break;

	case 36:
		if (cmd->_keyCode != 9 && cmd->_keyCode != 27 )
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

	_mapScene = g_fp->accessScene(SC_MAP);

	if (!_mapScene)
		error("ModalMap::initMap(): error accessing scene SC_MAP");

	PictureObject *pic;

	for (int i = 0; i < 200; i++) {
		if (!(g_fp->_mapTable[i] >> 16))
			break;

		pic = _mapScene->getPictureObjectById(g_fp->_mapTable[i] >> 16, 0);

		if ((g_fp->_mapTable[i] & 0xffff) == 1)
			pic->_flags |= 4;
		else
			pic->_flags &= 0xfffb;
	}

	pic = getScenePicture();

	Common::Point point;
	Common::Point point2;

	if (pic) {
		pic->getDimensions(&point);

		_rect2.left = point.x / 2 + pic->_ox - 400;
		_rect2.top = point.y / 2 + pic->_oy - 300;
		_rect2.right = _rect2.left + 800;
		_rect2.bottom = _rect2.top + 600;

		_mapScene->updateScrolling2();

		_pic = _mapScene->getPictureObjectById(PIC_MAP_I02, 0);
		_pic->getDimensions(&point2);

		_pic->setOXY(pic->_ox + point.x / 2 - point2.x / 2, point.y - point2.y / 2 + pic->_oy - 24);
		_pic->_flags |= 4;

		_pic = _mapScene->getPictureObjectById(PIC_MAP_I01, 0);
		_pic->getDimensions(&point2);

		_pic->setOXY(pic->_ox + point.x / 2 - point2.x / 2, point.y - point2.y / 2 + pic->_oy - 25);
		_pic->_flags |= 4;
	}

	g_fp->setArcadeOverlay(PIC_CSR_MAP);
}

PictureObject *ModalMap::getScenePicture() {
	int picId = 0;

	switch (g_fp->_currentScene->_sceneId) {
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
	}

	if (picId)
		return _mapScene->getPictureObjectById(picId, 0);

	error("ModalMap::getScenePicture(): Unknown scene id: %d", g_fp->_currentScene->_sceneId);
}

void FullpipeEngine::openMap() {
	if (!_modalObject) {
		ModalMap *map = new ModalMap;

		_modalObject = map;

		map->initMap();
	}
}

ModalFinal::ModalFinal() {
	_flags = 0;
	_counter = 255;
	_sfxVolume = g_fp->_sfxVolume;
}

ModalFinal::~ModalFinal() {
	if (g_vars->sceneFinal_var01) {
		g_fp->_gameLoader->unloadScene(SC_FINAL2);
		g_fp->_gameLoader->unloadScene(SC_FINAL3);
		g_fp->_gameLoader->unloadScene(SC_FINAL4);

		g_fp->_currentScene = g_fp->accessScene(SC_FINAL1);

		g_fp->stopAllSounds();

		g_vars->sceneFinal_var01 = 0;
	}

	g_fp->_sfxVolume = _sfxVolume;
}

bool ModalFinal::init(int counterdiff) {
	if (g_vars->sceneFinal_var01) {
		g_fp->_gameLoader->updateSystems(42);

		return true;
	}

	if (_counter > 0) {
		_flags |= 2u;

		g_fp->_gameLoader->updateSystems(42);

		return true;
	}

	unloadScenes();

	g_fp->_modalObject = new ModalCredits();

	return true;
}

void ModalFinal::unloadScenes() {
	g_fp->_gameLoader->unloadScene(SC_FINAL2);
	g_fp->_gameLoader->unloadScene(SC_FINAL3);
	g_fp->_gameLoader->unloadScene(SC_FINAL4);

	g_fp->_currentScene = g_fp->accessScene(SC_FINAL1);

	g_fp->stopAllSounds();
}

bool ModalFinal::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17 && cmd->_messageNum == 36 && cmd->_keyCode == 27) {
		g_fp->_modalObject = new ModalMainMenu();
		g_fp->_modalObject->_parentObj = this;

		return true;
	}

	return false;
}

void ModalFinal::update() {
	if (g_fp->_currentScene) {
		g_fp->_currentScene->draw();

		if (_flags & 1) {
			g_fp->drawAlphaRectangle(0, 0, 800, 600, 0xff - _counter);

			_counter += 10;

			if (_counter >= 255) {
				_counter = 255;
				_flags &= 0xfe;
			}
		} else {
			if (!(_flags & 2))
				return;

			g_fp->drawAlphaRectangle(0, 0, 800, 600, 0xff - _counter);
			_counter -= 10;

			if (_counter <= 0) {
				_counter = 0;
				_flags &= 0xFD;
			}
		}

		g_fp->_sfxVolume = _counter * (_sfxVolume + 3000) / 255 - 3000;

		g_fp->updateSoundVolume();
	}
}

ModalCredits::ModalCredits() {
	Common::Point point;

	_sceneTitles = g_fp->accessScene(SC_TITLES);

	_creditsPic = _sceneTitles->getPictureObjectById(PIC_TTL_CREDITS, 0);
	_creditsPic->_flags |= 4;

	_fadeIn = true;
	_fadeOut = false;

	_creditsPic->getDimensions(&point);

	_countdown = point.y / 2 + 470;
	_sfxVolume = g_fp->_sfxVolume;

	_currY = 630;
	_maxY = -1000 - point.y;

	_currX = 400 - point.x / 2;

	_creditsPic->setOXY(_currX, _currY);
}

ModalCredits::~ModalCredits() {
	g_fp->_gameLoader->unloadScene(SC_TITLES);

	g_fp->_sfxVolume = _sfxVolume;
}

bool ModalCredits::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17 && cmd->_messageNum == 36 && cmd->_keyCode == 27) {
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

		g_fp->_modalObject = menu;

		menu->_mfield_34 = 1;
	}

	return true;
}

void ModalCredits::update() {
	warning("STUB: ModalCredits::update()");

	if (_fadeOut) {
		if (_fadeIn) {
			_sceneTitles->draw();

			return;
		}
	} else if (_fadeIn) {
		//sceneFade(virt, this->_sceneTitles, 1); // TODO
		_fadeOut = 1;

		return;
	}

	if (_fadeOut) {
		//sceneFade(virt, this->_sceneTitles, 0); // TODO
		_fadeOut = 0;
		return;
	}

	_sceneTitles->draw();
}

ModalMainMenu::ModalMainMenu() {
	_areas.clear();

	_lastArea = 0;
	_hoverAreaId = 0;
	_mfield_34 = 0;
	_scene = g_fp->accessScene(SC_MAINMENU);
	_debugKeyCount = 0;
	_sliderOffset = 0;
	_screct.left = g_fp->_sceneRect.left;
	_screct.top = g_fp->_sceneRect.top;
	_screct.right = g_fp->_sceneRect.right;
	_screct.bottom = g_fp->_sceneRect.bottom;

	if (g_fp->_currentScene) {
		_bgX = g_fp->_currentScene->_x;
		_bgY = g_fp->_currentScene->_y;
	} else {
		_bgX = 0;
		_bgY = 0;
	}

	g_fp->_sceneRect.top = 0;
	g_fp->_sceneRect.left = 0;
	g_fp->_sceneRect.right = 800;
	g_fp->_sceneRect.bottom = 600;

	MenuArea *area;

	area = new MenuArea();
	area->picIdL = PIC_MNU_EXIT_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);

	area = new MenuArea();
	area->picIdL = PIC_MNU_CONTINUE_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);

	if (isSaveAllowed()) {
		area = new MenuArea();
		area->picIdL = PIC_MNU_SAVE_L;
		area->picObjD = 0;
		area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
		area->picObjL->_flags &= 0xFFFB;
		_areas.push_back(area);
	}

	area = new MenuArea();
	area->picIdL = PIC_MNU_LOAD_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);

	area = new MenuArea();
	area->picIdL = PIC_MNU_RESTART_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);

	area = new MenuArea();
	area->picIdL = PIC_MNU_AUTHORS_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);

	area = new MenuArea();
	area->picIdL = PIC_MNU_SLIDER_L;
	area->picObjD = _scene->getPictureObjectById(PIC_MNU_SLIDER_D, 0);
	area->picObjD->_flags |= 4;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);
	_menuSliderIdx = _areas.size() - 1;

	area = new MenuArea();
	area->picIdL = PIC_MNU_MUSICSLIDER_L;
	area->picObjD = _scene->getPictureObjectById(PIC_MNU_MUSICSLIDER_D, 0);
	area->picObjD->_flags |= 4;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);
	_musicSliderIdx = _areas.size() - 1;

	if (g_fp->_mainMenu_debugEnabled)
		enableDebugMenuButton();

	setSliderPos();
}

void ModalMainMenu::update() {
	_scene->draw();
}

bool ModalMainMenu::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	Common::Point point;

	if (message->_messageNum == 29) {
		point.x = message->_x;
		point.y = message->_y;

		int numarea = checkHover(point);

		if (numarea >= 0) {
			if (numarea == _menuSliderIdx) {
				_lastArea = _areas[_menuSliderIdx];
				_sliderOffset = _lastArea->picObjL->_ox - point.x;

				return false;
			}

			if (numarea == _musicSliderIdx) {
				_lastArea = _areas[_musicSliderIdx];
				_sliderOffset = _lastArea->picObjL->_ox - point.x;

				return false;
			}

			_hoverAreaId = _areas[numarea]->picIdL;
		}

		return false;
	}

	if (message->_messageNum == 30) {
		if (_lastArea)
			_lastArea = 0;

		return false;
	}

	if (message->_messageNum != 36)
		return false;

	if (message->_keyCode == 27)
		_hoverAreaId = PIC_MNU_CONTINUE_L;
	else
		enableDebugMenu(message->_keyCode);

	return false;
}

bool ModalMainMenu::init(int counterdiff) {
	switch (_hoverAreaId) {
	case PIC_MNU_RESTART_L:
		g_fp->restartGame();

		if (this == g_fp->_modalObject)
			return false;

		delete this;
		break;

	case PIC_MNU_EXIT_L:
		{
			ModalQuery *mq = new ModalQuery();

			g_fp->_modalObject = mq;

			mq->_parentObj = this;
			mq->create(_scene, (PictureObject *)_scene->_picObjList[0], PIC_MEX_BGR);

			_hoverAreaId = 0;

			return true;
		}

	case PIC_MNU_DEBUG_L:
		g_fp->_gameLoader->unloadScene(SC_MAINMENU);
		g_fp->_sceneRect = _screct;

		if (!g_fp->_currentScene)
			error("ModalMainMenu::init: Bad state");

		g_fp->_currentScene->_x = _bgX;
		g_fp->_currentScene->_y = _bgY;

		g_fp->_gameLoader->preloadScene(g_fp->_currentScene->_sceneId, SC_DBGMENU);

		return false;

	case PIC_MNU_CONTINUE_L:
		if (!_mfield_34) {
			g_fp->_gameLoader->unloadScene(SC_MAINMENU);
			g_fp->_sceneRect = _screct;

			if (g_fp->_currentScene) {
				g_fp->_currentScene->_x = _bgX;
				g_fp->_currentScene->_y = _bgY;
			}

			return false;
		}

		g_fp->restartGame();

		if (this == g_fp->_modalObject)
			return false;

		delete this;
		break;

	case PIC_MNU_AUTHORS_L:
		g_fp->_modalObject = new ModalCredits();
		g_fp->_modalObject->_parentObj = this;

		_hoverAreaId = 0;

		return true;

	case PIC_MNU_SAVE_L:
	case PIC_MNU_LOAD_L:
		{
			ModalSaveGame *sg = new ModalSaveGame();

			g_fp->_modalObject = sg;
			g_fp->_modalObject->_parentObj = _parentObj;

			int mode = 0;
			if (_hoverAreaId == PIC_MNU_SAVE_L)
				mode = 1;

			sg->setup(g_fp->accessScene(SC_MAINMENU), mode);
			sg->setScene(g_fp->accessScene(SC_MAINMENU));

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
			g_fp->_cursorId = PIC_CSR_DEFAULT;

			int idx = checkHover(g_fp->_mouseScreenPos);

			if (idx < 0)
				goto LABEL_40;

			g_fp->_cursorId = PIC_CSR_DEFAULT;

			if (idx != this->_menuSliderIdx && idx != this->_musicSliderIdx )
				goto LABEL_40;
		}

		g_fp->_cursorId = PIC_CSR_LIFT;

	LABEL_40:
		g_fp->setCursor(g_fp->_cursorId);

		updateVolume();

		return true;
	}

	return true;
}

void ModalMainMenu::updateVolume() {
	if (g_fp->_soundEnabled ) {
		for (int s = 0; s < g_fp->_currSoundListCount; s++)
			for (int i = 0; i < g_fp->_currSoundList1[s]->getCount(); i++) {
				updateSoundVolume(g_fp->_currSoundList1[s]->getSoundByIndex(i));
			}
	}
}

void ModalMainMenu::updateSoundVolume(Sound *snd) {
	if (!snd->_objectId)
		return;

	StaticANIObject *ani = g_fp->_currentScene->getStaticANIObject1ById(snd->_objectId, -1);
	if (!ani)
		return;

	int a, b;

	if (ani->_ox >= _screct.left) {
		int par, pan;

		if (ani->_ox <= _screct.right) {
			int dx;

			if (ani->_oy <= _screct.bottom) {
				if (ani->_oy >= _screct.top) {
					snd->setPanAndVolume(g_fp->_sfxVolume, 0);

					return;
				}
				dx = _screct.top - ani->_oy;
			} else {
				dx = ani->_oy - _screct.bottom;
			}

		    par = 0;

			if (dx > 800) {
				snd->setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			a = g_fp->_sfxVolume - (-3500);
			b = 800 - dx;
		} else {
			int dx = ani->_ox - _screct.right;

			if (dx > 800) {
				snd->setPanAndVolume(-3500, 0);
				return;
			}

			pan = -3500;
			par = dx * (-3500) / -800;
			a = g_fp->_sfxVolume - (-3500);
			b = 800 - dx;
		}

		int32 pp = b * a;

		snd->setPanAndVolume(pan + pp / 800, par);

		return;
	}

	int dx = _screct.left - ani->_ox;
	if (dx <= 800) {
		int32 s = (800 - dx) * (g_fp->_sfxVolume - (-3500));
		int32 p = -3500 + s / 800;

		if (p > g_fp->_sfxVolume)
			p = g_fp->_sfxVolume;

		snd->setPanAndVolume(p, dx * (-3500) / 800);
	} else {
		snd->setPanAndVolume(-3500, 0);
	}
}

void ModalMainMenu::updateSliderPos() {
	if (_lastArea->picIdL == PIC_MNU_SLIDER_L) {
		int x = g_fp->_mouseScreenPos.x + _sliderOffset;

		if (x >= 65) {
			if (x > 238)
				x = 238;
		} else {
			x = 65;
		}

		_lastArea->picObjD->setOXY(x, _lastArea->picObjD->_oy);
		_lastArea->picObjL->setOXY(x, _lastArea->picObjD->_oy);

		int vol = 1000 * (3 * x - 195);
		g_fp->_sfxVolume = vol / 173 - 3000;

		if (!(vol / 173))
			g_fp->_sfxVolume = -10000;

		g_fp->updateSoundVolume();
	} else if (_lastArea->picIdL == PIC_MNU_MUSICSLIDER_L) {
		int x = g_fp->_mouseScreenPos.x + _sliderOffset;

		if (x >= 65) {
			if (x > 238)
				x = 238;
		} else {
			x = 65;
		}

		_lastArea->picObjD->setOXY(x, _lastArea->picObjD->_oy);
		_lastArea->picObjL->setOXY(x, _lastArea->picObjD->_oy);

		g_fp->setMusicVolume(255 * (x - 65) / 173);
	}
}

int ModalMainMenu::checkHover(Common::Point &point) {
	for (uint i = 0; i < _areas.size(); i++) {
		if (_areas[i]->picObjL->isPixelHitAtPos(point.x, point.y)) {
			_areas[i]->picObjL->_flags |= 4;

			return i;
		} else {
			_areas[i]->picObjL->_flags &= 0xFFFB;
		}
	}

	if (isOverArea(_areas[_menuSliderIdx]->picObjL, &point)) {
		_areas[_menuSliderIdx]->picObjL->_flags |= 4;

		return _menuSliderIdx;
	}

	if (isOverArea(_areas[_musicSliderIdx]->picObjL, &point)) {
		_areas[_musicSliderIdx]->picObjL->_flags |= 4;

		return _musicSliderIdx;
	}

	return -1;
}

bool ModalMainMenu::isOverArea(PictureObject *obj, Common::Point *point) {
	Common::Point p;

	obj->getDimensions(&p);

	int left = point->x - 8;
	int right = point->x + 12;
	int down = point->y - 11;
	int up = point->y + 9;

	if (left >= obj->_ox && right < obj->_ox + p.x && down >= obj->_oy && up < obj->_oy + p.y)
		return true;

	return false;
}

bool ModalMainMenu::isSaveAllowed() {
	if (!g_fp->_isSaveAllowed)
		return false;

	if (g_fp->_aniMan->_flags & 0x100)
		return false;

	for (Common::Array<MessageQueue *>::iterator s = g_fp->_globalMessageQueueList->begin(); s != g_fp->_globalMessageQueueList->end(); ++s) {
		if (!(*s)->_isFinished && ((*s)->getFlags() & 1))
			return false;
	}

	return true;
}

void ModalMainMenu::enableDebugMenu(char c) {
	const char deb[] = "DEBUGER";

	if (c == deb[_debugKeyCount]) {
		_debugKeyCount++;

		if (deb[_debugKeyCount] )
			return;

		enableDebugMenuButton();
	}

	_debugKeyCount = 0;
}

void ModalMainMenu::enableDebugMenuButton() {
	MenuArea *area;

	for (uint i = 0; i < _areas.size(); i++)
		if (_areas[i]->picIdL == PIC_MNU_DEBUG_L)
			return;

	area = new MenuArea();
	area->picIdL = PIC_MNU_DEBUG_L;
	area->picObjD = 0;
	area->picObjL = _scene->getPictureObjectById(area->picIdL, 0);
	area->picObjL->_flags &= 0xFFFB;
	_areas.push_back(area);
}

void ModalMainMenu::setSliderPos() {
	int x = 173 * (g_fp->_sfxVolume + 3000) / 3000 + 65;
	PictureObject *obj = _areas[_menuSliderIdx]->picObjD;

	if (x >= 65) {
		if (x > 238)
			x = 238;
	} else {
		x = 65;
	}

	obj->setOXY(x, obj->_oy);
	_areas[_menuSliderIdx]->picObjL->setOXY(x, obj->_oy);

	x = 173 * g_fp->_musicVolume / 255 + 65;
	obj = _areas[_musicSliderIdx]->picObjD;

	if (x >= 65) {
		if (x > 238)
			x = 238;
	} else {
		x = 65;
	}

	obj->setOXY(x, obj->_oy);
	_areas[_musicSliderIdx]->picObjL->setOXY(x, obj->_oy);
}

ModalHelp::ModalHelp() {
	_mainMenuScene = 0;
	_bg = 0;
	_isRunning = false;
	_rect = g_fp->_sceneRect;
	_hx = g_fp->_currentScene->_x;
	_hy = g_fp->_currentScene->_y;

	g_fp->_sceneRect.left = 0;
	g_fp->_sceneRect.bottom = 600;
	g_fp->_sceneRect.top = 0;
	g_fp->_sceneRect.right = 800;
}

ModalHelp::~ModalHelp() {
	g_fp->_gameLoader->unloadScene(SC_MAINMENU);

	g_fp->_sceneRect = _rect;

	g_fp->_currentScene->_x = _hx;
	g_fp->_currentScene->_y = _hy;
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
	g_fp->setCursor(PIC_CSR_DEFAULT);

	return _isRunning;
}

void ModalHelp::update() {
	g_fp->_sceneRect.left = 0;
	g_fp->_sceneRect.top = 0;
	g_fp->_sceneRect.right = 800;
	g_fp->_sceneRect.bottom = 600;

	_bg->draw(0, 0, 0, 0);
}

void ModalHelp::launch() {
	_mainMenuScene = g_fp->accessScene(SC_MAINMENU);

	if (_mainMenuScene) {
		_bg = _mainMenuScene->getPictureObjectById(PIC_HLP_BGR, 0)->_picture;
		_isRunning = 1;
	}
}

ModalQuery::ModalQuery() {
	_picObjList = 0;
	_bg = 0;
	_okBtn = 0;
	_cancelBtn = 0;
	_queryResult = -1;
}

ModalQuery::~ModalQuery() {
	_bg->_flags &= 0xFFFB;
	_cancelBtn->_flags &= 0xFFFB;
	_okBtn->_flags &= 0xFFFB;
}

bool ModalQuery::create(Scene *sc, PictureObject *picObjList, int id) {
	if (id == PIC_MEX_BGR) {
		_bg = sc->getPictureObjectById(PIC_MEX_BGR, 0);

		if (!_bg)
			return false;

		_okBtn = sc->getPictureObjectById(PIC_MEX_OK, 0);

		if (!_okBtn)
			return false;

		_cancelBtn = sc->getPictureObjectById(PIC_MEX_CANCEL, 0);

		if (!_cancelBtn)
			return 0;
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
	_picObjList = picObjList;

	return true;
}

void ModalQuery::update() {
	if (_picObjList)
		_picObjList->draw();

	_bg->draw();

	if (_okBtn->_flags & 4)
		_okBtn->draw();

	if (_cancelBtn->_flags & 4)
		_cancelBtn->draw();
}

bool ModalQuery::handleMessage(ExCommand *cmd) {
	if (cmd->_messageKind == 17) {
		if (cmd->_messageNum == 29) {
			if (_okBtn->isPointInside(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y)) {
				_queryResult = 1;

				return false;
			}

			if (_cancelBtn->isPointInside(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y))
				_queryResult = 0;
		} else if (cmd->_messageNum == 36 && cmd->_keyCode == 27) {
			_queryResult = 0;

			return false;
		}
	}

	return false;
}

bool ModalQuery::init(int counterdiff) {
	if (_okBtn->isPointInside(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y))
		_okBtn->_flags |= 4;
	else
		_okBtn->_flags &= 0xFFFB;

	if (_cancelBtn->isPointInside(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y))
		_cancelBtn->_flags |= 4;
	else
		_cancelBtn->_flags &= 0xFFFB;

	if (_queryResult == -1) {
		return true;
	} else {
		if (_bg->_id == PIC_MEX_BGR) {
			_cancelBtn->_flags &= 0xFFFB;
			_okBtn->_flags &= 0xFFFB;

			if (_queryResult == 1) {
				warning("STUB: ModalQuery::init()");
				//sceneFade(g_vrtDrawHandle, (Scene *)this->_picObjList, 0);

				//if (inputArFlag) {
				//	g_needRestart = 1;
				//	return 0;
				//}
				//SendMessageA(hwndCallback, WM_DESTROY, 0, 0);
			}
		}
	}

	return false;
}

ModalSaveGame::ModalSaveGame() {
	_oldBgX = 0;
	_oldBgY = 0;

	_bgr = 0;
	_okD = 0;
	_okL = 0;
	_cancelD = 0;
	_cancelL = 0;
	_emptyD = 0;
	_emptyL = 0;
	_fullD = 0;
	_fullL = 0;
	_menuScene = 0;
	_queryRes = -1;
	_rect = g_fp->_sceneRect;
	_queryDlg = 0;
	_mode = 1;

	_objtype = kObjTypeModalSaveGame;
}

ModalSaveGame::~ModalSaveGame() {
	g_fp->_sceneRect = _rect;

	_arrayD.clear();
	_arrayL.clear();

	for (uint i = 0; i < _files.size(); i++)
		free(_files[i]);

	_files.clear();
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
			_queryDlg = 0;
		}

		return true;
	}

	if (_queryRes == -1)
		return true;

	g_fp->_sceneRect = _rect;

	if (g_fp->_currentScene) {
		g_fp->_currentScene->_x = _oldBgX;
		g_fp->_currentScene->_y = _oldBgY;
	}

	if (!_queryRes) {
		ModalMainMenu *m = new ModalMainMenu;

		g_fp->_modalObject = m;

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

	Common::Point point;

	int x = _bgr->_ox + _bgr->getDimensions(&point)->x / 2;
	int y = _bgr->_oy + 90;
	int w;
	FileInfo *fileinfo;

	for (int i = 0; i < 7; i++) {
		fileinfo = new FileInfo;
		memset(fileinfo, 0, sizeof(FileInfo));

		Common::strlcpy(fileinfo->filename, getSavegameFile(i), 160);

		if (!getFileInfo(i, fileinfo)) {
			fileinfo->empty = true;
			w = _emptyD->getDimensions(&point)->x;
		} else {
			w = 0;

			for (int j = 0; j < 16; j++) {
				_arrayL[j]->getDimensions(&point);
				w += point.x + 2;
			}
		}

		fileinfo->fx1 = x - w / 2;
		fileinfo->fx2 = x + w / 2;
		fileinfo->fy1 = y;
		fileinfo->fy2 = y + _emptyD->getDimensions(&point)->y;

		_files.push_back(fileinfo);

		y = fileinfo->fy2 + 3;
	}
}

char *ModalSaveGame::getSaveName() {
	if (_queryRes < 0)
		return 0;

	return _files[_queryRes]->filename;
}

bool ModalSaveGame::getFileInfo(int slot, FileInfo *fileinfo) {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Fullpipe::getSavegameFile(slot));

	if (!f)
		return false;

	Fullpipe::FullpipeSavegameHeader header;
	Fullpipe::readSavegameHeader(f, header);
	delete f;

	// Create the return descriptor
	SaveStateDescriptor desc(slot, header.saveName);
	char res[17];

	snprintf(res, 17, "%s  %s", desc.getSaveDate().c_str(), desc.getSaveTime().c_str());

	for (int i = 0; i < 16; i++) {
		switch(res[i]) {
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

	g_fp->_cursorId = PIC_CSR_DEFAULT;

	g_fp->setCursor(g_fp->_cursorId);

	Common::Point point;

	for (uint i = 0; i < _files.size(); i++) {
		if (g_fp->_mouseScreenPos.x < _files[i]->fx1 || g_fp->_mouseScreenPos.x > _files[i]->fx2 ||
			g_fp->_mouseScreenPos.y < _files[i]->fy1 || g_fp->_mouseScreenPos.y > _files[i]->fy2 ) {
			if (_files[i]->empty) {
				_emptyD->setOXY(_files[i]->fx1, _files[i]->fy1);
				_emptyD->draw();
			} else {
				int x = _files[i]->fx1;

				for (int j = 0; j < 16; j++) {
					_arrayL[_files[i]->date[j]]->setOXY(x + 1, _files[i]->fy1);
					_arrayL[_files[i]->date[j]]->draw();

					x += _arrayL[_files[i]->date[j]]->getDimensions(&point)->x + 2;
				}
			}
		} else {
			if (_files[i]->empty) {
				_emptyL->setOXY(_files[i]->fx1, _files[i]->fy1);
				_emptyL->draw();
			} else {
				int x = _files[i]->fx1;

				for (int j = 0; j < 16; j++) {
					_arrayD[_files[i]->date[j]]->setOXY(x + 1, _files[i]->fy1);
					_arrayD[_files[i]->date[j]]->draw();

					x += _arrayD[_files[i]->date[j]]->getDimensions(&point)->x + 2;
				}
			}
		}
	}
	if (_cancelL->isPixelHitAtPos(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y))
		_cancelL->draw();
	else if (_okL->isPixelHitAtPos(g_fp->_mouseScreenPos.x, g_fp->_mouseScreenPos.y))
		_okL->draw();
}

bool ModalSaveGame::handleMessage(ExCommand *cmd) {
	if (_queryDlg)
		return _queryDlg->handleMessage(cmd);

	if (cmd->_messageNum == 29)
		processMouse(cmd->_x, cmd->_y);
	else if (cmd->_messageNum == 36)
		processKey(cmd->_keyCode);

	return false;
}

void ModalSaveGame::processMouse(int x, int y) {
	for (uint i = 0; i < _files.size(); i++) {
		if (x >= _files[i]->fx1 && x <= _files[i]->fx2 && y >= _files[i]->fy1 && y <= _files[i]->fy2) {
			_queryRes = i + 1;

			if (_mode) {
				if (!_files[i]->empty) {
					_queryDlg = new ModalQuery;

					_queryDlg->create(_menuScene, 0, PIC_MOV_BGR);
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

			for (Common::Array<MessageQueue *>::iterator s = g_fp->_globalMessageQueueList->begin(); s != g_fp->_globalMessageQueueList->end(); ++s) {
				if (!(*s)->_isFinished && ((*s)->getFlags() & 1))
					allowed = false;
			}

			if (g_fp->_isSaveAllowed && allowed)
				g_fp->_gameLoader->writeSavegame(g_fp->_currentScene, getSaveName());
		}
	} else {
		if (getSaveName()) {
			if (_parentObj) {
				delete _parentObj;

				_parentObj = 0;
			}

			g_fp->stopAllSoundStreams();
			g_fp->stopSoundStream2();

			g_fp->_gameLoader->readSavegame(getSaveName());
		}
	}
}

void FullpipeEngine::openHelp() {
	if (!_modalObject) {
		ModalHelp *help = new ModalHelp;

		_modalObject = help;

		help->launch();
	}
}

void FullpipeEngine::openMainMenu() {
	ModalMainMenu *menu = new ModalMainMenu;

	menu->_parentObj = g_fp->_modalObject;

	g_fp->_modalObject = menu;
}

} // End of namespace Fullpipe

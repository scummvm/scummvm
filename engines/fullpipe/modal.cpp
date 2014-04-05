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
#include "fullpipe/modal.h"
#include "fullpipe/messages.h"
#include "fullpipe/constants.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/gameloader.h"

#include "fullpipe/constants.h"

#include "graphics/palette.h"
#include "video/avi_decoder.h"

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

		menu->_field_34 = 1;
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
	warning("STUB: ModalMainMenu::ModalMainMenu()");

	_field_34 = 0;
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

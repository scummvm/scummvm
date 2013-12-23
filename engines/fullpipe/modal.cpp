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

#include "fullpipe/fullpipe.h"
#include "fullpipe/modal.h"
#include "fullpipe/messages.h"
#include "fullpipe/constants.h"
#include "fullpipe/motion.h"
#include "fullpipe/scenes.h"
#include "fullpipe/gameloader.h"

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

void ModalVideoPlayer::play(const char *fname) {
	warning("STUB: ModalVideoPlayer::play(%s)", fname);
}

void FullpipeEngine::openMap() {
	warning("STUB: FullpipeEngine::openMap()");
}

void FullpipeEngine::openHelp() {
	warning("STUB: FullpipeEngine::openHelp()");
}

void FullpipeEngine::openMainMenu() {
	warning("STUB: FullpipeEngine::openMainMenu()");
}

} // End of namespace Fullpipe

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
#include "fullpipe/scenes.h"

namespace Fullpipe {

bool BaseModalObject::handleMessage(ExCommand *message) {
	warning("STUB: BaseModalObject::handleMessage()");

	return true;
}

bool BaseModalObject::init(int counterdiff) {
	warning("STUB: BaseModalObject::init(%d)", counterdiff);

	return true;
}

bool BaseModalObject::update() {
	warning("STUB: BaseModalObject::update()");

	return true;
}

void BaseModalObject::saveload() {
	warning("STUB: BaseModalObject::saveload()");
}


ModalIntro::ModalIntro() {
	_field_8 = 0;
	_countDown = 0;
	_needRedraw = 0;
	if (g_vars->sceneIntro_skipIntro) {
		_introFlags = 4;
	} else {
		_introFlags = 33;
		_countDown = 150;

		PictureObject *pict = g_fullpipe->accessScene(SC_INTRO1)->getPictureObjectById(PIC_IN1_PIPETITLE, 0);
		pict->setFlags(pict->_flags & 0xFFFB);
	}
	g_vars->sceneIntro_skipIntro = false;
	_sfxVolume = g_fullpipe->_sfxVolume;
}

bool ModalIntro::handleMessage(ExCommand *message) {
	if (message->_messageKind != 17)
		return false;

	if (message->_messageNum != 36)
		return false;

	if (message->_keyCode != 13 && message->_keyCode != 27 && message->_keyCode != 32)
		return false;

	if (_needRedraw) {
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

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

#ifdef ENABLE_SAGA2

// "Dinotopia" and "Faery Tale Adventure II: Halls of the Dead" Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/scene.h"
#include "saga/gfx.h"

#include "sound/mixer.h"
#include "graphics/surface.h"
#include "graphics/video/smk_decoder.h"

namespace Saga {

int Scene::DinoStartProc() {
	_vm->_gfx->showCursor(false);

	Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(_vm->_mixer);
	Graphics::VideoPlayer *player = new Graphics::VideoPlayer(smkDecoder);
	if (smkDecoder->loadFile("testvid.smk"))
		player->playVideo();        // Play introduction
	smkDecoder->closeFile();
	delete player;
	delete smkDecoder;

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2StartProc() {
	_vm->_gfx->showCursor(false);

	Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(_vm->_mixer);
	Graphics::VideoPlayer *player = new Graphics::VideoPlayer(smkDecoder);
	if (smkDecoder->loadFile("trimark.smk"))
		player->playVideo();      // Show Ignite logo
	smkDecoder->closeFile();
	if (smkDecoder->loadFile("intro.smk"))
		player->playVideo();        // Play introduction
	smkDecoder->closeFile();
	delete player;
	delete smkDecoder;

	// HACK: Forcibly quit here
	_vm->quitGame();

	return SUCCESS;
}

int Scene::FTA2EndProc(FTA2Endings whichEnding) {
	char videoName[20];

	switch (whichEnding) {
	case kFta2BadEndingLaw:
		strcpy(videoName, "end_1.smk");
		break;
	case kFta2BadEndingChaos:
		strcpy(videoName, "end_2.smk");
		break;
	case kFta2GoodEnding1:
		strcpy(videoName, "end_3a.smk");
		break;
	case kFta2GoodEnding2:
		strcpy(videoName, "end_3b.smk");
		break;
	case kFta2BadEndingDeath:
		strcpy(videoName, "end_4.smk");
		break;
	default:
		error("Unknown FTA2 ending");
	}

	_vm->_gfx->showCursor(false);

	// Play ending
	Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(_vm->_mixer);
	Graphics::VideoPlayer *player = new Graphics::VideoPlayer(smkDecoder);
	if (smkDecoder->loadFile(videoName)) {
		player->playVideo();
		smkDecoder->closeFile();
	}
	delete player;
	delete smkDecoder;

	return SUCCESS;
}

} // End of namespace Saga

#endif

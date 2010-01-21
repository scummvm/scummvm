/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/config-manager.h"

#include "engines/stark/gfx_opengl.h"

#include "engines/stark/stark.h"
#include "engines/stark/adpcm.h"
#include "engines/stark/sound.h"
#include "engines/stark/xmg.h"

#include "sound/mixer.h"

namespace Stark {

StarkEngine *g_stark = NULL;
GfxBase *g_driver = NULL;

StarkEngine::StarkEngine(OSystem *syst, const StarkGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	g_stark = this;

}

StarkEngine::~StarkEngine() {
	_xArchive.close();
}

Common::Error StarkEngine::run() {
	// Load in the main archive
	/*
	if (getGameID() == GID_TLJ) {
		if (!_xArchive.open("45/00/00.xarc")) {
			printf("Could not open x.xarc!\n");
			return Common::kNoError;
		}
		/*XRCFile *xrc = new XRCFile(_graphArchive.getRawData(0));
		delete xrc;* /

		Audio::SoundHandle *s = new Audio::SoundHandle();
		Common::File _f;
		_f.open("45/xarc/00000003.iss");
		Common::SeekableReadStream *dat = _f.readStream(_f.size());

		ISS *sound = new ISS(dat);
		_mixer->playInputStream(Audio::Mixer::kPlainSoundType, s, sound->_stream);

		g_system->delayMillis(100000);
		
	} else {
		
	}
	*/
	bool fullscreen = false;//(tolower(g_registry->get("fullscreen", "FALSE")[0]) == 't');

	////if (!_softRenderer && !g_system->hasFeature(OSystem::kFeatureOpenGL))
		//error("gfx backend doesn't support hardware rendering");

	//if (_softRenderer)
		g_driver = new GfxOpenGL();
/*#ifdef USE_OPENGL
	else
		g_driver = new GfxOpenGL();
#else
	else
		error("gfx backend doesn't support hardware rendering");
#endif*/

	g_driver->setupScreen(640, 480, fullscreen);

	/*Bitmap *splash_bm = NULL;
	if (!(_gameFlags & GF_DEMO))
		splash_bm = g_resourceloader->loadBitmap("splash.bm");
*/
	g_driver->clearScreen();
/*
	if (!(_gameFlags & GF_DEMO))
		splash_bm->draw();

	g_driver->flipBuffer();*/

	//g_stark->setMode(ENGINE_MODE_NORMAL);
	g_stark->mainLoop();

	return Common::kNoError;
}

void StarkEngine::mainLoop(){
	for(;;){
		// Process events
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			// Handle any buttons, keys and joystick operations
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.ascii == 'q') {
					/*handleExit();*/
					break;
				} else {
					//handleChars(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
				}
			}
			/*if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
				handleControls(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			}*/
			// Check for "Hard" quit"
			if (e.type == Common::EVENT_QUIT)
				return;
			/*if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;*/
		}

		updateDisplayScene();
		doFlip();
		g_system->delayMillis(50);
	}
}

void StarkEngine::updateDisplayScene(){
	g_driver->clearScreen();

	// Draw bg
	// Draw other things
	XMGDecoder *xmg = new XMGDecoder();
	Common::File _f;
	_f.open("house_layercenter.xmg");
	Common::SeekableReadStream *dat = _f.readStream(_f.size());
	Surface *bmp;
	bmp = xmg->decodeImage(dat);
	g_driver->drawBitmap(bmp);
	delete xmg;
	delete bmp;
	delete dat;

	_f.close();

	// setup cam

	g_driver->set3DMode();

	// setup lights

	// draw actors
	/*
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glColor3f(0.5f, 0.5f, 1.0f);
	glVertex2f(.5f, .5f);
	glColor3f(0.0f, .3f, .8f);
	glVertex2f(1.0f, -.75f);
	glEnd();
*/
	// draw overlay

//	g_driver->storeDisplay();
	//drawPrimitives
}

void StarkEngine::doFlip() {
	g_driver->flipBuffer();
}
} // end of namespace Stark

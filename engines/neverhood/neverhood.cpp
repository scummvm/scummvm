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

#include "common/file.h"
#include "common/config-manager.h"
#include "base/plugins.h"
#include "base/version.h"
#include "graphics/cursorman.h"
#include "engines/util.h"
#include "neverhood/neverhood.h"
#include "neverhood/blbarchive.h"
#include "neverhood/collisionman.h"
#include "neverhood/gamemodule.h"
#include "neverhood/gamevars.h"
#include "neverhood/graphics.h"
#include "neverhood/resourceman.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"
#include "neverhood/sound.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

NeverhoodEngine::NeverhoodEngine(OSystem *syst, const NeverhoodGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("neverhood");

}

NeverhoodEngine::~NeverhoodEngine() {
	delete _rnd;
}

Common::Error NeverhoodEngine::run() {
	initGraphics(640, 480, true);

	_isSaveAllowed = false;

	_mouseX = 0;
	_mouseY = 0;

	_gameState.sceneNum = 0;
	_gameState.field2 = 0;

	_staticData = new StaticData();
	_staticData->load("neverhood.dat");

	_gameVars = new GameVars();

	_screen = new Screen(this);

	_res = new ResourceMan();
	_res->addArchive("a.blb");
	_res->addArchive("c.blb");
	_res->addArchive("hd.blb");
	_res->addArchive("i.blb");
	_res->addArchive("m.blb");
	_res->addArchive("s.blb");
	_res->addArchive("t.blb");

	CursorMan.showMouse(true);
	{
		// DEBUG: Dummy cursor
		byte buffer[2*2];
		memset(buffer, 255, 4);
		CursorMan.replaceCursor(buffer, 2, 2, 0, 0, 0);
	}

#if 0
	// TODO: This should probably be implemented as debug command later 
	dumpAllResources();
#endif

#if 1

	_soundMan = new SoundMan(this);
	_audioResourceMan = new AudioResourceMan(this);
	
	_collisionMan = new CollisionMan(this);
	_gameModule = new GameModule(this);
	
	_gameModule->startup();

	// Preliminary main loop, needs some more work but works for testing
	while (!shouldQuit()) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
	
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_keyState = event.kbd.keycode;
				switch (_keyState) {
				case Common::KEYCODE_SPACE:
					_gameModule->handleSpaceKey();
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_KEYUP:
				_keyState = Common::KEYCODE_INVALID;
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				_gameModule->handleMouseMove(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
				_gameModule->handleMouseDown(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				_gameModule->handleMouseUp(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				break;
			default:
				break;
			}
		}

		//debug("millis %d", _system->getMillis());		
		_gameModule->handleUpdate();
		_gameModule->draw();
		_soundMan->update();
		_audioResourceMan->update();
		_screen->wait();
		_screen->update();
		
		debug(0, "---------------------------------------");
	
	}
	
	delete _gameModule;
	delete _collisionMan;
	delete _soundMan;
	delete _audioResourceMan;
#endif


	delete _res;
	delete _screen;

	delete _gameVars;
	delete _staticData;
	
	debug("Ok.");

	return Common::kNoError;
}

NPoint NeverhoodEngine::getMousePos() {
	NPoint pt;
	pt.x = _mouseX;
	pt.y = _mouseY;
	return pt;
}

void writeTga(const char *filename, byte *pixels, byte *palette, int16 width, int16 height) {
	byte identsize = 0;
	byte colourmaptype = 1;
	byte imagetype = 1;
	uint16 colourmapstart = 0;
	uint16 colourmaplength = 256;
	byte colourmapbits = 24;
	uint16 xstart = 0;
	uint16 ystart = 0;
	byte bits = 8;
	byte descriptor = 0x20;
	Common::DumpFile tga;
	tga.open(filename);
	tga.writeByte(identsize);
	tga.writeByte(colourmaptype);
	tga.writeByte(imagetype);
	tga.writeUint16LE(colourmapstart);
	tga.writeUint16LE(colourmaplength);
	tga.writeByte(colourmapbits);
	tga.writeUint16LE(xstart);
	tga.writeUint16LE(ystart);
	tga.writeUint16LE(width);
	tga.writeUint16LE(height);
	tga.writeByte(bits);
	tga.writeByte(descriptor);
	tga.write(palette, 768);
	tga.write(pixels, width * height);
	tga.close();
}

void NeverhoodEngine::dumpAllResources() {

	PaletteResource paletteResource(this);
	byte *vgaPalette = new byte[768];
	//paletteResource.load(0x4086520E);
	paletteResource.load(0x12C23307);
	byte *srcpalette = paletteResource.palette();
	for (int i = 0; i < 256; i++) {
		vgaPalette[i * 3 + 2] = srcpalette[i * 4 + 0];
		vgaPalette[i * 3 + 1] = srcpalette[i * 4 + 1];
		vgaPalette[i * 3 + 0] = srcpalette[i * 4 + 2];
	}

#if 0
	for (int i = 0; i < 768; i++)
		vgaPalette[i] <<= 2;
#endif

	uint entriesCount = _res->getEntryCount();
	debug("%d entries", entriesCount);

	for (uint i = 0; i < entriesCount; i++) {
		const ResourceFileEntry &entry = _res->getEntry(i);
		int type = _res->getResourceTypeByHash(entry.archiveEntry->fileHash);
		debug("hash: %08X; type: %d", entry.archiveEntry->fileHash, type);
		if (type == 4) {
			AnimResource anim(this);
			anim.load(entry.archiveEntry->fileHash);
			for (uint frameIndex = 0; frameIndex < anim.getFrameCount(); frameIndex++) {
				const AnimFrameInfo &frameInfo = anim.getFrameInfo(frameIndex);
				int16 width = (frameInfo.rect.width + 3) & 0xFFFC;
				byte *pixels = new byte[width * frameInfo.rect.height];
				memset(pixels, 0, width * frameInfo.rect.height);
				anim.draw(frameIndex, pixels, width, false, false);
				Common::String filename = 
					frameInfo.frameHash != 0
					? Common::String::format("%08X_%03d_%08X.tga", entry.archiveEntry->fileHash, frameIndex, frameInfo.frameHash) 
					: Common::String::format("%08X_%03d.tga", entry.archiveEntry->fileHash, frameIndex);
				writeTga(filename.c_str(), pixels, vgaPalette, width, frameInfo.rect.height);
				delete[] pixels;
			}
		}
	}
	
	delete[] vgaPalette;

}
	
} // End of namespace Neverhood

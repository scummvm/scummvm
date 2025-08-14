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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_sprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_rand
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose


#include "gamos/gamos.h" 
#include "graphics/framelimiter.h"
#include "gamos/detection.h"
#include "gamos/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/rect.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "common/keyboard.h"
#include "endian.h"
#include "audio/mididrv.h"
#include "audio/midiplayer.h"
#include <cstdio>

namespace Gamos {

GamosEngine *g_engine;


const byte GamosEngine::_xorKeys[32] =  {0xa7, 0x15, 0xf0, 0x56, 0xf3, 0xfa, 0x84, 0x2c, 
										 0xfd, 0x81, 0x38, 0xac, 0x73, 0xd2, 0x22, 0x47, 
										 0xa0, 0x12, 0xb8, 0x19, 0x20, 0x6a, 0x26, 0x7c, 
										 0x32, 0x57, 0xdd, 0xb2, 0x38, 0xa7, 0x95, 0x7a};

GamosEngine::GamosEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Gamos") {
	g_engine = this;
}

GamosEngine::~GamosEngine() {
	delete _screen;
}

uint32 GamosEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String GamosEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error GamosEngine::run() {
	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Common::String mname("solgamer.exe");
	init(mname);

	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		
		// g_system->getPaletteManager()->setPalette(pal, 0, 256);

		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error GamosEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

bool GamosEngine::loader2() {
	int32 skipsz = _arch.readSint32LE();
	_arch.skip(skipsz);

	if (_arch.readByte() != 7)
		return false;
	
	RawData data;
	if (!_arch.readCompressedData(&data))
		return false;

	int32 p1 = 0;
	int32 p2 = 0;
	int32 pid = 0;
	byte resType = 0;
	int32 resSize = 0;

	Common::MemoryReadStream dataStream(data.data(), data.size());
	while (!dataStream.eos()) {
		byte curByte = dataStream.readByte();
		
		if (curByte == 0) {
			break;
		} else if (curByte == 0x80) {
			p1 = 0;
			p2 = 0;
			pid = dataStream.readSint32LE();
		} else if (curByte == 1) {
			p1 = dataStream.readSint32LE();
		} else if (curByte == 2) {
			p2 = dataStream.readSint32LE();
		} else if (curByte == 7) {
			int32 needsz = dataStream.readSint32LE(); // check free mem ?
			printf("7777 want %d\n", needsz);
		} else if (curByte == 0x40) {
			resSize = 4;
			resType = 0x40;
			if (!loadResHandler(resType, pid, p1, p2, 0, data.data() + dataStream.pos(), resSize))
				return false;
			
			dataStream.skip(resSize);
		} else if (curByte == 0x41 || curByte == 0x42) {
			resSize = dataStream.readSint32LE();
			resType = curByte;
			if (!loadResHandler(resType, pid, p1, p2, 0, data.data() + dataStream.pos(), resSize))
				return false;
			
			dataStream.skip(resSize);
		} else if (curByte == 0x43) {
			resSize = 0x10;
			resType = 0x43;
			if (!loadResHandler(resType, pid, p1, p2, 0, data.data() + dataStream.pos(), resSize))
				return false;
			
			dataStream.skip(resSize);
		} else if (curByte == 0xff) {
			printf("0xFF  %d %d %d  ", pid, p1, p2);
			if (!reuseLastResource(resType, pid, p1, p2, 0))
				return false;
		} else {
			printf("loader2 want %x\n", curByte);
			return false;
		}
	}

	return true;
}

bool GamosEngine::loadModule(uint id) {
	if ( (!_runReadDataMod && !initOrLoadSave(_saveLoadID)) ||
	     !_arch.seekDir(1) )
		return false;

	_currentModuleID = id;
	const byte targetDir = 2 + id;

	bool prefixLoaded = false;
	byte prevByte = 0;
	bool doLoad = true;

	int32 p1 = 0;
	int32 p2 = 0;
	int32 p3 = 0;
	int32 pid = 0;
	
	while(doLoad) {
		byte curByte = _arch.readByte();

		switch(curByte) {
			case 0:
				if (prefixLoaded) {
					doLoad = false;
					break;
				}

				prefixLoaded = true;

				if (!_arch.seekDir(targetDir))
					return false;

				break;
			case CONFTP_P1:
				p1 = _arch.readPackedInt();
				break;
			case CONFTP_P2:
				p2 = _arch.readPackedInt();
				break;
			case CONFTP_P3:
				p3 = _arch.readPackedInt();
				break;
			case 4: {
				_resReadOffset = _arch.pos();
				bool isResource = true;
				if (prevByte == RESTP_F) {
					RawData data;
					if (!_arch.readCompressedData(&data))
						return false;
					if (_runReadDataMod && BYTE_004177f7 == 0)
						readData2(data);
					if (BYTE_004177f7 == 0) {
						//FUN_00403868();
					}
					isResource = false; /* do not loadResHandler */
				} else if (prevByte == RESTP_10) {
					if (!initMainDatas())
						return false;
					isResource = false; /* do not loadResHandler */
				} else if (prevByte == RESTP_11) {
					RawData data;
					if (!_arch.readCompressedData(&data))
						return false;
					if (pid == id)
						readElementsConfig(data);
					isResource = false; /* do not loadResHandler */
				} else if (prevByte == RESTP_18) {
					/* free elements ? */
				}

				if (isResource) {
					RawData data;
					if (!_arch.readCompressedData(&data))
						return false;

					if (!loadResHandler(prevByte, pid, p1, p2, p3, data))
						return false;
				}
				
				
				/* memory management
				if (prevByte == RESTP_43) {

				} else if (prevByte != RESTP_11 && prevByte != RESTP_20) {
					// grow used space
				}
				*/
				
				break;
			}
			case 5: {
				byte t = _arch.readByte();
				if (t == 0 || (t & 0xec) != 0xec)
					return false;
				
				byte sz = (t & 3) + 1;
				int32 movieSize = 0;
				for(uint i = 0; i < sz; ++i)
					movieSize |= _arch.readByte() << (i * 8);
				
				if (prevByte == 0x14)
					_movieOffsets[pid] = _arch.pos();
				
				_arch.skip(movieSize);
				break;
			}
			case 6:
				if (!loader2())
					return false;
				break;
			case 0xFF:
				return false;
				break;
			default:
				p1 = 0;
				p2 = 0;
				p3 = 0;
				pid = 0;
				prevByte = curByte & CONFTP_RESMASK;

				if ( (curByte & CONFTP_IDFLG) == 0 )
					pid = _arch.readPackedInt();

				break;
		}
	}

	_screen->addDirtyRect(_screen->getBounds());
	_screen->update();

	return true;
}

bool GamosEngine::loadResHandler(uint tp, uint pid, uint p1, uint p2, uint p3, const byte *data, size_t dataSize) {
	if (tp == RESTP_12) {
		setFPS(_fps);
	} else if (tp == RESTP_18) {
		printf("18 size %d\n", dataSize);
	} else if (tp == RESTP_40) {
		return loadRes40(pid, data, dataSize);		
	} else if (tp == RESTP_41) {
		return loadRes41(pid, data, dataSize);
	} else if (tp == RESTP_42) {
		return loadRes42(pid, p1, data, dataSize);
	} else if (tp == RESTP_43) {
		return loadRes43(pid, p1, p2, data, dataSize);
	} else if (tp == RESTP_50) {
		//printf("data 50 size %d\n", dataSize);
	} else if (tp == RESTP_51) {
		//printf("sound  size %d\n", dataSize);
	} else if (tp == RESTP_52) {
		return loadRes52(pid, data, dataSize);
		//printf("midi  size %d\n", dataSize);
	} else if (tp == RESTP_XORSEQ0) {
		loadXorSeq(data, dataSize, 0);
	} else if (tp == RESTP_XORSEQ1) {
		loadXorSeq(data, dataSize, 1);
	} else if (tp == RESTP_XORSEQ2) {
		loadXorSeq(data, dataSize, 2);
	} else {
		//printf("Unk Res %x\n", tp);
	}
	return true;
}

bool GamosEngine::loadResHandler(uint tp, uint pid, uint p1, uint p2, uint p3, const RawData &data) {
	return loadResHandler(tp, pid, p1, p2, p3, data.data(), data.size());
}

bool GamosEngine::reuseLastResource(uint tp, uint pid, uint p1, uint p2, uint p3) {
	if (tp == RESTP_43) {
		_sprites[pid].sequences[p1][p2].image = _images.back();
	} else {
		return false;
	}
	return true;
}


bool GamosEngine::initOrLoadSave(int32) {
	return false;
}

bool GamosEngine::initMainDatas() {
	RawData rawdata;

	if (!_arch.readCompressedData(&rawdata))
		return false;
	
	Common::MemoryReadStream dataStream(rawdata.data(), rawdata.size(), DisposeAfterUse::NO);
	
	_magic = dataStream.readUint32LE();
	_pages1kbCount = dataStream.readUint32LE();
	_readBufSize = dataStream.readUint32LE();
	_width = dataStream.readUint32LE();
	_height = dataStream.readUint32LE();
	_unk2 = dataStream.readUint32LE();
	_unk3 = dataStream.readUint32LE();
	_movieCount = dataStream.readUint32LE();
	_unk5 = dataStream.readByte();
	_unk6 = dataStream.readByte();
	_unk7 = dataStream.readByte();
	_fps = dataStream.readByte();
	_unk8 = dataStream.readByte();
	_unk9 = dataStream.readByte();
	_fadeEffectID = dataStream.readByte();
	_unk11 = dataStream.readByte();

	/*_winX = dataStream.readUint32LE();
	_winY = dataStream.readUint32LE();
	_winW = dataStream.readUint32LE();
	_winH = dataStream.readUint32LE();*/
	dataStream.skip(16);

	int64 pos = dataStream.pos();
	_string1 = dataStream.readString(0, 64);
	dataStream.seek(pos + 64);
	_winCaption = dataStream.readString(0, 9);

	if (!_screen) {
		initGraphics(_width, _height);
		_screen = new Graphics::Screen();
	}

	_movieOffsets.clear();
	_movieOffsets.resize(_movieCount, 0);

	return true;
}

bool GamosEngine::init(const Common::String &moduleName) {
	BYTE_004177f7 = 0;

	if (!_arch.open(Common::Path(moduleName)))
		return false;

	if (!loadInitModule())
		return false;
	

	if (!playIntro())
		return false;

	return true;
}

bool GamosEngine::loadInitModule() {

	_runReadDataMod = true;

	return loadModule(0);
}

void GamosEngine::setFPS(uint fps) {
	_delayTime = 0;
	if (fps)
		_delayTime = 1000 / fps;
}

void GamosEngine::readElementsConfig(const RawData &data) {
	Common::MemoryReadStream dataStream(data.data(), data.size(), DisposeAfterUse::NO);

	uint32 bkgnum1 = dataStream.readUint32LE(); // 0
	uint32 bkgnum2 = dataStream.readUint32LE(); // 4
	dataStream.readUint32LE(); // 8
	dataStream.readUint32LE(); // c
	dataStream.readUint32LE(); // 10
	dataStream.readUint32LE(); // 14
	dataStream.readUint32LE(); // 18
	dataStream.readUint32LE(); // 1c
	dataStream.readUint32LE(); // 20
	uint32 imageCount = dataStream.readUint32LE(); // 24
	dataStream.readUint32LE(); // 28
	uint32 midiCount = dataStream.readUint32LE(); // 2c
	dataStream.readUint32LE(); // 30

	_bkgImages.clear();
	_bkgImages.resize(bkgnum1 * bkgnum2);
	
	_sprites.clear();
	_sprites.resize(imageCount);

	_midiTracks.clear();
	_midiTracks.resize(midiCount);
}

void GamosEngine::loadXorSeq(const byte *data, size_t dataSize, int id) {
	Common::MemoryReadStream dataStream(data, dataSize);

	Common::Array<XorArg> &seq = _xorSeq[id];

	uint32 num = dataStream.readUint32LE();
	seq.resize(num);

	for(uint i = 0; i < num; ++i) {
		seq[i].len = dataStream.readUint32LE();
		seq[i].pos = dataStream.readUint32LE();
	}
}

bool GamosEngine::loadRes40(int32 id, const byte *data, size_t dataSize) {
	if (dataSize < 4)
		return false;
	
	_sprites[id].field_0 = data[0];
	_sprites[id].field_1 = data[1];
	_sprites[id].field_2 = data[2];
	_sprites[id].field_3 = data[3];

	return true;
}

bool GamosEngine::loadRes41(int32 id, const byte *data, size_t dataSize) {
	if (*(const uint32 *)data != 0) {
		printf("41 not null!!!\n");
		exit(0);
	}
	_sprites[id].sequences.resize(dataSize / 4);
	return true;
}

bool GamosEngine::loadRes42(int32 id, int32 p1, const byte *data, size_t dataSize) {
	//printf("loadRes42 pid %d p %d sz %x\n",id, p1, dataSize);

	if (_sprites[id].sequences.size() == 0)
		_sprites[id].sequences.resize(1);

	int32 count = dataSize / 8;
	_sprites[id].sequences[p1].resize(count);

	Common::MemoryReadStream strm(data, dataSize);
	for(int i = 0; i < count; ++i) {
		int32 dataz = strm.readSint32LE();
		if (dataz != 0) {
			printf("42    nut null \n");
			exit(0);
		}

		ImagePos &imgpos = _sprites[id].sequences[p1][i];
		imgpos.xoffset = strm.readSint16LE();
		imgpos.yoffset = strm.readSint16LE();
	}
	return true;
}

bool GamosEngine::loadRes43(int32 id, int32 p1, int32 p2, const byte *data, size_t dataSize) {
	_images.push_back( new Image() );
	_sprites[id].sequences[p1][p2].image = _images.back();

	Image *img = _sprites[id].sequences[p1][p2].image;

	Common::MemoryReadStream s(data, dataSize);
	img->surface.pitch = img->surface.w = s.readSint16LE();
	img->surface.h = s.readSint16LE();
	img->loaded = false;

	uint32 token = s.readUint32LE();

	/* token 'Disk' */
	if (token == 0x4469736b) {
		img->offset = s.readSint32LE();
		img->cSize = s.readSint32LE();
	} else {
		if (_sprites[id].field_1 & 0x80) {
			img->offset = _arch._lastReadDataOffset;
			img->cSize = _arch._lastReadSize;
		} else {
			img->loaded = true;
			img->rawData.assign(data + 4, data + dataSize);
			img->surface.setPixels(img->rawData.data());
			img->surface.format = Graphics::PixelFormat::createFormatCLUT8();
		}
	}

	return true;
}

bool GamosEngine::loadRes52(int32 id, const byte *data, size_t dataSize) {
	_midiTracks[id].assign(data, data + dataSize);
	return true;
}


bool GamosEngine::playIntro() {
	if (_movieCount != 0 && _unk11 == 1)
		return scriptFunc18(0);
	return true;
}


bool GamosEngine::scriptFunc18(int id) {
	if (true) {
		_isMoviePlay++;
		bool res = _moviePlayer.playMovie(&_arch, _movieOffsets[id], this);
		_isMoviePlay--;
		return res;
	}

	return true;
}

void GamosEngine::stopMidi() {
	_musicPlayer.stopMusic();
	_midiStarted = false;
}

void GamosEngine::stopMCI() {

}

void GamosEngine::stopSounds() {

}



void GamosEngine::setErrMessage(const Common::String &msg) {
	if (_errSet)
		return;
	
	_errMessage = msg;
	_errSet = true;
}

void GamosEngine::updateScreen(bool checkers, Common::Rect rect) {
	if (_width == 0 || _height == 0)
		return;

	if (!checkers) {
		_screen->addDirtyRect(rect);
		return;
	}

	static const Common::Point checkerCoords[16] = {
		{0, 0}, {16, 32}, {48, 16}, {16, 48}, 
		{0, 32}, {32, 48}, {16, 16}, {48, 0},
		{32, 32}, {0, 48}, {32, 16}, {16, 0},
		{48, 32}, {32, 0}, {0, 16}, {48, 48}};
	
	const int16 maxDelay = (500 / 10) - 1;
	
	for (int16 p = 0; p < 16; p++) {
		uint32 val = g_system->getMillis();
		const Common::Point point = checkerCoords[p];
		for (uint32 x = point.x; x < _width; x += 64) {
			for (uint32 y = point.y; y < _height; y += 64) {
				_screen->addDirtyRect(Common::Rect(x, y, x + 16, y + 16));
			}
		}
		_screen->update();
		val = g_system->getMillis() - val;

		if (val > maxDelay) 
			g_system->delayMillis(maxDelay - val);
	}
}


void GamosEngine::readData2(const RawData &data) {
	Common::MemoryReadStream dataStream(data.data(), data.size());
	dataStream.skip(4); // FIX ME
	_messageProc._gd2flags = dataStream.readByte();
	dataStream.skip(0x40 - 5); // FIX ME
	for (int i = 0; i < 12; i++) {
		_messageProc._keyCodes[i] = _winkeyMap[dataStream.readByte()];
	}
}


void GamosEngine::playMidi(Common::Array<byte> *buffer) {
	_musicPlayer.stopMusic();
	_musicPlayer.playMusic(buffer);
	_midiStarted = true;
}

} // End of namespace Gamos

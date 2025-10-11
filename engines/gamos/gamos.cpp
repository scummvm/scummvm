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
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout


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
#include "common/util.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "common/keyboard.h"
#include "common/endian.h"
#include "audio/mididrv.h"
#include "audio/midiplayer.h"

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

	VM::_callFuncs = callbackVMCallDispatcher;
	VM::_callingObject = this;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	g_system->showMouse(true);

	Common::String mname;
	if (Common::String(_gameDescription->gameId) == Common::String("solgamer"))
		mname = "solgamer.exe";
	else if (Common::String(_gameDescription->gameId) == Common::String("pilots"))
		mname = "pilots.exe";
	else if (Common::String(_gameDescription->gameId) == Common::String("pilots2"))
		mname = "pilots2.exe";

	init(mname);

	Common::Event e;

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			_messageProc.processMessage(e);
		}

		uint32 curTime = g_system->getMillis();
		if (curTime > _lastTimeStamp + _delayTime) {
			_lastTimeStamp = curTime;

			if (_messageProc._gd2flags & 2) {

			}

			uint8 result = 2;
			while (result == 2) {
				result = update({}, _messageProc._mouseReportedPos, _messageProc._mouseActPos, _messageProc._act2, _messageProc._act1, _messageProc._rawKeyCode, true);
			}

			if (!result)
				break;

			_messageProc._act2 = ACT_NONE;
			_messageProc._act1 = ACT_NONE;
			_messageProc._rawKeyCode = ACT_NONE;

			doDraw();
		}

		//if (_delayTime)
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
			//printf("7777 want %d\n", needsz);
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

			_loadedDataSize += (resSize + 3) & (~3);

			dataStream.skip(resSize);
		} else if (curByte == 0x43) {
			resSize = 0x10;
			resType = 0x43;
			if (!loadResHandler(resType, pid, p1, p2, 0, data.data() + dataStream.pos(), resSize))
				return false;

			_loadedDataSize += (resSize + 3) & (~3);

			dataStream.skip(resSize);
		} else if (curByte == 0xff) {
			//printf("0xFF  %d %d %d \n", pid, p1, p2);
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

	_readingBkgMainId = -1;

	/* Complete me */

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
					_readingBkgOffset = _arch.pos();
				}

				RawData data;
				if (isResource) {
					if (!_arch.readCompressedData(&data))
						return false;

					if (!loadResHandler(prevByte, pid, p1, p2, p3, data))
						return false;

				}

				uint32 datasz = (data.size() + 3) & (~3);

				switch (prevByte) {
					case RESTP_11:
					case RESTP_18:
					case RESTP_19:
					case RESTP_20:
					case RESTP_40:
					case RESTP_50:
						break;

					case RESTP_43:
						//printf("t %x sz %x sum %x\n", prevByte, data.size(), _loadedDataSize);
						if (_onlyScanImage)
							_loadedDataSize += 0x10;
						else
							_loadedDataSize += datasz;
						break;

					default:
						//printf("t %x sz %x sum %x\n", prevByte, data.size(), _loadedDataSize);
						_loadedDataSize += datasz;
						break;
				}

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
				if (!reuseLastResource(prevByte, pid, p1, p2, 0))
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

	//FUN_00404a28();
	if (BYTE_004177f7 == 0) {
		// Reverse Here

		setCursor(0, false);

		if (_readingBkgMainId == -1)
			_screen->setPalette(_bkgImages[0].palette);
			//FUN_00405ebc(0, false);
		else
			_screen->setPalette(_bkgImages[_readingBkgMainId].palette);
			//FUN_00405ebc(0, false);

		addDirtyRect(Common::Rect(Common::Point(), _bkgUpdateSizes ));
	} else {

	}
	_screen->update();

	return true;
}

bool GamosEngine::loadResHandler(uint tp, uint pid, uint p1, uint p2, uint p3, const byte *data, size_t dataSize) {
	if (tp == RESTP_12) {
		Common::MemoryReadStream dataStream(data, dataSize, DisposeAfterUse::NO);

		_addrBlk12 = _loadedDataSize;
		_addrFPS = _loadedDataSize + 1;
		_addrKeyDown = _loadedDataSize + 2;
		_addrKeyCode = _loadedDataSize + 3;
		_addrCurrentFrame = _loadedDataSize + 4;

		VM::memory().setU8( _addrBlk12, dataStream.readByte() );
		dataStream.skip(1);
		VM::memory().setU8( _addrFPS, _fps );
		VM::memory().setU8( _addrKeyDown, dataStream.readByte() );
		VM::memory().setU8( _addrKeyCode, dataStream.readByte() );
		VM::memory().setU32( _addrCurrentFrame, dataStream.readUint32LE() );

		setFPS(_fps);
	} else if (tp == RESTP_13) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
	} else if (tp == RESTP_18) {
		loadRes18(pid, data, dataSize);
	} else if (tp == RESTP_19) {
		if (BYTE_004177f7 == 0) {
			for (int i = 0; i < _states.size(); i++)
				_states.at(i) = 0xf0fe;

			DAT_004177f8 = 1;
			ProcessScript(true, data, dataSize);
			if (_needReload)
				warning("needs reload from loadResHandler, CANT HAPPEN!");
			DAT_004177f8 = 0;
			FUN_00404fcc(pid);
		}
	} else if (tp == RESTP_20) {
		if (dataSize != 4)
			return false;
		_someActsArr[pid].unk1 = getU32(data);
	} else if (tp == RESTP_21) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_someActsArr[pid].script1 = _loadedDataSize + p3;
		//printf("RESTP_21 %x pid %d sz %x\n", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_22) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_someActsArr[pid].script2 = _loadedDataSize + p3;
		//printf("RESTP_22 %x pid %d sz %x\n", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_23) {
		if (dataSize % 4 != 0 || dataSize < 4)
			return false;
		_someActsArr[pid].scriptS.resize(dataSize / 4);
	} else if (tp == RESTP_2A) {
		ScriptS &scr = _someActsArr[pid].scriptS[p1];
		scr.data.assign(data, data + dataSize);
	} else if (tp == RESTP_2B) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_someActsArr[pid].scriptS[p1].codes1 = _loadedDataSize + p3;
		//printf("RESTP_2B %x pid %d p1 %d sz %x\n", _loadedDataSize, pid, p1, dataSize);
	} else if (tp == RESTP_2C) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_someActsArr[pid].scriptS[p1].codes2 = _loadedDataSize + p3;
		//printf("RESTP_2C %x pid %d p1 %d sz %x\n", _loadedDataSize, pid, p1, dataSize);
	} else if (tp == RESTP_38) {
		warning("Data 38 size %zu", dataSize);
		_thing2[pid].field_0.assign(data, data + dataSize);
	} else if (tp == RESTP_39) {
		_thing2[pid].field_1.assign(data, data + dataSize);
	} else if (tp == RESTP_3A) {
		_thing2[pid].field_2.assign(data, data + dataSize);
	} else if (tp == RESTP_40) {
		return loadRes40(pid, data, dataSize);
	} else if (tp == RESTP_41) {
		return loadRes41(pid, data, dataSize);
	} else if (tp == RESTP_42) {
		return loadRes42(pid, p1, data, dataSize);
	} else if (tp == RESTP_43) {
		return loadRes43(pid, p1, p2, data, dataSize);
	} else if (tp == RESTP_50) {
		/* just ignore it? */
	} else if (tp == RESTP_51) {
		_soundSamples[pid].assign(data, data + dataSize);
		//printf("sound  size %d\n", dataSize);
	} else if (tp == RESTP_52) {
		return loadRes52(pid, data, dataSize);
		//printf("midi  size %d\n", dataSize);
	} else if (tp == RESTP_60) {
		_dat60[pid].assign(data, data + dataSize);
	} else if (tp == RESTP_61) {
		Common::MemoryReadStream dataStream(data, dataSize, DisposeAfterUse::NO);
		const int count = dataSize / 8;
		_dat61[pid].resize(count);

		for (int i = 0; i < count; i++) {
			Dat61 &d = _dat61[pid][i];

			d.x = dataStream.readSint16LE();
			d.y = dataStream.readSint16LE();
			d.v = dataStream.readUint16LE();

			dataStream.skip(2);
		}
	} else if (tp == RESTP_XORSEQ0) {
		loadXorSeq(data, dataSize, 0);
	} else if (tp == RESTP_XORSEQ1) {
		loadXorSeq(data, dataSize, 1);
	} else if (tp == RESTP_XORSEQ2) {
		loadXorSeq(data, dataSize, 2);
	} else {
		warning("Unk Res %x at %x sz %zx", tp, _loadedDataSize, dataSize);
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
	_gridCellW = dataStream.readSint32LE();
	_gridCellH = dataStream.readSint32LE();
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

	_drawElements.clear();

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
	rndSeed(g_system->getMillis());
	//DAT_0041723c = -1;
	_curObjIndex = -1;
	PTR_00417218 = nullptr;
    PTR_00417214 = nullptr;
	//DAT_00417238 = 0;
	_xorSeq[2].clear();
	_xorSeq[1].clear();
	_xorSeq[0].clear();
	_isMoviePlay = 0;
	DAT_00417802 = false;
	//DAT_00417808 = 0;
	_runReadDataMod = true;
	//DAT_00417807 = 0;
	//DAT_00417806 = 0;
	//DAT_004177fa = 0;
	//DAT_004177fb = 0;
	//_mouseInWindow = false;

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
	_statesWidth = dataStream.readUint32LE(); // 8
	_statesHeight = dataStream.readUint32LE(); // c
	_bkgUpdateSizes.x = dataStream.readUint32LE(); // 10
	_bkgUpdateSizes.y = dataStream.readUint32LE(); // 14
	/* bkgbufferSize */ dataStream.readUint32LE(); // 18
	uint32 actsCount = dataStream.readUint32LE(); // 1c
	uint32 unk1Count = dataStream.readUint32LE(); // 20
	uint32 imageCount = dataStream.readUint32LE(); // 24
	uint32 soundCount = dataStream.readUint32LE(); // 28
	uint32 midiCount = dataStream.readUint32LE(); // 2c
	uint32 dat6xCount = dataStream.readUint32LE(); // 30

	_statesShift = 2;
	for(int i = 2; i < 9; i++) {
		if (_statesWidth <= (1 << i)) {
			_statesShift = i;
			break;
		}
	}

	_states.clear();
	_states.resize(_statesWidth, _statesHeight);

	_bkgImages.clear();
	_bkgImages.resize(bkgnum1 * bkgnum2);

	_sprites.clear();
	_sprites.resize(imageCount);

	for (uint i = 0; i < imageCount; i++)
		_sprites[i].index = i;

	_midiTracks.clear();
	_midiTracks.resize(midiCount);

	_mixer->stopAll();
	_soundSamples.clear();
	_soundSamples.resize(soundCount);

	_thing2.clear();
	_thing2.resize(unk1Count);

	_someActsArr.clear();
	_someActsArr.resize(actsCount);

	_dat60.clear();
	_dat61.clear();

	_dat60.resize(dat6xCount);
	_dat61.resize(dat6xCount);

	_loadedDataSize = 0;
	VM::clearMemory();
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

	if (dataSize % 4)
		warning("dataSize > 4");

	_sprites[id].field_0 = data[0];
	_sprites[id].field_1 = data[1];
	_sprites[id].field_2 = data[2];
	_sprites[id].field_3 = data[3];

	_onlyScanImage = data[1] & 0x80;

	return true;
}

bool GamosEngine::loadRes41(int32 id, const byte *data, size_t dataSize) {
	if (*(const uint32 *)data != 0) {
		warning("41 not null!!!");
		exit(0);
	}
	if (dataSize % 4)
		warning("loadRes41 datasize > 4");
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
			warning("42    nut null");
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
	img->offset = -1;

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

bool GamosEngine::loadRes18(int32 id, const byte *data, size_t dataSize) {
	BkgImage &bimg = _bkgImages[id];
	bimg.loaded = true;
	bimg.offset = _readingBkgOffset;
	bimg.field2_0x8 = 0;
	bimg.field3_0xc = 0;
	bimg.palette = nullptr;

	bimg.rawData.assign(data, data + dataSize);

	Common::MemoryReadStream strm(data, dataSize);

	if (_readingBkgMainId == -1 && (strm.readUint32LE() & 0x80000000) )
		_readingBkgMainId = id;

	//printf("res 18 id %d 4: %x\n", id, strm.readUint32LE());

	strm.seek(8);

	bimg.surface.pitch = bimg.surface.w = strm.readUint32LE();
	bimg.surface.h = strm.readUint32LE();

	uint32 imgsize = strm.readUint32LE();

	//printf("res 18 id %d 14: %x\n", id, strm.readUint32LE());

	bimg.surface.setPixels(bimg.rawData.data() + 0x18);
	bimg.surface.format = Graphics::PixelFormat::createFormatCLUT8();

	bimg.palette = bimg.rawData.data() + 0x18 + imgsize;

	return true;
}


bool GamosEngine::playIntro() {
	if (_movieCount != 0 && _unk11 == 1)
		return scriptFunc18(0);
	return true;
}

bool GamosEngine::playMovie(int id) {
	bool res = _moviePlayer.playMovie(&_arch, _movieOffsets[id], this);
	if (_readingBkgMainId == -1)
		_screen->setPalette(_bkgImages[0].palette);
	else
		_screen->setPalette(_bkgImages[_readingBkgMainId].palette);
	return res;
}


bool GamosEngine::scriptFunc18(uint32 id) {
	if (true) {
		_isMoviePlay++;
		bool res = playMovie(id);
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
	warning("Not implemented stopMCI");
}

void GamosEngine::stopSounds() {
	warning("Not implemented stopSounds");
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
	dataStream.seek(4); // FIX ME
	_messageProc._gd2flags = dataStream.readByte(); //4
	//5
	//x14
	dataStream.seek(0x14);
	_d2_fld14 = dataStream.readByte(); // x14
	_enableMidi = dataStream.readByte() != 0 ? true : false; //x15
	_d2_fld16 = dataStream.readByte(); // x16
	_d2_fld17 = dataStream.readByte(); // x17
	_d2_fld18 = dataStream.readByte(); // x18
	//x19

	dataStream.seek(0x38);
	_midiTrack = dataStream.readUint32LE(); //0x38
	_mouseCursorImgId = dataStream.readUint32LE(); //0x3c
	//0x40
	for (int i = 0; i < 12; i++) {
		_messageProc._keyCodes[i] = dataStream.readByte();
	}
}


bool GamosEngine::playMidi(Common::Array<byte> *buffer) {
	_musicPlayer.stopMusic();
	_midiStarted = _musicPlayer.playMusic(buffer);
	return _midiStarted;
}

bool GamosEngine::playSound(uint id) {
	Audio::SeekableAudioStream *stream = Audio::makeRawStream(_soundSamples[id].data(), _soundSamples[id].size(), 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, nullptr, stream, id);
	return true;
}

uint8 GamosEngine::update(Common::Point screenSize, Common::Point mouseMove, Common::Point actPos, uint8 act2, uint8 act1, uint16 keyCode, bool mouseInWindow) {
	_needReload = false;
	VM::_interrupt = false;

	FUN_00402c2c(mouseMove, actPos, act2, act1);

	if ( FUN_00402bc4() ) {
		bool loop = false;
		if (!DAT_00417802)
			loop = FUN_00402fb4();
		/*else
			loop = FUN_00403314(_messageProc._act2);*/

		if (_needReload)
			return 2;  // rerun update after loadModule

		while (loop) {
			if (!PTR_00417388) {
				if (FUN_004033a8(mouseMove) && FUN_004038b8())
					return 1;
				else
					return 0;
			}

			RawKeyCode = ACT_NONE;

			if (!FUN_00402bc4())
				return 0;

			if (!DAT_00417802)
				loop = FUN_00402fb4();
			/*else
				loop = FUN_00403314(_messageProc._act2);*/

			if (_needReload)
				return 2; // rerun update after loadModule
		}
	}
	return 1;
}


int32 GamosEngine::ProcessScript(bool p1, const byte *data, size_t dataSize, int32 code1, int32 code2) {

	Common::Array<uint16> ARR_00412208(512);

	Common::MemoryReadStream rstream(data, dataSize);

	if (!p1) {
		DAT_00417228 = PTR_00417218->pos;
		DAT_0041722c = PTR_00417218->blk;
	} else {
		PTR_00417218 = nullptr;
		_curObjIndex = -1;
		PTR_00417214 = nullptr;
		//DAT_00417238 = 0;
		//DAT_0041723c = -1;
		DAT_0041722c = 0;
		DAT_00417228 = 0;
		BYTE_004177f6 = 0x10;
		_preprocDataId = 0;
		PTR_004173e8 = nullptr;
	}

	DAT_00417220 = DAT_00417228;
	DAT_00417224 = DAT_0041722c;

	int32 spos = -1;
	int32 sbuf[6];

	uint8 b[4];
	rstream.read(b, 4);

	//printf("FLAGS %x\n", b[0]);

	if (b[0] & 1) {
		if (code1 != -1) {
			if (!doScript(code1))
				return 0;
			if (_needReload)
				return 0;
		}
		rstream.skip(4);
	}

	if (b[0] & 2) {
		bool fastSkipAll = false;
		while (true) {
			uint16 sz = rstream.readUint16LE();
			uint8 f = rstream.readByte();
			uint8 t = rstream.readByte();

			if (fastSkipAll) {
				rstream.skip(sz * 4);
				if (f & 1)
					break;
				continue;
			}

			if (t == 4) {
				spos++;
				if (spos == 0) {
					sbuf[0] = 0;
					sbuf[1] = 0;
				} else {
					int32 p = sbuf[spos * 2 - 1];
					sbuf[spos * 2 + 1] = p;
					sbuf[spos * 2] = p;
				}
			} else {
				spos = -1;
			}
			int32 ps = spos * 2 + 1;
			for (int read = 0; read < sz; read++) {
				byte c[4];
				rstream.read(c, 4);
				preprocessData(_preprocDataId, c);

				uint16 fb = 0;
				if (!p1) {
					Common::Point xy;
					xy.x = ((int8)c[2] + DAT_00417220 + _statesWidth) % _statesWidth;
					xy.y = ((int8)c[3] + DAT_00417224 + _statesHeight) % _statesHeight;
					fb = _states.at( xy );
				} else {
					fb = _states.at(c[2], c[3]);
				}

				uint8 lb = fb & 0xff;
				uint8 hb = (fb >> 8) & 0xff;

				int cval = 0;
				int fnc = c[1] >> 4;
				if ((c[1] & 1) == 0) {
					if (c[0] == lb && (c[1] & hb & 0xf0)) {
						cval = 2;
					}
				} else if (lb != 0xfe &&
					       (_thing2[c[0]].field_0[(fb & 0xff) >> 3] & (1 << (fb & 7))) != 0) {

					if (!_thing2[c[0]].field_2.empty()) {
						c[1] = (c[1] & 0xf) | _thing2[c[0]].field_2[lb];
						preprocessData(fnc + 8, c);
					}

					if (hb & c[1] & 0xf0) {
						cval = 2;
					}
				}

				if ((c[1] & 2) == cval) {
					if ((c[1] & 0xc) == 0) {
						rstream.skip((sz - (read + 1)) * 4);
						break;
					}
					if ((c[1] & 0xc) == 4)
						return 0;
					if ((c[1] & 0xc) == 8) {
						fastSkipAll = true;
						rstream.skip((sz - (read + 1)) * 4);
						break;
					}
					ARR_00412208[ sbuf[ps] ] = (c[3] << 8) | c[2];
					sbuf[ps]++;
				} else if ((sz - read) == 1 && spos > -1 && sbuf[spos * 2] == sbuf[ps]) {
					return 0;
				}
			}

			if (f & 1)
				break;
		}
	}

	BYTE_00412200 = 0;

	if (b[0] & 4) {
		byte s = b[1];
		preprocessData(_preprocDataId, b);
		preprocessDataB1(b[1] >> 4, b);
		rnd();
		b[1] = (b[1] & 0xf0) | (s & 0xf);
		FUN_00402a68(b);
		if (_needReload)
			return 0;
	}

	BYTE_004177fc = 0;
	if (b[0] & 8) {
		uint32 fldsv;
		if (PTR_00417218)
			fldsv = PTR_00417218->fld_5;
		if (code2 != -1)
			doScript(code2);
		if (_needReload)
			return 0;
		rstream.skip(4);
		if (BYTE_004177fc == 0 && BYTE_00412200 == 0 && PTR_00417218 && PTR_00417218->fld_5 != fldsv && PTR_00417218->y != -1)
			addDirtRectOnObject( &_drawElements[PTR_00417218->y] );
	}

	if (BYTE_004177fc == 0 && BYTE_00412200 != 0)
		FUN_004095a0(PTR_00417218);

	int32 retval = 0;

	if (b[0] & 0x10) {
		int ivar5 = -1;
		while (true) {
			uint16 dcount = rstream.readUint16LE();
			uint8 dbits = rstream.readByte();
			uint8 dtype = rstream.readByte();

			/* set next pos before next iteration */
			uint32 nextpos = rstream.pos() + (dcount * 4);

			switch (dtype) {
			case 0: {
				uint16 rndval = rndRange16(b[1] & 3);

				if (rndval == 2) {
					rstream.skip(dcount * 4);
					dcount = rstream.readUint16LE();
					rstream.skip(2 + dcount * 4);
					dcount = rstream.readUint16LE();
					rstream.skip(2);
				} else if (rndval == 1) {
					rstream.skip(dcount * 4);
					dcount = rstream.readUint16LE();
					rstream.skip(2);
				}

				rnd();

				for (int i = 0; i < dcount; i++) {
					byte d[4];
					rstream.read(d, 4);
					retval += processData(p1, d);

					if (_needReload)
						return 0;
				}

				return retval + 1;
			} break;

			case 1: {
				int32 num = rndRange16(dcount);

				for (int i = 0; i < dcount; i++) {
					byte d[4];
					rstream.read(d, 4);

					if (num != 0) {
						retval += processData(p1, d);
						if (_needReload)
							return 0;
					}

					num--;
				}

			} break;

			case 2: {
				rstream.skip(4 * rndRange16(dcount));

				byte d[4];
				rstream.read(d, 4);

				retval += processData(p1, d);
				if (_needReload)
						return 0;
			} break;

			case 3: {
				for (int i = 0; i < dcount; i++) {
					uint16 doproc = rndRange16(2);

					byte d[4];
					rstream.read(d, 4);

					if (doproc != 0) {
						retval += processData(p1, d);

						if (_needReload)
							return 0;
					}
				}
			} break;

			default: {
				ivar5++;
				/* Seems it's has a error in original
				   think it's must be:
				   min + rnd(max-min) */

				uint32 lb = rnd() >> 0x10;
				uint32 idx = ((sbuf[ivar5 * 2 + 1] - sbuf[ivar5 * 2]) * lb + sbuf[ivar5 * 2]) >> 0x10;
				uint16 tval = ARR_00412208[ idx ];

				for (int i = 0; i < dcount; i++) {
					byte d[4];
					rstream.read(d, 4);

					if ( ((d[3] << 8) | d[2]) == tval ) {
						retval += processData(p1, d);
						if (_needReload)
							return 0;
						break;
					}
				}
			} break;

			}

			rstream.seek(nextpos);

			if (dbits & 1)
				break;
		}

	}
	return retval + 1;
}

uint32 GamosEngine::getU32(const void *ptr) {
	const uint8 *p = (const uint8 *)ptr;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


void GamosEngine::preprocessData(int id, byte *data) {
	switch (id) {

	default:
	case 0:
		break;

	case 1:
	case 10: {
		static const uint8 lookup[16] = {0, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0, 0x10, 0x30, 0x50, 0x70, 0x90, 0xB0, 0xD0, 0xF0};
		int8 tmp = (int8)data[3];
		data[3] = data[2];
		data[2] = -tmp;
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 2:
	case 12: {
		static const uint8 lookup[16] = {0, 0x40, 0x80, 0xC0, 0x10, 0x50, 0x90, 0xD0, 0x20, 0x60, 0xA0, 0xE0, 0x30, 0x70, 0xB0, 0xF0};
		data[3] = -((int8)data[3]);
		data[2] = -((int8)data[2]);
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 3:
	case 16: {
		static const uint8 lookup[16] = {0, 0x80, 0x10, 0x90, 0x20, 0xA0, 0x30, 0xB0, 0x40, 0xC0, 0x50, 0xD0, 0x60, 0xE0, 0x70, 0xF0};
		int8 tmp = (int8)data[2];
		data[2] = data[3];
		data[3] = -tmp;
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 4: {
		static const uint8 lookup[16] = {0, 0x10, 0x80, 0x90, 0x40, 0x50, 0xC0, 0xD0, 0x20, 0x30, 0xA0, 0xB0, 0x60, 0x70, 0xE0, 0xF0};
		data[2] = -((int8)data[2]);
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 5: {
		static const uint8 lookup[16] = {0, 0x20, 0x10, 0x30, 0x80, 0xA0, 0x90, 0xB0, 0x40, 0x60, 0x50, 0x70, 0xC0, 0xE0, 0xD0, 0xF0};
		int8 tmp = (int8)data[2];
		data[2] = -((int8)data[3]);
		data[3] = -tmp;
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 6: {
		static const uint8 lookup[16] = {0, 0x40, 0x20, 0x60, 0x10, 0x50, 0x30, 0x70, 0x80, 0xC0, 0xA0, 0xE0, 0x90, 0xD0, 0xB0, 0xF0};
		data[3] = -((int8)data[3]);
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;

	case 7: {
		static const uint8 lookup[16] = {0, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0};
		uint8 tmp = data[2];
		data[2] = data[3];
		data[3] = tmp;
		data[1] = (data[1] & 0xf) | lookup[ (data[1] >> 4) & 0xf ];
	} break;
	}
}

void GamosEngine::preprocessDataB1(int id, byte *data) {
	switch (id) {

		default:
		case 0:
			break;

		case 1:
		case 2:
		case 4:
		case 8:
			data[1] &= 0xf0;
			break;

		case 3: {
			static const uint8 lookup[2] = {0x10, 0x20};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 5: {
			static const uint8 lookup[2] = {0x10, 0x40};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 6: {
			static const uint8 lookup[2] = {0x20, 0x40};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 7: {
			static const uint8 lookup[3] = {0x10, 0x20, 0x40};
			data[1] = lookup[rndRange16(3)];
		} break;

		case 9: {
			static const uint8 lookup[2] = {0x10, 0x80};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 0xa: {
			static const uint8 lookup[2] = {0x20, 0x80};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 0xb: {
			static const uint8 lookup[3] = {0x10, 0x20, 0x80};
			data[1] = lookup[rndRange16(3)];
		} break;

		case 0xc: {
			static const uint8 lookup[2] = {0x40, 0x80};
			data[1] = lookup[rndRange16(2)];
		} break;

		case 0xd: {
			static const uint8 lookup[3] = {0x10, 0x40, 0x80};
			data[1] = lookup[rndRange16(3)];
		} break;

		case 0xe: {
			static const uint8 lookup[3] = {0x20, 0x40, 0x80};
			data[1] = lookup[rndRange16(3)];
		} break;

		case 0xf: {
			static const uint8 lookup[4] = {0x10, 0x20, 0x40, 0x80};
			data[1] = lookup[rndRange16(4)];
		} break;
	}
}

int GamosEngine::processData(int id, byte *data) {
	preprocessData(_preprocDataId, data);
	if (id == 0) {
		FUN_0040283c( ((int8)data[3] + DAT_00417224 + _statesHeight) % _statesHeight,
		              ((int8)data[2] + DAT_00417220 + _statesWidth) % _statesWidth,
					  data );
		if (_needReload)
			return 0;
		return data[2] == 0 && data[3] == 0;
	} else {
		FUN_0040283c( (int8)data[3], (int8)data[2],	data);
		return 0;
	}
}

void GamosEngine::FUN_00402a68(const byte *d) {
	if (d[2] != 0 || d[3] != 0) {
		DAT_00417220 = ((int8)d[2] + DAT_00417220 + _statesWidth) % _statesWidth;
		DAT_00417224 = ((int8)d[3] + DAT_00417224 + _statesHeight) % _statesHeight;

		uint8 t = PTR_00417218->fld_3;

        _states.at(DAT_00417228, DAT_0041722c) = ((PTR_00417218->fld_3 & 0xf0) << 8) | PTR_00417218->fld_2;

        FUN_00402654(0, DAT_00417224, DAT_00417220);

        PTR_00417218->pos = DAT_00417220;
        PTR_00417218->blk = DAT_00417224;

        uint16 &rthing = _states.at(DAT_00417220, DAT_00417224);

        PTR_00417218->fld_2 = rthing & 0xff;
        PTR_00417218->fld_3 = (t & 0xf) | ((rthing >> 8) & 0xf0);

        rthing = ((PTR_00417218->flags & 0xf0) << 8) | PTR_00417218->actID;

        BYTE_00412200 = 1;
	}

	if ((d[1] & 0xf0) != BYTE_004177f6) {
        BYTE_004177f6 = d[1] & 0xf0;
        PTR_00417218->flags = PTR_00417218->flags & 0xf;
        PTR_00417218->flags = PTR_00417218->flags | BYTE_004177f6;

		uint16 &tref = _states.at(DAT_00417220, DAT_00417224);
		tref = (tref & 0xff) | (BYTE_004177f6 << 8);

        BYTE_00412200 = 1;
    }
}

void GamosEngine::FUN_0040283c(int id, int pos, const byte *data) {
	byte td[4];
	memcpy(td, data, 4);

	uint16 &rthing = _states.at(pos, id);

	uint8 oid = td[0];

	if ((td[1] & 1) == 0) {
		if (oid == 0xfe) {
			FUN_00402654(1, id, pos);
			if (_needReload)
				return;

			rthing = (td[1] << 8) | td[0];
			return;
		}
	} else {
		Unknown1 &unk1 = _thing2[ oid ];
		uint8 index = rndRange16( unk1.field_1[0] );
		if (!unk1.field_2.empty()) {
			byte id1 = td[1];
			td[1] = unk1.field_2[ unk1.field_1[ index + 1 ] ];
			preprocessData(8 + (id1 >> 4), td);
		}
	}

	preprocessDataB1(td[1] >> 4, td);
	rnd(); // needed?

	td[0] = oid;

	Object *obj = nullptr;
	int index = 0;
	byte *odat = nullptr;

	SomeAction &act = _someActsArr[oid];
	if ((act.unk1 & 0xff) == 0) {
		FUN_00402654(1, id, pos);
		if (_needReload)
			return;
		obj = nullptr;
		index = -1;
		odat = nullptr;
	} else {
		FUN_00402654(0, id, pos);
		if (_needReload)
			return;
		obj = getFreeObject();
		obj->flags = (td[1] & 0xf0) | 3;
		obj->actID = oid;
		obj->fld_4 = 0;
		obj->fld_5 = (act.unk1 >> 16) & 0xff;
		obj->pos = pos;
		obj->blk = id;
		obj->x = -1;
		obj->y = -1;
		obj->fld_2 = rthing & 0xff;
		obj->fld_3 = (rthing >> 8 ) & 0xff;
		if (PTR_00417218 && obj->index > PTR_00417218->index)
			obj->fld_3 |= 1;

		int storageSize = ((act.unk1 >> 24) & 0xff) + 1;
		// if (storageSize < 5) {
		// 	obj->pImg = nullptr;
		// 	odat = &obj->pImg;
		// } else {
		// 	odat = malloc(storageSize);
		// 	obj->pImg = (Sprite *)odat;
		// 	obj->flags |= 8;
		// }
		obj->storage.clear();
		obj->storage.resize(storageSize, 0);
		odat = obj->storage.data();
		index = obj->index;
		if ((act.unk1 & 0xff) == 3 && PTR_004121b4 == nullptr)
			PTR_004121b4 = obj;
	}

	rthing = (td[1] << 8) | td[0];
	executeScript(td[1], id, pos, odat, index, obj, &act, act.script1);
}

void GamosEngine::removeObjectByIDMarkDirty(int32 id) {
	if (id != -1)
		removeObjectMarkDirty(&_drawElements[id]);
}


void GamosEngine::FUN_00402654(int mode, int id, int pos) {
	uint16 &rthing = _states.at(pos, id);

	uint8 actid = rthing & 0xff;

	if (actid == 0xfe)
		return;

	SomeAction &act = _someActsArr[actid];
	Object *povar4 = nullptr;
	bool multidel = false;

	for(uint i = 0; i < _drawElements.size(); i++) {
		Object &obj = _drawElements[i];
		if (obj.flags & 1) {
			if (obj.flags & 2) {
				if (obj.pos == pos && obj.blk == id) {
					removeObjectByIDMarkDirty(obj.y);
					if (obj.y != obj.x)
						removeObjectByIDMarkDirty(obj.x);
					/* if (obj.flags & 8)
						obj.storage.clear(); */
					FUN_004023d8(&obj);
					removeObject(&obj);
					FUN_0040255c(&obj);
					povar4 = &obj;
					if (!mode || multidel)
						break;

					multidel = true;
				}
			} else {
				if (mode && obj.fld_4 == pos && obj.fld_5 == id &&
					obj.pos == 0xff && obj.blk == 0xff && (obj.flags & 0x40) == 0) {

					removeObjectMarkDirty(&obj);
					if (multidel)
						break;

					multidel = true;
				}
			}
		}
	}

	if (povar4)
		rthing = ((povar4->fld_3 & 0xf0) << 8) | (povar4->fld_2 & 0xff);

	executeScript(rthing >> 8, id, pos, nullptr, -1, nullptr, &act, act.script2);
}

Object *GamosEngine::getFreeObject() {
	for (uint i = 0; i < _drawElements.size(); i++) {
		Object &rObj = _drawElements[i];
		if ( (rObj.flags & 1) == 0 ) {
			rObj.flags = 1;
			return &rObj;
		}
	}

	_drawElements.emplace_back();
	Object &rObj = _drawElements.back();
	rObj.flags = 1;
	rObj.index = _drawElements.size() - 1;
	return &rObj;
}

void GamosEngine::removeObject(Object *obj) {
	obj->flags = 0;
	/*if (&(_drawElements.back()) == obj) {
		int32 lastindex = _drawElements.size() - 1;
		for (int32 i = lastindex - 1; i >= 0; i--) {
			if ( _drawElements[i].flags & 1 ) {
				lastindex = i;
				break;
			}
		}
		_drawElements.resize(lastindex);
	}*/
}

void GamosEngine::removeObjectMarkDirty(Object *obj) {
	if (obj->flags & 0x80)
		addDirtRectOnObject(obj);
	removeObject(obj);
}


void GamosEngine::executeScript(uint8 p1, uint32 id, uint32 pos, byte *storage, int32 index, Object *pobj, SomeAction *act, int32 scriptAddr) {
	if (scriptAddr == -1)
		return;

	uint8 sv1 = BYTE_004177f6;
	byte *sv2 = PTR_004173e8;
	int32 sv3 = DAT_0041722c;
	int32 sv4 = DAT_00417228;
	int32 sv5 = DAT_00417224;
	int32 sv6 = DAT_00417220;
	int32 sv7 = _curObjIndex;
	Object *sv8 = PTR_00417218;
	SomeAction *sv9 = PTR_00417214;

	BYTE_004177f6 = p1;
	PTR_004173e8 = storage;
	DAT_0041722c = id;
	DAT_00417228 = pos;
	DAT_00417224 = id;
	DAT_00417220 = pos;
	_curObjIndex = index;
	PTR_00417218 = pobj;
	PTR_00417214 = act;

	doScript(scriptAddr);

	BYTE_004177f6 = sv1;
	PTR_004173e8 = sv2;
	DAT_0041722c = sv3;
	DAT_00417228 = sv4 ;
	DAT_00417224 = sv5;
	DAT_00417220 = sv6;
	_curObjIndex = sv7;
	PTR_00417218 = sv8;
	PTR_00417214 = sv9;
}

bool GamosEngine::FUN_00402fb4()
{
	if (_drawElements.empty())
		return true;

	Object *pobj = DAT_00412204;
	if (!pobj)
		pobj = &(_drawElements.front());

	for (int32 objIdx = pobj->index; objIdx < _drawElements.size(); objIdx++) {
		pobj = &_drawElements[objIdx];

		if ((pobj->flags & 3) == 3) {
			if (!PTR_00417388 || (PTR_00417388[ pobj->actID >> 3 ] & (1 << (pobj->actID & 7))) ) {
				if (pobj->fld_3 & 1) {
					pobj->fld_3 &= ~1;
				} else {
					if ((pobj->flags & 4) == 0) {
						if (pobj->y != -1 && FUN_00402f34(true, false, &_drawElements[pobj->y])) {
							pobj->y = pobj->x;
							if (pobj->x != -1) {
								Object &o = _drawElements[pobj->x];
								o.flags |= 0x80;
								o.fld_4 = pobj->pos;
								o.fld_5 = pobj->blk;
								FUN_0040921c(&o);
								addDirtRectOnObject(&o);
							}
						}
					} else {
						if (FUN_00402f34(pobj->y != pobj->x, true, &_drawElements[pobj->y])) {
							pobj->y = pobj->x;
							if (pobj->x != -1) {
								Object &o = _drawElements[pobj->x];
								o.flags |= 0x80;
								o.fld_4 = pobj->pos;
								o.fld_5 = pobj->blk;
								FUN_0040921c(&o);
								addDirtRectOnObject(&o);
							}
							pobj->flags &= ~4;
						} else {
							if (pobj == DAT_00412204) {
								goto exit;
							}
							goto continue_to_next_object;
						}
					}

					PTR_00417218 = pobj;
					_curObjIndex = pobj->index;
					PTR_00417214 = &_someActsArr[pobj->actID];
					PTR_004173e8 = pobj->storage.data();

					//DAT_00417804 = 0;
					for ( ScriptS &scr: PTR_00417214->scriptS ) {
						BYTE_004177f6 = PTR_00417218->flags & 0xf0;

						int ivr8 = 0;
						if (BYTE_004177f6 == 0x20)
							ivr8 = 1;
						else if (BYTE_004177f6 == 0x40)
							ivr8 = 2;
						else if (BYTE_004177f6 == 0x80)
							ivr8 = 3;

						bool tmp = false;
						for (int i = 0; i < 8; i++) {
							if ((PTR_00417214->unk1 >> 8) & (1 << i)) {
								//DAT_004173ec = ((i & 3) + ivr8) & 3;
								int fncid = ((i & 3) + ivr8) & 3;
								if (i > 3)
									fncid += 4;

								DAT_004177ff = false;
								_preprocDataId = fncid;
								int32 res = ProcessScript(false, scr.data.data(), scr.data.size(), scr.codes1, scr.codes2);

								if (_needReload)
									return false;

								if (res != 0) {
									if (res != 1) {
										if (DAT_00412204) {
											DAT_00412204 = nullptr;
											goto exit;
										}
										FUN_0040255c(pobj);
										goto continue_to_next_object;
									}
									if (!DAT_004177ff) {
										if (DAT_00412204) {
											DAT_00412204 = nullptr;
											goto exit;
										}
										goto continue_to_next_object;
									}
									tmp = true;
									break;
								}
							}
						}

						if (scr.data[0] & 0x80) {
							if (tmp) {
								DAT_00412204 = pobj;
								goto exit;
							}

							if (DAT_00412204) {
								DAT_00412204 = nullptr;
								goto exit;
							}

							break;
						}
					}
				}
			}
		} else {
			if (!PTR_00417388 && (pobj->flags & 0x83) == 0x81 && pobj->pos == 0xff && pobj->blk == 0xff)
				FUN_00402f34(true, false, pobj);
		}
continue_to_next_object:
		;
	}

exit:
	PTR_00417218 = nullptr;
	_curObjIndex = -1;
	return true;
}

bool GamosEngine::FUN_00402f34(bool p1, bool p2, Object *obj) {
	if (obj->fld_2 < 2) {
		if (p2 || (obj->flags & 4)) {
			addDirtRectOnObject(obj);
			if (p1)
				removeObject(obj);
			return true;
		}
	} else {
		addDirtRectOnObject(obj);
		obj->actID++;
		if (obj->actID == obj->fld_2) {
			obj->actID = 0;
			obj->pImg = obj->pImg - (obj->fld_2 - 1);
			if (p2 || (obj->flags & 4)) {
				addDirtRectOnObject(obj);
				if (p1)
					removeObject(obj);
				return true;
			}
		} else {
			obj->pImg++;
		}

		if ((obj->flags & 0x40) == 0)
			FUN_0040921c(obj);

		addDirtRectOnObject(obj);
	}
	return false;
}

void GamosEngine::FUN_0040921c(Object *obj) {
	ImagePos *imgPos = obj->pImg;
	Image *img = imgPos->image;

	int32 x = obj->fld_4 * _gridCellW;
	int32 y = obj->fld_5 * _gridCellH;

	if (obj->pos != 255 && obj->blk != 255) {
		Object *o = &_drawElements[(obj->blk * 0x100) + obj->pos];
		if (o->flags & 4) {
			int t = obj->actID + 1;
			x += (o->pos - obj->fld_4) * _gridCellW * t / obj->fld_2;
			y += (o->blk - obj->fld_5) * _gridCellH * t / obj->fld_2;
		}
	}

	if (obj->flags & 8)
		obj->x = x - (img->surface.w - _gridCellW - imgPos->xoffset);
	else
		obj->x = x - imgPos->xoffset;

	if (obj->flags & 0x10)
		obj->y = y - (img->surface.h - _gridCellH - imgPos->yoffset);
	else
		obj->y = y - imgPos->yoffset;
}

void GamosEngine::addDirtRectOnObject(Object *obj) {
	ImagePos *imgPos = obj->pImg;
	Common::Rect rect;
	rect.left = obj->x;
	rect.top = obj->y;
	if (obj->flags & 0x40) {
		rect.left -= imgPos->xoffset;
		rect.top -= imgPos->yoffset;
	}
	rect.right = rect.left + imgPos->image->surface.w;
	rect.bottom = rect.top + imgPos->image->surface.h;

	addDirtyRect(rect);
}

void GamosEngine::addDirtyRect(const Common::Rect &rect) {
	if (_dirtyRects.empty()) {
		_dirtyRects.push_back(rect);
		return;
	}

	int flags = 0;

	for(int i = 0; i < _dirtyRects.size(); i++) {
		Common::Rect &r = _dirtyRects[i];
		if (!rect.intersects(r))
			continue;

		flags |= 1;

		if (rect.left < r.left) {
			r.left = rect.left;
			flags |= 2;
		}
		if (rect.right > r.right) {
			r.right = rect.right;
			flags |= 2;
		}
		if (rect.top < r.top) {
			r.top = rect.top;
			flags |= 2;
		}
		if (rect.bottom > r.bottom) {
			r.bottom = rect.bottom;
			flags |= 2;
		}
		break;
	}

	if (flags == 0) {
		_dirtyRects.push_back(rect);
		return;
	}

	if ( !(flags & 2) )
		return;

	rerunCheck:
	for(int i = _dirtyRects.size() - 2; i > 0; i--) {
		for (int j = _dirtyRects.size() - 1; j > i; j--) {
			Common::Rect &r1 = _dirtyRects[i];
			Common::Rect &r2 = _dirtyRects[j];
			if (!r1.intersects(r2))
				continue;

			r1.extend(r2);
			_dirtyRects.remove_at(j);
			goto rerunCheck;
		}
	}
}

void GamosEngine::doDraw() {
	if (_dirtyRects.empty())
		return;

	int32 bkg = _readingBkgMainId;
	if (bkg == -1)
		bkg = 0;

	Common::Array<Object *> drawList( 1024 );//_drawElements.size(), 1024) );

	int cnt = 0;
	for (int i = 0; i < _drawElements.size(); i++) {
		Object &obj = _drawElements[i];
		if ((obj.flags & 0x83) == 0x81) {
			drawList[cnt] = &obj;
			cnt++;
		}
	}

	if (_unk9 == 0 /*&& */) {
		/*drawList[cnt] = &_cursorObject;
		cnt++;*/
	}

	drawList.resize(cnt);

	if (cnt) {
		for (int i = 0; i < drawList.size() - 1; i++) {
			for (int j = i + 1; j < drawList.size(); j++) {
				Object *o1 = drawList[i];
				Object *o2 = drawList[j];
				if (o1->fld_3 < o2->fld_3) {
					drawList[i] = o2;
					drawList[j] = o1;
				}
			}
		}
	}

	/* add mouse cursor here*/

	for (int i = 0; i < _dirtyRects.size(); i++) {
		Common::Rect &r = _dirtyRects[i];

		if (_bkgImages[bkg].loaded) {
			_screen->blitFrom(_bkgImages[bkg].surface, r, r);
		}
		_screen->addDirtyRect(r);
	}

	for(Object *o: drawList) {
		if (o->pImg && loadImage(o->pImg->image))
			_screen->blitFrom(o->pImg->image->surface, Common::Point(o->x, o->y));
	}

	_screen->update();
}

bool GamosEngine::loadImage(Image *img) {
	if (img->loaded)
		return true;

	if (img->offset < 0)
		return false;

	_arch.seek(img->offset, 0);

	if (img->cSize == 0) {
		img->rawData.resize((img->surface.w * img->surface.h + 16) & ~0xf);

		_arch.read(img->rawData.data(), img->surface.w * img->surface.h);
		img->surface.setPixels(img->rawData.data());
	} else {
		img->rawData.resize((img->surface.w * img->surface.h + 4 + 16) & ~0xf);

		RawData tmp(img->cSize);
		_arch.read(tmp.data(), tmp.size());
		_arch.decompress(&tmp, &img->rawData);
		img->surface.setPixels(img->rawData.data() + 4);
	}

	img->surface.format = Graphics::PixelFormat::createFormatCLUT8();
	img->loaded = true;
	return true;
}

uint32 GamosEngine::doScript(uint32 scriptAddress) {
	uint32 res = VM::doScript(scriptAddress, PTR_004173e8);
	return res;
}


void GamosEngine::vmCallDispatcher(VM *vm, uint32 funcID) {
	uint32 arg1 = 0, arg2 = 0, arg3 = 0;

	switch (funcID)
	{
	case 0:
		DAT_004177ff = true;
		vm->EAX.val = 1;
		break;
	case 2:
		arg1 = vm->pop32();
		if (PTR_00417218->x == -1)
			vm->EAX.val = 0;
		else
			vm->EAX.val = _drawElements[ PTR_00417218->x ].spr->index == arg1 ? 1 : 0;
		break;
	case 3:
		//warning("func 3 %x check 0x10", PTR_00417218->fld_4 & 0x90);
		vm->EAX.val = (PTR_00417218->fld_4 & 0x90) == 0x10 ? 1 : 0;
		break;
	case 4:
		//warning("func 4 %x check 0x20", PTR_00417218->fld_4 & 0xa0);
		vm->EAX.val = (PTR_00417218->fld_4 & 0xa0) == 0x20 ? 1 : 0;
		break;
	case 5:
		arg1 = vm->pop32();
		//printf("func 5 %x check %x \n", PTR_00417218->fld_4 & 0xb0, arg1);
		vm->EAX.val = (PTR_00417218->fld_4 & 0xb0) == arg1 ? 1 : 0;
		break;
	case 6:
		arg1 = vm->pop32();
		//warning("func 6 %x check %x", PTR_00417218->fld_4 & 0x4f, arg1);
		vm->EAX.val = (PTR_00417218->fld_4 & 0x4f) == arg1 ? 1 : 0;
		break;
	case 9:
		arg1 = vm->pop32();
		vm->EAX.val = FUN_004070f8(_dat60[arg1].data(), _dat60[arg1].size());
		break;
	case 13: {
		VM::Reg regRef = vm->popReg(); //implement
		Common::String str = vm->getString(regRef.ref, regRef.val);
		//warning("CallDispatcher 13 keycode %s", str.c_str());
		vm->EAX.val = 0;
		break;
	}

	case 14:
		arg1 = vm->pop32();
		loadModule(arg1);
		setNeedReload();
		vm->EAX.val = 1;
		break;

	case 16:
		arg1 = vm->pop32();
		vm->EAX.val = scriptFunc16(arg1);
		break;

	case 17:
		arg1 = vm->pop32();
		playSound(arg1);
		vm->EAX.val = 1;
		break;

	case 19:
		arg1 = vm->pop32();
		vm->EAX.val = scriptFunc19(arg1);
		break;

	case 20: {
		arg1 = vm->pop32();
		for (const Dat61 &d : _dat61[arg1]) {
			FUN_0040738c(d.v, d.x, d.y, true);
		}
		vm->EAX.val = FUN_004070f8(_dat60[arg1].data(), _dat60[arg1].size());
	} break;

	case 24: {
		VM::Reg regRef = vm->popReg();
		arg2 = vm->pop32();
		const Dat61 &d = _dat61[arg2][0];
		FUN_00407a68(vm, regRef.ref, regRef.val, d.v, d.x, d.y);

		vm->EAX.val = 1;
	} break;

	case 25: {
		arg1 = vm->pop32();
		if (PTR_00417218->fld_5 != arg1) {
			PTR_00417218->fld_5 = arg1;
			if (PTR_00417218->x != -1) {
				Object &obj = _drawElements[PTR_00417218->x];
				obj.fld_3 = arg1;
			}
			if (PTR_00417218->y != -1) {
				Object &obj = _drawElements[PTR_00417218->y];
				obj.fld_3 = arg1;
				addDirtRectOnObject(&obj);
			}
		}
		vm->EAX.val = 1;
	} break;

	case 26:
		FUN_004023d8(PTR_00417218);
		vm->EAX.val = 1;
		break;

	case 30: {
		if (PTR_00417218->y != -1) {
			Object *obj = &_drawElements[PTR_00417218->y];
			PTR_00417218->x = -1;
			PTR_00417218->y = -1;
			removeObjectMarkDirty(obj);
		}
	} break;

	case 31:
		arg1 = vm->pop32();
		setCursor(arg1, true);
		vm->EAX.val = 1;
		break;

	case 32:
		setCursor(0, false);
		vm->EAX.val = 1;
		break;

	case 33:
		PTR_00417218->fld_5 = _statesHeight - PTR_00417218->blk;
		vm->EAX.val = 1;
		break;

	case 47: {
		arg1 = vm->pop32();

		switch (arg1) {
		case 0:
			vm->EAX.val = _d2_fld16 != 0 ? 1 : 0;
			break;

		case 1:
			vm->EAX.val = _d2_fld14 != 0 ? 1 : 0;
			break;

		case 2:
			vm->EAX.val = 1; //BYTE_004177fb != 0 ? 1 : 0;
			break;

		case 3:
			vm->EAX.val = _d2_fld17 != 0 ? 1 : 0;
			break;

		case 4:
			vm->EAX.val = _d2_fld18 != 0 ? 1 : 0;
			break;

		default:
			break;
		}
	} break;

	case 49: {
		arg1 = vm->pop32();
		arg2 = vm->pop32();

		warning("Do save-load %d %d", arg1, arg2);
	} break;

	case 54:
		arg1 = vm->pop32();
		vm->EAX.val = rndRange16(arg1);
		break;

	case 55: {
		VM::Reg regRef = vm->popReg(); //implement
		Common::String str = vm->getString(regRef.ref, regRef.val);
		warning("PlayMovie 55: %s", str.c_str());
		vm->EAX.val = 1;
	} break;

	case 57: {
		VM::Reg regRef = vm->popReg(); //implement
		Common::String str = vm->getString(regRef.ref, regRef.val);
		warning("CallDispatcher 57 keycode %s", str.c_str());
		vm->EAX.val = 0;
	} break;

	default:
		warning("Call Dispatcher %d", funcID);
		vm->EAX.val = 0;
		break;
	}
}

void GamosEngine::callbackVMCallDispatcher(void *engine, VM *vm, uint32 funcID) {
	GamosEngine *gamos = (GamosEngine *)engine;
	gamos->vmCallDispatcher(vm, funcID);
}

uint32 GamosEngine::scriptFunc19(uint32 id) {
	BYTE_004177fc = 1;
	FUN_0040738c(id, DAT_00417220 * _gridCellW, DAT_00417224 * _gridCellH, false);

	return 1;
}

uint32 GamosEngine::scriptFunc16(uint32 id) {
	if (DAT_004177f8 == 0) {
		stopMidi();
		if (_enableMidi) {
			_midiTrack = id;
			if (id >= _midiTracks.size())
				return 0;

			return playMidi(&_midiTracks[id]) ? 1 : 0;
		}
	}
	return 1;
}


bool GamosEngine::FUN_0040738c(uint32 id, int32 x, int32 y, bool p) {
	Sprite &spr = _sprites[id];
	Object *pobj = getFreeObject();

	pobj->flags |= 0x80;

	if (spr.field_1 & 1)
		pobj->flags |= 4;

	pobj->fld_2 = spr.field_3;
	int32 idx = 0xffff;
	if (!p)
		idx = _curObjIndex;

	pobj->pos = idx & 0xff;
	pobj->blk = (idx >> 8) & 0xff;
	pobj->x = x;
	pobj->y = y;

	if (!p) {
		if (!PTR_00417218) {
			pobj->fld_3 = (PTR_00417214->unk1 >> 16) & 0xFF;
		} else {
			int32 index = pobj->index;
			if (PTR_00417218->y != -1) {
				Object *oobj = &_drawElements[PTR_00417218->y];
				addDirtRectOnObject(oobj);
				oobj->flags &= 0x7f;
				if (PTR_00417218->x != PTR_00417218->y)
					removeObject(oobj);
			}

			PTR_00417218->y = index;
			if (!(pobj->flags & 4)) {
				if (PTR_00417218->x != -1)
					removeObject(&_drawElements[PTR_00417218->x]);
				PTR_00417218->x = index;
			}

			pobj->fld_3 = PTR_00417218->fld_5;
			if (DAT_00417220 != DAT_00417228 || DAT_00417224 != DAT_0041722c) {
				PTR_00417218->flags |= 4;
			}
		}
	} else {
		pobj->fld_3 = PTR_00417218->fld_5;
		pobj->fld_4 = 0xff;
		pobj->fld_5 = 0xff;
	}

	FUN_00409378(&spr, pobj, p);
	return true;
}

void GamosEngine::FUN_00409378(Sprite *spr, Object *obj, bool p) {
	obj->flags &= ~0x18;
	obj->actID = 0;
	obj->spr = spr;

	if (spr->field_2 == 1) {
		obj->pImg = &spr->sequences[0][0];
		if (BYTE_004177f6 == 0x80) {
			if (spr->field_1 & 2)
				obj->flags |= 8;
		} else if (BYTE_004177f6 == 0x40 && (spr->field_1 & 4)) {
			obj->flags |= 0x10;
		}
	} else {
		int frm = 0;
		if (BYTE_004177f6 == 0x10) {
			frm = 1;
			if (DAT_00417224 == DAT_0041722c && (spr->field_1 & 8))
				frm = 0;
		} else if (BYTE_004177f6 == 0x20) {
			frm = 3;
			if (DAT_0041722c < DAT_00417224)
				frm = 2;
			else if (DAT_0041722c > DAT_00417224) {
				frm = 4;
				if (spr->field_1 & 4) {
					frm = 2;
					obj->flags |= 0x10;
				}
			} else if (DAT_00417220 == DAT_00417228 && (spr->field_1 & 8))
				frm = 0;
		} else if (BYTE_004177f6 == 0x40) {
			frm = 5;
			if (DAT_00417224 == DAT_0041722c && (spr->field_1 & 8))
				frm = 0;
			else if (spr->field_1 & 4) {
				frm = 1;
				obj->flags |= 0x10;
			}
		} else {
			frm = 7;
			if (DAT_00417224 == DAT_0041722c) {
				if ((spr->field_1 & 8) && DAT_00417220 == DAT_00417228)
					frm = 0;
				else if (spr->field_1 & 2) {
					frm = 3;
					obj->flags |= 8;
				}
			} else {
				if (DAT_0041722c < DAT_00417224) {
					frm = 8;
					if (spr->field_1 & 2) {
						frm = 2;
						obj->flags |= 8;
					}
				} else {
					frm = 6;
					if (spr->field_1 & 4) {
						frm = 8;
						obj->flags |= 0x10;

						if (spr->field_1 & 2) {
							frm = 2;
							obj->flags |= 8;
						}
					} else if (spr->field_1 & 2) {
						frm = 4;
						obj->flags |= 8;
					}
				}
			}
		}

		obj->pImg = &spr->sequences[frm][0];
	}
	if (!p) {
		obj->fld_4 = DAT_00417228;
		obj->fld_5 = DAT_0041722c;
		FUN_0040921c(obj);
	} else {
		obj->flags |= 0x40;
	}

	addDirtRectOnObject(obj);
}

void GamosEngine::FUN_004095a0(Object *obj) {
	if (obj->y != -1) {
		Object &yobj = _drawElements[obj->y];
		Sprite *spr = yobj.spr; //getSprite
		addDirtRectOnObject(&yobj);
		if (DAT_00417228 != DAT_00417220 || DAT_0041722c != DAT_00417224)
			obj->flags |= 4;
		FUN_00409378(spr, &yobj, false);
	}
}

void GamosEngine::FUN_004023d8(Object *obj) {
	if (obj->fld_3 & 2) {
		obj->fld_3 &= ~2;
		int32 index = obj->index;
		for (int index = obj->index; index < _drawElements.size(); index++) {
			Object *pobj = &_drawElements[index];
			if ((pobj->flags & 0xe3) == 0xe1 && ((pobj->blk << 8) | pobj->pos) == obj->index)
				removeObjectMarkDirty(pobj);
		}
	}
}

void GamosEngine::FUN_0040255c(Object *obj) {
	if (obj == PTR_004121b4) {
		PTR_004121b4 = nullptr;
		int32 n = 0;
		for (int32 i = 0; i < _drawElements.size(); i++) {
			Object &robj = _drawElements[i];

			if (robj.index > obj->index)
				n++;

			if ( (robj.flags & 3) == 3 && (_someActsArr[robj.actID].unk1 & 0xff) == 3 ) {
				if (n) {
					PTR_004121b4 = &robj;
					break;
				}
				if (!PTR_004121b4)
					PTR_004121b4 = &robj;
			}
		}
	}
}

void GamosEngine::setCursor(int id, bool dirtRect) {
	if (_unk9 == 0) {
		if (dirtRect && _cursorObject.spr)
			addDirtRectOnObject(&_cursorObject);

		_mouseCursorImgId = id;

		_cursorObject.spr = &_sprites[id];
		_cursorObject.flags = 0xc1;
		_cursorObject.fld_2 = _sprites[id].field_3; // max frames
		_cursorObject.actID = 0; //frame
		_cursorObject.x = 0;
		_cursorObject.y = 0;
		_cursorObject.pImg = &_sprites[id].sequences[0][0];

		g_system->setMouseCursor(_cursorObject.pImg->image->surface.getPixels(),
								 _cursorObject.pImg->image->surface.w,
								 _cursorObject.pImg->image->surface.h,
								 _cursorObject.pImg->xoffset,
								 _cursorObject.pImg->yoffset, 0);
	}
}


bool GamosEngine::FUN_00409600(Object *obj, Common::Point pos) {
	if (obj->y == -1)
		return false;

	Object &robj = _drawElements[obj->y];
	if (Common::Rect(robj.x, robj.y, robj.x + robj.pImg->image->surface.w, robj.y + robj.pImg->image->surface.h).contains(pos))
		return true;
	return false;
}

void GamosEngine::FUN_00402c2c(Common::Point move, Common::Point actPos, uint8 act2, uint8 act1) {
	uint8 tmpb = 0;
	if (act2 == ACT2_8f)
		FUN_0040255c(PTR_004121b4);
	else if (act2 == ACT2_82)
		tmpb = 0x90;
	else if (act2 == ACT2_83)
		tmpb = 0xa0;
	else if (act2 == ACT_NONE)
		actPos = move;

	if (act1 != 0xe)
		tmpb |= act1 | 0x40;

	//actPos +=
	warning("Not full FUN_00402c2c");

	Object *pobj = nullptr;
	uint8 actT = 0;
	uint8 pobjF5 = 0xff;

	for(int i = 0; i < _drawElements.size(); i++) {
		Object &obj = _drawElements[i];
		if ((obj.flags & 3) == 3) {
			SomeAction &action = _someActsArr[obj.actID];
			uint8 tp = action.unk1 & 0xff;
			if (tp == 1)
				obj.fld_4 = tmpb;
			else if (tp == 2)
				obj.fld_4 = tmpb & 0x4f;
			else if (tp == 3) {
				if (&obj == PTR_004121b4)
					obj.fld_4 = tmpb & 0x4f;
				else
					obj.fld_4 = 0;
			}

			if ((!pobj || obj.fld_5 <= pobjF5) && FUN_00409600(&obj, actPos)) {
				actT = tp;
				pobjF5 = obj.fld_5;
				pobj = &obj;
			}
		}
	}

	if (!pobj) {
		DAT_004173f4 = actPos.x / _gridCellW;
		DAT_004173f0 = actPos.y / _gridCellH;
		DAT_00417803 = _states.at(DAT_004173f4, DAT_004173f0) & 0xff;
	} else {
		DAT_00417803 = actT;
		if (actT == 2) {
			if (act2 == ACT_NONE)
				tmpb |= 0x10;
			else if (act2 == ACT2_81)
				tmpb |= 0x20;

			pobj->fld_4 = tmpb;
		} else if (actT == 3 && (tmpb == 0x90 || tmpb == 0xa0)) {
			PTR_004121b4 = pobj;
			pobj->fld_4 = tmpb;
		}

		DAT_004173f4 = pobj->pos;
		DAT_004173f0 = pobj->blk;
	}

	DAT_00417805 = act2;
	if (act2 == ACT2_82 || act2 == ACT2_83) {
		DAT_004177fe = act2;
		DAT_004177fd = DAT_00417803;
		DAT_004173fc = DAT_004173f4;
		DAT_004173f8 = DAT_004173f0;
	} else {
		if (act2 == ACT2_81)
			DAT_004177fe = ACT_NONE;
		DAT_00417805 = ACT_NONE;
	}

}

uint32 GamosEngine::FUN_004070f8(const byte *data, size_t dataSize) {
	uint8 sv1 = BYTE_004177fc;
	uint8 sv2 = BYTE_004177f6;
	byte *sv3 = PTR_004173e8;
	//uVar11 = DAT_0041723c;
	//uVar10 = DAT_00417238;
	uint8 sv6 = _preprocDataId;
	int32 sv7 = DAT_0041722c;
	int32 sv8 = DAT_00417228;
	int32 sv9 = DAT_00417224;
	int32 sv10 = DAT_00417220;
	int sv11 = _curObjIndex;
	Object *sv12 = PTR_00417218;
	SomeAction *sv13 = PTR_00417214;

	uint32 res = ProcessScript(true, data, dataSize);

	BYTE_004177fc = sv1;
	BYTE_004177f6 = sv2;
	PTR_004173e8 = sv3;
	//DAT_0041723c = uVar11;
	//DAT_00417238 = uVar10;
	_preprocDataId = sv6;
	DAT_0041722c = sv7;
	DAT_00417228 = sv8;
	DAT_00417224 = sv9;
	DAT_00417220 = sv10;
	_curObjIndex = sv11;
	PTR_00417218 = sv12;
	PTR_00417214 = sv13;

	return res;
}

void GamosEngine::FUN_00407a68(VM *vm, byte memtype, int32 offset, int32 val, int32 x, int32 y) {
	FUN_004023d8(PTR_00417218);
	PTR_00417218->fld_3 |= 2;

	while (true) {
		byte ib = vm->getMem8(memtype, offset);
		offset++;

		if (ib == 0)
			break;

		if (ib == 0xf) {
			byte flg = vm->getMem8(memtype, offset);
			offset++;
			byte b2 = vm->getMem8(memtype, offset);
			offset++;

			if ((flg & 0x70) == 0x20) {
				byte funcid = vm->getMem8(memtype, offset);
				offset++;
				warning("CHECKIT and write funcid %d", funcid);
			} else {
				if ((flg & 0x70) == 0 || (flg & 0x70) == 0x10) {
					int32 boff = 0;
					byte btp = VM::REF_EDI;

					if ((flg & 0x70) == 0x10)
						btp = VM::REF_EBX;

					if ((flg & 0x80) == 0) {
						boff = vm->getMem8(memtype, offset);
						offset++;
					} else {
						boff = vm->getMem32(memtype, offset);
						offset += 4;
					}

					warning("FUN_00407a68 unimplemented part");

					switch( flg & 7 ) {
					case 0:
						break;

					case 1:
						break;

					case 2:
						break;

					case 3:
						break;

					case 4:
						break;

					case 5:
						break;
					}
				}
			}
		} else {
			FUN_00407588(ib, val, &x, y);
		}
	}

}

Object *GamosEngine::FUN_00407588(int32 seq, int32 spr, int32 *pX, int32 y) {
	Object *obj = getFreeObject();
	obj->flags |= 0xe0;
	obj->actID = 0;
	obj->fld_2 = 1;
	obj->fld_3 = PTR_00417218->fld_5;
	obj->fld_4 = 0xff;
	obj->fld_5 = 0xff;
	obj->pos = _curObjIndex & 0xff;
	obj->blk = (_curObjIndex >> 8) & 0xff;
	obj->x = *pX;
	obj->y = y;
	obj->spr = &_sprites[spr];
	obj->pImg = &_sprites[spr].sequences[0][seq - _sprites[spr].field_1];

	*pX += obj->pImg->image->surface.w - obj->pImg->xoffset;

	addDirtRectOnObject(obj);
	return obj;
}

bool GamosEngine::FUN_00402bc4() {
	if (RawKeyCode == ACT_NONE) {
		VM::memory().setU8(_addrKeyCode, 0);
		VM::memory().setU8(_addrKeyDown, 0);
	} else {
		VM::memory().setU8(_addrKeyCode, RawKeyCode);
		VM::memory().setU8(_addrKeyDown, 1);
	}

	if (VM::memory().getU8(_addrBlk12) != 0)
		return false;

	uint32 frameval = VM::memory().getU32(_addrCurrentFrame);
	VM::memory().setU32(_addrCurrentFrame, frameval + 1);

	uint8 fpsval = VM::memory().getU8(_addrFPS);

	if (fpsval == 0) {
		fpsval = 1;
		VM::memory().setU8(_addrFPS, 1);
	} else if (fpsval > 50) {
		fpsval = 50;
		VM::memory().setU8(_addrFPS, 50);
	}

	if (fpsval != _fps) {
		_fps = fpsval;
		setFPS(_fps);
	}

	return true;
}


void GamosEngine::FUN_00404fcc(int32 id) {
	warning("Not implemented FUN_00404fcc");
}

bool GamosEngine::FUN_004033a8(Common::Point mouseMove) {
	_cursorObject.x = mouseMove.x;
	_cursorObject.y = mouseMove.y;
	warning("Not implemented FUN_004033a8");
	return true;
}

bool GamosEngine::FUN_004038b8() {
	warning("Not implemented FUN_004038b8");
	return true;
}

void GamosEngine::dumpActions() {
	Common::String t = Common::String::format("actions_%d.txt", _currentModuleID);
	FILE *f = fopen(t.c_str(), "wb");
	int i = 0;
	for (SomeAction &act : _someActsArr) {
		fprintf(f, "Act %d : %x\n", i, act.unk1);
		if (act.script1 != -1) {
			t = VM::disassembly(act.script1);
			fprintf(f, "Script1 : \n");
			fwrite(t.c_str(), t.size(), 1, f);
			fprintf(f, "\n");
		}

		if (act.script2 != -1) {
			t = VM::disassembly(act.script2);
			fprintf(f, "Script2 : \n");
			fwrite(t.c_str(), t.size(), 1, f);
			fprintf(f, "\n");
		}

		int j = 0;
		for (ScriptS &sc : act.scriptS) {
			fprintf(f, "subscript %d : \n", j);

			if (sc.codes1 != -1) {
				t = VM::disassembly(sc.codes1);
				fprintf(f, "condition : \n");
				fwrite(t.c_str(), t.size(), 1, f);
				fprintf(f, "\n");
			}

			if (sc.codes2 != -1) {
				t = VM::disassembly(sc.codes2);
				fprintf(f, "action : \n");
				fwrite(t.c_str(), t.size(), 1, f);
				fprintf(f, "\n");
			}

			j++;
		}


		fprintf(f, "\n\n#############################################\n\n");

		i++;
	}
}


} // End of namespace Gamos

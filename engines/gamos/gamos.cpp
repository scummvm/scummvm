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

			Actions acts;
			acts.parse(data, dataSize);
			doActions(acts, true);

			if (_needReload)
				warning("needs reload from loadResHandler, CANT HAPPEN!");

			DAT_004177f8 = 0;
			FUN_00404fcc(pid);
		}
	} else if (tp == RESTP_20) {
		if (dataSize != 4)
			return false;
		_objectActions[pid].unk1 = getU32(data);
	} else if (tp == RESTP_21) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].onCreateAddress = _loadedDataSize + p3;
		//printf("RESTP_21 %x pid %d sz %x\n", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_22) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].onDeleteAddress = _loadedDataSize + p3;
		//printf("RESTP_22 %x pid %d sz %x\n", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_23) {
		if (dataSize % 4 != 0 || dataSize < 4)
			return false;
		_objectActions[pid].actions.resize(dataSize / 4);
	} else if (tp == RESTP_2A) {
		Actions &scr = _objectActions[pid].actions[p1];
		scr.parse(data, dataSize);
	} else if (tp == RESTP_2B) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].actions[p1].conditionAddress = _loadedDataSize + p3;
		//printf("RESTP_2B %x pid %d p1 %d sz %x\n", _loadedDataSize, pid, p1, dataSize);
	} else if (tp == RESTP_2C) {
		VM::writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].actions[p1].functionAddress = _loadedDataSize + p3;
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
		uint32 datSz = getU32(data) & (~3);
		_soundSamples[pid].assign(data + 4, data + 4 + datSz);
		//printf("sound  size %d\n", dataSize);
	} else if (tp == RESTP_52) {
		return loadRes52(pid, data, dataSize);
		//printf("midi  size %d\n", dataSize);
	} else if (tp == RESTP_60) {
		_subtitleActions[pid].parse(data, dataSize);
	} else if (tp == RESTP_61) {
		Common::MemoryReadStream dataStream(data, dataSize, DisposeAfterUse::NO);
		const int count = dataSize / 8;
		_subtitlePoints[pid].resize(count);

		for (int i = 0; i < count; i++) {
			SubtitlePoint &d = _subtitlePoints[pid][i];

			d.x = dataStream.readSint16LE();
			d.y = dataStream.readSint16LE();
			d.sprId = dataStream.readUint16LE();

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

	_objects.clear();

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

	_statesCount = _statesHeight * _statesWidth;
	_pathRight = _statesWidth - 1;
	_pathBottom = _statesHeight - 1;
	_pathMap.clear();
	_pathMap.resize(_statesWidth, _statesHeight);

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

	_objectActions.clear();
	_objectActions.resize(actsCount);

	_subtitleActions.clear();
	_subtitlePoints.clear();

	_subtitleActions.resize(dat6xCount);
	_subtitlePoints.resize(dat6xCount);

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

	RawKeyCode = keyCode;

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

int32 GamosEngine::doActions(const Actions &a, bool absolute) {
	Common::Array<Common::Point> ARR_00412208(512);

	if (!absolute) {
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
		BYTE_004177f6 = 1;
		_preprocDataId = 0;
		PTR_004173e8 = nullptr;
	}

	DAT_00417220 = DAT_00417228;
	DAT_00417224 = DAT_0041722c;

	int32 spos = -1;
	int32 sbuf[6];


	if (a.flags & Actions::HAS_CONDITION) {
		if (a.conditionAddress != -1) {
			if (!doScript(a.conditionAddress))
				return 0;
			if (_needReload)
				return 0;
		}
	}

	if (a.flags & Actions::HAS_ACT2) {
		bool fastSkipAll = false;
		for(const ActTypeEntry &ate : a.act_2) {

			if (ate.t == 4) {
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

			for (int i = 0; i < ate.entries.size(); i++) {
				/* use copy of entrie because it will be modified */
				ActEntry e = ate.entries[i];

				preprocessData(_preprocDataId, &e);

				uint16 fb = 0;
				if (!absolute) {
					Common::Point xy;
					xy.x = (e.x + DAT_00417220 + _statesWidth) % _statesWidth;
					xy.y = (e.y + DAT_00417224 + _statesHeight) % _statesHeight;
					fb = _states.at( xy );
				} else {
					fb = _states.at(e.x, e.y);
				}

				uint8 lb = fb & 0xff;
				uint8 hb = (fb >> 8) & 0xff;

				int cval = 0;
				int fnc = e.t;
				if ((e.flags & 1) == 0) {
					if (e.value == lb && ((hb >> 4) & e.t)) {
						cval = 2;
					}
				} else if (lb != 0xfe &&
					       (_thing2[e.value].field_0[(fb & 0xff) >> 3] & (1 << (fb & 7))) != 0) {

					if (!_thing2[e.value].field_2.empty()) {
						e.t = _thing2[e.value].field_2[lb] >> 4;
						preprocessData(fnc + 8, &e);
					}

					if ((hb >> 4) & e.t) {
						cval = 2;
					}
				}

				if ((e.flags & 2) == cval) {
					if ((e.flags & 0xc) == 0) {
						break;
					}
					if ((e.flags & 0xc) == 4)
						return 0;
					if ((e.flags & 0xc) == 8) {
						fastSkipAll = true;
						break;
					}
					ARR_00412208[ sbuf[ps] ] = Common::Point(e.x, e.y);
					sbuf[ps]++;
				} else if ( (ate.entries.size() - i) == 1 && spos > -1 && sbuf[spos * 2] == sbuf[ps]) {
					return 0;
				}
			}
		}
	}

	BYTE_00412200 = 0;

	if (a.flags & Actions::HAS_ACT4) {
		ActEntry e = a.act_4;
		preprocessData(_preprocDataId, &e);
		preprocessDataB1(e.t, &e);
		rnd();
		e.flags = a.act_4.flags;
		FUN_00402a68(e);
		if (_needReload)
			return 0;
	}

	BYTE_004177fc = 0;
	if (a.flags & Actions::HAS_FUNCTION) {
		uint32 fldsv;
		if (PTR_00417218)
			fldsv = PTR_00417218->fld_5;
		if (a.functionAddress != -1)
			doScript(a.functionAddress);
		if (_needReload)
			return 0;
		if (BYTE_004177fc == 0 && BYTE_00412200 == 0 && PTR_00417218 && PTR_00417218->fld_5 != fldsv && PTR_00417218->y != -1)
			addDirtRectOnObject( &_objects[PTR_00417218->y] );
	}

	if (BYTE_004177fc == 0 && BYTE_00412200 != 0)
		FUN_004095a0(PTR_00417218);

	int32 retval = 0;

	if (a.flags & Actions::HAS_ACT10) {
		int ivar5 = -1;
		for(const ActTypeEntry &ate : a.act_10) {
			switch (ate.t)
			{
			case 0: {
				uint16 rndval = rndRange16(a.num_act_10e);
				rnd();
				for (ActEntry e : a.act_10end[rndval]) {
					retval += processData(e, absolute);
					if (_needReload)
						return 0;
				}
			} break;

			case 1: {
				int32 num = rndRange16(ate.entries.size());
				for (int i = 0; i < ate.entries.size(); i++) {
					if (num != 0) {
						ActEntry e = ate.entries[i];
						retval += processData(e, absolute);
						if (_needReload)
							return 0;
					}
					num--;
				}

			} break;

			case 2: {
				int32 num = rndRange16(ate.entries.size());
				ActEntry e = ate.entries[num];
				retval += processData(e, absolute);
				if (_needReload)
						return 0;
			} break;

			case 3: {
				for (int i = 0; i < ate.entries.size(); i++) {
					uint16 doproc = rndRange16(2);

					if (doproc != 0) {
						ActEntry e = ate.entries[i];
						retval += processData(e, absolute);
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
				Common::Point point = ARR_00412208[ idx ];

				for (ActEntry e : ate.entries) {
					if ( Common::Point(e.x, e.y) == point ) {
						retval += processData(e, absolute);
						if (_needReload)
							return 0;
						break;
					}
				}
			} break;

			}
		}
	}

	return retval + 1;
}

uint32 GamosEngine::getU32(const void *ptr) {
	const uint8 *p = (const uint8 *)ptr;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


void GamosEngine::preprocessData(int id, ActEntry *e) {
	switch (id) {

	default:
	case 0:
		break;

	case 1:
	case 10: {
		static const uint8 lookup[16] = {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15};
		int8 tmp = e->y;
		e->y = e->x;
		e->x = -tmp;
		e->t = lookup[ e->t ];
	} break;

	case 2:
	case 12: {
		static const uint8 lookup[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
		e->y = -e->y;
		e->x = -e->x;
		e->t = lookup[ e->t ];
	} break;

	case 3:
	case 16: {
		static const uint8 lookup[16] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
		int8 tmp = e->x;
		e->x = e->y;
		e->y = -tmp;
		e->t = lookup[ e->t ];
	} break;

	case 4: {
		static const uint8 lookup[16] = {0, 1, 8, 9, 4, 5, 12, 13, 2, 3, 10, 11, 6, 7, 14, 15};
		e->x = -e->x;
		e->t = lookup[ e->t ];
	} break;

	case 5: {
		static const uint8 lookup[16] = {0, 2, 1, 3, 8, 10, 9, 11, 4, 6, 5, 7, 12, 14, 13, 15};
		int8 tmp = e->x;
		e->x = -e->y;
		e->y = -tmp;
		e->t = lookup[ e->t ];
	} break;

	case 6: {
		static const uint8 lookup[16] = {0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15};
		e->y = -e->y;
		e->t = lookup[ e->t ];
	} break;

	case 7: {
		static const uint8 lookup[16] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
		uint8 tmp = e->x;
		e->x = e->y;
		e->y = tmp;
		e->t = lookup[ e->t ];
	} break;
	}
}

void GamosEngine::preprocessDataB1(int id, ActEntry *e) {
	switch (id) {

		default:
		case 0:
			break;

		case 1:
		case 2:
		case 4:
		case 8:
			//e->t = e->t;
			break;

		case 3: {
			static const uint8 lookup[2] = {1, 2};
			e->t = lookup[rndRange16(2)];
		} break;

		case 5: {
			static const uint8 lookup[2] = {1, 4};
			e->t = lookup[rndRange16(2)];
		} break;

		case 6: {
			static const uint8 lookup[2] = {2, 4};
			e->t = lookup[rndRange16(2)];
		} break;

		case 7: {
			static const uint8 lookup[3] = {1, 2, 4};
			e->t = lookup[rndRange16(3)];
		} break;

		case 9: {
			static const uint8 lookup[2] = {1, 8};
			e->t = lookup[rndRange16(2)];
		} break;

		case 0xa: {
			static const uint8 lookup[2] = {2, 8};
			e->t = lookup[rndRange16(2)];
		} break;

		case 0xb: {
			static const uint8 lookup[3] = {1, 2, 8};
			e->t = lookup[rndRange16(3)];
		} break;

		case 0xc: {
			static const uint8 lookup[2] = {4, 8};
			e->t = lookup[rndRange16(2)];
		} break;

		case 0xd: {
			static const uint8 lookup[3] = {1, 4, 8};
			e->t = lookup[rndRange16(3)];
		} break;

		case 0xe: {
			static const uint8 lookup[3] = {2, 4, 8};
			e->t = lookup[rndRange16(3)];
		} break;

		case 0xf: {
			static const uint8 lookup[4] = {1, 2, 4, 8};
			e->t = lookup[rndRange16(4)];
		} break;
	}
}

int GamosEngine::processData(ActEntry e, bool absolute) {
	preprocessData(_preprocDataId, &e);
	if (!absolute) {
		FUN_0040283c(e,
					 (e.x + DAT_00417220 + _statesWidth) % _statesWidth,
					 (e.y + DAT_00417224 + _statesHeight) % _statesHeight );
		if (_needReload)
			return 0;
		return e.x == 0 && e.y == 0;
	} else {
		FUN_0040283c(e, e.x, e.y);
		return 0;
	}
}

void GamosEngine::FUN_00402a68(ActEntry e) {
	if (e.x != 0 || e.y != 0) {
		DAT_00417220 = (e.x + DAT_00417220 + _statesWidth) % _statesWidth;
		DAT_00417224 = (e.y + DAT_00417224 + _statesHeight) % _statesHeight;

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

	if (e.t != BYTE_004177f6) {
        BYTE_004177f6 = e.t;
        PTR_00417218->flags = (PTR_00417218->flags & 0xf) | (e.t << 4);

		uint16 &tref = _states.at(DAT_00417220, DAT_00417224);
		tref = (tref & 0xff) | (BYTE_004177f6 << 8);

        BYTE_00412200 = 1;
    }
}

void GamosEngine::FUN_0040283c(ActEntry e, int32 x, int32 y) {
	uint16 &rthing = _states.at(x, y);

	uint8 oid = e.value;

	if ((e.flags & 1) == 0) {
		if (oid == 0xfe) {
			FUN_00402654(1, y, x);
			if (_needReload)
				return;

			rthing = (e.t << 12) | (e.flags << 8) | e.value;
			return;
		}
	} else {
		Unknown1 &unk1 = _thing2[ oid ];
		uint8 index = rndRange16( unk1.field_1[0] );
		oid = unk1.field_1[ index + 1 ];
		if (!unk1.field_2.empty()) {
			byte id1 = e.t;
			e.t = unk1.field_2[ oid ] >> 4;
			preprocessData(8 + e.t, &e);
		}
	}

	preprocessDataB1(e.t, &e);

	e.flags = 0;

	rnd(); // needed?

	Object *obj = nullptr;
	int index = 0;
	byte *odat = nullptr;

	ObjectAction &act = _objectActions[oid];
	if ((act.unk1 & 0xff) == 0) {
		FUN_00402654(1, y, x);
		if (_needReload)
			return;
		obj = nullptr;
		index = -1;
		odat = nullptr;
	} else {
		FUN_00402654(0, y, x);
		if (_needReload)
			return;
		obj = getFreeObject();
		obj->flags = (e.t << 4) | 3;
		obj->actID = oid;
		obj->fld_4 = 0;
		obj->fld_5 = (act.unk1 >> 16) & 0xff;
		obj->pos = x;
		obj->blk = y;
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

	executeScript(e.t << 4, y, x, odat, index, obj, &act, act.onCreateAddress);
	rthing = (e.t << 12) | (e.flags << 8) | oid;
}

void GamosEngine::removeObjectByIDMarkDirty(int32 id) {
	if (id != -1)
		removeObjectMarkDirty(&_objects[id]);
}


void GamosEngine::FUN_00402654(int mode, int id, int pos) {
	uint16 &rthing = _states.at(pos, id);

	uint8 actid = rthing & 0xff;

	if (actid == 0xfe)
		return;

	ObjectAction &act = _objectActions[actid];
	Object *povar4 = nullptr;
	bool multidel = false;

	for(uint i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if (obj.flags & 1) {
			if (obj.flags & 2) {
				if (obj.pos == pos && obj.blk == id) {
					removeObjectByIDMarkDirty(obj.y);
					if (obj.y != obj.x)
						removeObjectByIDMarkDirty(obj.x);
					/* if (obj.flags & 8)
						obj.storage.clear(); */
					removeSubtitles(&obj);
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

	executeScript(rthing >> 8, id, pos, nullptr, -1, nullptr, &act, act.onDeleteAddress);
}

Object *GamosEngine::getFreeObject() {
	Object *obj = nullptr;
	for (uint i = 0; i < _objects.size(); i++) {
		Object &rObj = _objects[i];
		if ( (rObj.flags & 1) == 0 ) {
			obj = &rObj;
			break;
		}
	}

	if (!obj) {
		_objects.emplace_back();
		obj = &_objects.back();
		obj->index = _objects.size() - 1;
	}

	obj->flags = 1;
	obj->sprId = -1;
	obj->seqId = -1;
	obj->frame = -1;

	obj->actID = 0;
	obj->fld_2 = 0;
	obj->fld_3 = 0;
	obj->fld_4 = 0;
	obj->fld_5 = 0;
	obj->pos = 0xff;
	obj->blk = 0xff;
	obj->x = 0;
	obj->y = 0;
	obj->pImg = nullptr;
	return obj;
}

void GamosEngine::removeObject(Object *obj) {
	obj->flags = 0;
	/*if (&(_objects.back()) == obj) {
		int32 lastindex = _objects.size() - 1;
		for (int32 i = lastindex - 1; i >= 0; i--) {
			if ( _objects[i].flags & 1 ) {
				lastindex = i;
				break;
			}
		}
		_objects.resize(lastindex);
	}*/
}

void GamosEngine::removeObjectMarkDirty(Object *obj) {
	if (obj->flags & 0x80)
		addDirtRectOnObject(obj);
	removeObject(obj);
}


void GamosEngine::executeScript(uint8 p1, uint32 id, uint32 pos, byte *storage, int32 index, Object *pobj, ObjectAction *act, int32 scriptAddr) {
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
	ObjectAction *sv9 = PTR_00417214;

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
	if (_objects.empty())
		return true;

	Object *pobj = DAT_00412204;
	if (!pobj)
		pobj = &(_objects.front());

	for (int32 objIdx = pobj->index; objIdx < _objects.size(); objIdx++) {
		pobj = &_objects[objIdx];

		if ((pobj->flags & 3) == 3) {
			if (!PTR_00417388 || (PTR_00417388[ pobj->actID >> 3 ] & (1 << (pobj->actID & 7))) ) {
				if (pobj->fld_3 & 1) {
					pobj->fld_3 &= ~1;
				} else {
					if ((pobj->flags & 4) == 0) {
						if (pobj->y != -1 && FUN_00402f34(true, false, &_objects[pobj->y])) {
							pobj->y = pobj->x;
							if (pobj->x != -1) {
								Object &o = _objects[pobj->x];
								o.flags |= 0x80;
								o.fld_4 = pobj->pos;
								o.fld_5 = pobj->blk;
								FUN_0040921c(&o);
								addDirtRectOnObject(&o);
							}
						}
					} else {
						if (FUN_00402f34(pobj->y != pobj->x, true, &_objects[pobj->y])) {
							pobj->y = pobj->x;
							if (pobj->x != -1) {
								Object &o = _objects[pobj->x];
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
					PTR_00417214 = &_objectActions[pobj->actID];
					PTR_004173e8 = pobj->storage.data();

					DAT_00417804 = 0;
					for ( Actions &scr: PTR_00417214->actions ) {
						BYTE_004177f6 = PTR_00417218->flags >> 4;

						int ivr8 = 0;
						if (BYTE_004177f6 == 2)
							ivr8 = 1;
						else if (BYTE_004177f6 == 4)
							ivr8 = 2;
						else if (BYTE_004177f6 == 8)
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
								int32 res = doActions(scr, false);

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

						if (scr.flags & 0x80) {
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
		Object *o = &_objects[(obj->blk * 0x100) + obj->pos];
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
	for (int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
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
		/*if (o->pImg && loadImage(o->pImg->image)) {
			Common::Rect out(Common::Point(o->x, o->y), o->pImg->image->surface.w, o->pImg->image->surface.h);
			out.clip(_screen->getBounds());
			out.translate(-o->x, -o->y);
			_screen->copyRectToSurfaceWithKey(o->pImg->image->surface, o->x+out.left, o->y+out.top, out, 0);
		}*/
		if (o->pImg && loadImage(o->pImg->image)) {
			uint flip = 0;
			if (o->flags & 8)
				flip |= Graphics::FLIP_H;
			if (o->flags & 0x10)
				flip |= Graphics::FLIP_V;
			Blitter::blit(&o->pImg->image->surface,
						   Common::Rect(o->pImg->image->surface.w, o->pImg->image->surface.h),
		                   _screen->surfacePtr(),
						   Common::Rect(o->x, o->y, _screen->w, _screen->h), flip);
		}
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
	case 1:
		vm->EAX.val = PTR_00417218->y == -1 ? 1 : 0;
		break;

	case 2:
		arg1 = vm->pop32();
		if (PTR_00417218->x == -1)
			vm->EAX.val = 0;
		else
			vm->EAX.val = _objects[ PTR_00417218->x ].sprId == arg1 ? 1 : 0;
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
		vm->EAX.val = savedDoActions(_subtitleActions[arg1]);
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
		for (const SubtitlePoint &d : _subtitlePoints[arg1]) {
			FUN_0040738c(d.sprId, d.x, d.y, true);
		}
		vm->EAX.val = savedDoActions(_subtitleActions[arg1]);
	} break;

	case 24: {
		VM::Reg regRef = vm->popReg();
		arg2 = vm->pop32();
		const SubtitlePoint &d = _subtitlePoints[arg2][0];
		addSubtitles(vm, regRef.ref, regRef.val, d.sprId, d.x, d.y);

		vm->EAX.val = 1;
	} break;

	case 25: {
		arg1 = vm->pop32();
		if (PTR_00417218->fld_5 != arg1) {
			PTR_00417218->fld_5 = arg1;
			if (PTR_00417218->x != -1) {
				Object &obj = _objects[PTR_00417218->x];
				obj.fld_3 = arg1;
			}
			if (PTR_00417218->y != -1) {
				Object &obj = _objects[PTR_00417218->y];
				obj.fld_3 = arg1;
				addDirtRectOnObject(&obj);
			}
		}
		vm->EAX.val = 1;
	} break;

	case 26:
		removeSubtitles(PTR_00417218);
		vm->EAX.val = 1;
		break;

	case 30: {
		if (PTR_00417218->y != -1) {
			Object *obj = &_objects[PTR_00417218->y];
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

	case 35:
		arg1 = vm->pop32();

		switch (arg1)
		{
		case 3:
			FUN_00408648(0xe, 0xff, 0xff);
			break;

		case 4:
			FUN_00408648(0xe, 0xfe, 0xff);
			break;

		case 5:
			FUN_00408648(0xe, 0xfe, 0xfe);
			break;

		case 6:
			FUN_00408648(0x82, 0xff, 0xff);
			break;

		case 7:
			FUN_00408648(0x82, 0xfe, 0xff);
			break;

		case 8:
			FUN_00408648(0x82, 0xfe, 0xfe);
			break;

		case 9:
			FUN_00408648(0x83, 0xff, 0xff);
			break;

		case 10:
			FUN_00408648(0x83, 0xfe, 0xff);
			break;

		case 11:
			FUN_00408648(0x83, 0xfe, 0xfe);
			break;

		default:
			break;
		}

		break;

	case 36:
		arg1 = vm->pop32();
		arg2 = vm->pop32();

		switch (arg1)
		{
		case 1:
			FUN_00408648(0, arg2, 0xff);
			break;

		case 2:
			FUN_00408648(0, arg2, 0xfe);
			break;

		case 3:
			FUN_00408648(0xe, arg2, 0xff);
			break;

		case 4:
			FUN_00408648(0xe, arg2, 0xfe);
			break;

		case 5:
			FUN_00408648(0xe, arg2, arg2);
			break;

		case 6:
			FUN_00408648(0x82, arg2, 0xff);
			break;

		case 7:
			FUN_00408648(0x82, arg2, 0xfe);
			break;

		case 8:
			FUN_00408648(0x82, arg2, arg2);
			break;

		case 9:
			FUN_00408648(0x83, arg2, 0xff);
			break;

		case 10:
			FUN_00408648(0x83, arg2, 0xfe);
			break;

		case 11:
			FUN_00408648(0x83, arg2, arg2);
			break;

		default:
			break;
		}

		break;

	case 38:
		arg1 = vm->pop32();
		if (DAT_00417804 == 0 || (int32)arg1 != INT_00412ca0)
			vm->EAX.val = 0;
		else
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
				Object *oobj = &_objects[PTR_00417218->y];
				addDirtRectOnObject(oobj);
				oobj->flags &= 0x7f;
				if (PTR_00417218->x != PTR_00417218->y)
					removeObject(oobj);
			}

			PTR_00417218->y = index;
			if (!(pobj->flags & 4)) {
				if (PTR_00417218->x != -1)
					removeObject(&_objects[PTR_00417218->x]);
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

	FUN_00409378(id, pobj, p);
	return true;
}

void GamosEngine::FUN_00409378(int32 sprId, Object *obj, bool p) {
	obj->flags &= ~0x18;
	obj->actID = 0;
	obj->sprId = sprId;

	Sprite &spr = _sprites[sprId];

	if (spr.field_2 == 1) {
		obj->pImg = &spr.sequences[0][0];
		if (BYTE_004177f6 == 8) {
			if (spr.field_1 & 2)
				obj->flags |= 8;
		} else if (BYTE_004177f6 == 4 && (spr.field_1 & 4)) {
			obj->flags |= 0x10;
		}
	} else {
		int frm = 0;
		if (BYTE_004177f6 == 1) {
			frm = 1;
			if (DAT_00417224 == DAT_0041722c && (spr.field_1 & 8))
				frm = 0;
		} else if (BYTE_004177f6 == 2) {
			frm = 3;
			if (DAT_0041722c < DAT_00417224)
				frm = 2;
			else if (DAT_0041722c > DAT_00417224) {
				frm = 4;
				if (spr.field_1 & 4) {
					frm = 2;
					obj->flags |= 0x10;
				}
			} else if (DAT_00417220 == DAT_00417228 && (spr.field_1 & 8))
				frm = 0;
		} else if (BYTE_004177f6 == 4) {
			frm = 5;
			if (DAT_00417224 == DAT_0041722c && (spr.field_1 & 8))
				frm = 0;
			else if (spr.field_1 & 4) {
				frm = 1;
				obj->flags |= 0x10;
			}
		} else {
			frm = 7;
			if (DAT_00417224 == DAT_0041722c) {
				if ((spr.field_1 & 8) && DAT_00417220 == DAT_00417228)
					frm = 0;
				else if (spr.field_1 & 2) {
					frm = 3;
					obj->flags |= 8;
				}
			} else {
				if (DAT_0041722c < DAT_00417224) {
					frm = 8;
					if (spr.field_1 & 2) {
						frm = 2;
						obj->flags |= 8;
					}
				} else {
					frm = 6;
					if (spr.field_1 & 4) {
						frm = 8;
						obj->flags |= 0x10;

						if (spr.field_1 & 2) {
							frm = 2;
							obj->flags |= 8;
						}
					} else if (spr.field_1 & 2) {
						frm = 4;
						obj->flags |= 8;
					}
				}
			}
		}

		obj->pImg = &spr.sequences[frm][0];
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
		Object &yobj = _objects[obj->y];
		addDirtRectOnObject(&yobj);
		if (DAT_00417228 != DAT_00417220 || DAT_0041722c != DAT_00417224)
			obj->flags |= 4;
		FUN_00409378(yobj.sprId, &yobj, false);
	}
}

void GamosEngine::removeSubtitles(Object *obj) {
	if (obj->fld_3 & 2) {
		obj->fld_3 &= ~2;
		//for (int index = obj->index; index < _objects.size(); index++) {
		for (int index = 0; index < _objects.size(); index++) {
			Object *pobj = &_objects[index];
			if ((pobj->flags & 0xe3) == 0xe1 && ((pobj->blk << 8) | pobj->pos) == obj->index)
				removeObjectMarkDirty(pobj);
		}
	}
}

void GamosEngine::FUN_0040255c(Object *obj) {
	if (obj == PTR_004121b4) {
		PTR_004121b4 = nullptr;
		int32 n = 0;
		for (int32 i = 0; i < _objects.size(); i++) {
			Object &robj = _objects[i];

			if (robj.index > obj->index)
				n++;

			if ( (robj.flags & 3) == 3 && (_objectActions[robj.actID].unk1 & 0xff) == 3 ) {
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
		if (dirtRect && _cursorObject.sprId != -1)
			addDirtRectOnObject(&_cursorObject);

		_mouseCursorImgId = id;

		_cursorObject.sprId = id;
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

	Object &robj = _objects[obj->y];
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

	for(int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if ((obj.flags & 3) == 3) {
			ObjectAction &action = _objectActions[obj.actID];
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

uint32 GamosEngine::savedDoActions(const Actions &a) {
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
	ObjectAction *sv13 = PTR_00417214;

	uint32 res = doActions(a, true);

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

void GamosEngine::addSubtitles(VM *vm, byte memtype, int32 offset, int32 sprId, int32 x, int32 y) {
	removeSubtitles(PTR_00417218);
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

					warning("addSubtitles unimplemented part");

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
			addSubtitleImage(ib, sprId, &x, y);
		}
	}

}

Object *GamosEngine::addSubtitleImage(int32 frame, int32 spr, int32 *pX, int32 y) {
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
	obj->sprId = spr;
	obj->seqId = 0;
	obj->frame = frame - _sprites[spr].field_1;
	obj->pImg = &_sprites[spr].sequences[obj->seqId][obj->frame];

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

void GamosEngine::FUN_00407db8(uint8 p)
{
	if ((p == 0x82) || (p == 0x83)) {
		DAT_00412c94 = DAT_004173fc;
		DAT_00412c98 = DAT_004173f8;
	} else {
		DAT_00412c94 = DAT_004173f4;
		DAT_00412c98 = DAT_004173f0;
	}
	DAT_00412c8c = (uint)PTR_00417218->pos;
	DAT_00412c90 = (uint)PTR_00417218->blk;
	INT_00412ca0 = -1;
	INT_00412c9c = -1;
	DAT_00417804 = 0;
}

void GamosEngine::FUN_00408648(uint8 p1, uint8 p2, uint8 p3) {
	FUN_00407db8(p1);

	if (p1 == 0x82 || p1 == 0x83) {
		if (p1 != DAT_004177fe)
			return;
		if (p2 != 0xff && p2 != DAT_004177fd)
			return;
	} else {
		if (p1 != 0xe) {
			if (p3 == 0xff)
				FUN_004084bc(p2);
			else
				FUN_00408510(p2);
			return;
		}
		if (p2 != 0xff && p2 != DAT_00417803)
			return;
	}

	if (p3 == 0xff)
		FUN_00407e2c();
	else if (p3 == 0xfe)
		FUN_0040856c();
	else
		FUN_004085d8(p2);
}

void GamosEngine::FUN_004084bc(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint16 th1 = _states.at(i, j);
			if ((th1 & 0xff) != p)
				_pathMap.at(i, j) = 0;
			else
				_pathMap.at(i, j) = 2;
		}
	}
	FUN_0040841c(true);
}

void GamosEngine::FUN_00408510(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint16 th1 = _states.at(i, j);

			if ((th1 & 0xff) == 0xfe)
				_pathMap.at(i, j) = 0;
			else if ((th1 & 0xff) == p)
				_pathMap.at(i, j) = 2;
			else
				_pathMap.at(i, j) = 3;
		}
	}
	FUN_0040841c(false);
}

void GamosEngine::FUN_0040856c() {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint16 th1 = _states.at(i, j);

			if ((th1 & 0xff) == 0xfe)
				_pathMap.at(i, j) = 0;
			else
				_pathMap.at(i, j) = 3;
		}
	}
	_pathMap.at(DAT_00412c94, DAT_00412c98) = 2;
	FUN_0040841c(false);
}

void GamosEngine::FUN_004085d8(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint16 th1 = _states.at(i, j);

			if ((th1 & 0xff) == p)
				_pathMap.at(i, j) = 0;
			else
				_pathMap.at(i, j) = 3;
		}
	}
	_pathMap.at(DAT_00412c94, DAT_00412c98) = 2;
	FUN_0040841c(false);
}

void GamosEngine::FUN_0040841c(bool p) {
	_pathMap.at(DAT_00412c8c, DAT_00412c90) = 6;

	while(true) {
		byte res = FUN_004081b8(6, 4);
		if (res == 0)
			break;
		else if (res == 1) {
			if (p)
				FUN_00407e2c();
			else
				FUN_00407f70(6);
			break;
		}

		res = FUN_004081b8(4, 5);
		if (res == 0)
			break;
		else if (res == 1) {
			if (p)
				FUN_00407e2c();
			else
				FUN_00407f70(4);
			break;
		}

		res = FUN_004081b8(5, 6);
		if (res == 0)
			break;
		else if (res == 1) {
			if (p)
				FUN_00407e2c();
			else
				FUN_00407f70(5);
			break;
		}
	}
}

byte GamosEngine::FUN_00407e2c()
{
	int32 iVar2 = DAT_00412c8c - DAT_00412c94;
	if (iVar2 < 1)
		iVar2 = -iVar2;

	int32 iVar1 = DAT_00412c90 - DAT_00412c98;
	if (iVar1 < 1)
		iVar1 = -iVar1;

	if ((iVar2 == 0) && (iVar1 == 0))
		return 0;

	if ((iVar2 == 0) || (iVar1 / iVar2) > 3) {
		if (iVar1 > 1) {
			INT_00412c9c = 4;
			if (DAT_00412c98 <= DAT_00412c90)
				INT_00412c9c = 0;
		}
		INT_00412ca0 = 4;
		if (DAT_00412c98 <= DAT_00412c90)
			INT_00412ca0 = 0;
	} else if ((iVar1 == 0) || (iVar2 / iVar1) > 3) {
		if (iVar2 > 1) {
			INT_00412c9c = 2;
			if (DAT_00412c94 <= DAT_00412c8c)
				INT_00412c9c = 6;
		}
		INT_00412ca0 = 2;
		if (DAT_00412c94 <= DAT_00412c8c)
			INT_00412ca0 = 6;
	} else {
		if (DAT_00412c8c < DAT_00412c94) {
			INT_00412c9c = 3;
			if (DAT_00412c98 <= DAT_00412c90)
				INT_00412c9c = 1;
		} else {
			INT_00412c9c = 5;
			if (DAT_00412c98 <= DAT_00412c90)
				INT_00412c9c = 7;
		}

		if (iVar1 < iVar2) {
			INT_00412ca0 = 2;
			if (DAT_00412c94 <= DAT_00412c8c)
				INT_00412ca0 = 6;
		} else {
			INT_00412ca0 = 4;
			if (DAT_00412c98 <= DAT_00412c90)
				INT_00412ca0 = 0;
		}
	}

	DAT_00417804 = 1;
	return 1;
}

byte GamosEngine::FUN_00407f70(uint8 p) {
	int32 x = DAT_00412c94;
	int32 y = DAT_00412c98;
	int32 px = -1;
	int32 py = -1;

	while (true) {
		int32 xdist = DAT_00412c8c - x;
		if (xdist < 1)
			xdist = -xdist;
		int32 ydist = DAT_00412c90 - y;
		if (ydist < 1)
			ydist = -ydist;

		int32 xx = x;
		int32 yy = y;

		if (ydist < xdist) {
			if (x == 0 || _pathMap.at(x - 1, y) != p) {
				if ((x >= _pathRight) || _pathMap.at(x + 1, y) != p) {
					if ((y == 0) || _pathMap.at(x, y - 1) != p) {
						if ((y >= _pathBottom) || _pathMap.at(x, y + 1) != p) {
							return ydist;
						} else {
							yy = y + 1;
						}
					} else {
						yy = y - 1;
					}
				} else {
					xx = x + 1;
				}
			} else {
				xx = x - 1;
			}
		} else {
			if ((y == 0) || _pathMap.at(x, y - 1) != p) {
				if ((y >= _pathBottom) || _pathMap.at(x, y + 1) != p) {
					if ((x == 0) || _pathMap.at(x - 1, y) != p) {
						if (x >= _pathRight || _pathMap.at(x + 1, y) != p) {
							return ydist;
						} else {
							xx = x + 1;
						}
					} else {
						xx = x - 1;
					}
				} else {
					yy = y + 1;
				}
			} else {
				yy = y - 1;
			}
		}

		if (xx == DAT_00412c8c && yy == DAT_00412c90) {
			INT_00412ca0 = 2;
			if (x <= xx) {
				INT_00412ca0 = 6;
				if (x >= xx) {
					INT_00412ca0 = 4;
					if (y <= yy)
						INT_00412ca0 = 0;
				}
			}
			if (px != -1) {
				if (py > yy) {
					INT_00412c9c = 3;
					if (px <= xx) {
						INT_00412c9c = 5;
						if (px >= xx)
							INT_00412c9c = 4;
					}
				} else if (py < yy) {
					INT_00412c9c = 1;
					if (px <= xx) {
						INT_00412c9c = 7;
						if (px >= xx)
							INT_00412c9c = 0;
					}
				} else {
					INT_00412c9c = 2;
					if (px <= xx)
						INT_00412c9c = 6;
				}
			}
			DAT_00417804 = 1;
			return 1;
		}

		py = y;
		px = x;

		y = yy;
		x = xx;

		if (p == 4)
			p = 6;
		else if (p == 5)
			p = 4;
		else if (p == 6)
			p = 5;
	}
}

byte GamosEngine::FUN_004081b8(uint8 cv, uint8 sv) {
	uint8 ret = 0;

	for (int32 y = 0; y < _statesHeight; y++) {
		for (int32 x = 0; x < _statesWidth; x++) {
			uint8 &rval = _pathMap.at(x, y);
			if ( rval == 0) {
				if ( (x > 0 && _pathMap.at(x - 1, y) == cv) ||
					(x < _pathRight && _pathMap.at(x + 1, y) == cv) ||
					(y > 0 && _pathMap.at(x, y - 1) == cv) ||
					(y < _pathBottom && _pathMap.at(x, y + 1) == cv) ) {
					ret = sv;
					rval = sv;
				}
			} else if (rval == 2) {
				if ( (x > 0 && _pathMap.at(x - 1, y) == cv) ||
					(x < _pathRight && _pathMap.at(x + 1, y) == cv) ||
					(y > 0 && _pathMap.at(x, y - 1) == cv) ||
					(y < _pathBottom && _pathMap.at(x, y + 1) == cv) ) {
					DAT_00412c94 = x;
					DAT_00412c98 = y;
					return 1;
				}
			}
		}
	}
	return ret;
}

void Actions::parse(const byte *data, size_t dataSize) {
	Common::MemoryReadStream rstream(data, dataSize);

	/* clean first */
	act_2.clear();
	act_10.clear();
	act_10end[0].clear();
	act_10end[1].clear();
	act_10end[2].clear();

	/* start parsing */
	flags = rstream.readByte();

	uint8 tmp = rstream.readByte();
	act_4.value = 0;
	act_4.flags = 0;
	act_4.t = tmp >> 4;
	act_4.x = rstream.readSByte();
	act_4.y = rstream.readSByte();

	num_act_10e = tmp & 0x3;

	if (flags & HAS_CONDITION)
		rstream.skip(4);

	if (flags & HAS_ACT2) {
		act_2.reserve(4);

		while (true) {
			act_2.emplace_back();
			ActTypeEntry &entrie = act_2.back();

			uint16 num = rstream.readUint16LE();
			uint8 bits = rstream.readByte();

			entrie.t = rstream.readByte();
			entrie.entries.resize(num);

			for (uint16 i = 0; i < num; i++) {
				ActEntry &a = entrie.entries[i];
				a.value = rstream.readByte();
				tmp = rstream.readByte();
				a.flags = tmp & 0xf;
				a.t = tmp >> 4;
				a.x = rstream.readSByte();
				a.y = rstream.readSByte();
			}

			if (bits & 1)
				break;
		}
	}

	if (flags & HAS_FUNCTION)
		rstream.skip(4);

	if (flags & HAS_ACT10) {
		act_10.reserve(4);

		while (true) {
			act_10.emplace_back();
			ActTypeEntry &entrie = act_10.back();

			uint16 num = rstream.readUint16LE();
			uint8 f = rstream.readByte();

			entrie.t = rstream.readByte();

			if (entrie.t == 0) {
				for (int j = 0; j < num_act_10e; j++) {
					act_10end[j].resize(num);
					for (uint16 i = 0; i < num; i++) {
						ActEntry &a = act_10end[j][i];
						a.value = rstream.readByte();
						tmp = rstream.readByte();
						a.flags = tmp & 0xf;
						a.t = tmp >> 4;
						a.x = rstream.readSByte();
						a.y = rstream.readSByte();
					}

					if (num_act_10e - j > 1) {
						num = rstream.readUint16LE();
						rstream.skip(2);
					}
				}
				break;
			}

			entrie.entries.resize(num);

			for (uint16 i = 0; i < num; i++) {
				ActEntry &a = entrie.entries[i];
				a.value = rstream.readByte();
				tmp = rstream.readByte();
				a.flags = tmp & 0xf;
				a.t = tmp >> 4;
				a.x = rstream.readSByte();
				a.y = rstream.readSByte();
			}

			if (f & 1)
				break;
		}
	}
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
	for (ObjectAction &act : _objectActions) {
		fprintf(f, "Act %d : %x\n", i, act.unk1);
		if (act.onCreateAddress != -1) {
			t = VM::disassembly(act.onCreateAddress);
			fprintf(f, "Script1 : \n");
			fwrite(t.c_str(), t.size(), 1, f);
			fprintf(f, "\n");
		}

		if (act.onDeleteAddress != -1) {
			t = VM::disassembly(act.onDeleteAddress);
			fprintf(f, "Script2 : \n");
			fwrite(t.c_str(), t.size(), 1, f);
			fprintf(f, "\n");
		}

		int j = 0;
		for (Actions &sc : act.actions) {
			fprintf(f, "subscript %d : \n", j);

			if (sc.conditionAddress != -1) {
				t = VM::disassembly(sc.conditionAddress);
				fprintf(f, "condition : \n");
				fwrite(t.c_str(), t.size(), 1, f);
				fprintf(f, "\n");
			}

			if (sc.functionAddress != -1) {
				t = VM::disassembly(sc.functionAddress);
				fprintf(f, "action : \n");
				fwrite(t.c_str(), t.size(), 1, f);
				fprintf(f, "\n");
			}

			j++;
		}


		fprintf(f, "\n\n#############################################\n\n");

		i++;
	}
	fclose(f);

	warning("Actions saved into actions_%d.txt", _currentModuleID);
}


} // End of namespace Gamos

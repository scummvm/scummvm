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
#include "gamos/console.h"
#include "gamos/detection.h"
#include "gamos/gamos.h"

#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/util.h"

#include "audio/mididrv.h"
#include "audio/midiplayer.h"

namespace Gamos {

GamosEngine::GamosEngine(OSystem *syst, const GamosGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Gamos"),
	_messageProc(this),
	_vm(this, callbackVMCallDispatcher),
	_txtInputVMAccess(_vm) {}

GamosEngine::~GamosEngine() {
	freeImages();
	freeSequences();
	delete _screen;
}

uint32 GamosEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String GamosEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::String GamosEngine::getRunFile() const {
	return _gameDescription->runFile;
}

uint32 GamosEngine::getEngineVersion() const {
	return _gameDescription->engineVersion;
}

void GamosEngine::freeImages() {
	for (Image *img : _images)
		delete img;

	_images.clear();
}

void GamosEngine::freeSequences() {
	for (ImageSeq *seq : _imgSeq)
		delete seq;

	_imgSeq.clear();
}

Common::Error GamosEngine::run() {
	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	CursorMan.setDefaultArrowCursor();
	CursorMan.showMouse(true);

	init(getRunFile());

	Common::Event e;

	while (!shouldQuit()) {
		Common::Point prevMousePos = _messageProc._mouseReportedPos;

		while (_system->getEventManager()->pollEvent(e)) {
			_messageProc.processMessage(e);
		}

		uint32 curTime = _system->getMillis();
		if (curTime >= _lastTimeStamp + _delayTime) {
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
		} else {
			if (prevMousePos != _messageProc._mouseReportedPos)
				_system->updateScreen();
			_system->delayMillis(1);
		}

		//if (_delayTime)
	}

	stopSounds();
	stopMidi();
	stopMCI();

	_d2_fld17 = 1;
	_enableMidi = true;
	_d2_fld14 = 1;
	_d2_fld16 = 1;
	_runReadDataMod = true;
	writeStateFile();

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
			/*int32 needsz = dataStream.readSint32LE(); // check free mem ? */
			dataStream.skip(4);
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
			//warning("0xFF  %d %d %d", pid, p1, p2);
			if (!reuseLastResource(resType, pid, p1, p2, 0))
				return false;
		} else {
			warning("loader2 want %x", curByte);
			return false;
		}
	}

	return true;
}

bool GamosEngine::loadModule(uint id) {
	_keySeq.clear();

	if ((!_runReadDataMod && !writeStateFile()) ||
	        !_arch.seekDir(1))
		return false;

	_currentModuleID = id;
	const byte targetDir = 2 + id;

	//DAT_004126e4 = 1;
	_currentGameScreen = -1;
	_readingBkgMainId = -1;
	_countReadedBkg = 0;
	//DAT_004126ec = 0;
	//INT_004126e8 = 0;

	_xorSeq[0].clear();
	_xorSeq[1].clear();
	_xorSeq[2].clear();

	stopMidi();
	stopMCI();
	stopSounds();

	/* Complete me */

	bool prefixLoaded = false;
	byte prevByte = 0;
	bool doLoad = true;

	int32 p1 = 0;
	int32 p2 = 0;
	int32 p3 = 0;
	int32 pid = 0;

	while (doLoad) {
		byte curByte = _arch.readByte();

		switch (curByte) {
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
					_scrollY = 0;
					_scrollX = 0;
					_scrollTrackObj = -1;
					_scrollSpeed = 16;
					_scrollCutoff = 80;
					_scrollSpeedReduce = -1;
					_scrollBorderB = 0;
					_scrollBorderU = 0;
					_scrollBorderR = 0;
					_scrollBorderL = 0;
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
				_countReadedBkg++;
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
				//warning("t %x sz %x sum %x", prevByte, data.size(), _loadedDataSize);
				if (_onlyScanImage)
					_loadedDataSize += 0x10;
				else
					_loadedDataSize += datasz;
				break;

			default:
				//warning("t %x sz %x sum %x", prevByte, data.size(), _loadedDataSize);
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
			for (uint i = 0; i < sz; ++i)
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

			if ((curByte & CONFTP_IDFLG) == 0)
				pid = _arch.readPackedInt();

			break;
		}
	}

	//FUN_00404a28();
	if (BYTE_004177f7)
		return true;

	// Reverse Here
	setCursor(0, false);

	if (!loadStateFile())
		return false;

	int bkg = _readingBkgMainId;
	if (bkg == -1)
		bkg = 0;

	if (!switchToGameScreen(bkg, false))
		return false;

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

		_vm.memory().setU8(_addrBlk12, dataStream.readByte());
		dataStream.skip(1);
		_vm.memory().setU8(_addrFPS, _fps);
		_vm.memory().setU8(_addrKeyDown, dataStream.readByte());
		_vm.memory().setU8(_addrKeyCode, dataStream.readByte());
		_vm.memory().setU32(_addrCurrentFrame, dataStream.readUint32LE());

		setFPS(_fps);
	} else if (tp == RESTP_13) {
		_vm.writeMemory(_loadedDataSize, data, dataSize);
	} else if (tp == RESTP_18) {
		loadRes18(pid, data, dataSize);
	} else if (tp == RESTP_19) {
		if (BYTE_004177f7 == 0) {
			for (int i = 0; i < _states.size(); i++)
				_states.at(i) = ObjState(0xfe, 0, 0xf);

			DAT_004177f8 = 1;

			Actions acts;
			acts.parse(data, dataSize);
			doActions(acts, true);

			if (_needReload)
				warning("needs reload from loadResHandler, CANT HAPPEN!");

			DAT_004177f8 = 0;

			storeToGameScreen(pid);
		}
	} else if (tp == RESTP_20) {
		if (dataSize != 4)
			return false;
		_objectActions[pid].actType = data[0];
		_objectActions[pid].mask = data[1];
		_objectActions[pid].priority = data[2];
		_objectActions[pid].storageSize = data[3] + 1;
	} else if (tp == RESTP_21) {
		_vm.writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].onCreateAddress = _loadedDataSize + p3;
		//warning("RESTP_21 %x pid %d sz %x", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_22) {
		_vm.writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].onDeleteAddress = _loadedDataSize + p3;
		//warning("RESTP_22 %x pid %d sz %x", _loadedDataSize, pid, dataSize);
	} else if (tp == RESTP_23) {
		if (dataSize % 4 != 0 || dataSize < 4)
			return false;
		_objectActions[pid].actions.resize(dataSize / 4);
	} else if (tp == RESTP_2A) {
		Actions &scr = _objectActions[pid].actions[p1];
		scr.parse(data, dataSize);
	} else if (tp == RESTP_2B) {
		_vm.writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].actions[p1].conditionAddress = _loadedDataSize + p3;
		//warning("RESTP_2B %x pid %d p1 %d sz %x", _loadedDataSize, pid, p1, dataSize);
	} else if (tp == RESTP_2C) {
		_vm.writeMemory(_loadedDataSize, data, dataSize);
		_objectActions[pid].actions[p1].functionAddress = _loadedDataSize + p3;
		//warning("RESTP_2C %x pid %d p1 %d sz %x", _loadedDataSize, pid, p1, dataSize);
	} else if (tp == RESTP_38) {
		//warning("Data 38 size %zu", dataSize);
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
		//warning("sound  size %d", dataSize);
	} else if (tp == RESTP_52) {
		return loadRes52(pid, data, dataSize);
		//warning("midi  size %d", dataSize);
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
		_sprites[pid].sequences[p1]->operator[](p2).image = _images.back();
	} else if (tp == RESTP_42) {
		_sprites[pid].sequences[p1] = _imgSeq.back();
	} else {
		error("Reuse of resource not implemented: resource type %x, id %d %d %d %d", tp, pid, p1, p2, p3);
	}
	return true;
}


bool GamosEngine::initMainDatas() {
	RawData rawdata;

	if (!_arch.readCompressedData(&rawdata))
		return false;

	Common::MemoryReadStream dataStream(rawdata.data(), rawdata.size(), DisposeAfterUse::NO);

	_magic = dataStream.readUint32LE();

	if (_magic != getEngineVersion()) {
		error("InitMainData: Invalid engine version! get %x expecting %x", _magic, getEngineVersion());
		return false;
	}

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

	_introPos.x = dataStream.readSint32LE();
	_introPos.y = dataStream.readSint32LE();
	_introSize.x = dataStream.readSint32LE();
	_introSize.y = dataStream.readSint32LE();

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

	_savedSndVolume = !ConfMan.hasKey("sfx_volume") ? 255 : ConfMan.getInt("sfx_volume");
	_savedMidiVolume = !ConfMan.hasKey("music_volume") ? 255 : ConfMan.getInt("music_volume");
	_sndVolumeTarget = _savedSndVolume;
	_midiVolumeTarget = _savedMidiVolume;

	playVideo("intro", _introPos, _introSize);

	if (!playIntro())
		return false;

	return true;
}

bool GamosEngine::loadInitModule() {
	rndSeed(_system->getMillis());
	_curObjIndex = -1;
	PTR_00417218 = nullptr;
	PTR_00417214 = nullptr;
	_xorSeq[2].clear();
	_xorSeq[1].clear();
	_xorSeq[0].clear();
	_isMoviePlay = 0;
	_txtInputActive = false;
	//_updateMouse = false;
	_runReadDataMod = true;
	_savedSndVolume = 0;
	_savedMidiVolume = 0;
	_sndVolumeTarget = 0;
	_midiVolumeTarget = 0;
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

	freeImages();
	freeSequences();

	uint32 bkgnum1 = dataStream.readUint32LE(); // 0
	uint32 bkgnum2 = dataStream.readUint32LE(); // 4
	_statesWidth = dataStream.readUint32LE(); // 8
	_statesHeight = dataStream.readUint32LE(); // c
	_bkgSize.x = dataStream.readUint32LE(); // 10
	_bkgSize.y = dataStream.readUint32LE(); // 14
	/* bkgbufferSize */ dataStream.readUint32LE(); // 18
	uint32 actsCount = dataStream.readUint32LE(); // 1c
	uint32 unk1Count = dataStream.readUint32LE(); // 20
	uint32 imageCount = dataStream.readUint32LE(); // 24
	uint32 soundCount = dataStream.readUint32LE(); // 28
	uint32 midiCount = dataStream.readUint32LE(); // 2c
	uint32 dat6xCount = dataStream.readUint32LE(); // 30

	_statesShift = 2;
	for (int i = 2; i < 9; i++) {
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

	_gameScreens.clear();
	_gameScreens.resize(bkgnum1 * bkgnum2);

	_sprites.clear();
	_sprites.resize(imageCount);

	for (uint i = 0; i < imageCount; i++)
		_sprites[i].index = i;

	_midiTracks.clear();
	_midiTracks.resize(midiCount);

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
	_vm.clearMemory();
}

void GamosEngine::loadXorSeq(const byte *data, size_t dataSize, int id) {
	Common::MemoryReadStream dataStream(data, dataSize);

	Common::Array<XorArg> &seq = _xorSeq[id];

	uint32 num = dataStream.readUint32LE();
	seq.resize(num);

	for (uint i = 0; i < num; ++i) {
		seq[i].pos = dataStream.readUint32LE();
		seq[i].len = dataStream.readUint32LE();
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
	if (*(const uint32 *)data != 0)
		error("41 not null!!!");

	if (dataSize % 4)
		warning("loadRes41 datasize > 4");
	_sprites[id].sequences.resize(dataSize / 4);
	return true;
}

bool GamosEngine::loadRes42(int32 id, int32 p1, const byte *data, size_t dataSize) {
	//warning("loadRes42 pid %d p %d sz %x",id, p1, dataSize);

	if (_sprites[id].sequences.size() == 0)
		_sprites[id].sequences.resize(1);

	int32 count = dataSize / 8;
	_imgSeq.push_back(new ImageSeq(count));
	_sprites[id].sequences[p1] = _imgSeq.back();

	Common::MemoryReadStream strm(data, dataSize);
	for (int i = 0; i < count; ++i) {
		int32 dataz = strm.readSint32LE();
		if (dataz != 0) {
			error("42    nut null");
		}

		ImagePos &imgpos = _sprites[id].sequences[p1]->operator[](i);
		imgpos.xoffset = strm.readSint16LE();
		imgpos.yoffset = strm.readSint16LE();
	}
	return true;
}

bool GamosEngine::loadRes43(int32 id, int32 p1, int32 p2, const byte *data, size_t dataSize) {
	_images.push_back(new Image());
	_sprites[id].sequences[p1]->operator[](p2).image = _images.back();

	Image *img = _sprites[id].sequences[p1]->operator[](p2).image;

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
			if (_arch._lastReadDecompressedSize) {
				img->offset = _arch._lastReadDataOffset;
				img->cSize = _arch._lastReadSize;
			} else {
				img->offset = _arch._lastReadDataOffset;
				img->cSize = 0;
			}
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
	GameScreen &bimg = _gameScreens[id];
	bimg.loaded = true;
	bimg.offset = _readingBkgOffset;
	bimg._savedStates.clear();
	bimg._savedObjects.clear();
	bimg.palette = nullptr;

	bimg._bkgImageData.assign(data, data + dataSize);

	Common::MemoryReadStream strm(data, dataSize);

	if (_readingBkgMainId == -1 && (strm.readUint32LE() & 0x80000000))
		_readingBkgMainId = id;

	//warning("res 18 id %d 4: %x", id, strm.readUint32LE());

	strm.seek(8);

	bimg._bkgImage.pitch = bimg._bkgImage.w = strm.readUint32LE();
	bimg._bkgImage.h = strm.readUint32LE();

	uint32 imgsize = strm.readUint32LE();

	//warning("res 18 id %d 14: %x", id, strm.readUint32LE());

	bimg._bkgImage.setPixels(bimg._bkgImageData.data() + 0x18);
	bimg._bkgImage.format = Graphics::PixelFormat::createFormatCLUT8();

	bimg.palette = bimg._bkgImageData.data() + 0x18 + imgsize;

	return true;
}


bool GamosEngine::playIntro() {
	if (_movieCount != 0 && _unk11 == 1)
		return scriptFunc18(0);
	return true;
}

bool GamosEngine::playMovie(int id) {
	bool res = _moviePlayer.playMovie(&_arch, _movieOffsets[id], this);
	return res;
}


bool GamosEngine::scriptFunc18(uint32 id) {
	if (_d2_fld17 != 0) {
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
	//warning("Not implemented stopMCI");
}

void GamosEngine::stopSounds() {
	_mixer->stopAll();
}



void GamosEngine::setErrMessage(const Common::String &msg) {
	if (_errSet)
		return;

	_errMessage = msg;
	_errSet = true;
}

void GamosEngine::updateScreen(bool checkers, const Common::Rect &rect) {
	if (_width == 0 || _height == 0)
		return;

	if (!checkers || shouldQuit()) {
		_screen->addDirtyRect(rect);
		return;
	}

	/* checkers update */
	static const Common::Point checkerCoords[16] = {
		{0, 0}, {16, 32}, {48, 16}, {16, 48},
		{0, 32}, {32, 48}, {16, 16}, {48, 0},
		{32, 32}, {0, 48}, {32, 16}, {16, 0},
		{48, 32}, {32, 0}, {0, 16}, {48, 48}
	};

	/* 0.4sec */
	const int16 maxDelay = (400 / 16) - 1;

	_screen->clearDirtyRects();

	for (int16 p = 0; p < 16; p++) {
		uint32 val = _system->getMillis();
		const Common::Point point = checkerCoords[p];
		for (uint32 x = point.x; x < _width; x += 64) {
			for (uint32 y = point.y; y < _height; y += 64) {
				_screen->addDirtyRect(Common::Rect(x, y, x + 16, y + 16));
			}
		}
		_screen->update();

		while (_system->getMillis() - val < maxDelay) {
			_system->delayMillis(1);

			if (eventsSkip()) {
				_screen->addDirtyRect(rect);
				_screen->update();
				return;
			} else {
				_system->updateScreen();
			}
		}
	}
}

void GamosEngine::flushDirtyRects(bool apply) {
	if (apply) {
		for (const Common::Rect &r : _dirtyRects) {
			updateScreen(false, r);
		}
	}
	_dirtyRects.clear();

	_screen->update();

	DAT_004177fd = 0xff;
	DAT_004177fe = ACT_NONE;
	PTR_00417388 = nullptr;

	rndSeed(_system->getMillis());
	PTR_004121b4 = nullptr;
	FUN_0040255c(nullptr);
}

bool GamosEngine::usePalette(const byte *pal, int num, int fade, bool winColors) {

	static const byte winColorMap[20][3] = {
		/* r     g     b */
		{ 0x00, 0x00, 0x00 },
		{ 0x80, 0x00, 0x00 },
		{ 0x00, 0x80, 0x00 },
		{ 0x80, 0x80, 0x00 },
		{ 0x00, 0x00, 0x80 },
		{ 0x80, 0x00, 0x80 },
		{ 0x00, 0x80, 0x80 },
		{ 0xc0, 0xc0, 0xc0 },
		{ 0xc0, 0xdc, 0xc0 },
		{ 0xa6, 0xca, 0xf0 },

		{ 0xff, 0xfb, 0xf0 },
		{ 0xa0, 0xa0, 0xa4 },
		{ 0x80, 0x80, 0x80 },
		{ 0xff, 0x00, 0x00 },
		{ 0x00, 0xff, 0x00 },
		{ 0xff, 0xff, 0x00 },
		{ 0x00, 0x00, 0xff },
		{ 0xff, 0x00, 0xff },
		{ 0x00, 0xff, 0xff },
		{ 0xff, 0xff, 0xff }
	};

	if (!pal)
		return false;

	if (_width != 0 && _height != 0) {
		if (fade == 0 || shouldQuit()) {
			uint16 color = _screen->getPalette().findBestColor(0, 0, 0);
			_screen->fillRect(_screen->getBounds(), color);
			_screen->update();
		} else {
			uint16 color = 0;
			if (fade == 2)
				color = _screen->getPalette().findBestColor(0x80, 0x80, 0x80);
			else if (fade == 3)
				color = _screen->getPalette().findBestColor(0xc0, 0xc0, 0xc0);
			else if (fade == 4)
				color = _screen->getPalette().findBestColor(0xff, 0xff, 0xff);
			else
				color = _screen->getPalette().findBestColor(0, 0, 0);

			/* 0.4sec */
			const int16 maxDelay = (400 / 8) - 1;

			for (int j = 0 ; j < 8; j++) {
				uint32 val = _system->getMillis();

				for (int i = j; i < _screen->w; i += 8)
					_screen->drawLine(i, 0, i, _screen->h - 1, color);
				for (int i = j; i < _screen->h; i += 8)
					_screen->drawLine(0, i, _screen->w - 1, i, color);

				_screen->update();

				while (_system->getMillis() - val < maxDelay) {
					_system->delayMillis(1);

					if (eventsSkip()) {
						j = 8;
						color = _screen->getPalette().findBestColor(0, 0, 0);
						_screen->fillRect(_screen->getBounds(), color);
						_screen->update();
						break;
					} else {
						_system->updateScreen();
					}
				}
			}
		}
	}

	Graphics::Palette newPal(256);
	newPal.set(pal, 0, num);

	if (winColors) {
		newPal.set(winColorMap[0], 0, 10);
		newPal.set(winColorMap[10], 246, 10);
	}

	newPal.resize(num, true);

	_screen->setPalette(newPal);
	return true;
}

bool GamosEngine::setPaletteCurrentGS() {
	_currentFade = _fadeEffectID;

	int curGS = _currentGameScreen;
	if (curGS == -1)
		curGS = 0;

	if (!usePalette(_gameScreens[curGS].palette, 256, _currentFade, true))
		return false;

	addDirtyRect(Common::Rect(_bkgSize.x, _bkgSize.y));

	return true;
}


void GamosEngine::readData2(const RawData &data) {
	Common::MemoryReadStream dataStream(data.data(), data.size());

	//warning("Game data size %d", data.size());

	if (getEngineVersion() == 0x80000018) {
		_stateExt = dataStream.readString(0, 4); // FIX ME
		dataStream.seek(4);
		_messageProc._gd2flags = dataStream.readByte(); //4
		dataStream.seek(8);
		_svModuleId = dataStream.readSint32LE(); // 8
		_svGameScreen = dataStream.readSint32LE(); // c
		_d2_fld10 = dataStream.readUint32LE(); // x10
		_d2_fld14 = dataStream.readByte(); // x14
		_enableMidi = dataStream.readByte() != 0 ? true : false; //x15
		_d2_fld16 = dataStream.readByte(); // x16
		_d2_fld17 = dataStream.readByte(); // x17
		_d2_fld18 = dataStream.readByte(); // x18
		_d2_fld19 = dataStream.readByte(); // x19
		dataStream.seek(0x1c);
		_scrollX = dataStream.readSint32LE(); // x1c
		_scrollY = dataStream.readSint32LE(); // x20
		_scrollTrackObj = dataStream.readSint16LE(); // x24
		_scrollSpeed = dataStream.readSint16LE(); // x26
		_scrollCutoff = dataStream.readSint16LE(); // x28
		_scrollSpeedReduce = dataStream.readSint16LE(); // x2a
		_scrollBorderL = dataStream.readByte(); // x2c
		_scrollBorderR = dataStream.readByte(); // x2d
		_scrollBorderU = dataStream.readByte(); // x2e
		_scrollBorderB = dataStream.readByte(); // x2f
		_sndChannels = dataStream.readByte(); // x30
		_sndVolume = dataStream.readByte(); // x34
		_midiVolume = dataStream.readByte(); // x1a
		_svFps = dataStream.readByte(); // x1b
		_svFrame = dataStream.readSint32LE(); // x1c
		_midiTrack = dataStream.readUint32LE(); //0x38
		_mouseCursorImgId = dataStream.readSint32LE(); //0x3c
		//0x40
		for (int i = 0; i < 12; i++) {
			_messageProc._keyCodes[i] = dataStream.readByte();
		}
	} else if (getEngineVersion() == 0x80000016) {
		_stateExt = dataStream.readString(0, 4); // FIX ME
		dataStream.seek(4);
		_messageProc._gd2flags = dataStream.readByte(); //4
		dataStream.seek(8);
		_svModuleId = dataStream.readSint32LE();
		_svGameScreen = dataStream.readSint32LE();
		_d2_fld10 = dataStream.readUint32LE();
		_d2_fld14 = dataStream.readByte(); // x14
		_enableMidi = dataStream.readByte() != 0 ? true : false; //x15
		_d2_fld16 = dataStream.readByte(); // x16
		_d2_fld17 = dataStream.readByte(); // x17
		_d2_fld18 = 0;
		_d2_fld19 = 0;
		_scrollX = 0;
		_scrollY = 0;
		_scrollTrackObj = -1;
		_scrollSpeed = 16;
		_scrollCutoff = 80;
		_scrollSpeedReduce = -1;
		_scrollBorderL = 0;
		_scrollBorderR = 0;
		_scrollBorderU = 0;
		_scrollBorderB = 0;
		_sndChannels = dataStream.readByte(); // x18
		_sndVolume = dataStream.readByte(); // x19
		_midiVolume = dataStream.readByte(); // x1a
		_svFps = dataStream.readByte(); // x1b
		_svFrame = dataStream.readSint32LE(); // x1c
		_midiTrack = dataStream.readUint32LE(); // x20
		_mouseCursorImgId = dataStream.readSint32LE(); // x24
		//0x28
		for (int i = 0; i < 12; i++) {
			_messageProc._keyCodes[i] = dataStream.readByte();
		}
	}
}


bool GamosEngine::playMidi(Common::Array<byte> *buffer) {
	_musicPlayer.stopMusic();
	_midiStarted = _musicPlayer.playMusic(buffer);
	return _midiStarted;
}

bool GamosEngine::playSound(uint id) {
	Audio::SeekableAudioStream *stream = Audio::makeRawStream(_soundSamples[id].data(), _soundSamples[id].size(), 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, nullptr, stream, -1, _sndVolume);
	return true;
}

int GamosEngine::stepVolume(int volume, int target) {
	int d = target - volume;
	if (d == 0)
		return 0;

	int step = 255 / _fps;
	if (d < 0) {
		step = -step;
		if (step < d)
			step = d;
	} else {
		if (step > d)
			step = d;
	}
	return step;
}

void GamosEngine::changeVolume() {
	const int sndStep = stepVolume(_sndVolume, _sndVolumeTarget);
	if (sndStep) {
		_sndVolume += sndStep;
		_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, _sndVolume);
	}
	const int midiStep = stepVolume(_midiVolume, _midiVolumeTarget);
	if (midiStep) {
		_midiVolume += midiStep;
		_musicPlayer.setVolume(_midiVolume);
	}
}

uint8 GamosEngine::update(Common::Point screenSize, Common::Point mouseMove, Common::Point actPos, uint8 act2, uint8 act1, uint16 keyCode, bool mouseInWindow) {
	_needReload = false;
	_vm._interrupt = false;

	if (_d2_fld16 == 0) {
		act1 = ACT_NONE;
		act2 = ACT_NONE;
		RawKeyCode = ACT_NONE;
	}

	RawKeyCode = keyCode;

	if (RawKeyCode != 0 && RawKeyCode != ACT_NONE) {
		if (_keySeq.size() >= 32)
			_keySeq = _keySeq.substr(_keySeq.size() - 31);

		_keySeq += RawKeyCode;
	}

	FUN_00402c2c(mouseMove, actPos, act2, act1);
	changeVolume();

	if (!FUN_00402bc4())
		return 0;

	bool loop = false;
	if (!_txtInputActive)
		loop = FUN_00402fb4();
	else
		loop = onTxtInputUpdate(act2);

	if (_needReload)
		return 2;  // rerun update after loadModule

	while (loop) {
		if (!PTR_00417388) {
			if (updateMouseCursor(mouseMove) && scrollAndDraw())
				return 1;
			else
				return 0;
		}

		RawKeyCode = ACT_NONE;

		if (!FUN_00402bc4())
			return 0;

		if (!_txtInputActive)
			loop = FUN_00402fb4();
		else
			loop = onTxtInputUpdate(act2);

		if (_needReload)
			return 2; // rerun update after loadModule
	}

	return 0;
}

int32 GamosEngine::doActions(const Actions &a, bool absolute) {
	Common::Array<Common::Point> ARR_00412208(512);

	if (!absolute) {
		DAT_00417228 = PTR_00417218->cell;
	} else {
		PTR_00417218 = nullptr;
		_curObjIndex = -1;
		PTR_00417214 = nullptr;
		DAT_00417228 = Common::Point();
		BYTE_004177f6 = 1;
		_preprocDataId = 0;
		PTR_004173e8 = nullptr;
	}

	DAT_00417220 = DAT_00417228;

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
		for (const ActTypeEntry &ate : a.act_2) {

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

				ObjState fb;
				if (!absolute) {
					Common::Point xy;
					xy.x = (e.x + DAT_00417220.x + _statesWidth) % _statesWidth;
					xy.y = (e.y + DAT_00417220.y + _statesHeight) % _statesHeight;
					fb = _states.at(xy);
				} else {
					fb = _states.at(e.x, e.y);
				}

				int cval = 0;
				int fnc = e.t;
				if ((e.flags & 1) == 0) {
					if (e.actid == fb.actid && (fb.t & e.t)) {
						cval = 2;
					}
				} else if (fb.actid != 0xfe &&
				           (_thing2[e.actid].field_0[(fb.actid) >> 3] & (1 << (fb.actid & 7))) != 0) {

					if (!_thing2[e.actid].field_2.empty()) {
						e.t = _thing2[e.actid].field_2[fb.actid] >> 4;
						preprocessData(fnc + 8, &e);
					}

					if (fb.t & e.t) {
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
				} else if ((ate.entries.size() - i) == 1 && spos > -1 && sbuf[spos * 2] == sbuf[ps]) {
					return 0;
				}
			}

			if (fastSkipAll)
				break;
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
			fldsv = PTR_00417218->priority;
		if (a.functionAddress != -1)
			doScript(a.functionAddress);
		if (_needReload)
			return 0;
		if (BYTE_004177fc == 0 && BYTE_00412200 == 0 && PTR_00417218 && PTR_00417218->priority != fldsv && PTR_00417218->curObjectId != -1)
			addDirtRectOnObject(&_objects[PTR_00417218->curObjectId]);
	}

	if (BYTE_004177fc == 0 && BYTE_00412200 != 0)
		FUN_004095a0(PTR_00417218);

	int32 retval = 0;

	if (a.flags & Actions::HAS_ACT10) {
		int ivar5 = -1;
		for (const ActTypeEntry &ate : a.act_10) {
			switch (ate.t) {
			case 0: {
				uint16 rndval = rndRange16(a.num_act_10e);
				for (ActEntry e : a.act_10end[rndval]) {
					retval += processData(e, absolute);
					if (_needReload)
						return 0;
				}
			}
			break;

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

			}
			break;

			case 2: {
				int32 num = rndRange16(ate.entries.size());
				ActEntry e = ate.entries[num];
				retval += processData(e, absolute);
				if (_needReload)
					return 0;
			}
			break;

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
			}
			break;

			default: {
				ivar5++;
				/* Seems it's has a error in original
				   think it's must be:
				   min + rnd(max-min) */

				uint32 lb = rnd() >> 0x10;
				uint32 idx = ((sbuf[ivar5 * 2 + 1] - sbuf[ivar5 * 2]) * lb + sbuf[ivar5 * 2]) >> 0x10;
				Common::Point point = ARR_00412208[ idx ];

				for (ActEntry e : ate.entries) {
					if (Common::Point(e.x, e.y) == point) {
						retval += processData(e, absolute);
						if (_needReload)
							return 0;
						break;
					}
				}
			}
			break;

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
	}
	break;

	case 2:
	case 12: {
		static const uint8 lookup[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
		e->y = -e->y;
		e->x = -e->x;
		e->t = lookup[ e->t ];
	}
	break;

	case 3:
	case 16: {
		static const uint8 lookup[16] = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
		int8 tmp = e->x;
		e->x = e->y;
		e->y = -tmp;
		e->t = lookup[ e->t ];
	}
	break;

	case 4: {
		static const uint8 lookup[16] = {0, 1, 8, 9, 4, 5, 12, 13, 2, 3, 10, 11, 6, 7, 14, 15};
		e->x = -e->x;
		e->t = lookup[ e->t ];
	}
	break;

	case 5: {
		static const uint8 lookup[16] = {0, 2, 1, 3, 8, 10, 9, 11, 4, 6, 5, 7, 12, 14, 13, 15};
		int8 tmp = e->x;
		e->x = -e->y;
		e->y = -tmp;
		e->t = lookup[ e->t ];
	}
	break;

	case 6: {
		static const uint8 lookup[16] = {0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15};
		e->y = -e->y;
		e->t = lookup[ e->t ];
	}
	break;

	case 7: {
		static const uint8 lookup[16] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
		uint8 tmp = e->x;
		e->x = e->y;
		e->y = tmp;
		e->t = lookup[ e->t ];
	}
	break;
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
	}
	break;

	case 5: {
		static const uint8 lookup[2] = {1, 4};
		e->t = lookup[rndRange16(2)];
	}
	break;

	case 6: {
		static const uint8 lookup[2] = {2, 4};
		e->t = lookup[rndRange16(2)];
	}
	break;

	case 7: {
		static const uint8 lookup[3] = {1, 2, 4};
		e->t = lookup[rndRange16(3)];
	}
	break;

	case 9: {
		static const uint8 lookup[2] = {1, 8};
		e->t = lookup[rndRange16(2)];
	}
	break;

	case 0xa: {
		static const uint8 lookup[2] = {2, 8};
		e->t = lookup[rndRange16(2)];
	}
	break;

	case 0xb: {
		static const uint8 lookup[3] = {1, 2, 8};
		e->t = lookup[rndRange16(3)];
	}
	break;

	case 0xc: {
		static const uint8 lookup[2] = {4, 8};
		e->t = lookup[rndRange16(2)];
	}
	break;

	case 0xd: {
		static const uint8 lookup[3] = {1, 4, 8};
		e->t = lookup[rndRange16(3)];
	}
	break;

	case 0xe: {
		static const uint8 lookup[3] = {2, 4, 8};
		e->t = lookup[rndRange16(3)];
	}
	break;

	case 0xf: {
		static const uint8 lookup[4] = {1, 2, 4, 8};
		e->t = lookup[rndRange16(4)];
	}
	break;
	}
}

int GamosEngine::processData(ActEntry e, bool absolute) {
	preprocessData(_preprocDataId, &e);
	if (!absolute) {
		createActiveObject(e, Common::Point(
		             (e.x + DAT_00417220.x + _statesWidth) % _statesWidth,
		             (e.y + DAT_00417220.y + _statesHeight) % _statesHeight) );
		if (_needReload)
			return 0;
		return e.x == 0 && e.y == 0;
	} else {
		createActiveObject(e, Common::Point(e.x, e.y) );
		return 0;
	}
}

void GamosEngine::FUN_00402a68(ActEntry e) {
	if (e.x != 0 || e.y != 0) {
		DAT_00417220.x = (e.x + DAT_00417220.x + _statesWidth) % _statesWidth;
		DAT_00417220.y = (e.y + DAT_00417220.y + _statesHeight) % _statesHeight;

		ObjState st = PTR_00417218->state;
		_states.at(DAT_00417228) = ObjState(st.actid, 0, st.t);

		removeObjectAtCoords(DAT_00417220, false);

		PTR_00417218->cell = DAT_00417220;

		ObjState rthing = _states.at(DAT_00417220);
		PTR_00417218->state = ObjState(rthing.actid, st.flags, rthing.t);

		_states.at(DAT_00417220) = ObjState(PTR_00417218->actID, 0, PTR_00417218->t);

		BYTE_00412200 = 1;
	}

	if (e.t != BYTE_004177f6) {
		BYTE_004177f6 = e.t;
		PTR_00417218->t = e.t;

		ObjState &stref = _states.at(DAT_00417220);
		stref.flags = 0;
		stref.t = BYTE_004177f6;

		BYTE_00412200 = 1;
	}
}

void GamosEngine::createActiveObject(ActEntry e, Common::Point cell) {
	ObjState &stref = _states.at(cell);

	uint8 oid = e.actid;

	if ((e.flags & 1) == 0) {
		if (oid == 0xfe) {
			removeObjectAtCoords(cell, true);
			if (_needReload)
				return;

			stref = e;
			return;
		}
	} else {
		Unknown1 &unk1 = _thing2[ oid ];
		uint8 index = rndRange16(unk1.field_1[0]);
		oid = unk1.field_1[ index + 1 ];
		if (!unk1.field_2.empty()) {
			byte id1 = e.t;
			e.t = unk1.field_2[ oid ] >> 4;
			preprocessData(8 + id1, &e);
		}
	}

	preprocessDataB1(e.t, &e);

	e.flags = 0;

	rnd(); // needed?

	Object *obj = nullptr;
	int index = 0;
	byte *odat = nullptr;

	ObjectAction &act = _objectActions[oid];
	if (act.actType == 0) {
		removeObjectAtCoords(cell, true);
		if (_needReload)
			return;
		obj = nullptr;
		index = -1;
		odat = nullptr;
	} else {
		removeObjectAtCoords(cell, false);
		if (_needReload)
			return;
		obj = getFreeObject();
		obj->flags = Object::FLAG_VALID | Object::FLAG_HASACTION;
		obj->t = e.t;
		obj->actID = oid;
		obj->inputFlag = 0;
		obj->priority = act.priority;
		obj->cell = cell;
		obj->tgtObjectId = -1;
		obj->curObjectId = -1;
		obj->state = stref;
		if (PTR_00417218 && obj->index > PTR_00417218->index)
			obj->state.flags |= 1;

		// if (storageSize < 5) {
		//  obj->pImg = nullptr;
		//  odat = &obj->pImg;
		// } else {
		//  odat = malloc(storageSize);
		//  obj->pImg = (Sprite *)odat;
		//  obj->flags |= 8;
		// }
		obj->storage.clear();
		obj->storage.resize(act.storageSize, 0);
		odat = obj->storage.data();
		index = obj->index;
		if (act.actType == 3 && PTR_004121b4 == nullptr)
			PTR_004121b4 = obj;
	}

	stref = ObjState(oid, e.flags, e.t);
	executeScript(act.onCreateAddress, &act, obj, index, odat, cell, e.t);
}

void GamosEngine::removeObjectByIDMarkDirty(int32 id) {
	if (id != -1)
		removeObjectMarkDirty(&_objects[id]);
}


void GamosEngine::removeObjectAtCoords(Common::Point cell, bool deleteGfxObj) {
	ObjState &stref = _states.at(cell);

	uint8 actid = stref.actid;

	if (actid == 0xfe)
		return;

	ObjectAction &act = _objectActions[actid];
	Object *povar4 = nullptr;
	bool multidel = false;

	for (uint i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if (obj.flags & Object::FLAG_VALID) {
			if (obj.flags & Object::FLAG_HASACTION) {
				if (obj.cell == cell) {
					removeObjectByIDMarkDirty(obj.curObjectId);
					if (obj.curObjectId != obj.tgtObjectId)
						removeObjectByIDMarkDirty(obj.tgtObjectId);
					/* if (obj.flags & Object::FLAG_STORAGE)
					    obj.storage.clear(); */
					removeSubtitles(&obj);
					removeObject(&obj);
					FUN_0040255c(&obj);
					povar4 = &obj;
					if (!deleteGfxObj || multidel)
						break;

					multidel = true;
				}
			} else {
				if (deleteGfxObj && obj.cell == cell &&
				        obj.actObjIndex == -1 && (obj.flags & Object::FLAG_FREECOORDS) == 0) {

					removeObjectMarkDirty(&obj);
					if (multidel)
						break;

					multidel = true;
				}
			}
		}
	}

	if (povar4)
		stref = ObjState(povar4->state.actid, 0, povar4->state.t);

	executeScript(act.onDeleteAddress, &act, nullptr, -1, nullptr, cell, stref.t);
}

Object *GamosEngine::getFreeObject() {
	Object *obj = nullptr;
	for (uint i = 0; i < _objects.size(); i++) {
		Object &rObj = _objects[i];
		if ((rObj.flags & Object::FLAG_VALID) == 0) {
			obj = &rObj;
			break;
		}
	}

	if (!obj) {
		_objects.emplace_back();
		obj = &_objects.back();
		obj->index = _objects.size() - 1;
	}

	obj->flags = Object::FLAG_VALID;
	obj->priority = 0;
	obj->cell.x = 0;
	obj->cell.y = 0;

	obj->sprId = -1;
	obj->seqId = -1;
	obj->frame = -1;
	obj->frameMax = -1;
	obj->position.x = 0;
	obj->position.y = 0;
	obj->actObjIndex = -1;

	obj->actID = 0;
	obj->t = 0;
	obj->state = ObjState();
	obj->inputFlag = 0;
	obj->tgtObjectId = -1;
	obj->curObjectId = -1;

	obj->pImg = nullptr;
	obj->storage.clear(); // clear it so gfx object will not have data
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
	if (obj->flags & Object::FLAG_GRAPHIC)
		addDirtRectOnObject(obj);
	removeObject(obj);
}

void GamosEngine::executeScript(int32 scriptAddr, ObjectAction *act, Object *pobj, int32 index, byte *storage, Common::Point cell, uint8 t) {
	if (scriptAddr == -1)
		return;

	uint8 sv1 = BYTE_004177f6;
	byte *sv2 = PTR_004173e8;
	Common::Point sv4 = DAT_00417228;
	Common::Point sv6 = DAT_00417220;
	int32 sv7 = _curObjIndex;
	Object *sv8 = PTR_00417218;
	ObjectAction *sv9 = PTR_00417214;

	BYTE_004177f6 = t;
	PTR_004173e8 = storage;
	DAT_00417228 = cell;
	DAT_00417220 = cell;
	_curObjIndex = index;
	PTR_00417218 = pobj;
	PTR_00417214 = act;

	doScript(scriptAddr);

	BYTE_004177f6 = sv1;
	PTR_004173e8 = sv2;
	DAT_00417228 = sv4;
	DAT_00417220 = sv6;
	_curObjIndex = sv7;
	PTR_00417218 = sv8;
	PTR_00417214 = sv9;
}

bool GamosEngine::FUN_00402fb4() {
	if (_objects.empty())
		return true;

	Object *pobj = DAT_00412204;
	if (!pobj)
		pobj = &(_objects.front());

	for (int32 objIdx = pobj->index; objIdx < _objects.size(); objIdx++) {
		pobj = &_objects[objIdx];

		if (pobj->isActionObject()) {
			if (!PTR_00417388 || (PTR_00417388[ pobj->actID >> 3 ] & (1 << (pobj->actID & 7)))) {
				if (pobj->state.flags & 1) {
					pobj->state.flags &= ~1;
				} else {
					if ((pobj->flags & Object::FLAG_TRANSITION) == 0) {
						if (pobj->curObjectId != -1 && updateGfxFrames(&_objects[pobj->curObjectId], false, true)) {
							pobj->curObjectId = pobj->tgtObjectId;
							if (pobj->tgtObjectId != -1) {
								Object &o = _objects[pobj->tgtObjectId];
								o.flags |= Object::FLAG_GRAPHIC;
								o.cell = pobj->cell;
								FUN_0040921c(&o);
								addDirtRectOnObject(&o);
							}
						}
					} else {
						if (updateGfxFrames(&_objects[pobj->curObjectId], true, pobj->curObjectId != pobj->tgtObjectId)) {
							pobj->curObjectId = pobj->tgtObjectId;
							if (pobj->tgtObjectId != -1) {
								Object &o = _objects[pobj->tgtObjectId];
								o.flags |= Object::FLAG_GRAPHIC;
								o.cell = pobj->cell;
								FUN_0040921c(&o);
								addDirtRectOnObject(&o);
							}
							pobj->flags &= ~Object::FLAG_TRANSITION;
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

					_pathInMove = false;
					for (Actions &scr : PTR_00417214->actions) {
						BYTE_004177f6 = PTR_00417218->t;

						int ivr8 = 0;
						if (BYTE_004177f6 == 2)
							ivr8 = 1;
						else if (BYTE_004177f6 == 4)
							ivr8 = 2;
						else if (BYTE_004177f6 == 8)
							ivr8 = 3;

						bool tmp = false;
						for (int i = 0; i < 8; i++) {
							if (PTR_00417214->mask & (1 << i)) {
								int fncid = ((i & 3) + ivr8) & 3;
								if (i > 3)
									fncid += 4;
								DAT_004173ec = fncid;

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
			if (!PTR_00417388 && pobj->isGraphicObject() && pobj->actObjIndex == -1)
				updateGfxFrames(pobj, false, true);
		}
continue_to_next_object:
		;
	}

exit:
	PTR_00417218 = nullptr;
	_curObjIndex = -1;
	return true;
}

bool GamosEngine::updateGfxFrames(Object *obj, bool p2, bool p1) {
	if (obj->frameMax < 2) {
		if (p2 || (obj->flags & Object::FLAG_DIRTRECT)) {
			addDirtRectOnObject(obj);
			if (p1)
				removeObject(obj);
			return true;
		}
	} else {
		addDirtRectOnObject(obj);
		obj->frame++;

		if (obj->frame == obj->frameMax) {
			obj->frame = 0;
			obj->pImg = &_sprites[obj->sprId].sequences[obj->seqId]->operator[](obj->frame);
			if (p2 || (obj->flags & Object::FLAG_DIRTRECT)) {
				addDirtRectOnObject(obj);
				if (p1)
					removeObject(obj);
				return true;
			}
		} else {
			obj->pImg = &_sprites[obj->sprId].sequences[obj->seqId]->operator[](obj->frame);
		}

		if ((obj->flags & Object::FLAG_FREECOORDS) == 0)
			FUN_0040921c(obj);

		addDirtRectOnObject(obj);
	}
	return false;
}

void GamosEngine::FUN_0040921c(Object *gfxObj) {
	ImagePos *imgPos = gfxObj->pImg;
	Image *img = imgPos->image;

	int32 x = gfxObj->cell.x * _gridCellW;
	int32 y = gfxObj->cell.y * _gridCellH;

	if (gfxObj->actObjIndex != -1) {
		Object *o = &_objects[ gfxObj->actObjIndex ];
		if (o->flags & Object::FLAG_TRANSITION) {
			int t = gfxObj->frame + 1;
			x += (o->cell.x - gfxObj->cell.x) * _gridCellW * t / gfxObj->frameMax;
			y += (o->cell.y - gfxObj->cell.y) * _gridCellH * t / gfxObj->frameMax;
		}
	}

	if (gfxObj->flags & Object::FLAG_FLIPH)
		gfxObj->position.x = x - (img->surface.w - _gridCellW - imgPos->xoffset);
	else
		gfxObj->position.x = x - imgPos->xoffset;

	if (gfxObj->flags & Object::FLAG_FLIPV)
		gfxObj->position.y = y - (img->surface.h - _gridCellH - imgPos->yoffset);
	else
		gfxObj->position.y = y - imgPos->yoffset;
}

void GamosEngine::addDirtRectOnObject(Object *obj) {
	ImagePos *imgPos = obj->pImg;
	Common::Rect rect;
	rect.left = obj->position.x;
	rect.top = obj->position.y;
	if (obj->flags & Object::FLAG_FREECOORDS) {
		rect.left -= imgPos->xoffset;
		rect.top -= imgPos->yoffset;
	}
	rect.setWidth(imgPos->image->surface.w);
	rect.setHeight(imgPos->image->surface.h);

	addDirtyRect(rect);
}

void GamosEngine::addDirtyRect(const Common::Rect &rect) {
	if (_dirtyRects.empty()) {
		_dirtyRects.push_back(rect);
		return;
	}

	bool intersects = 0;
	for (int i = 0; i < _dirtyRects.size(); i++) {
		Common::Rect &r = _dirtyRects[i];

		if (!rect.intersects(r))
			continue;

		intersects = true;

		r.extend(rect);
		break;
	}

	if (!intersects) {
		_dirtyRects.push_back(rect);
		return;
	}

rerunCheck:
	for (int i = _dirtyRects.size() - 2; i > 0; i--) {
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

	int32 bkg = _currentGameScreen;
	if (bkg == -1)
		bkg = 0;

	Common::Array<Object *> drawList(1024);  //_drawElements.size(), 1024) );

	int cnt = 0;
	for (int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if ( obj.isGraphicObject() ) {
			drawList[cnt] = &obj;
			cnt++;
		}
	}

	drawList.resize(cnt);

	if (cnt) {
		for (int i = 0; i < drawList.size() - 1; i++) {
			for (int j = i + 1; j < drawList.size(); j++) {
				Object *o1 = drawList[i];
				Object *o2 = drawList[j];
				if (o1->priority < o2->priority) {
					drawList[i] = o2;
					drawList[j] = o1;
				}
			}
		}
	}

	for (int i = 0; i < _dirtyRects.size(); i++) {
		Common::Rect r = _dirtyRects[i];

		r.translate(-_scrollX, -_scrollY);
		r.clip(_screen->getBounds());

		if (r.isEmpty())
			continue;

		Common::Rect srcRect = r;
		srcRect.translate(_scrollX, _scrollY);

		/* update bkg at this rect */
		if (_gameScreens[bkg].loaded) {
			_screen->blitFrom(_gameScreens[bkg]._bkgImage, srcRect, r.origin());
		}

		for (Object *o : drawList) {
			if (o->pImg && loadImage(o->pImg->image)) {

				Common::Rect s;
				s.left = o->position.x - _scrollX;
				s.top = o->position.y - _scrollY;

				if (o->flags & Object::FLAG_FREECOORDS) {
					s.left -= o->pImg->xoffset;
					s.top -= o->pImg->yoffset;
				}

				s.setWidth(o->pImg->image->surface.w);
				s.setHeight(o->pImg->image->surface.h);

				if (!s.intersects(r))
					continue;

				Common::Rect sdirt = s;
				sdirt.clip(r);

				Common::Rect ssrc(sdirt.origin() - s.origin(), sdirt.width(), sdirt.height());

				uint flip = 0;
				if (o->flags & Object::FLAG_FLIPH)
					flip |= Graphics::FLIP_H;
				if (o->flags & Object::FLAG_FLIPV)
					flip |= Graphics::FLIP_V;

				Blitter::blit(&o->pImg->image->surface, ssrc, _screen->surfacePtr(), sdirt, flip);
			}
		}

		if (!_currentFade)
			_screen->addDirtyRect(r);
	}


	if (_currentFade)
		updateScreen(true, Common::Rect(_bkgSize.x, _bkgSize.y));

	_currentFade = 0;

	_dirtyRects.clear();

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
	uint32 res = _vm.doScript(scriptAddress, PTR_004173e8);
	return res;
}


void GamosEngine::vmCallDispatcher(VM::Context *ctx, uint32 funcID) {
	uint32 arg1 = 0, arg2 = 0;

	switch (funcID) {
	case 0:
		DAT_004177ff = true;
		ctx->EAX.setVal(1);
		break;
	case 1:
		ctx->EAX.setVal( PTR_00417218->curObjectId == -1 ? 1 : 0 );
		break;

	case 2:
		arg1 = ctx->pop32();
		if (PTR_00417218->tgtObjectId == -1)
			ctx->EAX.setVal(0);
		else
			ctx->EAX.setVal( _objects[ PTR_00417218->tgtObjectId ].sprId == arg1 ? 1 : 0 );
		break;
	case 3:
		ctx->EAX.setVal( (PTR_00417218->inputFlag & 0x90) == 0x10 ? 1 : 0 );
		break;
	case 4:
		ctx->EAX.setVal( (PTR_00417218->inputFlag & 0xa0) == 0x20 ? 1 : 0 );
		break;
	case 5:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( (PTR_00417218->inputFlag & 0xb0) == arg1 ? 1 : 0 );
		break;
	case 6:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( (PTR_00417218->inputFlag & 0x4f) == arg1 ? 1 : 0 );
		break;
	case 7:
		arg1 = ctx->pop32();
		if ((PTR_00417218->inputFlag & 0x40) == 0 || (PTR_00417218->inputFlag & 8) != (arg1 & 8))
			ctx->EAX.setVal(0);
		else
			ctx->EAX.setVal( FUN_0040705c(arg1 & 7, PTR_00417218->inputFlag & 7) ? 1 : 0 );
		break;
	case 8:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( PTR_00417218->priority == arg1 ? 1 : 0 );
		break;
	case 9:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( savedDoActions(_subtitleActions[arg1]) );
		break;
	case 10:
		ctx->EAX.setVal( PTR_00417218->state.actid == 0xfe ? 1 : 0 );
		break;
	case 11:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( PTR_00417218->state.actid == arg1 ? 1 : 0 );
		break;
	case 12:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( _thing2[arg1].field_0[ PTR_00417218->state.actid >> 3 ] & (1 << (PTR_00417218->state.actid & 7)) );
		break;
	case 13: {
		VM::ValAddr regRef = ctx->popReg();
		Common::String str = ctx->getString(regRef);

		ctx->EAX.setVal(0);

		for(uint i = 0; i < str.size(); i++) {
			if (str[i] == RawKeyCode) {
				ctx->EAX.setVal(1);
				break;
			}
		}
		break;
	}

	case 14:
		arg1 = ctx->pop32();
		loadModule(arg1);
		setNeedReload();
		ctx->EAX.setVal(1);
		break;

	case 15:
		arg1 = ctx->pop32();
		switchToGameScreen(arg1, false);
		setNeedReload();
		break;

	case 16:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( scriptFunc16(arg1) );
		break;

	case 17:
		arg1 = ctx->pop32();
		playSound(arg1);
		ctx->EAX.setVal(1);
		break;

	case 18:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( scriptFunc18(arg1) ? 1 : 0 );
		break;

	case 19:
		arg1 = ctx->pop32();
		ctx->EAX.setVal( scriptFunc19(arg1) );
		break;

	case 20: {
		arg1 = ctx->pop32();
		for (const SubtitlePoint &d : _subtitlePoints[arg1]) {
			createGfxObject(d.sprId, d, true);
		}
		ctx->EAX.setVal( savedDoActions(_subtitleActions[arg1]) );
	} break;

	case 21: {
		VM::ValAddr regRef = ctx->popReg();
		arg2 = ctx->pop32();
		ctx->EAX.setVal( txtInputBegin(ctx, regRef.getMemType(), regRef.getOffset(), arg2, DAT_00417220.x * _gridCellW, DAT_00417220.y * _gridCellH) );
	} break;

	case 22: {
		VM::ValAddr regRef = ctx->popReg();
		arg2 = ctx->pop32();
		const SubtitlePoint &d = _subtitlePoints[arg2][0];
		ctx->EAX.setVal( txtInputBegin(ctx, regRef.getMemType(), regRef.getOffset(), d.sprId, d.x, d.y) );
	} break;

	case 23: {
		VM::ValAddr regRef = ctx->popReg();
		arg2 = ctx->pop32();
		addSubtitles(ctx, regRef.getMemType(), regRef.getOffset(), arg2, DAT_00417220.x * _gridCellW, DAT_00417220.y * _gridCellH);
		ctx->EAX.setVal(1);
	} break;

	case 24: {
		VM::ValAddr regRef = ctx->popReg();
		arg2 = ctx->pop32();
		const SubtitlePoint &d = _subtitlePoints[arg2][0];
		addSubtitles(ctx, regRef.getMemType(), regRef.getOffset(), d.sprId, d.x, d.y);

		ctx->EAX.setVal(1);
	}
	break;

	case 25: {
		arg1 = ctx->pop32();
		if (PTR_00417218->priority != arg1) {
			PTR_00417218->priority = arg1;
			if (PTR_00417218->tgtObjectId != -1) {
				Object &obj = _objects[PTR_00417218->tgtObjectId];
				obj.priority = arg1;
			}
			if (PTR_00417218->curObjectId != -1) {
				Object &obj = _objects[PTR_00417218->curObjectId];
				obj.priority = arg1;
				addDirtRectOnObject(&obj);
			}
		}
		ctx->EAX.setVal(1);
	}
	break;

	case 26:
		removeSubtitles(PTR_00417218);
		ctx->EAX.setVal(1);
		break;

	case 27:
		FUN_004025d0();
		ctx->EAX.setVal(1);
		break;

	case 28:
		arg1 = ctx->pop32();
		FUN_0040279c(arg1, false);
		ctx->EAX.setVal(1);
		break;

	case 29:
		arg1 = ctx->pop32();
		FUN_0040279c(arg1, true);
		ctx->EAX.setVal(1);
		break;

	case 30: {
		if (PTR_00417218->curObjectId != -1) {
			Object *obj = &_objects[PTR_00417218->curObjectId];
			PTR_00417218->tgtObjectId = -1;
			PTR_00417218->curObjectId = -1;
			removeObjectMarkDirty(obj);
		}
	} break;

	case 31:
		arg1 = ctx->pop32();
		setCursor(arg1, true);
		ctx->EAX.setVal(1);
		break;

	case 32:
		setCursor(0, false);
		ctx->EAX.setVal(1);
		break;

	case 33:
		PTR_00417218->priority = _statesHeight - PTR_00417218->cell.y;
		ctx->EAX.setVal(1);
		break;

	case 34: {
		VM::ValAddr regRef = ctx->popReg();
		ctx->setMem8(regRef, PTR_00417218->priority);
		ctx->EAX.setVal(1);
	} break;

	case 35: {
		arg1 = ctx->pop32();
		uint ret = 0;
		switch (arg1) {
		case 3:
			ret = FUN_00408648(0xe, 0xff, 0xff);
			break;

		case 4:
			ret = FUN_00408648(0xe, 0xfe, 0xff);
			break;

		case 5:
			ret = FUN_00408648(0xe, 0xfe, 0xfe);
			break;

		case 6:
			ret = FUN_00408648(0x82, 0xff, 0xff);
			break;

		case 7:
			ret = FUN_00408648(0x82, 0xfe, 0xff);
			break;

		case 8:
			ret = FUN_00408648(0x82, 0xfe, 0xfe);
			break;

		case 9:
			ret = FUN_00408648(0x83, 0xff, 0xff);
			break;

		case 10:
			ret = FUN_00408648(0x83, 0xfe, 0xff);
			break;

		case 11:
			ret = FUN_00408648(0x83, 0xfe, 0xfe);
			break;

		default:
			break;
		}
		ctx->EAX.setVal(ret);
	} break;

	case 36: {
		arg1 = ctx->pop32();
		arg2 = ctx->pop32();

		uint ret = 0;
		switch (arg1) {
		case 1:
			ret = FUN_00408648(0, arg2, 0xff);
			break;

		case 2:
			ret = FUN_00408648(0, arg2, 0xfe);
			break;

		case 3:
			ret = FUN_00408648(0xe, arg2, 0xff);
			break;

		case 4:
			ret = FUN_00408648(0xe, arg2, 0xfe);
			break;

		case 5:
			ret = FUN_00408648(0xe, arg2, arg2);
			break;

		case 6:
			ret = FUN_00408648(0x82, arg2, 0xff);
			break;

		case 7:
			ret = FUN_00408648(0x82, arg2, 0xfe);
			break;

		case 8:
			ret = FUN_00408648(0x82, arg2, arg2);
			break;

		case 9:
			ret = FUN_00408648(0x83, arg2, 0xff);
			break;

		case 10:
			ret = FUN_00408648(0x83, arg2, 0xfe);
			break;

		case 11:
			ret = FUN_00408648(0x83, arg2, arg2);
			break;

		default:
			break;
		}
		ctx->EAX.setVal(ret);
	} break;

	case 37: {
		arg1 = ctx->pop32();
		arg2 = ctx->pop32();

		uint ret = 0;
		switch (arg1) {
		case 1:
			ret = FUN_004088cc(0, arg2, 0xff);
			break;

		case 2:
			ret = FUN_004088cc(0, arg2, 0xfe);
			break;

		case 3:
			ret = FUN_004088cc(0xe, arg2, 0xff);
			break;

		case 4:
			ret = FUN_004088cc(0xe, arg2, 0xfe);
			break;

		case 5:
			ret = FUN_004088cc(0xe, arg2, arg2);
			break;

		case 6:
			ret = FUN_004088cc(0x82, arg2, 0xff);
			break;

		case 7:
			ret = FUN_004088cc(0x82, arg2, 0xfe);
			break;

		case 8:
			ret = FUN_004088cc(0x82, arg2, arg2);
			break;

		case 9:
			ret = FUN_004088cc(0x83, arg2, 0xff);
			break;

		case 10:
			ret = FUN_004088cc(0x83, arg2, 0xfe);
			break;

		case 11:
			ret = FUN_004088cc(0x83, arg2, arg2);
			break;

		default:
			break;
		}
		ctx->EAX.setVal(ret);
	} break;

	case 38:
		arg1 = ctx->pop32();
		if (_pathInMove && (int32)arg1 == _pathDir4)
			ctx->EAX.setVal(1);
		else
			ctx->EAX.setVal(0);
		break;

	case 39:
		arg1 = ctx->pop32();
		if (_pathInMove && (int32)arg1 == _pathDir8)
			ctx->EAX.setVal(1);
		else
			ctx->EAX.setVal(0);
		break;

	case 40:
		arg1 = ctx->pop32();
		if (_pathInMove && FUN_0040705c(arg1, _pathDir4) != 0)
			ctx->EAX.setVal(1);
		else
			ctx->EAX.setVal(0);
		break;

	case 41:
		arg1 = ctx->pop32();
		if (_pathInMove && FUN_0040705c(arg1, _pathDir8) != 0)
			ctx->EAX.setVal(1);
		else
			ctx->EAX.setVal(0);
		break;

	case 42: {
		arg1 = ctx->pop32();
		if (_pathInMove != false) {
			if (arg1 == 0) {
				_pathInMove = false;
				DAT_004177fe = 255;
				DAT_00417805 = 255;
				DAT_004177fd = 255;
			} else if (arg1 == 1) {
				ActEntry tmp;
				tmp.actid = 0xfe;
				tmp.t = BYTE_004177f6;
				tmp.flags = 0;
				createActiveObject(tmp, _pathTargetCell);
			} else if (arg1 == 2) {
				ActEntry tmp;
				tmp.actid = 0;
				tmp.t = BYTE_004177f6;
				tmp.flags = 0;
				tmp.x = _pathTargetCell.x - _pathStartCell.x;
				tmp.y = _pathTargetCell.y - _pathStartCell.y;
				FUN_00402a68(tmp);
			}
		}
		ctx->EAX.setVal(1);
	} break;

	case 43: {
		arg1 = ctx->pop32();
		if (_pathInMove) {
			ActEntry tmp;
			tmp.actid = arg1;
			tmp.t = BYTE_004177f6;
			tmp.flags = 0;
			createActiveObject(tmp, _pathTargetCell);
		}
		ctx->EAX.setVal(1);
	} break;

	case 44: {
		arg1 = ctx->pop32();
		if (_pathInMove) {
			ActEntry tmp;
			tmp.actid = arg1;
			tmp.t = BYTE_004177f6;
			tmp.flags = 1;
			createActiveObject(tmp, _pathTargetCell);
		}
		ctx->EAX.setVal(1);
	} break;

	case 45:
		arg1 = ctx->pop32();
		// Seems here needed only ->t ?
		// In AiTi arg1 0x20, 0x40, 0x80
		ctx->EAX.setVal( ((PTR_00417218->flags | (PTR_00417218->t << 4)) & arg1) ? 1 : 0 );
		break;

	case 46: {
		VM::ValAddr a1 = ctx->popReg();
		VM::ValAddr a2 = ctx->popReg();
		Common::String s = ctx->getString(a1);
		for(int i = 0; i <= s.size(); i++) {
			ctx->setMem8(a2.getMemType(), a2.getOffset() + i, s.c_str()[i]);
		}
	} break;

	case 47: {
		arg1 = ctx->pop32();

		switch (arg1) {
		case 0:
			ctx->EAX.setVal(_d2_fld16 != 0 ? 1 : 0);
			break;

		case 1:
			ctx->EAX.setVal(_d2_fld14 != 0 ? 1 : 0);
			break;

		case 2:
			ctx->EAX.setVal(_midiVolumeTarget != 0 ? 1 : 0);
			break;

		case 3:
			ctx->EAX.setVal(_d2_fld17 != 0 ? 1 : 0);
			break;

		case 4:
			ctx->EAX.setVal(_d2_fld18 != 0 ? 1 : 0);
			break;

		default:
			break;
		}
	}
	break;

	case 48: {
		arg1 = ctx->pop32();

		switch (arg1) {
		case 0:
			_d2_fld16 = 0;
			break;
		case 1:
			_d2_fld16 = 1;
			break;
		case 2:
			_d2_fld14 = 0;
			_sndVolumeTarget = 0;
			break;
		case 3:
			_d2_fld14 = 1;
			_sndVolumeTarget = _savedSndVolume;
			break;
		case 4:
			_midiVolumeTarget = 0;
			break;
		case 5:
			_midiVolumeTarget = _savedMidiVolume;
			break;
		case 6:
			_d2_fld17 = 0;
			break;
		case 7:
			_d2_fld17 = 1;
			break;
		case 8:
			//FUN_0040a9c0(0);
			_d2_fld18 = 0;
			break;
		case 9:
			if (_d2_fld19 != 0xff) {
				//FUN_0040a958(_d2_fld19);
			}
			_d2_fld18 = 1;
			break;
		default:
			break;
		}
		ctx->EAX.setVal(1);
	} break;

	case 49: {
		arg1 = ctx->pop32();
		arg2 = ctx->pop32();

		if (arg1 == 0) {
			ctx->EAX.setVal(deleteSaveFile(arg2) ? 1 : 0);
		} else if (arg1 == 1) {
			ctx->EAX.setVal(writeSaveFile(arg2) ? 1 : 0);
		} else if (arg1 == 2) {
			ctx->EAX.setVal(loadSaveFile(arg2) ? 1 : 0);
		}
	}
	break;

	case 50:
		arg1 = ctx->pop32();
		PTR_00417388 = _thing2[arg1].field_0.data();
		ctx->EAX.setVal(1);
		break;

	case 51:
		PTR_00417388 = nullptr;
		ctx->EAX.setVal(1);
		break;

	case 52:
		arg1 = ctx->pop32();
		/* HELP */
		//FUN_0040c614(arg1);
		ctx->EAX.setVal(1);
		break;

	case 53: {
		arg1 = ctx->pop32();
		VM::ValAddr adr = ctx->popReg();
		uint kode = ctx->getMem8(adr);
		_messageProc._keyCodes[arg1] = kode;
		ctx->EAX.setVal(kode);
	} break;

	case 54:
		arg1 = ctx->pop32();
		ctx->EAX.setVal(rndRange16(arg1));
		break;

	case 55: {
		VM::ValAddr regRef = ctx->popReg(); //implement
		Common::String str = ctx->getString(regRef);

		char buffer[256];
		int a = 0, b = 0, c = 0, d = 0;
		if ( sscanf(str.c_str(), "%s %d %d %d %d", buffer, &a, &b, &c, &d) > 0) {
			stopMidi();
			stopSounds();

			playVideo(Common::String(buffer), Common::Point(a, b), Common::Point(c, d));

			if (_d2_fld19 != 0xff) {
				/* vm func 58 */
			}

			if (_midiTrack != -1) {
				scriptFunc16(_midiTrack);
			}
		}
		ctx->EAX.setVal(1);
	}
	break;

	case 56: {
		VM::ValAddr regRef = ctx->popReg(); //implement
		Common::String str = ctx->getString(regRef);
		warning("Create process: %s", str.c_str());
		ctx->EAX.setVal(1);
	} break;

	case 57: {
		VM::ValAddr regRef = ctx->popReg(); //implement
		Common::String str = ctx->getString(regRef);
		if (_keySeq.find(str) != Common::String::npos) {
			_keySeq.clear();
			ctx->EAX.setVal(1);
		} else
			ctx->EAX.setVal(0);
	} break;

	case 58: {
		arg1 = ctx->pop32();
		/* CD AUDIO */
		ctx->EAX.setVal(1);
	} break;

	case 59: {
		arg1 = ctx->pop32();
		/* CD AUDIO */
		ctx->EAX.setVal(1);
	} break;

	case 60:
		arg1 = ctx->pop32();
		if (arg1 == 0)
			_scrollTrackObj = -1;
		else
			_scrollTrackObj = _curObjIndex;
		ctx->EAX.setVal(1);
		break;

	case 61: {
		arg1 = ctx->pop32();
		VM::ValAddr adr = ctx->popReg();
		Common::String tmp = ctx->getString(adr);

		int val1 = 0, val2 = 0, val3 = 0, val4 = 0;
		sscanf(tmp.c_str(), "%d %d %d %d", &val1, &val2, &val3, &val4);

		if (arg1 == 0) {
			_scrollBorderL = val1;
			_scrollBorderR = val2;
			_scrollBorderU = val3;
			_scrollBorderB = val4;
		} else {
			_scrollSpeed = val1;
			_scrollCutoff = val2;
			_scrollSpeedReduce = val3;
		}
		ctx->EAX.setVal(1);
	} break;

	default:
		warning("Call Dispatcher %d", funcID);
		ctx->EAX.setVal(0);
		break;
	}
}

void GamosEngine::callbackVMCallDispatcher(void *engine, VM::Context *ctx, uint32 funcID) {
	GamosEngine *gamos = (GamosEngine *)engine;
	gamos->vmCallDispatcher(ctx, funcID);
}

uint32 GamosEngine::scriptFunc19(uint32 id) {
	BYTE_004177fc = 1;
	createGfxObject(id, Common::Point(DAT_00417220.x * _gridCellW, DAT_00417220.y * _gridCellH), false);

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


bool GamosEngine::createGfxObject(uint32 id, Common::Point position, bool staticObject) {
	Sprite &spr = _sprites[id];
	Object *gfxObj = getFreeObject();

	gfxObj->flags |= Object::FLAG_GRAPHIC;

	if (spr.field_1 & 1)
		gfxObj->flags |= Object::FLAG_DIRTRECT;

	gfxObj->frameMax = spr.field_3;
	int16 idx = -1;
	if (!staticObject)
		idx = _curObjIndex;

	gfxObj->actObjIndex = idx;
	gfxObj->position = position;

	if (!staticObject) {
		if (!PTR_00417218) {
			gfxObj->priority = PTR_00417214->priority;
		} else {
			int32 index = gfxObj->index;
			if (PTR_00417218->curObjectId != -1) {
				Object *pgObj = &_objects[PTR_00417218->curObjectId];
				addDirtRectOnObject(pgObj);
				pgObj->flags &= ~Object::FLAG_GRAPHIC;
				if (PTR_00417218->tgtObjectId != PTR_00417218->curObjectId)
					removeObject(pgObj);
			}

			PTR_00417218->curObjectId = index;
			if (!(gfxObj->flags & Object::FLAG_DIRTRECT)) {
				if (PTR_00417218->tgtObjectId != -1)
					removeObject(&_objects[PTR_00417218->tgtObjectId]);
				PTR_00417218->tgtObjectId = index;
			}

			gfxObj->priority = PTR_00417218->priority;
			if (DAT_00417220 != DAT_00417228) {
				PTR_00417218->flags |= Object::FLAG_TRANSITION;
			}
		}
	} else {
		gfxObj->priority = PTR_00417218->priority;
		gfxObj->cell.x = -1;
		gfxObj->cell.y = -1;
	}

	FUN_00409378(id, gfxObj, staticObject);
	return true;
}

void GamosEngine::FUN_00409378(int32 sprId, Object *obj, bool p) {
	obj->flags &= ~(Object::FLAG_FLIPH | Object::FLAG_FLIPV);
	obj->actID = 0;
	obj->frame = 0;
	obj->sprId = sprId;
	obj->seqId = 0;

	Sprite &spr = _sprites[sprId];


	if (spr.field_2 == 1) {
		obj->pImg = &spr.sequences[0]->operator[](0);
		if (BYTE_004177f6 == 8) {
			if (spr.field_1 & 2)
				obj->flags |= Object::FLAG_FLIPH;
		} else if (BYTE_004177f6 == 4 && (spr.field_1 & 4)) {
			obj->flags |= Object::FLAG_FLIPV;
		}
	} else {
		if (BYTE_004177f6 == 1) {
			obj->seqId = 1;
			if (DAT_00417220.y == DAT_00417228.y && (spr.field_1 & 8))
				obj->seqId = 0;
		} else if (BYTE_004177f6 == 2) {
			obj->seqId = 3;
			if (DAT_00417228.y < DAT_00417220.y)
				obj->seqId = 2;
			else if (DAT_00417228.y > DAT_00417220.y) {
				obj->seqId = 4;
				if (spr.field_1 & 4) {
					obj->seqId = 2;
					obj->flags |= Object::FLAG_FLIPV;
				}
			} else if (DAT_00417220.x == DAT_00417228.x && (spr.field_1 & 8))
				obj->seqId = 0;
		} else if (BYTE_004177f6 == 4) {
			obj->seqId = 5;
			if (DAT_00417220.y == DAT_00417228.y && (spr.field_1 & 8))
				obj->seqId = 0;
			else if (spr.field_1 & 4) {
				obj->seqId = 1;
				obj->flags |= Object::FLAG_FLIPV;
			}
		} else {
			obj->seqId = 7;
			if (DAT_00417220.y == DAT_00417228.y) {
				if ((spr.field_1 & 8) && DAT_00417220.x == DAT_00417228.x)
					obj->seqId = 0;
				else if (spr.field_1 & 2) {
					obj->seqId = 3;
					obj->flags |= Object::FLAG_FLIPH;
				}
			} else {
				if (DAT_00417228.y < DAT_00417220.y) {
					obj->seqId = 8;
					if (spr.field_1 & 2) {
						obj->seqId = 2;
						obj->flags |= Object::FLAG_FLIPH;
					}
				} else {
					obj->seqId = 6;
					if (spr.field_1 & 4) {
						obj->seqId = 8;
						obj->flags |= Object::FLAG_FLIPV;

						if (spr.field_1 & 2) {
							obj->seqId = 2;
							obj->flags |= Object::FLAG_FLIPH;
						}
					} else if (spr.field_1 & 2) {
						obj->seqId = 4;
						obj->flags |= Object::FLAG_FLIPH;
					}
				}
			}
		}
		obj->pImg = &spr.sequences[obj->seqId]->operator[](0);
	}
	if (!p) {
		obj->cell = DAT_00417228;
		FUN_0040921c(obj);
	} else {
		obj->flags |= Object::FLAG_FREECOORDS;
	}

	addDirtRectOnObject(obj);
}

void GamosEngine::FUN_004095a0(Object *obj) {
	if (obj->curObjectId != -1) {
		Object &yobj = _objects[obj->curObjectId];
		addDirtRectOnObject(&yobj);
		if (DAT_00417228 != DAT_00417220)
			obj->flags |= Object::FLAG_TRANSITION;
		FUN_00409378(yobj.sprId, &yobj, false);
	}
}

void GamosEngine::removeSubtitles(Object *obj) {
	if (obj->state.flags & 2) {
		obj->state.flags &= ~2;
		//for (int index = obj->index; index < _objects.size(); index++) {
		for (int index = 0; index < _objects.size(); index++) {
			Object *pobj = &_objects[index];
			if (pobj->isOverlayObject() && pobj->actObjIndex == obj->index)
				removeObjectMarkDirty(pobj);
		}
	}
}

void GamosEngine::FUN_0040255c(Object *obj) {
	if (obj == PTR_004121b4) {
		PTR_004121b4 = nullptr;
		int32 n = 0;

		int16 objIndex = -1;
		if (obj)
			objIndex = obj->index;

		for (int32 i = 0; i < _objects.size(); i++) {
			Object &robj = _objects[i];

			if (robj.index > objIndex)
				n++;

			if (robj.isActionObject() && _objectActions[robj.actID].actType == 3) {
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
	if (_unk9 == 0)
		_mouseCursorImgId = id;
	else
		_mouseCursorImgId = -1;

	_cursorFrame = 0;
}


bool GamosEngine::FUN_00409600(Object *obj, Common::Point pos) {
	if (obj->curObjectId == -1)
		return false;

	Object &gfxobj = _objects[obj->curObjectId];
	if (Common::Rect(gfxobj.position.x, gfxobj.position.y, gfxobj.position.x + gfxobj.pImg->image->surface.w, gfxobj.position.y + gfxobj.pImg->image->surface.h).contains(pos))
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

	if (act1 != ACT_NONE)
		tmpb |= act1 | 0x40;

	actPos += Common::Point(_scrollX, _scrollY);

	Object *pobj = nullptr;
	uint8 actT = 0;
	uint8 pobjF5 = 255;

	for (int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if (obj.isActionObject()) {
			ObjectAction &action = _objectActions[obj.actID];
			if (action.actType == 1)
				obj.inputFlag = tmpb;
			else if (action.actType == 2)
				obj.inputFlag = tmpb & 0x4f;
			else if (action.actType == 3) {
				if (&obj == PTR_004121b4)
					obj.inputFlag = tmpb & 0x4f;
				else
					obj.inputFlag = 0;
			}

			if ((!pobj || obj.priority <= pobjF5) && FUN_00409600(&obj, actPos)) {
				actT = action.actType;
				pobjF5 = obj.priority;
				pobj = &obj;
			}
		}
	}

	if (!pobj) {
		DAT_004173f0.x = actPos.x / _gridCellW;
		DAT_004173f0.y = actPos.y / _gridCellH;
		DAT_00417803 = _states.at(DAT_004173f0).actid;
	} else {
		DAT_00417803 = actT;
		if (actT == 2) {
			if (act2 == ACT_NONE)
				tmpb |= 0x10;
			else if (act2 == ACT2_81)
				tmpb |= 0x20;

			pobj->inputFlag = tmpb;
		} else if (actT == 3 && (tmpb == 0x90 || tmpb == 0xa0)) {
			PTR_004121b4 = pobj;
			pobj->inputFlag = tmpb;
		}

		DAT_004173f0 = pobj->cell;
	}

	DAT_00417805 = act2;
	if (act2 == ACT2_82 || act2 == ACT2_83) {
		DAT_004177fe = act2;
		DAT_004177fd = DAT_00417803;
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
	uint8 sv6 = _preprocDataId;
	Common::Point sv8 = DAT_00417228;
	Common::Point sv10 = DAT_00417220;
	int sv11 = _curObjIndex;
	Object *sv12 = PTR_00417218;
	ObjectAction *sv13 = PTR_00417214;

	uint32 res = doActions(a, true);

	BYTE_004177fc = sv1;
	BYTE_004177f6 = sv2;
	PTR_004173e8 = sv3;
	_preprocDataId = sv6;
	DAT_00417228 = sv8;
	DAT_00417220 = sv10;
	_curObjIndex = sv11;
	PTR_00417218 = sv12;
	PTR_00417214 = sv13;

	return res;
}

void GamosEngine::addSubtitles(VM::Context *ctx, byte memtype, int32 offset, int32 sprId, int32 x, int32 y) {
	removeSubtitles(PTR_00417218);
	PTR_00417218->state.flags |= 2;

	while (true) {
		uint8 ib = ctx->getMem8(memtype, offset);
		offset++;

		if (ib == 0)
			break;

		if (ib == 0xf) {
			byte flg = ctx->getMem8(memtype, offset);
			offset++;
			byte b2 = ctx->getMem8(memtype, offset);
			offset++;

			if ((flg & 0x70) == 0x20) {
				byte funcid = ctx->getMem8(memtype, offset);
				offset++;
				warning("CHECKIT and write funcid %d", funcid);
			} else {
				if ((flg & 0x70) == 0 || (flg & 0x70) == 0x10) {
					int32 boff = 0;
					byte btp = VM::REF_EDI;

					if ((flg & 0x70) == 0x10)
						btp = VM::REF_EBX;

					if ((flg & 0x80) == 0) {
						boff = ctx->getMem8(memtype, offset);
						offset++;
					} else {
						boff = ctx->getMem32(memtype, offset);
						offset += 4;
					}

					Common::String tmp;
					switch (flg & 7) {
					case 0:
						tmp = gamos_itoa((int32)(int8)ctx->getMem8(btp, boff), 10);
						break;

					case 1: {
						VM::ValAddr addr;
						addr.setVal( ctx->getMem32(btp, boff) );
						tmp = ctx->getString(addr, b2);
					} break;

					case 2:
						tmp = ctx->getString(btp, boff, b2);
						break;

					case 3:
						tmp = gamos_itoa(ctx->getMem32(btp, boff), 10);
						break;

					case 4: {
						VM::ValAddr addr;
						addr.setVal( ctx->getMem32(btp, boff) );
						tmp = gamos_itoa(ctx->getMem32(addr), 10);
					} break;

					case 5:
						break;
					}

					for (int i = 0; i < tmp.size(); i++) {
						addSubtitleImage((uint8)tmp[i], sprId, &x, y);
					}
				}
			}
		} else {
			addSubtitleImage(ib, sprId, &x, y);
		}
	}

}

Object *GamosEngine::addSubtitleImage(uint32 frame, int32 spr, int32 *pX, int32 y) {
	Object *gfxObj = getFreeObject();
	gfxObj->flags |= Object::FLAG_GRAPHIC | Object::FLAG_OVERLAY | Object::FLAG_FREECOORDS;
	gfxObj->frame = 0;
	gfxObj->frameMax = 1;
	gfxObj->priority = PTR_00417218->priority;
	gfxObj->cell.x = -1;
	gfxObj->cell.y = -1;
	gfxObj->actObjIndex = _curObjIndex;
	gfxObj->position.x = *pX;
	gfxObj->position.y = y;
	gfxObj->sprId = spr;
	gfxObj->seqId = 0;
	gfxObj->frame = frame - _sprites[spr].field_1;
	gfxObj->pImg = &_sprites[spr].sequences[gfxObj->seqId]->operator[](gfxObj->frame);

	*pX += gfxObj->pImg->image->surface.w - gfxObj->pImg->xoffset;

	addDirtRectOnObject(gfxObj);
	return gfxObj;
}

bool GamosEngine::FUN_00402bc4() {
	if (RawKeyCode == ACT_NONE) {
		_vm.memory().setU8(_addrKeyCode, 0);
		_vm.memory().setU8(_addrKeyDown, 0);
	} else {
		_vm.memory().setU8(_addrKeyCode, RawKeyCode);
		_vm.memory().setU8(_addrKeyDown, 1);
	}

	if (_vm.memory().getU8(_addrBlk12) != 0)
		return false;

	uint32 frameval = _vm.memory().getU32(_addrCurrentFrame);
	_vm.memory().setU32(_addrCurrentFrame, frameval + 1);

	uint8 fpsval = _vm.memory().getU8(_addrFPS);

	if (fpsval == 0) {
		fpsval = 1;
		_vm.memory().setU8(_addrFPS, 1);
	} else if (fpsval > 50) {
		fpsval = 50;
		_vm.memory().setU8(_addrFPS, 50);
	}

	if (fpsval != _fps) {
		_fps = fpsval;
		setFPS(_fps);
	}

	return true;
}

void GamosEngine::FUN_00407db8(uint8 p) {
	if ((p == 0x82) || (p == 0x83))
		_pathTargetCell = DAT_004173f8;
	else
		_pathTargetCell = DAT_004173f0;

	_pathStartCell = PTR_00417218->cell;
	_pathDir4 = -1;
	_pathDir8 = -1;
	_pathInMove = false;
}

byte GamosEngine::FUN_00408648(uint8 p1, uint8 p2, uint8 p3) {
	FUN_00407db8(p1);

	if (p1 == 0x82 || p1 == 0x83) {
		if (p1 != DAT_004177fe)
			return 0;
		if (p2 != 0xff && p2 != DAT_004177fd)
			return 0;
	} else {
		if (p1 != 0xe) {
			if (p3 == 0xff)
				return FUN_004084bc(p2);
			else
				return FUN_00408510(p2);
		}
		if (p2 != 0xff && p2 != DAT_00417803)
			return 0;
	}

	if (p3 == 0xff)
		return pathFindMoveToTarget();
	else if (p3 == 0xfe)
		return FUN_0040856c();
	else
		return FUN_004085d8(p2);
}

byte GamosEngine::FUN_004084bc(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			const uint8 id = _states.at(i, j).actid;
			if (id == p)
				_pathMap.at(i, j) = PATH_TARGET;
			else
				_pathMap.at(i, j) = PATH_FREE;
		}
	}
	return pathFindCalcMove(true);
}

byte GamosEngine::FUN_00408510(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			const uint8 id = _states.at(i, j).actid;

			if (id == 0xfe)
				_pathMap.at(i, j) = PATH_FREE;
			else if (id == p)
				_pathMap.at(i, j) = PATH_TARGET;
			else
				_pathMap.at(i, j) = PATH_OBSTACLE;
		}
	}
	return pathFindCalcMove(false);
}

byte GamosEngine::FUN_0040856c() {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint8 id = _states.at(i, j).actid;

			if (id == 0xfe)
				_pathMap.at(i, j) = PATH_FREE;
			else
				_pathMap.at(i, j) = PATH_OBSTACLE;
		}
	}
	_pathMap.at(_pathTargetCell) = PATH_TARGET;
	return pathFindCalcMove(false);
}

byte GamosEngine::FUN_004085d8(uint8 p) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			uint8 id = _states.at(i, j).actid;

			if (id == p)
				_pathMap.at(i, j) = PATH_FREE;
			else
				_pathMap.at(i, j) = PATH_OBSTACLE;
		}
	}
	_pathMap.at(_pathTargetCell) = PATH_TARGET;
	return pathFindCalcMove(false);
}

byte GamosEngine::pathFindCalcMove(bool faceTarget) {
	_pathMap.at(_pathStartCell.x, _pathStartCell.y) = PATH_STEP1;

	while (true) {
		byte res = pathFindSetNeighbor(PATH_STEP1, PATH_STEP3);
		if (res == 0) // no set
			return 0;
		else if (res == 1) { // target achieve
			if (faceTarget)
				return pathFindMoveToTarget();
			else
				return pathFindTraceMove(PATH_STEP1);
		}

		res = pathFindSetNeighbor(PATH_STEP3, PATH_STEP2);
		if (res == 0) // no set
			return 0;
		else if (res == 1) { // target achieve
			if (faceTarget)
				return pathFindMoveToTarget();
			else
				return pathFindTraceMove(PATH_STEP3);
		}

		res = pathFindSetNeighbor(PATH_STEP2, PATH_STEP1);
		if (res == 0) // no set
			return 0;
		else if (res == 1) { // target achieve
			if (faceTarget)
				return pathFindMoveToTarget();
			else
				return pathFindTraceMove(PATH_STEP2);
		}
	}
}

byte GamosEngine::pathFindMoveToTarget() {
	int32 xdist = _pathStartCell.x - _pathTargetCell.x;
	if (xdist < 1)
		xdist = -xdist;

	int32 ydist = _pathStartCell.y - _pathTargetCell.y;
	if (ydist < 1)
		ydist = -ydist;

	if ((xdist == 0) && (ydist == 0))
		return 0;

	if ((xdist == 0) || (ydist / xdist) > 3) {
		if (ydist > 1) {
			_pathDir8 = PATH_DIR_D;
			if (_pathTargetCell.y <= _pathStartCell.y)
				_pathDir8 = PATH_DIR_U;
		}
		_pathDir4 = PATH_DIR_D;
		if (_pathTargetCell.y <= _pathStartCell.y)
			_pathDir4 = PATH_DIR_U;
	} else if ((ydist == 0) || (xdist / ydist) > 3) {
		if (xdist > 1) {
			_pathDir8 = PATH_DIR_R;
			if (_pathTargetCell.x <= _pathStartCell.x)
				_pathDir8 = PATH_DIR_L;
		}
		_pathDir4 = PATH_DIR_R;
		if (_pathTargetCell.x <= _pathStartCell.x)
			_pathDir4 = PATH_DIR_L;
	} else {
		if (_pathStartCell.x < _pathTargetCell.x) {
			_pathDir8 = PATH_DIR_DR;
			if (_pathTargetCell.y <= _pathStartCell.y)
				_pathDir8 = PATH_DIR_UR;
		} else {
			_pathDir8 = PATH_DIR_DL;
			if (_pathTargetCell.y <= _pathStartCell.y)
				_pathDir8 = PATH_DIR_UL;
		}

		if (ydist < xdist) {
			_pathDir4 = PATH_DIR_R;
			if (_pathTargetCell.x <= _pathStartCell.x)
				_pathDir4 = PATH_DIR_L;
		} else {
			_pathDir4 = PATH_DIR_D;
			if (_pathTargetCell.y <= _pathStartCell.y)
				_pathDir4 = PATH_DIR_U;
		}
	}

	_pathInMove = true;
	return 1;
}

byte GamosEngine::pathFindTraceMove(uint8 p) {
	int32 x = _pathTargetCell.x;
	int32 y = _pathTargetCell.y;
	int32 px = -1;
	int32 py = -1;

	while (true) {
		int32 xdist = _pathStartCell.x - x;
		if (xdist < 1)
			xdist = -xdist;
		int32 ydist = _pathStartCell.y - y;
		if (ydist < 1)
			ydist = -ydist;

		int32 xx = x;
		int32 yy = y;

		if (ydist < xdist) {
			if (x >= 1 && _pathMap.at(x - 1, y) == p) {
				xx = x - 1;
			} else if (x <= _pathRight - 1 && _pathMap.at(x + 1, y) == p) {
				xx = x + 1;
			} else if (y >= 1 && _pathMap.at(x, y - 1) == p) {
				yy = y - 1;
			} else if (y <= _pathBottom - 1 && _pathMap.at(x, y + 1) == p) {
				yy = y + 1;
			} else {
				return ydist;
			}
		} else {
			if (y >= 1 && _pathMap.at(x, y - 1) == p) {
				yy = y - 1;
			} else if (y <= _pathBottom - 1 && _pathMap.at(x, y + 1) == p) {
				yy = y + 1;
			} else if (x >= 1 && _pathMap.at(x - 1, y) == p) {
				xx = x - 1;
			} else if (x <= _pathRight - 1 && _pathMap.at(x + 1, y) == p) {
				xx = x + 1;
			} else {
				return ydist;
			}
		}

		if (xx == _pathStartCell.x && yy == _pathStartCell.y) {
			_pathDir4 = PATH_DIR_R;
			if (x <= xx) {
				_pathDir4 = PATH_DIR_L;
				if (x >= xx) {
					_pathDir4 = PATH_DIR_D;
					if (y <= yy)
						_pathDir4 = PATH_DIR_U;
				}
			}
			if (px != -1) {
				if (py > yy) {
					_pathDir8 = PATH_DIR_DR;
					if (px <= xx) {
						_pathDir8 = PATH_DIR_DL;
						if (px >= xx)
							_pathDir8 = PATH_DIR_D;
					}
				} else if (py < yy) {
					_pathDir8 = PATH_DIR_UR;
					if (px <= xx) {
						_pathDir8 = PATH_DIR_UL;
						if (px >= xx)
							_pathDir8 = PATH_DIR_U;
					}
				} else {
					_pathDir8 = PATH_DIR_R;
					if (px <= xx)
						_pathDir8 = PATH_DIR_L;
				}
			}
			_pathInMove = true;
			return 1;
		}

		py = y;
		px = x;

		y = yy;
		x = xx;

		if (p == PATH_STEP3)
			p = PATH_STEP1;
		else if (p == PATH_STEP2)
			p = PATH_STEP3;
		else if (p == PATH_STEP1)
			p = PATH_STEP2;
	}
}

byte GamosEngine::pathFindSetNeighbor(uint8 checkVal, uint8 setVal) {
	uint8 ret = 0;

	for (int32 y = 0; y < _statesHeight; y++) {
		for (int32 x = 0; x < _statesWidth; x++) {
			uint8 &rval = _pathMap.at(x, y);
			if (rval == PATH_FREE) {
				if ((x > 0 && _pathMap.at(x - 1, y) == checkVal) ||
				        (x < _pathRight && _pathMap.at(x + 1, y) == checkVal) ||
				        (y > 0 && _pathMap.at(x, y - 1) == checkVal) ||
				        (y < _pathBottom && _pathMap.at(x, y + 1) == checkVal)) {
					ret = setVal;
					rval = setVal;
				}
			} else if (rval == PATH_TARGET) {
				if ((x > 0 && _pathMap.at(x - 1, y) == checkVal) ||
				        (x < _pathRight && _pathMap.at(x + 1, y) == checkVal) ||
				        (y > 0 && _pathMap.at(x, y - 1) == checkVal) ||
				        (y < _pathBottom && _pathMap.at(x, y + 1) == checkVal)) {
					_pathTargetCell.x = x;
					_pathTargetCell.y = y;
					return 1;
				}
			}
		}
	}
	return ret;
}

byte GamosEngine::FUN_004088cc(uint8 p1, uint8 p2, uint8 p3) {
	FUN_00407db8(p1);

	if (p1 == 0x82 || p1 == 0x83) {
		if (p1 != DAT_004177fe)
			return 0;

		if ( (_thing2[p2].field_0[ DAT_004177fd >> 3 ] & (1 << (DAT_004177fd & 7))) == 0 )
			return 0;
	} else {
		if (p1 != 0xe) {
			if (p3 == 0xff)
				return FUN_004086e4(_thing2[p2].field_0);
			else
				return FUN_00408778(_thing2[p2].field_0);
		}

		if ( (_thing2[p2].field_0[ DAT_00417803 >> 3 ] & (1 << (DAT_00417803 & 7))) == 0 )
			return 0;
	}

	if (p3 == 0xff)
		return pathFindMoveToTarget();
	else if (p3 == 0xfe)
		return FUN_0040881c(_thing2[p2].field_0);
	else
		return FUN_0040856c();
}

byte GamosEngine::FUN_004086e4(const Common::Array<byte> &arr) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			const uint8 id = _states.at(i, j).actid;

			if ( ((arr[id >> 3]) & (1 << (id & 7))) == 0 )
				_pathMap.at(i, j) = PATH_FREE;
			else
				_pathMap.at(i, j) = PATH_TARGET;
		}
	}
	return pathFindCalcMove(true);
}

byte GamosEngine::FUN_00408778(const Common::Array<byte> &arr) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			const uint8 id = _states.at(i, j).actid;

			if ( ((arr[id >> 3]) & (1 << (id & 7))) == 0 )
				_pathMap.at(i, j) = PATH_OBSTACLE;
			else
				_pathMap.at(i, j) = PATH_TARGET;
		}
	}
	return pathFindCalcMove(false);
}

byte GamosEngine::FUN_0040881c(const Common::Array<byte> &arr) {
	for (int j = 0; j < _statesHeight; j++) {
		for (int i = 0; i < _statesWidth; i++) {
			const uint8 id = _states.at(i, j).actid;

			if ( ((arr[id >> 3]) & (1 << (id & 7))) == 0 )
				_pathMap.at(i, j) = PATH_OBSTACLE;
			else
				_pathMap.at(i, j) = PATH_FREE;
		}
	}
	_pathMap.at(_pathTargetCell) = PATH_TARGET;
	return pathFindCalcMove(false);
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
	act_4.actid = 0;
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
				a.actid = rstream.readByte();
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
						a.actid = rstream.readByte();
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
				a.actid = rstream.readByte();
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

void GamosEngine::FUN_0040279c(uint8 val, bool rnd) {
	FUN_004025d0();

	if (rnd)
		val = _thing2[val].field_1[ 1 + rndRange16(_thing2[val].field_1[0]) ];

	PTR_00417218->state = ObjState(val, 0, 1);

	ObjectAction &act = _objectActions[val];
	executeScript(act.onCreateAddress, &act, nullptr, -1, nullptr, PTR_00417218->cell, 1);
}

void GamosEngine::FUN_004025d0() {
	if (PTR_00417218->state.actid != 0xfe) {
		ObjectAction &act = _objectActions[PTR_00417218->state.actid];

		for (int i = 0; i < _objects.size(); i++) {
			Object &obj = _objects[i];
			if (obj.isStaticObject() &&
			        obj.actObjIndex == -1 &&
			        obj.cell.x == PTR_00417218->cell.x &&
			        obj.cell.y == PTR_00417218->cell.y) {

				removeObjectMarkDirty(&obj);
				break;
			}
		}

		executeScript(act.onDeleteAddress, &act, nullptr, -1, nullptr, PTR_00417218->cell, PTR_00417218->state.t);
		PTR_00417218->state = ObjState(0xfe, 0, 0xf);
	}
}


bool GamosEngine::updateMouseCursor(Common::Point mouseMove) {
	if (_mouseCursorImgId >= 0 && _unk9 == 0) {
		Sprite &cursorSpr = _sprites[_mouseCursorImgId];

		if (cursorSpr.field_3 > 1) {

			_cursorFrame++;
			if (_cursorFrame >= cursorSpr.field_3)
				_cursorFrame = 0;

			ImagePos &impos = cursorSpr.sequences[0]->operator[](_cursorFrame);
			Graphics::Surface &surf = impos.image->surface;
			CursorMan.replaceCursor(surf, -impos.xoffset, -impos.yoffset, 0);
			CursorMan.disableCursorPalette(true);
		} else {
			if (_currentCursor != _mouseCursorImgId) {
				ImagePos &impos = cursorSpr.sequences[0]->operator[](0);
				Graphics::Surface &surf = impos.image->surface;
				CursorMan.replaceCursor(surf, -impos.xoffset, -impos.yoffset, 0);
				CursorMan.disableCursorPalette(true);
			}
		}
	} else {
		if (_currentCursor != _mouseCursorImgId)
			CursorMan.setDefaultArrowCursor();
	}

	_currentCursor = _mouseCursorImgId;
	return true;
}

bool GamosEngine::scrollAndDraw() {
	if (_scrollTrackObj != -1) {
		Object &obj = _objects[_scrollTrackObj];
		Common::Point objPos(obj.cell.x * _gridCellW, obj.cell.y * _gridCellH);

		Common::Rect objArea;
		objArea.right = _scrollX + _width - (_scrollBorderR + 1) * _gridCellW;
		objArea.bottom = _scrollY + _height - (_scrollBorderB + 1) * _gridCellH;
		objArea.left = _scrollX + _scrollBorderL * _gridCellW;
		objArea.top = _scrollY + _scrollBorderU * _gridCellH;

		int32 lDistance = 0;
		int32 rDistance = 0;
		int32 uDistance = 0;
		int32 dDistance = 0;

		if (objPos.x < objArea.left) {
            lDistance = objArea.left - objPos.x;
            if (lDistance > _scrollX)
                lDistance = _scrollX;
        } else if (objPos.x > objArea.right) {
            int32 maxR = _bkgSize.x - _width - _scrollX;
            rDistance = objPos.x - objArea.right;
            if (rDistance > maxR)
                rDistance = maxR;
        }

        if (objPos.y < objArea.top) {
            uDistance = objArea.top - objPos.y;
            if (uDistance > _scrollY)
                uDistance = _scrollY;
        } else if (objPos.y > objArea.bottom) {
            int32 maxD = _bkgSize.y - _height - _scrollY;
            dDistance = objPos.y - objArea.bottom;
            if (dDistance > maxD)
                dDistance = maxD;
        }

		if (lDistance != 0 || rDistance != 0 || uDistance != 0 || dDistance != 0) {
			int32 lSpeed = _scrollSpeed;
			int32 rSpeed = _scrollSpeed;
			int32 uSpeed = _scrollSpeed;
			int32 dSpeed = _scrollSpeed;
			while (lDistance != 0 || rDistance != 0 || uDistance != 0 || dDistance != 0) {
				int32 lDelta = (lDistance < lSpeed) ? lDistance : lSpeed;
				int32 rDelta = (rDistance < rSpeed) ? rDistance : rSpeed;
				int32 uDelta = (uDistance < uSpeed) ? uDistance : uSpeed;
				int32 dDelta = (dDistance < dSpeed) ? dDistance : dSpeed;

				_scrollX += rDelta - lDelta;
				_scrollY += dDelta - uDelta;

				doDraw();

				lDistance -= lDelta;
				if (lDistance != 0 && lDistance <= _scrollCutoff) {
					lSpeed += _scrollSpeedReduce;
					if (lSpeed < 2)
						lSpeed = 1;
				}

				rDistance -= rDelta;
				if (rDistance != 0 && rDistance <= _scrollCutoff) {
					rSpeed += _scrollSpeedReduce;
					if (rSpeed < 2)
						rSpeed = 1;
				}

				uDistance -= uDelta;
				if (uDistance != 0 && uDistance <= _scrollCutoff) {
					uSpeed += _scrollSpeedReduce;
					if (uSpeed < 2)
						uSpeed = 1;
				}

				dDistance -= dDelta;
				if (dDistance != 0 && dDistance <= _scrollCutoff) {
					dSpeed += _scrollSpeedReduce;
					if (dSpeed < 2)
						dSpeed = 1;
				}

				_system->delayMillis(1000 / 15); // 15fps
			}
		}
	}

	doDraw();

	return true;
}

Common::String GamosEngine::gamos_itoa(int n, uint radix) {
	Common::String tmp;
	bool minus = false;
	uint un = n;
	if (radix == 10 && n < 0) {
		un = -n;
		minus = true;
	}

	if (un == 0) {
		tmp += '0';
	} else {
		while (un != 0) {
			uint r = un % radix;
			un /= radix;
			if (r > 9)
				tmp += 'A' + r - 10;
			else
				tmp += '0' + r;
		}
	}
	if (minus)
		tmp += '-';

	for (int i = 0, j = tmp.size() - 1; i < j; i++, j--) {
		char c = tmp[i];
		tmp.setChar(tmp[j], i);
		tmp.setChar(c, j);
	}
	return tmp;
}


bool GamosEngine::FUN_0040705c(int a, int b) {
	static const int arr[8] = {PATH_DIR_U, PATH_DIR_UL, PATH_DIR_L, PATH_DIR_DL, PATH_DIR_D, PATH_DIR_DR, PATH_DIR_R, PATH_DIR_UR};
	int v = DAT_004173ec;
	if (v > 3) {
		v -= 4;
		a = arr[a];
	}

	return ((a + v * 2) & 7) == b;
}

int GamosEngine::txtInputBegin(VM::Context *ctx, byte memtype, int32 offset, int sprId, int32 x, int32 y) {
	if (memtype != VM::REF_EDI) {
		error("Unsupported memtype");
		return 0;
	}

	if (_txtInputActive == false) {
		removeSubtitles(PTR_00417218);
		PTR_00417218->state.flags |= 2;
		_txtInputVmOffset = offset;
		_txtInputSpriteID = sprId;
		_txtInputX = x;
		_txtInputY = y;
		_txtInputObject = PTR_00417218;
		_txtInputAction = PTR_00417214;
		_txtInputObjectIndex = _curObjIndex;

		txtInputProcess(0);
		return 1;
	}
	return 0;
}

void GamosEngine::txtInputProcess(uint8 c) {
	PTR_00417218 = _txtInputObject;
	PTR_00417214 = _txtInputAction;
	_curObjIndex = _txtInputObjectIndex;

	Sprite &spr = _sprites[_txtInputSpriteID];

	uint8 ib = c;

	while (true) {
		if (ib == 0) {
			if (_txtInputActive) {
				_txtInputActive = false;
				removeSubtitles(PTR_00417218);
				return;
			}
			_txtInputActive = true;
			_txtInputTyped = false;
			ib = _vm.memory().getU8(_txtInputVmOffset);
			_txtInputVmOffset++;
			continue;
		} else if (ib == KeyCodes::WIN_BACK) {
			if (_txtInputTyped) {
				if (_txtInputLength)
					txtInputEraseBack(1);
				return;
			}
		} else if (ib == KeyCodes::WIN_RETURN) {
			if (_txtInputTyped) {
				_txtInputBuffer[_txtInputLength] = 0;
				switch (_txtInputFlags & 7) {
					case 0:
						_txtInputVMAccess.setU8( atoi((char *)_txtInputBuffer) );
						break;

					case 1: {
						VmTxtFmtAccess adr(_vm);
						adr.setVal( _txtInputVMAccess.getU32() );
						adr.write(_txtInputBuffer, _txtInputLength + 1);
					} break;

					case 2:
						_txtInputVMAccess.write(_txtInputBuffer, _txtInputLength + 1);
						break;

					case 3:
						_txtInputVMAccess.setU32( atoi((char *)_txtInputBuffer) );
						break;

					case 4: {
						VmTxtFmtAccess adr(_vm);
						adr.setVal( _txtInputVMAccess.getU32() );
						adr.setU32( atoi((char *)_txtInputBuffer) );
					} break;
				}

				_txtInputTyped = false;
				ib = _vm.memory().getU8(_txtInputVmOffset);
				_txtInputVmOffset++;
				continue;
			}
		} else if (ib == 0xf) {
			_txtInputFlags = _vm.memory().getU8(_txtInputVmOffset);
			_txtInputVmOffset++;
			_txtInputMaxLength = _vm.memory().getU8(_txtInputVmOffset);
			_txtInputVmOffset++;

			if ((_txtInputFlags & 0x70) == 0 || (_txtInputFlags & 0x70) == 0x10) {
				_txtInputVMAccess.setMemType(VM::REF_EDI);
				if ((_txtInputFlags & 0x70) == 0x10) {
					_txtInputVMAccess.setMemType(VM::REF_EBX);
					_txtInputVMAccess.objMem = PTR_004173e8;
				}
				if ( (_txtInputFlags & 0x80) == 0 ) {
					_txtInputVMAccess.setOffset( _vm.memory().getU8(_txtInputVmOffset) );
					_txtInputVmOffset++;
				} else {
					_txtInputVMAccess.setOffset( _vm.memory().getU32(_txtInputVmOffset) );
					_txtInputVmOffset += 4;
				}
				switch (_txtInputFlags & 7) {
					case 0:
					case 3:
					case 4:
						_txtInputIsNumber = true;
						break;

					case 1:
					case 2:
						_txtInputIsNumber = false;
						break;
				}

				_txtInputLength = 0;
				_txtInputTyped = true;
				return;
			}
		} else if (ib == KeyCodes::WIN_ESCAPE) {
			if (_txtInputTyped) {
				if (_txtInputLength != 0) {
                    txtInputEraseBack(_txtInputLength);
                    return;
                }

				if (_txtInputActive) {
					_txtInputActive = false;
					removeSubtitles(PTR_00417218);
					return;
				}
				_txtInputActive = true;
				_txtInputTyped = false;
				ib = _vm.memory().getU8(_txtInputVmOffset);
				_txtInputVmOffset++;
				continue;
			}
		}

		if (_txtInputTyped) {
			if (_txtInputLength < _txtInputMaxLength) {
				if (ib < spr.field_1)
					ib = tolower(ib);
				if (ib > spr.field_2)
					ib = toupper(ib);
				if (ib >= spr.field_1 && ib <= spr.field_2 &&
				   (_txtInputIsNumber == false || Common::isDigit(ib))) {
					_txtInputBuffer[_txtInputLength] = ib;
					_txtInputObjects[_txtInputLength] = addSubtitleImage(ib, _txtInputSpriteID, &_txtInputX, _txtInputY);
					_txtInputLength++;
				}
			}
			return;
		} else {
			addSubtitleImage(ib, _txtInputSpriteID, &_txtInputX, _txtInputY);
			ib = _vm.memory().getU8(_txtInputVmOffset);
			_txtInputVmOffset++;
		}
	}
}

void GamosEngine::txtInputEraseBack(int n) {
	for (int32 i = _txtInputLength - 1; i >= 0 && n > 0; i--, n--) {
		ImagePos *ips = _txtInputObjects[i]->pImg;
		_txtInputX -= ips->image->surface.w - ips->xoffset;
		removeObjectMarkDirty(_txtInputObjects[i]);

		_txtInputLength--;
	}
}

bool GamosEngine::onTxtInputUpdate(uint8 c) {
	for(int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];
		if ((obj.flags & (Object::FLAG_GRAPHIC | Object::FLAG_VALID | Object::FLAG_HASACTION | Object::FLAG_TRANSITION)) == (Object::FLAG_GRAPHIC | Object::FLAG_VALID)) {
			if ((obj.frame + 1 == obj.frameMax) && obj.actObjIndex != -1) {
				obj.cell = _objects[ obj.actObjIndex ].cell;
			}
			updateGfxFrames(&obj, false, false);
		}
	}

	if (RawKeyCode != KeyCodes::WIN_SPACE && RawKeyCode != KeyCodes::WIN_RETURN &&
		(RawKeyCode == ACT_NONE || c != ACT_NONE) )
		return true;

	txtInputProcess(RawKeyCode);
	return true;
}

bool GamosEngine::eventsSkip(bool breakOnInput) {
	bool brk = false;
	Common::Event e;
	while(_system->getEventManager()->pollEvent(e)) {
		if (breakOnInput){
			if (e.type == Common::EVENT_LBUTTONUP ||
				e.type == Common::EVENT_RBUTTONUP ||
				e.type == Common::EVENT_KEYUP)
				brk = true;
		}
	}

	return shouldQuit() || brk;
}


void GamosEngine::dumpActions() {
	Common::String t = Common::String::format("./actions_%d.txt", _currentModuleID);

	Common::DumpFile f;

	if (!f.open(t.c_str(), true))
		error("Cannot create actions dump file");

	int i = 0;
	for (ObjectAction &act : _objectActions) {
		f.writeString(Common::String::format("Act %d : actType %x mask %x priority %x storage size %x\n", i, act.actType, act.mask, act.priority, act.storageSize));
		if (act.onCreateAddress != -1) {
			t = _vm.disassembly(act.onCreateAddress);
			f.writeString(Common::String::format("Script1 : \n%s\n", t.c_str()));
		}

		if (act.onDeleteAddress != -1) {
			t = _vm.disassembly(act.onDeleteAddress);
			f.writeString(Common::String::format("Script2 : \n%s\n", t.c_str()));
		}

		int j = 0;
		for (Actions &sc : act.actions) {
			f.writeString(Common::String::format("subscript %d : \n", j));

			if (sc.conditionAddress != -1) {
				t = _vm.disassembly(sc.conditionAddress);
				f.writeString(Common::String::format("condition : \n%s\n", t.c_str()));
			}

			if (sc.functionAddress != -1) {
				t = _vm.disassembly(sc.functionAddress);
				f.writeString(Common::String::format("action : \n%s\n", t.c_str()));
			}

			j++;
		}


		f.writeString("\n\n#############################################\n\n");

		i++;
	}

	i = 0;
	for (const Actions &act : _subtitleActions) {
		if (act.flags & Actions::HAS_CONDITION) {
			t = _vm.disassembly(act.conditionAddress);
			f.writeString(Common::String::format("SubAct %d condition : \n%s\n", i, t.c_str()));
		}

		if (act.flags & Actions::HAS_FUNCTION) {
			t = _vm.disassembly(act.functionAddress);
			f.writeString(Common::String::format("SubAct %d action : \n%s\n", i, t.c_str()));
		}

		i++;
	}

	f.flush();
	f.close();

	warning("Actions saved into actions_%d.txt", _currentModuleID);
}

} // End of namespace Gamos
